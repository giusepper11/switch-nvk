## Context

See `proposal.md` for motivation and `specs/compute-root-address-reuse-distance/spec.md` for the behavioral contract.

The synchronized starting point is clean `master` at `8e2b2882c31ce5be163fb3baa41a1aa6261969eb`, equal to `origin/master`, with no active OpenSpec changes. The latest accepted hardware result is `specific_qmd_shader_constant_cache_invalidate_insufficient`: setting only QMD v0.6 `INVALIDATE_SHADER_CONSTANT_CACHE` bit 255 left all 64 outputs identical to its valid two-root/two-QMD control. FG-2 remains `BLOCKED`, and the canonical milestone and capability matrix name root reuse distance as the next separate discriminator.

The retained root-address pair proves more than adjacent address change but less than global freshness. Its variant dispatched:

```text
record 1: root A 0xc7f40000 / QMD X 0xc7f40800 / seed 5  -> exact
record 2: root B 0xc7f40a00 / QMD Y 0xc7f40900 / seed 42 -> exact
record 3: root A 0xc7f40000 / QMD X 0xc7f40800 / seed 79 -> retained seed 42
records 4-64: A/B and X/Y alternate                         -> retained seed 42
```

All root/QMD copies, generated-layout root decodes, QMD equality outside root-address fields, direct-`PCAS` correlations, ordering, teardown, and fault prerequisites passed. Thus 63/63 adjacent root transitions did not mean globally unreused roots: each address returned after one intervening dispatch. The exact retained outputs needed for this change are seed 5 `0xfa47d33f` / `0xb7d223e5`, seed 42 `0xf5031a17` / `0x0daf4ac5`, and seed 79 `0xf0bf610f` / `0x1fcdf2e5`.

All nine retained raw streams for the preceding flush, QMD identity, QMD address, root address, and QMD constant-cache experiments still match their recorded SHA256 values and line counts. The current constant-cache validator accepts both 64-record streams, and its mutation suite rejects all tested invalid inputs. These checks confirm provenance and current evidence consistency; they are not new hardware evidence.

Public GM20B/Maxwell Compute B material distinguishes QMD/SKED state from shader-constant-cache controls and justified the completed bit-255 test. It provides no public evidence for the root-state key, hidden launch-state lifetime, VA-versus-physical identity, or replacement policy. This design therefore treats reuse distance as a behavioral discriminator only and does not name an undocumented cache architecture.

Mesa source authority remains pristine Mesa 25.0.7 plus `patches/switch-nvk-mesa-25.0.7.patch`. The gitignored `mesa-25/` tree is reconstructed and build-authoritative for a particular build but disposable; implementation must update the durable patch first and then prove regenerated touched files byte-identical. Tracked harness sources live under `winsys/`. Existing experiment infrastructure already provides strict selector parsing, command-buffer-keyed root/QMD correlation, generated-layout root decoding, full-payload comparison, direct-dispatch address checks, and paired artifact scripts.

Command-upload allocations are owned by the command-buffer upload path and remain live through submission completion, the existing wait/reset boundary, pool return, and teardown. The retained two-root/two-QMD experiment used one 64-KiB upload allocation, 4,608 bytes live, and 60,928 bytes remaining. The proposed three-root/four-QMD scaffold nominally uses 7,168 bytes and leaves 58,368 bytes, but host reconstruction must establish actual values and one-BO/NvMap identity before hardware.

## Goals / Non-Goals

**Goals:**

- Reuse the existing experiment machinery to create one symmetric pair whose only causal difference is an A/B versus A/B/C selected-root ring.
- Keep QMD addresses unreused until iteration 5 so the control's iteration-3 and variant's iteration-4 root revisits are independently interpretable.
- Preserve retained A/X/Y/B allocation positions when actual allocator evidence permits, while treating every new address as measured rather than assumed.
- Calculate reuse distance and first-revisit records from dispatched GPU VAs and retain enough exact evidence to reject false phase shifts.
- Make control validity, immutable provenance, full-log inspection, and four narrow classifications mechanically reviewable.
- Stop after planning in this workflow; implementation requires a separate apply request.

**Non-Goals:**

- Establishing a root cache key, cache associativity, lifetime, VA-versus-physical identity, TLB behavior, or generic GM20B/NVK defect.
- Making a three-root ring, four-QMD ring, or changed allocator policy part of ordinary execution.
- Retesting QMD invalidation, other cache/membar fields or methods, WFI, serialization, CPU flushes, physical backing, or timing.
- Changing shaders, resources, image synchronization, submission/wait/fence behavior, or starting FG-3/FG-4.
- Running the ordinary image-chain acceptance test or advancing FG-2 in this change.

