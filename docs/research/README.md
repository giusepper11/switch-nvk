# Research Notes

`docs/research/` is the experiment log for unresolved technical questions, failed approaches, hardware observations, and measurements that are too detailed for a spec or capability matrix.

## What belongs here

- hypotheses that need investigation;
- experiment plans/results;
- hardware logs summarized with links/paths to retained raw evidence;
- reverse-engineering observations about public/observable interfaces;
- rejected approaches and why they failed;
- performance investigations;
- comparisons between emulator and real hardware behavior;
- questions that cross multiple milestones.

## What does not belong here

- the stable project goal (`PROJECT.md`);
- ordered project gates (`MILESTONES.md`);
- the final architecture contract (`docs/architecture/ARCHITECTURE.md`);
- long-lived decisions (`docs/decisions/`);
- raw proprietary binaries/dumps that cannot legally be redistributed.

## Naming

Prefer dated or topic-focused notes:

```text
2026-08-20-async-submit-baseline.md
external-nvmap-lifetime-notes.md
vi-native-fence-experiment.md
```

## Experiment note template

```markdown
# <Experiment title>

Date: YYYY-MM-DD
Related milestone/spec: <ID/path>
Status: HYPOTHESIS | IN_PROGRESS | PROVEN_HOST | PROVEN_HW | BLOCKED | REJECTED

## Question

What single question are we trying to answer?

## Baseline

What was already known/proven?

## Setup

- hardware
- build/commit
- test artifact
- configuration/resolution/clocks
- diagnostics enabled

## Procedure

Minimal reproducible steps.

## Result

Observed facts. Keep interpretation separate.

## Evidence

Logs, checksums, photos/video references, timing output, error notifier data.

## Interpretation

What the result supports or falsifies.

## False-positive check

Could another mechanism explain the apparent result?

## Next action

One concrete next experiment or milestone update.
```

## Negative results

Never delete a negative result because a later implementation succeeds. Record the failed approach and the distinguishing detail that made the successful approach different.

This repository already benefited from failures such as headless WSI/presentation experiments. Preserve that culture: failures narrow the search space.

## Raw evidence

Large device logs, binaries, captures, or private material should not automatically be committed.

When raw evidence cannot live in Git:

- record a stable local/artifact identifier if available;
- include the relevant summarized observations and checksums/metadata;
- state why the raw artifact is not committed;
- never fabricate a path or claim evidence was retained when it was not.
