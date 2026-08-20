/*
 * nvk_compute.c — FG-1 deterministic compute + storage-image smoke test.
 *
 * Fixed contract (also documented in compute.comp):
 *   - 64 uint32 input values: 0x10203040 + index * 0x01010101
 *   - storage-buffer output: (input ^ 0xa5a5a5a5) + index
 *   - 8x8 RGBA8 source image: R=(x*13+7), G=(y*29+11),
 *     B=((x+y)*17+3), A=255
 *   - sampled-image operation: swap source R/B into an RGBA8 storage image
 *   - 64 complete submit/wait/readback iterations
 *
 * This is intentionally a standalone artifact.  It reports the intended
 * compute/storage-buffer/sampled-image/storage-image/readback path and never
 * substitutes a CPU image operation for the GPU dispatch.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <switch.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "shaders/tri_shaders.h"

u32    __nx_applet_type = AppletType_Application;
size_t __nx_heap_size   = 0;

extern VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *pName);
extern void (*g_drm_shim_log_sink)(const char *);

#define IMAGE_W 8u
#define IMAGE_H 8u
#define ELEMENTS (IMAGE_W * IMAGE_H)
#define IMAGE_BYTES (ELEMENTS * 4u)
#define ITERATIONS 64u

static FILE *g_log;

static void shim_log_sink(const char *s)
{
   if (g_log) { fputs(s, g_log); fflush(g_log); }
   printf("%s", s); fflush(stdout);
}

static void logf_line(const char *fmt, ...)
{
   char buf[512];
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(buf, sizeof(buf), fmt, ap);
   va_end(ap);
   if (g_log) { fputs(buf, g_log); fputc('\n', g_log); fflush(g_log); }
   printf("%s\n", buf); fflush(stdout);
}
#define LOG(...) logf_line(__VA_ARGS__)

static uint32_t pick_mem_type(const VkPhysicalDeviceMemoryProperties *mp,
                              uint32_t type_bits, VkMemoryPropertyFlags want)
{
   for (uint32_t i = 0; i < mp->memoryTypeCount; i++) {
      if ((type_bits & (1u << i)) &&
          (mp->memoryTypes[i].propertyFlags & want) == want)
         return i;
   }
   return UINT32_MAX;
}

static uint32_t input_value(uint32_t i)
{
   return 0x10203040u + i * 0x01010101u;
}

static uint32_t expected_buffer_value(uint32_t i)
{
   return (input_value(i) ^ 0xa5a5a5a5u) + i;
}

static uint32_t source_pixel(uint32_t x, uint32_t y)
{
   uint32_t r = x * 13u + 7u;
   uint32_t g = y * 29u + 11u;
   uint32_t b = (x + y) * 17u + 3u;
   return r | (g << 8) | (b << 16) | 0xff000000u;
}

static uint32_t expected_image_pixel(uint32_t x, uint32_t y)
{
   uint32_t source = source_pixel(x, y);
   return (source & 0xff00ff00u) |
          ((source & 0x000000ffu) << 16) |
          ((source & 0x00ff0000u) >> 16);
}

static uint32_t checksum_words(const uint32_t *values, uint32_t count)
{
   uint32_t hash = 2166136261u; /* FNV-1a, deterministic and cheap to report. */
   for (uint32_t i = 0; i < count; i++) {
      hash ^= values[i];
      hash *= 16777619u;
   }
   return hash;
}

static uint32_t expected_buffer_checksum(void)
{
   uint32_t values[ELEMENTS];
   for (uint32_t i = 0; i < ELEMENTS; i++) values[i] = expected_buffer_value(i);
   return checksum_words(values, ELEMENTS);
}

static uint32_t expected_image_checksum(void)
{
   uint32_t values[ELEMENTS];
   for (uint32_t y = 0; y < IMAGE_H; y++)
      for (uint32_t x = 0; x < IMAGE_W; x++)
         values[y * IMAGE_W + x] = expected_image_pixel(x, y);
   return checksum_words(values, ELEMENTS);
}