## Decisions

### Use identical three-root/four-QMD scaffolding with only root selection differing

Both new arms enter one common experiment mode. Each reserves and initializes A, B, C, X, Y, Z, and W, copies the current root bytes into all three roots in the same order, performs the same reference/selected/mapped comparisons, and retains the same fixed diagnostic state. Selection is:

| Record | Control root/QMD | Variant root/QMD |
|---:|---|---|
| 1 | A / X | A / X |
| 2 | B / Y | B / Y |
| 3 | A / Z | C / Z |
| 4 | B / W | A / W |
| 5 | A / X | B / X |
| 6 | B / Y | C / Y |

The root rings repeat A/B and A/B/C respectively; the QMD ring repeats X/Y/Z/W in both. The primary causal window is records 1-4. QMD reuse beginning at record 5 remains observable but cannot support an independent QMD-reuse conclusion.

Alternative: give control only two roots. Rejected because omitted root-C reservation/copy/diagnostic work would make allocation positions and observer work asymmetric.

Alternative: use three QMDs. Rejected because variant record 4 would revisit QMD X at the exact first-root-revisit boundary.

Alternative: allocate 64 roots/QMDs. Rejected because it changes footprint, lifetime, and allocator behavior far beyond the smallest reuse-distance discriminator.

### Append new reservations after the retained A/X/Y/B layout

Use this candidate order in both arms:

```text
root A: 0x800 bytes, existing minimum-cbuf alignment
QMD X:  0x100 bytes, 0x100 alignment
QMD Y:  0x100 bytes, 0x100 alignment
root B: 0x800 bytes, existing minimum-cbuf alignment
QMD Z:  0x100 bytes, 0x100 alignment
QMD W:  0x100 bytes, 0x100 alignment
root C: 0x800 bytes, existing minimum-cbuf alignment
```

Appending Z/W/C preserves the retained A=`0xc7f40000`, X=`0xc7f40800`, Y=`0xc7f40900`, and B=`0xc7f40a00` positions when the reconstructed allocator remains identical. Do not predict Z/W/C VAs by arithmetic in evidence or logic. Record actual CPU mappings, offsets, alignments, GPU VAs, remaining capacity, BO/NvMap identity, and later upload consumers.

Allocation remains command-buffer-owned through the existing submit/wait/reset boundary. Both arms release through the unchanged pool/teardown path. A second BO/NvMap, a different backing identity between arms, a capacity rollover, or later upload displacement is a hard stop requiring OpenSpec revision before hardware.

Alternative: insert root C before QMD X. Rejected because it would deliberately displace every retained QMD/root-B address and weaken comparison with the conclusive pair.

Alternative: allocate a dedicated BO to guarantee addresses. Rejected because backing identity would become a new causal variable.

### Add a distinct strict selector family rather than composing old selectors

Add build selectors `ROOT_ADDRESS_REUSE_DISTANCE_CONTROL` and `ROOT_ADDRESS_REUSE_DISTANCE_VARIANT`, forwarded as exact runtime values `NVK_ROOT_ADDRESS_REUSE_DISTANCE_CONTROL` and `NVK_ROOT_ADDRESS_REUSE_DISTANCE_VARIANT`. The application reports parsed build values, effective arm, schedule, and diagnostic limit before Vulkan instance creation. Driver-side parsing independently rejects ambiguous enablement.

Only exact zero/one values are legal. Exactly one new selector is enabled in an experiment artifact. Both are incompatible with every existing root-upload cache, QMD identity/cache, QMD address, root address, QMD constant-cache, and future registered intervention selector. Absent/zero new selectors enter none of the new reservation, copy, diagnostic, or selection code.

Alternative: combine `ROOT_ADDRESS_FRESH=1` with a new three-root toggle. Rejected because selector composition obscures the effective experiment and permits historical behavior to drift underneath the pair.

### Extend current fixed diagnostic state with VA-history reuse calculations

Extend the existing command-buffer-keyed bounded state rather than introduce a new tracing framework. Keep fixed history for each distinct dispatched root and QMD GPU VA, storing last record number and whether it has ever been dispatched. For record `n`:

```text
distance = n - last_record_for_exact_va
first_revisit = previously_seen && never_previously_revisited_for_this_va
```

The first-dispatch case reports no prior distance. Aggregate fields report `root_first_revisit_iteration`, `qmd_first_revisit_iteration`, and each arm's scheduled/observed distance. Slot names remain labels only. The history capacity is bounded by three roots and four QMDs and does not allocate in the dispatch-critical path.

