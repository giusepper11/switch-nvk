# FG-2 QMD shader-constant-cache invalidation finding — 2026-08-21

## Finding

A valid immutable control reproduced the retained two-root/two-QMD signature on real Tegra: seeds 5
and 42 were exact, then iterations 3-64 retained seed-42 pixel `0xf5031a17` and checksum
`0x0daf4ac5`. It proved 63/63 root and QMD transitions plus 64/64 root copies, QMD copies, typed
one-bit counterfactual comparisons, field/root decodes, direct-`PCAS` correlations, ordering, and
complete teardown without a reported GPU fault.

The same-source variant set only QMD v0.6 `INVALIDATE_SHADER_CONSTANT_CACHE` bit 255. All 64 variant
pixels and checksums exactly matched control, including the 62 retained seed-42 failures. Every
causal, provenance, and fault prerequisite remained valid.

## Consequence

Outcome: `specific_qmd_shader_constant_cache_invalidate_insufficient`.

This one QMD field is insufficient for the controlled A/B-root and X/Y-QMD launch pattern. The result
does not prove a cache key, lifetime, backing rule, generic GM20B cache defect, or general NVK defect.
Root reuse distance is the next smaller separate discriminator and requires its own OpenSpec change;
no further intervention is combined here. FG-2 remains `BLOCKED`, and FG-3/FG-4 are unchanged.

Complete evidence is in
`docs/testing/FG2_QMD_SHADER_CONSTANT_CACHE_INVALIDATE_RUN_2026-08-21.md` and the paired raw streams
under `docs/testing/raw/`.
