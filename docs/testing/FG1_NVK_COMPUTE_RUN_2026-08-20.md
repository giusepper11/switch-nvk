# FG-1 `nvk_compute` hardware run — 2026-08-20

Target: Nintendo Switch OLED, `192.168.15.13:28280` via nxlink.

Artifact: `APP=nvk_compute`, `BUILD=compute1`, `VERSION=0.60.0-compute1`.

Immutable provenance:

```text
Repository commit: 10da2b3e40997bdf36275bcd6aef85085b07e5c7
Artifact SHA256: cd58f469477245b7b06930c515743d35a8d3d2bc8c1fb0fa1e841738bd1129c9
Build/version identifier: APP=nvk_compute, TITLE="NVK Compute", BUILD=compute1, VERSION=0.60.0-compute1
Hardware/model: Nintendo Switch OLED, real Tegra X1
Iterations: 64
Expected checksum: buffer=0x5d09a285 image=0xa4a662a5
Observed checksum: buffer=0x5d09a285 image=0xa4a662a5
GPU fault/error state: no ERRNOTIF, ERRINFO, timeout, or unexplained GPU fault in the complete retained application and combined nxlink/driver streams
Raw log references: docs/testing/raw/FG1_NVK_COMPUTE_APP_2026-08-20.txt; docs/testing/raw/FG1_NVK_COMPUTE_NXLINK_2026-08-20.txt; sdmc:/nvk_compute.log
Conclusion: PROVEN_HW for the tested FG-1 path from the exact committed source and hashed artifact
```

The artifact was rebuilt from the clean commit above. The complete hardware run was repeated from
that artifact and passed exact element/pixel validation on all 64 iterations. The combined nxlink
stream was retained because Mesa emitted its expected non-conformance warning there. Although the
artifact configured `MESA_LOG_FILE=sdmc:/nvk_compute_mesa.log`, that separate file was not created;
the SD root was inspected over FTP and contained `nvk_compute.log` only. This absence is recorded
rather than represented as a retained file.

The following is the complete quiet application-log capture. `DRM_SHIM_DEBUG` was disabled for
readability. The complete combined nxlink stdout/stderr capture additionally contained only the
expected NVK non-conformance warning and no `ERRNOTIF`, `ERRINFO`, timeout, or fault line.

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
