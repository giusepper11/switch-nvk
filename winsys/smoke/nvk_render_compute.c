/*
 * nvk_render_compute.c — FG-2 deterministic graphics-to-compute image chain.
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

#define IMAGE_W 16u
#define IMAGE_H 16u
#define ELEMENTS (IMAGE_W * IMAGE_H)
#define IMAGE_BYTES (ELEMENTS * 4u)
#define ITERATIONS 64u

#ifndef FG2_ROOT_DIAG_LIMIT
#define FG2_ROOT_DIAG_LIMIT 0u
#endif

#ifndef FG2_ROOT_UPLOAD_CACHE_FLUSH
#define FG2_ROOT_UPLOAD_CACHE_FLUSH 0u
#endif

#ifndef FG2_QMD_UPLOAD_IDENTITY
#define FG2_QMD_UPLOAD_IDENTITY 0u
#endif

#ifndef FG2_QMD_UPLOAD_CACHE_FLUSH
#define FG2_QMD_UPLOAD_CACHE_FLUSH 0u
#endif

#ifndef FG2_QMD_ADDRESS_CONTROL
#define FG2_QMD_ADDRESS_CONTROL 0u
#endif

#ifndef FG2_QMD_ADDRESS_FRESH
#define FG2_QMD_ADDRESS_FRESH 0u
#endif

#ifndef FG2_ROOT_ADDRESS_CONTROL
#define FG2_ROOT_ADDRESS_CONTROL 0u
#endif

#ifndef FG2_ROOT_ADDRESS_FRESH
#define FG2_ROOT_ADDRESS_FRESH 0u
#endif

#ifndef FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL
#define FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL 0u
#endif

#ifndef FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE
#define FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE 0u
#endif

#if FG2_ROOT_UPLOAD_CACHE_FLUSH != 0u && FG2_ROOT_UPLOAD_CACHE_FLUSH != 1u
#error "FG2_ROOT_UPLOAD_CACHE_FLUSH must be 0 or 1"
#endif

#if FG2_QMD_UPLOAD_IDENTITY != 0u && FG2_QMD_UPLOAD_IDENTITY != 1u
#error "FG2_QMD_UPLOAD_IDENTITY must be 0 or 1"
#endif

#if FG2_QMD_UPLOAD_CACHE_FLUSH != 0u && FG2_QMD_UPLOAD_CACHE_FLUSH != 1u
#error "FG2_QMD_UPLOAD_CACHE_FLUSH must be 0 or 1"
#endif

#if FG2_QMD_UPLOAD_CACHE_FLUSH == 1u && FG2_QMD_UPLOAD_IDENTITY != 1u
#error "FG2_QMD_UPLOAD_CACHE_FLUSH requires FG2_QMD_UPLOAD_IDENTITY=1"
#endif

#if FG2_QMD_ADDRESS_CONTROL != 0u && FG2_QMD_ADDRESS_CONTROL != 1u
#error "FG2_QMD_ADDRESS_CONTROL must be 0 or 1"
#endif

#if FG2_QMD_ADDRESS_FRESH != 0u && FG2_QMD_ADDRESS_FRESH != 1u
#error "FG2_QMD_ADDRESS_FRESH must be 0 or 1"
#endif

#if FG2_QMD_ADDRESS_CONTROL == 1u && FG2_QMD_ADDRESS_FRESH == 1u
#error "FG2 QMD address selectors are mutually exclusive"
#endif

#if (FG2_QMD_ADDRESS_CONTROL == 1u || FG2_QMD_ADDRESS_FRESH == 1u) && \
    (FG2_ROOT_UPLOAD_CACHE_FLUSH == 1u || FG2_QMD_UPLOAD_CACHE_FLUSH == 1u)
#error "FG2 QMD address experiment requires cache selectors disabled"
#endif

#if FG2_ROOT_ADDRESS_CONTROL != 0u && FG2_ROOT_ADDRESS_CONTROL != 1u
#error "FG2_ROOT_ADDRESS_CONTROL must be 0 or 1"
#endif
#if FG2_ROOT_ADDRESS_FRESH != 0u && FG2_ROOT_ADDRESS_FRESH != 1u
#error "FG2_ROOT_ADDRESS_FRESH must be 0 or 1"
#endif
#if FG2_ROOT_ADDRESS_CONTROL == 1u && FG2_ROOT_ADDRESS_FRESH == 1u
#error "FG2 root address selectors are mutually exclusive"
#endif
#if (FG2_ROOT_ADDRESS_CONTROL == 1u || FG2_ROOT_ADDRESS_FRESH == 1u) && \
    (FG2_ROOT_UPLOAD_CACHE_FLUSH == 1u || FG2_QMD_UPLOAD_IDENTITY == 1u || \
     FG2_QMD_UPLOAD_CACHE_FLUSH == 1u || FG2_QMD_ADDRESS_CONTROL == 1u || \
     FG2_QMD_ADDRESS_FRESH == 1u)
#error "FG2 root address experiment excludes root-cache and QMD experiments"
#endif

#if FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL != 0u && \
    FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL != 1u
#error "FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL must be 0 or 1"
#endif
#if FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE != 0u && \
    FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE != 1u
#error "FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE must be 0 or 1"
#endif
#if FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u && \
    FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u
#error "FG2 QMD shader constant-cache selectors are mutually exclusive"
#endif
#if (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u || \
     FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u) && \
    (FG2_ROOT_UPLOAD_CACHE_FLUSH == 1u || FG2_QMD_UPLOAD_IDENTITY == 1u || \
     FG2_QMD_UPLOAD_CACHE_FLUSH == 1u || FG2_QMD_ADDRESS_CONTROL == 1u || \
     FG2_QMD_ADDRESS_FRESH == 1u || FG2_ROOT_ADDRESS_CONTROL == 1u || \
     FG2_ROOT_ADDRESS_FRESH == 1u)
#error "FG2 QMD shader constant-cache experiment excludes historical interventions"
#endif

#if (FG2_ROOT_ADDRESS_CONTROL == 1u || FG2_ROOT_ADDRESS_FRESH == 1u || \
     FG2_QMD_ADDRESS_CONTROL == 1u || FG2_QMD_ADDRESS_FRESH == 1u || \
     FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u || \
     FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u) && \
    FG2_ROOT_DIAG_LIMIT == 0u
#define FG2_EFFECTIVE_DIAG_LIMIT 64u
#elif FG2_QMD_UPLOAD_IDENTITY == 1u && FG2_ROOT_DIAG_LIMIT == 0u
#define FG2_EFFECTIVE_DIAG_LIMIT 2u
#else
#define FG2_EFFECTIVE_DIAG_LIMIT FG2_ROOT_DIAG_LIMIT
#endif

#if FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u
#define FG2_BUILD_TAG "chain2-qmd-constant-cache-invalidate"
#elif FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u
#define FG2_BUILD_TAG "chain2-qmd-constant-cache-control"
#elif FG2_ROOT_ADDRESS_FRESH == 1u
#define FG2_BUILD_TAG "chain2-root-address-fresh"
#elif FG2_ROOT_ADDRESS_CONTROL == 1u
#define FG2_BUILD_TAG "chain2-root-address-control"
#elif FG2_QMD_ADDRESS_FRESH == 1u
#define FG2_BUILD_TAG "chain2-qmd-address-fresh"
#elif FG2_QMD_ADDRESS_CONTROL == 1u
#define FG2_BUILD_TAG "chain2-qmd-address-control"
#elif FG2_QMD_UPLOAD_CACHE_FLUSH == 1u
#define FG2_BUILD_TAG "chain2-qmdcache1"
#elif FG2_QMD_UPLOAD_IDENTITY == 1u
#define FG2_BUILD_TAG "chain2-qmdidentity1-control"
#elif FG2_ROOT_UPLOAD_CACHE_FLUSH == 1u
#define FG2_BUILD_TAG "chain2-rootflush1"
#elif FG2_ROOT_DIAG_LIMIT > 0u
#define FG2_BUILD_TAG "chain2-rootdiag1-control"
#else
#define FG2_BUILD_TAG "chain2"
#endif

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

static uint32_t iteration_seed(uint32_t iteration)
{
   return (iteration * 37u + 5u) & 255u;
}

static uint32_t image_a_pixel(uint32_t x, uint32_t y, uint32_t seed)
{
   uint32_t r = (x * 17u + seed * 3u + 11u) & 255u;
   uint32_t g = (y * 29u + seed * 5u + 19u) & 255u;
   uint32_t b = ((x ^ y) * 13u + seed * 7u + 23u) & 255u;
   return r | (g << 8) | (b << 16) | 0xff000000u;
}

static uint32_t expected_image_pixel(uint32_t x, uint32_t y, uint32_t seed)
{
   uint32_t a = image_a_pixel(x, y, seed);
   uint32_t r = ((a >> 16) & 255u) ^ seed;
   uint32_t g = 255u - ((a >> 8) & 255u);
   uint32_t b = ((a & 255u) + seed * 9u) & 255u;
   uint32_t alpha = 255u ^ (seed & 15u);
   return r | (g << 8) | (b << 16) | (alpha << 24);
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

static uint32_t expected_image_checksum(uint32_t seed)
{
   uint32_t values[ELEMENTS];
   for (uint32_t y = 0; y < IMAGE_H; y++)
      for (uint32_t x = 0; x < IMAGE_W; x++)
         values[y * IMAGE_W + x] = expected_image_pixel(x, y, seed);
   return checksum_words(values, ELEMENTS);
}

static uint32_t observed_behavior_seed(const uint32_t *values)
{
   uint32_t found = UINT32_MAX;
   for (uint32_t seed = 0; seed <= 255u; seed++) {
      bool match = true;
      for (uint32_t i = 0; i < ELEMENTS; i++) {
         if (values[i] != expected_image_pixel(i % IMAGE_W, i / IMAGE_W, seed)) {
            match = false;
            break;
         }
      }
      if (match) {
         if (found != UINT32_MAX)
            return UINT32_MAX;
         found = seed;
      }
   }
   return found;
}

int main(void)
{
   VkInstance inst = VK_NULL_HANDLE;
   VkDevice dev = VK_NULL_HANDLE;
   VkQueue queue = VK_NULL_HANDLE;
   VkCommandPool pool = VK_NULL_HANDLE;
   VkCommandBuffer cmd = VK_NULL_HANDLE;
   VkBuffer readback = VK_NULL_HANDLE;
   VkDeviceMemory readback_mem = VK_NULL_HANDLE;
   VkImage source_img = VK_NULL_HANDLE, destination_img = VK_NULL_HANDLE;
   VkDeviceMemory source_mem = VK_NULL_HANDLE, destination_mem = VK_NULL_HANDLE;
   VkImageView source_view = VK_NULL_HANDLE, destination_view = VK_NULL_HANDLE;
   VkSampler sampler = VK_NULL_HANDLE;
   VkShaderModule vert_shader = VK_NULL_HANDLE, frag_shader = VK_NULL_HANDLE;
   VkShaderModule shader = VK_NULL_HANDLE;
   VkRenderPass render_pass = VK_NULL_HANDLE;
   VkFramebuffer framebuffer = VK_NULL_HANDLE;
   VkPipeline graphics_pipeline = VK_NULL_HANDLE;
   VkDescriptorSetLayout descriptor_layout = VK_NULL_HANDLE;
   VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
   VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
   VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
   VkPipeline pipeline = VK_NULL_HANDLE;
   void *readback_cpu = NULL;
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
   PFN_vkDestroyRenderPass pDestroyRenderPass = NULL;
   PFN_vkDestroyFramebuffer pDestroyFramebuffer = NULL;
   PFN_vkDestroyCommandPool pDestroyCommandPool = NULL;

   g_log = fopen("sdmc:/nvk_render_compute.log", "w");
   if (__nxlink_host.s_addr != 0 && R_SUCCEEDED(socketInitializeDefault()))
      nxlinkStdio();
   LOG("=== NVK FG-2 render-compute image chain [BUILD %s] ===", FG2_BUILD_TAG);
   LOG("contract: graphics draw -> image A -> sampled compute -> image B -> readback; %ux%u RGBA8, %u iterations", 
       IMAGE_W, IMAGE_H, ITERATIONS);
   LOG("sentinels: image A=0x5a17c3e1 image B=0xa6d42b7f; seeds=(iteration*37+5)&255");
   LOG("FG2_ROOT_CACHE experiment=%s selector=%u path=%s",
       FG2_ROOT_UPLOAD_CACHE_FLUSH ? "reused_compute_root_cpu_flush" : "disabled_control",
       FG2_ROOT_UPLOAD_CACHE_FLUSH,
       FG2_ROOT_UPLOAD_CACHE_FLUSH ? "eligible_reused_root_only" : "ordinary_no_root_flush");
   LOG("FG2_QMD experiment=%s identity_selector=%u cache_selector=%u path=%s",
       FG2_QMD_UPLOAD_CACHE_FLUSH ? "eligible_reused_qmd_cpu_flush" :
       (FG2_QMD_UPLOAD_IDENTITY ? "qmd_identity_control" : "disabled"),
       FG2_QMD_UPLOAD_IDENTITY, FG2_QMD_UPLOAD_CACHE_FLUSH,
       FG2_QMD_UPLOAD_CACHE_FLUSH ? "eligible_reused_qmd_only" :
       (FG2_QMD_UPLOAD_IDENTITY ? "identity_only_no_qmd_flush" : "ordinary"));
   LOG("FG2_QMD_ADDRESS experiment=%s control_selector=%u fresh_selector=%u path=%s transitions=63 addresses_may_recur_after_one_dispatch=1",
       FG2_QMD_ADDRESS_FRESH ? "fresh_address_variant" :
       (FG2_QMD_ADDRESS_CONTROL ? "same_source_control" : "disabled"),
       FG2_QMD_ADDRESS_CONTROL, FG2_QMD_ADDRESS_FRESH,
       FG2_QMD_ADDRESS_FRESH ? "alternating_primary_secondary" :
       (FG2_QMD_ADDRESS_CONTROL ? "primary_only" : "ordinary_one_slot"));
   LOG("FG2_ROOT_ADDRESS experiment=%s control_selector=%u fresh_selector=%u path=%s transitions=63 qmd_transitions=63 addresses_may_recur_after_one_dispatch=1",
       FG2_ROOT_ADDRESS_FRESH ? "fresh_address_variant" :
       (FG2_ROOT_ADDRESS_CONTROL ? "same_source_control" : "disabled"),
       FG2_ROOT_ADDRESS_CONTROL, FG2_ROOT_ADDRESS_FRESH,
       FG2_ROOT_ADDRESS_FRESH ? "alternating_primary_alternate" :
       (FG2_ROOT_ADDRESS_CONTROL ? "primary_only" : "ordinary"));
   LOG("FG2_QMD_CONSTANT_CACHE experiment=%s control_selector=%u invalidate_selector=%u path=%s expected_bit=%u root_transitions=63 qmd_transitions=63",
       FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE ? "invalidate_variant" :
       (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL ? "same_source_control" : "disabled"),
       FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL,
       FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE,
       (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL ||
        FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE) ?
          "alternating_root_and_qmd" : "ordinary",
       FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE);

   g_drm_shim_log_sink = shim_log_sink;
   setenv("NVK_I_WANT_A_BROKEN_VULKAN_DRIVER", "1", 1);
   setenv("MESA_SHADER_CACHE_DISABLE", "1", 1);
   setenv("MESA_LOG_FILE", "sdmc:/nvk_render_compute_mesa.log", 1);
   if (FG2_EFFECTIVE_DIAG_LIMIT > 0u) {
      char trace_limit[4];
      snprintf(trace_limit, sizeof(trace_limit), "%u", FG2_EFFECTIVE_DIAG_LIMIT);
      setenv("NVK_ROOT_TRACE", trace_limit, 1);
   }
   if (FG2_ROOT_UPLOAD_CACHE_FLUSH == 1u)
      setenv("NVK_ROOT_UPLOAD_CACHE_FLUSH", "1", 1);
   if (FG2_ROOT_ADDRESS_CONTROL == 1u)
      setenv("NVK_ROOT_ADDRESS_CONTROL", "1", 1);
   if (FG2_ROOT_ADDRESS_FRESH == 1u)
      setenv("NVK_ROOT_ADDRESS_FRESH", "1", 1);
   if (FG2_QMD_UPLOAD_IDENTITY == 1u)
      setenv("NVK_QMD_UPLOAD_IDENTITY", "1", 1);
   if (FG2_QMD_UPLOAD_CACHE_FLUSH == 1u)
      setenv("NVK_QMD_UPLOAD_CACHE_FLUSH", "1", 1);
   if (FG2_QMD_ADDRESS_CONTROL == 1u)
      setenv("NVK_QMD_ADDRESS_CONTROL", "1", 1);
   if (FG2_QMD_ADDRESS_FRESH == 1u)
      setenv("NVK_QMD_ADDRESS_FRESH", "1", 1);
   if (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u)
      setenv("NVK_QMD_SHADER_CONSTANT_CACHE_CONTROL", "1", 1);
   if (FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u)
      setenv("NVK_QMD_SHADER_CONSTANT_CACHE_INVALIDATE", "1", 1);

   PFN_vkCreateInstance pCreateInstance =
      (PFN_vkCreateInstance)vk_icdGetInstanceProcAddr(NULL, "vkCreateInstance");
   if (!pCreateInstance) { LOG("FAIL setup: vkCreateInstance entrypoint missing"); goto done; }
   VkApplicationInfo app = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "nvk_render_compute", .apiVersion = VK_API_VERSION_1_1 };
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
      if (qfi == UINT32_MAX &&
          (qf[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) ==
          (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) qfi = i;
   }
   if (qfi == UINT32_MAX) { LOG("FAIL setup: no graphics+compute queue family"); goto done; }
   LOG("C selected graphics+compute queue family %u", qfi);

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
   PFN_vkCreateRenderPass pCreateRenderPass = GD(vkCreateRenderPass);
   PFN_vkCreateFramebuffer pCreateFramebuffer = GD(vkCreateFramebuffer);
   PFN_vkCreateGraphicsPipelines pCreateGraphicsPipelines = GD(vkCreateGraphicsPipelines);
   PFN_vkCreateCommandPool pCreateCommandPool = GD(vkCreateCommandPool);
   PFN_vkAllocateCommandBuffers pAllocateCommandBuffers = GD(vkAllocateCommandBuffers);
   PFN_vkBeginCommandBuffer pBeginCommandBuffer = GD(vkBeginCommandBuffer);
   PFN_vkResetCommandBuffer pResetCommandBuffer = GD(vkResetCommandBuffer);
   PFN_vkCmdPipelineBarrier pCmdPipelineBarrier = GD(vkCmdPipelineBarrier);
   PFN_vkCmdCopyBufferToImage pCmdCopyBufferToImage = GD(vkCmdCopyBufferToImage);
   PFN_vkCmdClearColorImage pCmdClearColorImage = GD(vkCmdClearColorImage);
   PFN_vkCmdCopyImageToBuffer pCmdCopyImageToBuffer = GD(vkCmdCopyImageToBuffer);
   PFN_vkCmdCopyBuffer pCmdCopyBuffer = GD(vkCmdCopyBuffer);
   PFN_vkCmdBindPipeline pCmdBindPipeline = GD(vkCmdBindPipeline);
   PFN_vkCmdBindDescriptorSets pCmdBindDescriptorSets = GD(vkCmdBindDescriptorSets);
   PFN_vkCmdDispatch pCmdDispatch = GD(vkCmdDispatch);
   PFN_vkCmdBeginRenderPass pCmdBeginRenderPass = GD(vkCmdBeginRenderPass);
   PFN_vkCmdEndRenderPass pCmdEndRenderPass = GD(vkCmdEndRenderPass);
   PFN_vkCmdDraw pCmdDraw = GD(vkCmdDraw);
   PFN_vkCmdPushConstants pCmdPushConstants = GD(vkCmdPushConstants);
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
   pDestroyRenderPass = GD(vkDestroyRenderPass);
   pDestroyFramebuffer = GD(vkDestroyFramebuffer);
   pDestroyCommandPool = GD(vkDestroyCommandPool);
   if (!pGetQueue || !pAlloc || !pMap || !pCreateBuffer || !pBufferRequirements ||
       !pBindBuffer || !pCreateImage || !pImageRequirements || !pBindImage ||
       !pCreateImageView || !pCreateSampler || !pCreateShaderModule ||
       !pCreateDescriptorSetLayout || !pCreateDescriptorPool ||
       !pAllocateDescriptorSets || !pUpdateDescriptorSets || !pCreatePipelineLayout ||
       !pCreateComputePipelines || !pCreateRenderPass || !pCreateFramebuffer ||
       !pCreateGraphicsPipelines || !pCreateCommandPool || !pAllocateCommandBuffers ||
       !pResetCommandBuffer ||
       !pBeginCommandBuffer || !pCmdPipelineBarrier || !pCmdCopyBufferToImage ||
       !pCmdClearColorImage ||
       !pCmdCopyImageToBuffer || !pCmdCopyBuffer || !pCmdBindPipeline ||
       !pCmdBindDescriptorSets || !pCmdDispatch || !pCmdBeginRenderPass ||
       !pCmdEndRenderPass || !pCmdDraw || !pCmdPushConstants || !pEndCommandBuffer ||
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

   MAKE_HOST_BUFFER(readback, readback_mem, readback_cpu, IMAGE_BYTES, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
   memset(readback_cpu, 0, IMAGE_BYTES);
   armDCacheFlush(readback_cpu, IMAGE_BYTES);
   LOG("E host-visible readback initialized; CPU image processing disabled");

   MAKE_DEVICE_IMAGE(source_img, source_mem, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT);
   MAKE_DEVICE_IMAGE(destination_img, destination_mem, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
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
   LOG("F distinct test-owned image A=%p and image B=%p allocated (RGBA8, %ux%u)",
       (void *)source_img, (void *)destination_img, IMAGE_W, IMAGE_H);

   VkShaderModuleCreateInfo smci = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = render_compute_comp_spv_sz, .pCode = render_compute_comp_spv };
   r = pCreateShaderModule(dev, &smci, NULL, &shader);
   LOG("G NAK compute shader module (%u bytes) -> %d", render_compute_comp_spv_sz, r);
   if (r != VK_SUCCESS) goto done;
   VkDescriptorSetLayoutBinding bindings[2] = {
      { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL },
      { 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL },
   };
   VkDescriptorSetLayoutCreateInfo dslci = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 2, .pBindings = bindings };
   r = pCreateDescriptorSetLayout(dev, &dslci, NULL, &descriptor_layout);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: descriptor layout -> %d", r); goto done; }
   VkDescriptorPoolSize pool_sizes[2] = {
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
   };
   VkDescriptorPoolCreateInfo dpci = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1, .poolSizeCount = 2, .pPoolSizes = pool_sizes };
   r = pCreateDescriptorPool(dev, &dpci, NULL, &descriptor_pool);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: descriptor pool -> %d", r); goto done; }
   VkDescriptorSetAllocateInfo dsai = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool, .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_layout };
   r = pAllocateDescriptorSets(dev, &dsai, &descriptor_set);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: descriptor set -> %d", r); goto done; }
   VkDescriptorImageInfo sampled_di = { sampler, source_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
   VkDescriptorImageInfo storage_di = { VK_NULL_HANDLE, destination_view, VK_IMAGE_LAYOUT_GENERAL };
   VkWriteDescriptorSet writes[2] = {
      { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptor_set, .dstBinding = 0,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = &sampled_di },
      { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptor_set, .dstBinding = 1,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .pImageInfo = &storage_di },
   };
   pUpdateDescriptorSets(dev, 2, writes, 0, NULL);
   VkPushConstantRange shared_push = {
      VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
      0,
      sizeof(uint32_t),
   };
   VkPipelineLayoutCreateInfo plci = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1, .pSetLayouts = &descriptor_layout,
      .pushConstantRangeCount = 1, .pPushConstantRanges = &shared_push };
   r = pCreatePipelineLayout(dev, &plci, NULL, &pipeline_layout);
   if (r != VK_SUCCESS) { LOG("FAIL pipeline: shared layout -> %d", r); goto done; }
   VkPipelineShaderStageCreateInfo stage = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT, .module = shader, .pName = "main" };
   VkComputePipelineCreateInfo cpci = { .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = stage, .layout = pipeline_layout };
   r = pCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &cpci, NULL, &pipeline);
   LOG("G vkCreateComputePipelines -> %d", r);
   if (r != VK_SUCCESS) goto done;
   LOG("G intended path: sampled image A binding 0 + storage image B binding 1; nearest/clamp sampler");

   VkAttachmentDescription attachment = { .format = VK_FORMAT_R8G8B8A8_UNORM,
      .samples = VK_SAMPLE_COUNT_1_BIT, .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
   VkAttachmentReference color_ref = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
   VkSubpassDescription subpass = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1, .pColorAttachments = &color_ref };
   VkRenderPassCreateInfo rpci = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1, .pAttachments = &attachment,
      .subpassCount = 1, .pSubpasses = &subpass };
   r = pCreateRenderPass(dev, &rpci, NULL, &render_pass);
   if (r != VK_SUCCESS) { LOG("FAIL graphics: render pass -> %d", r); goto done; }
   VkFramebufferCreateInfo fbci = { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = render_pass, .attachmentCount = 1, .pAttachments = &source_view,
      .width = IMAGE_W, .height = IMAGE_H, .layers = 1 };
   r = pCreateFramebuffer(dev, &fbci, NULL, &framebuffer);
   if (r != VK_SUCCESS) { LOG("FAIL graphics: framebuffer -> %d", r); goto done; }
   VkShaderModuleCreateInfo gfx_smci = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = render_compute_vert_spv_sz, .pCode = render_compute_vert_spv };
   r = pCreateShaderModule(dev, &gfx_smci, NULL, &vert_shader);
   if (r != VK_SUCCESS) { LOG("FAIL graphics: vertex shader -> %d", r); goto done; }
   gfx_smci.codeSize = render_compute_frag_spv_sz;
   gfx_smci.pCode = render_compute_frag_spv;
   r = pCreateShaderModule(dev, &gfx_smci, NULL, &frag_shader);
   if (r != VK_SUCCESS) { LOG("FAIL graphics: fragment shader -> %d", r); goto done; }
   VkPipelineShaderStageCreateInfo gfx_stages[2] = {
      { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vert_shader, .pName = "main" },
      { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = frag_shader, .pName = "main" },
   };
   VkPipelineVertexInputStateCreateInfo vi = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
   VkPipelineInputAssemblyStateCreateInfo ia = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
   VkViewport viewport = { 0, 0, (float)IMAGE_W, (float)IMAGE_H, 0, 1 };
   VkRect2D scissor = { { 0, 0 }, { IMAGE_W, IMAGE_H } };
   VkPipelineViewportStateCreateInfo vp = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor };
   VkPipelineRasterizationStateCreateInfo rs = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE, .lineWidth = 1.0f };
   VkPipelineMultisampleStateCreateInfo ms = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT };
   VkPipelineColorBlendAttachmentState blend_attachment = { .colorWriteMask = 0xf };
   VkPipelineColorBlendStateCreateInfo blend = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1, .pAttachments = &blend_attachment };
   VkGraphicsPipelineCreateInfo gpci = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2, .pStages = gfx_stages, .pVertexInputState = &vi,
      .pInputAssemblyState = &ia, .pViewportState = &vp, .pRasterizationState = &rs,
      .pMultisampleState = &ms, .pColorBlendState = &blend,
      .layout = pipeline_layout, .renderPass = render_pass };
   r = pCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &gpci, NULL, &graphics_pipeline);
   if (r != VK_SUCCESS) { LOG("FAIL graphics: pipeline -> %d", r); goto done; }
   LOG("H graphics pipeline ready: oversized full-coverage triangle; shared graphics+compute layout; fragment draw (not clear-only)");

   VkCommandPoolCreateInfo cpci_pool = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = qfi };
   r = pCreateCommandPool(dev, &cpci_pool, NULL, &pool);
   if (r != VK_SUCCESS) { LOG("FAIL submit: command pool -> %d", r); goto done; }
   VkCommandBufferAllocateInfo cbai = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
   r = pAllocateCommandBuffers(dev, &cbai, &cmd);
   if (r != VK_SUCCESS) { LOG("FAIL submit: command buffer -> %d", r); goto done; }

   for (uint32_t iteration = 0; iteration < ITERATIONS; iteration++) {
      memset(readback_cpu, 0, IMAGE_BYTES);
      armDCacheFlush(readback_cpu, IMAGE_BYTES);
      uint32_t seed = iteration_seed(iteration);
      if (iteration > 0) {
         r = pResetCommandBuffer(cmd, 0);
         if (r != VK_SUCCESS) { LOG("FAIL iteration %u: reset -> %d", iteration, r); goto done; }
      }
      r = pBeginCommandBuffer(cmd, &(VkCommandBufferBeginInfo){
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT });
      if (r != VK_SUCCESS) { LOG("FAIL iteration %u: begin -> %d", iteration, r); goto done; }

      VkImageMemoryBarrier source_to_clear = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .image = source_img, .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, NULL, 0, NULL, 1, &source_to_clear);
      VkClearColorValue sentinel_a = { .float32 = {
         0x5a / 255.0f, 0x17 / 255.0f, 0xc3 / 255.0f, 0xe1 / 255.0f } };
      VkImageSubresourceRange color_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
      pCmdClearColorImage(cmd, source_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          &sentinel_a, 1, &color_range);
      VkImageMemoryBarrier source_to_color = source_to_clear;
      source_to_color.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      source_to_color.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      source_to_color.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      source_to_color.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          0, 0, NULL, 0, NULL, 1, &source_to_color);
      VkRenderPassBeginInfo rp_begin = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass = render_pass, .framebuffer = framebuffer,
         .renderArea = { { 0, 0 }, { IMAGE_W, IMAGE_H } } };
      pCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
      pCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
      if (iteration < FG2_EFFECTIVE_DIAG_LIMIT)
         LOG("FG2_ROOT_DIAG phase=marker record=%u iteration=%u expected_seed=%u cmd=%p",
             iteration + 1u, iteration + 1u, seed, (void *)cmd);
      pCmdPushConstants(cmd, pipeline_layout,
                        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
                        0, sizeof(seed), &seed);
      pCmdDraw(cmd, 3, 1, 0, 0);
      pCmdEndRenderPass(cmd);
      VkImageMemoryBarrier source_to_sample = source_to_color;
      source_to_sample.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      source_to_sample.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      source_to_sample.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      source_to_sample.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          0, 0, NULL, 0, NULL, 1, &source_to_sample);
      VkImageMemoryBarrier destination_to_clear = source_to_clear;
      destination_to_clear.image = destination_img;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, NULL, 0, NULL, 1, &destination_to_clear);
      VkClearColorValue sentinel_b = { .float32 = {
         0xa6 / 255.0f, 0xd4 / 255.0f, 0x2b / 255.0f, 0x7f / 255.0f } };
      pCmdClearColorImage(cmd, destination_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          &sentinel_b, 1, &color_range);
      VkImageMemoryBarrier destination_to_compute = destination_to_clear;
      destination_to_compute.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      destination_to_compute.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      destination_to_compute.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      destination_to_compute.newLayout = VK_IMAGE_LAYOUT_GENERAL;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          0, 0, NULL, 0, NULL, 1, &destination_to_compute);
      pCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
      pCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                             &descriptor_set, 0, NULL);
      pCmdDispatch(cmd, IMAGE_W / 8u, IMAGE_H / 8u, 1);
      VkImageMemoryBarrier destination_to_copy = destination_to_compute;
      destination_to_copy.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      destination_to_copy.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      destination_to_copy.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
      destination_to_copy.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      pCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, NULL, 0, NULL, 1, &destination_to_copy);
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
      armDCacheFlush(readback_cpu, IMAGE_BYTES);

      if (iteration < FG2_EFFECTIVE_DIAG_LIMIT) {
         const uint32_t observed_checksum =
            checksum_words(readback_cpu, ELEMENTS);
         const uint32_t behavior_seed =
            observed_behavior_seed((const uint32_t *)readback_cpu);
         if (behavior_seed == UINT32_MAX) {
            LOG("FG2_ROOT_DIAG phase=result record=%u iteration=%u expected_seed=%u cmd=%p observed_pixel0=0x%08x expected_pixel0=0x%08x observed_checksum=0x%08x expected_checksum=0x%08x observed_behavior_seed=UNKNOWN",
                iteration + 1u, iteration + 1u, seed, (void *)cmd,
                ((uint32_t *)readback_cpu)[0], expected_image_pixel(0, 0, seed),
                observed_checksum, expected_image_checksum(seed));
         } else {
            LOG("FG2_ROOT_DIAG phase=result record=%u iteration=%u expected_seed=%u cmd=%p observed_pixel0=0x%08x expected_pixel0=0x%08x observed_checksum=0x%08x expected_checksum=0x%08x observed_behavior_seed=%u",
                iteration + 1u, iteration + 1u, seed, (void *)cmd,
                ((uint32_t *)readback_cpu)[0], expected_image_pixel(0, 0, seed),
                observed_checksum, expected_image_checksum(seed), behavior_seed);
         }
      }

      uint32_t bad_images = 0, first_bad = UINT32_MAX;
      for (uint32_t i = 0; i < ELEMENTS; i++) {
         uint32_t x = i % IMAGE_W, y = i / IMAGE_W;
         if (((uint32_t *)readback_cpu)[i] != expected_image_pixel(x, y, seed)) {
            if (first_bad == UINT32_MAX) first_bad = i;
            bad_images++;
         }
      }
      if (FG2_QMD_ADDRESS_CONTROL == 1u || FG2_QMD_ADDRESS_FRESH == 1u) {
         const uint32_t observed_checksum = checksum_words(readback_cpu, ELEMENTS);
         LOG("FG2_QMD_ADDRESS phase=result path=%s record=%u iteration=%u seed=%u pixel=0x%08x checksum=0x%08x expected_pixel=0x%08x expected_checksum=0x%08x oracle_match=%u mismatches=%u fault_state=INSPECT_COMPLETE_STREAM",
             FG2_QMD_ADDRESS_FRESH ? "fresh" : "control",
             iteration + 1u, iteration + 1u, seed,
             ((uint32_t *)readback_cpu)[0], observed_checksum,
             expected_image_pixel(0, 0, seed), expected_image_checksum(seed),
             bad_images == 0, bad_images);
      }
      if (FG2_ROOT_ADDRESS_CONTROL == 1u || FG2_ROOT_ADDRESS_FRESH == 1u) {
         const uint32_t observed_checksum = checksum_words(readback_cpu, ELEMENTS);
         LOG("FG2_ROOT_ADDRESS phase=result path=%s record=%u iteration=%u seed=%u pixel=0x%08x checksum=0x%08x expected_pixel=0x%08x expected_checksum=0x%08x oracle_match=%u mismatches=%u fault_state=INSPECT_COMPLETE_STREAM",
             FG2_ROOT_ADDRESS_FRESH ? "fresh" : "control",
             iteration + 1u, iteration + 1u, seed,
             ((uint32_t *)readback_cpu)[0], observed_checksum,
             expected_image_pixel(0, 0, seed), expected_image_checksum(seed),
             bad_images == 0, bad_images);
      }
      if (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u ||
          FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u) {
         const uint32_t observed_checksum = checksum_words(readback_cpu, ELEMENTS);
         LOG("FG2_QMD_CONSTANT_CACHE phase=result path=%s record=%u iteration=%u seed=%u pixel=0x%08x checksum=0x%08x expected_pixel=0x%08x expected_checksum=0x%08x oracle_match=%u mismatches=%u fault_state=INSPECT_COMPLETE_STREAM",
             FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE ? "invalidate" : "control",
             iteration + 1u, iteration + 1u, seed,
             ((uint32_t *)readback_cpu)[0], observed_checksum,
             expected_image_pixel(0, 0, seed), expected_image_checksum(seed),
             bad_images == 0, bad_images);
      }
      if (bad_images) {
         failures++;
         uint32_t x = first_bad % IMAGE_W, y = first_bad / IMAGE_W;
         LOG("FAIL iteration %u seed=%u: first mismatch (%u,%u) observed=0x%08x expected=0x%08x; mismatches=%u observed_checksum=0x%08x expected_checksum=0x%08x",
             iteration, seed, x, y, ((uint32_t *)readback_cpu)[first_bad],
             expected_image_pixel(x, y, seed), bad_images,
             checksum_words(readback_cpu, ELEMENTS), expected_image_checksum(seed));
      } else if (iteration == 0 || iteration + 1 == ITERATIONS) {
         LOG("iteration %u/%u seed=%u EXACT OK: pixel[0]=0x%08x checksum observed=0x%08x expected=0x%08x",
             iteration + 1, ITERATIONS, seed, ((uint32_t *)readback_cpu)[0],
             checksum_words(readback_cpu, ELEMENTS), expected_image_checksum(seed));
      }
   }
   if (FG2_QMD_ADDRESS_CONTROL == 1u || FG2_QMD_ADDRESS_FRESH == 1u)
      LOG("FG2_QMD_ADDRESS phase=oracle_aggregate path=%s validations=%u/64 mismatched_iterations=%u fault_state=INSPECT_COMPLETE_STREAM",
          FG2_QMD_ADDRESS_FRESH ? "fresh" : "control",
          ITERATIONS - failures, failures);
   if (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u ||
       FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u)
      LOG("FG2_QMD_CONSTANT_CACHE phase=oracle_aggregate path=%s validations=%u/64 mismatched_iterations=%u fault_state=INSPECT_COMPLETE_STREAM",
          FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE ? "invalidate" : "control",
          ITERATIONS - failures, failures);
   if (failures == 0) {
      uint32_t final_seed = iteration_seed(ITERATIONS - 1u);
      LOG("RESULT PASS: %u/%u iterations exact; final checksum observed=0x%08x expected=0x%08x",
          ITERATIONS, ITERATIONS, checksum_words(readback_cpu, ELEMENTS),
          expected_image_checksum(final_seed));
      LOG("INTENDED GPU PATH EXECUTED: fragment-shader draw -> image A -> explicit A color-write/sample-read transition -> nearest sampled read -> compute storage write image B -> explicit B storage-write/transfer-read transition -> host-visible readback");
      LOG("FALLBACK/BYPASS: none; images distinct; no CPU image processing, direct copy, constant-output path, clear-only graphics, or skipped stage");
      LOG("GPU FAULT/ERROR NOTIFIER: inspect complete device log and Mesa log; no local fallback is accepted");
   } else {
      LOG("RESULT FAIL: %u/%u iterations mismatched; capability remains unproven", failures, ITERATIONS);
   }

done:
   if (pQueueWaitIdle && queue) pQueueWaitIdle(queue);
   if (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u ||
       FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u)
      LOG("FG2_QMD_CONSTANT_CACHE phase=teardown path=%s queue_wait_complete=%u command_buffer_lifetime_complete=1 cleanup_begin=1 fault_state=INSPECT_COMPLETE_STREAM",
          FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE ? "invalidate" : "control",
          queue != VK_NULL_HANDLE);
   if (pDestroyCommandPool && pool) pDestroyCommandPool(dev, pool, NULL);
   if (pDestroyPipeline && graphics_pipeline) pDestroyPipeline(dev, graphics_pipeline, NULL);
   if (pDestroyPipeline && pipeline) pDestroyPipeline(dev, pipeline, NULL);
   if (pDestroyPipelineLayout && pipeline_layout) pDestroyPipelineLayout(dev, pipeline_layout, NULL);
   if (pDestroyFramebuffer && framebuffer) pDestroyFramebuffer(dev, framebuffer, NULL);
   if (pDestroyRenderPass && render_pass) pDestroyRenderPass(dev, render_pass, NULL);
   if (pDestroyDescriptorPool && descriptor_pool) pDestroyDescriptorPool(dev, descriptor_pool, NULL);
   if (pDestroyDescriptorSetLayout && descriptor_layout) pDestroyDescriptorSetLayout(dev, descriptor_layout, NULL);
   if (pDestroyShaderModule && shader) pDestroyShaderModule(dev, shader, NULL);
   if (pDestroyShaderModule && vert_shader) pDestroyShaderModule(dev, vert_shader, NULL);
   if (pDestroyShaderModule && frag_shader) pDestroyShaderModule(dev, frag_shader, NULL);
   if (pDestroySampler && sampler) pDestroySampler(dev, sampler, NULL);
   if (pDestroyImageView && source_view) pDestroyImageView(dev, source_view, NULL);
   if (pDestroyImageView && destination_view) pDestroyImageView(dev, destination_view, NULL);
   if (pDestroyImage && source_img) pDestroyImage(dev, source_img, NULL);
   if (pDestroyImage && destination_img) pDestroyImage(dev, destination_img, NULL);
   if (pDestroyBuffer && readback) pDestroyBuffer(dev, readback, NULL);
   if (pFreeMemory && readback_mem) pFreeMemory(dev, readback_mem, NULL);
   if (pFreeMemory && source_mem) pFreeMemory(dev, source_mem, NULL);
   if (pFreeMemory && destination_mem) pFreeMemory(dev, destination_mem, NULL);
   if (pDestroyDevice && dev) pDestroyDevice(dev, NULL);
   if (pDestroyInstance && inst) pDestroyInstance(inst, NULL);
   if (inst) LOG("cleanup: Vulkan device and instance destroyed");
   if (FG2_QMD_SHADER_CONSTANT_CACHE_CONTROL == 1u ||
       FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE == 1u)
      LOG("FG2_QMD_CONSTANT_CACHE phase=teardown path=%s cleanup_complete=1 fault_state=INSPECT_COMPLETE_STREAM",
          FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE ? "invalidate" : "control");
   LOG("=== done; log at sdmc:/nvk_render_compute.log ===");
   if (g_log) fclose(g_log);
   return failures ? 1 : (r != VK_SUCCESS ? 1 : 0);
}