Each of 64 joined records contains arm, record/iteration/seed, all reserved VAs, selected slot, previous/selected VA, last-use distance and first-revisit flags, root source-to-all-map equality, QMD source/map/reference decisions, decoded root, outside-root-mask equality, selected QMD and direct `PCAS`, ordering, pixel/checksum/oracle, and fault state. Detailed payload/address output covers at least records 1-5; compact decisions cover all 64. Teardown records prove the existing queue-wait, command-buffer lifetime, cleanup begin, and cleanup completion boundaries.

Alternative: infer distance from ring indices. Rejected because allocator recycling or unexpected aliasing could make labels disagree with actual dispatched GPU VAs.

Alternative: log only records 1-5. Rejected because the normal 64-record run is needed to expose later instability and complete lifetime/teardown behavior.

### Reuse authoritative QMD layout decode and complete comparisons

For each record, generate the selected QMD through the normal QMD path with the selected root VA. Use the generated-layout cbuf descriptor information already exposed by the root-address experiment to decode the selected root address and construct the allowed root-address difference mask. Generate or reuse fixed-stack reference QMDs as needed so full 256-byte comparisons prove equality outside that mask. Copy only the selected QMD to the selected X/Y/Z/W mapping and dispatch only that mapping.

Hashes are compact identities, never equality evidence. Full in-process comparisons decide root copy, QMD copy, and outside-mask validity. CPU-mapped equality remains explicitly `UNPROVEN` for GPU visibility. `INVALIDATE_SHADER_CONSTANT_CACHE` and every other QMD cache/membar field retain the ordinary false/current values in both arms.

Alternative: compare only decoded root fields or hashes. Rejected because either can miss an unrelated QMD difference that causes the observed phase shift.

### Freeze the retained workload and compare artifacts semantically

Freeze and hash the seed formula; three shader sources and generated header; root table size, alignment, and semantic contents; images, formats, descriptors, samplers, pipeline state; layouts/barriers; command-buffer reset/rerecord; GPU method sequence including `PCAS`, `SEND_PCAS_A`, `SEND_SIGNALING_PCAS_B`, and existing SKED initialization; dispatch dimensions; submit/wait; CPU cache behavior; backing; readback; oracle; and teardown.

The control and variant are not expected to be byte-identical binaries or command streams because their selectors and selected root-address operands differ. Review instead proves identical frozen semantics and scaffolding, with the root schedule and mechanically dependent QMD root fields as the only causal difference.

Host validation must include:

1. strict OpenSpec and repository policy validation plus `git diff --check`;
2. durable-patch dry reconstruction from pristine Mesa 25.0.7 and byte equivalence of touched regenerated sources;
3. absent, zero, valid, malformed, contradictory, and every historical-selector combination test;
4. ordinary-path side-effect-freedom checks;
5. exact seven-slot reservation, alignment, capacity, one-backing, ownership/lifetime, cleanup, and later-consumer checks;
6. 64-record A/B and A/B/C root-ring simulations plus identical X/Y/Z/W QMD-ring simulation from actual selected addresses;
7. proof that first QMD reuse occurs after both decisive root revisits;
8. complete root copy, QMD copy, root decode, outside-mask, direct-`PCAS`, order, aggregate, and mutation/failure-path tests;
9. shader/resource/layout/barrier/cache/sync/timing identity review;
10. same-revision artifact builds with exact patch, source, shader, generated-header, and NRO SHA256 provenance.

### Gate hardware with the reconstructed control signature

Commit the exact experiment source before interpretive hardware execution and rebuild both NROs from that immutable revision. Run control first, capture the complete combined stream, and read it before filtering. It authorizes variant only if the following decisive table and all 64 causal records pass:

| Record | Selection | Expected behavior |
|---:|---|---|
| 1 | A / X | seed 5 exact: `0xfa47d33f` / `0xb7d223e5` |
| 2 | B / Y | seed 42 exact: `0xf5031a17` / `0x0daf4ac5` |
| 3 | A / Z | first root revisit, QMD new, retain seed 42 |
| 4 | B / W | root revisited, QMD new, retain seed 42 |

An exact schedule, no QMD reuse before record 5, all copies/decode/mask/dispatch/order checks, 64 complete records, teardown, and no unexplained timeout/notifier/error-info/GPU fault are mandatory. A changed control is retained as useful evidence but classified `inconclusive`; it does not authorize adding or interpreting another hypothesis.

