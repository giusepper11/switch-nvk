## 1. Reconfirm authority and freeze the experiment baseline

- [x] 1.1 Record the current branch, HEAD, clean/dirty worktree, synchronized `origin/master`, recent commits, active changes, and archived predecessor; stop or revise this change if newer evidence supersedes the hypothesis.
- [x] 1.2 Re-read the canonical FG-2 specs plus complete root-flush, QMD-identity, QMD-address, and root-address hardware/research records; record `behavior_changed_unresolved`, the exact seed-42-retention signature, and FG-2 `BLOCKED`.
- [x] 1.3 Re-read the retained complete QMD payload lines and mechanically decode dword 7 bit 31 in reference, selected, and mapped copies; record bit 255=`0`, or stop this intervention if any authoritative payload has it set or inconsistent.
- [x] 1.4 Verify Mesa 25.0.7 plus `patches/switch-nvk-mesa-25.0.7.patch` remains the durable source, `mesa-25/` remains the disposable build representation, enumerate all intentional mirrors, and compare pristine/patch/reconstructed sources before editing.
- [x] 1.5 Reinspect the generated QMD v0.6 definition, NAK typed QMD builders, every `nak_qmd_info` caller, GM20B `MAXWELL_COMPUTE_B` selection, `nvk_cmd_upload_qmd`, mapped copy, and direct `PCAS` ownership path; stop on any source/layout mismatch.
- [x] 1.6 Freeze and hash the 64-iteration seed formula, root table contents/size/alignment, three shader sources, generated header, images, descriptors, sampler, pipeline layouts, layouts/barriers, command-buffer lifecycle, methods, submission/waits, cache maintenance, backing, readback, and exact oracle.
- [x] 1.7 Re-audit the retained root A/QMD X/QMD Y/root B allocation order, 4,608-byte footprint, 60,928-byte remainder, one-BO/NvMap identity, exact VAs/offsets, later upload consumers, wait/reset lifetime, and teardown.
- [x] 1.8 Run strict OpenSpec validation and repository policy checks before implementation; resolve planning defects without touching project code until the change remains valid.

## 2. Add typed QMD field construction in the durable source

- [x] 2.1 Update the durable patch first to add a zero-defaulted shader-constant-cache-invalidation boolean to `nak_qmd_info`, preserving ABI/layout expectations and ordinary false initialization at every caller.
- [x] 2.2 Add a typed QMD trait/builder setter using each generated layout's named `INVALIDATE_SHADER_CONSTANT_CACHE` field and invoke it during normal `fill_qmd` construction.
- [x] 2.3 Gate nonzero use to the strict experiment variant on `MAXWELL_COMPUTE_B`/QMD v0.6; reject an enabled variant on an incompatible compute class before dispatch and make no cross-generation behavior claim.
- [x] 2.4 Expose or reuse a generated-field-backed layout/decode helper so diagnostics derive the field position authoritatively and do not duplicate an unexplained raw dword mask in NVK.
- [x] 2.5 Set the new QMD input explicitly false in control and true in variant before the ordinary mapped copy; keep every ordinary and unrelated caller at false.
- [x] 2.6 Prove by source review that no direct `qmd[7]` mutation, permanent v0.6 default, other invalidation bit, QMD membar, or unrelated NAK behavior was introduced.
- [x] 2.7 Reconstruct `mesa-25/` from pristine Mesa 25.0.7 plus the updated durable patch and prove every touched NAK/NVK source exactly matches the intended build representation.

## 3. Implement strict selectors and the identical retained schedule

- [x] 3.1 Add exact build selectors `QMD_SHADER_CONSTANT_CACHE_CONTROL` and `QMD_SHADER_CONSTANT_CACHE_INVALIDATE`, matching `NVK_` runtime values, unique artifact tags, and startup reporting before Vulkan work.
- [x] 3.2 Reject nonzero/nonone values, both-new-selector combinations, and combinations with root-flush, QMD identity/cache, QMD-address, root-address, or any registered diagnostic intervention selector in both harness and runtime.
- [x] 3.3 Add one common experiment mode entered by either new arm that reuses the existing root-address-fresh scaffolding without enabling or composing the historical root-address selector.
- [x] 3.4 Reserve and copy primary root A, QMD X, QMD Y, and alternate root B in the retained order, sizes, alignments, one-BO backing, and lifetime in both arms.
- [x] 3.5 Select root A/QMD X on odd records and root B/QMD Y on even records in both arms, and verify actual previous/current VAs rather than trusting slot labels.
- [x] 3.6 Assert the retained exact VAs and offsets, 4,608-byte footprint, remaining capacity, no second BO/NvMap allocation, no later consumer displacement, and complete wait/reset/final cleanup boundaries.
- [x] 3.7 Keep absent/zero selectors on the existing ordinary one-root/one-QMD path with the QMD field false and no experimental allocation, copy, diagnostic, or dispatch behavior.
- [x] 3.8 Preserve every historical selector's existing behavior and add no compatibility alias that permits two interventions to run together.
- [x] 3.9 Review both arms to prove no third root, fourth QMD, reuse-distance change, backing change, allocator redesign, cache operation, synchronization, wait, submission, sleep, timing perturbation, or payload nonce was added.

