## Context

See `proposal.md` for motivation and `specs/compute-qmd-address-reuse/spec.md` for the behavioral contract. The retained hardware evidence fixes the current boundary:

```text
iteration-2 current root bytes
    -> reused root GPU VA 0xc7f40000
    -> QMD encodes that current root VA
    -> complete 256-byte QMD equals iteration 1
    -> reused QMD GPU VA 0xc7f40800
    -> direct PCAS selects that QMD VA
    -> stale result
```

The complete identity-control log proves the iteration-1 exact result and stable iterations-2-64 stale signature without a relevant fault. It also proves exact generated-to-mapped QMD equality and exact cross-iteration QMD equality for the first two records. The previous root-only CPU cache flush was invoked correctly and was insufficient; the identical-QMD flush was correctly classified as non-discriminating and not run. Those questions remain closed for this change.

The build-authoritative Mesa representation is `patches/switch-nvk-mesa-25.0.7.patch`; `mesa-25/` is a regenerated build tree rather than the durable source. Today `nvk_cmd_upload_qmd` allocates the 2048-byte root first, then obtains one 256-byte-aligned QMD upload allocation and copies the 64-dword payload. Command-buffer reset returns its owned command memory to the pool, and the next recording ordinarily recovers the same root and QMD GPU VAs. The artifact resets and rerecords one primary command buffer only after the existing submission wait.

## Goals / Non-Goals

**Goals:**

- Keep allocator activity identical between the paired control and variant while making only the selected QMD slot/address differ.
- Guarantee that each of the 63 variant transitions selects a QMD GPU VA different from the immediately preceding dispatch.
- Preserve the current root allocation and all application/GPU workload semantics outside QMD placement.
- Produce enough exact, bounded evidence to validate the causal pair and all outcome gates without relying on hashes for byte equality.

**Non-Goals:**

- Define a production QMD allocator, cache policy, launch-state workaround, or ordinary NVK behavior.
- Guarantee that a variant address has never appeared earlier in the 64-iteration run; this experiment tests freshness relative to the immediately preceding dispatch.
- Add a cache flush/invalidation, barrier, wait, submission change, payload nonce, unrelated QMD field change, shader change, or alternate dispatch method.
- Re-diagnose root upload, QMD generation/copy identity, sampled-image state, or every possible GPU-consumption mechanism.
- Advance FG-2, design an FG-3 change, or treat a successful experimental variant as unmodified-chain acceptance.

## Decisions

### Reserve two identical QMD slots in both paths and change only slot selection

Under a strict experiment selector, keep the existing first 256-byte, `0x100`-aligned QMD upload allocation as the primary slot and reserve an immediately following secondary 256-byte, `0x100`-aligned slot. Perform these same two allocations, in the same order and with the same sizes/alignment, in both control and variant after the unchanged root allocation.

Copy the naturally generated 256-byte QMD exactly once into the selected slot. The control always selects the primary slot, preserving the ordinary QMD GPU address and its reset-time reuse. The variant selects the primary slot for record 1 and alternates primary/secondary for records 2-64. The selected sequence therefore has 63/63 consecutive address changes while the allocation calls, allocation order, command-memory ownership, root allocation, QMD payload, and copy size remain identical across the pair. Both slots remain owned by the recorded command buffer through the existing submit/wait; reset reclaims them only at the same lifecycle point as current upload memory.

The initial primary allocation deliberately remains the current ordinary 256-byte request rather than replacing it with a larger monolithic reservation. The unused slot is the minimum paired observer-effect cost needed to equalize allocation behavior between control and variant. The control must still prove that this reservation does not alter the retained address/output baseline.

Alternative: allocate 64 globally unique QMD slots. Rejected because the contract requires only freshness relative to the preceding dispatch and a larger reservation can cross allocator/BO boundaries, broadening the experiment. Alternative: force a new BO or GPU mapping each iteration. Rejected because it adds allocation lifetime, mapping, and ownership variables. Alternative: alternate command buffers. Rejected because it changes command-buffer lifecycle. Alternative: add variable padding only in the variant. Rejected because allocation calls would differ between the pair. Alternative: mutate QMD bytes. Forbidden because it changes payload semantics.

