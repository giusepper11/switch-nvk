## Context

See `proposal.md` for motivation and `specs/compute-root-state-diagnostics/spec.md` for the observable contract. FG-1 independently passed 64/64 compute iterations on real Tegra. FG-2 artifacts `4b1ba31` and `d9dfc65` both rendered with the changing graphics seed but produced the same post-iteration-1 compute result equivalent to seed 5; the latter used one compatible shared graphics/compute layout and one combined-stage push, so that hypothesis is `REJECTED`.

The relevant direct-dispatch path in Mesa 25.0.7 is:

```text
nvk_CmdPushConstants2KHR
  -> nvk_push_constants
  -> nvk_descriptor_state_set_root_array(... push ...)

nvk_CmdDispatchBase
  -> nvk_flush_compute_state
  -> nvk_cmd_flush_cs_qmd
  -> nvk_cmd_upload_qmd
       -> nvk_cmd_buffer_upload_alloc(root)
       -> memcpy(root_desc_map, root, sizeof(*root))
       -> nak_fill_qmd(... root cbuf address ...)
       -> nvk_cmd_buffer_upload_data(qmd)
  -> SEND_PCAS_A(qmd_addr >> 8)
```

`winsys/smoke/nvk_render_compute.c` resets and rerecords one primary command buffer per iteration, pushes one seed for fragment and compute stages, performs one direct dispatch, submits, waits, and exhaustively validates readback. Iteration 1 uses seed 5 and iteration 2 uses seed 42. This one-push/one-direct-dispatch ordering provides a narrow correlation contract; it is not intended as a general multi-threaded tracing facility.

The reproducible build consumes the gitignored extracted `mesa-25/` tree, while `patches/switch-nvk-mesa-25.0.7.patch` is the durable source needed to recreate it. The current extracted tree must be compared with a clean Mesa 25.0.7 baseline before editing because ignored diagnostic leftovers are not authoritative. The diagnostic implementation must update the durable patch and the extracted build tree, must not edit `pristine-25.0.7/` as though it were patched source, and must update an intentionally tracked mirror only if the build/reproduction review proves one exists for a touched file.

## Goals / Non-Goals

**Goals:**

- Determine, for iteration 1 and iteration 2, the last boundary containing the expected seed and the first boundary containing stale seed or stale address state.
- Capture exact root bytes and identities at push, upload, QMD construction, and direct dispatch while preserving the original GPU command stream.
- Distinguish bytes observed through the CPU mapping backing a GPU virtual address from bytes proven visible to the GPU.
- Produce a bounded, parseable record that can be joined to the existing exact output oracle and retained as complete real-hardware evidence.
- Make observer effects and unobserved boundaries explicit rather than interpreting changed behavior as a fix.

**Non-Goals:**

- Changing push-constant, descriptor, upload, cache, QMD, dispatch, reset, or synchronization semantics.
- Adding a cache flush, invalidation, wait, readback command, replacement shader, or additional GPU dispatch to the baseline diagnostic run.
- General-purpose NVK tracing, thread-safe cross-application event correlation, broad QMD dumping, or a public Vulkan extension/API.
- Async submit, WSI, external memory, frame-generation algorithms, NVN/native-title integration, Mesa refactoring, or FG-3 work.

## Decisions

### Gate the trace independently and route it through the existing Switch log sink

Use a dedicated numeric environment control such as `NVK_ROOT_TRACE=3`, set by the diagnostic build of `nvk_render_compute` before Vulkan instance creation. Absence or zero disables the path. The value is the maximum number of matching push/direct-dispatch records, normally two and at most three for this experiment.

On Switch, diagnostic helpers in the NVK command-buffer sources route single-line records through the existing `g_drm_shim_log_sink`, which the smoke artifact already installs into both its SD log and nxlink stream. Any non-Switch compilation path remains build-safe and disabled unless an equivalent explicit diagnostic sink is available. The helper does not allocate GPU memory, mutate descriptor/root structures, or emit GPU commands.

Alternative: add a permanent `NVK_DEBUG` enum and general trace subsystem. Rejected because this is a single-blocker experiment and a new general facility would broaden both code and validation. Alternative: print all 64 iterations. Rejected because SD/nxlink logging can materially perturb timing and the first two seeds already distinguish fresh from stale state.

### Correlate records by bounded sequence, command-buffer identity, seed, and app markers

Immediately before the combined-stage push, `winsys/smoke/nvk_render_compute.c` emits:

```text
FG2_ROOT_DIAG phase=marker record=1 iteration=1 expected_seed=5 cmd=<handle>
```

and similarly for record 2 / iteration 2 / seed 42. Driver records include the same bounded direct-path sequence plus the command-buffer pointer. Because the artifact records one matching compute-inclusive push followed by one direct dispatch per iteration on one thread, the app marker and synchronous record order are the primary join; the seed, command-buffer identity, root identity, upload address, and QMD address are cross-checks. A count or ordering mismatch invalidates correlation rather than being silently repaired.

