# FG-2 compute-root stale-boundary finding — 2026-08-20

Status: `BLOCKED`

The observation-only root diagnostic reproduces the two earlier FG-2 failures exactly while
showing that iteration 2's seed 42 remains current at every CPU/QMD boundary instrumented by this
change:

1. API input and both graphics and compute descriptor roots contain 42 immediately after the
   combined fragment+compute push.
2. The compute-root source contains 42 before upload copy.
3. The CPU mapping backing root GPU VA `0xc7f40000` contains 42 after the copy. This observes the
   CPU alias only; it does not prove visibility to Tegra.
4. NAK's generated QMD decodes its root cbuf 0 address to the same `0xc7f40000`.
5. Direct dispatch submits QMD VA `0xc7f40800` (`PCAS=0x00c7f408`) correlated with that root VA.
6. Despite those current values, iteration 2 and every later iteration return the exact stable
   stale pixel/checksum signature from both pre-instrumentation artifacts.

Marker order, command-buffer identity, cbuf decoding, root-address match, and dispatch order all
correlate. Root and QMD upload addresses are reused across command-buffer reset, but the CPU mapping
is rewritten with 42 before the reused address is encoded and dispatched. No GPU fault, timeout,
notifier error, or diagnostic observer effect appears in the complete retained stream.

The one justified classification is **unresolved after current CPU/QMD state before GPU
consumption**. The last confirmed-current boundary is the CPU mapping backing the root GPU VA plus
the generated/dispatched QMD address encoding. The first stale behavior is the shader-produced
output. This evidence does not distinguish CPU-cache visibility, GPU constant-buffer cache state,
address-translation visibility, or constant-buffer consumption, so none of those mechanisms is
claimed as the cause.

FG-2 remains `BLOCKED`, and the shared-layout/push-stage-cache hypothesis remains `REJECTED`. The
diagnostic does not satisfy the original render-compute-image-chain acceptance contract.

The smallest follow-up OpenSpec change should be named
`test-compute-root-upload-cache-flush`. It should add exactly one controlled semantic variant:
flush only the reused compute-root upload's CPU mapping before the existing QMD upload/dispatch,
retain the same artifact and 64-iteration oracle otherwise, and compare the complete signature
against this baseline. That experiment targets only CPU-to-GPU visibility at the unresolved
boundary; it must not be folded into the diagnostic baseline or treated as a fix without hardware
evidence.

Evidence:

- `docs/testing/FG2_ROOT_STATE_DIAGNOSTIC_RUN_2026-08-20.md`
- `docs/testing/raw/FG2_ROOT_DIAG_NXLINK_2026-08-20.txt`
- `docs/testing/FG2_NVK_RENDER_COMPUTE_FAILURE_2026-08-20.md`
- `docs/testing/FG2_NVK_RENDER_COMPUTE_SHARED_LAYOUT_FAILURE_2026-08-20.md`
