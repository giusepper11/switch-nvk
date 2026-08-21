# Compute-root address reuse-distance Specification

## Purpose

Determine whether the retained FG-2 stale-state onset follows the first revisit of a previously dispatched compute-root GPU virtual address by shifting that revisit one iteration without allowing QMD-address reuse at either decisive boundary.

## Requirements

### Requirement: A same-source pair isolates compute-root reuse distance
The experiment SHALL produce control and variant artifacts from one committed source revision. Both arms SHALL reserve, initialize, compare, own, and release three root slots and four QMD slots with the same ordering and observer work. The control SHALL select roots A/B as a repeating ring, the variant SHALL select roots A/B/C as a repeating ring, and both SHALL select QMDs X/Y/Z/W as a repeating ring for all 64 iterations.

#### Scenario: Decisive schedules are established
- **WHEN** records 1 through 5 are generated in either arm
- **THEN** the control selections are A/X, B/Y, A/Z, B/W, A/X; the variant selections are A/X, B/Y, C/Z, A/W, B/X; control first revisits a root at iteration 3, variant first revisits a root at iteration 4, and neither arm revisits a QMD before iteration 5

#### Scenario: Scaffolding is symmetric
- **WHEN** the paired artifacts are reviewed before hardware execution
- **THEN** both arms prove identical reservation, initialization, complete-copy comparison, diagnostic, ownership, lifetime, and teardown work for roots A/B/C and QMDs X/Y/Z/W even though control never selects root C

#### Scenario: A schedule or observer difference is detected
- **WHEN** either arm selects a different ring, omits a reserved slot, performs asymmetric experiment-only work, or revisits a QMD at or before either decisive first-root-revisit boundary
- **THEN** the experiment is `inconclusive` and no output is attributed to root reuse distance

### Requirement: Retained positions and one backing allocation are causally gated
The experiment SHALL prefer the retained allocation order root A, QMD X, QMD Y, root B, QMD Z, QMD W, root C so the established A/X/Y/B positions remain unchanged. Before hardware, it SHALL prove actual offsets, sizes, alignments, distinct mapped GPU VAs, capacity, one command-upload BO/NvMap backing object, ownership through completion, teardown, and absence of later upload-consumer displacement rather than deriving addresses from expected arithmetic.

#### Scenario: Candidate allocation fits the retained upload allocation
- **WHEN** root slots are 2,048 bytes each, QMD slots are 256 bytes each, and the reconstructed allocator preserves the candidate order
- **THEN** host evidence proves the actual 7,168-byte footprint, actual remaining capacity, root/QMD alignment, retained A/X/Y/B positions, appended Z/W/C positions, one backing object, and unchanged later consumers

#### Scenario: Actual allocation differs safely before implementation
- **WHEN** host reconstruction reports an offset, address, capacity, alignment, or later-consumer result different from the retained expectation while still using one backing allocation
- **THEN** the discrepancy is recorded and this OpenSpec is revised before interpretive hardware execution rather than silently substituting assumed addresses

#### Scenario: Isolation requires another backing allocation
- **WHEN** any arm crosses into a second BO/NvMap, changes backing identity relative to the paired arm, or cannot preserve ownership and lifetime through dispatch completion
- **THEN** causal review stops and no root-reuse-distance hardware classification is permitted until a separately reviewed design is accepted

### Requirement: Root contents and dependent QMD differences are exact
For all 64 records, both arms SHALL copy identical current root source bytes to every required root mapping in the same order and SHALL prove complete source-to-mapping equality. Each selected QMD SHALL encode the actual selected root GPU VA using authoritative generated layout information, exactly equal its selected 256-byte mapped copy, and equal the corresponding reference QMD outside every identified root-address field. The selected mapped QMD GPU VA SHALL equal the direct `PCAS` dispatch VA.

#### Scenario: A correlated record is causally valid
- **WHEN** a record reaches direct dispatch
- **THEN** all root mappings exactly equal the current source, the selected QMD decodes to the selected root GPU VA, complete QMD comparison is equal outside the generated root-address mask, generated and mapped QMD bytes are identical, selected and dispatched QMD VAs match, and copy-before-dispatch ordering is complete

#### Scenario: A dependent QMD field changes as required
- **WHEN** corresponding QMDs encode different selected root GPU VAs
- **THEN** the difference is treated only as a mechanically required consequence of root selection and full-payload comparison proves no bit outside the authoritative root-address field set changed

#### Scenario: Any byte or address prerequisite fails
- **WHEN** a root copy, QMD copy, root decode, outside-mask comparison, selected-to-dispatch address, or ordering check is missing or false
- **THEN** the run is `inconclusive` regardless of its pixel or checksum output

