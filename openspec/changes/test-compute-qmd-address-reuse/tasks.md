## 1. Reconfirm source authority and freeze the experiment

- [x] 1.1 Compare the current extracted QMD upload/direct-dispatch path with clean Mesa 25.0.7 and `patches/switch-nvk-mesa-25.0.7.patch`; record the durable source, regenerated tree, and every intentional mirror that must remain synchronized.
- [x] 1.2 Re-audit `winsys/smoke/nvk_render_compute.c`, generated shaders, the retained 108-line QMD identity stream, and prior root-flush evidence to freeze the root/QMD allocation order, seed sequence, resources, descriptors, layouts/barriers, command-buffer reset/rerecord lifecycle, direct dispatch, submission/wait, readback, 64-iteration oracle, and exact baseline signatures.
- [x] 1.3 Verify the command upload allocator's 256-byte size/alignment behavior, reset-time pool reuse, owned-memory lifetime through the existing wait, and available adjacent capacity; prove on host that two distinct aligned 256-byte slots can be reserved after the unchanged root allocation without shifting the primary QMD address.
- [x] 1.4 If task 1.3 cannot guarantee a distinct secondary GPU VA for every consecutive transition without changing another frozen semantic variable, document the limitation and stop before implementation or hardware rather than substituting a broader allocator strategy.

## 2. Implement the isolated paired QMD-slot mechanism

- [x] 2.1 Add strict, dedicated control and fresh-address selectors whose absent, zero, malformed, contradictory, and non-experiment cases are side-effect free and retain the ordinary one-slot QMD upload path; keep all root/QMD cache selectors disabled and independent.
- [x] 2.2 Under either address-experiment selector, perform the ordinary primary 256-byte, `0x100`-aligned QMD allocation first and reserve one secondary allocation of the same size/alignment immediately afterward, in identical order for control and variant and only after the unchanged root allocation.
- [x] 2.3 Copy the naturally generated QMD exactly once: always to the primary slot in the control, and to primary on record 1 then alternating secondary/primary through record 64 in the variant; preserve command-memory ownership and the existing reset-after-wait lifecycle.
- [x] 2.4 Validate before selection that both slot mappings are usable, both GPU VAs are `0x100` aligned and distinct, and the selected sequence changes address on all 63 consecutive variant transitions; report that addresses may recur after one intervening dispatch.
- [x] 2.5 Return only the selected slot address through the existing direct-QMD path and preserve the `SEND_PCAS_A` method structure, dispatch sequence, and semantics except for the required QMD address operand.
- [x] 2.6 Prove by targeted source review that neither path adds a CPU cache flush, GPU invalidation, barrier, synchronization operation, wait, submission change, payload nonce, unrelated QMD-field change, root-address intervention, shader/resource change, alternate dispatch method, or ordinary-runtime policy.

## 3. Add exact bounded correlation

- [x] 3.1 Add fixed-size experiment state keyed to the dedicated command buffer for records 1-64: record/order counters, first and previous generated QMD bytes, previous selected QMD VA, compact identities, and aggregate copy/equality/freshness counts; reset correlation on command-buffer identity or order failure and allocate no diagnostic memory in the loop.
- [x] 3.2 For every record, use full 256-byte comparisons for generated-to-selected-mapping, current-to-first, and current-to-previous QMD equality; retain hashes only as compact labels and emit one bounded complete generated/mapped QMD payload record per artifact for exact cross-artifact comparison.
- [x] 3.3 Emit a fixed per-iteration correlation record joining path/selector, record/iteration/seed, root source/mapped seed, previous/current root VA and reuse, source/mapped QMD identities and exact-equality flags, primary/secondary/selected and previous QMD VAs, freshness, dispatch/`PCAS` address and order matches, pixel/checksum/oracle result, and fault/error state.
- [x] 3.4 Retain detailed root/QMD structural records for the first two dispatches and add a final aggregate proving or disproving `fresh_transitions=63/63`, `dispatch_matches=64/64`, exact QMD copy/equality counts, validation count, and interpretable GPU fault state.
- [x] 3.5 Keep diagnostics bounded to the fixed 64-iteration artifact, use fixed buffers and the existing combined stream rather than unbounded per-frame SD writes, and make the instrumentation identical across the pair so the control can detect observer effects.

## 4. Integrate artifacts and correct current boundary wording

- [x] 4.1 Add distinct same-source control and variant build/version/path reporting to `nvk_render_compute`, set the strict selector before Vulkan creation, and preserve identical shaders, generated shader bytes, images, descriptors, pipeline layout, barriers/layouts, command-buffer lifecycle, submission/waits, readback, and oracle.
- [x] 4.2 Update `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` during apply to name QMD-address reuse/GPU consumption state as the immediate discriminating FG-2 boundary instead of generic QMD-upload visibility; cite the retained QMD identity evidence and keep FG-2 `BLOCKED`.
- [x] 4.3 Update the durable Mesa patch first, regenerate/apply the extracted build tree according to `REPRODUCE.md`, and update only representations confirmed as authoritative or intentional mirrors.
- [x] 4.4 Ensure ordinary builds with experiment selectors absent remain byte/source-equivalent in the relevant QMD upload path and that no experimental branch is promoted into general runtime behavior.

