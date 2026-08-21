## Context

See `proposal.md` for motivation and `specs/compute-root-address-reuse/spec.md` for the behavioral contract.

The clean starting point is synchronized `master` at `82b7c49b895cdffe8542c9ceec3e39f0e4120af6`; `c04e1ae` retains the completed QMD-address hardware result and `82b7c49` archives that change. The paired QMD-address run proved 63/63 adjacent QMD-address transitions, 64/64 exact QMD copies and direct-`PCAS` correlations, iteration 1 exact output, and the established stale pixel/checksum for iterations 2-64 without a reported GPU fault. QMD address is therefore a held-constant control in this change, not the tested variable.

The build-authoritative Mesa representation is reconstructed from Mesa 25.0.7 plus `patches/switch-nvk-mesa-25.0.7.patch`; that durable patch must change first. Gitignored `mesa-25/` is the regenerated build tree. There is no tracked mirror of `nvk_cmd_dispatch.c`. The tracked artifact-side sources are `winsys/smoke/nvk_render_compute.c`, `winsys/build-nro.sh`, and the generated shader container whose identity must remain fixed.

In the current direct-compute path, `nvk_cmd_upload_qmd` allocates and copies a 2,048-byte root table first, encodes its GPU VA in the naturally generated 256-byte QMD, copies the QMD to command upload memory, and dispatches that QMD through `SEND_PCAS_A`. `NVK_CMD_MEM_SIZE` is 64 KiB. The retained reset/rerecord loop waits with `vkQueueWaitIdle` before reset; command upload memory stays owned through execution and is returned to the pool by the existing reset lifecycle. CPU-mapped equality is observable but does not prove Tegra visibility or consumption.

`MILESTONES.md` still describes the QMD-address experiment as pending, while `docs/status/CAPABILITY_MATRIX.md` already records `specific_qmd_address_change_insufficient`. Apply must reconcile that mismatch without rewriting immutable historical evidence.

## Goals / Non-Goals

**Goals:**

- Make compute-root GPU VA freshness the single independent semantic variable in a same-source paired experiment.
- Preserve the previously proven fresh-QMD two-slot behavior in both arms, including its primary/secondary allocation positions and alternating selection.
- Prove every root content, root address, QMD encoding, QMD mapping, direct-dispatch address, output, order, and fault prerequisite needed for a narrow causal result.
- Bound observer effects and resource growth to the 64-iteration diagnostic artifact, with ordinary execution unchanged when selectors are absent.
- Produce immutable host and hardware evidence that supports exactly one specified outcome while leaving FG-2 `BLOCKED`.

**Non-Goals:**

- A production allocator, root-upload, cache, descriptor, or synchronization policy.
- A general claim about constant buffers, caches, address translation, upload allocation, launch-state caching, shader execution, or sampled-image freshness.
- Any shader, image, descriptor, layout, barrier, command-buffer lifecycle, submission, wait, readback, timing, or oracle change.
- Removing waits, changing asynchronous submission or native fence semantics, starting FG-3, or running the original unmodified image-chain acceptance test in this change.

## Decisions

### Use strict paired root-address selectors and leave all existing paths intact

Add build selectors `ROOT_ADDRESS_CONTROL` and `ROOT_ADDRESS_FRESH`, forwarded only as exact-value runtime selectors `NVK_ROOT_ADDRESS_CONTROL=1` and `NVK_ROOT_ADDRESS_FRESH=1`. They are mutually exclusive and incompatible with root-cache, QMD-identity/cache, and earlier QMD-address experiment selectors. Either new selector enables the same common two-root/two-QMD reservation and diagnostic structure. Control always selects the ordinary primary root; variant selects primary for odd records and alternate for even records. Both select alternating primary/secondary QMD slots.

Absent or zero selectors retain the current one-root/one-QMD ordinary path. Malformed or contradictory build configurations fail with exit 2, and runtime logic enables a root experiment only when exactly one selector is `1` and excluded selectors are inactive. Existing historical selectors remain available and behaviorally unchanged.

