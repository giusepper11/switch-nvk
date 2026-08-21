# FG-2 reused root-upload CPU cache-flush paired hardware run — 2026-08-21

```text
Date: 2026-08-21
Repository commit: c79700438f42b588c6a1a9b7dcff296f8235349e (immutable experiment source)
Spec/milestone: openspec/changes/test-compute-root-upload-cache-flush / FG-2
Hardware/model: Nintendo Switch OLED, same real Tegra X1 console at 192.168.15.13:28280 used by the preceding FG-2 diagnostic
Firmware/Atmosphere/libnx context when relevant: firmware/Atmosphere versions not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application .nro, paired same-source control and enabled variant
Diagnostics enabled: ROOT_DIAG_LIMIT=2; runtime NVK_ROOT_TRACE=2; enabled artifact alone sets NVK_ROOT_UPLOAD_CACHE_FLUSH=1; complete combined nxlink/application/driver streams
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM optimal-tiled images
Swapchain/buffer count/present mode when relevant: N/A — headless artifact
Clock/OC state when relevant: not changed/captured
Control artifact/version: nvk_render_compute_rootflush_control.nro; BUILD=chain2-rootdiag1-control; VERSION=0.62.0-rootflush-control
Control artifact SHA256: 1656f810a07b52a77e5848da3c549842fa591f0411fb2e495bf4cacc3235a8ba
Enabled artifact/version: nvk_render_compute_rootflush_enabled.nro; BUILD=chain2-rootflush1; VERSION=0.62.0-rootflush-enabled
Enabled artifact SHA256: 5c2302d73c597d996aada097f9a7407d8d35b2af75788db83b4d39c7f882ca39
Generated shader SHA256: vertex a5710f31551964e99298c1f94289c001c9d729a4305c0271129e92966018a3df; fragment bcccb45ad89425e16c5dc13d11d3465466eb5417db5381956957d4d1a79d7c20; compute d2742455295117324b12268247857e4f4d19095b4eaf7c1e26817aa3832133d3
Run duration/iteration count: both artifacts completed all 64 submit/wait/readback iterations; detailed diagnostics bounded to iterations 1-2
Expected result: control reproduces the retained stale signature; enabled run invokes the reused-root-only flush and either changes behavior or matches the exact 64-iteration oracle
Observed result: both runs passed iteration 1 seed 5 and produced the identical retained stale pixel/checksum for iterations 2-64; enabled iteration 2 confirmed root-only cache invocation
Deterministic validation/checksum: iteration 1 pixel[0]=0xfa47d33f and checksum 0xb7d223e5 exact; iterations 2-64 pixel[0]=0xfab61a38 and checksum 0xc17a35a5 in both artifacts; RESULT FAIL with 63/64 iterations mismatched in both
GPU error notifier/error info: both complete streams inspected; no timeout, ERRNOTIF, ERRINFO, GPU fault, or unexplained driver warning; expected NVK non-conformance warning only; no fault-triggered notifier capture was applicable
Relevant timing summary: N/A — correctness-only causal experiment
Raw control evidence: docs/testing/raw/FG2_ROOT_UPLOAD_CACHE_FLUSH_CONTROL_NXLINK_2026-08-21.txt (105 lines, SHA256 f1f01fa8e664c7410c23c1ad0ed5ffc7f40a2804426570fd703543f7ea533250)
Raw enabled evidence: docs/testing/raw/FG2_ROOT_UPLOAD_CACHE_FLUSH_ENABLED_NXLINK_2026-08-21.txt (105 lines, SHA256 393e71ef6e106f8b9c5946d7e403d324371eba46ec7626b2490ec77581a93c4e)
Configured device logs: sdmc:/nvk_render_compute.log was overwritten by each run and not separately retrieved; the installed shim sink is present in each complete combined nxlink stream; the configured Mesa log was not separately retrieved
Instrumentation observer effect: none detected; the paired disabled control exactly reproduced the three prior retained stale-signature artifacts
Conclusion/status: PROVEN_HW experiment outcome; classification specific_flush_insufficient; the requested CPU flush of the reused root upload is not sufficient to change the stale result; FG-2 remains BLOCKED
```

## Paired correlation

The complete streams were read before filtering. Each bounded marker, push, upload, QMD,
QMD-upload, dispatch, and result record appeared once and in order. The differing CPU pointers and
command-buffer identities are expected across separate processes; the GPU VAs and semantic fields are
identical.

| Run | Selector | Iteration / seed | Root source / mapped seed | Previous / current root GPU VA | Reuse / action / requested range | QMD root VA | Dispatched QMD VA | Pixel / checksum | Validation | GPU fault state |
|---|---:|---|---|---|---|---|---|---|---|---|
| Control | 0 | 1 / 5 | 5 / 5 | none / `0xc7f40000` | `no_prior` / `disabled` / none | `0xc7f40000` | `0xc7f40800` | `0xfa47d33f` / `0xb7d223e5` | exact, 256/256 | none reported |
| Enabled | 1 | 1 / 5 | 5 / 5 | none / `0xc7f40000` | `no_prior` / `skipped_no_prior` / none | `0xc7f40000` | `0xc7f40800` | `0xfa47d33f` / `0xb7d223e5` | exact, 256/256 | none reported |
| Control | 0 | 2 / 42 | 42 / 42 | `0xc7f40000` / `0xc7f40000` | `reused` / `disabled` / none | `0xc7f40000` | `0xc7f40800` | `0xfab61a38` / `0xc17a35a5` | mismatch, 0/256 | none reported |
| Enabled | 1 | 2 / 42 | 42 / 42 | `0xc7f40000` / `0xc7f40000` | `reused` / `invoked` / `root_map=0x95d166000`, 2048 bytes | `0xc7f40000` | `0xc7f40800` | `0xfab61a38` / `0xc17a35a5` | mismatch, 0/256 | none reported |
| Control | 0 | 3-64 / fixed sequence | bounded root fields N/A | source-controlled same stream | disabled | source-controlled | source-controlled | always `0xfab61a38` / `0xc17a35a5` | 0/256 each | none reported |
| Enabled | 1 | 3-64 / fixed sequence | bounded root fields N/A | source-controlled same stream | reused-root branch remains enabled; detailed records bounded | source-controlled | source-controlled | always `0xfab61a38` / `0xc17a35a5` | 0/256 each | none reported |

For enabled record 2, `ordering=after_copy_before_qmd`, `order_match=1`, `address_match=1`, and
`decode_ok=1`. The log reports invocation of `armDCacheFlush(root_desc_map, sizeof(*root))`; because the
libnx primitive returns `void` and rounds internally to cache-line boundaries, this proves only the
requested call and completed control flow, not byte-granular hardware behavior or GPU visibility.

## Classification

Classification: `specific_flush_insufficient`.

The disabled control exactly reproduced the retained baseline, the enabled path was fully correlated,
and the enabled output remained byte-for-byte at the retained stale signature without a fault. Therefore
flushing only the CPU-written reused compute-root upload range is not sufficient to change this failure.
This does not prove that CPU cache state is irrelevant generally, does not identify the final stale
boundary, and does not establish a production cache policy.

FG-2 remains `BLOCKED` because the original unmodified render-compute-image-chain contract still fails.
No experimental cache branch is promoted into ordinary execution.

The next smallest discriminating OpenSpec change is `test-compute-qmd-upload-cache-flush`.
