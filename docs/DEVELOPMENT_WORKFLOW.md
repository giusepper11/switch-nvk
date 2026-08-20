# Development Workflow

This fork uses milestone-driven, spec-driven vertical slices.

## 1. Pick the next gate, not the most interesting feature

Start from `MILESTONES.md` and `docs/status/CAPABILITY_MATRIX.md`.

Default rule: work on the earliest unproven prerequisite for the intended outcome.

Example:

```text
Do not start native-title presentation hooks
if external image import and native fence bridging are still unproven.
```

A later-layer research probe is acceptable only when it is explicitly required to resolve an earlier design unknown.

## 2. Open or create a spec

For non-trivial changes, create a spec from `docs/specs/000-template.md`.

Keep the spec narrow. A useful spec should normally be finishable as one vertical slice or a small sequence of PRs with the same acceptance test.

Bad scope:

> Implement external memory, synchronization, capture, frame generation, and presentation.

Good scope:

> Import one controlled external NvMap allocation as a borrowed GPU buffer and validate deterministic GPU read/write without changing image handling.

## 3. Establish the baseline

Before modifying low-level code:

- identify the current authoritative implementation;
- run/inspect the smallest existing regression relevant to the area;
- record current behavior/timing when the change is performance-sensitive;
- identify fallback behavior that could mask failure.

For Mesa changes, explicitly determine whether the durable patch, tracked source mirror, or extracted tree drives the reproducible build.

## 4. Implement one new primitive

Prefer:

```text
new primitive + dedicated smoke test
```

over:

```text
new framework + multiple speculative consumers
```

The test should fail before the implementation for the expected reason whenever practical.

## 5. Validate locally/host-side

Run the available compile/link/unit/static checks.

Host validation can move status to `PROVEN_HOST` or `IMPLEMENTED_UNPROVEN`; it cannot establish Tegra behavior.

## 6. Produce the hardware artifact

When hardware is required:

- create a dedicated NRO/test artifact where practical;
- make success deterministic rather than visual-only;
- ensure the artifact reports which fast/fallback path ran;
- include relevant GPU fault/error state;
- avoid instrumentation that invalidates performance measurements.

## 7. Hardware result states

### Pass

If all hardware acceptance criteria pass:

- mark spec `ACCEPTED`;
- promote capability to `PROVEN_HW`;
- update milestone status if its gate is complete;
- record surprising findings in `docs/research/`;
- create/update ADR if the experiment established a durable architecture choice.

### Partial/uncertain

If code runs but evidence cannot distinguish the intended mechanism from fallback or a false positive:

- keep `IMPLEMENTED_UNPROVEN`;
- refine observability/test before adding more architecture.

### Fail

If the hypothesis is falsified:

- keep the experiment/research note;
- mark approach/spec `REJECTED` or `BLOCKED` as appropriate;
- state what was learned;
- choose the next smallest discriminating experiment.

Do not hide the failure by widening the change.

## 8. PR shape

Prefer PRs that make exactly one new claim.

Examples:

- "Prove compute storage-image dispatch on GM20B"
- "Make Nouveau EXEC return without draining the submitted fence"
- "Pass render-complete NvMultiFence to nwindow present"
- "Import borrowed NvMap as buffer memory"

A PR may include supporting refactors, but its acceptance criteria should still center on one capability.

## 9. After a task is delivered

Do not regenerate the whole project plan.

The loop is:

```text
spec -> implement -> validate -> update evidence/status -> select next milestone -> next spec
```

`PROJECT.md` should change rarely. `MILESTONES.md` changes when sequencing or gates genuinely change. The capability matrix changes frequently as evidence accumulates.

## 10. Agent handoff

At the end of substantial work, leave enough state that a new agent can continue without reconstructing the session:

- active milestone/spec;
- exact status (`PROVEN_HW`, `BLOCKED`, etc.);
- commit/branch;
- tests run;
- hardware result/evidence location;
- remaining unknown;
- single recommended next action.

Put durable facts in repository docs rather than relying on chat history.
