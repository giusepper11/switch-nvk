## Purpose

This capability determines whether compute-root GPU virtual-address reuse is sufficient to explain the retained FG-2 stale result while keeping fresh-QMD dispatch and every unrelated workload semantic controlled.

## Requirements

### Requirement: A same-source pair preserves the frozen FG-2 workload
The experiment SHALL produce a control and variant from one committed source revision. Both arms SHALL preserve the ordinary changing seed sequence; root semantic contents, size, and alignment; current shaders and generated shader bytes; images; descriptors; pipeline layouts; barriers and layout transitions; command-buffer reset and rerecord behavior; GPU method sequence; dispatch dimensions; submission; waits; readback; cache behavior; synchronization; and independent 64-iteration exact-pixel oracle. Both arms SHALL use QMD storage fresh relative to the immediately preceding dispatch. The only intended independent semantic difference in the variant SHALL be selection of a compute-root GPU VA fresh relative to the immediately preceding dispatch; the QMD root-address encoding change required by that selection SHALL be treated as dependent.

#### Scenario: Control reproduces the retained baseline
- **WHEN** the control executes all 64 iterations on real Tegra hardware
- **THEN** iteration 1 passes exactly with pixel `0xfa47d33f` and checksum `0xb7d223e5`, iterations 2-64 reproduce pixel `0xfab61a38` and checksum `0xc17a35a5`, the selected root GPU VA is reused on all 63 transitions, the QMD GPU VA is fresh on all 63 transitions, all 64 QMD dispatch correlations are exact, and no unexplained GPU fault occurs

#### Scenario: Control is invalid
- **WHEN** the control changes the retained output signature, fails any expected root-reuse or QMD-freshness transition, lacks exact QMD dispatch correlation, changes another frozen semantic, or reports an unexplained fault
- **THEN** the experiment is `inconclusive`, the deviation is retained, and the variant cannot support a causal classification

#### Scenario: Pair differs beyond the isolated variable
- **WHEN** source, artifact, or runtime evidence finds an unrelated shader, resource, descriptor, root-content, allocation-order, layout, barrier, command-buffer, GPU-method, submission, wait, readback, oracle, cache, or synchronization difference between the arms
- **THEN** the experiment is `inconclusive` and no compute-root-address conclusion is drawn

### Requirement: The variant proves compute-root address freshness for the complete oracle
For every transition from iteration 1 to 2 through iteration 63 to 64, the variant SHALL select a root GPU VA different from the immediately preceding dispatch while the control SHALL select the ordinary reused root GPU VA. Freshness SHALL be established from the exact mapped allocation encoded into the QMD, not inferred from an allocation call, CPU pointer, or mapped-byte equality. The paired allocation and copy pattern SHALL be identical between arms, bounded to the 64-iteration smoke test, and SHALL report its maximum additional live memory and cleanup boundary.

#### Scenario: Every required root transition is fresh
- **WHEN** the variant completes the 64-iteration run
- **THEN** all 63 consecutive root-address transitions are recorded as fresh, all current root source bytes exactly equal the selected mapped bytes, and every QMD decodes to the current selected root GPU VA

#### Scenario: Root freshness is missing or uncorrelated
- **WHEN** any post-first variant dispatch reuses the immediately preceding root GPU VA, lacks previous/current address evidence, has a source-to-mapping mismatch, or cannot correlate the selected mapping to QMD encoding
- **THEN** root-address freshness is unproven and the experiment is `inconclusive`

#### Scenario: Bounded allocation cannot isolate the variable
- **WHEN** the smallest experiment-only allocation strategy cannot guarantee 63/63 root-address transitions while preserving QMD freshness and all frozen workload semantics
- **THEN** the limitation is documented before hardware execution and no broader allocator or synchronization intervention is substituted

