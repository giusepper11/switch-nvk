## Context

See `proposal.md` for motivation and `specs/compute-root-upload-cache-flush/spec.md` for the experiment contract. The retained real-hardware diagnostic shows iteration 2 seed 42 in the compute root source and in the CPU mapping backing root GPU VA `0xc7f40000`; NAK encodes that address as root cbuf 0, and direct dispatch uses the correlated reused QMD VA `0xc7f40800`. Output nevertheless matches the earlier deterministic stale signature. This proves current CPU/QMD state only and leaves CPU-cache visibility, GPU constant-buffer cache state, address-translation visibility, and constant-buffer consumption unresolved.

The direct path in Mesa 25.0.7 is:

```text
nvk_CmdDispatchBase
  -> nvk_flush_compute_state
  -> nvk_cmd_flush_cs_qmd
  -> nvk_cmd_upload_qmd
       -> nvk_cmd_buffer_upload_alloc(root)
       -> memcpy(root_desc_map, root, sizeof(*root))
       -> construct QMD with root_desc_addr
       -> nvk_cmd_buffer_upload_data(qmd)
  -> SEND_PCAS_A(qmd_addr >> 8)
```

Command-buffer reset frees the owned upload memory and later allocation reuses the same CPU mapping/GPU VA in the observed artifact. The build consumes the ignored extracted `mesa-25/` tree, while `patches/switch-nvk-mesa-25.0.7.patch` is the durable reproducible source. The previously confirmed source review found no intentional tracked mirror for the touched NVK command-buffer files; implementation must recheck that fact and preserve unrelated extracted-tree edits.

## Goals / Non-Goals

**Goals:**

- Test one falsifiable hypothesis: whether cleaning/flushing the CPU-written reused compute-root mapping before QMD construction changes the stale shader result on real GM20B hardware.
- Keep a same-revision disabled control and an enabled variant comparable at source, shader, GPU-command, resource, synchronization, and oracle layers.
- Make selector state, address reuse, cache-operation scope, QMD/dispatch correlation, exact output, and fault state auditable from complete logs.
- Leave an unambiguous negative or partial result that selects the next narrow experiment instead of layering additional guesses.

**Non-Goals:**

- Installing a permanent NVK upload-cache policy or claiming a generalized coherency fix.
- Flushing the QMD upload, the whole command-upload arena/BO, graphics state, descriptors, images, readback memory, or any unrelated allocation.
- Adding a GPU cache-invalidate method, GPU command, wait, barrier, fresh-allocation strategy, shader readback probe, additional dispatch, or command-buffer lifetime change.
- Changing the observation-only `compute-root-state-diagnostics` capability or relaxing the original `render-compute-image-chain` acceptance contract.
- Async submit, WSI, external memory, frame-generation algorithms, or native-title integration.

## Decisions

### Use a strict experiment selector and keep ordinary execution unchanged

Add a Switch-only environment control named `NVK_ROOT_UPLOAD_CACHE_FLUSH`. Only the exact value `1` enables the branch; absence, `0`, malformed values, and non-Switch builds select the existing path. Add a build-time `FG2_ROOT_UPLOAD_CACHE_FLUSH` value constrained to `0` or `1` in `nvk_render_compute.c`; the dedicated artifact sets the environment control before Vulkan instance creation and logs the build tag and selected mode.

The driver-side selector is cached before command recording, matching the existing bounded root-trace pattern. Experiment bookkeeping is separate from the observation-only trace limit so the cache branch applies consistently across all 64 iterations even though detailed root records remain bounded to iterations 1-2. The selector must not become a Vulkan API, configuration default, or silently enabled production behavior.

Alternative: make the cache operation unconditional for all Switch compute uploads. Rejected because a passing result would conflate experiment and proposed fix, affect unrelated consumers, and prevent a same-revision disabled control. Alternative: key the operation only from `NVK_ROOT_TRACE`. Rejected because trace bounds would flush only the first two records and couple observation with semantics.

### Track reuse only within the selected command-buffer experiment stream

Maintain disabled-by-default experiment state containing the current command-buffer identity, the immediately preceding correlated compute-root GPU VA, and a monotonically increasing upload record. When the command-buffer identity changes, clear the prior-address state. Do not clear it on reset of the same command buffer, because cross-reset address reuse is the condition under test.

For each direct compute-root upload, derive `reused` by comparing the current GPU VA with the immediately preceding correlated upload from the same command-buffer stream. The first upload records `no_prior`; a different later address records `fresh_address`. Only `reused` is eligible for the cache operation. Update the saved VA after classifying the current upload.

This experiment assumes the dedicated single-command-buffer, one-direct-dispatch-per-iteration artifact. Any extra matching upload, marker/order mismatch, changed command-buffer identity, or lack of reuse makes the affected correlation inconclusive rather than broadening the heuristic.

Alternative: infer reuse from a stable CPU pointer alone. Rejected because the GPU consumes the virtual address and the existing evidence is correlated around that identity. Alternative: add state to general command-buffer lifetime structures. Rejected because test-local selector state is sufficient for this single consumer and avoids changing layouts in ordinary execution.

### Flush exactly the copied root mapping between copy and QMD construction

In the existing `nvk_cmd_upload_qmd` direct path, retain the current `memcpy(root_desc_map, root, sizeof(*root))` and diagnostic mapped-byte read. When the strict selector is enabled and the current root GPU VA is classified `reused`, invoke the Horizon/libnx CPU data-cache flush primitive on `root_desc_map` for exactly `sizeof(*root)`. The primitive may internally operate at cache-line granularity, but the requested range must not be widened to the upload BO or neighboring allocations.