This keeps the experiment opt-in and prevents accidental combination with another intervention. Reusing the earlier QMD-fresh selector directly was rejected because it cannot express “fresh QMD in both arms” while independently selecting reused versus fresh root behavior. Generalizing the command allocator was rejected because no production consumer requires that policy.

### Reserve slots in an order that preserves the established QMD addresses

Under either root-address selector, reserve memory in this exact order for each command recording:

```text
ordinary primary root:  0x800 bytes, existing minimum-cbuf alignment
primary QMD:            0x100 bytes, 0x100 alignment
secondary QMD:          0x100 bytes, 0x100 alignment
alternate root:         0x800 bytes, existing minimum-cbuf alignment
```

With the validated baseline offsets this occupies `0x1200`/4,608 bytes of the 64-KiB upload BO and leaves `0xee00`/60,928 bytes. The experiment therefore adds exactly one 2,048-byte root reservation per recording beyond the established fresh-QMD control and is not expected to allocate another BO. Host validation must prove the actual sizes, alignments, returned mappings, distinct VAs, offsets, capacity, and absence of later upload-allocation displacement before hardware. Any mismatch is a hard stop, not permission to adopt a broader allocation strategy.

QMD slots are reserved before the alternate root so their allocation order and expected positions remain identical to the completed fresh-QMD experiment. QMD generation occurs after root selection; reserving QMD storage does not require QMD bytes to exist yet. Both roots are copied from the same current source bytes in the same order in both artifacts and validated with complete comparison before selection. Control selects primary on every record. Variant selects primary on record 1 and alternates alternate/primary through record 64, which yields 63 adjacent address changes if the validated slots remain distinct.

Alternative: reserve the alternate root immediately after the primary root. Rejected because it shifts both QMD slots and weakens continuity with the valid fresh-QMD baseline. Alternative: retain 64 unique root allocations or BOs. Rejected because adjacent freshness requires only two distinct selected VAs, 64 roots exceed one 64-KiB BO, and extra BO lifetime/allocation ordering would add avoidable observer effects. Alternative: rely on pool behavior to return a different VA. Rejected because freshness must be selected and proven, not inferred from allocator behavior.

### Keep lifetime bounded and make recycling irrelevant to the adjacent-address claim

Both root mappings and both QMD mappings are command-buffer-owned through the existing submit and `vkQueueWaitIdle`. Existing reset then returns command memory to the pool, and final command-buffer/pool destruction performs cleanup. Previous-iteration allocations do not need to remain live: both deterministic root slots are reserved again before selection, and the selected slot alternates so the current GPU VA is proven different from the immediately preceding dispatched root VA. A slot may recur after one intervening dispatch, exactly matching the stated hypothesis boundary.

The maximum incremental live reservation is 2,048 bytes in the existing upload BO; it is bounded to one in-flight, wait-completed recording in this 64-iteration smoke test. If actual allocator behavior creates an additional BO, recycles both reservations to one VA, changes QMD slots, or disturbs a later upload consumer, host validation stops the change before hardware. This mechanism is experiment scaffolding only and must remain selector-gated or be rolled back after evidence capture.

### Treat QMD root-pointer changes as dependent and prove the complete diff

The selected root VA is placed in the ordinary `nak_qmd_info` root-cbuf entry and the production `nak_fill_qmd` path generates the selected QMD normally. For comparison, use fixed stack storage to generate or canonicalize a reference QMD for the primary root with otherwise identical inputs in both artifacts. Derive the permitted bit ranges from `nak_get_qmd_cbuf_desc_layout`, decode low/high address fields for both payloads, and perform full 256-byte comparisons with these gates:

1. selected and reference QMDs decode to their respective expected root VAs;
2. every bit outside the union of identified root-address ranges is exactly equal;
3. normal selected QMD bytes exactly equal the selected mapped copy;
4. the selected QMD VA exactly equals the direct `PCAS` dispatch VA;
5. both arms retain the same QMD slot allocation and odd/even selection sequence.

