## 1. Reconfirm authority and freeze the evidence boundary

- [x] 1.1 Reinspect branch, HEAD, `origin/master`, worktree cleanliness, recent commits, active/archived OpenSpec state, and overlap; use current synchronized `master` and stop rather than modify any completed archived experiment.
- [x] 1.2 Re-read the five retained FG-2 hardware records, complete raw streams, corresponding research findings, and accepted capabilities; verify recorded hashes/counts and confirm the current chain ends with `specific_qmd_shader_constant_cache_invalidate_insufficient`, root reuse distance next, and FG-2 `BLOCKED`.
- [x] 1.3 Record the public GM20B/QMD research boundary using its public-evidence vocabulary and preserve the absence of public evidence for root-state key, lifetime, VA-versus-physical identity, or replacement policy; do not convert source evidence into `PROVEN_HW`.
- [x] 1.4 Compare pristine Mesa 25.0.7, `patches/switch-nvk-mesa-25.0.7.patch`, and reconstructed `mesa-25/`; confirm the durable patch is authoritative, enumerate tracked harness/artifact sources and intentional mirrors, and stop on unexplained divergence.
- [x] 1.5 Freeze and hash the 64-iteration seed formula, seed-5/42/79 exact oracle values, root table size/alignment/contents, vertex/fragment/compute sources and generated header, images, descriptors, sampler/pipeline state, layouts/barriers, command-buffer lifecycle, GPU methods, dispatch, submit/waits, cache behavior, backing, readback, oracle, and teardown.
- [x] 1.6 Re-audit command-upload allocation ownership, `NVK_CMD_MEM_SIZE`, root/QMD sizes and alignments, pool/reset lifetime, one-BO/NvMap behavior, and the retained A/X/Y/B positions before changing the durable source.

## 2. Add strict paired experiment selection

- [x] 2.1 Add exact build selectors `ROOT_ADDRESS_REUSE_DISTANCE_CONTROL` and `ROOT_ADDRESS_REUSE_DISTANCE_VARIANT` plus matching exact runtime selectors and unambiguous pre-device arm/schedule markers.
- [x] 2.2 Reject non-binary values, both-new-selector combinations, and combinations with every historical root-cache, QMD-upload, QMD-address, root-address, QMD constant-cache, synchronization, or registered intervention selector before device work.
- [x] 2.3 Independently validate selector exclusivity in the application/build harness and driver so an artifact cannot silently enter a different experiment path.
- [x] 2.4 Keep absent and explicit-zero new selectors on the ordinary path with no seven-slot reservation, copy, diagnostic, schedule, QMD-field, or dispatch side effect; preserve every historical selector's existing behavior.
- [x] 2.5 Add one same-source paired artifact build script/recipes with unique names, versions, output paths, exact selector forwarding, and a shared 64-record diagnostic configuration.

## 3. Implement symmetric three-root/four-QMD scaffolding

- [x] 3.1 Extend the durable Mesa patch first so either new arm reserves root A, QMD X, QMD Y, root B, QMD Z, QMD W, and root C in the same candidate order and with existing root/QMD size and alignment primitives.
- [x] 3.2 Record and validate actual CPU mappings, offsets, alignments, distinct GPU VAs, remaining capacity, BO/NvMap identity, ownership, and lifetime for all seven slots without deriving Z/W/C addresses from arithmetic.
- [x] 3.3 Copy the complete current root source bytes to A, B, and C in the same order and perform the same complete source-to-all-mappings comparisons in both arms, including control's unused root C.
- [x] 3.4 Select the control root ring A/B/A/B... and variant root ring A/B/C/A/B/C... for exactly 64 records using one common scaffolding path whose only causal branch is selected root index.
- [x] 3.5 Select the identical QMD ring X/Y/Z/W/X... in both arms and copy only the normal generated selected QMD to its selected mapping before unchanged direct dispatch.
- [x] 3.6 Prove the actual candidate layout uses one command-upload BO/NvMap, preserves retained A/X/Y/B positions when reconstruction matches, occupies 7,168 bytes or records the actual validated footprint, and leaves the actual expected capacity without later upload-consumer displacement.
- [x] 3.7 Hard-stop causal review if a second backing allocation, capacity rollover, changed backing identity, later consumer displacement, or ownership/lifetime discrepancy occurs; revise OpenSpec before hardware rather than silently altering the allocation strategy.
- [x] 3.8 Preserve command-memory ownership through the existing submit/wait/reset and cleanup path; add no production allocator abstraction, 64-allocation list, or alternate backing policy.

