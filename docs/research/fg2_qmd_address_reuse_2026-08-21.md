# FG-2 QMD address-reuse finding — 2026-08-21

## Finding

Real Tegra execution proves that the fresh-address artifact changed the selected QMD GPU VA on all 63
consecutive transitions by alternating `0xc7f40800` and `0xc7f40900`. Every generated 256-byte QMD was
an exact copy of its selected mapping, every direct-dispatch `PCAS` address matched that selection, and
the complete bounded payload was identical to the valid same-source control.

The intervention did not change output. Both runs passed iteration 1 seed 5 exactly, then produced the
established stale pixel `0xfab61a38` and checksum `0xc17a35a5` for iterations 2-64. Neither complete
497-line stream reported a GPU fault.

## Consequence

Outcome: `specific_qmd_address_change_insufficient`.

This result applies only to alternating two distinct adjacent QMD slots, where a slot may recur after
one intervening dispatch. It does not reject all QMD-address, cache, allocator, launch-state, or
GPU-consumption hypotheses and is not a production fix. The experiment selectors remain opt-in.

FG-2 remains `BLOCKED`. Any further causal intervention requires a separately specified smaller test;
FG-3 is unchanged.

Evidence: `docs/testing/FG2_QMD_ADDRESS_REUSE_RUN_2026-08-21.md` and the two immutable raw streams under
`docs/testing/raw/FG2_QMD_ADDRESS_{CONTROL,FRESH}_NXLINK_2026-08-21.txt`.
