## Purpose

This capability determines whether reused compute QMD storage contains meaningfully changed payload bytes before allowing a QMD-only CPU cache-flush experiment, so real-hardware runs distinguish an informative visibility test from flushing identical data.

## ADDED Requirements

### Requirement: QMD identity capture is opt-in, bounded, and observation-only
The diagnostic path SHALL be disabled for ordinary runs, SHALL bound detailed QMD identity capture to the first two correlated FG-2 iterations with an optional third confirmation, and SHALL preserve the existing shaders, resources, command stream, synchronization, dispatch, and 64-iteration oracle.

#### Scenario: Identity diagnostic is enabled
- **WHEN** the dedicated QMD identity artifact runs with a valid diagnostic selector
- **THEN** it reports the selected path, emits bounded QMD identity records, and executes the unchanged FG-2 validation without adding a cache operation, wait, GPU command, allocation-policy change, or shader behavior change

#### Scenario: Identity diagnostic is disabled
- **WHEN** the selector is absent, disabled, or invalid
- **THEN** no QMD identity or QMD cache operation is performed and the ordinary compute upload and dispatch path remains selected

### Requirement: Generated and mapped QMD payloads are correlated completely
For every captured dispatch, the diagnostic SHALL compute deterministic identity values over all 256 QMD bytes both before and immediately after the upload copy, SHALL compare the two payloads for exact equality, and SHALL correlate them with iteration, root GPU address, previous and current QMD GPU addresses, QMD-address reuse, and the address selected by direct dispatch. The record SHALL state that CPU-mapped equality does not prove GPU visibility.

#### Scenario: Generated QMD is copied exactly
- **WHEN** a captured QMD is generated and copied to its mapped upload range
- **THEN** the record reports the source identity, mapped identity, exact copy-match result, byte count, CPU mapping, QMD GPU address, root GPU address, dispatched QMD address, and correlation/order result

#### Scenario: Source and mapped QMD differ
- **WHEN** the mapped 256-byte upload does not exactly match the generated QMD before any cache operation
- **THEN** the run is inconclusive, no QMD cache flush is permitted, and no GPU-visibility conclusion is drawn

### Requirement: Payload identity determines cache-flush eligibility
The experiment SHALL compare the complete generated QMD payloads for the first two correlated iterations. A QMD-only cache-flush variant SHALL be eligible only when the disabled control is valid, the QMD GPU address is reused, each source-to-mapping copy is exact, and the later generated payload differs from the preceding payload without an artificial nonce or unrelated semantic change.

#### Scenario: Reused QMD payload changes naturally
- **WHEN** iteration 2 reuses iteration 1's QMD GPU address, both QMD copies are exact, and the complete generated payload identity changes because of the existing workload state
- **THEN** the record classifies the QMD-only cache-flush variant as eligible and identifies the differing payload identities without claiming which field or GPU mechanism causes stale behavior

#### Scenario: Reused QMD payload is identical
- **WHEN** iteration 2 reuses iteration 1's QMD GPU address and the complete generated QMD payload is byte-for-byte identical
- **THEN** the experiment classifies a CPU flush of that identical QMD range as non-discriminating, performs no QMD cache flush, records the identity finding, and names only a separately specified QMD-address-reuse or GPU-consumption experiment as the possible next step

#### Scenario: QMD address is not reused
- **WHEN** the later correlated QMD upload receives a different GPU address
- **THEN** the experiment records the address change, marks the reuse-specific cache-flush hypothesis ineligible, and does not infer cache behavior

### Requirement: Eligible variant flushes only the reused QMD upload
When the eligibility contract is satisfied and the enabled selector is valid, the experiment SHALL invoke one CPU data-cache flush over exactly the current 256-byte QMD CPU mapping after the QMD copy and identity check and before the unchanged direct dispatch. It SHALL NOT flush the root upload or any other allocation, widen the requested range to the upload arena, add GPU invalidation or synchronization, force a fresh allocation, or modify QMD bytes.

