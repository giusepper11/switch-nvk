# FG-2 QMD upload identity hardware run — 2026-08-21

```text
Date: 2026-08-21
Repository experiment-source commit: 6828d6e23d5562801dbc0f6e1060e0b0b9b2c7a0
Evidence follow-up commit before execution: fb26bec
Spec/milestone: openspec/changes/diagnose-compute-qmd-upload-identity / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1 console at 192.168.15.13:28280
Firmware/Atmosphere/libnx context when relevant: firmware/Atmosphere versions not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application .nro, identity-control artifact
Diagnostics enabled: ROOT_DIAG_LIMIT=2; NVK_ROOT_TRACE=2; NVK_QMD_UPLOAD_IDENTITY=1; NVK_QMD_UPLOAD_CACHE_FLUSH absent; root-cache selector absent; complete combined nxlink/application/driver stream
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM optimal-tiled images
Swapchain/buffer count/present mode when relevant: N/A — headless artifact
Clock/OC state when relevant: not changed/captured
Control artifact/version: nvk_render_compute_qmdidentity_control.nro; BUILD=chain2-qmdidentity1-control; VERSION=0.63.0-qmdidentity-control
Control artifact SHA256: e78804a5692d9c5a699fddc1051470b7c177b99486c5b86f968ae95ab59b8c60
Enabled artifact/version: nvk_render_compute_qmdcache_enabled.nro; BUILD=chain2-qmdcache1; VERSION=0.63.0-qmdcache-enabled
Enabled artifact SHA256: 4c538c22cadb1a93052d9c5a307e62a54edf6f2d21af6fe0a1c24788f3db048d
Enabled hardware execution: FORBIDDEN by the valid control's identical-payload decision gate; artifact was not sent or run
Generated shader-source SHA256: vertex dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0; fragment 14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235; compute 1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49; generated tri_shaders.h 21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14
Run duration/iteration count: control completed all 64 submit/wait/readback iterations; detailed diagnostics bounded to iterations 1-2
Expected result: valid control reproduces the retained stale signature and establishes whether the reused 256-byte QMD payload changes naturally
Observed result: iteration 1 passed exactly; iterations 2-64 reproduced the retained stale pixel/checksum; iterations 1 and 2 used byte-identical generated/mapped QMD payloads at the reused QMD VA
Deterministic validation/checksum: iteration 1 pixel[0]=0xfa47d33f and checksum 0xb7d223e5 exact; iterations 2-64 pixel[0]=0xfab61a38 and checksum 0xc17a35a5; RESULT FAIL with 63/64 iterations mismatched
GPU error notifier/error info: complete 108-line stream inspected before filtering; no timeout, ERRNOTIF, ERRINFO, GPU fault, or unexplained driver warning; expected NVK non-conformance warning only; no fault-triggered notifier capture was applicable
Relevant timing summary: N/A — correctness-only identity experiment
Raw evidence: docs/testing/raw/FG2_QMD_UPLOAD_IDENTITY_CONTROL_NXLINK_2026-08-21.txt; 108 lines; SHA256 d603c24ff5ba5ab57ba090a651bba29222fdd4934073359939e4c15a7be1c5c2
Configured device logs: sdmc:/nvk_render_compute.log was not separately retrieved; the installed shim sink is present in the complete combined nxlink stream; the configured Mesa log was not separately retrieved
Instrumentation observer effect: none detected; control exactly reproduced the established iteration-1 pass and iterations-2-64 stale signature
Conclusion/status: PROVEN_HW for byte-identical source/mapped QMD payload identity at the reused QMD address in the tested first two iterations; classification identical_payload_non_discriminating; enabled QMD flush forbidden; FG-2 remains BLOCKED
```

## Identity-control correlation

The complete raw stream was read before filtering. Both bounded marker, push, root upload, decoded-QMD,
QMD-identity upload, legacy QMD-upload, dispatch, and result records appear once and in order. The
source/mapped equality and cross-iteration equality decisions use full 256-byte `memcmp` results; the
hashes below are compact identities only.

| Selector | Iteration / seed | QMD source / mapped identity | Exact source/map | Previous / current QMD VA | Reuse / payload / eligibility | Cache action / requested range | Root / dispatched QMD VA | Pixel / checksum | Validation | GPU fault state |
|---|---|---|---:|---|---|---|---|---|---|---|
| identity=1, cache=0 | 1 / 5 | `d4d9b1921f9202ad` / `d4d9b1921f9202ad` | 1 | none / `0xc7f40800` | no prior / N/A / `no_prior` | disabled / 0 | `0xc7f40000` / `0xc7f40800` | `0xfa47d33f` / `0xb7d223e5` | exact, 256/256 | none reported |
| identity=1, cache=0 | 2 / 42 | `d4d9b1921f9202ad` / `d4d9b1921f9202ad` | 1 | `0xc7f40800` / `0xc7f40800` | reused / exact equal / `identical_payload` | disabled / 0 | `0xc7f40000` / `0xc7f40800` | `0xfab61a38` / `0xc17a35a5` | mismatch, 0/256 | none reported |
| identity=1, cache=0 | 3-64 / fixed sequence | bounded identity N/A | N/A | bounded identity N/A | bounded identity N/A | disabled | source-controlled | always `0xfab61a38` / `0xc17a35a5` | 0/256 each | none reported |

For both bounded records, `source_map_match=1`, `order_match=1`,
`cpu_mapping_backing_gpu_va=1`, and `artificial_qmd_change=0`. The decoded QMD root address matches the
uploaded root VA, and the direct-dispatch `PCAS` address matches the recorded QMD VA. CPU equality does
not establish GPU visibility.

## Decision and classification

Classification: `identical_payload_non_discriminating`.

The valid control proves that the naturally generated QMD payload is byte-identical across the first
two correlated iterations while the QMD address is reused and the established stale signature remains.
Flushing that identical QMD range cannot discriminate current from stale QMD data, so the enabled
artifact was forbidden and was not executed. This is not a rejection of all QMD/GPU-consumption
hypotheses and is not a production fix.

FG-2 remains `BLOCKED` because the original render-to-texture → sampled image → compute → storage image
contract still fails after iteration 1. The next smallest work is a separately specified QMD-address-
reuse experiment; this change adds no fresh allocation, GPU invalidation, wait, or semantic variant.