The variant is not required to have byte-identical QMDs to the control when the root VA differs. “Only expected root fields differ” is established by the complete bit-mask comparison, not by hashes or an assumed dword location. Hashes remain compact labels. Unexpected changed bits, decode failure, or a mapping/dispatch mismatch makes the result `inconclusive`.

Alternative: compare hashes only. Rejected because a hash cannot attribute differences to an encoded field. Alternative: ignore QMD differences as mechanically necessary. Rejected because that would leave a second uncontrolled variable. Alternative: mutate a completed QMD address field directly. Rejected because the selected payload must come from normal QMD generation.

### Extend fixed correlation state without loop-time diagnostic allocation

Extend the existing command-buffer-keyed bounded diagnostic state for exactly 64 ordered records. Track previous selected root/QMD VAs, root source and both mapped contents, selected/reference QMD bytes, exact comparisons and hashes, decoded addresses and allowed-difference counts, dispatch matches, transition totals, and ordering. Reset state on command-buffer identity or ordering failure rather than carrying a misleading correlation.

Emit one compact record per iteration that joins iteration/seed, selector/path, primary/alternate/previous/selected root VAs, root source-to-both-mapping equality, root freshness, primary/secondary/previous/selected QMD VAs, QMD freshness, source-to-map equality, decoded root address, outside-mask equality, direct `PCAS` match, ordering, output pixel/checksum/oracle, and fault state. Retain bounded complete payload records for the primary-root and alternate-root cases needed for cross-artifact review; all 64 decisions still use complete in-process comparison. Reuse fixed buffers and the existing combined stream, with no loop-time diagnostic allocation or unbounded SD logging.

The application marker and result records remain keyed by the existing 1-based record/iteration and seed. Aggregate records require control `root_transitions=0/63`, variant `root_transitions=63/63`, both `qmd_transitions=63/63`, exact root/QMD copy counts, outside-mask equality, 64/64 dispatch matches, 64 oracle decisions, complete ordering, and fault state.

### Freeze all non-address semantics and use the control as an observer gate

Before implementation, record hashes and source review for the seed formula, three shaders and generated header, two 16x16 RGBA8 images, descriptors, shared pipeline layout, layouts/barriers, one primary command buffer, reset/rerecord, direct dispatch, one submit, existing queue-idle wait, readback, and 256-pixel oracle. The experiment adds no flush, invalidation, barrier, wait, submission, nonce, artificial mutation, sleep, or timing perturbation.

Both artifacts execute the identical extra allocation, copy, reference-QMD comparison, and reporting structure. Run the control first. It is valid only if it proves ordinary root reuse, fresh QMD selection and dispatch, and the exact retained iteration-1 pass plus iterations-2-64 stale signature with no unexplained fault. A changed baseline is an observer effect and makes the experiment `inconclusive`; do not authorize the variant.

### Build and run immutable artifacts in a strict sequence

Update the durable patch first, regenerate `mesa-25/`, and prove pristine Mesa 25.0.7 plus the complete patch reconstructs every touched build source. Build NVK and paired NROs from one committed experiment revision. Record exact control/variant NRO SHA256 values, durable patch hash, source revision, build commands, selector/path tags, and vertex/fragment/compute source plus generated-header hashes. Review source and artifact differences before device contact.

Run the control for 64 iterations on the intended real Tegra configuration and read its complete unfiltered combined stream before filtering. Only a valid control authorizes the variant on the same configuration. Read the complete variant stream, then produce the paired correlation/evidence table and apply one exact classification from the spec. Host/static/build or emulator results remain `IMPLEMENTED_UNPROVEN` and cannot classify GPU behavior.

### Reconcile documentation without promoting the experiment

