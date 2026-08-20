# FG-1 `nvk_compute` hardware run — 2026-08-20

Target: Nintendo Switch OLED, `192.168.15.13:28280` via nxlink.

Artifact: `APP=nvk_compute`, `BUILD=compute1`, `VERSION=0.60.0-compute1`.

The following is the complete quiet nxlink stdout capture. `DRM_SHIM_DEBUG` was disabled for
readability; a preceding diagnostic package/run also completed without an `ERRNOTIF` or `ERRINFO`
line.

```text
=== NVK FG-1 compute/storage-image smoke [BUILD compute1] ===
contract: 64 elements, 8x8 RGBA8, 64 iterations
A vkCreateInstance -> 0
vk_instance: enumerate_cb NULL
vk_instance: try_create_for_drm SET -> drm path
vk_instance: &drmGetDevices2=0x85946ce90 ret=1
WARNING: NVK is not a conformant Vulkan implementation, testing use only.
B enumerate count -> 0 (1 device(s))
C qfam[0]: flags=0xf count=1
C using compute queue family 0
D vkCreateDevice -> 0
E host-visible buffers initialized; CPU fallback disabled
F source image + storage image allocated (RGBA8, 8x8)
G NAK compute shader module (2252 bytes) -> 0
G vkCreateComputePipelines -> 0
G intended path: NAK compute pipeline + 4 explicit bindings
iteration 1/64 VERIFY OK: buffer[0]=0xb58595e5 image[0]=0xff070b03
iteration 64/64 VERIFY OK: buffer[0]=0xb58595e5 image[0]=0xff070b03
RESULT PASS: 64/64 iterations exact; checksum buffer=0x5d09a285 image=0xa4a662a5
EXPECTED CHECKSUM: buffer=0x5d09a285 image=0xa4a662a5
INTENDED GPU PATH EXECUTED: NAK compute -> storage buffers + sampled image -> storage image -> transfer readback
FALLBACK/BYPASS: none; CPU only initialized inputs and validated readback
GPU FAULT/ERROR NOTIFIER: inspect complete device log and Mesa log; no local fallback is accepted
cleanup: Vulkan device and instance destroyed
=== done; log at sdmc:/nvk_compute.log ===
exiting ...
```