## 4. Prove one-bit payload isolation and bounded correlation

- [x] 4.1 Generate the selected dispatched QMD once through normal typed construction, then generate a fixed-stack, nonmapped, nondispatched counterfactual with only the field boolean inverted in both arms.
- [x] 4.2 Decode selected and counterfactual field values from the authoritative layout and emit `qmd_shader_constant_cache_invalidate=0|1` plus `qmd_constant_cache_field_match=0|1`.
- [x] 4.3 Compare all 256 bytes for every selected/counterfactual pair and require the exclusive-or to contain exactly bit 255, with no hash-only equality decision.
- [x] 4.4 Preserve the existing root-cbuf layout decode and prove selected/counterfactual QMDs encode the same current root VA while corresponding cross-artifact iterations encode identical root VAs.
- [x] 4.5 Copy only the selected generated QMD to the selected mapped slot and require exact 256-byte generated-to-mapped equality for all 64 records.
- [x] 4.6 Correlate selected root/QMD maps and VAs, previous/current A/B and X/Y transitions, selected mapped QMD VA, direct `SEND_PCAS_A`/`PCAS` VA, `SEND_SIGNALING_PCAS_B`, ordering, and ownership for all 64 records.
- [x] 4.7 Extend fixed command-buffer-keyed state and aggregates for 63/63 root transitions, 63/63 QMD transitions, 64/64 root copies, QMD copies, one-bit comparisons, field/root decodes, dispatch matches, order checks, oracle results, and teardown/fault state.
- [x] 4.8 Emit bounded complete selected/counterfactual/mapped 64-dword payload records for iterations 1-5 and compact joined correlation for all 64 iterations without dynamic dispatch-path allocation or high-volume SD-only logging.
- [x] 4.9 Join application iteration/seed markers and exact pixel/checksum/oracle results to driver record keys and emit a compact final artifact aggregate.
- [x] 4.10 Implement and test hard-invalid paths for unexpected QMD bits, wrong field decode, root decode mismatch, source/mapped mismatch, address transition mismatch, QMD-to-`PCAS` mismatch, record-order failure, incomplete aggregate, and ambiguous fault state.
- [x] 4.11 Keep diagnostic language explicit that CPU-mapped equality and successful field construction do not prove GPU visibility or consumption.

## 5. Integrate paired artifacts and evidence scaffolding

- [x] 5.1 Add same-source control and variant NRO recipes with exact selectors, unique build/version/path markers, identical diagnostic limits, and the unchanged 64-iteration image-chain workload.
- [x] 5.2 Add host evidence scaffolding that records source authority, retained bit-zero precheck, typed field path, selector matrix, schedule/footprint/backing, full-payload proofs, workload hashes, build commands, toolchain, and artifact provenance.
- [x] 5.3 Add a paired hardware-record template with control authorization, bit decode, all 64 causal/oracle decisions, complete-log locations, notifier/error-info review, exact output signatures, and the four allowed classifications.
- [x] 5.4 Reconcile stale `MILESTONES.md` wording during apply so the completed root-address result remains `behavior_changed_unresolved` and this one-bit experiment is the current FG-2 discriminator without advancing FG-2.
- [x] 5.5 Update `docs/status/CAPABILITY_MATRIX.md` only to `IMPLEMENTED_UNPROVEN` after all host gates pass; preserve every prior `PROVEN_HW` negative/result row and keep the render-compute chain `BLOCKED`.
- [x] 5.6 Record public QMD definitions as `PUBLICLY_DOCUMENTED` and inspected Gallium/NAK construction as `SOURCE_CODE_EVIDENCE`; do not label them `PROVEN_HW` or infer undocumented cache-key, lifetime, backing, or general defect claims.
- [x] 5.7 Update reproduction/source-authority documentation only if implementation proves current `REPRODUCE.md` wording incomplete; do not normalize unrelated mirrors or historical files.

## 6. Complete host, build, static, and causal validation

