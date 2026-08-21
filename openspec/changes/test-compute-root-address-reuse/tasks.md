## 1. Reconfirm authority and freeze the causal baseline

- [x] 1.1 Reinspect the current branch, revision, clean worktree, local and authoritative remote `master`, active/archived OpenSpec state, and expected `c04e1ae`/QMD-address evidence; report any discrepancy and use the current synchronized `master` unless policy requires stopping.
- [x] 1.2 Re-read the canonical FG-2 image-chain, root-diagnostic, QMD-identity, root-flush, and QMD-address specs plus the complete retained QMD-address host/hardware/research evidence; record that `specific_qmd_address_change_insufficient` is settled and FG-2 remains `BLOCKED`.
- [x] 1.3 Compare pristine Mesa 25.0.7, `patches/switch-nvk-mesa-25.0.7.patch`, and the regenerated `mesa-25/` compute-root/QMD path; confirm the durable patch is authoritative, enumerate every intentional mirror, and stop on any unexplained divergence.
- [x] 1.4 Freeze and hash the 64-iteration artifact's seed formula, vertex/fragment/compute shader inputs and generated header, images, descriptors, pipeline layout, layouts/barriers, command-buffer reset/rerecord, direct dispatch, submission/wait, readback, exact oracle, and retained output signatures.
- [x] 1.5 Re-audit `nvk_cmd_buffer_upload_alloc`, command upload ownership, `NVK_CMD_MEM_SIZE`, the 2,048-byte root size/alignment, the two 256-byte QMD slots, pool return after the existing wait/reset, and final cleanup lifetime.
- [x] 1.6 Prove that reserving primary root → primary QMD → secondary QMD → alternate root occupies exactly 4,608 bytes in the existing upload BO, preserves the established QMD positions, leaves 60,928 bytes, and does not displace any later upload consumer; if not, document the blocker and stop before implementation or hardware.

## 2. Implement the isolated paired allocation and selection

- [x] 2.1 Add strict `ROOT_ADDRESS_CONTROL`/`ROOT_ADDRESS_FRESH` build selectors and exact-value `NVK_ROOT_ADDRESS_CONTROL`/`NVK_ROOT_ADDRESS_FRESH` runtime selectors with distinct build/path reporting.
- [x] 2.2 Reject malformed and contradictory root-address selectors and combinations with root-cache, QMD-identity/cache, or earlier QMD-address experiment selectors; require both new arms to use the same effective 64-record diagnostic limit.
- [x] 2.3 Keep selector-absent/zero execution on the current ordinary one-root/one-QMD path and preserve every historical experiment selector's existing behavior.
- [x] 2.4 Under either new selector, perform the ordinary primary-root allocation first, copy all current root bytes, and retain its existing size/alignment and expected GPU VA behavior.
- [x] 2.5 Reserve the primary and secondary 256-byte, `0x100`-aligned QMD slots next in the same order and positions as the completed fresh-QMD experiment, before reserving the alternate root.
- [x] 2.6 Reserve one alternate 2,048-byte root with the same minimum-cbuf alignment, copy the identical current root bytes in the same order in both arms, and validate both mappings, alignment, distinct VAs, same-BO capacity, and exact source/primary/alternate equality.
- [x] 2.7 Select primary root for every control record and primary on odd records/alternate on even records for the variant; validate the actual previous/current sequence rather than trusting slot labels and report that a VA may recur after one intervening dispatch.
- [x] 2.8 Select primary QMD on odd records and secondary QMD on even records in both arms so both prove 63/63 adjacent QMD-address transitions with an identical allocation/selection mechanism.
- [x] 2.9 Feed only the selected root VA into normal QMD generation, copy the naturally generated selected QMD exactly once to the selected QMD mapping, and return only that QMD VA through the unchanged direct-dispatch path.
- [x] 2.10 Preserve existing command-memory ownership through submit/wait/reset and cleanup; add no retained 64-allocation list, new production allocator policy, or additional BO unless the pre-implementation hard gate is explicitly revised through OpenSpec.
- [x] 2.11 Prove by targeted source review that neither arm changes shaders, root semantic contents/shape, images, descriptors, layouts, barriers, command-buffer lifecycle, GPU methods, dispatch dimensions, submission, waits, readback, oracle, cache operations, invalidations, synchronization, fences, payloads, sleeps, or timing.

## 3. Prove dependent QMD encoding and add bounded correlation

