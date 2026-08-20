# docs/ — project guidance, knowledge, and research trail

This fork preserves the accumulated research behind getting **Mesa NVK (Vulkan) running on Nintendo Switch GM20B**, while adding a new project layer for low-latency frame-generation and external-GPU-interoperability research.

## Start here for the fork

1. **[`../PROJECT.md`](../PROJECT.md)** — project purpose, boundaries, evidence vocabulary, and source-of-truth rules.
2. **[`../MILESTONES.md`](../MILESTONES.md)** — ordered research/implementation gates.
3. **[`status/CAPABILITY_MATRIX.md`](status/CAPABILITY_MATRIX.md)** — what is currently proven, unproven, blocked, or rejected.
4. **[`../AGENTS.md`](../AGENTS.md)** — operating contract for coding/research agents.
5. **[`DEVELOPMENT_WORKFLOW.md`](DEVELOPMENT_WORKFLOW.md)** — spec -> implementation -> hardware evidence -> next-gate workflow.
6. **[`architecture/ARCHITECTURE.md`](architecture/ARCHITECTURE.md)** — subsystem boundaries and target data/synchronization flow.

## Working documentation

- **[`specs/`](specs/README.md)** — implementation-facing specs; start from `specs/000-template.md`.
- **[`research/`](research/README.md)** — experiment logs, negative results, measurements, and unresolved questions.
- **[`testing/HARDWARE_EVIDENCE.md`](testing/HARDWARE_EVIDENCE.md)** — minimum evidence required for `PROVEN_HW`.
- **[`decisions/`](decisions/README.md)** — architecture decision records for durable choices.
- **[`status/CAPABILITY_MATRIX.md`](status/CAPABILITY_MATRIX.md)** — concise live capability state.

## Historical upstream bring-up documentation

The files below remain important evidence and debugging knowledge from the original NVK-on-Switch work. They are intentionally preserved rather than rewritten into the new project structure.

- **[`../BUILD_AND_RUN.md`](../BUILD_AND_RUN.md)** — build/run pipeline, Docker, dependencies, and device workflow.
- **[`../REPRODUCE.md`](../REPRODUCE.md)** — fresh-machine reproduction path.
- **[`../RESUME_NVK.md`](../RESUME_NVK.md)** — historical live notebook/source of truth for the upstream bring-up.
- **[`../PLAN_NVK.md`](../PLAN_NVK.md)** — original phased NVK plan.
- **[`../PLAN_WSI_NWINDOW.md`](../PLAN_WSI_NWINDOW.md)** — VI/nwindow WSI research and performance plan.
- **[`../PLAN_DAWN_VULKAN.md`](../PLAN_DAWN_VULKAN.md)** — Dawn-over-Vulkan bridge work.
- **[`../ROADMAP.md`](../ROADMAP.md)** — original ports-readiness roadmap.

For the **new frame-generation effort**, `PROJECT.md`, `MILESTONES.md`, the capability matrix, and active specs take precedence when an older status document is stale.

## knowledge/ — accumulated research & debugging trail

- **[`vulkan_nvk_switch_path.md`](knowledge/vulkan_nvk_switch_path.md)** — detailed chronological port history.
- **[`nvk_winsys_debugging_patterns.md`](knowledge/nvk_winsys_debugging_patterns.md)** — reusable low-level debugging heuristics and anti-patterns.
- **[`dan_nvk_intel_and_goal.md`](knowledge/dan_nvk_intel_and_goal.md)** — prior Switch-NVK research context and hints.
- **[`feedback_nvk_read_full_log.md`](knowledge/feedback_nvk_read_full_log.md)** — standing full-device-log workflow rule.
- **[`SKILL-switch-port.md`](knowledge/SKILL-switch-port.md)** — broader historical Switch-port dossier.

Some historical docs use `[[wiki-link]]` cross-references from their original authoring workflow. Preserve them unless a focused documentation cleanup explicitly migrates the links.