- [x] 6.1 Dry-apply and apply the complete durable patch to pristine Mesa 25.0.7 and prove all reconstructed touched sources match `mesa-25/` byte-for-byte with no disposable-tree-only edit.
- [x] 6.2 Build the NVK archive, control NRO, and variant NRO with the authoritative toolchain and run all applicable compile, link, static, and repository checks.
- [x] 6.3 Test absent, zero, malformed, contradictory, prior-selector-plus-new-selector, wrong-compute-class, valid-control, and valid-variant configurations, including required pre-device failures and ordinary-path side-effect freedom.
- [x] 6.4 Run a deterministic payload test for all 64 corresponding schedule records proving control field zero, variant field one, `control XOR variant == bit 255`, identical root encodings/addresses, and equality of every other QMD bit.
- [x] 6.5 Prove selector-absent and explicit-control-false QMD generation matches the retained prechange payload exactly and that the typed false default changes no ordinary or unrelated caller.
- [x] 6.6 Prove A/B and X/Y selection, 63/63 transitions, exact retained VAs/offsets, 4,608-byte footprint, one BO/NvMap, unchanged later allocations, ownership, reset, and cleanup in both artifacts.
- [x] 6.7 Compare control/variant source and generated method semantics to prove bit 255 plus reporting labels are the only causal differences and existing `INVALIDATE_SKED_CACHES`, `SEND_PCAS_A`, `SEND_SIGNALING_PCAS_B`, cache maintenance, submission, and waits remain unchanged.
- [x] 6.8 Verify the seed formula, root bytes, vertex/fragment/compute sources, generated header, images, descriptors, layouts, barriers, command-buffer lifecycle, readback, and oracle identities match the retained root-address experiment and each other.
- [x] 6.9 Exercise every correlation failure path so no unexpected bit, copy/decode/address/order/aggregate mismatch, incomplete teardown, or ambiguous fault state can produce a valid classification.
- [x] 6.10 Record exact commands, toolchain/environment, durable-patch SHA256, source-equivalence results, selector matrix, payload comparison, allocation/backing checks, shader/generated-header hashes, and both artifact hashes in the host record.
- [x] 6.11 Run strict OpenSpec validation, repository policy checks, `git diff --check`, source/artifact causal review, and clean-worktree review; stop before hardware unless every gate is complete.

## 7. Freeze provenance and run the gated hardware control

- [ ] 7.1 Commit the exact experiment source without unrelated changes, rebuild both artifacts from that immutable revision, and reproduce the recorded commit, patch, artifact, shader, and generated-header hashes.
- [ ] 7.2 Perform the final source/artifact review proving both arms use identical A/B–X/Y allocation/selection and differ causally only in typed QMD v0.6 bit 255.
- [ ] 7.3 Run only the control for all 64 iterations on the intended real Switch/Tegra configuration and retain the complete combined unfiltered stream under `docs/testing/raw/`.
- [ ] 7.4 Read the entire control log before filtering and inspect startup, all records, teardown, warnings, timeout, notifier/error-info, and GPU fault state.
- [ ] 7.5 Require seed 5 and seed 42 exact, iterations 3-64 at the retained seed-42 pixel/checksum, 63/63 root and QMD transitions, bit 255 zero, 64/64 copies/one-bit counterfactual checks/root decodes/`PCAS` links/oracle records, complete order, and no unexplained fault.
- [ ] 7.6 If any control prerequisite fails, retain the exact deviation, classify `inconclusive`, forbid variant execution or interpretation, update only justified evidence/state wording, and stop without adding another intervention.
- [ ] 7.7 If hardware is unavailable after host completion, record `IMPLEMENTED_UNPROVEN` and `hardware-ready`, report no device contact, leave hardware tasks open, and do not archive the change as complete.

## 8. Run the authorized variant, classify, and retain evidence

- [ ] 8.1 Only after task 7.5 passes, run the unchanged variant for all 64 iterations on the same intended console/configuration and retain the complete combined unfiltered stream.
- [ ] 8.2 Read the entire variant log before filtering and inspect startup, all records, teardown, warnings, timeout, notifier/error-info, GPU faults, and observer effects.
- [ ] 8.3 Prove bit 255 one on every selected variant QMD, exact selected/counterfactual one-bit differences, exact selected-to-mapped copies, identical corresponding root encodings/schedule, 63/63 root and QMD transitions, 64/64 `PCAS` correlations, ordering, and complete fault evidence.
- [ ] 8.4 Compare all 64 pixels/checksums and independent oracle decisions with expected values and the valid control, retaining any changed signature exactly.
- [ ] 8.5 Produce a paired causal table covering commits/artifacts/selectors, iteration/seed, root contents/addresses, QMD full-payload decisions and addresses, direct dispatch, ordering, output/oracle, teardown, and notifier/error state.
- [ ] 8.6 Apply exactly one result: `qmd_shader_constant_cache_invalidate_supported_experiment_only`, `specific_qmd_shader_constant_cache_invalidate_insufficient`, `behavior_changed_unresolved`, or `inconclusive`; make no broader cache, backing, lifetime, or NVK claim.
- [ ] 8.7 Retain complete raw logs, a full hardware record, and a narrow research finding; update `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` only with the exact result and limits while keeping FG-2 `BLOCKED` and FG-3/FG-4 unchanged.
- [ ] 8.8 If exactly negative, name root reuse distance only as the next separate OpenSpec discriminator; if changed-but-failing, derive one smaller separate discriminator; do not implement either or combine another Gallium field here.
- [ ] 8.9 If supported experimentally, record that a separate production-remediation decision and separate ordinary image-chain 64/64 real-hardware acceptance change are mandatory before any promotion.
- [ ] 8.10 Leave the selector opt-in or roll back experiment-only call-site wiring after evidence capture, preserving retained evidence and ordinary false defaults.
- [ ] 8.11 Run final build/reconstruction/repository/diff/OpenSpec checks, sync the accepted delta only after evidence is reconciled, archive only through the explicit archive workflow when every applicable task is complete, and leave the worktree clean.
