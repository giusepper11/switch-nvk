## Why

FG-2 remains `BLOCKED` after real-hardware evidence proved that flushing only the reused compute-root upload does not change the stable stale output signature. The proposed QMD-upload flush is not yet demonstrably discriminating because the changing seed resides in the root buffer while the reused QMD may contain identical bytes across iterations; payload identity must be established before spending a hardware run on that semantic variant.

## What Changes

- Add bounded, observation-only diagnostics that correlate the generated QMD bytes, mapped upload bytes, QMD GPU virtual address, root GPU virtual address, dispatch address, and exact output for the first two FG-2 iterations.
- Require compact whole-payload identity values for the 256-byte QMD source and mapped upload, while retaining the existing bounded root-address field decoding as an independent structural check.
- Gate the QMD-only CPU cache-flush variant on a valid control, reused QMD address, exact source-to-mapping copy, and meaningfully changed QMD payload; identical payloads make that variant non-discriminating and require the change to stop without performing the flush.
- When eligible, compare same-source disabled-control and enabled artifacts that differ only by a CPU data-cache flush over the reused 256-byte QMD mapping after copy and before direct dispatch.
- Classify hardware outcomes narrowly and retain complete evidence without promoting an experimental cache branch into ordinary execution or claiming FG-2 success.

## Capabilities

### New Capabilities

- `compute-qmd-upload-identity`: Bounded QMD payload/upload identity diagnostics, eligibility rules for a reused-QMD-only CPU cache-flush experiment, paired-control requirements, and real-hardware outcome classification.

### Modified Capabilities

None.

## Impact

- Affects the durable Mesa 25.0.7 Switch patch for the NVK compute QMD upload/dispatch path and the `nvk_render_compute` experiment selector/reporting.
- Adds host artifact comparison and real-Tegra evidence work under `docs/testing/` and `docs/research/`, followed by capability-matrix updates appropriate to the observed result.
- Does not change public Vulkan APIs, shader behavior, image resources/layouts, command ordering, synchronization, allocation policy, or the ordinary non-experimental runtime path.