#### Scenario: Eligible QMD-only flush executes
- **WHEN** an enabled iteration satisfies every payload, copy, address-reuse, ordering, and control prerequisite
- **THEN** the record reports one targeted QMD cache invocation with the CPU mapping, requested byte range, QMD GPU address, previous QMD address, source and mapped identities, root GPU address, and dispatched QMD address

#### Scenario: Any eligibility prerequisite fails
- **WHEN** payload identity, copy equality, QMD-address reuse, ordering, selector state, or control validity is missing or inconsistent
- **THEN** no QMD cache operation is performed and the run is classified as ineligible or inconclusive with an explicit reason

### Requirement: Disabled control and enabled variant preserve the frozen baseline
The paired artifacts SHALL be built from one committed source revision and SHALL preserve identical shader bytes, QMD generation and copy semantics, images, descriptors, pipeline layout, seed sequence, barriers, command-buffer reset/rerecord behavior, GPU commands, direct dispatch, submission/wait behavior, readback, and 64-iteration validation except for selection/reporting and the eligible targeted QMD cache operation. Any local expansion of the existing QMD upload helper into equivalent allocation and copy operations SHALL be present in both artifacts.

#### Scenario: Paired artifacts pass host review
- **WHEN** the disabled control and enabled variant are prepared for hardware execution
- **THEN** their identities and generated shader hashes are recorded and source/artifact review confirms that the QMD mapping exposure is equivalent in both while no root flush, allocation-policy change, GPU command, wait, or unrelated cache operation differs

#### Scenario: Disabled control changes the retained baseline
- **WHEN** the disabled control does not reproduce the established first-iteration pass and stable iterations 2-64 stale signature or otherwise changes relevant execution behavior
- **THEN** the comparison is invalidated as a control or observer-effect failure and the enabled result is not used for a cache conclusion

### Requirement: Real-hardware evidence supports only the narrow observed outcome
Material conclusions SHALL require committed source, hashed artifacts, complete unfiltered real-Tegra logs, exact 64-iteration output validation, bounded identity and address correlation, and recorded GPU fault/error state. Host or emulator evidence SHALL NOT establish QMD cache visibility or FG-2 success.

#### Scenario: Identity-only run finds identical reused payloads
- **WHEN** valid real-hardware evidence shows byte-identical generated and mapped QMD payloads at the reused address while the stale signature remains
- **THEN** the finding records QMD payload identity as `PROVEN_HW`, records the proposed identical-payload flush as non-discriminating rather than rejected, keeps FG-2 `BLOCKED`, and stops without running or promoting a QMD cache variant

#### Scenario: Eligible enabled variant passes all iterations
- **WHEN** a valid disabled control reproduces the baseline and the eligible enabled variant produces the independent oracle for all 64 iterations with the targeted path confirmed and no unexplained fault
- **THEN** the finding records that the reused-QMD-range CPU flush supports the tested visibility hypothesis, but does not call it a production fix or mark the unmodified FG-2 chain proven

#### Scenario: Eligible enabled variant retains the exact stale signature
- **WHEN** the valid enabled run invokes the targeted flush yet reproduces the exact disabled-control output without a GPU fault
- **THEN** the finding records `specific_qmd_flush_insufficient`, keeps the remaining boundary unresolved, and does not generalize the result to all CPU or GPU cache behavior

#### Scenario: Eligible enabled variant changes output without passing
- **WHEN** the enabled output differs from the valid control but does not satisfy the complete oracle
- **THEN** the exact new signature is retained as evidence of an effect and no supported, rejected, or fix conclusion is made until a smaller follow-up experiment is specified

#### Scenario: Evidence is incomplete or faulted
- **WHEN** a required identity, address, ordering, invocation, output, complete-log, or fault-state record is missing or inconsistent
- **THEN** the run remains inconclusive and no hardware cache claim is promoted
