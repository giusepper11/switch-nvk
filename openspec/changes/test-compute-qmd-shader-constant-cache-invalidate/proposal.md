## Why

FG-2 remains `BLOCKED` after the valid two-root/two-QMD hardware pair at `f542b44` changed the failure from seed-5 retention to seed-42 retention: iterations 1-2 were exact, then iterations 3-64 retained iteration-2 output despite 63/63 adjacent root and QMD address transitions. The retained complete QMD payload has dwords 0-7 equal to zero, so pre-Pascal QMD v0.6 `INVALIDATE_SHADER_CONSTANT_CACHE` (bit 255, dword 7 bit 31) is currently clear and provides an untested one-bit discriminator for whether explicit constant-cache invalidation participates in this exact reused-root behavior.

## What Changes

- Add a same-source paired FG-2 causal experiment around the retained failure-producing schedule: both arms alternate root A/B and QMD X/Y for 64 iterations; control generates bit 255 as `0`, while the variant generates only `INVALIDATE_SHADER_CONSTANT_CACHE=TRUE`.
- Prefer expressing the experiment through the typed NAK QMD v0.6 construction interface at the normal ownership/generation point. Keep any raw post-construction mutation as a rejected-by-default fallback that requires layout-derived justification, mutation before the normal mapped copy, and exact payload proof.
- Add strict mutually exclusive build/runtime selectors, symmetric bounded diagnostics, and early rejection of malformed, contradictory, or combined cache/root/QMD experiment configurations. Absent selectors preserve ordinary execution exactly.
- Require full 256-byte paired QMD comparisons proving `control XOR variant` equals only bit 255 for each corresponding iteration, with field decode, root encoding, generated-to-mapped equality, QMD-to-`PCAS` correlation, schedule, ordering, and independent output-oracle evidence.
- Freeze the retained seed sequence, root contents and A/B addresses, QMD X/Y addresses and allocation order, shaders, images, descriptors, layouts, barriers, command-buffer lifecycle, direct dispatch, existing `INVALIDATE_SKED_CACHES`, submission/waits, cache maintenance, BO/NvMap backing, readback, oracle, and 64-iteration count.
- Prohibit every additional cache, membar, synchronization, timing, allocation, backing, nonce, and reuse-distance intervention. In particular, do not add shader-data/texture/method-level invalidations, QMD membars, WFI, sleeps, a third root, a fourth QMD, or a different BO/NvMap allocation.
- Gate variant execution on a control that exactly reproduces iterations 1-2 and seed-42 retention on iterations 3-64, complete unfiltered-log review, immutable artifact provenance, all causal correlations, and no unexplained fault.
- Define only the outcomes `qmd_shader_constant_cache_invalidate_supported_experiment_only`, `specific_qmd_shader_constant_cache_invalidate_insufficient`, `behavior_changed_unresolved`, and `inconclusive`. Preserve exact negative or changed signatures and keep FG-2 `BLOCKED` regardless of outcome.
- Retain host/build evidence and, only after authorization, paired real-Tegra logs and findings. Experimental support still requires a separate production-path remediation decision and a separate original image-chain 64/64 hardware acceptance change before any capability promotion.

This one-bit intervention has higher immediate information value than increasing root reuse distance because it directly exercises a public QMD v0.6 control over shader constant-cache invalidation while preserving the already validated address schedule, allocation footprint, and launch ordering. Public QMD definitions are `PUBLICLY_DOCUMENTED`; old Nouveau Gallium setting the field and modern inspected NAK leaving it clear are `SOURCE_CODE_EVIDENCE`. Neither is `PROVEN_HW`, identifies an undocumented GM20B cache key, or establishes a general NVK defect.

## Capabilities

### New Capabilities

- `compute-qmd-shader-constant-cache-invalidate`: Paired FG-2 QMD v0.6 bit-255 experiment, exact one-bit payload isolation, strict control and hardware gates, bounded correlation, narrow result classifications, and non-promotion rules.

### Modified Capabilities

None.

## Impact

- Planning targets the durable Mesa 25.0.7 Switch patch first. The reconstructed `mesa-25/` tree is build-authoritative but disposable; no tracked mirror of `nvk_cmd_dispatch.c` exists. The likely typed path also touches NAK QMD construction declarations/implementation reconstructed from the durable patch.
- Experiment integration affects `nvk_cmd_upload_qmd`, the `nvk_render_compute` selector/build harness, bounded diagnostics, and host/provenance checks; it does not change public Vulkan APIs, ordinary execution, the fixed shader/image workload, or FG-3 submission/fence behavior.
- Evidence work will add host and optional paired hardware records under `docs/testing/`, a narrow finding under `docs/research/`, and result-only reconciliation in `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` without rewriting prior evidence or advancing FG-2.
- Root reuse distance, physical backing identity, other cache bits/methods, membars, synchronization, allocator redesign, FG-3, FG-4, and native-title work remain separate changes.