int main(void)
{
   VkInstance inst = VK_NULL_HANDLE;
   VkDevice dev = VK_NULL_HANDLE;
   VkQueue queue = VK_NULL_HANDLE;
   VkCommandPool pool = VK_NULL_HANDLE;
   VkCommandBuffer cmd = VK_NULL_HANDLE;
   VkBuffer input_buf = VK_NULL_HANDLE, output_buf = VK_NULL_HANDLE;
   VkBuffer staging = VK_NULL_HANDLE, readback = VK_NULL_HANDLE;
   VkDeviceMemory input_mem = VK_NULL_HANDLE, output_mem = VK_NULL_HANDLE;
   VkDeviceMemory staging_mem = VK_NULL_HANDLE, readback_mem = VK_NULL_HANDLE;
   VkImage source_img = VK_NULL_HANDLE, destination_img = VK_NULL_HANDLE;
   VkDeviceMemory source_mem = VK_NULL_HANDLE, destination_mem = VK_NULL_HANDLE;
   VkImageView source_view = VK_NULL_HANDLE, destination_view = VK_NULL_HANDLE;
   VkSampler sampler = VK_NULL_HANDLE;
   VkShaderModule shader = VK_NULL_HANDLE;
   VkDescriptorSetLayout descriptor_layout = VK_NULL_HANDLE;
   VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
   VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
   VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
   VkPipeline pipeline = VK_NULL_HANDLE;
   void *input_cpu = NULL, *output_cpu = NULL, *staging_cpu = NULL, *readback_cpu = NULL;
   uint32_t qfi = UINT32_MAX;
   uint32_t failures = 0;
   VkResult r = VK_SUCCESS;
   PFN_vkQueueWaitIdle pQueueWaitIdle = NULL;
   PFN_vkDestroyInstance pDestroyInstance = NULL;
   PFN_vkDestroyDevice pDestroyDevice = NULL;
   PFN_vkDestroyBuffer pDestroyBuffer = NULL;
   PFN_vkFreeMemory pFreeMemory = NULL;
   PFN_vkDestroyImage pDestroyImage = NULL;
   PFN_vkDestroyImageView pDestroyImageView = NULL;
   PFN_vkDestroySampler pDestroySampler = NULL;
   PFN_vkDestroyShaderModule pDestroyShaderModule = NULL;
   PFN_vkDestroyDescriptorPool pDestroyDescriptorPool = NULL;
   PFN_vkDestroyDescriptorSetLayout pDestroyDescriptorSetLayout = NULL;
   PFN_vkDestroyPipelineLayout pDestroyPipelineLayout = NULL;
   PFN_vkDestroyPipeline pDestroyPipeline = NULL;
   PFN_vkDestroyCommandPool pDestroyCommandPool = NULL;

   g_log = fopen("sdmc:/nvk_compute.log", "w");
   if (__nxlink_host.s_addr != 0 && R_SUCCEEDED(socketInitializeDefault()))
      nxlinkStdio();
   LOG("=== NVK FG-1 compute/storage-image smoke [BUILD compute1] ===");
   LOG("contract: %u elements, %ux%u RGBA8, %u iterations", ELEMENTS,
       IMAGE_W, IMAGE_H, ITERATIONS);

   g_drm_shim_log_sink = shim_log_sink;
   setenv("NVK_I_WANT_A_BROKEN_VULKAN_DRIVER", "1", 1);
   setenv("MESA_SHADER_CACHE_DISABLE", "1", 1);
   setenv("MESA_LOG_FILE", "sdmc:/nvk_compute_mesa.log", 1);

   PFN_vkCreateInstance pCreateInstance =
      (PFN_vkCreateInstance)vk_icdGetInstanceProcAddr(NULL, "vkCreateInstance");
   if (!pCreateInstance) { LOG("FAIL setup: vkCreateInstance entrypoint missing"); goto done; }
   VkApplicationInfo app = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "nvk_compute", .apiVersion = VK_API_VERSION_1_1 };
   VkInstanceCreateInfo ici = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app };
   r = pCreateInstance(&ici, NULL, &inst);
   LOG("A vkCreateInstance -> %d", r);
   if (r != VK_SUCCESS) goto done;
   pDestroyInstance =
      (PFN_vkDestroyInstance)vk_icdGetInstanceProcAddr(inst, "vkDestroyInstance");

