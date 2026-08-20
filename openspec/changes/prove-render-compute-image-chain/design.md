## Context

See `proposal.md` for motivation. FG-1 establishes the compute-side primitives on real Tegra, while the inherited graphics tests establish graphics rendering in adjacent paths. No current acceptance artifact proves that color-attachment writes in an offscreen optimal-tiled image become sampled-image reads in compute and then storage-image writes in a second image.

The repository already has the two useful halves: `winsys/smoke/nvk_tri.c` creates an RGBA8 offscreen render target, graphics pipeline, render pass, and image-to-buffer readback; `winsys/smoke/nvk_compute.c` creates sampled/storage images, descriptors, a NAK compute pipeline, explicit barriers, repeated exact validation, and intended-path logging. The implementation should reuse their direct Vulkan/libnx patterns without extracting a framework.

## Goals / Non-Goals

**Goals:**

- Produce one standalone headless smoke artifact with this exact command chain: graphics render pass to image A, compute sampled read from A, compute storage write to image B, transfer copy from B, CPU validation.
- Make the graphics result and compute transformation simple enough for an independent CPU oracle to calculate every pixel exactly.
- Make ownership, lifetime, layouts, stages, access masks, descriptors, sentinel initialization, path reporting, and GPU error inspection explicit.
- Package and test one concrete vertical slice before considering reusable helpers.

**Non-Goals:**

- Presentation, WSI, generalized render graphs, asynchronous submission, winsys synchronization changes, native fences, external `NvMap`, external images, NVN, native-title hooks, interpolation, or performance claims.
- Proving that FG-1 or inherited graphics behavior generalizes beyond this controlled test-owned image chain.

## Decisions

### Use a standalone headless artifact and one queue

Add a dedicated selectable smoke application through the existing `APP=...` build path. Record graphics, compute, and transfer work on the selected graphics+compute-capable queue, using explicit intra-command-buffer dependencies. One queue removes queue-family ownership transfer from FG-2 so the test isolates image chaining rather than FG-3 scheduling.

Alternative: extend `nvk_tri` or `nvk_compute`. Rejected because it would blur their already-proven contracts and make regression results harder to attribute. Alternative: introduce shared smoke infrastructure. Deferred until a second concrete consumer proves which helpers are stable.

### Use two test-owned 16x16 RGBA8 optimal-tiled images

Images A and B use `VK_FORMAT_R8G8B8A8_UNORM`, one mip, one layer, and optimal tiling. Image A has color-attachment and sampled usage; image B has storage and transfer-source usage. A 16x16 extent is large enough to expose addressing/quadrant errors and small enough for exhaustive readback. Each image owns a separate allocation, view, and lifetime covering all submitted work.

Alternative: reuse one image in place. Rejected because it cannot prove the required second storage image and introduces read/write aliasing. Alternative formats are deferred because FG-2 tests chaining, not format breadth.

### Render a full-coverage, integer-defined pattern

Use one oversized full-screen triangle so every sample location is covered without shared-edge ambiguity. The fragment shader derives one of several exact RGBA8 colors from integer pixel coordinates and an iteration seed/push constant. The CPU oracle implements the same discrete region/color rule. Image A is first set to a sentinel distinct from all valid rendered colors so a skipped or partially covered render fails.

Alternative: validate the existing yellow triangle. Rejected because background/edge coverage could let a partial draw look plausible and rasterization-edge conventions complicate exact full-image validation. Alternative: use only a render-pass clear. Rejected because it would not prove the intended graphics shader/draw path.

### Transform sampled texels in compute into image B

Bind image A through a combined sampled-image descriptor with nearest filtering and clamp addressing, and bind image B through a storage-image descriptor. Dispatch exactly over the image extent. The compute shader reads the corresponding texel from A and applies a reversible channel transformation—for example swap R/B, invert G, and combine a small iteration-seed term—before writing B. The CPU oracle calculates expected image A and transformed image B without reading A back.

The transformation must depend on every sampled input channel and the run seed so a compute shader that merely emits constants or repeats an earlier iteration cannot pass. Image B begins each iteration with a distinct sentinel that is not a valid transformed value.

Alternative: direct image copy A to B. Rejected because it does not prove compute sampling or storage-image writes. Alternative: sample with linear filtering. Rejected because floating interpolation and rounding complicate exact byte validation without adding value to FG-2.

### Make every layout and dependency explicit

