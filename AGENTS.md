# AGENTS.md

This file defines how coding/research agents should work in this fork.

## Read order

Before changing code, read only what is needed, in this order:

1. `PROJECT.md`
2. `MILESTONES.md`
3. `docs/status/CAPABILITY_MATRIX.md`
4. the active spec in `docs/specs/`
5. `docs/architecture/ARCHITECTURE.md` if the task crosses subsystem boundaries
6. `docs/testing/HARDWARE_EVIDENCE.md` for any hardware-gated change
7. relevant historical upstream docs only when needed:
   - `RESUME_NVK.md`
   - `BUILD_AND_RUN.md`
   - `REPRODUCE.md`
   - `PLAN_*.md`
   - `docs/knowledge/`

Do not load the entire historical corpus by default.

## Core operating rule

**Never infer success at one technical layer from success at another.**

Examples:

- NVK graphics working does not prove compute works on this winsys.
- external `NvMap` import working does not prove an NVN image can be interpreted correctly.
- NVN image access working does not prove cross-context synchronization.
- generated-image rendering does not prove generated-frame insertion.
- emulator success does not prove real Tegra behavior.

## Evidence language

Use only these status terms in project state documents:

- `PROVEN_HW`
- `PROVEN_HOST`
- `IMPLEMENTED_UNPROVEN`
- `HYPOTHESIS`
- `BLOCKED`
- `REJECTED`
- `NOT_STARTED`
- `IN_PROGRESS`

A code review, build, emulator run, static analysis, or plausible upstream capability is not `PROVEN_HW`.

## Change workflow

For non-trivial work:

1. identify the milestone;
2. read or create the active spec;
3. state the hypothesis and acceptance test;
4. implement the smallest vertical slice;
5. run host/build checks;
6. when hardware matters, produce a hardware artifact/test and stop at `IMPLEMENTED_UNPROVEN` until a hardware result exists;
7. analyze the full hardware log, not only matching lines;
8. update capability status and research findings;
9. add an ADR only for decisions that are intended to persist across milestones.

Do not create broad abstractions before the first concrete consumer requires them.

## Specs

Substantial implementation must be driven by a file under `docs/specs/`.

A spec must define:

- context/problem;
- current evidence;
- hypothesis;
- scope and non-goals;
- interfaces/data ownership if relevant;
- implementation slice;
- validation plan;
- real-hardware acceptance criteria;
- observability/logging requirements;
- risks/failure modes;
- rollback/fallback;
- unresolved questions.

Use `docs/specs/000-template.md`.

Tiny documentation fixes, typo fixes, mechanical refactors, and obvious test maintenance do not require a new spec.

## Hardware-first discipline

Switch GPU behavior is frequently different from what host code inspection or emulators suggest.

When a task depends on:

- GM20B command execution;
- memory tiling/kind;
- cache coherency;
- `NvFence`/syncpoints;
- `NvMap` lifetime or GPU VA behavior;
- `nwindow`/VI buffer queues;
- frame pacing;
- performance;

real hardware is the final authority.

Never silently substitute Eden/Yuzu/other emulator success for a hardware gate.

## Logging/debugging

- Retain the complete device log for a hardware experiment when practical.
- Do not grep-slice before the first full-log inspection.
- Decode libnx `Result` values before theorizing from the raw integer alone.
- Capture GPU error notifier/error-info data after timeouts or GPU faults when the path supports it.
- Avoid high-volume per-frame SD logging in performance runs; instrumentation itself can dominate timing.
- Separate diagnostic builds from performance builds.

Historical debugging heuristics are in `docs/knowledge/nvk_winsys_debugging_patterns.md` and related files.

## Source-of-truth and generated/mirrored code

This repository inherits multiple representations of Mesa changes: patch files, extracted/generated trees, and selected source mirrors.

Before editing Mesa-derived code:

1. determine which representation is authoritative for the reproducible build;
2. identify all generated/mirrored copies;
3. avoid fixing only a disposable extracted tree;
4. update the durable source/patch and any intentionally tracked mirror required by the build;
5. verify `REPRODUCE.md` still describes reality.

Do not "clean up" old research files simply because they are stale; they may be valuable evidence.

## Implementation style

- Prefer direct C/C++/Mesa/libnx primitives over framework creation.
- Keep Horizon-specific code behind explicit platform boundaries.
- Preserve upstream Mesa semantics where possible; isolate Switch deltas.
- Do not fake Linux concepts if Horizon has a clearer native primitive. For example, external memory should be designed around the actual `NvMap`/GPU-VA ownership model rather than inventing dma-buf behavior unless Mesa integration specifically requires an internal compatibility representation.
- Make ownership and lifetime explicit for GPU memory and fences.
- Favor deterministic smoke tests over visual-only demos.
- Include negative/failure-path tests for low-level memory and synchronization code.

## Performance work

Correctness first, then latency.

For performance claims record at minimum:

- hardware/configuration;
- resolution/format;
- buffer count/present mode;
- clocks if changed from normal;
- build type/diagnostics enabled;
- measurement point and units;
- sample count/distribution when relevant.

Do not report FPS alone when the milestone concerns latency or scheduling. Measure the relevant phases.

## Native-title integration research

Keep native-title-specific work outside reusable winsys/runtime primitives.

The dependency direction should remain:

```text
native-title adapter/research probe
        -> external image + sync APIs
        -> frame-generation pipeline
        -> switch-nvk runtime
```

Do not hard-code game addresses, layouts, or assumptions into the core NVK/winsys layer.

A demanding commercial title is a late-stage validation target, not a substitute for controlled interoperability tests.

## Provenance

Do not commit proprietary SDK files, proprietary binaries, game assets, private dumps, or source material without redistribution rights.

For imported/open-source code record:

- upstream URL/repository;
- version/commit;
- license;
- local modifications.

For reverse-engineering-derived behavior, document the observable interface/behavior and reference source without committing restricted artifacts.

## PR expectations

A meaningful PR should answer:

- Which milestone/spec does this advance?
- What changed?
- What claim is newly supported?
- What evidence exists now?
- What remains unproven?
- What tests ran?
- Was real hardware required/run?
- What new risk or fallback exists?

Use `.github/pull_request_template.md`.

## Stop conditions

Stop implementation and record a blocker instead of layering guesses when:

- a lower-level primitive is unproven;
- ownership semantics are unknown and guessing risks corruption;
- the test cannot distinguish success from a false positive;
- hardware contradicts the assumed architecture;
- required provenance/licensing is unclear for material that would be committed or redistributed.

A well-documented `BLOCKED` or `REJECTED` result is preferable to speculative code that appears to work.
