## Why

FG-2 remains `BLOCKED` after real-Tegra evidence at `c04e1ae` proved that fresh QMD storage relative to every preceding dispatch is insufficient: 63/63 QMD-address transitions and 64/64 exact direct-dispatch correlations retained the established stale result. The next smallest unresolved boundary is whether the GPU result changes when the current compute-root contents are placed at a GPU virtual address fresh relative to the immediately preceding dispatch.

## What Changes

- Add a same-source paired FG-2 control/variant experiment around the existing 64-iteration render → sampled image → compute → storage image artifact. Both arms retain fresh QMD storage; the control reuses the ordinary compute-root GPU VA, while the variant changes only the selected compute-root GPU VA relative to the preceding dispatch.
- Guarantee and prove 63/63 adjacent root-address transitions in the variant with a bounded experiment-only allocation mechanism, while giving both arms identical allocation/copy observer structure and preserving ordinary execution when selectors are absent.
- Treat the naturally changed QMD root-pointer encoding as a required dependent consequence, not a second independent variable. Require complete QMD comparison to prove that every difference is confined to the decoded root-address field(s), with exact source-to-mapping and dispatch-address correlation.
- Preserve shaders, seeds, root semantic contents and shape, images, descriptors, layouts, barriers, command-buffer lifecycle, GPU method sequence, submission, waits, readback, cache behavior, synchronization, and the independent oracle. Prohibit cache operations, invalidations, extra synchronization, payload mutation, timing perturbation, and unrelated allocator policy.
- Gate the variant on a valid same-source control, immutable artifacts and shader identities, complete unfiltered real-hardware logs, 64-iteration correlation, exact oracle results, and interpretable GPU fault/error state.
- Define only the narrow outcomes `root_address_reuse_hypothesis_supported_experiment_only`, `specific_root_address_change_insufficient`, `behavior_changed_unresolved`, and `inconclusive`; keep FG-2 `BLOCKED` regardless of result, require a later separately reviewed original-chain hardware acceptance run after any experimental pass, and exclude all FG-3 work.
- Reconcile current project wording during apply: preserve the completed QMD-address negative result, update stale `MILESTONES.md` next-boundary text to compute-root-address reuse, and retain immutable historical evidence.

## Capabilities

### New Capabilities

- `compute-root-address-reuse`: Paired FG-2 reused/fresh compute-root GPU-address experiment, dependent QMD root-pointer isolation, bounded correlation, hardware gates, narrow classifications, and non-promotion rules.

### Modified Capabilities

None.

## Impact

- Affects the durable Mesa 25.0.7 Switch patch for NVK compute-root/QMD upload and direct dispatch, plus the `nvk_render_compute` experiment selectors, bounded diagnostics, and paired artifact reporting. The gitignored `mesa-25/` tree remains a regenerated build representation, not the durable source.
- Adds host/provenance and, only after the control gate, real-Tegra evidence work under `docs/testing/` and `docs/research/`; updates `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` only with wording and results justified by retained evidence.
- Does not change public Vulkan APIs, normal non-experimental allocation behavior, shaders, image-chain resources or synchronization, FG-2 status, or any FG-3 implementation.