- [x] 3.1 Generate or canonicalize a fixed-stack reference QMD for the primary root with otherwise identical `nak_qmd_info`, while generating the selected QMD only through the normal `nak_fill_qmd` path in both arms.
- [x] 3.2 Use `nak_get_qmd_cbuf_desc_layout` to identify every root-address bit range, decode reference and selected root addresses, and construct the exact permitted-difference mask without assuming fixed dword locations.
- [x] 3.3 For all 64 records, require decoded-reference/selected root-address matches, complete selected-source-to-selected-mapping equality, and full-payload equality outside the permitted root-address mask; retain hashes only as compact identities.
- [x] 3.4 Add exact full-root comparisons from source to primary mapping and source to alternate mapping, selected mapping identity, and explicit wording that CPU-mapped equality does not prove GPU visibility.
- [x] 3.5 Extend fixed command-buffer-keyed state for previous/first root and QMD VAs, complete comparison results, identities, order, transition counts, dispatch counts, and mask/decode counts; reset correlation on command-buffer identity or record-order failure.
- [x] 3.6 Emit one bounded per-iteration correlation record joining record/iteration/seed, selector/path, root source/mapped identity, primary/alternate/previous/selected root VAs and freshness, QMD source/mapped identity, primary/secondary/previous/selected QMD VAs and freshness, decoded root VA, outside-mask equality, and order.
- [x] 3.7 Retain bounded complete reference/selected/mapped QMD payload records for the primary-root and alternate-root cases needed for exact cross-artifact review, while keeping all equality decisions based on complete in-process comparisons.
- [x] 3.8 Correlate the selected QMD VA to the exact direct `SEND_PCAS_A`/`PCAS` dispatch address for every record and emit a final aggregate covering control/variant root transitions, both QMD transitions, root/QMD copies, root-field-only QMD differences, 64 dispatch matches, ordering, and driver fault state.
- [x] 3.9 Join application-side iteration/seed markers and pixel/checksum/oracle results to the driver record keys, with exactly 64 independent decisions and a compact artifact aggregate.
- [x] 3.10 Keep both artifacts' diagnostic calls and copy work symmetric, use fixed buffers with no diagnostic allocation in the critical loop, cap logging, and avoid unbounded or high-volume per-frame SD writes.

## 4. Integrate artifacts and reconcile the current boundary

- [x] 4.1 Add paired control/variant NRO build recipes, unique version/path tags, and startup reporting that proves the exact selector state before Vulkan creation.
- [x] 4.2 Update `MILESTONES.md` during apply to record the completed QMD-address outcome `specific_qmd_address_change_insufficient` and name compute-root-address reuse as the current smallest discriminator while retaining FG-2 `BLOCKED`.
- [x] 4.3 Tighten the render-compute-chain wording in `docs/status/CAPABILITY_MATRIX.md` to the same active boundary, preserve the existing QMD-address `PROVEN_HW` negative-result row, and do not alter immutable historical evidence.
- [x] 4.4 Update `patches/switch-nvk-mesa-25.0.7.patch` first, regenerate/apply `mesa-25/` according to `REPRODUCE.md`, and synchronize only representations confirmed as build-authoritative or intentional mirrors.
- [x] 4.5 Prepare host and hardware evidence templates with source authority, selector schema, exact allocation footprint/lifetime, QMD-difference rules, artifact/shader hashes, 64-record correlation, complete-log locations, fault review, exact classifications, and FG-2/FG-3 non-promotion fields.

## 5. Validate causal isolation and paired host artifacts

- [x] 5.1 Dry-apply and apply the complete durable patch to pristine Mesa 25.0.7, prove every reconstructed touched source matches the build tree byte-for-byte, and run repository patch/source consistency checks.
- [x] 5.2 Build the NVK archive with the authoritative toolchain and run applicable compile/static checks for all touched Mesa and harness sources.
- [x] 5.3 Test absent, zero, malformed, contradictory, cache-plus-root-address, QMD-experiment-plus-root-address, and valid selector combinations, including the required exit-2 failures and ordinary-path side-effect freedom.
- [x] 5.4 Add/run host checks for slot size/alignment/distinctness, 4,608-byte total footprint, same upload BO, QMD offset preservation, 2,048-byte maximum incremental live memory, lifecycle cleanup, and absence of later upload displacement.
- [x] 5.5 Add/run deterministic checks proving control root reuse, variant odd/even root selection with 63/63 adjacent transitions, and identical odd/even QMD selection with 63/63 transitions in both arms.
- [x] 5.6 Exercise root source/mapping mismatch, QMD source/mapping mismatch, decode mismatch, outside-mask difference, address reuse, dispatch mismatch, ordering failure, and aggregate failure paths so none can produce a valid classification.
- [x] 5.7 Review complete control/variant source and generated method semantics to prove identical allocation/copy observer structure and frozen workload behavior except selected root VA plus its mechanically required QMD encoding; make no raw command-byte identity claim.
- [x] 5.8 Cross-build both NROs from one working revision, verify the vertex/fragment/compute source and generated-header hashes match exactly, and inspect artifact/path tags and bounded log schema.
- [x] 5.9 Record all build commands, toolchain context, patch hash, source-equivalence results, selector tests, allocator tests, artifact hashes, shader hashes, and causal review in a host record classified only as `IMPLEMENTED_UNPROVEN` until hardware evidence exists.
- [x] 5.10 Run strict OpenSpec validation, repository policy checks, `git diff --check`, and any relevant host/static experiment validation; stop before hardware if 63/63 root and QMD transitions, full QMD attribution, dispatch correlation, ordinary-path isolation, or provenance cannot be guaranteed.

