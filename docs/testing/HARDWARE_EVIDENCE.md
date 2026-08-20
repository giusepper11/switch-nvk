# Hardware Evidence Standard

Real Switch/Tegra behavior is the final authority for hardware-sensitive claims in this project.

This document defines the minimum evidence required to mark a capability `PROVEN_HW`.

## 1. Evidence hierarchy

From strongest to weakest for hardware-dependent behavior:

1. repeatable real-hardware execution with deterministic validation and retained logs;
2. repeatable real-hardware visual result plus instrumented state/error checks;
3. real-hardware API success without semantic validation;
4. emulator result;
5. successful cross-build/link;
6. source inspection / upstream capability reasoning.

Only levels 1-2 normally justify `PROVEN_HW`.

An API returning success is not sufficient if the test does not verify the requested operation actually occurred.

## 2. Required evidence record

For a hardware-gated spec, capture:

```text
Date:
Repository commit:
Spec/milestone:
Hardware/model:
Firmware/Atmosphere/libnx context when relevant:
Build type:
Diagnostics enabled:
Resolution/format:
Swapchain/buffer count/present mode when relevant:
Clock/OC state when relevant:
Test artifact/version:
Run duration/iteration count:
Expected result:
Observed result:
Deterministic validation/checksum:
GPU error notifier/error info:
Relevant timing summary:
Raw evidence location/reference:
Conclusion/status:
```

Use `N/A` rather than silently omitting a relevant field.

## 3. Deterministic validation

Prefer checks that validate semantics, for example:

- known buffer pattern readback;
- exact sampled pixel values;
- hash/checksum of output image;
- monotonic sequence/counter validation;
- expected fence ordering;
- known overlap/blend equation result;
- repeated no-fault iteration count.

A screenshot is useful supporting evidence but is weaker than a deterministic validation when one is possible.

## 4. Full-log rule

For a new low-level behavior:

1. retain/read the complete device log from the run before relying on grep-filtered excerpts;
2. inspect for errors that occurred before/after the apparent success line;
3. retain relevant error notifier/error-info output after GPU timeout/fault;
4. document unexpected warnings even if the visual result looks correct.

Do not conclude success from the final line alone.

## 5. Emulator evidence

Emulators are valuable for:

- host iteration;
- API wiring;
- obvious control-flow bugs;
- build/package testing;
- application-level behavior not dependent on exact Tegra semantics.

They are not authoritative for:

- GPFIFO behavior;
- privileged/engine method behavior;
- `NvFence`/syncpoint details;
- block-linear layout/kind;
- cache coherency;
- `NvMap`/GPU VA behavior;
- VI/nwindow pacing;
- performance.

Record emulator-only evidence as `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`, not `PROVEN_HW`.

## 6. Performance evidence

Performance runs must distinguish diagnostic overhead from the feature under test.

Record:

- build type;
- logging/tracing flags;
- resolution and image format;
- buffer count;
- clocks if modified;
- number of measured frames/samples;
- metric definition and measurement boundaries;
- average plus distribution/percentiles where meaningful.

For frame-generation scheduling, preferred phase metrics include:

```text
CPU submit duration
GPU interpolation duration
acquire wait duration
producer dependency wait
present queue duration
frame interval
real/generated cadence
```

Do not use FPS as the only metric for a latency/synchronization change.

## 7. Fallback-path detection

If an implementation has a correctness fallback, evidence must prove which path ran.

Examples:

- zero-copy vs CPU-copy present;
- native-fence path vs host wait;
- external import vs copied staging image.

The path indicator should be low-overhead and unambiguous.

## 8. Repetition

One successful frame is enough only for the earliest bring-up proof.

Before marking a reusable primitive `PROVEN_HW`, run enough repetitions to expose lifetime/synchronization faults. The spec should define the count/duration. For resource ownership and synchronization changes, default toward hundreds or thousands of iterations rather than one visual frame.

## 9. Negative tests

Memory and synchronization features should include failure cases where practical:

- invalid handle/id;
- invalid offset/size;
- unsupported layout;
- stale/released external resource;
- timeout/unmet fence;
- recreate/destroy while resources exist.

A primitive that only works on the happy path is not yet a safe interoperability layer.

## 10. Capability promotion

A capability can be promoted to `PROVEN_HW` only when:

- the active spec's hardware acceptance criteria pass;
- evidence identifies the exact commit/test artifact;
- the test distinguishes the intended path from fallback/false-positive paths;
- no unexplained GPU fault invalidates the run;
- `docs/status/CAPABILITY_MATRIX.md` is updated.

If evidence is incomplete, use `IMPLEMENTED_UNPROVEN`.