During apply, update `MILESTONES.md` to record the completed QMD-address result `specific_qmd_address_change_insufficient` and name compute-root-address reuse as the active discriminator. Tighten the existing capability-matrix FG-2 wording to the same boundary while preserving all earlier `PROVEN_HW` negative-result rows and keeping FG-2 `BLOCKED`.

After hardware, retain complete raw logs under `docs/testing/raw/`, a full hardware record under `docs/testing/`, and the narrow finding under `docs/research/`. A supported result permits only a separately reviewed ordinary-path remediation decision and then a separate original unmodified image-chain 64/64 hardware acceptance run. An insufficient result records only that the tested adjacent root-address change was insufficient. A changed-but-failing result retains its exact signature and proposes one smaller boundary. An inconclusive result fixes only the failed causal/evidence prerequisite. None starts FG-3.

## Risks / Trade-offs

- [The extra root reservation changes the control] → Give both arms identical allocation/copy work, place the extra root after the two established QMD slots, and reject the experiment unless the control reproduces the exact baseline.
- [A later command upload is displaced] → Audit all allocations after compute dispatch and prove host-side offsets/BO identity; stop before hardware if any unrelated consumer moves.
- [Two-slot alternation is mistaken for globally fresh allocation identity] → Report that freshness is only relative to the immediately preceding dispatch and that a slot may recur after one intervening dispatch.
- [Pool recycling collapses or changes the intended address sequence] → Instrument actual previous/current selected GPU VAs for every record and require exact aggregate counts; do not infer freshness from slot labels.
- [An additional upload BO changes more than root placement] → Require the validated 4,608-byte footprint in the existing 64-KiB BO and hard-stop if allocation crosses a BO boundary.
- [Root copies differ or mapped equality is overclaimed] → Compare complete source/primary/alternate bytes, classify any mismatch as inconclusive, and state that mapped equality does not prove GPU visibility.
- [QMD changes are attributed too broadly] → Decode layout-provided root fields and require full-payload equality outside their bit mask plus exact selected-to-mapped and selected-to-dispatch correlation.
- [Reference QMD generation perturbs behavior] → Use fixed stack storage and identical calls in both arms; the valid control is the observer-effect gate.
- [Diagnostic volume changes timing] → Use fixed state, compact per-iteration records, bounded complete payload dumps, no dynamic diagnostic allocation, and no high-volume SD-only logging.
- [A passing experiment is promoted as a fix] → Keep selectors opt-in or roll them back, leave FG-2 `BLOCKED`, and require a separate ordinary-path decision and original-chain hardware acceptance change.
- [A failing experiment is generalized] → Use only the specified classification and name only the next boundary supported by complete evidence.

## Migration Plan

1. Reconfirm synchronized source, durable-patch authority, reconstructed-tree equivalence, retained QMD evidence, and the frozen 64-iteration workload.
2. Implement strict selectors, common reservation order, paired root copies/selection, fresh QMD selection in both arms, dependent QMD-diff proof, and bounded correlation in the durable patch and tracked artifact sources only.
3. Regenerate `mesa-25/`, reconcile current milestone/matrix wording, and run selector, allocator, offset/capacity, reconstruction, build, static-correlation, shader-identity, artifact-provenance, repository-policy, OpenSpec, and diff checks.
4. Commit the exact experiment source, rebuild the pair, record hashes, and review that the root selection plus dependent QMD encoding is the sole semantic difference.
5. Run and fully inspect the control. Stop and retain `inconclusive` evidence if any baseline or causal prerequisite fails.
6. Only after a valid control, run and fully inspect the variant; retain the paired raw logs, hardware record, research finding, exact classification, and source-of-truth result wording.
7. Leave the experimental path disabled or remove its allocation-selection hunk after evidence capture. Sync and archive only when implementation, evidence reconciliation, and all tasks are complete; validate strictly again and leave the worktree clean.

Rollback is selector removal or disabling plus removal of the experiment-only durable-patch hunk after evidence retention. It does not alter the ordinary allocation path, canonical historical evidence, or FG-2/FG-3 status.