#define GI(n) ((PFN_##n)vk_icdGetInstanceProcAddr(inst, #n))
   PFN_vkEnumeratePhysicalDevices pEnum = GI(vkEnumeratePhysicalDevices);
   PFN_vkGetPhysicalDeviceMemoryProperties pMemP = GI(vkGetPhysicalDeviceMemoryProperties);
   PFN_vkGetPhysicalDeviceQueueFamilyProperties pQF = GI(vkGetPhysicalDeviceQueueFamilyProperties);
   PFN_vkCreateDevice pCreateDev = GI(vkCreateDevice);
   PFN_vkGetDeviceProcAddr pGDPA = GI(vkGetDeviceProcAddr);
   if (!pEnum || !pMemP || !pQF || !pCreateDev || !pGDPA) {
      LOG("FAIL setup: instance entrypoint missing"); goto done;
   }
   uint32_t ndev = 0;
   r = pEnum(inst, &ndev, NULL);
   LOG("B enumerate count -> %d (%u device(s))", r, ndev);
   if (r != VK_SUCCESS || ndev == 0) goto done;
   VkPhysicalDevice phys[4];
   if (ndev > 4) ndev = 4;
   r = pEnum(inst, &ndev, phys);
   if (r != VK_SUCCESS) { LOG("FAIL setup: enumerate devices -> %d", r); goto done; }

   uint32_t nqf = 0;
   pQF(phys[0], &nqf, NULL);
   VkQueueFamilyProperties qf[8];
   if (nqf > 8) nqf = 8;
   pQF(phys[0], &nqf, qf);
   for (uint32_t i = 0; i < nqf; i++) {
      LOG("C qfam[%u]: flags=0x%x count=%u", i, qf[i].queueFlags, qf[i].queueCount);
      if (qfi == UINT32_MAX && (qf[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) qfi = i;
   }
   if (qfi == UINT32_MAX) { LOG("FAIL setup: no compute-capable queue family"); goto done; }
   LOG("C using compute queue family %u", qfi);

   float priority = 1.0f;
   VkDeviceQueueCreateInfo qci = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = qfi, .queueCount = 1, .pQueuePriorities = &priority };
   VkDeviceCreateInfo dci = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1, .pQueueCreateInfos = &qci };
   r = pCreateDev(phys[0], &dci, NULL, &dev);
   LOG("D vkCreateDevice -> %d", r);
   if (r != VK_SUCCESS) goto done;

