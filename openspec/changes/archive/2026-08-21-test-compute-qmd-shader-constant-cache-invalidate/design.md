## Context

See `proposal.md` for motivation and `specs/compute-qmd-shader-constant-cache-invalidate/spec.md` for the behavioral contract.

The clean synchronized starting point is `master` at `c147c8adc58d19450803676c177641e1a20cf9e1`. There are no active OpenSpec changes. The latest completed experiment is the archived `test-compute-root-address-reuse` change: its valid fresh-root arm alternated root VAs `0xc7f40000`/`0xc7f40a00` and QMD VAs `0xc7f40800`/`0xc7f40900`, passed seeds 5 and 42, then retained seed-42 output through iteration 64. Its classification is `behavior_changed_unresolved`; FG-2 remains `BLOCKED`.

The retained complete 256-byte QMD records in `docs/testing/raw/FG2_ROOT_ADDRESS_{CONTROL,FRESH}_NXLINK_2026-08-21.txt` show dwords 0-7 as zero in reference, selected, and mapped copies. The authoritative generated class definition maps `INVALIDATE_SHADER_CONSTANT_CACHE` to bit 255, so the existing field value is zero. This satisfies the mandatory no-hardware precheck; it does not prove GPU interpretation.

Public architectural evidence is deliberately bounded. Mesa's generated pre-Pascal QMD v0.6 definitions expose the field at bit 255 (`PUBLICLY_DOCUMENTED`). Mesa's older Nouveau Gallium path sets the field for inspected launches, while the inspected NAK v0.6 builder initializes it to zero and does not set it (`SOURCE_CODE_EVIDENCE`). NVK constructs a QMD from `nak_qmd_info`, copies the 256-byte result to GPU-visible command upload memory, and directly dispatches that mapped address. Existing `INVALIDATE_SKED_CACHES` methods are separate from this QMD field. None of those sources establishes an undocumented GM20B cache key, hidden-state lifetime, physical-backing behavior, or a general hardware requirement.

The durable source is Mesa 25.0.7 plus `patches/switch-nvk-mesa-25.0.7.patch`. The gitignored `mesa-25/` tree is the regenerated build representation; `REPRODUCE.md` confirms it is disposable. No tracked mirror of `nvk_cmd_dispatch.c`, NAK `qmd.rs`, or `nak.h` is authoritative. Tracked harness sources include `winsys/smoke/nvk_render_compute.c` and `winsys/build-nro.sh`. Apply must update the durable patch first, then reconstruct and verify the build tree.

The QMD ownership path is:

```text
nvk_cmd_upload_qmd
  -> assemble nak_qmd_info, including selected root cbuf VA
  -> nak_fill_qmd selects pre-Pascal Qmd0_6 on MAXWELL_COMPUTE_B
  -> typed QMD bitfield setters produce a 256-byte stack payload
  -> selected payload is copied to the selected command-upload QMD slot
  -> SEND_PCAS_A directly references that mapped QMD VA
```

The existing root-address experiment already reserves primary root, QMD X, QMD Y, and alternate root in that order within one 64-KiB command upload BO. Reusing that structure in both new arms avoids changing address reuse distance, allocation order, backing, or lifetime.

## Goals / Non-Goals

**Goals:**

- Express the tested bit as typed QMD construction input, not as an unexplained raw payload patch.
- Reuse the exact retained A/B root and X/Y QMD schedule in both arms, including its allocation order, addresses, ownership, and one-intervening-dispatch reuse distance.
- Make the selected QMD field the single independent semantic variable and prove that fact with full-payload comparisons before hardware.
- Preserve ordinary execution byte-for-byte at the QMD input level and side-effect-free at runtime when selectors are absent.
- Correlate every causal prerequisite needed to distinguish field support, exact negative, changed-but-failing, and evidence failure.
- Stop at planning in this workflow; implementation requires a later apply request.

**Non-Goals:**

- Defining a production constant-cache policy or changing ordinary NAK/NVK defaults.
- Testing all Gallium invalidation or membar settings, shader-data/texture/method-level cache invalidation, WFI, or serialization.
- Changing root/QMD reuse distance, BO/NvMap identity, allocator policy, synchronization, barriers, submissions, waits, shaders, descriptors, or image-chain semantics.
- Inferring the GM20B cache key, hidden launch-state lifetime, physical-backing semantics, a generic hardware bug, or general NVK incorrectness.
- Running ordinary image-chain acceptance, advancing FG-2, or beginning FG-3/FG-4 work.

## Decisions

### Add typed shader-constant-cache invalidation to NAK QMD construction