### Requirement: Selectors are strict and ordinary execution is unchanged
The paired artifacts SHALL use exact-value selectors `ROOT_ADDRESS_REUSE_DISTANCE_CONTROL` and `ROOT_ADDRESS_REUSE_DISTANCE_VARIANT`, with unambiguous matching runtime markers. Exactly one SHALL equal `1` in an experiment artifact. They SHALL be mutually exclusive and incompatible with every retained root-cache, QMD-upload, QMD-address, root-address, QMD-cache-bit, synchronization, or other intervention selector. Absent or zero new selectors SHALL preserve ordinary execution.

#### Scenario: Control selector is valid
- **WHEN** only `ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1` is configured
- **THEN** the three-root/four-QMD common scaffolding runs with the A/B control root ring and reports the selected path before device work

#### Scenario: Variant selector is valid
- **WHEN** only `ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1` is configured
- **THEN** the identical common scaffolding runs with the A/B/C variant root ring and reports the selected path before device work

#### Scenario: Selector configuration is malformed or contradictory
- **WHEN** a new selector is not exactly zero or one, both new selectors are enabled, or either is combined with any excluded experiment selector
- **THEN** the configuration fails with a distinct nonzero result before device work and cannot produce experiment evidence

#### Scenario: New selectors are absent
- **WHEN** neither new selector is enabled
- **THEN** no three-root/four-QMD reservation, experimental selection, copy, diagnostic, or dispatch behavior is introduced into ordinary execution

### Requirement: Every non-target workload semantic remains frozen
Both arms SHALL preserve 64 iterations; the seed formula; root semantic contents, size, alignment, and source bytes; QMD construction other than the required root encoding; shaders and generated headers; pipeline and resource state; images, formats, descriptors, samplers, layouts, and barriers; command-buffer reset/rerecord behavior; GPU method sequence and dispatch dimensions; direct `PCAS`, `SEND_PCAS_A`, `SEND_SIGNALING_PCAS_B`, and existing SKED initialization; submission, waits, CPU cache behavior, backing, readback, oracle, and teardown. QMD v0.6 `INVALIDATE_SHADER_CONSTANT_CACHE` SHALL remain false in both arms.

#### Scenario: Frozen-semantics review passes
- **WHEN** control and variant source and artifacts are reviewed before hardware
- **THEN** selected root schedule plus its required QMD root encoding is the only causal semantic difference and all frozen inputs, methods, hashes, synchronization, and observer work match

#### Scenario: A prohibited combined intervention is present
- **WHEN** either arm changes a QMD cache/data/texture invalidation field, method-level shader-cache or SKED invalidation, QMD membar, WFI, serialization, CPU flush, GPU invalidation, barrier, queue/device wait, submission count, fence behavior, sleep, nonce, root contents, backing, allocator policy, TLB state, or FG-3/FG-4 behavior
- **THEN** hardware execution stops or the completed run is `inconclusive`, and the added intervention requires a separate OpenSpec change

### Requirement: Reuse distance is measured from actual dispatched addresses
For every record, the experiment SHALL calculate root and QMD reuse distance, first-revisit state, and first-revisit iteration from actual previously dispatched GPU VAs rather than slot labels. Correlation SHALL include record, iteration, seed, arm, all root and QMD VAs, previous/selected VAs, root/QMD last-used distance, first-revisit flags, copies, QMD decode and mask decisions, direct dispatch, ordering, observed and expected pixel/checksum, oracle result, fault state, and teardown state.

#### Scenario: Complete 64-record evidence is produced
- **WHEN** either artifact completes
- **THEN** all 64 records contain joined root/QMD/dispatch/oracle decisions, detailed evidence covers at least iterations 1 through 5, aggregates identify the exact first root and QMD revisit iterations and reuse distances, and complete teardown and interpretable fault state are present

#### Scenario: Bounded diagnostics preserve symmetry
- **WHEN** diagnostics are enabled in the pair
- **THEN** both arms use fixed bounded state and the same experiment-only observer work without dynamic allocation in the dispatch-critical path or high-volume SD-only logging

#### Scenario: Records cannot exclude a false positive
- **WHEN** a record is missing, out of order, inferred from a slot name, ambiguously joined, inconsistent with its aggregate, lacks a complete-copy decision, or has incomplete fault or teardown evidence
- **THEN** the experiment is `inconclusive` even if the output matches a predicted signature

### Requirement: A valid control gates variant execution and interpretation
The control SHALL execute first on real Tegra hardware. It is valid only if iteration 1 seed 5 is exact at pixel `0xfa47d33f` and checksum `0xb7d223e5`, iteration 2 seed 42 is exact at pixel `0xf5031a17` and checksum `0x0daf4ac5`, iteration 3 selects root A with QMD Z as the first root revisit and first failure, and iterations 3-64 retain pixel `0xf5031a17` and checksum `0x0daf4ac5`, with every causal prerequisite and no unexplained fault.

