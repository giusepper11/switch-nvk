# FG-2 shared-layout controlled variant hardware failure — 2026-08-20

```text
Date: 2026-08-20
Repository commit: d9dfc6568dd0580884a2352a0622bb3bbb166b6f
Spec/milestone: openspec/changes/prove-render-compute-image-chain / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1, netloaded at 192.168.15.13:28280
Firmware/Atmosphere/libnx context when relevant: firmware/CFW version not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application `.nro`
Diagnostics enabled: complete combined nxlink stdout/stderr capture; application log path configured as sdmc:/nvk_render_compute.log; MESA_LOG_FILE configured as sdmc:/nvk_render_compute_mesa.log
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM optimal-tiled images
Swapchain/buffer count/present mode when relevant: N/A — headless artifact
Clock/OC state when relevant: not changed/captured
Test artifact/version: APP=nvk_render_compute, TITLE="NVK Render Compute", BUILD=chain2, VERSION=0.61.0-chain2; `.nro` SHA256 505586148e0f2282ea71dfd063a3255ccecf9dd433c9b50e76f24a58fec7f30e
Run duration/iteration count: all 64 submit/wait/readback iterations executed
Expected result: exact CPU-oracle match for all 256 pixels on all 64 changing-seed iterations
Observed result: iteration 1/64 (seed 5) exact; iterations 2-64 each mismatched all 256 pixels; RESULT FAIL
Deterministic validation/checksum: seed-5 observed/expected 0xb7d223e5; seed-42 observed 0xc17a35a5 versus expected 0x0daf4ac5; subsequent observed checksum remained 0xc17a35a5 while expected checksums changed
GPU error notifier/error info: complete combined stream inspected; no timeout, ERRNOTIF, ERRINFO, GPU fault, or unexplained driver warning; expected NVK non-conformance warning only
Relevant timing summary: N/A — correctness-only artifact
Raw evidence location/reference: docs/testing/raw/FG2_NVK_RENDER_COMPUTE_SHARED_LAYOUT_NXLINK_FAILURE_2026-08-20.txt; device application log was configured at sdmc:/nvk_render_compute.log but was not retrievable because FTP was unavailable after nxlink exit
Conclusion/status: BLOCKED; the controlled shared-layout variant does not prove FG-2 and rejects the shared-layout/cache-interaction hypothesis
```

The controlled variant changed only the pipeline-layout and push-state contract: one layout was used
for graphics and compute, its push range covered both stages, and one combined-stage seed push was
recorded before the draw. The shaders, images, barriers, dispatch, oracle, and validation remained
unchanged.

Its validation signature is identical to the failed `4b1ba31` baseline: both runs have 63 failing
iterations, the same seed-5 first-iteration success, the same seed-42 first mismatch
`0xfab61a38` versus `0xf5031a17`, the same constant observed checksum `0xc17a35a5`, and the same final
seed-32 mismatch. A shared compatible layout and a single multi-stage push therefore do not change
the stale compute-seed behavior.

The next experiment must instrument the lower-level push/root-descriptor path before changing
runtime behavior. At minimum it must distinguish these boundaries for each iteration:

1. the seed written into both graphics and compute descriptor roots by
   `nvk_CmdPushConstants2KHR`;
2. the compute root value and upload address copied by `nvk_cmd_upload_qmd`;
3. the root-descriptor constant-buffer address encoded in the compute QMD/dispatch;
4. whether the GPU consumes the newly uploaded root data or seed-5 data.

If the CPU descriptor roots or upload copy already contain seed 5, the defect is in command-buffer
state/reset or push-state propagation. If those contain the current seed but the QMD points at stale
storage, the defect is in dispatch encoding or upload lifetime. If the QMD address and uploaded bytes
are current but the shader still observes seed 5, the next gate is GPU-visible constant-buffer cache
or coherency instrumentation. Do not make another semantic fix until this boundary is identified.

## Final review

The accepted capability contract was synced to
`openspec/specs/render-compute-image-chain/spec.md`; it explicitly retains the real-hardware pass gate
and does not claim the capability is proven. No ADR is warranted because the shared-layout approach
was rejected and no durable architectural choice was accepted. The research finding and evidence
records are the durable outputs of this negative experiment.
