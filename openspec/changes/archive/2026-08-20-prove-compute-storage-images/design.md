## Context

The proposal establishes the need for the FG-1 proof. The current matrix records compute, storage-buffer, and storage-image behavior as `IMPLEMENTED_UNPROVEN`; inherited graphics and WSI proofs do not establish compute-engine behavior on real Tegra hardware. The design must therefore make the smallest path observable end to end and must keep hardware evidence separate from host or emulator evidence.

## Goals / Non-Goals

**Goals:**

- Add one deterministic smoke artifact that exercises compute pipeline creation, NAK compute execution, storage-buffer read/write, sampled-image input, storage-image output, explicit dependencies, and CPU-visible validation.
- Keep the test self-contained and reproducible with fixed inputs, expected outputs, and a bounded repetition count suitable for exposing synchronization or lifetime faults.
- Make intended-path execution, fallback use, and GPU fault/error-notifier state explicit in the report.
- Preserve the existing build and correctness paths while making the new artifact independently runnable.

**Non-Goals:**

- Asynchronous submission, native fence export/import, VI scheduling, external `NvMap` or block-linear image import, NVN interoperability, or native-title integration.
- Frame-generation quality, performance optimization, or a production interpolation algorithm.
- Treating an emulator, host build, visual result, or upstream NVK support as hardware proof.

## Decisions

### Use one deterministic vertical path

The artifact will use fixed source data and a simple operation whose buffer and image outputs can be calculated independently. This keeps failures attributable to compute execution, resource binding, image usage, dependency handling, or readback rather than to an algorithm or presentation system. A broader multi-test framework is deferred until a concrete consumer requires it.

### Validate both buffer and image results

The test will include a storage-buffer transformation and a sampled-image-to-storage-image transformation in the same coherent proof. Buffer validation catches basic shader and memory-write failures; image validation exercises format, layout, sampling, storage access, and visibility requirements that buffer-only compute would miss.

### Use explicit synchronization and report its phases

The design will separate source initialization, compute access, destination write, and CPU readback with explicit Vulkan dependency phases. The report will identify the phases exercised and fail validation on stale or nondeterministic data. This makes a visually plausible result insufficient evidence.

### Keep ownership local to the test

All buffers and images used by the proof are owned and destroyed by the test artifact. The test will not introduce borrowed-resource or cross-context lifetime semantics; those belong to later FG-5/FG-6/NI-2 changes. CPU readback is an observation/validation step, not an image-processing fallback.

### Prefer a correctness-first diagnostic mode

The initial artifact will retain enough low-volume diagnostics to identify the selected path and inspect GPU fault/error-notifier state. Repetition and logging must be bounded so diagnostic output does not become the measured feature or overwhelm the device log. A later performance mode can reduce logging after correctness is proven.

### Alternatives considered

- **Reuse an existing graphics smoke test:** rejected because graphics success does not prove compute-engine dispatch, storage-image access, or compute-specific NAK execution.
- **Use only a host or emulator test:** rejected because those environments cannot prove GM20B command execution, layout/kind behavior, or Horizon synchronization semantics.
- **Validate only a screenshot:** rejected because visual output does not distinguish CPU fallback, stale data, or a partial path from the intended GPU operation.
- **Build a reusable compute test framework first:** deferred because FG-1 needs one falsifiable consumer, and premature abstraction would obscure the actual primitive under test.

## Risks / Trade-offs

- [Hardware behavior differs from host/emulator behavior] → Keep the status `IMPLEMENTED_UNPROVEN` until the complete real-hardware evidence record passes; retain full device logs and error state.
- [A CPU/staging fallback masks missing GPU image support] → Make path selection explicit and fail acceptance if any required operation is bypassed.
- [Nondeterministic or stale results pass intermittently] → Use fixed expected values, repeated iterations, explicit dependencies, and exact checksum/value validation.
- [Diagnostic logging changes timing or causes noise] → Keep per-iteration output low-volume and separate correctness diagnostics from later performance measurements.
- [Readback visibility is confused with general external-memory support] → Scope ownership and visibility to test-local resources and record no claim for external interoperability.

## Migration Plan

1. Implement the smoke artifact and minimal build/package integration described by the tasks.
2. Run host/build checks and record any host-only result without promoting the capability.
3. Run the artifact on real Tegra hardware, inspect the complete device log and GPU fault/error-notifier state, and retain the hardware evidence record.
4. If all acceptance criteria pass, update the FG-1 capability rows and milestone status, then archive this change and sync its delta spec to `openspec/specs/`.
5. If the hypothesis fails, retain the change and record the result as `BLOCKED` or `REJECTED` with a focused follow-up experiment; do not widen this change to later milestones.

## Open Questions

- The exact existing smoke-test registration/build target and the final repetition count should be confirmed from the authoritative build files during implementation; neither changes the behavioral contract.