## 4. Extend exact root/QMD/dispatch correlation

- [x] 4.1 Reuse authoritative generated QMD cbuf layout information to identify all root-address fields, decode the selected root VA, and construct the complete permitted-difference mask without hard-coded field guesses.
- [x] 4.2 Generate or reuse fixed-stack reference QMDs and require full 256-byte equality outside the root-address mask, exact selected generated-to-mapped equality, and unchanged QMD cache/membar values including `INVALIDATE_SHADER_CONSTANT_CACHE=FALSE` for every record.
- [x] 4.3 Correlate every selected mapped QMD VA with the exact direct `SEND_PCAS_A`/`PCAS` dispatch address and preserve existing method sequence and copy-before-dispatch ordering.
- [x] 4.4 Extend fixed command-buffer-keyed diagnostic state with bounded exact-VA history for three roots and four QMDs, calculating last-used distance, first-revisit flags, and first-revisit iterations from dispatched VAs rather than slot labels.
- [x] 4.5 Emit symmetric detailed records for at least iterations 1-5 and compact joined records for all 64 containing arm, record/iteration/seed, all root/QMD VAs, selected slots, previous/current VAs, reuse distances, first-revisit flags, root copies, QMD copies/mask/decode, direct dispatch, ordering, output oracle, and fault state.
- [x] 4.6 Emit aggregates for exact record count; control/variant root schedules; common QMD schedule; root/QMD first-revisit iterations and distances; all root/QMD copies; decodes; outside-mask comparisons; `PCAS` matches; ordering; oracle; one-backing footprint; and teardown/fault state.
- [x] 4.7 Join application iteration/seed markers and exact pixel/checksum results to driver record keys, retain the expected seed-5/42/79 values from the repository oracle, and state explicitly that CPU-mapped equality leaves GPU visibility `UNPROVEN`.
- [x] 4.8 Use fixed arrays/stack storage with no diagnostic allocation in the dispatch-critical path, identical observer work in both arms, and no unbounded or high-volume per-frame SD-only logging.

## 5. Integrate validation and evidence scaffolding

- [x] 5.1 Add deterministic host schedule tests for all 64 records proving control A/B, variant A/B/C, common X/Y/Z/W, control first root revisit at 3, variant first root revisit at 4, and first QMD revisit at 5 from exact selected VAs.
- [x] 5.2 Add allocation tests for seven-slot order, actual offsets, sizes, alignments, distinctness, retained A/X/Y/B positions, footprint/capacity, one-BO/NvMap identity, ownership through completion, cleanup, and absence of later upload displacement.
- [x] 5.3 Add complete root/QMD correlation tests and invalid mutations covering root-copy mismatch, QMD-copy mismatch, root-decode mismatch, outside-mask difference, unexpected alias/revisit, QMD-to-`PCAS` mismatch, ordering failure, missing/out-of-order record, aggregate mismatch, incomplete teardown, and ambiguous fault state.
- [x] 5.4 Add source/artifact semantic review checks proving both arms reserve/copy/compare all seven slots identically and differ only in selected root schedule plus the mechanically required QMD root encoding.
- [x] 5.5 Add host and hardware evidence templates with source authority, immutable revision, selectors, actual allocation/backing/lifetime, full schedule, reuse metrics, QMD correlation, artifact/patch/shader hashes, 64-record oracle, complete-log locations, notifier/fault review, decisive iterations 1-5 table, exact classifications, and non-promotion fields.
- [x] 5.6 Reconcile current documentation only as needed to identify this active discriminator; preserve every prior negative result and do not change FG-2, FG-3, or FG-4 status during implementation-only work.

## 6. Complete host/build/provenance gates

