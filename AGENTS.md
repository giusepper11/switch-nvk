# AGENTS.md

This file defines how coding/research agents should work in this fork.

## Read order

Before changing code, read only what is needed, in this order:

1. `PROJECT.md`
2. `MILESTONES.md`
3. `docs/status/CAPABILITY_MATRIX.md`
4. the active OpenSpec change under `openspec/changes/` (and relevant current capability specs under `openspec/specs/`)
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

OpenSpec is the only implementation-planning workflow for this repository. For non-trivial work:

1. identify the milestone and the specific unknown or capability claim;
2. use OpenSpec explore when research is needed to clarify the problem;
3. use OpenSpec propose to create a change with `proposal.md`, delta specs under `specs/`, `design.md`, and `tasks.md`;
4. review the proposal, capability delta, design, and tasks before implementation;
5. use OpenSpec apply to implement the smallest vertical slice;
6. run host/build checks;
7. when hardware matters, produce a hardware artifact/test and stop at `IMPLEMENTED_UNPROVEN` until a hardware result exists;
8. analyze the full hardware log, not only matching lines;
9. update the capability matrix, research findings, and ADRs when appropriate;
10. use OpenSpec archive after implementation and sync the accepted delta specs to `openspec/specs/`.

Do not create broad abstractions before the first concrete consumer requires them.

## OpenSpec artifacts

Substantial implementation must be driven by an active change under `openspec/changes/`. The change's proposal, delta specs, design, and tasks must make the following explicit where relevant:

- context, current evidence, and a falsifiable hypothesis;
- scope, non-goals, and the smallest implementation slice;
- interfaces, ownership, lifetime, and synchronization rules;
- host/build validation and real-hardware acceptance criteria;
- observability, logging, and false-positive detection;
- risks, failure modes, rollback/fallback, and unresolved questions.

Accepted behavioral truth belongs in `openspec/specs/`. Experiment logs belong in `docs/research/`, validation policy in `docs/testing/`, and durable architecture decisions in `docs/decisions/`. Do not create numbered implementation specs under `docs/specs/`.

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
