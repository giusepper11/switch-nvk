# FG-2 push-constant stage-state finding — 2026-08-20

Status: `REJECTED`

The first immutable FG-2 artifact passed its first seed and then produced a stable incorrect compute
result for every later seed. Exact pixel algebra shows that graphics consumed each new seed while
compute continued to consume seed 5. The image chain, draw, dispatch, and transfer all completed
without a reported GPU fault.

The tested hypothesis was a push-constant stage/layout cache interaction: graphics and compute used
separate pipeline layouts with the same four-byte offset, and each iteration wrote the same seed to
fragment and then compute stages. A driver-side state cache might have suppressed the second write
after seeing the same bytes even though the consumer stage/layout differed.

Immutable controlled variant `d9dfc65` replaced that arrangement with one compatible pipeline
layout, one push range covering fragment and compute, and one combined-stage seed push before the
draw. Its 64-iteration real-hardware validation signature was identical to the first failure: seed 5
passed, every later iteration returned the same stale compute result, and no GPU fault was reported.
The shared-layout/cache-interaction hypothesis is therefore `REJECTED`.

This result does not establish a general NVK defect. Before another semantic change, lower-level
instrumentation must capture the seed in both descriptor roots after `nvk_CmdPushConstants2KHR`, the
compute root bytes and upload address in `nvk_cmd_upload_qmd`, and the root-descriptor address encoded
for dispatch. Those observations will distinguish command-buffer state/reset, upload lifetime/QMD
encoding, and GPU-visible constant-buffer cache/coherency as the next boundary.

Evidence: `docs/testing/FG2_NVK_RENDER_COMPUTE_FAILURE_2026-08-20.md` and
`docs/testing/raw/FG2_NVK_RENDER_COMPUTE_NXLINK_FAILURE_2026-08-20.txt`, plus
`docs/testing/FG2_NVK_RENDER_COMPUTE_SHARED_LAYOUT_FAILURE_2026-08-20.md` and
`docs/testing/raw/FG2_NVK_RENDER_COMPUTE_SHARED_LAYOUT_NXLINK_FAILURE_2026-08-20.txt`.
