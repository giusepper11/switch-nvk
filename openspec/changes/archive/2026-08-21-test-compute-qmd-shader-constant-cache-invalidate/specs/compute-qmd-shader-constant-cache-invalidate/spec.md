## Purpose

Determine whether pre-Pascal QMD v0.6 shader-constant-cache invalidation is sufficient to change the retained FG-2 stale compute-root behavior in an exactly isolated, hardware-gated one-bit experiment.

## ADDED Requirements

### Requirement: Retained QMD evidence gates the intervention
Before implementation or hardware authorization, the experiment SHALL verify from a retained complete 256-byte QMD payload that `INVALIDATE_SHADER_CONSTANT_CACHE` bit 255 equals zero in the established failure-producing path. The decision SHALL decode bit 255 as dword 7 bit 31 and SHALL use the retained payload bytes rather than a hash, source default, or prompt summary.

#### Scenario: Retained field is zero
- **WHEN** retained reference, selected, and mapped QMD payloads show dword 7 bit 31 clear
- **THEN** the one-bit control/variant experiment is eligible to proceed and records the precheck evidence location and decoded value

#### Scenario: Retained field is already one
- **WHEN** any authoritative retained failure-producing QMD shows dword 7 bit 31 set
- **THEN** implementation and hardware execution stop, the already-present intervention is documented, and root reuse distance MAY be proposed only as a separate change

#### Scenario: Retained payload is incomplete or inconsistent
- **WHEN** the complete relevant QMD cannot be recovered or reference, selected, and mapped copies disagree at the field
- **THEN** the experiment is blocked from implementation or hardware interpretation until the evidence prerequisite is repaired

### Requirement: A same-source pair changes exactly one QMD field
The experiment SHALL produce control and variant artifacts from one committed source revision. Both arms SHALL use the same 64-iteration root A/B and QMD X/Y alternating schedule retained from the root-address experiment. The control SHALL generate QMD v0.6 `INVALIDATE_SHADER_CONSTANT_CACHE=FALSE`; the variant SHALL generate `INVALIDATE_SHADER_CONSTANT_CACHE=TRUE`. For each corresponding iteration, this field SHALL be the only QMD bit and only workload semantic that differs between arms.

#### Scenario: Paired one-bit isolation passes
- **WHEN** corresponding control and variant QMDs are compared across all 64 iterations
- **THEN** each complete 256-byte exclusive-or equals exactly bit 255, the field decodes as zero in control and one in variant, and every other QMD bit is equal

#### Scenario: Another QMD bit differs
- **WHEN** any paired full-payload comparison contains a difference outside bit 255
- **THEN** the pair is `inconclusive`, the exact difference is retained, and no output change is attributed to shader-constant-cache invalidation

#### Scenario: A non-QMD semantic differs
- **WHEN** the arms differ in seed sequence, root contents or size/alignment, root or QMD schedule/address/allocation order, shaders, generated headers, images, descriptors, sampler state, pipeline layouts, image layouts, barriers, command-buffer lifecycle, dispatch mechanism, cache behavior other than bit 255, submission, waits, backing, readback, oracle, ordering, or iteration count
- **THEN** the pair is `inconclusive` and is not authorized for causal hardware interpretation

### Requirement: The retained failure-producing schedule and workload remain frozen
Both arms SHALL preserve root A at `0xc7f40000`, root B at `0xc7f40a00`, QMD X at `0xc7f40800`, and QMD Y at `0xc7f40900` when reconstructed allocation evidence matches the retained experiment. They SHALL select A/X on odd iterations and B/Y on even iterations, yielding 63/63 adjacent root-address and QMD-address transitions with the same allocation order, `NvMap`/BO backing, ownership, lifetime, and direct-dispatch ordering as the retained pair. Any unavoidable address discrepancy discovered before implementation SHALL require an OpenSpec revision rather than silent substitution.

#### Scenario: Frozen schedule is reproduced
- **WHEN** either artifact completes its 64-iteration host correlation or real-hardware run
- **THEN** it proves the expected root and QMD VAs, odd/even selections, 63/63 transitions, exact root copies, exact QMD copies, allocation footprint and order, ownership through completion, and complete teardown

#### Scenario: Schedule or backing changes
- **WHEN** either artifact uses a different slot count, selection order, allocation order, BO/NvMap backing, lifetime, address sequence, or later upload placement
- **THEN** the experiment is `inconclusive` and no reuse-distance, allocator, or backing change is substituted

### Requirement: Selectors are strict, exclusive, and side-effect free when absent
The paired artifacts SHALL use exact-value experiment selectors `QMD_SHADER_CONSTANT_CACHE_CONTROL` and `QMD_SHADER_CONSTANT_CACHE_INVALIDATE`, with matching unambiguous runtime path markers. Exactly one selector SHALL equal `1` in an experiment artifact. They SHALL be mutually exclusive and incompatible with every retained root-cache, QMD-upload, QMD-address, root-address, or other diagnostic intervention selector. Absent or zero selectors SHALL retain ordinary execution exactly.

