# Deprecated specification directory

This directory is retained as a migration marker for the former numbered `docs/specs/NNN-*.md` workflow. Do not create new implementation specs here.

OpenSpec is now the repository’s single implementation-planning workflow:

- current behavioral capability truth belongs in `../../openspec/specs/`;
- proposed changes, designs, delta specs, and implementation tasks belong in `../../openspec/changes/<change>/`;
- research logs remain in `../research/`;
- hardware-validation policy remains in `../testing/HARDWARE_EVIDENCE.md`;
- durable architecture decisions remain in `../decisions/`.

The former template has been removed so this directory cannot continue to advertise a competing workflow. The useful requirements it captured—falsifiable hypotheses, narrow scope, ownership and synchronization, hardware acceptance, observability, false-positive analysis, rollback, and failure handling—are now expected in the relevant OpenSpec artifacts and project guidance.
