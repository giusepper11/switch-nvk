# render-compute-image-chain Specification

## Purpose

This capability defines the deterministic real-Tegra proof that graphics output stored in an offscreen image can be sampled by compute, transformed into a second storage image, and read back with correct explicit dependencies and no hidden CPU substitute.

## Requirements

### Requirement: A graphics pass produces deterministic offscreen image content
The artifact SHALL execute a graphics render pass into test-owned offscreen image A and SHALL define the expected value of every rendered pixel independently of the observed GPU output.

#### Scenario: Offscreen rendering produces the defined pattern
- **WHEN** the graphics pass renders the fixed test geometry and deterministic fragment pattern into image A
- **THEN** every pixel required by the contract has the independently calculable value for that run

#### Scenario: Graphics setup or execution fails
- **WHEN** render-target creation, graphics-pipeline creation, command recording, submission, or execution cannot complete
- **THEN** the artifact identifies the failing phase and does not classify the chain as proven

### Requirement: Compute samples the rendered image
The artifact SHALL bind image A as a sampled image in a later compute pass and SHALL make its graphics writes available and visible to compute shader reads.

#### Scenario: Rendered pixels are consumed by compute
- **WHEN** the graphics pass completes and the compute pass executes
- **THEN** the compute result depends on the defined pixel values produced in image A

#### Scenario: A substitute input path is detected
- **WHEN** compute uses CPU-generated data, a separately initialized image, constants that do not depend on image A, or another bypass instead of the rendered image
- **THEN** the artifact reports the bypass and the run is invalid for acceptance

### Requirement: Compute writes a second storage image
The compute pass SHALL apply a specified deterministic transformation to the sampled values from image A and SHALL write the results to distinct test-owned storage image B.

#### Scenario: Storage-image output matches the defined transformation
- **WHEN** compute samples each contracted pixel from image A
- **THEN** it writes the independently calculable transformed value to the corresponding pixel of image B

#### Scenario: Image A is reused as output
- **WHEN** the implementation does not use a distinct image B for compute storage writes
- **THEN** the artifact rejects the run because it does not prove the required two-image chain

### Requirement: Image layouts and dependencies are explicit and correct
The artifact SHALL specify the layouts, execution stages, and access dependencies required for color-attachment writes to image A, compute sampled reads from image A, compute storage writes to image B, and transfer reads from image B.

#### Scenario: Complete dependency chain preserves visibility
- **WHEN** graphics, compute, and transfer operations execute in order
- **THEN** image A is transitioned from color-attachment output to compute sampling, image B is transitioned for compute storage output and then transfer readback, and the observed output is deterministic

#### Scenario: Dependency or layout is missing or invalid
- **WHEN** a required layout transition, stage dependency, or access dependency is absent or incorrect
- **THEN** the artifact fails setup or deterministic validation and does not accept a stale or coincidentally matching result

### Requirement: Readback validates every output pixel deterministically
The artifact SHALL copy image B through a supported transfer/readback path, compare every output pixel against an independently calculated expected value, and report expected and observed checksums over the defined repetition count.

#### Scenario: Every repeated iteration is exact
- **WHEN** the complete chain runs for the required number of iterations
- **THEN** every pixel in every iteration matches exactly and the expected and observed checksums match

#### Scenario: Hash collision or partial match cannot produce acceptance
- **WHEN** a checksum matches but any individual pixel differs, or any output region is not validated
- **THEN** the artifact reports failure and does not accept the checksum alone as proof

### Requirement: False-positive and fallback paths are observable
The artifact SHALL initialize resources and report path selection so that stale data, prefilled expected output, CPU image processing, a direct transfer-only path, skipped graphics, skipped compute, or a previous iteration's output cannot appear to prove the intended chain.

#### Scenario: Intended chain executes without substitute processing
- **WHEN** graphics renders image A, compute samples A and writes B, and transfer reads B
- **THEN** the report identifies each stage, confirms no CPU processing or alternate GPU bypass occurred, and validation rejects untouched sentinel data

#### Scenario: Required stage is bypassed
- **WHEN** graphics, compute sampling, storage output, or transfer readback is skipped or replaced
- **THEN** the artifact identifies the missing/substitute stage and marks the run invalid

### Requirement: Real-hardware evidence is required for acceptance
The capability SHALL remain `IMPLEMENTED_UNPROVEN` until the complete deterministic chain passes on real Switch/Tegra hardware with immutable source/artifact provenance and the evidence required by `docs/testing/HARDWARE_EVIDENCE.md`.

#### Scenario: Hardware acceptance record is complete
- **WHEN** the artifact passes the defined repeated validation on real Tegra hardware
- **THEN** the record identifies the exact commit and artifact hash, hardware context, formats and dimensions, iterations, expected and observed results, intended path, complete raw logs, and GPU fault/error state

#### Scenario: Fault or unexplained warning occurs
- **WHEN** a GPU timeout, `ERRNOTIF`, `ERRINFO`, unexplained driver warning, fallback, or output mismatch occurs
- **THEN** the run is invalid for `PROVEN_HW` and the complete failure evidence is retained

#### Scenario: Only host or emulator evidence exists
- **WHEN** the artifact only builds, passes static checks, or runs in an emulator
- **THEN** the result is recorded as host evidence or `IMPLEMENTED_UNPROVEN`, not `PROVEN_HW`