#### Scenario: Control selector is valid
- **WHEN** only `QMD_SHADER_CONSTANT_CACHE_CONTROL=1` is configured
- **THEN** the retained two-root/two-QMD diagnostic schedule runs with bit 255 decoded as zero and reports the control path before Vulkan work begins

#### Scenario: Variant selector is valid
- **WHEN** only `QMD_SHADER_CONSTANT_CACHE_INVALIDATE=1` is configured
- **THEN** the identical diagnostic schedule runs with bit 255 decoded as one and reports the variant path before Vulkan work begins

#### Scenario: Selector configuration is malformed or contradictory
- **WHEN** a selector has a value other than exact zero or one, both new selectors are enabled, or either is combined with an excluded intervention selector
- **THEN** the configuration fails before device work with a distinct nonzero result and cannot produce experiment evidence

#### Scenario: Experiment selectors are absent
- **WHEN** neither new selector is enabled
- **THEN** no experimental QMD field, allocation, copy, diagnostic, or dispatch behavior is introduced into ordinary execution

### Requirement: No additional cache, synchronization, timing, or allocation intervention is permitted
Neither arm SHALL add `INVALIDATE_SHADER_DATA_CACHE`, texture cache invalidation, method-level shader-cache invalidation, a QMD membar change, WFI/serialization, a new barrier, wait, submission, fence behavior, CPU cache maintenance, sleep, payload nonce, root/QMD reuse-distance change, third root, fourth QMD, alternate BO/NvMap backing, or allocator redesign. Existing `INVALIDATE_SKED_CACHES`, `SEND_PCAS_A`, `SEND_SIGNALING_PCAS_B`, submission, queue/device waits, cache maintenance, and synchronization SHALL remain identical.

#### Scenario: Isolation review finds no prohibited intervention
- **WHEN** control and variant source, generated artifacts, and static QMD comparisons are reviewed before hardware
- **THEN** the only causal difference is QMD v0.6 bit 255 and all existing methods and synchronization remain unchanged

#### Scenario: A prohibited intervention is present
- **WHEN** review or runtime correlation detects any additional cache, membar, synchronization, timing, backing, allocation, or payload intervention
- **THEN** hardware execution stops or the completed run is `inconclusive`, and the additional intervention requires a separate change

### Requirement: QMD generation, copy, dispatch, and ordering are exactly correlated
For all 64 iterations in both arms, evidence SHALL correlate the generated QMD, selected mapped QMD, decoded bit-255 value, decoded root GPU VA, selected QMD GPU VA, direct `PCAS` dispatch VA, root/QMD selection schedule, and operation order. Complete byte comparison SHALL be authoritative; hashes MAY be emitted only as compact identities. CPU-mapped equality SHALL NOT be described as proof of GPU visibility or consumption.

#### Scenario: Per-iteration correlation is complete
- **WHEN** a correlated dispatch record is accepted
- **THEN** the selected mapped QMD exactly equals the generated QMD, its decoded root VA matches the paired control/variant iteration, its selected QMD VA equals the direct-dispatch VA, bit 255 matches the selected arm, and copy-before-dispatch ordering is complete

#### Scenario: Paired field proof is complete
- **WHEN** host validation compares the same iteration across arms
- **THEN** it reports field zero/one decode, exact one-bit exclusive-or, identical encoded root VA, identical selected QMD VA, identical schedule and ordering, and no other changed bit

#### Scenario: Any correlation prerequisite fails
- **WHEN** generated-to-mapped equality, field decode, root decode, QMD-to-`PCAS` address equality, schedule, order, or paired full-payload equality is missing or false
- **THEN** the run is `inconclusive` regardless of observed pixels or checksums

### Requirement: Diagnostics are symmetric, bounded, and sufficient for false-positive detection
Both arms SHALL execute the same fixed diagnostic structure without dynamic allocation in the dispatch-critical path. Detailed records SHALL cover at least iterations 1-5, while compact correlation and independent oracle evidence SHALL cover all 64 iterations. The evidence SHALL include selector/path, iteration/seed, root source/mapped equality, root and QMD addresses, field decode, complete-copy and one-bit-difference decisions, dispatch/order correlation, output pixel/checksum/oracle, aggregates, teardown, and interpretable GPU notifier/error state.

#### Scenario: Diagnostics are complete and bounded
- **WHEN** either 64-iteration artifact completes
- **THEN** iterations 1-5 contain decisive detailed records, all 64 iterations have joined correlation and oracle decisions, aggregates match individual records, logging remains bounded, and no high-volume per-frame SD-only path is introduced