#define GD(n) ((PFN_##n)pGDPA(dev, #n))
   PFN_vkGetDeviceQueue pGetQueue = GD(vkGetDeviceQueue);
   PFN_vkAllocateMemory pAlloc = GD(vkAllocateMemory);
   PFN_vkMapMemory pMap = GD(vkMapMemory);
   PFN_vkCreateBuffer pCreateBuffer = GD(vkCreateBuffer);
   PFN_vkGetBufferMemoryRequirements pBufferRequirements = GD(vkGetBufferMemoryRequirements);
   PFN_vkBindBufferMemory pBindBuffer = GD(vkBindBufferMemory);
   PFN_vkCreateImage pCreateImage = GD(vkCreateImage);
   PFN_vkGetImageMemoryRequirements pImageRequirements = GD(vkGetImageMemoryRequirements);
   PFN_vkBindImageMemory pBindImage = GD(vkBindImageMemory);
   PFN_vkCreateImageView pCreateImageView = GD(vkCreateImageView);
   PFN_vkCreateSampler pCreateSampler = GD(vkCreateSampler);
   PFN_vkCreateShaderModule pCreateShaderModule = GD(vkCreateShaderModule);
   PFN_vkCreateDescriptorSetLayout pCreateDescriptorSetLayout = GD(vkCreateDescriptorSetLayout);
   PFN_vkCreateDescriptorPool pCreateDescriptorPool = GD(vkCreateDescriptorPool);
   PFN_vkAllocateDescriptorSets pAllocateDescriptorSets = GD(vkAllocateDescriptorSets);
   PFN_vkUpdateDescriptorSets pUpdateDescriptorSets = GD(vkUpdateDescriptorSets);
   PFN_vkCreatePipelineLayout pCreatePipelineLayout = GD(vkCreatePipelineLayout);
   PFN_vkCreateComputePipelines pCreateComputePipelines = GD(vkCreateComputePipelines);
   PFN_vkCreateCommandPool pCreateCommandPool = GD(vkCreateCommandPool);
   PFN_vkAllocateCommandBuffers pAllocateCommandBuffers = GD(vkAllocateCommandBuffers);
   PFN_vkBeginCommandBuffer pBeginCommandBuffer = GD(vkBeginCommandBuffer);
   PFN_vkCmdPipelineBarrier pCmdPipelineBarrier = GD(vkCmdPipelineBarrier);
   PFN_vkCmdCopyBufferToImage pCmdCopyBufferToImage = GD(vkCmdCopyBufferToImage);
   PFN_vkCmdCopyImageToBuffer pCmdCopyImageToBuffer = GD(vkCmdCopyImageToBuffer);
   PFN_vkCmdCopyBuffer pCmdCopyBuffer = GD(vkCmdCopyBuffer);
   PFN_vkCmdBindPipeline pCmdBindPipeline = GD(vkCmdBindPipeline);
   PFN_vkCmdBindDescriptorSets pCmdBindDescriptorSets = GD(vkCmdBindDescriptorSets);
   PFN_vkCmdDispatch pCmdDispatch = GD(vkCmdDispatch);
   PFN_vkEndCommandBuffer pEndCommandBuffer = GD(vkEndCommandBuffer);
   PFN_vkQueueSubmit pQueueSubmit = GD(vkQueueSubmit);
   pQueueWaitIdle = GD(vkQueueWaitIdle);
   pDestroyDevice = GD(vkDestroyDevice);
   pDestroyBuffer = GD(vkDestroyBuffer);
   pFreeMemory = GD(vkFreeMemory);
   pDestroyImage = GD(vkDestroyImage);
   pDestroyImageView = GD(vkDestroyImageView);
   pDestroySampler = GD(vkDestroySampler);
   pDestroyShaderModule = GD(vkDestroyShaderModule);
   pDestroyDescriptorPool = GD(vkDestroyDescriptorPool);
   pDestroyDescriptorSetLayout = GD(vkDestroyDescriptorSetLayout);
   pDestroyPipelineLayout = GD(vkDestroyPipelineLayout);
   pDestroyPipeline = GD(vkDestroyPipeline);
   pDestroyCommandPool = GD(vkDestroyCommandPool);
   if (!pGetQueue || !pAlloc || !pMap || !pCreateBuffer || !pBufferRequirements ||
       !pBindBuffer || !pCreateImage || !pImageRequirements || !pBindImage ||
       !pCreateImageView || !pCreateSampler || !pCreateShaderModule ||
       !pCreateDescriptorSetLayout || !pCreateDescriptorPool ||
       !pAllocateDescriptorSets || !pUpdateDescriptorSets || !pCreatePipelineLayout ||
       !pCreateComputePipelines || !pCreateCommandPool || !pAllocateCommandBuffers ||
       !pBeginCommandBuffer || !pCmdPipelineBarrier || !pCmdCopyBufferToImage ||
       !pCmdCopyImageToBuffer || !pCmdCopyBuffer || !pCmdBindPipeline ||
       !pCmdBindDescriptorSets || !pCmdDispatch || !pEndCommandBuffer ||
       !pQueueSubmit || !pQueueWaitIdle) {
      LOG("FAIL setup: required device entrypoint missing"); goto done;
   }
   pGetQueue(dev, qfi, 0, &queue);
   VkPhysicalDeviceMemoryProperties mp;
   pMemP(phys[0], &mp);

