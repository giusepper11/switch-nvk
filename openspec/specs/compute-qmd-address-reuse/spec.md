# compute-qmd-address-reuse Specification

## Purpose

This capability determines whether avoiding compute-QMD GPU-address reuse changes the established FG-2 stale-result behavior while holding the existing render-to-sampled-image-to-compute workload and all other relevant semantics fixed.

## Requirements

### Requirement: A same-source paired experiment preserves the frozen FG-2 workload
The experiment SHALL provide a control and a variant from one committed source revision. Both paths SHALL preserve the reused root-address behavior, changing seed sequence, naturally generated 256-byte QMD contents and copy semantics, shaders and generated shader bytes, images, descriptors, pipeline layout, barriers and layouts, command-buffer reset/rerecord lifecycle, direct-dispatch method sequence and semantics, submission and wait behavior, readback, and independent 64-iteration oracle. The only intended semantic difference in the variant SHALL be the QMD GPU address operand required to select storage fresh relative to the preceding dispatch.

#### Scenario: Control reproduces the retained baseline
- **WHEN** the control executes all 64 iterations on real Tegra hardware
- **THEN** iteration 1 seed 5 passes exactly, iterations 2-64 reproduce the retained pixel `0xfab61a38` and checksum `0xc17a35a5` stale signature, the ordinary QMD address is reused according to current allocator behavior, and no unexplained GPU fault occurs

#### Scenario: Control does not reproduce the retained baseline
- **WHEN** the control changes the first-iteration result, the iterations-2-64 signature, relevant address/copy/dispatch behavior, or another frozen workload property
- **THEN** the paired comparison is `inconclusive`, the variant cannot support a causal conclusion, and the deviation is retained as an observer-effect or control-invalid result

#### Scenario: Pair differs beyond QMD address selection
- **WHEN** source or artifact review finds a shader, resource, descriptor, layout, barrier, command-buffer, dispatch-method, submission, wait, readback, oracle, QMD payload, or root-address semantic difference beyond selection/reporting and the required QMD address operand
- **THEN** the pair is rejected as a causal comparison and no address-reuse conclusion is drawn

### Requirement: The variant proves QMD-address freshness for the complete oracle
For every variant transition from iteration 1 to iteration 2 through iteration 63 to iteration 64, the current dispatched QMD GPU VA SHALL differ from the immediately preceding dispatched QMD GPU VA while the root GPU VA behavior remains the same as the valid control. Freshness SHALL be established from the address of the exact mapped QMD copy and the address selected by direct dispatch, not inferred from allocation calls or CPU pointers.

#### Scenario: Every required transition is fresh
- **WHEN** the variant completes the 64-iteration run
- **THEN** all 63 consecutive QMD-address transitions are recorded as fresh, every current mapped QMD address equals the corresponding dispatched `PCAS` address, and the run remains eligible for outcome classification

#### Scenario: A QMD address is reused or cannot be correlated
- **WHEN** any iteration after the first reuses the preceding dispatched QMD VA, lacks a previous/current address, or cannot correlate the mapped QMD to direct dispatch
- **THEN** QMD freshness is unproven, the run is `inconclusive`, and no output difference is attributed to QMD-address reuse

#### Scenario: Safe allocation cannot guarantee all 63 fresh transitions
- **WHEN** the smallest safe QMD-storage strategy cannot guarantee freshness relative to the preceding dispatch without changing another semantic variable
- **THEN** the limitation is documented before hardware execution, the full 64-iteration causal acceptance classification is unavailable, and no weaker address pattern is silently treated as equivalent

### Requirement: Correlation is complete, exact, and bounded
The experiment SHALL correlate iteration and seed; root source and mapped seed; previous/current root GPU VA and reuse state; complete generated and mapped 256-byte QMD identities; exact source-to-mapped and required cross-path/cross-iteration QMD byte comparisons; previous/current QMD GPU VA and freshness state; dispatched QMD VA and `PCAS` correlation; ordering; control/variant selector state; output pixel, checksum, and oracle result; and GPU fault/error state. Exact byte comparison SHALL be authoritative for QMD equality; hashes MAY be emitted only as compact identities.

#### Scenario: Correlation record is valid
- **WHEN** either artifact records a compute dispatch
- **THEN** its bounded evidence unambiguously joins the application iteration and seed to the root upload, QMD source and mapped copy, selected QMD storage, direct-dispatch address, output validation, selector, order, and fault state

#### Scenario: QMD copy and contents remain invariant
- **WHEN** corresponding captured QMDs are compared within and across the control and variant
- **THEN** all 256 source bytes exactly match their mapped copies, naturally generated QMD contents remain exact where required for the causal pair, and any compact identity agrees with the full-byte decision

