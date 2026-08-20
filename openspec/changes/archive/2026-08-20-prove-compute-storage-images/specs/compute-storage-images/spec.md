## Purpose

This capability defines the deterministic GPU compute and image-processing proof required by FG-1. It establishes whether the Horizon/NVK stack can execute the exact storage-buffer and sampled-image-to-storage-image primitives needed by later frame-generation work.

## ADDED Requirements

### Requirement: Compute pipeline execution is exercised on the target winsys

The test artifact SHALL create and dispatch a Vulkan compute pipeline whose shader execution is performed by the target Horizon/NVK GPU path.

#### Scenario: Compute dispatch produces an observable result
- **WHEN** the artifact initializes a valid compute pipeline and submits a dispatch with known inputs
- **THEN** the dispatch completes and produces an output that can be validated by the test

#### Scenario: Pipeline or dispatch failure is reported as failure
- **WHEN** compute pipeline creation, command recording, submission, or completion fails
- **THEN** the artifact reports the failing stage and does not classify the compute capability as proven

### Requirement: Storage buffers support deterministic GPU read/write

The compute path SHALL read known values from a storage buffer, write the specified transformation to a storage buffer, and expose the result for validation without relying on an unreported CPU substitute.

#### Scenario: Known storage-buffer transformation matches the expected result
- **WHEN** the shader receives the defined input pattern
- **THEN** the GPU-written storage-buffer output exactly matches the independently calculated expected pattern

#### Scenario: Invalid or incomplete buffer setup is rejected
- **WHEN** the required storage-buffer binding, range, or memory visibility is invalid
- **THEN** the artifact reports the setup failure and does not accept a partial or fallback result

### Requirement: Sampled-image to storage-image processing is supported

The compute path SHALL sample a known source image and write a defined result to a storage image using GPU image operations.

#### Scenario: Image sample and storage write produce the expected result
- **WHEN** the shader samples the defined source image and writes the defined storage-image transformation
- **THEN** CPU-visible validation of the destination matches the expected pixel values or checksum

#### Scenario: Unsupported image usage is distinguishable from success
- **WHEN** the source or destination image cannot be used with the requested sampled or storage access
- **THEN** the artifact reports the unsupported usage and does not substitute a CPU copy or alternate image path silently

### Requirement: Image layouts and dependencies are explicit and correct

The test SHALL establish and validate the image layouts and execution dependencies required between image initialization, compute sampling, storage writes, and readback.

#### Scenario: Required image transitions preserve data visibility
- **WHEN** the artifact executes the complete source-image to destination-image sequence
- **THEN** the destination reflects the source data according to the defined operation and no synchronization error or stale-data result occurs

#### Scenario: Missing or incorrect dependency prevents acceptance
- **WHEN** an image transition or execution dependency is absent or incorrect
- **THEN** the artifact fails deterministic validation or reports the synchronization failure rather than accepting nondeterministic output

### Requirement: Output is deterministic and CPU-visible

The artifact SHALL make the GPU result visible to the CPU through the supported readback path and SHALL validate it against a fixed expected result over the specified repetition count.

#### Scenario: Repeated runs produce the same validated output
- **WHEN** the test runs the defined input and operation repeatedly
- **THEN** every iteration produces the expected checksum or exact values and the complete run is reported as deterministic

#### Scenario: Readback validation cannot be completed
- **WHEN** the GPU result cannot be made CPU-visible or the observed output differs from the expected result
- **THEN** the run is reported as failed or unproven with the mismatch details retained

### Requirement: Intended-path execution and GPU fault state are observable

The artifact SHALL report whether the intended GPU compute, storage-buffer, sampled-image, storage-image, and readback path executed, and SHALL inspect and retain relevant GPU fault/error-notifier state for the run.

#### Scenario: Clean intended-path run is reported
- **WHEN** all required operations complete and no GPU fault/error-notifier condition is observed
- **THEN** the report identifies the intended path, records the deterministic validation, and records the clean diagnostic state

#### Scenario: Fault or fallback is detected
- **WHEN** a GPU fault/error-notifier condition occurs or a fallback path is used
- **THEN** the report identifies the condition and marks the result invalid for `PROVEN_HW`

### Requirement: Hardware evidence is sufficient for capability promotion

The test result SHALL retain the configuration, artifact identity, repetition count, expected and observed results, diagnostic output, and complete raw device evidence required by `docs/testing/HARDWARE_EVIDENCE.md` before the capability can be promoted to `PROVEN_HW`.

#### Scenario: Hardware acceptance record is complete
- **WHEN** the artifact passes deterministic validation on real Tegra hardware
- **THEN** the evidence record contains the exact commit, test artifact, hardware context, run configuration, raw log location, and conclusion

#### Scenario: Host-only or emulator-only evidence is insufficient
- **WHEN** the test passes only in a host build, emulator, or adjacent graphics path
- **THEN** the result remains `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN` and is not reported as hardware proof