#### Scenario: Control reproduces the decisive boundary
- **WHEN** the complete unfiltered control stream proves A/X, B/Y, A/Z, B/W through iteration 4; first root revisit at iteration 3; no QMD revisit before iteration 5; exact copies, decode, mask, dispatch, ordering, all 64 results, and teardown; and no unexplained timeout, notifier, error-info event, or GPU fault
- **THEN** the variant is authorized on the same intended configuration

#### Scenario: Control deviates
- **WHEN** the control consumes iteration 3's current seed, produces another signature, faults, changes its schedule/backing/observer behavior, or fails any causal or evidence prerequisite
- **THEN** the experiment is `inconclusive`, the changed control evidence is retained, the variant is neither authorized nor interpreted, and no second hypothesis is combined

### Requirement: Hardware outcomes are classified exactly and narrowly
After a valid control and a causally valid variant, the experiment SHALL apply exactly one of `root_address_reuse_distance_hypothesis_supported_experiment_only`, `specific_root_address_reuse_distance_change_insufficient`, `behavior_changed_unresolved`, or `inconclusive`. Post-iteration-4 behavior SHALL NOT support an independent QMD-reuse claim because QMD reuse begins at iteration 5.

#### Scenario: Root reuse-distance hypothesis is supported experimentally
- **WHEN** the variant proves A/X, B/Y, C/Z, A/W through iteration 4; consumes seeds 5, 42, and 79 exactly; first fails on root A's iteration-4 first revisit while QMD W is new; retains the immediately preceding seed-79 pixel `0xf0bf610f` and checksum `0x1fcdf2e5`; all 64 records and causal prerequisites pass; and no unexplained fault occurs
- **THEN** the result is `root_address_reuse_distance_hypothesis_supported_experiment_only`, meaning only that increasing the tested root-address reuse distance shifted stale-state onset with the first root revisit in this controlled experiment

#### Scenario: Specific reuse-distance change is insufficient
- **WHEN** the control is valid, the variant proves its intended root schedule and no QMD reuse through iteration 4, the failure boundary does not shift as predicted, and output retains the relevant known stale signature without another material behavior change
- **THEN** the result is `specific_root_address_reuse_distance_change_insufficient` and only the tested three-root reuse-distance intervention is rejected as sufficient

#### Scenario: Behavior changes without the exact phase shift
- **WHEN** the control is valid and the causally valid variant changes failure iteration, retained seed, output signature, or later transitions without producing the complete predefined phase shift and oracle
- **THEN** the result is `behavior_changed_unresolved` and the exact signature is retained without adding another intervention

#### Scenario: A causal or evidence prerequisite fails
- **WHEN** control validity, schedule, backing, root/QMD copies, root decode, outside-field equality, QMD-to-`PCAS` correlation, ordering, observer symmetry, provenance, complete records, teardown, or interpretable fault state is absent or contradicted
- **THEN** the result is `inconclusive` and no hardware hypothesis or project capability is promoted

### Requirement: Immutable evidence and non-promotion rules are enforced
Interpretive hardware execution SHALL require strict OpenSpec validation, repository policy checks, durable-patch reconstruction from pristine Mesa 25.0.7, regenerated-source byte equivalence, selector and negative-path tests, actual allocation/backing/lifetime proof, 64-record schedule simulations, frozen-semantics review, committed immutable source, and reproducible artifact, patch, and shader hashes. Complete unfiltered control and authorized variant logs SHALL be inspected before filtering and retained according to `docs/testing/HARDWARE_EVIDENCE.md`.

#### Scenario: Hardware is unavailable
- **WHEN** implementation and every host gate pass but no authorized real-Tegra execution occurs
- **THEN** the implementation stops at `IMPLEMENTED_UNPROVEN` and `hardware-ready` with an explicit statement that no device was contacted

#### Scenario: Experimental support is observed
- **WHEN** the result is `root_address_reuse_distance_hypothesis_supported_experiment_only`
- **THEN** FG-2 remains `BLOCKED`, the three-root ring is not promoted into production allocator policy, and a separate remediation decision plus separate ordinary image-chain 64/64 hardware acceptance change is required before capability promotion

#### Scenario: Any hardware outcome is retained
- **WHEN** the control-only or paired experiment concludes
- **THEN** complete raw logs, a full hardware record, a narrow research finding, and justified milestone/capability wording preserve all prior evidence; the experimental path remains disabled or is rolled back; FG-2 remains `BLOCKED`; and FG-3/FG-4 work does not begin