After a valid control, run variant on the same intended configuration, capture and inspect its complete stream, then require:

| Record | Selection | Supported-hypothesis signature |
|---:|---|---|
| 1 | A / X | seed 5 exact |
| 2 | B / Y | seed 42 exact |
| 3 | C / Z | seed 79 exact: `0xf0bf610f` / `0x1fcdf2e5` |
| 4 | A / W | first root revisit, QMD new, retain seed 79 |

Complete all 64 records even after the decisive boundary. Produce a full paired evidence table and a compact records-1-5 comparison. Do not interpret record 5 or later as an independent QMD-address experiment.

### Apply exactly one classification and keep remediation separate

`root_address_reuse_distance_hypothesis_supported_experiment_only` requires the exact one-record phase shift, immediate-predecessor retention, no QMD reuse at either decisive boundary, all causal evidence, and no unexplained fault. It says only that onset tracked first root revisit in this pair.

`specific_root_address_reuse_distance_change_insufficient` requires a valid pair whose variant does not shift onset as predicted and retains the relevant known stale signature without another material change. It rejects only the tested reuse-distance increase as sufficient.

`behavior_changed_unresolved` retains any causally valid signature that is neither the exact phase shift nor the narrow unchanged-stale result. `inconclusive` covers any failed control, schedule, allocation/backing, copy, decode, mask, dispatch, ordering, symmetry, provenance, log, teardown, or fault prerequisite.

Every outcome leaves FG-2 `BLOCKED`. Even the supported outcome cannot establish a production allocator policy. It permits a later separately reviewed remediation decision, followed by a separately specified ordinary render-to-sampled-to-compute-to-storage 64/64 hardware acceptance run.

## Risks / Trade-offs

- [Common extra reservations change the control signature] → Require the exact four-QMD control boundary and make control validity gate all variant execution and interpretation.
- [Appending Z/W/C displaces a later upload consumer] → Prove every actual offset and later consumer during host reconstruction; stop rather than accept a silent displacement.
- [The nominal 7,168-byte calculation hides allocation rollover] → Check actual remaining capacity and BO/NvMap identity; a second backing allocation is a hard review stop.
- [Allocator aliasing defeats the intended ring] → Calculate reuse from exact dispatched GPU VAs and require the expected first-revisit records; never infer from slot labels.
- [QMD reuse contaminates the decisive boundary] → Use four QMD slots and reject any observed QMD revisit through record 4.
- [Root copies or QMD payloads differ beyond root encoding] → Use complete byte comparisons, generated-layout masks, and mutation tests; classify any mismatch `inconclusive`.
- [Symmetric control still has an observer effect] → Run control first and require the exact retained seed-42 boundary before variant authorization.
- [Detailed history/logging perturbs timing] → Use fixed bounded state, identical observer work, detailed records only for the decisive window, and no dynamic or high-volume SD-only tracing.
- [A supported phase shift is overgeneralized] → Use the experiment-only classification, explicitly leave root key/lifetime/backing unknown, and keep FG-2 `BLOCKED`.
- [A negative result is overgeneralized] → Reject only this one-step reuse-distance increase and select any next discriminator in a later OpenSpec change.

## Migration Plan

1. Reconfirm synchronized source, no overlapping active change, retained evidence hashes/log validity, source authority, and the frozen FG-2 workload.
2. Update the durable Mesa patch and tracked harness sources with strict selectors, common seven-slot reservation, A/B versus A/B/C selection, X/Y/Z/W selection, exact QMD correlation, and fixed reuse-history diagnostics.
3. Reconstruct `mesa-25/`; run selector, schedule, allocation/backing/lifetime, mutation, ordinary-path, QMD/root correlation, build, provenance, repository-policy, diff, and strict OpenSpec checks.
4. Commit the exact experiment source, rebuild both artifacts, reproduce all hashes, and perform final causal source/artifact review.
5. Run and fully inspect the 64-record control. Stop with retained `inconclusive` evidence if any gate fails.
6. Only after valid control, run and fully inspect the 64-record variant; retain complete raw logs, the full hardware record, compact decisive table, and one exact classification.
7. Reconcile the research finding, milestone, capability matrix, and accepted specs without changing FG-2; leave selectors disabled or remove the experiment-only path after evidence capture; archive only after all applicable tasks and validation pass.

Rollback removes or disables the two new selectors and the experiment-only seven-slot/schedule/diagnostic hunk after retaining evidence. It does not change ordinary allocation behavior, rewrite historical results, promote FG-2, or begin FG-3/FG-4 work.