#define MAKE_HOST_BUFFER(out_buf, out_mem, out_cpu, size_bytes, usage_flags) do { \
      VkBufferCreateInfo _bi = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, \
         .size = (size_bytes), .usage = (usage_flags), .sharingMode = VK_SHARING_MODE_EXCLUSIVE }; \
      r = pCreateBuffer(dev, &_bi, NULL, &(out_buf)); \
      if (r != VK_SUCCESS) { LOG("FAIL resource: create host buffer -> %d", r); goto done; } \
      VkMemoryRequirements _mr; pBufferRequirements(dev, (out_buf), &_mr); \
      uint32_t _mt = pick_mem_type(&mp, _mr.memoryTypeBits, \
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); \
      if (_mt == UINT32_MAX) { LOG("FAIL resource: no host-visible buffer memory"); goto done; } \
      VkMemoryAllocateInfo _ai = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, \
         .allocationSize = _mr.size, .memoryTypeIndex = _mt }; \
      r = pAlloc(dev, &_ai, NULL, &(out_mem)); \
      if (r != VK_SUCCESS) { LOG("FAIL resource: alloc host buffer -> %d", r); goto done; } \
      r = pMap(dev, (out_mem), 0, VK_WHOLE_SIZE, 0, &(out_cpu)); \
      if (r != VK_SUCCESS || !(out_cpu)) { LOG("FAIL resource: map host buffer -> %d", r); goto done; } \
      r = pBindBuffer(dev, (out_buf), (out_mem), 0); \
      if (r != VK_SUCCESS) { LOG("FAIL resource: bind host buffer -> %d", r); goto done; } \
   } while (0)
