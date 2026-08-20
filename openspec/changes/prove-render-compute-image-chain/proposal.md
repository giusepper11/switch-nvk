## Why

FG-1 proves NAK compute dispatch, storage buffers, sampled-image reads, storage-image writes, and readback on real Tegra, but it does not prove that graphics output in an offscreen image is visible and correct when sampled by a later compute pass. FG-2 is the next prerequisite because temporal pipelines require that exact graphics-to-image-to-compute dependency chain before scheduling, external-memory, or frame-generation work is meaningful.

## What Changes

- Add one standalone deterministic FG-2 smoke capability that renders into offscreen image A, samples image A from compute, writes image B, and reads image B back for exact CPU validation.
- Require explicit image layouts, access masks, pipeline stages, and dependencies between graphics attachment writes, compute sampling/storage writes, and transfer readback.
- Require independently calculated expected pixels and a checksum over repeated real-hardware runs.
- Require unambiguous intended-path, fallback/bypass, and GPU fault/error reporting.
- Keep the change planning and eventual proof narrow: no presentation requirement, render graph, asynchronous winsys work, external memory, NVN, or native-title integration.

## Capabilities

### New Capabilities

- `render-compute-image-chain`: Deterministic offscreen graphics rendering followed by compute sampling, storage-image output, explicit dependencies, readback validation, and real-Tegra evidence.

### Modified Capabilities

<!-- No existing capability requirements are modified. FG-1 remains independently accepted. -->

## Impact

- Expected implementation/test area: a dedicated source under `winsys/smoke/`, embedded deterministic graphics/compute shaders, and the existing `winsys/build-nro.sh` application-selection path.
- Candidate reuse comes from `nvk_tri.c` for offscreen render-pass/pipeline/image/readback setup and `nvk_compute.c` for descriptors, compute dispatch, storage-image output, exact validation, and intended-path logging.
- The proof exercises Vulkan graphics and compute pipelines, two test-owned optimal-tiled RGBA8 images, explicit synchronization/layout transitions, and a host-visible staging buffer.
- Project state changes for FG-2 remain forbidden until real-hardware acceptance evidence exists; this proposal does not promote render-to-texture or chaining capability.