The result line reports the exact observed/expected pixel and checksum and, when the existing pixel algebra uniquely identifies it, `observed_behavior_seed=5`; otherwise it reports `UNKNOWN` rather than guessing.

Alternative: add an iteration field to `nvk_cmd_buffer` or encode an iteration marker into GPU state. Rejected because either changes driver object layout/allocation behavior or the command stream. Alternative: correlate only by upload address. Rejected because allocator reuse is itself a diagnostic observation and cannot serve as a unique key.

### Inspect graphics and compute roots immediately after the push update

Instrument `nvk_CmdPushConstants2KHR` in `mesa-25/src/nouveau/vulkan/nvk_cmd_buffer.c` after both stage-conditioned `nvk_push_constants` calls. For the matching bounded records, copy the four bytes at `nvk_root_descriptor_offset(push) + info->offset` from both `cmd->state.gfx.descriptors.root` and `cmd->state.cs.descriptors.root` into local scalars and log:

```text
FG2_ROOT_DIAG phase=push record=<n> cmd=<ptr> stages=<mask>
  offset=<B> size=<B> input_seed=<u32>
  gfx_desc=<ptr> gfx_root=<ptr> gfx_seed=<u32>
  cs_desc=<ptr> cs_root=<ptr> cs_seed=<u32>
```

The implementation validates that the diagnostic range covers the expected four bytes before interpreting a seed; otherwise it logs raw bounded bytes and `seed=UNAVAILABLE`. `nvk_cmd_buffer.h` is read to use the real `nvk_root_descriptor_table` layout and offset helpers; it is changed only if a small internal trace declaration is required, not to add diagnostic fields to descriptor or command-buffer state.

Alternative: log only `pValues`. Rejected because it proves the API input but not either stage's stored root. Alternative: instrument the macro globally. Rejected because it would capture unrelated descriptor mutations and generate high-volume logs.

### Observe the upload source and the CPU mapping that backs the returned GPU VA

Instrument the direct compute-root branch in `nvk_cmd_upload_qmd` in `mesa-25/src/nouveau/vulkan/nvk_cmd_dispatch.c`. Immediately before the existing `memcpy`, read the seed from `root->push`; immediately after it, read the same bytes from `root_desc_map`. Log the descriptor/source identity, CPU mapping, returned `root_desc_addr`, allocation size/alignment, source seed, and mapped seed:

```text
FG2_ROOT_DIAG phase=upload record=<n> cmd=<ptr> cs_desc=<ptr>
  root_src=<ptr> source_seed=<u32> root_map=<ptr> mapped_seed=<u32>
  root_gpu_va=<u64> size=<B> alignment=<B> reused_vs_previous=<0|1>
```

This read validates the CPU alias returned by `nvk_cmd_buffer_upload_alloc`; it does not prove that Tegra has observed the new bytes. The evidence labels it `CPU mapping backing GPU VA`, records whether CPU and GPU addresses differ, and does not call `armDCacheFlush`, add a memory barrier, or perform a GPU readback in the observation-only build.

Alternative: flush or invalidate the allocation before reading it. Rejected because that is a semantic cache experiment capable of hiding the defect. Alternative: add a GPU copy/readback immediately. Deferred because it changes the GPU command stream; it is an acceptable separately planned follow-up only if the baseline narrows the uncertainty to GPU visibility/consumption.

### Decode only the QMD fields that carry the root constant-buffer address

While iterating `shader->cbuf_map` in `nvk_cmd_upload_qmd`, record the cbuf index whose type is `NVK_CBUF_TYPE_ROOT_DESC` and the exact address/size placed into `qmd_info.cbufs`. After `nak_fill_qmd`, use `nak_get_qmd_cbuf_desc_layout(&pdev->info, root_cbuf_index)` and a bounded bit-field extractor to reconstruct the lower and upper address fields from the generated 64-dword QMD. Log the root cbuf index, only the dwords intersecting those fields, decoded address, `root_desc_addr`, and equality result.

After QMD upload, also log the QMD GPU address. Extend the internal `nvk_cmd_flush_cs_qmd` result path only as needed to return the correlated root address to `nvk_CmdDispatchBase`; there log the `qmd_addr` used by `SEND_PCAS_A`, its encoded `qmd_addr >> 8` PCAS value, and the correlated root address. This verifies construction and dispatch wiring without dumping unrelated QMD state.

Alternative: trust `qmd_info` without decoding the generated QMD. Rejected because the experiment must distinguish the construction input from the actual encoded fields. Alternative: dump all QMD dwords or enable global push dumps. Rejected because it increases volume and obscures the hypothesis-specific fields.

### Use one stable line schema and produce a boundary table

Every diagnostic line begins with `FG2_ROOT_DIAG` and uses space-separated `key=value` fields. Addresses are hexadecimal, seeds are unsigned decimal plus optional raw hexadecimal, absent data is `UNAVAILABLE`, and match flags are explicit. No conclusion is derived from pointer identity alone.

The hardware review joins the records into:

| Iteration | Expected seed | Graphics CPU root | Compute CPU root | Upload source | Uploaded CPU mapping | Upload GPU VA | QMD root VA | Dispatched QMD VA | Observed behavior seed |
|---|---:|---:|---:|---:|---:|---|---|---|---:|
| 1 | 5 | captured | captured | captured | captured | captured | captured | captured | captured |
| 2 | 42 | captured | captured | captured | captured | captured | captured | captured | captured |

The conclusion names the last confirmed-current boundary and first confirmed-stale boundary. If all CPU/QMD observations are current but output is stale, the precise result is only “after the uploaded CPU mapping and QMD dispatch encoding, before or at GPU constant-buffer consumption.” It does not distinguish CPU cache visibility, GPU cache state, address translation, or shader consumption without another experiment.

### Keep the first hardware run observation-only and stop when discriminating evidence exists

Build an immutable diagnostic artifact from an exact committed source revision and record its SHA256, version/build tag, diagnostic flags, and source-diff review. Run all 64 original validation iterations on a real Switch/Tegra device, but capture detailed root records only for the first two or three. Retain and inspect the complete combined application/driver stream before filtering, and collect supported `ERRNOTIF`/`ERRINFO` state after any fault or timeout.

The evidence record includes:

```text
Repository commit:
Artifact SHA256:
Hardware/model:
Artifact/version:
Diagnostic build flags:
Iterations inspected:
Expected seed:
Graphics CPU root seed:
Compute CPU root seed:
Upload source seed:
Uploaded CPU-mapping seed:
Upload GPU address:
QMD/root address:
Dispatched QMD address:
Observed result/behavior seed:
Instrumentation observer effect:
GPU fault/error state:
Raw logs:
Conclusion:
```

Compare the run with both retained failure signatures, including seed-5 first-iteration success, seed-42 first mismatch, observed checksum behavior, and absence/presence of GPU faults. Record whether the build added GPU allocations, changed upload reuse, modified command-buffer layout, added synchronization, or changed shader bytes; the baseline design permits none of those intentional changes.

If one boundary is clearly stale, stop and document it. If the trace changes behavior, record `observer effect detected` and plan a lower-impact capture such as a preallocated in-memory trace drained only after completion. If all logged CPU/QMD fields are current but output is stale, plan one separate GPU-side visibility/consumption experiment. Never introduce a cache flush or wait into the baseline artifact and then report its pass as a fix.

## Risks / Trade-offs

- [Synchronous logging changes CPU timing or binary/heap layout enough to alter the symptom] → Limit capture to two or three records, avoid GPU allocations and structure changes, compare the full deterministic signature, and classify any changed behavior as an observer effect.
- [Independent bounded counters drift because an unrelated compute-inclusive push or internal dispatch occurs] → Require marker/order/command-buffer cross-checks and invalidate the correlation on any mismatch; do not silently renumber records.
- [The CPU mapping contains 42 but Tegra observes 5] → State only that the CPU alias and QMD are current; defer GPU visibility versus consumption to a separate controlled experiment.
- [QMD address fields straddle dwords or contain unrelated bits] → Use NAK's device-specific `nak_get_qmd_cbuf_desc_layout` ranges and bounded extraction; log only intersecting raw dwords as audit data.
- [Upload GPU addresses are reused across command-buffer resets] → Treat reuse as evidence, correlate by record and mapping contents, and never assume a new address is required for correctness.
- [The ignored `mesa-25/` tree contains pre-existing local diagnostics] → Diff against a clean Mesa 25.0.7 source, preserve unrelated user work, and make the durable patch the reproducible change rather than trusting the extracted tree.
- [Driver output does not reach the configured Mesa file] → Route through the installed shim sink and retain combined nxlink output, while explicitly recording any missing configured log file.
- [The hardware run faults before a complete record] → Retain the full log, capture supported notifier/error-info data, mark the affected boundary unresolved, and keep FG-2 `BLOCKED`.

## Migration Plan

1. Confirm the clean Mesa 25.0.7 source paths, current extracted-tree differences, durable patch ownership, and absence/presence of tracked mirrors before editing.
2. Add the disabled-by-default bounded diagnostics and the app correlation markers without changing the command stream, descriptor/root layout, shader artifacts, synchronization, or oracle.
3. Regenerate/update the durable Mesa patch, reconstruct or review the extracted build tree, build the dedicated artifact, and verify that ordinary diagnostic-disabled builds emit no trace.
4. Commit the diagnostic source, hash the artifact, run the observation-only artifact on real Tegra, and retain complete raw logs and fault state.
5. Produce the iteration table and first-boundary finding. Keep FG-2 `BLOCKED`; update the capability matrix only for a durable real-hardware fact and create no ADR unless a persistent architecture decision results.
6. Disable the diagnostic by removing the environment control for ordinary runs. Rollback removes the opt-in trace hunks and app markers from the durable patch/source while retaining the negative or positive evidence records; no runtime semantic rollback is required because the baseline diagnostic adds no semantic change.