Extend `nak_qmd_info` with a boolean semantic input for shader-constant-cache invalidation. Extend the internal QMD trait/builders with a typed setter backed by each generated QMD layout's named `INVALIDATE_SHADER_CONSTANT_CACHE` field, and call it during `fill_qmd`. The new value defaults to false for all ordinary callers. The experiment sets it only when the strict variant selector is active and only after confirming the device selects pre-Pascal QMD v0.6 on `MAXWELL_COMPUTE_B`.

For this experiment, control explicitly supplies false and variant supplies true. This places the decision in the normal builder before the mapped copy and derives the bit position from generated class definitions. Host tests must prove that false reproduces the prior payload and true changes only bit 255 on GM20B.

Alternative: mutate `qmd[7] |= 0x80000000` in `nvk_cmd_upload_qmd`. Rejected because it bypasses typed generation, hard-codes layout knowledge in NVK, and weakens provenance. It is allowed only as a future OpenSpec revision if the typed path is proven infeasible; that revision must derive the location from the authoritative layout, mutate before the normal copy, remain experiment-gated, and retain the same full-payload proofs.

Alternative: permanently set the field in `Qmd0_6::new` because old Gallium does. Rejected because public source behavior is not hardware proof or a universal requirement, and it would change ordinary execution before causal evidence.

Alternative: combine all cache and membar bits used by older Gallium. Rejected because it destroys one-variable attribution.

### Reuse the prior fresh-root schedule identically in both arms

Add a new common experiment mode selected by either new arm. It uses the already implemented root-address-fresh allocation and selection mechanics without enabling the historical selector:

```text
allocation order per recording:
  primary root A: 0x800 bytes
  QMD X:          0x100 bytes
  QMD Y:          0x100 bytes
  alternate root B: 0x800 bytes

selection:
  odd iterations:  root A / QMD X
  even iterations: root B / QMD Y
```

Both arms therefore retain the 4,608-byte footprint, 60,928-byte remainder, one upload BO, same mappings/copies, 63/63 adjacent root transitions, 63/63 adjacent QMD transitions, and existing wait/reset lifetime. On the retained configuration, the exact VAs are A=`0xc7f40000`, B=`0xc7f40a00`, X=`0xc7f40800`, and Y=`0xc7f40900`. Host reconstruction must stop if these offsets, addresses, later consumers, or backing differ; implementation must not compensate with a new allocation strategy.

The control is not ordinary execution: it is an opt-in reproduction of the latest failure-producing A/B–X/Y schedule with bit 255 clear. The variant uses identical scaffolding with bit 255 set. Ordinary selector-absent behavior remains the existing one-root/one-QMD path.

Alternative: use the historical root-address-fresh selector alongside a new cache selector. Rejected because the new selectors must be mutually exclusive with prior interventions and because composing selectors makes malformed causal configurations easier.

Alternative: introduce three roots/four QMDs now. Rejected because reuse distance is explicitly deferred and would add allocation/address changes to the bit test.

### Use exact selectors and reject combinations before device work

Add build selectors `QMD_SHADER_CONSTANT_CACHE_CONTROL` and `QMD_SHADER_CONSTANT_CACHE_INVALIDATE`, forwarded as exact runtime values `NVK_QMD_SHADER_CONSTANT_CACHE_CONTROL=1` and `NVK_QMD_SHADER_CONSTANT_CACHE_INVALIDATE=1`. Each artifact reports its build tag, both parsed values, effective path, expected bit value, schedule, and diagnostic limit before Vulkan instance creation.

Only exact string/integer values zero and one are accepted. Exactly one new selector must be one for an experiment artifact. Both new selectors are incompatible with root-upload cache flush, QMD upload identity/cache, QMD address, root address, and any future registered intervention selector. The harness rejects invalid builds with exit 2; driver parsing also refuses to enable an ambiguous mode. Both layers are required so an artifact cannot silently run a different path.

When both selectors are absent or zero, the new `nak_qmd_info` field remains false, the special allocation schedule is not entered, no new diagnostics execute, and the existing QMD payload/copy/dispatch path remains unchanged.

### Generate a fixed-stack counterfactual payload in each arm

For every selected QMD, generate the dispatched payload once through normal `nak_fill_qmd`. Then, using fixed stack storage and an otherwise identical `nak_qmd_info`, generate a counterfactual payload with only the boolean inverted. Do not map or dispatch the counterfactual. Perform a complete 256-byte comparison and require:

1. selected field decodes to the arm's expected value;
2. counterfactual field decodes to the opposite value;
3. selected XOR counterfactual contains exactly bit 255;
4. both decode the same selected root VA;
5. the selected generated payload exactly equals the mapped payload;
6. the selected mapped QMD VA exactly equals direct `PCAS`;
7. the selected root/QMD VAs and ordering match the frozen schedule.

