# FG-2 QMD upload identity finding — 2026-08-21

## Finding

Real Tegra execution proves that the first two correlated FG-2 compute launches generate and copy the
same complete 256-byte QMD payload at reused GPU VA `0xc7f40800`. Both source and mapped identities are
`0xd4d9b1921f9202ad`, both source-to-mapping comparisons are exact, and the cross-iteration full
`memcmp` reports equality. Root VA `0xc7f40000` and direct-dispatch QMD VA correlate exactly.

The control also reproduces the established behavior: iteration 1 seed 5 is exact, while iterations
2-64 retain pixel `0xfab61a38` and checksum `0xc17a35a5`. No GPU fault was reported in the complete
stream.

## Consequence

Outcome: `identical_payload_non_discriminating`.

A CPU flush of the reused QMD mapping would only flush bytes identical to the prior launch and therefore
cannot test visibility of changed QMD contents. The enabled artifact was forbidden by the decision gate
and was not executed. This does not reject QMD-address reuse or GPU-consumption state generally and does
not alter the earlier finding that the separate root-only CPU flush was insufficient.

FG-2 remains `BLOCKED`. The next smallest work is a separately specified QMD-address-reuse experiment.

Evidence: `docs/testing/FG2_QMD_UPLOAD_IDENTITY_RUN_2026-08-21.md` and
`docs/testing/raw/FG2_QMD_UPLOAD_IDENTITY_CONTROL_NXLINK_2026-08-21.txt`.
