# FG-2 reused compute-root CPU cache-flush finding

## Finding

A paired same-source real-Tegra experiment classified the reused-root-only CPU cache flush as
`specific_flush_insufficient`.

The disabled control reproduced the exact established failure: iteration 1 seed 5 passed, while
iterations 2-64 each returned pixel `0xfab61a38` and checksum `0xc17a35a5`. In the enabled run,
iteration 1 correctly skipped because it had no prior root address. Iteration 2 copied and observed seed
42, reused root GPU VA `0xc7f40000`, invoked `armDCacheFlush` for the requested 2048-byte root mapping
after the copy and before QMD construction, encoded that same root VA in QMD cbuf 0, and dispatched QMD
VA `0xc7f40800`. The output nevertheless remained exactly `0xfab61a38` / `0xc17a35a5` for iterations
2-64.

Both complete 105-line streams ended normally, contained no timeout or GPU-fault indicator, and differed
in the intended selector/action fields without an observed output difference. This rules out only the
specified root-range CPU flush as a sufficient intervention. It does not establish whether the remaining
boundary is the separately reused QMD upload, GPU cache/translation state, or constant-buffer
consumption, and it does not support a generalized production cache policy.

Immutable evidence is retained in
`docs/testing/FG2_ROOT_UPLOAD_CACHE_FLUSH_RUN_2026-08-21.md` and its two raw logs. FG-2 remains
`BLOCKED`; the original unmodified image-chain contract has not passed.

The next smallest discriminating OpenSpec change is `test-compute-qmd-upload-cache-flush`.
