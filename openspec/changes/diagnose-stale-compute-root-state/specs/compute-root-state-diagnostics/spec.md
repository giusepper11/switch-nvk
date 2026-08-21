## Purpose

This capability makes the FG-2 compute push/root-state path observable on real Tegra hardware so evidence can identify the first boundary at which a changing iteration seed becomes stale without treating instrumentation as a fix.

## ADDED Requirements

### Requirement: Diagnostic capture is opt-in, bounded, and observation-only
The diagnostic path SHALL be disabled for ordinary builds or runs, SHALL limit detailed state capture to iteration 1 and iteration 2 with iteration 3 permitted as a bounded confirmation, and SHALL preserve the baseline shaders, resources, command sequence, synchronization, dispatch, and validation semantics.

#### Scenario: Baseline diagnostic run is enabled
- **WHEN** the dedicated diagnostic artifact runs with state capture enabled
- **THEN** it emits detailed records only for the configured first two or three iterations and executes the original 64-iteration FG-2 validation contract without adding a cache operation, wait, GPU command, substitute data path, or shader behavior change

#### Scenario: Diagnostic capture is disabled
- **WHEN** the diagnostic control is absent or disabled
- **THEN** no root-state diagnostic records are emitted and the normal driver path remains selected

#### Scenario: Instrumentation changes the failure signature
- **WHEN** the diagnostic artifact no longer reproduces the prior deterministic seed-5 stale-compute signature or otherwise changes allocation, command, synchronization, or shader behavior relevant to the result
- **THEN** the run is recorded as an observer effect and is not accepted as evidence that FG-2 or the stale-state defect is fixed

### Requirement: Command-buffer push state is observable by consumer stage
The diagnostic record SHALL expose the pushed seed bytes and the resulting graphics and compute root bytes immediately after the push-state update, together with the command-buffer and descriptor/root identities needed to distinguish the two consumers.

#### Scenario: Iteration 1 push state is captured
- **WHEN** iteration 1 records expected seed 5 for fragment and compute consumers
- **THEN** the record reports the pushed bytes, graphics-root seed bytes, compute-root seed bytes, stage mask, byte range, and root identities associated with that iteration

#### Scenario: Iteration 2 push state is captured
- **WHEN** iteration 2 records expected seed 42 for fragment and compute consumers
- **THEN** the record unambiguously reports whether the compute CPU root contains 42 or is already stale at 5 while separately reporting the graphics CPU root

### Requirement: Compute root upload is correlated with the dispatch
For each captured compute dispatch, the diagnostic path SHALL report the source root bytes, uploaded root bytes available through the CPU mapping, allocation/mapping identity, GPU virtual address, byte size, and correlation identity for the current iteration.

#### Scenario: Current seed reaches the upload copy
- **WHEN** iteration 2 enters compute-root upload with seed 42 in the compute root
- **THEN** the record reports whether seed 42 is present both in the copy source and in the CPU mapping backing the returned upload GPU address

#### Scenario: Upload allocation is reused or replaced
- **WHEN** root upload storage has the same GPU address as an earlier captured iteration or receives a new address
- **THEN** the record identifies the reuse or change and preserves the iteration-to-address association

#### Scenario: Upload cannot be inspected safely
- **WHEN** the memory addressed for compute root consumption cannot be read safely through the available CPU mapping
- **THEN** the record marks that boundary unobserved, distinguishes CPU and GPU mappings explicitly, and does not infer GPU-visible bytes from an unrelated copy

### Requirement: QMD and dispatch root addressing is observable
The diagnostic path SHALL identify the QMD constant-buffer slot used for the root descriptor, decode the root GPU address from the relevant generated QMD address fields, report the QMD upload address and dispatched QMD address, and compare the encoded root address with the current root upload.

#### Scenario: QMD points to the current root upload
- **WHEN** a captured QMD is constructed for a compute dispatch
- **THEN** the record identifies the root constant-buffer slot, relevant encoded field values, decoded root address, uploaded root address, address-match result, QMD address, and dispatch address

#### Scenario: QMD points to stale storage
- **WHEN** the decoded QMD root address differs from the root upload correlated with the same iteration
- **THEN** the diagnostic result identifies QMD construction or dispatch-address propagation as the first demonstrated stale boundary without requiring an entire QMD dump

### Requirement: Evidence identifies the first stale boundary
The diagnostic output SHALL correlate iteration, expected seed, graphics root seed, compute root seed, upload source seed, uploaded mapping seed, upload GPU address, QMD root address, dispatched QMD address, and observed output-seed behavior in one reviewable record or table.

#### Scenario: State first becomes stale at an observed boundary
- **WHEN** two adjacent captured boundaries contain different seed state or address identity for the same iteration
- **THEN** the finding names the last boundary with the expected value and the first boundary with the stale value and stops before proposing a broader-layer explanation

#### Scenario: CPU mapping and QMD are current but shader behavior is stale
- **WHEN** iteration 2 has seed 42 in the compute root and uploaded CPU mapping, the QMD encodes the matching current upload address, the dispatch uses that QMD, and output still behaves as seed 5
- **THEN** the finding narrows the unresolved boundary to GPU visibility or constant-buffer consumption and does not claim which of those mechanisms is responsible without an additional discriminating experiment

#### Scenario: Evidence remains ambiguous
- **WHEN** one or more adjacent boundaries cannot be observed or correlated reliably
- **THEN** the finding identifies the exact unresolved interval, keeps FG-2 `BLOCKED`, and defines only the next smallest controlled experiment needed to observe that interval

### Requirement: Material conclusions require immutable real-hardware evidence
Any conclusion about the stale compute-state boundary SHALL be based on a committed diagnostic source revision, a hashed artifact, a complete real-Tegra run, retained unfiltered logs, deterministic result correlation, and recorded GPU fault/error state.

#### Scenario: Hardware record supports a boundary finding
- **WHEN** the diagnostic artifact runs on real Tegra hardware and the complete evidence is reviewed
- **THEN** the record includes repository commit, artifact SHA256 and version, hardware/model, diagnostic flags, inspected iterations, expected and observed boundary values and addresses, output behavior, observer-effect assessment, GPU fault/error state, raw-log references, and the strongest justified conclusion

#### Scenario: Only host or emulator evidence exists
- **WHEN** the diagnostic path only builds, passes static checks, or runs in an emulator
- **THEN** its implementation may be recorded as `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`, but no hardware boundary is claimed

#### Scenario: Diagnostic finding is recorded
- **WHEN** hardware evidence identifies or narrows the stale boundary
- **THEN** the detailed finding is retained under `docs/research/`, FG-2 remains `BLOCKED` unless the original render-compute-image-chain acceptance contract later passes, the shared-layout hypothesis remains `REJECTED`, and project status changes only for a durable evidence-backed fact