Place the operation after the current bytes have been copied and observed through the CPU mapping, and before QMD construction/upload. Do not modify QMD data or flush its mapping; keeping the reused QMD upload untouched is necessary for this experiment to isolate only root-data CPU visibility. Guard the platform include and call with `__SWITCH__`; non-Switch compilation retains a side-effect-free disabled branch.

Alternative: flush both root and QMD uploads. Rejected because success could not identify which upload needed visibility handling. Alternative: flush the entire command upload BO. Rejected because it would touch push commands, QMD data, and neighboring allocations. Alternative: force a new root allocation per iteration. Rejected because it tests address reuse/aliasing rather than cache visibility. Alternative: emit a GPU constant-buffer-cache invalidate or wait. Deferred because each targets a different boundary and needs its own change if this root-only CPU flush is insufficient.

### Reuse the bounded root diagnostics and add one cache-operation record

Preserve the existing marker, push, upload, QMD, QMD-upload, dispatch, and result records for iterations 1-2. Extend the correlated upload record or add one adjacent `phase=root_cache_flush` record containing:

```text
record, command-buffer identity, selector state
previous and current root GPU VA, reuse classification
root CPU mapping, requested size
action=invoked | skipped_no_prior | skipped_fresh_address | disabled
ordering=after_copy_before_qmd
```

The cache primitive has no result value, so logs report invocation and completed control flow, not a fabricated success code or GPU-visibility claim. The enabled-path startup record identifies the experiment, while source review proves that the same branch governs later iterations. Existing exact per-iteration validation continues for all 64 iterations; no high-volume driver trace is required.

Alternative: log one driver record on every iteration. Rejected because bounded records plus unchanged source-controlled execution and existing per-iteration result logs are sufficient, while avoiding unnecessary instrumentation volume. Alternative: rely on artifact name alone. Rejected because a stale or mislaunched binary could otherwise create a false comparison.

### Use paired immutable artifacts and hardware runs for causal comparison

From one committed source revision, build a disabled control and enabled variant with distinct build/version tags and hashes. Verify both use identical generated shader bytes and that static/source comparison shows the only intended semantic difference is selector activation and the root-range cache operation. The disabled control must run on real hardware and reproduce the retained seed-5 stale signature before the enabled result is interpreted; earlier artifacts remain corroborating evidence but do not replace this observer-effect control.

Run both artifacts for the unchanged 64 iterations, retain complete unfiltered combined streams, inspect all lines before filtering, and record supported `ERRNOTIF`/`ERRINFO` data after any timeout/fault. Use these classifications:

1. `control_invalid` — disabled run does not reproduce the baseline or paired-artifact review finds another semantic difference;
2. `inconclusive` — selector/reuse/flush/QMD/result correlation is incomplete or a fault prevents interpretation;
3. `specific_flush_insufficient` — enabled flush is confirmed and the exact stale signature remains;
4. `behavior_changed_unresolved` — enabled output changes but does not pass the oracle;
5. `hypothesis_supported_experiment_only` — valid control plus 64/64 exact enabled output, without promoting a production runtime behavior.

FG-2 remains `BLOCKED` under every classification until the original unmodified render-compute-image-chain contract independently passes. A passing diagnostic variant is evidence for the next design/fix change, not permission to silently retain the experiment as production policy.

## Risks / Trade-offs

- [The CPU cache primitive also invalidates or cache-line-aligns internally] → Record the requested pointer/range, limit the request to `sizeof(*root)`, and state the platform primitive used; do not claim byte-granular hardware action.
- [The reused QMD mapping, rather than root data, is stale] → Leave QMD cache behavior unchanged; an unchanged result rejects only root-range CPU flushing as sufficient and can motivate a separately specified QMD experiment.
- [Static experiment state observes an unrelated upload or command buffer] → Enable it only in the dedicated artifact, key it to command-buffer identity, validate one correlated direct upload per marker, and classify mismatches as inconclusive.
- [The first enabled run passes because instrumentation or build layout changed] → Require a same-commit disabled hardware control, identical shader hashes, source/static paired review, and exact retained-signature comparison.
- [A changed but incorrect image is overinterpreted] → Retain exact pixels/checksums and use `behavior_changed_unresolved`; do not collapse partial change into hypothesis support.
- [The cache call is unavailable or incorrectly declared in Mesa's Switch build] → Use the established libnx platform declaration under `__SWITCH__`, compile the NVK target and both NROs, and keep non-Switch compilation side-effect free before hardware execution.
- [The ignored Mesa tree or durable patch drifts] → Diff touched files against clean Mesa 25.0.7 and current patch, preserve unrelated user changes, regenerate the durable patch, and verify clean reconstruction byte-for-byte.
- [A hardware fault truncates evidence] → Retain the full stream, capture notifier/error-info data where supported, mark the run inconclusive, and stop rather than adding another semantic change.

## Migration Plan

1. Reconfirm the clean Mesa 25.0.7 baseline, current extracted-tree differences, durable patch ownership, and absence/presence of tracked mirrors for every touched file.
2. Add the strict selector, reuse tracking, root-only cache operation, cache record, and artifact startup mode while preserving the frozen command and validation contract.
3. Update/regenerate the durable Mesa patch and extracted tree, verify clean reconstruction, compile NVK, and build same-revision disabled/enabled artifacts with identical shader hashes.
4. Commit the exact source revision, hash both artifacts, run the disabled control and then the enabled variant on real Tegra, and retain/inspect complete logs and fault state.
5. Record the paired hardware evidence and exact outcome under `docs/testing/` and `docs/research/`; update the capability matrix only for a durable evidence-backed finding and add no ADR unless a persistent production decision is later proposed.
6. Roll back operational use by leaving the selector unset or removing the dedicated experiment artifacts. Preserve the patch history and all positive, negative, or inconclusive evidence; do not promote the gated branch into ordinary execution within this change.
