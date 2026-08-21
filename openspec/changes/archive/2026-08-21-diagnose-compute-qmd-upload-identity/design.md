## Context

See `proposal.md` for motivation and `specs/compute-qmd-upload-identity/spec.md` for the experiment contract. The retained FG-2 hardware record shows that command-buffer reset/rerecord reuses root GPU VA `0xc7f40000` and QMD GPU VA `0xc7f40800`. Iteration 2 places seed 42 in the root source and CPU mapping, QMD cbuf 0 encodes the reused root address, direct dispatch selects the reused QMD address, and a root-only CPU cache flush is insufficient to change the stable output signature.

The build-authoritative Mesa representation is `patches/switch-nvk-mesa-25.0.7.patch`; the extracted `mesa-25/` tree is regenerable. Upstream Mesa 25.0.7 builds a 64-dword QMD on the CPU and passes it to `nvk_cmd_buffer_upload_data`, which obtains a 256-byte-aligned mapped upload allocation and copies the bytes without returning the mapping. The changing push-constant seed is in the separately uploaded root table, so the full QMD may be identical when its root address and all other launch inputs are reused.

The experiment remains hardware-gated. CPU inspection can prove generated/mapped byte identity and address correlation, but only real GM20B execution can establish whether a targeted cache operation affects behavior.

## Goals / Non-Goals

**Goals:**

- Prove complete QMD source-to-mapping identity and cross-iteration payload identity without altering GPU work.
- Make an explicit, auditable decision about whether a reused-QMD CPU flush is discriminating.
- If eligible, isolate exactly one semantic variable: a CPU data-cache flush requested for the reused 256-byte QMD mapping.
- Preserve exact control/variant correlation, complete-log inspection, deterministic output validation, and narrow evidence language.

**Non-Goals:**

- Establish a production cache policy or alter the ordinary NVK upload path.
- Force a new QMD or root allocation, insert a payload nonce, or modify a QMD field merely to make hashes differ.
- Add a GPU QMD/constant-buffer invalidation, memory barrier, wait, submission change, or allocation-lifetime change.
- Diagnose sampled-image freshness, GPU address translation, constant-buffer cache behavior, or every remaining FG-2 boundary in the same experiment.
- Mark FG-2 `PROVEN_HW` from an experimental variant.

## Decisions

### Expose the QMD mapping by locally expanding the existing upload helper

Within the direct compute QMD path, use the existing upload allocator with the same 256-byte size and `0x100` alignment, then perform the same `memcpy` currently performed by the upload-data helper. Keep this expansion in both disabled and enabled artifacts. This exposes the exact CPU mapping backing the returned QMD GPU address without changing allocation order, requested size/alignment, or copied payload.

Alternative: change the shared upload-data helper to optionally return its mapping. Rejected because it broadens an experiment-specific interface and risks affecting unrelated upload callers. Alternative: rediscover the pointer from internal allocation state after the helper returns. Rejected because it couples the diagnostic to allocator internals and can identify the wrong range.

### Use exact byte comparisons as authority and compact hashes as log identities

Compute a deterministic 64-bit FNV-1a identity over all 256 QMD bytes for the generated source and mapped copy. Use `memcmp` over all 256 bytes for source-to-mapping equality and for current-versus-previous generated-payload equality; hashes are compact audit values, not the authority for equality. Save the preceding captured generated payload, address, and identity only in the opt-in bounded diagnostic state keyed to the command buffer and reset the correlation state when that identity changes.

Alternative: log all 64 dwords. Rejected because it adds unnecessary volume and makes complete-stream review harder. Alternative: rely on a hash alone. Rejected because a collision would create a false eligibility decision. Alternative: inspect only the already decoded root-address dwords. Rejected because other QMD fields could differ while those fields remain stable.

### Separate observation eligibility from the semantic cache action

Use an explicit QMD diagnostic selector for bounded observation and a separate strict QMD-cache selector for the enabled artifact. The disabled control performs mapping exposure, copying, identity calculation, equality checks, and logging but never performs the cache operation. At iteration 2, eligibility requires:

1. the bounded record/order correlation is valid;
2. the previous QMD address exists and equals the current address;
3. generated and mapped bytes match exactly for the current record;
4. the preceding generated payload was captured validly;
5. the current generated payload differs byte-for-byte from the preceding generated payload; and
6. the enabled QMD-cache selector is exactly valid.

An identical payload produces `ineligible_identical_payload` and no cache call. A changed address produces `ineligible_fresh_address`. Missing/mismatched observation produces an explicit inconclusive reason. No artificial QMD change is permitted to satisfy eligibility.