## 5. Validate the paired host artifacts

- [x] 5.1 Verify the complete durable patch applies cleanly to pristine Mesa 25.0.7 and that touched regenerated sources match the durable representation byte-for-byte.
- [x] 5.2 Cross-build NVK plus the control and fresh-address NROs and run applicable host/static checks for selector parsing, slot allocation/alignment/distinctness, exact comparison failure paths, record/order reset, control-primary reuse, 63/63 variant alternation, dispatch correlation, and bounded summaries without making a hardware claim.
- [x] 5.3 Compare same-source artifacts and generated shaders; prove allocation calls/order, QMD generation/copy, root behavior, method structure, resources, synchronization, submission/waits, and oracle are identical except for selector/reporting and the selected QMD address operand, explicitly avoiding a raw GPU-command-byte identity claim.
- [ ] 5.4 Record commit, build commands, control/variant artifact hashes, generated shader hashes, selector/log schema, source-equivalence checks, and all host results in a host evidence record classified only as `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`.
- [x] 5.5 Stop before hardware if the host evidence cannot guarantee the full 63-transition freshness contract, exact payload/copy invariants, or sole-variable isolation.

## 6. Run and gate on the real-Tegra control

- [ ] 6.1 Commit the exact experiment source and verify both recorded artifacts and shader hashes derive from that immutable revision before contacting hardware.
- [ ] 6.2 Run only the control for all 64 iterations on real Switch/Tegra hardware, retain the complete unfiltered stream, and inspect it before filtering for unexpected warnings, timeout, `ERRNOTIF`, `ERRINFO`, or another GPU fault.
- [ ] 6.3 Require the control to reproduce iteration-1 seed-5 exact output, the exact iterations-2-64 `0xfab61a38`/`0xc17a35a5` stale signature, reused root and primary QMD addresses, complete QMD equality/copy evidence, `PCAS` correlation, ordinary command behavior, and a clean interpretable fault state.
- [ ] 6.4 If any control prerequisite fails, retain the complete evidence, classify the comparison `inconclusive`, do not run the variant, and stop without widening the experiment.

## 7. Run the gated fresh-address variant

- [ ] 7.1 Only after task 6.3 passes, run the variant for the unchanged 64 iterations on the same console/configuration and retain the complete unfiltered combined stream.
- [ ] 7.2 Inspect the entire variant log before filtering and prove all 63 consecutive selected QMD-address transitions are fresh, all 64 selected mapped addresses equal the direct-dispatch/`PCAS` addresses, root behavior matches control, and ordering is complete.
- [ ] 7.3 Prove with exact comparisons that each generated QMD equals its selected mapped copy and the frozen naturally generated payload, compare the bounded complete payload records across artifacts, and reject hashes as the authority for equality.
- [ ] 7.4 Compare all 64 exact pixels/checksums and the independent oracle, selector/path state, method structure, warnings, and GPU fault/error state against the valid control; retain any new output signature without interpretation beyond the specified classification gates.

## 8. Classify, retain, and prevent premature promotion

- [ ] 8.1 Produce the paired evidence table covering selector, iteration/seed, root source/mapped seed and addresses, complete QMD identities and exact equalities, primary/secondary/selected/previous QMD VAs, freshness, dispatch/`PCAS` correlation, ordering, pixel/checksum/oracle, artifact/shader provenance, and GPU fault state.
- [ ] 8.2 Classify exactly one justified result: `qmd_address_reuse_hypothesis_supported_experiment_only`, `specific_qmd_address_change_insufficient`, `behavior_changed_unresolved`, or `inconclusive`; preserve the precise scope and do not generalize to all QMD, launch-state, cache, allocator, or GPU-consumption behavior.
- [ ] 8.3 Retain immutable raw logs and a complete hardware record under `docs/testing/`, add the narrow causal finding under `docs/research/`, and update `docs/status/CAPABILITY_MATRIX.md` only with facts justified by the executed hardware evidence while keeping FG-2 `BLOCKED` and all prior negative evidence intact.
- [ ] 8.4 If the variant passes, record that it is not a production fix and stop with a requirement for a new, independently specified and run original unmodified image-chain acceptance test; do not create or run that acceptance change inside this apply workflow.
- [ ] 8.5 If the output changes without passing, require a smaller separately specified follow-up; if it retains the exact stale signature, record only that this two-slot consecutive-address intervention was insufficient.
- [ ] 8.6 Leave the experiment selectors disabled or remove the experimental slot-selection hunk after evidence capture, do not change FG-2 status, and do not start, design, or modify FG-3.
