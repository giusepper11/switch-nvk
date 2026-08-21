## 1. Reconfirm the authoritative source and frozen control

- [x] 1.1 Compare the current `mesa-25/src/nouveau/vulkan/nvk_cmd_dispatch.c` and any other touched NVK files against clean Mesa 25.0.7, the durable `patches/switch-nvk-mesa-25.0.7.patch`, and tracked mirrors; record the authoritative/durable representations and preserve unrelated ignored-tree edits.
- [x] 1.2 Re-audit `winsys/smoke/nvk_render_compute.c` and its generated shaders to freeze the 16x16 RGBA8 resources, shared layout, seed sequence, barriers, reset/rerecord flow, one direct dispatch, submit/wait/readback path, 64-iteration oracle, and retained stale pixel/checksum signature.
- [x] 1.3 Confirm the established Horizon/libnx CPU data-cache flush declaration and semantics available to the Mesa Switch build, including its void result and possible cache-line granularity, without selecting a broader winsys or GPU-cache operation.

## 2. Add the isolated root-upload cache experiment

- [x] 2.1 Add strict Switch-only parsing for `NVK_ROOT_UPLOAD_CACHE_FLUSH=1`, with absent, zero, malformed, and non-Switch cases side-effect free and independent of `NVK_ROOT_TRACE` bounds.
- [x] 2.2 Add the `FG2_ROOT_UPLOAD_CACHE_FLUSH` build control and startup/path records to `nvk_render_compute`, constrain it to `0` or `1`, and set the environment selector before Vulkan instance creation only for the enabled artifact.
- [x] 2.3 Track the immediately preceding direct compute-root GPU VA for the selected command-buffer stream across resets, classify each upload as `no_prior`, `reused`, or `fresh_address`, and invalidate unexpected identity/order correlations rather than expanding experiment scope.
- [x] 2.4 After copying and inspecting the current root bytes, invoke the libnx CPU cache flush on exactly `root_desc_map` and `sizeof(*root)` only for `reused`, before existing QMD construction/upload; do not flush the first/fresh root, QMD, upload BO, or any other allocation and do not add a GPU command or wait.
- [x] 2.5 Extend the bounded root diagnostics with selector, prior/current GPU VA, reuse classification, CPU mapping/range, action/skip reason, and `after_copy_before_qmd` ordering, reporting invocation rather than a fabricated cache or GPU-visibility result.

## 3. Reconstruct and host-validate paired artifacts

- [x] 3.1 Regenerate/update the durable Mesa 25.0.7 patch and ignored extracted build tree, update only confirmed intentional mirrors, and verify a clean Mesa reconstruction applies the complete patch and matches every touched generated source byte-for-byte without modifying `pristine-25.0.7/`.
- [x] 3.2 Compile the NVK target and applicable host/static checks, including absent, `0`, malformed, and `1` selector cases, command-buffer identity changes, first/fresh upload skips, reused upload invocation, and diagnostic-disabled behavior without making a hardware claim.
- [x] 3.3 Build same-source disabled-control and enabled-variant `nvk_render_compute` NROs with distinct build/version tags, record both hashes, and prove their generated shader hashes, GPU command path, resources, synchronization, QMD behavior, dispatch, oracle, and 64-iteration contract are identical apart from selection/reporting and the targeted CPU cache call.
- [x] 3.4 Add a `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN` host record covering source revision, reconstruction, build commands/results, selector checks, artifact identities, paired review, shader hashes, and all remaining hardware-only claims.

## 4. Run the paired real-Tegra experiment

- [ ] 4.1 Commit the exact experiment source revision and confirm both hashed artifacts were built from it before any hardware execution.
- [ ] 4.2 Run the disabled control for all 64 iterations on real Switch/Tegra hardware, retain the complete unfiltered application/driver stream, and require the exact retained stale signature, bounded diagnostic correlation, and clean fault state before interpreting the variant.
- [ ] 4.3 Run the enabled variant for the unchanged 64 iterations on the same hardware/configuration, confirm iteration 1 skips for `no_prior`, iteration 2 invokes the root-only flush for the reused VA before correlated QMD/dispatch, and retain exact per-iteration outputs and the complete unfiltered stream.
- [ ] 4.4 Inspect both complete logs before filtering, record commit/artifact hashes, hardware/model, firmware/Atmosphere/libnx context, diagnostic flags, iteration count, expected/observed values, address reuse, requested flush ranges, shader hashes, observer effects, and supported `ERRNOTIF`/`ERRINFO` state according to `docs/testing/HARDWARE_EVIDENCE.md`.

## 5. Classify and record the evidence

- [ ] 5.1 Produce a paired control-versus-variant table correlating selector, iteration/seed, root source/mapped bytes, prior/current root VA, reuse/action/range, QMD root VA, dispatched QMD VA, pixel/checksum, validation count, and GPU fault state; mark missing or inconsistent correlations inconclusive.
- [ ] 5.2 Classify exactly one result from the full evidence: `control_invalid`, `inconclusive`, `specific_flush_insufficient`, `behavior_changed_unresolved`, or `hypothesis_supported_experiment_only`, without labeling the gated variant a production fix.
- [ ] 5.3 Retain the immutable paired hardware record and raw logs under `docs/testing/`, add the detailed causal finding under `docs/research/`, and explicitly preserve both positive and negative results.
- [ ] 5.4 Update `docs/status/CAPABILITY_MATRIX.md` only for durable facts supported by the paired hardware evidence, keep FG-2 `BLOCKED` until the original unmodified render-compute-image-chain contract passes, and add an ADR only if a later persistent production decision requires one.
- [ ] 5.5 If the result is not a complete valid pass, name only the next smallest discriminating OpenSpec change for the remaining boundary and stop without adding a QMD flush, GPU invalidation, wait, new allocation policy, or other semantic variant to this change.