## 6. Freeze provenance and run the gated real-Tegra control

- [x] 6.1 Commit the exact experiment source, verify the revision is immutable and contains no unrelated changes, rebuild both artifacts from it, and reproduce the recorded artifact, durable-patch, and shader hashes.
- [x] 6.2 Perform a final source/artifact review confirming the exact causal difference: control selects the reused primary root, variant alternates root VAs, both alternate fresh QMD VAs, and all dependent QMD changes are confined to decoded root-address fields.
- [ ] 6.3 Run only the control for all 64 iterations on the intended real Switch/Tegra configuration, retain the complete unfiltered combined stream, and inspect it before filtering for warnings, timeout, notifier/error-info data, or GPU faults.
- [ ] 6.4 Require iteration 1 exact output, iterations 2-64 exact retained stale output, 0/63 selected-root transitions with ordinary root reuse, 63/63 QMD transitions, exact root/QMD copies, root-field attribution, 64/64 `PCAS` correlations, complete ordering, and no unexplained fault.
- [ ] 6.5 If any control prerequisite fails, retain the evidence, classify the experiment `inconclusive`, do not run or interpret the variant, and stop without adding another intervention.

## 7. Run and analyze the gated fresh-root variant

- [ ] 7.1 Only after task 6.4 passes, run the variant for the unchanged 64 iterations on the same console/configuration and retain the complete unfiltered combined stream.
- [ ] 7.2 Inspect the entire variant stream before filtering for unexpected warnings, timeout, notifier/error-info data, GPU faults, incomplete records, or observer effects.
- [ ] 7.3 Prove 63/63 selected-root transitions, 63/63 QMD transitions, exact root source-to-both-mapping copies, exact selected QMD copies, decoded-root-to-selected-root matches, complete equality outside root-address fields, 64/64 selected-QMD-to-`PCAS` correlations, and complete ordering.
- [ ] 7.4 Compare all 64 variant pixels/checksums and independent oracle decisions against expected values and the valid control, retaining any changed signature exactly without adding or combining a follow-up hypothesis.
- [ ] 7.5 Produce the paired causal table covering source/artifact identities, selectors, iteration/seed, root contents and addresses, QMD contents/address-field attribution and addresses, direct dispatch, ordering, exact output/oracle, and GPU fault/error state.

## 8. Classify, retain, and prevent premature promotion

- [ ] 8.1 Apply exactly one justified classification: `root_address_reuse_hypothesis_supported_experiment_only`, `specific_root_address_change_insufficient`, `behavior_changed_unresolved`, or `inconclusive`, using every prerequisite in the delta spec.
- [ ] 8.2 Retain immutable complete raw logs under `docs/testing/raw/`, a full paired hardware record under `docs/testing/`, and a narrow causal finding under `docs/research/`, including exact hashes, output behavior, transition/correlation counts, and notifier/fault review.
- [ ] 8.3 Update `docs/status/CAPABILITY_MATRIX.md` and `MILESTONES.md` only with the executed result and its exact limits; preserve all negative evidence and keep FG-2 `BLOCKED` regardless of outcome.
- [ ] 8.4 If supported experimentally, record that it is not a production fix and require a separately reviewed ordinary-path remediation decision plus a new original unmodified image-chain 64/64 real-hardware acceptance run before FG-2 can advance.
- [ ] 8.5 If insufficient, record only that this adjacent two-slot compute-root-address change was insufficient and identify only the next smallest unresolved boundary supported by the evidence; if behavior changed or evidence was inconclusive, retain the exact signature/prerequisite failure and propose only a smaller separate follow-up.
- [ ] 8.6 Leave the experiment selectors disabled or remove the experiment-only allocation-selection hunk after evidence capture; do not promote it into ordinary execution, alter FG-3 submission/fence behavior, or perform the original-chain acceptance run in this change.
- [ ] 8.7 Complete all repository/OpenSpec/policy/build-provenance checks, sync the accepted delta into canonical specs, archive only after implementation and evidence reconciliation are complete, run strict validation again, and leave the worktree clean.