### Scope the conclusion to consecutive-address freshness

The two-slot sequence may return to an address used two iterations earlier. This is intentional and shall be reported: the intervention proves only that the current dispatch did not select the immediately preceding dispatch's QMD address. If a host allocation check cannot demonstrate two distinct aligned addresses for all 63 transitions, implementation stops before hardware rather than silently widening the allocator intervention.

A passing variant supports only immediate QMD-address reuse as exercised by this ping-pong placement. A failing exact-stale result means only that this specific consecutive-address change was insufficient; it does not exclude longer-lived launch/cache state keyed to an earlier address.

### Keep direct-dispatch structure fixed while acknowledging the address operand change

Return the selected slot's GPU VA through the existing direct-QMD path and retain the existing `SEND_PCAS_A` method sequence. The dispatch record must prove that the shifted `PCAS` operand reconstructs exactly the selected mapped slot address. Source/artifact review compares method structure and semantics, not raw GPU command-byte identity, because the changed QMD address operand necessarily changes command bytes.

No new GPU method, barrier, invalidation, or wait is emitted. The first variant record uses the primary address so the retained iteration-1 observation is directly comparable; subsequent records alternate only the address operand.

### Use a dedicated strict selector and fixed-size experiment state

Add separate control and fresh-address selectors set by the artifact before Vulkan creation. Absent, zero, malformed, contradictory, or non-experiment configurations remain side-effect free and use the ordinary one-slot path. The QMD-address experiment is limited to the dedicated single-command-buffer FG-2 artifact and is not general tracing infrastructure.

Keep fixed state for one command-buffer identity: record counter, previous selected QMD VA, first generated QMD bytes, previous generated QMD bytes, compact identities, correlation state, and aggregate freshness/equality counts. Reset correlation on a command-buffer identity change or invalid ordering. Do not allocate diagnostic memory in the recording loop.

Alternative: infer parity from CPU pointers or allocator calls. Rejected because only mapped GPU VAs and the dispatched address establish the tested condition. Alternative: expose this as a general allocator mode. Rejected because there is no production consumer or evidence for such an abstraction.

### Combine one bounded full-payload record with fixed per-iteration summaries

For each path, retain one complete 256-byte generated-QMD record and one complete mapped-QMD record for the first correlated dispatch so the pair can be compared exactly offline. For every dispatch, use full `memcmp` for source-to-mapped equality and current-to-first/current-to-previous generated-QMD equality; emit hashes only as compact labels. A fixed 64-record summary correlates:

```text
path/selector, record, iteration, seed,
root source seed, root mapped seed,
previous/current root GPU VA, root reuse,
QMD source/mapped hash, source_map_match,
first/current and previous/current exact equality,
primary/secondary/selected QMD GPU VA,
previous selected QMD GPU VA, freshness,
dispatched QMD GPU VA, PCAS/address/order match,
pixel, checksum, per-iteration oracle,
GPU fault/error state
```

Detailed root/QMD structural records remain limited to the first two dispatches; the per-iteration line is compact and capped at 64 so all 63 freshness transitions are auditable. Emit a final aggregate with `fresh_transitions=63/63`, exact-copy/equality counts, validation count, and fault state. Reuse fixed buffers, avoid SD-only high-volume writes, and keep the same instrumentation in both artifacts. Any altered control address/signature invalidates the observer.

Alternative: log hashes only. Rejected because exact bytes are authoritative. Alternative: dump both QMDs on every iteration. Rejected because one full reference plus in-process exact comparisons is sufficient and materially increases observer pressure. Alternative: prove only iteration 2 freshness. Rejected because it would weaken the existing 64-iteration oracle.

### Gate hardware execution and classify only the complete pair

Build same-source control and variant artifacts with distinct version/path tags, record exact artifact and generated-shader hashes, and inspect source and GPU method-generation differences before hardware. Run the control first on the same console/configuration intended for the variant and read its complete unfiltered log. Stop if it does not reproduce the retained addresses and exact stale signature or if any required equality/correlation/fault field is invalid.