#### Scenario: Ordinary execution is selected
- **WHEN** the dedicated control and variant selectors are absent, zero, malformed, contradictory, or combined with an excluded experiment selector
- **THEN** malformed or contradictory experiment builds are rejected as configured, all other non-enabled execution retains the ordinary root/QMD allocation path, and no experimental allocation policy is applied

### Requirement: QMD freshness is held constant and root encoding changes only as required
Both arms SHALL prove QMD GPU-address freshness on all 63 adjacent transitions, exact source-to-selected-mapping equality for all 64 QMDs, and exact selected-QMD-to-direct-dispatch address correlation for all 64 dispatches. Complete QMD payload comparison SHALL be authoritative and SHALL prove that any control/variant or primary/alternate-root difference is entirely attributable to the encoded root-address field or fields identified by the QMD layout; hashes MAY be used only as compact identities.

#### Scenario: Dependent QMD encoding is isolated
- **WHEN** a QMD is generated for a selected root GPU VA
- **THEN** its decoded root address exactly equals that allocation, all bits outside the identified root-address encoding field or fields exactly match the corresponding reference QMD, and normal QMD generation and copy semantics are preserved

#### Scenario: QMD freshness and dispatch correlation are complete
- **WHEN** either arm completes its 64-iteration run
- **THEN** it reports 63/63 adjacent QMD-address transitions, 64/64 exact generated-to-mapped copies, and 64/64 selected mapped QMD addresses equal to the direct-dispatch `PCAS` addresses

#### Scenario: An unexplained QMD difference occurs
- **WHEN** a complete QMD comparison finds any changed bit outside the identified encoded root-address field or fields, a root address decodes incorrectly, a source-to-mapping comparison fails, or a QMD/dispatch address does not correlate
- **THEN** the experiment is `inconclusive` and the exact differing evidence is retained

### Requirement: Correlation is complete, exact, and bounded
The experiment SHALL correlate all 64 iterations with: expected seed; root source and selected mapped content identities and exact equality; previous/current root GPU VAs and reuse/freshness; QMD source and selected mapped identities and exact equality; complete QMD-difference attribution; previous/current QMD GPU VAs and freshness; decoded QMD root GPU VA; selected direct-dispatch QMD VA and `PCAS`; selector/path state; ordering; output pixel and checksum; exact oracle result; and GPU fault/error state. CPU-mapped equality SHALL NOT be described as proof of GPU visibility.

#### Scenario: A correlation record is valid
- **WHEN** either artifact records a compute dispatch
- **THEN** its bounded evidence unambiguously joins the application iteration and expected seed to the selected root mapping, dependent QMD encoding, selected QMD mapping, direct dispatch, output validation, selector, ordering, and fault state

#### Scenario: Diagnostics remain bounded and symmetric
- **WHEN** the 64-iteration pair runs
- **THEN** both arms use the same fixed diagnostic structure, avoid dynamic diagnostic allocation in the critical dispatch loop, cap detailed payload records while retaining full in-process comparisons and all per-iteration address correlations, and avoid unbounded or high-volume per-frame SD logging

#### Scenario: Correlation is incomplete
- **WHEN** any required iteration, address transition, byte-equality decision, dependent-difference attribution, dispatch link, oracle result, ordering marker, selector state, or interpretable fault state is missing or inconsistent
- **THEN** the experiment is `inconclusive` regardless of observed output

### Requirement: The experiment adds no cache, synchronization, or timing intervention
Neither arm SHALL add a CPU cache flush, GPU cache invalidation, memory barrier, synchronization operation, queue or device idle call, submission, payload nonce, artificial QMD mutation, sleep, timing perturbation, or unrelated allocator change. The experiment SHALL preserve existing waits and SHALL NOT change asynchronous submission or native fence semantics.

#### Scenario: Isolation passes source and artifact review
- **WHEN** the paired artifacts are prepared for hardware execution
- **THEN** review confirms that the root selection and its mechanically required QMD root encoding are the only semantic differences and that all prohibited interventions are absent