#### Scenario: Evidence can produce a false positive
- **WHEN** missing records, mismatched keys, incomplete logs, an unexpected QMD bit, a source/mapped mismatch, a dispatch mismatch, incomplete teardown, or unexplained notifier/error state prevents exact attribution
- **THEN** the result is `inconclusive` even if the final output appears correct

### Requirement: A valid control gates all variant interpretation
The control SHALL execute first on real Tegra hardware and SHALL reproduce the retained two-root/two-QMD signature: iteration 1 seed 5 is exact at pixel `0xfa47d33f` and checksum `0xb7d223e5`; iteration 2 seed 42 is exact at pixel `0xf5031a17` and checksum `0x0daf4ac5`; iterations 3-64 retain the iteration-2 pixel/checksum and fail their current-seed oracle. The complete unfiltered stream SHALL be inspected before filtering and SHALL contain the exact schedule, copies, bit-255 zero decode, QMD-to-`PCAS` correlation, ordering, all 64 results, complete teardown, and no unexplained timeout, notifier, error-info event, or GPU fault.

#### Scenario: Control reproduces every prerequisite
- **WHEN** the control completes with the exact retained output and all causal/evidence checks pass
- **THEN** the variant is authorized on the same intended configuration

#### Scenario: Control deviates
- **WHEN** the control output, schedule, payload, bit decode, address correlation, ordering, record count, teardown, or fault state differs from the retained baseline or is incomplete
- **THEN** the result is `inconclusive`, variant execution or interpretation is forbidden, and the deviation is retained without adding another intervention

### Requirement: Hardware outcomes are classified narrowly
After a valid control and causally valid variant, the experiment SHALL emit exactly one predefined result based on all 64 independent oracle decisions and complete fault evidence. No outcome SHALL establish an undocumented cache key, physical-backing behavior, a generic GM20B cache defect, a general NVK defect, or a universal requirement inferred from old Gallium.

#### Scenario: Variant passes all 64 oracle checks
- **WHEN** the control is valid, the variant differs only by bit 255, all 64 variant iterations pass exactly, all causal prerequisites hold, and no unexplained fault occurs
- **THEN** the result is `qmd_shader_constant_cache_invalidate_supported_experiment_only`, meaning only that this QMD field is sufficient for this controlled launch pattern

#### Scenario: Variant retains the exact control stale signature
- **WHEN** the control is valid, the variant differs only by bit 255, iterations 1-2 are exact, iterations 3-64 retain pixel `0xf5031a17` and checksum `0x0daf4ac5`, and no unexplained fault occurs
- **THEN** the result is `specific_qmd_shader_constant_cache_invalidate_insufficient`, the exact negative evidence is preserved, and root reuse distance is named only as the next separate discriminator

#### Scenario: Variant changes behavior but still fails
- **WHEN** the control is valid, all one-bit causal prerequisites hold, and variant output differs from the control stale signature but any current-seed oracle check fails
- **THEN** the result is `behavior_changed_unresolved`, the exact new signature is retained, and the next smallest discriminator is derived in a separate change without combining other Gallium cache or membar settings

#### Scenario: Evidence or causal validity fails
- **WHEN** any baseline, provenance, isolation, copy, decode, address, dispatch, ordering, logging, teardown, or fault prerequisite is absent or inconsistent
- **THEN** the result is `inconclusive` and no hardware hypothesis or project capability is promoted

### Requirement: Immutable evidence and non-promotion rules are enforced
Interpretive hardware execution SHALL require strict OpenSpec validation, repository policy checks, durable-patch reconstruction from pristine Mesa 25.0.7, build-source authority proof, selector tests, ordinary-path unchanged proof, exact one-bit static comparison, unchanged allocator/shader/resource/synchronization evidence, committed source, and reproducible artifact/patch/shader hashes. Complete raw logs and paired records SHALL be retained according to `docs/testing/HARDWARE_EVIDENCE.md`. Host, build, static, or emulator evidence alone SHALL be `IMPLEMENTED_UNPROVEN` at most.

#### Scenario: Hardware is unavailable
- **WHEN** implementation and all host gates pass but no authorized real-Tegra run occurs
- **THEN** the result stops at `IMPLEMENTED_UNPROVEN` and `hardware-ready` without implying device contact

#### Scenario: Experimental support is observed
- **WHEN** the hardware result is `qmd_shader_constant_cache_invalidate_supported_experiment_only`
- **THEN** FG-2 remains `BLOCKED`, the selector remains opt-in or is rolled back, and a separate production-remediation decision plus separate ordinary image-chain 64/64 hardware acceptance change are required before promotion

#### Scenario: Any hardware outcome is retained
- **WHEN** the control-only or paired experiment concludes
- **THEN** complete raw logs, a hardware record, a narrow research finding, and justified milestone/capability wording preserve prior evidence; historical runs are not rewritten, FG-2 remains `BLOCKED`, and FG-3/FG-4 work does not begin