This symmetric local proof catches builder/layout mistakes before cross-artifact comparison. Host validation also captures and compares all 64 corresponding control/variant payloads, where `control XOR variant` must equal bit 255 and all addresses/ordering metadata must match. Runtime hashes are labels only; `memcmp` and bitwise masks decide validity.

Alternative: compare only dword 7. Rejected because it cannot detect unrelated differences elsewhere in the QMD.

Alternative: compare hashes. Rejected because hashes cannot attribute the difference to one field.

Alternative: generate no counterfactual and compare only logs from separate processes. Rejected because a symmetric in-process reference gives a stronger per-iteration false-positive check without changing GPU-visible memory or dispatch.

### Derive and decode bit 255 from authoritative layout definitions

The typed builder uses the generated named field. Diagnostic code must not trust the selector alone: it decodes the resulting QMD at bit 255 and reports `qmd_shader_constant_cache_invalidate=0|1` plus `qmd_constant_cache_field_match=1|0`. A small layout helper exposed from NAK or a generated-field-backed helper is preferred so NVK does not invent a second raw layout definition. Static tests still assert the public mapping bit 255 = dword 7 bit 31 for GM20B QMD v0.6.

The existing root-cbuf layout helper continues to decode the root VA. The one-bit mask and root-address masks are distinct: paired same-iteration payloads must have identical root encodings, while odd/even payloads within an arm legitimately differ at the root field. These comparisons must not be conflated.

### Extend existing bounded correlation without critical-path allocation

Extend fixed command-buffer-keyed diagnostic state rather than creating a new framework. Track 64 ordered records and aggregates for selector/path, seed, primary/alternate/selected root maps and VAs, primary/secondary/selected QMD maps and VAs, source/mapped/counterfactual payload decisions, field decode, root decode, previous/current transitions, copy/dispatch/order matches, output oracle, and teardown/fault state.

Emit detailed payload records for iterations 1-5, covering all 64 dwords for selected, counterfactual, and mapped QMDs. Emit compact joined correlation for every iteration and final aggregates for 63 root transitions, 63 QMD transitions, 64 exact QMD copies, 64 exact one-bit differences, 64 root decodes, 64 `PCAS` matches, 64 oracle results, order, and fault state. Use fixed arrays/stack buffers, no loop-time diagnostic allocation, and the existing combined stream; do not add high-volume SD-only logging.

The application retains the existing exact pixel/checksum oracle and joins results to driver record keys. Diagnostic text must say CPU-mapped equality is `UNPROVEN` for GPU visibility.

### Freeze the workload and prove source/artifact isolation before device contact

Record hashes and semantic review for the seed formula, root table shape and contents, three shader sources, generated header, images, descriptors, sampler, pipeline layouts, layouts/barriers, command-buffer reset/rerecord, `SEND_PCAS_A`, `SEND_SIGNALING_PCAS_B`, existing `INVALIDATE_SKED_CACHES`, submit/waits, CPU cache maintenance, BO/NvMap footprint, readback, and exact oracle. Compare these against the retained root-address experiment and between the new artifacts.

Host validation must reconstruct the durable patch from pristine Mesa 25.0.7, prove every touched reconstructed file matches the build tree, and demonstrate that no second BO/NvMap allocation or later upload displacement occurs. It must exercise absent, zero, malformed, contradictory, prior-selector-plus-new-selector, valid control, and valid variant configurations. A standalone payload test must construct corresponding QMDs for all 64 schedule records and prove exactly one bit differs.

Do not contact hardware until source is committed, artifacts are rebuilt from that immutable revision, hashes reproduce, and the control/variant source and artifact review shows no second intervention.

### Gate variant execution on the exact retained control signature

Run control first. It is valid only if:

```text
iteration 1 / seed 5:  0xfa47d33f / 0xb7d223e5 exact
iteration 2 / seed 42: 0xf5031a17 / 0x0daf4ac5 exact
iterations 3-64:       0xf5031a17 / 0x0daf4ac5 retained
root transitions:      63/63
QMD transitions:       63/63
bit 255:               0 on all selected payloads
```

All copies, counterfactual one-bit checks, root encodings, QMD-to-`PCAS` links, ordering, records, teardown, and notifier/error review must pass. Read the entire unfiltered control stream before extracting summaries. Any deviation is `inconclusive`; do not run or interpret variant.

Only then run variant on the same intended console/configuration with bit 255 equal to one. Read its complete stream, prove every causal prerequisite, compare all 64 outputs, and apply exactly one classification from the spec. Do not add another cache bit or schedule change in response to a negative or changed result.

### Keep experimental support separate from production remediation

Even a 64/64 variant result proves only sufficiency for this controlled schedule. Leave the selector opt-in or roll back its experiment-only call-site wiring after evidence retention. FG-2 stays `BLOCKED`. A separate change must decide whether and where a production behavior belongs; another separate change must run the ordinary image-chain artifact 64/64 on hardware before promotion.

