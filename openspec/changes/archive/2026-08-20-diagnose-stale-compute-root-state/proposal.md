## Why

FG-1 proves independent compute, but FG-2 remains `BLOCKED`: graphics consumes each new iteration seed while compute deterministically behaves as though seed 5 remains active after iteration 1. The shared compatible pipeline-layout variant reproduced the same signature, so the shared-layout/push-stage-cache hypothesis is `REJECTED` and another semantic change would be premature until evidence identifies where compute root state first becomes stale.

## What Changes

- Add an opt-in, narrowly bounded diagnostic path for the existing `nvk_render_compute` FG-2 artifact that correlates iteration 1 and iteration 2 (optionally iteration 3) across command-buffer push state, compute root upload, QMD constant-buffer encoding, the CPU mapping backing the uploaded GPU address, and observed output.
- Require the diagnostic record to distinguish graphics and compute root bytes immediately after `nvk_CmdPushConstants2KHR`, including whether iteration 2 contains seed 42 in each root.
- Require compute-root upload bytes, allocation identity, CPU mapping, GPU virtual address, size, root identity, and dispatch correlation to be observable without changing baseline dispatch semantics.
- Require the root constant-buffer slot/address encoded by QMD construction and the address submitted by dispatch to be decoded and checked against the current upload rather than inferred from a full undifferentiated QMD dump.
- Require an immutable observation-only diagnostic artifact and complete real-Tegra logs that identify the first stale boundary, explicitly report observer effects and GPU fault/error state, and leave FG-2 `BLOCKED` unless the original full acceptance contract later passes.
- Permit only a separately planned smallest controlled follow-up when baseline observation cannot discriminate a boundary; cache flushes, waits, or other semantic changes are not part of the baseline diagnostic.

## Capabilities

### New Capabilities

- `compute-root-state-diagnostics`: Iteration-correlated inspection of NVK push/root state, compute-root upload, QMD root-address encoding, uploaded-memory mappings, dispatch results, observer effects, and real-hardware evidence sufficient to locate the first stale compute-state boundary.

### Modified Capabilities

<!-- No accepted capability requirements change. The existing render-compute-image-chain contract and its real-hardware pass gate remain intact. -->

## Impact

- Expected diagnostic source points: `mesa-25/src/nouveau/vulkan/nvk_cmd_buffer.c`, `nvk_cmd_buffer.h`, and `nvk_cmd_dispatch.c`, with the durable Mesa 25.0.7 patch updated as the reproducible source of truth and any intentionally tracked mirror kept coherent.
- Expected test correlation point: `winsys/smoke/nvk_render_compute.c`; its shaders, image chain, barriers, dispatch dimensions, seed sequence, CPU oracle, synchronization, and validation behavior remain unchanged in the observation-only baseline.
- Evidence outputs: a diagnostic hardware record and complete raw logs under `docs/testing/`, plus a detailed first-boundary finding under `docs/research/`. `docs/status/CAPABILITY_MATRIX.md` changes only if real hardware establishes a durable fact; an ADR is added only if a persistent architectural decision results.
- No public Vulkan API or reusable runtime behavior is added. Scope excludes asynchronous submit, WSI, external memory, frame-generation algorithms, NVN/native-title work, speculative cache flushes, broad push-constant rewrites, and Mesa architectural refactors.