#### Scenario: An excluded intervention is detected
- **WHEN** either arm adds or changes an excluded cache, synchronization, timing, payload, allocator, submission, wait, or fence behavior
- **THEN** the experiment is `inconclusive` and the intervention is not interpreted as evidence about root-address reuse

### Requirement: Hardware outcomes are classified narrowly
After a valid control and a variant with complete root/QMD/dispatch correlation, the experiment SHALL emit exactly one applicable outcome. No outcome SHALL generalize beyond the tested adjacent root-address intervention.

#### Scenario: Fresh-root variant passes the complete oracle
- **WHEN** the control is valid, the variant proves 63/63 root-address transitions, QMD freshness and dependent encoding remain valid, all 64 variant iterations pass the independent exact oracle, and no unexplained GPU fault occurs
- **THEN** the result is `root_address_reuse_hypothesis_supported_experiment_only`, meaning only that the tested root-address-reuse hypothesis is supported and not that a production fix or FG-2 proof exists

#### Scenario: Fresh-root variant retains the exact stale signature
- **WHEN** the control is valid, root-address freshness and all QMD/dispatch prerequisites are proven, iterations 2-64 retain pixel `0xfab61a38` and checksum `0xc17a35a5`, and no relevant GPU fault occurs
- **THEN** the result is `specific_root_address_change_insufficient` and does not reject all constant-buffer, cache, descriptor, allocator, launch-state, address-translation, shader-execution, sampled-image, or GPU-consumption explanations

#### Scenario: Fresh-root variant changes behavior without passing
- **WHEN** the control is valid, root/QMD/dispatch correlation is complete, and output differs from the retained stale signature but fails any part of the complete oracle
- **THEN** the result is `behavior_changed_unresolved`, the exact new signature is retained, and only a smaller separately specified follow-up is permitted

#### Scenario: A causal prerequisite fails
- **WHEN** control validity, all-transition root or QMD freshness, byte equality, dependent QMD-difference attribution, root encoding, dispatch correlation, observer-effect isolation, complete hardware evidence, or interpretable fault state is absent or contradicted
- **THEN** the result is `inconclusive` and no hardware hypothesis or capability status is promoted

### Requirement: Evidence is immutable and experimental success does not promote FG-2
Material conclusions SHALL require committed immutable source; exact control, variant, and generated-shader hashes; recorded build and hardware context; complete unfiltered logs inspected before filtering; all 64 iterations; exact output validation; full root/QMD/dispatch correlation; and GPU notifier/error review according to `docs/testing/HARDWARE_EVIDENCE.md`. FG-2 SHALL remain `BLOCKED` after this change regardless of outcome, FG-3 SHALL remain out of scope, and experimental selectors or allocation behavior SHALL NOT be promoted into ordinary execution.

#### Scenario: Hardware execution is gated
- **WHEN** design, implementation, host/static/build validation, immutable-source provenance, artifact hashes, shader identities, and causal source/artifact review are incomplete
- **THEN** neither artifact is authorized for interpretive hardware execution and no result is classified from host or emulator evidence

#### Scenario: Control gates the variant
- **WHEN** the control has not completed with the exact retained baseline, required root/QMD/dispatch evidence, complete-log review, and no unexplained fault
- **THEN** the variant is not authorized and the experiment is retained as `inconclusive`

#### Scenario: Experiment-only support is recorded
- **WHEN** the result is `root_address_reuse_hypothesis_supported_experiment_only`
- **THEN** the project requires a separately reviewed ordinary-path remediation decision followed by an original unmodified image-chain artifact and real-hardware 64/64 acceptance run before FG-2 can be considered for promotion

#### Scenario: Experiment concludes without production promotion
- **WHEN** any hardware outcome is recorded
- **THEN** raw logs and narrow findings are retained, prior negative evidence is preserved, current source-of-truth wording is reconciled, the experiment path is disabled or rolled back, FG-2 remains `BLOCKED`, and no FG-3 work begins
