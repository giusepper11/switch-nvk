## Why

FG-2 remains `BLOCKED` after real-hardware diagnostics proved that iteration 2's current compute seed reaches the CPU mapping backing the reused root-upload GPU virtual address and the QMD submitted for dispatch, while shader output retains the prior iteration's stale signature. The next smallest discriminating experiment is to change only CPU-to-GPU cache visibility for that reused root upload and determine on real Tegra whether the failure signature changes.

## What Changes

- Add one opt-in controlled variant of `nvk_render_compute` that flushes only the CPU-written mapped range for the reused compute-root upload before the existing QMD upload and direct dispatch.
- Preserve the baseline shader bytes, images, descriptors, shared pipeline layout, seed sequence, barriers, command order, submission/wait behavior, 64-iteration CPU oracle, and root/QMD diagnostics so the cache operation is the only intended semantic variable.
- Require explicit path reporting and exact baseline-versus-variant correlation, including upload address reuse, flushed address/range, cache-operation invocation, output pixels/checksums, diagnostic ordering, and GPU fault/error state.
- Require immutable host-built artifacts and complete real-Tegra evidence before drawing a CPU-to-GPU visibility conclusion; host or emulator results remain non-authoritative for cache behavior.
- Classify outcomes narrowly: a changed/passing signature supports the tested root-upload cache-visibility hypothesis but is not a production fix; an unchanged signature rejects this specific flush variant and leaves the unresolved boundary elsewhere before or at GPU constant-buffer consumption.
- Keep the observation-only root diagnostic unchanged and keep FG-2 `BLOCKED` unless the original render-compute-image-chain acceptance contract independently passes.

## Capabilities

### New Capabilities

- `compute-root-upload-cache-flush`: Controlled real-hardware experiment for flushing the CPU-written reused compute-root upload range before QMD upload/dispatch, with strict baseline preservation, observability, false-positive detection, and evidence-based outcome classification.

### Modified Capabilities

<!-- No accepted capability requirements change. The observation-only compute-root-state diagnostic and the render-compute-image-chain acceptance contract remain unchanged. -->

## Impact

- Expected source points: the compute-root upload path in Mesa NVK, the existing bounded FG-2 root diagnostics, and `winsys/smoke/nvk_render_compute.c` for experiment selection and result correlation.
- The durable `patches/switch-nvk-mesa-25.0.7.patch` remains the reproducible Mesa source of truth; the ignored extracted `mesa-25/` build tree must be updated coherently, with no edits to `pristine-25.0.7/` and no assumed tracked mirror.
- Evidence outputs include a host-validation record, immutable artifact hashes, a complete hardware record/raw log, a baseline-versus-variant research finding, and evidence-backed capability-matrix updates where justified.
- No public Vulkan API, generalized cache policy, reusable runtime behavior, WSI/external-memory path, asynchronous submission behavior, shader algorithm, or native-title integration is added.
