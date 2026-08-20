## 1. Confirm the vertical-slice contract

- [x] 1.1 Inspect `nvk_tri.c`, `nvk_compute.c`, their embedded shader sources/build products, and `winsys/build-nro.sh`; confirm the dedicated artifact is the smallest authoritative integration point and record any source/mirror implications.
- [x] 1.2 Finalize the 16x16 RGBA8 per-iteration graphics pattern, compute transformation, image-A/image-B sentinels, 64-iteration seed sequence, CPU oracle, and expected checksum calculation before implementing GPU commands.

## 2. Add deterministic graphics output

- [x] 2.1 Add the standalone FG-2 smoke artifact and minimal selectable build/package wiring without modifying unrelated runtime, winsys, WSI, or existing smoke-test behavior.
- [x] 2.2 Add the full-coverage graphics shaders and pipeline, test-owned offscreen image A, view, render pass, framebuffer, and explicit ownership/lifetime cleanup.
- [x] 2.3 Record the sentinel initialization and deterministic draw into image A, with logging that distinguishes a real shader draw from clear-only, CPU initialization, or skipped graphics.

## 3. Chain graphics output through compute

- [x] 3.1 Add test-owned image B, its view, nearest/clamp sampled-image descriptor for A, storage-image descriptor for B, compute shader, pipeline, and explicit ownership/lifetime cleanup.
- [x] 3.2 Add the explicit color-attachment-write to compute-sampled-read transition for image A, including the required layouts, stages, access masks, and full subresource range.
- [x] 3.3 Dispatch compute over the full extent so output B depends on every sampled A pixel and the iteration seed; log and reject any CPU processing, direct-copy path, constant-output bypass, or image aliasing.
- [x] 3.4 Add the explicit storage-write to transfer-read transition for image B and copy B into a host-visible readback buffer with the required transfer-to-host visibility handling.

## 4. Validate and expose failures

- [x] 4.1 Compare every output pixel on every iteration with the independent CPU oracle, reject sentinel/stale/partial output, and report the first coordinate/value mismatch before checksum summaries.
- [x] 4.2 Calculate expected and observed checksums independently across all pixels, execute 64 changing-seed iterations, and keep success logging bounded to the contract plus first/last iterations.
- [x] 4.3 Report the selected graphics+compute queue, graphics/draw stage, A-to-compute transition, sampled-image read, B storage write, B-to-transfer transition, readback, fallback/bypass state, and supported GPU fault/error diagnostics.

## 5. Build and host-validate the artifact

- [x] 5.1 Generate/embed the shader artifacts using the repository's authoritative process and verify their provenance and build inputs.
- [x] 5.2 Build and package the dedicated `.nro`, verify its application title/version/build tag and SHA256, and run applicable compile/static checks without promoting the capability beyond `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`.
- [x] 5.3 Review the recorded barriers, descriptors, sentinels, CPU oracle, and failure paths against the delta spec, including the documented false-positive cases.

## 6. Run real-hardware acceptance

- [x] 6.1 From an exact committed source revision, rebuild and hash the artifact, then run all 64 iterations on real Switch/Tegra hardware; do not substitute an emulator.
- [x] 6.2 Retain and inspect the complete application and Mesa/driver streams before filtered excerpts, record all warnings, and collect supported `ERRNOTIF`/`ERRINFO` data after any timeout or GPU fault.
- [x] 6.3 Record hardware/model, firmware/toolchain context, format and extent, commit, artifact hash/version, iteration count, exact expected/observed pixels and checksums, intended-path/fallback state, GPU error state, and raw evidence references according to `docs/testing/HARDWARE_EVIDENCE.md`.

## 7. Update project state only after evidence review

- [x] 7.1 If and only if all hardware acceptance criteria pass, promote FG-2 and the render-to-texture/image-chain capability rows to `PROVEN_HW`; otherwise retain the evidence and use `IMPLEMENTED_UNPROVEN`, `BLOCKED`, or `REJECTED` as warranted.

## 8. Test the shared-layout hypothesis

- [x] 8.1 Revise the artifact to create one compatible pipeline layout used by both graphics and compute, with the sampled/storage descriptor-set layout and one four-byte push-constant range covering fragment and compute stages.
- [x] 8.2 Push the current iteration seed once for both stages before the draw, remove the second compute-only push, and verify that shaders, image contents, barriers, dispatch, CPU oracle, and stale-seed detection remain unchanged.
- [x] 8.3 Build and package the controlled variant from the authoritative source, review layout/descriptor compatibility, and record a new immutable commit, artifact version, and SHA256 without promoting beyond host evidence.
- [x] 8.4 Repeat the complete 64-iteration real-Tegra acceptance run, retain and inspect the full application and driver streams, record supported GPU fault state, and compare the exact output with the failed `4b1ba31` baseline.
- [x] 8.5 If the run passes, record that it supports rather than proves the cache-interaction hypothesis and promote only the tested FG-2 behavior; if it fails, reject the hypothesis, retain `BLOCKED`, and define lower-level instrumentation before another implementation change.

## 9. Finalize the change after the controlled result

- [x] 9.1 Review the complete controlled result for a research note or durable architecture decision, sync the accepted delta spec, and archive the change only after implementation and evidence are complete.