#define MAKE_DEVICE_IMAGE(out_img, out_mem, image_format, usage_flags) do { \
      VkImageCreateInfo _ii = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, \
         .imageType = VK_IMAGE_TYPE_2D, .format = (image_format), \
         .extent = { IMAGE_W, IMAGE_H, 1 }, .mipLevels = 1, .arrayLayers = 1, \
         .samples = VK_SAMPLE_COUNT_1_BIT, .tiling = VK_IMAGE_TILING_OPTIMAL, \
         .usage = (usage_flags), .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED }; \
      r = pCreateImage(dev, &_ii, NULL, &(out_img)); \
      if (r != VK_SUCCESS) { LOG("FAIL resource: create image -> %d", r); goto done; } \
      VkMemoryRequirements _mr; pImageRequirements(dev, (out_img), &_mr); \
      uint32_t _mt = pick_mem_type(&mp, _mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); \
      if (_mt == UINT32_MAX) _mt = pick_mem_type(&mp, _mr.memoryTypeBits, 0); \
      if (_mt == UINT32_MAX) { LOG("FAIL resource: no image memory"); goto done; } \
      VkMemoryAllocateInfo _ai = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, \
         .allocationSize = _mr.size, .memoryTypeIndex = _mt }; \
      r = pAlloc(dev, &_ai, NULL, &(out_mem)); \
      if (r != VK_SUCCESS) { LOG("FAIL resource: alloc image -> %d", r); goto done; } \
      r = pBindImage(dev, (out_img), (out_mem), 0); \
      if (r != VK_SUCCESS) { LOG("FAIL resource: bind image -> %d", r); goto done; } \
   } while (0)

   MAKE_HOST_BUFFER(input_buf, input_mem, input_cpu, ELEMENTS * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
   MAKE_HOST_BUFFER(output_buf, output_mem, output_cpu, ELEMENTS * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
   MAKE_HOST_BUFFER(staging, staging_mem, staging_cpu, IMAGE_BYTES, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
   MAKE_HOST_BUFFER(readback, readback_mem, readback_cpu, IMAGE_BYTES, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
   for (uint32_t i = 0; i < ELEMENTS; i++) {
      ((uint32_t *)input_cpu)[i] = input_value(i);
      ((uint32_t *)output_cpu)[i] = 0;
   }
   for (uint32_t y = 0; y < IMAGE_H; y++)
      for (uint32_t x = 0; x < IMAGE_W; x++)
         ((uint32_t *)staging_cpu)[y * IMAGE_W + x] = source_pixel(x, y);
   memset(readback_cpu, 0, IMAGE_BYTES);
   armDCacheFlush(input_cpu, ELEMENTS * sizeof(uint32_t));
   armDCacheFlush(output_cpu, ELEMENTS * sizeof(uint32_t));
   armDCacheFlush(staging_cpu, IMAGE_BYTES);
   armDCacheFlush(readback_cpu, IMAGE_BYTES);
   LOG("E host-visible buffers initialized; CPU fallback disabled");

   MAKE_DEVICE_IMAGE(source_img, source_mem, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
   MAKE_DEVICE_IMAGE(destination_img, destination_mem, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
   VkImageViewCreateInfo source_vci = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = source_img, .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
   r = pCreateImageView(dev, &source_vci, NULL, &source_view);
   if (r != VK_SUCCESS) { LOG("FAIL resource: source image view -> %d", r); goto done; }
   source_vci.image = destination_img;
   r = pCreateImageView(dev, &source_vci, NULL, &destination_view);
   if (r != VK_SUCCESS) { LOG("FAIL resource: destination image view -> %d", r); goto done; }
   VkSamplerCreateInfo sci = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .maxLod = 1.0f };
   r = pCreateSampler(dev, &sci, NULL, &sampler);
   if (r != VK_SUCCESS) { LOG("FAIL resource: sampler -> %d", r); goto done; }
   LOG("F source image + storage image allocated (RGBA8, %ux%u)", IMAGE_W, IMAGE_H);

   VkShaderModuleCreateInfo smci = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = compute_spv_sz, .pCode = compute_spv };
   r = pCreateShaderModule(dev, &smci, NULL, &shader);
   LOG("G NAK compute shader module (%u bytes) -> %d", compute_spv_sz, r);
   if (r != VK_SUCCESS) goto done;
   VkDescriptorSetLayoutBinding bindings[4] = {
      { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL },
      { 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL },
      { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL },
      { 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL },
   };
   VkDescriptorSetLayoutCreateInfo dslci = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 4, .pBindings = bindings };
   r = pCreateDescriptorSetLayout(dev, &dslci, NULL, &descriptor_layout);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: descriptor layout -> %d", r); goto done; }
   VkDescriptorPoolSize pool_sizes[3] = {
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
   };
   VkDescriptorPoolCreateInfo dpci = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1, .poolSizeCount = 3, .pPoolSizes = pool_sizes };
   r = pCreateDescriptorPool(dev, &dpci, NULL, &descriptor_pool);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: descriptor pool -> %d", r); goto done; }
   VkDescriptorSetAllocateInfo dsai = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool, .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_layout };
   r = pAllocateDescriptorSets(dev, &dsai, &descriptor_set);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: descriptor set -> %d", r); goto done; }
   VkDescriptorBufferInfo input_di = { input_buf, 0, ELEMENTS * sizeof(uint32_t) };
   VkDescriptorBufferInfo output_di = { output_buf, 0, ELEMENTS * sizeof(uint32_t) };
   VkDescriptorImageInfo sampled_di = { sampler, source_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
   VkDescriptorImageInfo storage_di = { VK_NULL_HANDLE, destination_view, VK_IMAGE_LAYOUT_GENERAL };
   VkWriteDescriptorSet writes[4] = {
      { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptor_set, .dstBinding = 0,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &input_di },
      { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptor_set, .dstBinding = 1,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &output_di },
      { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptor_set, .dstBinding = 2,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = &sampled_di },
      { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptor_set, .dstBinding = 3,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .pImageInfo = &storage_di },
   };
   pUpdateDescriptorSets(dev, 4, writes, 0, NULL);
   VkPipelineLayoutCreateInfo plci = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1, .pSetLayouts = &descriptor_layout };
   r = pCreatePipelineLayout(dev, &plci, NULL, &pipeline_layout);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: layout -> %d", r); goto done; }
   VkPipelineShaderStageCreateInfo stage = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT, .module = shader, .pName = "main" };
   VkComputePipelineCreateInfo cpci = { .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = stage, .layout = pipeline_layout };
   r = pCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &cpci, NULL, &pipeline);
   LOG("G vkCreateComputePipelines -> %d", r);
   if (r != VK_SUCCESS) goto done;
   LOG("G intended path: NAK compute pipeline + 4 explicit bindings");

   VkCommandPoolCreateInfo cpci_pool = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = qfi };
   r = pCreateCommandPool(dev, &cpci_pool, NULL, &pool);
   if (r != VK_SUCCESS) { LOG("FAIL submit: command pool -> %d", r); goto done; }
   VkCommandBufferAllocateInfo cbai = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
   r = pAllocateCommandBuffers(dev, &cbai, &cmd);
   if (r != VK_SUCCESS) { LOG("FAIL submit: command buffer -> %d", r); goto done; }

   for (uint32_t iteration = 0; iteration < ITERATIONS; iteration++) {
      memset(output_cpu, 0, ELEMENTS * sizeof(uint32_t));
      memset(readback_cpu, 0, IMAGE_BYTES);
      armDCacheFlush(output_cpu, ELEMENTS * sizeof(uint32_t));
      armDCacheFlush(readback_cpu, IMAGE_BYTES);
      r = pBeginCommandBuffer(cmd, &(VkCommandBufferBeginInfo){
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT });
      if (r != VK_SUCCESS) { LOG("FAIL iteration %u: begin -> %d", iteration, r); goto done; }

      VkImageMemoryBarrier source_to_copy = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .image = source_img, .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, NULL, 0, NULL, 1, &source_to_copy);
      VkBufferImageCopy upload = { .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
         .imageExtent = { IMAGE_W, IMAGE_H, 1 } };
      pCmdCopyBufferToImage(cmd, staging, source_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &upload);
      VkImageMemoryBarrier source_to_sample = source_to_copy;
      source_to_sample.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      source_to_sample.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      source_to_sample.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      source_to_sample.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          0, 0, NULL, 0, NULL, 1, &source_to_sample);
      VkImageMemoryBarrier destination_to_compute = source_to_copy;
      destination_to_compute.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      destination_to_compute.newLayout = VK_IMAGE_LAYOUT_GENERAL;
      destination_to_compute.image = destination_img;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          0, 0, NULL, 0, NULL, 1, &destination_to_compute);
      VkBufferMemoryBarrier input_host_to_compute = { .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
         .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .buffer = input_buf, .offset = 0, .size = ELEMENTS * sizeof(uint32_t) };
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          0, 0, NULL, 1, &input_host_to_compute, 0, NULL);
      pCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
      pCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                             &descriptor_set, 0, NULL);
      pCmdDispatch(cmd, 1, 1, 1);
      VkImageMemoryBarrier destination_to_copy = destination_to_compute;
      destination_to_copy.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      destination_to_copy.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      destination_to_copy.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
      destination_to_copy.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, NULL, 0, NULL, 1, &destination_to_copy);
      VkBufferMemoryBarrier output_to_copy = { .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
         .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .buffer = output_buf, .offset = 0, .size = ELEMENTS * sizeof(uint32_t) };
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, NULL, 1, &output_to_copy, 0, NULL);
      VkBufferCopy output_copy = { 0, 0, ELEMENTS * sizeof(uint32_t) };
      pCmdCopyBuffer(cmd, output_buf, readback, 1, &output_copy);
      VkBufferImageCopy image_copy = { .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
         .imageExtent = { IMAGE_W, IMAGE_H, 1 } };
      pCmdCopyImageToBuffer(cmd, destination_img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            readback, 1, &image_copy);
      VkBufferMemoryBarrier readback_to_host = { .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
         .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT, .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .buffer = readback, .offset = 0, .size = IMAGE_BYTES };
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                          0, 0, NULL, 1, &readback_to_host, 0, NULL);
      r = pEndCommandBuffer(cmd);
      if (r != VK_SUCCESS) { LOG("FAIL iteration %u: end -> %d", iteration, r); goto done; }
      VkSubmitInfo submit = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = 1, .pCommandBuffers = &cmd };
      r = pQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
      if (r != VK_SUCCESS) { LOG("FAIL iteration %u: submit -> %d", iteration, r); goto done; }
      r = pQueueWaitIdle(queue);
      if (r != VK_SUCCESS) { LOG("FAIL iteration %u: wait/readback -> %d", iteration, r); goto done; }
      armDCacheFlush(output_cpu, ELEMENTS * sizeof(uint32_t));
      armDCacheFlush(readback_cpu, IMAGE_BYTES);

      uint32_t bad_buffers = 0, bad_images = 0;
      for (uint32_t i = 0; i < ELEMENTS; i++) {
         if (((uint32_t *)output_cpu)[i] != expected_buffer_value(i)) bad_buffers++;
         uint32_t x = i & 7u, y = i >> 3u;
         if (((uint32_t *)readback_cpu)[i] != expected_image_pixel(x, y)) bad_images++;
      }
      if (bad_buffers || bad_images) {
         failures++;
         LOG("FAIL iteration %u: buffer mismatches=%u image mismatches=%u first buffer=0x%08x expected=0x%08x first image=0x%08x expected=0x%08x",
             iteration, bad_buffers, bad_images, ((uint32_t *)output_cpu)[0],
             expected_buffer_value(0), ((uint32_t *)readback_cpu)[0], expected_image_pixel(0, 0));
      } else if (iteration == 0 || iteration + 1 == ITERATIONS) {
         LOG("iteration %u/%u VERIFY OK: buffer[0]=0x%08x image[0]=0x%08x",
             iteration + 1, ITERATIONS, ((uint32_t *)output_cpu)[0], ((uint32_t *)readback_cpu)[0]);
      }
   }
   if (failures == 0) {
      LOG("RESULT PASS: %u/%u iterations exact; checksum buffer=0x%08x image=0x%08x",
          ITERATIONS, ITERATIONS, checksum_words(output_cpu, ELEMENTS),
          checksum_words(readback_cpu, ELEMENTS));
      LOG("EXPECTED CHECKSUM: buffer=0x%08x image=0x%08x",
          expected_buffer_checksum(), expected_image_checksum());
      LOG("INTENDED GPU PATH EXECUTED: NAK compute -> storage buffers + sampled image -> storage image -> transfer readback");
      LOG("FALLBACK/BYPASS: none; CPU only initialized inputs and validated readback");
      LOG("GPU FAULT/ERROR NOTIFIER: inspect complete device log and Mesa log; no local fallback is accepted");
   } else {
      LOG("RESULT FAIL: %u/%u iterations mismatched; capability remains unproven", failures, ITERATIONS);
   }

