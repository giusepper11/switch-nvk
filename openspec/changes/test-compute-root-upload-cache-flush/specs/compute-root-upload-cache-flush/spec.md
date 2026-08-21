## Purpose

This capability defines a controlled real-Tegra experiment that isolates CPU-cache visibility for a reused compute-root upload without treating a diagnostic variant as a production fix or as proof of the full FG-2 image chain.

## ADDED Requirements

### Requirement: The cache-flush variant is explicit and disabled by default
The experiment SHALL provide an unambiguous selector for the root-upload cache-flush variant, SHALL report whether the selector is enabled, and SHALL leave ordinary builds and runs on the existing non-flushing path when the selector is absent or disabled.

#### Scenario: Controlled variant is enabled
- **WHEN** the dedicated cache-flush artifact starts with the valid experiment selector
- **THEN** it reports the variant as enabled before recording work and makes the controlled root-upload cache operation eligible for the run

#### Scenario: Selector is disabled or invalid
- **WHEN** the experiment selector is absent, disabled, or malformed
- **THEN** no root-upload cache operation is performed and the ordinary compute-root upload path remains selected

### Requirement: Only the reused compute-root upload range is flushed
The enabled experiment SHALL perform one CPU data-cache flush over exactly the current compute-root upload mapping and size only when its GPU virtual address is reused from the preceding correlated compute-root upload. The flush SHALL occur after the current root bytes are copied to the CPU mapping and before the existing QMD upload and direct dispatch.

#### Scenario: Iteration 2 reuses the root upload address
- **WHEN** iteration 2 copies seed 42 into a compute-root upload whose GPU virtual address matches iteration 1
- **THEN** the experiment flushes the CPU mapping range backing that reused root upload before QMD upload/dispatch and reports the mapping, GPU address, byte range, reuse result, and flush invocation

#### Scenario: First upload has no prior reused address
- **WHEN** iteration 1 creates the first correlated compute-root upload
- **THEN** the experiment records that no preceding address exists and does not flush that upload

#### Scenario: A later upload is not reused
- **WHEN** a correlated upload receives a GPU virtual address different from the preceding upload
- **THEN** the experiment reports the changed address, does not flush the fresh allocation, and marks that iteration ineligible to test the reuse-specific hypothesis

### Requirement: The baseline and variant differ only by the targeted cache operation
The paired control and enabled artifacts SHALL preserve identical shader bytes, images, descriptors, pipeline layout, seed sequence, barriers, command-buffer reset/rerecord behavior, GPU command order, QMD contents and upload behavior, direct dispatch, submission/wait behavior, readback, CPU oracle, and 64-iteration validation, except for experiment selection, path reporting, and the targeted CPU cache operation.

#### Scenario: Paired artifacts are reviewed before hardware execution
- **WHEN** the disabled control and enabled variant are built from the same committed source revision
- **THEN** source and artifact review confirms the frozen baseline fields, proves that no cache operation targets the QMD or another allocation, and records both artifact identities and shader hashes

#### Scenario: An unintended semantic difference is found
- **WHEN** the paired artifacts differ in a GPU command, shader, resource, synchronization operation, oracle, dispatch, QMD behavior, or cache operation outside the reused root range
- **THEN** the experiment is invalidated and no cache-visibility conclusion is drawn

### Requirement: Execution and false-positive state are observable
The experiment SHALL correlate each bounded diagnostic record with iteration, expected seed, root source and mapped seed, current and previous upload GPU addresses, reuse result, flushed CPU address and size or explicit skip reason, QMD root and dispatch addresses, observed pixel/checksum, path selection, and GPU fault/error state.

#### Scenario: The targeted path executes
- **WHEN** an enabled iteration reuses the root upload address and reaches dispatch
- **THEN** the retained record proves that the current bytes were copied, the targeted cache operation was invoked for that mapping and size, the existing QMD/dispatch remained correlated, and the exact output was validated

#### Scenario: The targeted path cannot be confirmed
- **WHEN** selector state, marker order, address reuse, flush invocation, QMD/dispatch correlation, or result correlation is missing or inconsistent
- **THEN** the run is classified as inconclusive rather than pass, reject, or fix evidence

#### Scenario: Instrumentation or the disabled control changes the baseline
- **WHEN** the paired disabled control does not reproduce the retained deterministic stale signature or otherwise changes relevant allocation, command, synchronization, or shader behavior
- **THEN** the comparison is classified as an observer effect or invalid control and no causal conclusion is drawn from the enabled variant

### Requirement: Hardware evidence determines only the tested hypothesis outcome
The experiment SHALL use committed source, hashed paired artifacts, complete unfiltered logs, exact 64-iteration validation, real Tegra hardware, and recorded GPU fault/error state. Host builds and emulator runs SHALL NOT establish cache behavior.

#### Scenario: Enabled variant passes all iterations after a valid control
- **WHEN** the disabled control reproduces the stale baseline and the enabled variant produces the independently calculated output for all 64 iterations with the targeted path confirmed and no unexplained fault or warning
- **THEN** the finding records that flushing the reused compute-root upload supports the tested CPU-to-GPU visibility hypothesis, keeps the change classified as an experiment rather than a generalized runtime fix, and leaves FG-2 `BLOCKED` pending an independent unmodified-chain proof

#### Scenario: Enabled variant retains the exact stale signature
- **WHEN** the disabled control is valid and the enabled variant invokes the targeted flush yet reproduces the exact baseline pixel/checksum signature without a GPU fault
- **THEN** the finding rejects this specific root-upload CPU-cache flush as sufficient and keeps the unresolved boundary before or at GPU constant-buffer consumption

#### Scenario: Enabled variant changes output without passing
- **WHEN** the valid enabled run differs from the baseline but does not match the complete 64-iteration oracle
- **THEN** the finding records the exact new signature as evidence that the cache operation affected behavior but makes no supported-versus-rejected or fix claim until a smaller discriminating follow-up is planned

#### Scenario: Only host or emulator evidence exists
- **WHEN** the paired artifacts only compile, pass static checks, or run in an emulator
- **THEN** the implementation remains `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN` and the cache-visibility hypothesis remains unresolved