If the variant is exactly negative, retain `specific_qmd_shader_constant_cache_invalidate_insufficient` and recommend root reuse distance as the next separate discriminator. If behavior changes but fails, retain the exact signature and choose a smaller new discriminator from that evidence. Do not automatically test the other Gallium fields.

### Reconcile current-state documents without rewriting evidence

During apply, update stale `MILESTONES.md` wording so the completed root-address result remains `behavior_changed_unresolved` and this QMD bit is the current discriminator. Add an `IMPLEMENTED_UNPROVEN` row only after host-ready implementation and a `PROVEN_HW` experiment-result row only after valid hardware evidence, using project status vocabulary. Update the blocked image-chain next gate without changing FG-2 status. Retain raw logs under `docs/testing/raw/`, the paired hardware record under `docs/testing/`, and the narrow finding under `docs/research/`. Sync and archive only after all applicable implementation/evidence tasks are complete.

## Risks / Trade-offs

- [Adding a generic NAK input accidentally changes ordinary QMDs] → Zero-initialize/default the boolean, compare selector-absent QMDs byte-for-byte with the retained payload, and gate nonzero use to validated GM20B QMD v0.6 experiment mode.
- [Typed support for multiple QMD versions broadens the claim] → Implement generated-field setters only as plumbing; enable and test the experiment solely for `MAXWELL_COMPUTE_B`/v0.6 and make no cross-generation behavior claim.
- [Counterfactual generation perturbs execution] → Use fixed stack storage, do not map or dispatch it, perform identical work in both arms, and reject the pair if control no longer reproduces the exact baseline.
- [The reused allocation helper subtly changes slot addresses or a later consumer] → Assert the retained 4,608-byte footprint, offsets, one-BO identity, remaining capacity, and later allocations before hardware; stop instead of compensating.
- [Root odd/even differences are mistaken for cache-bit differences] → Compare corresponding iterations across arms and use separate authoritative masks/decoders for root fields and bit 255.
- [The selected QMD says bit 255 changed but the dispatched bytes do not] → Require generated-to-mapped full equality and selected mapped VA-to-`PCAS` equality on every iteration.
- [Selector composition accidentally adds another intervention] → Reject malformed, contradictory, and historical-selector combinations in both build harness and runtime before device work.
- [Logging changes timing or allocation behavior] → Reuse fixed diagnostic state, bound full dumps to iterations 1-5, keep all 64 comparisons in process, and add no dynamic allocation or SD-only per-frame writes.
- [Control baseline differs because repository/build state drifted] → Treat it as `inconclusive`, retain the deviation, and prohibit variant interpretation rather than relaxing the oracle.
- [A pass is overgeneralized into a cache mechanism or production fix] → Use the experiment-only classification, keep FG-2 `BLOCKED`, and require separate remediation and ordinary-path acceptance changes.
- [A negative is overgeneralized] → State only that this specific QMD bit was insufficient for this schedule; defer reuse distance and every other intervention to separate changes.
- [Hardware is unavailable] → Stop at `IMPLEMENTED_UNPROVEN`/`hardware-ready` and report no device contact.

## Migration Plan

1. Reconfirm synchronized source, no superseding active change, durable-patch authority, retained A/B–X/Y evidence, and bit-255 zero precheck.
2. Update the durable patch with typed NAK QMD input/setter support and selector-gated NVK use; update tracked harness/build sources; reconstruct `mesa-25/` and prove equivalence.
3. Reuse the exact common root/QMD allocation schedule in both arms, add strict selector rejection, fixed counterfactual payload generation, full-payload proof, and bounded correlation without changing ordinary execution.
4. Run selector, payload, schedule, allocation/backing, ordinary-path, shader/resource, build, reconstruction, static, repository-policy, OpenSpec, and diff checks. Reconcile current-state wording only to `IMPLEMENTED_UNPROVEN` if implementation is host-ready.
5. Commit the exact experiment source, rebuild immutable control/variant artifacts, reproduce patch/artifact/shader hashes, and complete the final causal source/artifact review.
6. Run and fully inspect control. If any gate fails, retain `inconclusive` evidence and stop without variant execution.
7. Only after valid control authorization, run and fully inspect variant, retain paired evidence, apply one exact classification, and reconcile project documents without advancing FG-2.
8. Leave the experimental selector disabled or remove experiment-only call-site wiring after evidence capture. Sync accepted behavior and archive only after every applicable task is complete.

Rollback is removal or disabling of the experiment selectors and their call-site wiring plus regeneration of `mesa-25/` from the durable patch revision chosen after evidence retention. The ordinary false default remains unchanged; retained logs and findings are never deleted or rewritten.