Use barriers or equivalent render-pass dependencies for these states:

1. image A: undefined/sentinel initialization to `COLOR_ATTACHMENT_OPTIMAL` for color-attachment writes;
2. image A: color-attachment output/write to compute shader/sampled read, ending in `SHADER_READ_ONLY_OPTIMAL`;
3. image B: undefined/sentinel initialization to compute shader/storage write, ending in `GENERAL`;
4. image B: compute shader/storage write to transfer/read, ending in `TRANSFER_SRC_OPTIMAL`;
5. readback buffer: transfer write to host read before CPU validation after completion.

The implementation must spell out source/destination stages, access masks, layouts, and full color subresource ranges. A queue wait may be used for final correctness readback, but it is not a substitute for the intra-chain dependencies and makes no asynchronous-submission claim.

Alternative: split passes with `vkQueueWaitIdle`. Rejected because serialized completion could mask the missing GPU dependency FG-2 exists to prove.

### Validate exact pixels before checksums

Copy B into a host-visible coherent staging buffer and compare every pixel with the CPU oracle on every iteration. Report the first mismatch with coordinates, expected value, and observed value, then report independently calculated expected and observed FNV-1a checksums. Run 64 iterations with a changing deterministic seed to expose stale-data and lifetime errors while keeping logs bounded to the first and last successful iteration.

Checksum equality alone is never acceptance. Presentation is omitted because it adds a CPU display path and is weaker than exhaustive readback.

### Make false positives and GPU errors observable

The artifact reports resource identities/roles, selected queue, each intended stage, sentinel values, iteration contract, exact/checksum results, and a final statement that the graphics-to-A-to-compute-to-B-to-transfer path executed with no CPU image processing. CPU work is limited to seed/input calculation, sentinel upload/clear where needed, and output validation.

How could the test appear to pass without proving graphics → offscreen image → compute → output image? It could validate a prefilled B, read stale B from an earlier iteration, let compute generate the expected constants without sampling A, initialize A on the CPU instead of rendering it, copy A directly to B, validate only a checksum or subset of pixels, use a render-pass clear without a draw, or rely on queue-idle serialization while omitting the required image dependency. Distinct per-iteration sentinels/seeds, a full-coverage draw-generated pattern, an input-dependent compute transformation, distinct images, exhaustive pixel comparison, explicit path logging, and explicit barriers make those cases fail or become visible.

After a timeout or fault, retain the complete application and Mesa/driver streams and collect supported `ERRNOTIF`/`ERRINFO` diagnostics. Inspect complete logs before accepting success. The capability remains `IMPLEMENTED_UNPROVEN` until the immutable commit, artifact hash, hardware/model, 64-iteration results, and fault state are recorded.

## Risks / Trade-offs

- [RGBA8 shader conversion or coordinate convention differs from the CPU oracle] → Use exact representable byte values, integer region decisions, nearest texel access, and verify the oracle contract during host review before hardware promotion.
- [Sentinel setup adds an untracked transfer dependency] → Include sentinel initialization in the documented command sequence and explicit barriers; make sentinels diagnostic only, never expected output.
- [Queue-idle completion hides missing graphics-to-compute visibility] → Keep graphics, compute, and transfer in one dependency chain and require explicit intra-chain barriers even though final host readback waits for completion.
- [A combined artifact obscures which stage failed] → Log creation, transition, draw, dispatch, transfer, and validation phases separately and fail at the first Vulkan or data error.
- [Driver logs are not written to the configured SD file] → Retain combined nxlink stdout/stderr as the driver stream and explicitly record any missing configured file, following the FG-1 provenance precedent.
- [Premature helper extraction widens scope] → Duplicate or locally adapt the small proven patterns first; refactor only when a later concrete consumer requires it.

## Migration Plan

1. Add the standalone artifact and embedded shaders without modifying runtime semantics.
2. Build/package it from the authoritative source and run host/static checks; record only host evidence.
3. Execute the 64-iteration acceptance run on real Tegra, inspect complete logs and fault state, and retain immutable provenance.
4. Promote the FG-2 milestone and capability rows only after the hardware record satisfies the spec; otherwise retain `IMPLEMENTED_UNPROVEN`, `BLOCKED`, or `REJECTED` evidence.
5. Roll back by removing the standalone artifact/build selection and leaving runtime behavior unchanged; negative results remain retained research evidence.