Alternative: always flush a reused QMD even when its bytes are identical. Rejected because the negative outcome would not discriminate visibility of current versus stale QMD data. Alternative: decide eligibility only from the disabled artifact and compile an unconditional variant afterward. Rejected because retaining the same runtime guard in both artifacts makes accidental execution and evidence correlation easier to audit.

### Place the cache operation after identity validation and before dispatch

When all eligibility conditions hold, request `armDCacheFlush(qmd_map, sizeof(qmd))` after `memcpy`, source/mapping hashing, and exact equality checks, and before returning the QMD address to the unchanged direct-dispatch path. The requested range is exactly 256 bytes; the record acknowledges that libnx may internally operate at cache-line granularity and that call completion does not itself prove GPU visibility.

Do not enable the existing root-upload cache selector, flush the containing upload BO, alter `SEND_PCAS` methods, or add a command/wait. The existing QMD contents, address, and dispatch commands remain the same.

Alternative: flush the root and QMD together. Rejected because the prior root-only experiment and the QMD boundary could no longer be separated. Alternative: add GPU invalidation in the same variant. Rejected because CPU visibility and GPU cache/launch state are distinct hypotheses.

### Preserve and extend the existing bounded correlation stream

Retain the existing marker, push, root upload, decoded QMD root address, QMD upload, dispatch, result, and fault-state records for iterations 1-2. Extend the QMD-upload record or add an adjacent record with:

```text
record, cmd, selector, qmd_source_hash, qmd_mapped_hash,
source_map_match, qmd_map, qmd_size,
previous_qmd_gpu_va, qmd_gpu_va, qmd_reuse,
previous_payload_hash, payload_equal,
eligibility, cache_action, requested_range,
root_gpu_va, ordering, cpu_mapping_backing_gpu_va=1,
gpu_visible=UNPROVEN
```

The dispatch record remains the proof of the address actually selected by `SEND_PCAS_A`. Any missing count, ordering, address, copy, selector, or result correlation invalidates the causal comparison instead of being repaired during analysis.

### Run hardware in a gated sequence

Build same-source identity-control and cache-enabled artifacts with distinct version tags and recorded hashes. First run and inspect the complete identity-control stream. Continue to the enabled hardware run only when the control reproduces the retained signature and proves a reused, naturally changed, exactly copied QMD payload. If the payload is identical, record that hardware identity finding and stop without running the enabled artifact.

If eligible, run the enabled artifact on the same console/configuration for the unchanged 64 iterations. Compare exact per-iteration pixels/checksums, selector and action state, QMD/root addresses and identities, invocation range, dispatch, complete warnings, and GPU fault state. A passing experimental variant does not promote FG-2; only a later unmodified-chain hardware pass can do that.

## Risks / Trade-offs

- [Expanding the upload helper changes the baseline path] → Make the equivalent allocation/copy expansion identical in both artifacts and require the disabled control to reproduce the retained signature exactly.
- [Hash collision creates false payload identity] → Use full `memcmp` results for all decisions; hashes are log labels only.
- [Instrumentation changes allocation timing or behavior] → Keep state bounded, avoid allocations and full dumps in the traced loop, and reject any control that changes addresses, command behavior, or output signature unexpectedly.
- [Static diagnostic correlation is unsafe for general multithreaded applications] → Keep it opt-in and dedicated to the single-command-buffer smoke artifact; do not promote it as general tracing infrastructure.
- [The cache primitive rounds the range internally] → Preserve 256-byte alignment, log the exact requested mapping/range, and claim only that the call was invoked—not byte-granular hardware behavior.
- [A changed enabled output is overinterpreted] → Retain the exact new signature and classify it as an effect without calling the hypothesis supported or rejected unless the full oracle passes or the exact baseline persists.
- [Identical QMDs leave the stale boundary unresolved] → Treat identical payload identity as a useful hardware result, stop this experiment, and require a separate proposal for QMD-address reuse or GPU consumption.

## Migration Plan

1. Update the durable Mesa patch and regenerate/apply the build tree as required by `REPRODUCE.md`; update only tracked representations that are intentionally authoritative.
2. Add opt-in experiment selection/reporting to the tracked FG-2 artifact while leaving ordinary builds disabled.
3. Run host patch-application, cross-build, source-equivalence, artifact-hash, shader-hash, and log-schema checks.
4. Commit the exact experiment source before hardware execution.
5. Run the disabled identity control and inspect its complete log before deciding whether the enabled run is allowed.
6. If eligible, run the enabled variant and compare the complete paired evidence; otherwise stop after recording the identity result.
7. Update testing/research evidence and the capability matrix with only the justified status. Rollback consists of leaving the opt-in selectors disabled or reverting the experimental patch hunk; no persistent data or public API migration is involved.
