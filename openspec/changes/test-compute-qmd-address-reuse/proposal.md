## Why

FG-2 remains `BLOCKED` after real-hardware evidence proved that iteration 2 carries the current root bytes through the reused root GPU VA into a byte-identical QMD at reused GPU VA `0xc7f40800`, and direct dispatch selects that reused QMD address while the established stale result persists. The next discriminating boundary is therefore whether GPU consumption changes when the QMD storage address is fresh relative to the preceding dispatch, not generic QMD payload generation or CPU-copy visibility.

## What Changes

- Add a smallest paired control/variant experiment for the existing 64-iteration FG-2 image chain: the control preserves ordinary reused-QMD allocation behavior, while the variant changes only the QMD GPU address operand required to dispatch otherwise identical naturally generated QMD contents from storage fresh relative to the preceding dispatch.
- Require the control to reproduce the retained iteration-1 pass and iterations-2-64 stale signature before the variant can support any causal conclusion.
- Preserve root-address behavior, seeds, QMD generation/copy semantics, shader bytes, resources, descriptors, layouts, barriers, command-buffer lifecycle, direct-dispatch method structure, submission/waits, readback, and the independent oracle; prohibit cache operations, added synchronization, payload nonces, and unrelated QMD-field changes.
- Add bounded correlation for root/QMD source and mapped identities, previous/current addresses, freshness, dispatched `PCAS` address, ordering, selector state, output/oracle result, and GPU fault state, with exact byte comparison authoritative for QMD equality.
- Define narrow experiment-only outcome classifications and require a later independently specified/run unmodified image-chain acceptance test after any successful fresh-address variant; do not promote a variant into ordinary NVK behavior or advance FG-2 from this experiment.
- Correct `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` during apply so their immediate-next-boundary wording names QMD-address reuse rather than generic QMD-upload visibility, without changing FG-2 status.

## Capabilities

### New Capabilities

- `compute-qmd-address-reuse`: Paired FG-2 control/fresh-QMD-address experiment, causal isolation and bounded correlation requirements, real-hardware evidence gates, and narrow outcome classification.

### Modified Capabilities

None.

## Impact

- Affects the durable Mesa 25.0.7 Switch patch for the NVK compute-QMD upload/direct-dispatch path and the `nvk_render_compute` experiment selector, correlation, and artifact reporting; regenerated build trees or intentional mirrors must follow the repository's source-authority rules.
- Adds same-source host artifact comparison plus real-Tegra evidence work under `docs/testing/` and `docs/research/`, followed by source-of-truth wording updates justified by the result.
- Does not change public Vulkan APIs, shaders, image-chain resources or synchronization, ordinary non-experimental execution, FG-2 status, or any FG-3 design or implementation.
