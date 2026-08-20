# Specifications

`docs/specs/` contains implementation-facing specifications for milestone work.

The purpose of a spec is to make an experiment falsifiable before code complexity accumulates. Specs are deliberately smaller than a traditional full-project design document.

## Naming

Use:

```text
NNN-short-name.md
```

Examples:

```text
001-compute-storage-image-smoke.md
002-async-submit.md
003-native-fence-wsi.md
004-external-nvmap-import.md
```

Numbers are sequencing aids, not milestone IDs. Include the milestone ID inside the spec.

## Lifecycle

A spec has one of:

- `DRAFT`
- `ACTIVE`
- `BLOCKED`
- `ACCEPTED`
- `REJECTED`
- `SUPERSEDED`

Keep rejected specs. A failed approach is reusable research evidence.

## Scope

Create a spec for changes that introduce or materially alter:

- GPU execution behavior;
- memory ownership/mapping;
- synchronization;
- WSI/presentation;
- external interoperability;
- frame-generation pipeline behavior;
- performance-sensitive scheduling;
- native-title integration boundaries.

Do not create a spec for a typo, obvious documentation repair, mechanical build fix, or narrow test maintenance unless the behavior itself is uncertain.

## Required content

Use `000-template.md`. Every active spec should make the following answerable before implementation becomes large:

1. What exactly are we trying to prove?
2. What evidence do we already have?
3. Which layer is under test?
4. What is explicitly out of scope?
5. What is the smallest implementation that can falsify the hypothesis?
6. How will host/build validation work?
7. How will real hardware validate it?
8. What output/log distinguishes success from a false positive?
9. What are the ownership and synchronization rules?
10. What do we do if the approach fails?

## Updating project state

When a spec becomes `ACCEPTED`, update:

- `docs/status/CAPABILITY_MATRIX.md`;
- `MILESTONES.md` if the milestone gate changes;
- relevant ADR if a durable architecture decision was made;
- research notes for surprising/negative findings.

Do not convert the spec itself into a chronological debug diary. Put detailed experiment iterations under `docs/research/` and keep the spec focused on the contract and final outcome.
