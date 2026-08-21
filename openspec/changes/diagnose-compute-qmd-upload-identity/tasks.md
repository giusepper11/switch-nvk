## 1. Reconfirm source authority and the frozen experiment

- [x] 1.1 Compare the current extracted NVK QMD upload path with clean Mesa 25.0.7 and `patches/switch-nvk-mesa-25.0.7.patch`; record the durable source and every generated or intentionally tracked representation that must remain synchronized.
- [x] 1.2 Re-audit `winsys/smoke/nvk_render_compute.c`, generated shaders, and retained FG-2 logs to freeze the resources, layouts, seeds, barriers, command-buffer reset/rerecord flow, QMD/direct-dispatch path, submission/wait behavior, 64-iteration oracle, and exact baseline output signatures.
- [x] 1.3 Confirm the existing QMD allocation size/alignment/copy semantics and the Switch CPU cache primitive's declaration, void result, and possible cache-line rounding before changing the durable patch.

## 2. Add bounded QMD payload identity diagnostics

- [x] 2.1 Add strict opt-in QMD identity and QMD-cache selectors whose absent, zero, malformed, and non-Switch cases are side-effect free; keep the existing root-upload cache selector disabled and independent.
- [x] 2.2 Locally replace the direct QMD upload-data call with the equivalent existing upload allocation using the same 256-byte size and `0x100` alignment followed by the same `memcpy`, exposing the returned mapping identically in control and enabled artifacts without changing allocation order or policy.
- [x] 2.3 Compute deterministic 64-bit FNV-1a identities over all 256 generated and mapped QMD bytes, use full `memcmp` for source-to-mapping equality, and report explicitly that CPU equality leaves GPU visibility unproven.
- [x] 2.4 Track the preceding valid generated QMD bytes, identity, GPU address, record, and command-buffer identity only for the bounded diagnostic stream; reset correlation on an identity change and avoid dynamic allocation or whole-QMD logging in the traced loop.
- [x] 2.5 Classify iteration 2 as eligible only when order is valid, both copies are exact, the QMD GPU address is reused, and the complete naturally generated payload differs from iteration 1; emit explicit `no_prior`, `fresh_address`, `identical_payload`, copy/order failure, and eligible states.
- [x] 2.6 Extend the bounded QMD-upload/dispatch records with source and mapped identities, exact equality results, previous/current QMD addresses, reuse and payload classifications, QMD mapping/size, root address, cache selector/action/range, dispatch address, and false-positive indicators.

## 3. Add the conditionally eligible QMD-only cache variant

- [x] 3.1 On Switch only, invoke `armDCacheFlush(qmd_map, sizeof(qmd))` after copy and identity validation and before unchanged direct dispatch only when every eligibility condition and the strict enabled selector are satisfied.
- [x] 3.2 Prove from source and targeted checks that the variant never flushes identical, first, fresh-address, mismatched, or out-of-order QMDs and never flushes the root, containing upload arena, push commands, descriptors, or another allocation.
- [x] 3.3 Confirm the experiment adds no artificial QMD nonce or field change, fresh-allocation policy, GPU command/invalidation, barrier, wait, submission change, shader change, or readback/oracle change.
- [x] 3.4 Add distinct identity-control and cache-enabled build/version/path reporting to `nvk_render_compute`, set selectors before Vulkan creation, and preserve the unchanged 64-iteration validation contract.

## 4. Reconstruct and validate host artifacts

- [x] 4.1 Regenerate/update the durable Mesa patch and extracted build tree according to `REPRODUCE.md`, update only confirmed authoritative or intentional mirrors, and verify the complete patch applies cleanly to pristine Mesa 25.0.7 with touched generated sources matching byte-for-byte.
- [x] 4.2 Cross-build NVK plus identity-control and cache-enabled NROs and run applicable static/host checks for selector parsing, bounded records, source/map mismatch, no-prior, fresh/reused address, identical/changed payload, eligibility, invocation, and skip paths without making a hardware claim.
- [x] 4.3 Record paired artifact hashes and generated shader hashes and prove the artifacts preserve identical resources, QMD generation/copy, GPU commands, synchronization, dispatch, submission/wait behavior, oracle, and 64-iteration contract except for selection/reporting and the eligible QMD-only CPU cache call.
- [x] 4.4 Add a host evidence record with reconstruction/build commands and results, exact source revision, selector and log-schema checks, artifact/shader identities, paired source review, and all claims that remain `IMPLEMENTED_UNPROVEN` pending hardware.

## 5. Run the gated real-Tegra experiment

- [x] 5.1 Commit the exact experiment source and confirm the recorded artifacts were built from that immutable revision before hardware execution.
- [ ] 5.2 Run the disabled identity control for all 64 iterations on real Switch/Tegra hardware, retain and inspect the complete unfiltered stream, and require the exact established baseline signature, source/map equality, address/dispatch correlation, and clean recorded fault state.
- [ ] 5.3 From the valid control, classify QMD payload identity and enforce the decision gate: if the reused payload is identical, do not run the enabled artifact; if it differs naturally and every prerequisite is valid, authorize the enabled run; otherwise classify the result inconclusive and stop.
- [ ] 5.4 Only when authorized by task 5.3, run the enabled artifact for the unchanged 64 iterations on the same hardware/configuration, confirm the QMD-only invocation and exact requested range before correlated dispatch, and retain the complete unfiltered stream and exact per-iteration results.
- [ ] 5.5 Inspect every executed complete log before filtering and record commit/artifact hashes, console/model and environment, diagnostic selectors, iteration count, QMD/root identities and addresses, equality/reuse/eligibility/action state, pixels/checksums, observer-effect assessment, warnings, and supported `ERRNOTIF`/`ERRINFO` or other GPU fault state according to `docs/testing/HARDWARE_EVIDENCE.md`.

## 6. Classify and retain the evidence

- [ ] 6.1 Produce the applicable identity-only or paired comparison table covering selector, iteration/seed, source/mapped QMD identities and exact equality, previous/current QMD VA, reuse/payload/eligibility/action/range, root and dispatched addresses, pixel/checksum, validation count, and GPU fault state.
- [ ] 6.2 Classify exactly one justified outcome: `control_invalid`, `inconclusive`, `identical_payload_non_discriminating`, `specific_qmd_flush_insufficient`, `behavior_changed_unresolved`, or `hypothesis_supported_experiment_only`; never label the experimental path a production fix.
- [ ] 6.3 Retain immutable raw logs and the hardware record under `docs/testing/`, add the causal finding under `docs/research/`, and preserve whether the enabled hardware artifact was forbidden, not needed, or executed.
- [ ] 6.4 Update `docs/status/CAPABILITY_MATRIX.md` only with durable hardware facts, keep FG-2 `BLOCKED` until the original unmodified image-chain contract passes, and add an ADR only if a later production decision requires one.
- [ ] 6.5 If the result does not satisfy the original FG-2 contract, name only the next smallest separately specified experiment and stop without adding fresh-QMD allocation, GPU invalidation, waits, or other semantic variants to this change.
