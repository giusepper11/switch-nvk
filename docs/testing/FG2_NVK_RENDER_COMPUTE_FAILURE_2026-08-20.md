# FG-2 `nvk_render_compute` hardware failure — 2026-08-20

```text
Date: 2026-08-20
Repository commit: 4b1ba311861704ae7257d9e1e14fab175e84f988
Spec/milestone: openspec/changes/prove-render-compute-image-chain / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1, netloaded at 192.168.15.13:28280
Firmware/Atmosphere/libnx context when relevant: firmware/CFW version not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application `.nro`
Diagnostics enabled: complete combined nxlink stdout/stderr capture; application log path configured as sdmc:/nvk_render_compute.log; MESA_LOG_FILE configured as sdmc:/nvk_render_compute_mesa.log
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM optimal-tiled images
Swapchain/buffer count/present mode when relevant: N/A — headless artifact
Clock/OC state when relevant: not changed/captured
Test artifact/version: APP=nvk_render_compute, TITLE="NVK Render Compute", BUILD=chain1, VERSION=0.61.0-chain1; `.nro` SHA256 9e23dab9097dd3be4d19b583c35bed996b41cef402f7f3d16be6bc4c72993770
Run duration/iteration count: all 64 submit/wait/readback iterations executed; result reproduced twice
Expected result: exact CPU-oracle match for all 256 pixels on all 64 changing-seed iterations
Observed result: iteration 1/64 (seed 5) exact; iterations 2-64 each mismatched all 256 pixels; RESULT FAIL
Deterministic validation/checksum: first pass observed/expected 0xb7d223e5; second iteration observed 0xc17a35a5 versus expected 0x0daf4ac5; subsequent observed checksum remained 0xc17a35a5 while expected checksums changed
GPU error notifier/error info: complete combined stream inspected; no timeout, ERRNOTIF, ERRINFO, or GPU fault; expected NVK non-conformance warning only
Relevant timing summary: N/A — correctness-only artifact
Raw evidence location/reference: docs/testing/raw/FG2_NVK_RENDER_COMPUTE_NXLINK_FAILURE_2026-08-20.txt; device application log sdmc:/nvk_render_compute.log
Conclusion/status: BLOCKED; the artifact does not prove FG-2
```

The mismatch is diagnostic rather than arbitrary. For iteration 2, graphics used seed 42, while the
observed first pixel `0xfab61a38` is exactly the compute transformation of that seed-42 graphics
pixel using stale compute seed 5. Alpha `0xfa` independently identifies low seed bits 5. This is
consistent with compute-stage push-constant state not being refreshed when separate graphics and
compute pipeline layouts use the same offset/value in sequence. It is an inference from the exact
output, not yet a proven driver root cause.

The next falsifiable test is to revise the OpenSpec design to use one compatible shared pipeline
layout and one push update covering fragment and compute stages, then rebuild from a new immutable
commit and repeat the full hardware acceptance run. No capability promotion is justified by this
failure.
