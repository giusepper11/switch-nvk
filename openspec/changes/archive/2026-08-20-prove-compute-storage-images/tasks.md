## 1. Establish the smoke-test integration point

- [x] 1.1 Inspect the authoritative smoke-test sources, build/package definitions, and existing resource/readback helpers; select the smallest supported integration point for the FG-1 artifact.
- [x] 1.2 Define the fixed input pattern, buffer transformation, source-image format/content, destination-image operation, expected values/checksum, and repetition count in the test's durable documentation or constants.

## 2. Implement the deterministic compute/image path

- [x] 2.1 Add the compute smoke artifact and its minimal build/package registration without changing unrelated runtime or GPU behavior.
- [x] 2.2 Create the compute pipeline and dispatch path, including the NAK compute shader and explicit bindings for the storage buffers and sampled/storage images.
- [x] 2.3 Implement test-local resource ownership and initialization for the buffers and images, including the supported CPU-visible readback resources.
- [x] 2.4 Record and submit the explicit image layout transitions and execution dependencies for source initialization, compute sampling/storage access, and readback.

## 3. Add validation and observability

- [x] 3.1 Validate storage-buffer output and sampled-image-to-storage-image output against independently calculated expected values on every iteration.
- [x] 3.2 Run the deterministic path for the selected repetition count and report mismatches, incomplete readback, setup failures, and synchronization failures with enough context to diagnose them.
- [x] 3.3 Report unambiguously that the intended GPU compute/image/readback path executed, identify any fallback or bypass, and capture the supported GPU fault/error-notifier state.

## 4. Verify on host and real hardware

- [x] 4.1 Run the relevant host/build/package checks and retain the result without promoting the capability beyond `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`.
- [x] 4.2 Run the artifact on real Tegra hardware with the required configuration, inspect the complete device log before filtered excerpts, and retain GPU fault/error-notifier output.
- [x] 4.3 Assemble the `docs/testing/HARDWARE_EVIDENCE.md` record with commit, artifact, hardware context, configuration, repetitions, expected/observed result, checksum, raw evidence location, and conclusion.

## 5. Update project state after evidence review

- [x] 5.1 If all FG-1 acceptance criteria pass, update the relevant capability-matrix rows and milestone status to the evidence-supported state; otherwise record the result as `IMPLEMENTED_UNPROVEN`, `BLOCKED`, or `REJECTED` with the next discriminating experiment.
- [x] 5.2 Review the result for surprising or negative findings; no additional research note or ADR is required for this expected passing result.