- [x] 6.1 Dry-apply and apply the complete durable patch to pristine Mesa 25.0.7, reconstruct `mesa-25/`, and prove every touched regenerated source is byte-identical to the build tree.
- [x] 6.2 Build the NVK archive and both NRO artifacts with the authoritative toolchain and run applicable compile, shell-syntax, static, repository-policy, and patch/source-consistency checks.
- [x] 6.3 Test absent, zero, valid control, valid variant, malformed, contradictory, both-new, and new-plus-every-historical-selector configurations, including required pre-device nonzero failures and ordinary-path unchanged behavior.
- [x] 6.4 Run the 64-record schedule/reuse simulations and prove QMD first reuse occurs strictly after both decisive root revisits; reject any result inferred only from slot labels.
- [x] 6.5 Validate all root source-to-A/B/C mappings, complete QMD comparisons and outside-root masks, root decodes, selected-QMD-to-`PCAS` addresses, ordering, aggregates, observer symmetry, and cleanup/lifetime paths.
- [x] 6.6 Review control versus variant shaders, generated headers, root contents, pipeline/resources, layouts/barriers, cache/membar bits and methods, command-buffer lifecycle, submission, waits, CPU cache behavior, synchronization, timing, backing, readback, and oracle; prove no prohibited intervention exists.
- [x] 6.7 Record exact build commands, toolchain/environment, actual allocation schedule and footprint, source-equivalence results, validation outputs, patch/source/shader/generated-header/NRO SHA256 values, and source/artifact causal review in a host record.
- [x] 6.8 Run strict OpenSpec validation, `git diff --check`, and all applicable host/static tests; if hardware is unavailable, stop at `IMPLEMENTED_UNPROVEN` and `hardware-ready` with an explicit no-device-contact statement.

## 7. Freeze provenance and run the gated hardware control

- [ ] 7.1 Commit the exact experiment source without unrelated changes, rebuild both artifacts from that immutable revision, reproduce all hashes, and perform a final source/artifact review before device contact.
- [ ] 7.2 Record device/model/configuration, build environment, immutable commit, selector state, actual allocation schedule/backing, patch/shader/artifact hashes, and the exact intended 64-record protocol.
- [ ] 7.3 Run only the control first on the intended real Switch/Tegra hardware for all 64 iterations and retain the complete unfiltered combined stream under `docs/testing/raw/`.
- [ ] 7.4 Read the entire control stream before filtering and verify A/X, B/Y, A/Z, B/W through iteration 4; root first revisit at 3; QMD first revisit at 5; exact root/QMD copies and decode/mask/dispatch/order decisions; all 64 oracle records; complete teardown; and no unexplained timeout, notifier, error-info event, warning, or GPU fault.
- [ ] 7.5 Require seed 5 and seed 42 exact, iteration 3 first failure retaining pixel `0xf5031a17` / checksum `0x0daf4ac5`, and the same retained signature through iteration 64; if any prerequisite deviates, retain `inconclusive`, do not run or interpret variant, and add no intervention.

## 8. Run, classify, retain, and prevent promotion

- [ ] 8.1 Only after task 7.5 passes, run the variant for all 64 iterations on the same intended configuration and retain its complete unfiltered combined stream.
- [ ] 8.2 Read the entire variant stream before filtering and verify A/X, B/Y, C/Z, A/W through iteration 4; root first revisit at 4; QMD first revisit at 5; exact allocation/backing, copies, decode/mask/dispatch/order, all 64 oracle records, teardown, and notifier/fault state.
- [ ] 8.3 Compare all 64 paired records and produce the required full causal table plus a compact iterations-1-5 table; verify whether iteration 3 consumes seed 79 `0xf0bf610f` / `0x1fcdf2e5` and whether iteration 4 first retains that state.
- [ ] 8.4 Apply exactly one classification: `root_address_reuse_distance_hypothesis_supported_experiment_only`, `specific_root_address_reuse_distance_change_insufficient`, `behavior_changed_unresolved`, or `inconclusive`; preserve the exact signature and do not derive an independent QMD-reuse claim from iteration 5 or later.
- [ ] 8.5 Retain complete raw logs, a full paired hardware record, and a narrow research finding with immutable provenance, causal prerequisites, exact onset/retained-seed behavior, all 64 records, and complete fault review.
- [ ] 8.6 Update `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` only with the executed result and its limits, preserve historical evidence, keep FG-2 `BLOCKED`, and leave FG-3/FG-4 unchanged.
- [ ] 8.7 If experimentally supported, state that remediation selection is now separately justified but do not implement it; if insufficient or changed, name at most one next smallest discriminator in a later OpenSpec change; if inconclusive, repair only the failed prerequisite.
- [ ] 8.8 Leave the experiment selectors disabled or remove the experiment-only seven-slot/schedule hunk after evidence capture, without promoting the ring into ordinary allocator policy or running ordinary-chain acceptance in this change.
- [ ] 8.9 Complete all repository/OpenSpec/policy/build-provenance checks, sync the accepted delta spec, archive only after every applicable implementation/evidence task is complete, validate strictly again, and leave the worktree clean.