done:
   if (pQueueWaitIdle && queue) pQueueWaitIdle(queue);
   if (pDestroyCommandPool && pool) pDestroyCommandPool(dev, pool, NULL);
   if (pDestroyPipeline && pipeline) pDestroyPipeline(dev, pipeline, NULL);
   if (pDestroyPipelineLayout && pipeline_layout) pDestroyPipelineLayout(dev, pipeline_layout, NULL);
   if (pDestroyDescriptorPool && descriptor_pool) pDestroyDescriptorPool(dev, descriptor_pool, NULL);
   if (pDestroyDescriptorSetLayout && descriptor_layout) pDestroyDescriptorSetLayout(dev, descriptor_layout, NULL);
   if (pDestroyShaderModule && shader) pDestroyShaderModule(dev, shader, NULL);
   if (pDestroySampler && sampler) pDestroySampler(dev, sampler, NULL);
   if (pDestroyImageView && source_view) pDestroyImageView(dev, source_view, NULL);
   if (pDestroyImageView && destination_view) pDestroyImageView(dev, destination_view, NULL);
   if (pDestroyImage && source_img) pDestroyImage(dev, source_img, NULL);
   if (pDestroyImage && destination_img) pDestroyImage(dev, destination_img, NULL);
   if (pDestroyBuffer && input_buf) pDestroyBuffer(dev, input_buf, NULL);
   if (pDestroyBuffer && output_buf) pDestroyBuffer(dev, output_buf, NULL);
   if (pDestroyBuffer && staging) pDestroyBuffer(dev, staging, NULL);
   if (pDestroyBuffer && readback) pDestroyBuffer(dev, readback, NULL);
   if (pFreeMemory && input_mem) pFreeMemory(dev, input_mem, NULL);
   if (pFreeMemory && output_mem) pFreeMemory(dev, output_mem, NULL);
   if (pFreeMemory && staging_mem) pFreeMemory(dev, staging_mem, NULL);
   if (pFreeMemory && readback_mem) pFreeMemory(dev, readback_mem, NULL);
   if (pFreeMemory && source_mem) pFreeMemory(dev, source_mem, NULL);
   if (pFreeMemory && destination_mem) pFreeMemory(dev, destination_mem, NULL);
   if (pDestroyDevice && dev) pDestroyDevice(dev, NULL);
   if (pDestroyInstance && inst) pDestroyInstance(inst, NULL);
   if (inst) LOG("cleanup: Vulkan device and instance destroyed");
   LOG("=== done; log at sdmc:/nvk_compute.log ===");
   if (g_log) fclose(g_log);
   return failures ? 1 : (r != VK_SUCCESS ? 1 : 0);
}
