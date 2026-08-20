# Architecture Decision Records

Use `docs/decisions/` for durable decisions that future milestones should inherit.

Do not create an ADR for every implementation choice. Create one when reversing the decision later would require meaningful architectural work or when multiple plausible approaches exist and the rationale matters.

## Naming

```text
NNN-short-decision.md
```

Example:

```text
001-horizon-native-external-memory.md
002-native-fence-presentation.md
```

## ADR template

```markdown
# ADR NNN — <Title>

Status: PROPOSED | ACCEPTED | SUPERSEDED | REJECTED
Date: YYYY-MM-DD
Related spec/milestone: <path/ID>

## Context

What forces the decision? What evidence exists?

## Decision

What are we choosing?

## Alternatives considered

### Alternative A
Why it was plausible and why it was not selected.

### Alternative B
...

## Consequences

### Positive
- ...

### Negative / trade-offs
- ...

## Validation

What experiment or evidence supports the decision?

## Revisit when

Concrete conditions that would justify reopening the decision.
```

## Rules

- A `PROPOSED` ADR is not an architecture guarantee.
- Prefer evidence-backed `ACCEPTED` decisions after the relevant experiment.
- Never erase a superseded ADR; link the replacement.
- Keep experimental iteration details in `docs/research/`, not in the ADR.