Only after a valid control, run the 64-iteration variant and inspect its complete log before filtering. Apply the spec's exact classifications:

- `qmd_address_reuse_hypothesis_supported_experiment_only` only for a valid control, 63/63 fresh transitions, 64/64 independent-oracle passes, complete correlation, and no unexplained fault;
- `specific_qmd_address_change_insufficient` only for a valid control, proven freshness, the exact retained stale signature, and no relevant fault;
- `behavior_changed_unresolved` for proven freshness and a changed but still failing output;
- `inconclusive` for any failed prerequisite, unexpected QMD difference, observer effect, missing evidence, or uninterpretable fault.

If the experiment-only support classification occurs, the next action is not promotion: propose and run a separate original unmodified image-chain acceptance artifact with all experimental selectors absent. FG-2 remains `BLOCKED` until that ordinary path passes on hardware.

### Correct current source-of-truth wording during apply

The existing evidence already establishes QMD-address reuse as the immediate discriminating boundary, so the apply workflow shall update `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` rather than placing those edits in this planning-only proposal commit. The edits will replace generic QMD-upload-visibility wording with the narrower QMD-address-reuse boundary, cite the retained identity evidence, and retain FG-2 as `BLOCKED`. Hardware-result documentation later adds only the outcome actually observed and preserves all prior negative records.

## Risks / Trade-offs

- [The extra reserved slot changes allocator behavior enough to alter the control] -> Keep both 256-byte allocations identical across the pair, retain the ordinary primary allocation first, and reject the experiment unless the control reproduces the address and output baseline exactly.
- [Two-slot ping-pong is mistaken for globally fresh storage] -> Log the selected sequence and state explicitly that freshness is only relative to the immediately preceding dispatch.
- [The secondary slot is not contiguous, aligned, or distinct] -> Validate both returned GPU VAs and mappings on every record; stop before hardware or classify `inconclusive` on any failure.
- [Diagnostic logging perturbs timing or allocation] -> Use fixed state, bounded records, no loop-time dynamic allocations, identical instrumentation in both paths, and the control observer-effect gate.
- [A hash collision hides a payload difference] -> Make every decision with full 256-byte comparison; hashes remain labels only, with one complete source/mapped payload retained per artifact for exact cross-artifact review.
- [Static experiment state is unsafe for general applications] -> Enable it only under the strict dedicated-artifact selector and never promote it as a runtime facility.
- [Changed `PCAS` bytes are misreported as command equivalence] -> Compare method structure/semantics and explicitly exclude the required address operand from equality claims.
- [A passing variant is promoted prematurely] -> Leave it opt-in/rollbackable, keep FG-2 `BLOCKED`, and require a separate unmodified-chain acceptance change/run.
- [A failing variant is overgeneralized] -> Use only `specific_qmd_address_change_insufficient` for the exact stale result and retain longer-lived allocator/cache/GPU-consumption possibilities as unresolved.

## Migration Plan

1. Reconfirm the durable patch, regenerated tree, and any intentional mirrors according to `REPRODUCE.md`; freeze the current QMD allocation/copy and artifact oracle before edits.
2. Add the strict paired selectors, identical two-slot allocation sequence, control/variant slot selection, fixed correlation state, and bounded reporting to the durable source and dedicated artifact only.
3. Update `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md` to name QMD-address reuse as the immediate boundary without changing FG-2 status.
4. Regenerate the build tree and run patch-application, cross-build, selector, allocation/address, exact-comparison, source-equivalence, shader-hash, method-structure, and log-schema checks. Record host evidence as `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`, never hardware proof.
5. Commit the exact experiment source, record artifact/provenance hashes, run the control, and inspect the complete log. Stop on any invalid baseline or prerequisite.
6. If authorized by the valid control, run the variant on the same real Tegra context, inspect the complete log, compare the pair, and retain immutable raw/testing/research evidence under the narrow classification.
7. Leave the selector disabled or remove the experimental slot-selection hunk after evidence capture; do not promote it into ordinary NVK behavior. A supported experiment requires a new unmodified-chain acceptance proposal before FG-2 can advance.