#### Scenario: QMD contents unexpectedly differ
- **WHEN** a source-to-mapped comparison fails or corresponding naturally generated QMD payloads differ unexpectedly
- **THEN** the run is `inconclusive`, the exact differing evidence is retained, and address freshness is not treated as the sole semantic variable

#### Scenario: Diagnostics remain bounded
- **WHEN** the 64-iteration artifact runs
- **THEN** detailed payload/address records and compact per-iteration freshness/oracle evidence have fixed limits, avoid dynamic allocation or unbounded per-frame SD writes in the traced path, and are sufficient to prove all 63 freshness transitions without changing allocation behavior

### Requirement: The variant changes no cache, synchronization, or payload semantics
Neither path SHALL add a CPU cache flush, GPU cache invalidation, barrier, synchronization operation, wait, submission change, payload nonce, QMD field change unrelated to storage placement, root-address change, or alternate dispatch mechanism. GPU command structure, method sequence, and semantics SHALL remain unchanged except for the QMD address operand necessarily changed by the variant; the experiment SHALL NOT claim byte-identical GPU commands across that operand difference.

#### Scenario: Variant isolation passes host review
- **WHEN** the same-source control and variant are prepared for hardware execution
- **THEN** source review and artifact/shader provenance confirm the prohibited operations are absent and identify only selection/reporting plus the QMD storage/address-selection mechanism as relevant differences

#### Scenario: An excluded semantic change is detected
- **WHEN** either path performs an added cache operation, invalidation, barrier, synchronization, wait, payload modification, root-address intervention, or dispatch-method change
- **THEN** the experiment is `inconclusive` and cannot support or reject the QMD-address-reuse hypothesis

### Requirement: Hardware outcomes are classified narrowly
After a valid control and a variant with proven freshness, the experiment SHALL emit exactly one applicable classification based on the complete independent 64-iteration oracle and fault state. No classification SHALL generalize beyond the particular fresh-address intervention tested.

#### Scenario: Fresh-address variant passes the complete oracle
- **WHEN** the control is valid, all 63 variant transitions prove QMD-address freshness, all 64 variant iterations pass the independent exact-pixel oracle, required correlation is complete, and no unexplained GPU fault occurs
- **THEN** the result is `qmd_address_reuse_hypothesis_supported_experiment_only`, meaning only that avoiding QMD-address reuse supports the tested hypothesis in this experiment and is not a production fix or FG-2 proof

#### Scenario: Fresh-address variant retains the exact stale signature
- **WHEN** the control is valid, QMD-address freshness is proven, iterations 2-64 retain the exact established stale pixel/checksum signature, and no relevant GPU fault occurs
- **THEN** the result is `specific_qmd_address_change_insufficient` and does not reject all QMD, launch-state, cache, allocator, or GPU-consumption explanations

#### Scenario: Fresh-address variant changes behavior without passing
- **WHEN** the control is valid, QMD-address freshness is proven, and the output differs from the retained stale signature but fails any part of the complete oracle
- **THEN** the result is `behavior_changed_unresolved`, the exact new behavior is retained, and a smaller separately specified follow-up is required before a causal conclusion

#### Scenario: A prerequisite or evidence gate fails
- **WHEN** control validity, all-transition QMD freshness, address/dispatch correlation, QMD equality, observer-effect isolation, required evidence, or interpretable fault state is missing or inconsistent
- **THEN** the result is `inconclusive` and no hardware hypothesis or capability status is promoted

### Requirement: Evidence is immutable and experiment success does not promote FG-2
Material conclusions SHALL require committed source, hashes for both artifacts and generated shaders, recorded hardware/environment/build context, complete unfiltered logs inspected before filtering, exact per-iteration output validation, bounded correlation, and GPU fault/error state according to `docs/testing/HARDWARE_EVIDENCE.md`. FG-2 SHALL remain `BLOCKED` after this experiment regardless of outcome, FG-3 SHALL remain out of scope, and ordinary runtime behavior SHALL NOT adopt the variant from this experiment.

#### Scenario: Experiment-only support is recorded
- **WHEN** the result is `qmd_address_reuse_hypothesis_supported_experiment_only`
- **THEN** the evidence and research finding state that a later independently specified and executed original unmodified image-chain acceptance run must pass before FG-2 can advance

#### Scenario: Only host or emulator evidence exists
- **WHEN** the artifacts build, pass static checks, or run outside real Tegra hardware without the required hardware record
- **THEN** the implementation remains `IMPLEMENTED_UNPROVEN` and no QMD-consumption or FG-2 hardware claim is made

#### Scenario: Experiment concludes without production promotion
- **WHEN** any hardware outcome is recorded
- **THEN** the variant remains opt-in or is rolled back, retained negative evidence is preserved, source-of-truth documents name QMD-address reuse as the tested immediate boundary without upgrading FG-2, and no FG-3 work begins

