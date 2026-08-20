# PROJECT.md — Switch NVK Frame-Generation Research Fork

## Purpose

This fork evolves `switch-nvk` from a Vulkan-on-Horizon bring-up project into a research-grade GPU runtime and test harness for low-latency frame-generation experiments on Nintendo Switch hardware.

The immediate goal is **not** to claim that frame generation for native commercial games is feasible. The goal is to turn the already-proven NVK/GM20B/VI stack into a platform where the remaining unknowns can be isolated and tested rigorously.

The project treats these as separate technical problems:

1. Vulkan/compute execution on GM20B under Horizon.
2. Low-latency asynchronous GPU submission.
3. Native GPU fence export/import and VI synchronization.
4. External `NvMap` memory interoperability.
5. Correct interpretation of block-linear external images.
6. Frame interpolation on GPU-resident images.
7. Acquisition of frames produced by a native NVN application.
8. Cross-context synchronization between NVN and NVK work.
9. Presentation/insertion of generated frames.
10. Frame pacing, latency, image quality, stability, and performance.

Success at one layer is not evidence that another layer works.

## Project outcome

A successful research outcome is one of the following:

- a validated architecture for native frame generation on Switch;
- a validated subset of the architecture with clearly documented blockers;
- or a high-confidence finding that one or more required layers are not practical on the target hardware/runtime.

A negative result is a valid project result when it is reproducible and well evidenced.

## Near-term target

The first major target is a self-contained frame-generation harness that owns all of its images:

```text
frame N-1 ─┐
           ├─> Vulkan compute/interpolation ─> generated frame ─> zero-copy WSI ─> VI
frame N ───┘
```

The harness must run on real Tegra X1 hardware without CPU image copies and without unconditional GPU-wide waits in the steady-state frame loop.

Only after that path is proven should the project move into native NVN frame acquisition and cross-context interoperability research.

## Existing foundation inherited from upstream

The fork starts from an unusually strong baseline:

- Mesa NVK cross-compiled for Horizon/libnx.
- GM20B device discovery and GPU execution through the custom Nouveau/libdrm shim.
- VM bind, GPFIFO submission, GPU fences, and CPU/GPU coherency.
- NAK graphics shaders on real Tegra.
- Textures, depth, indexed rendering, blending, cubemaps, mipmaps, sRGB, and BC1 hardware tests.
- `VK_NN_vi_surface` implementation over libnx `nwindow`/VI.
- zero-copy block-linear scanout using `NvGraphicBuffer`.
- Dawn-over-Vulkan proof on real hardware.

These inherited capabilities remain evidence-backed baseline assumptions, but new work must not silently extrapolate beyond them.

## Non-goals for the initial phases

Until the prerequisite milestones are green, do not optimize for or claim support for:

- a specific commercial game;
- arbitrary NVN interception;
- executing a Windows DLL directly on Horizon;
- production-quality frame-generation image quality;
- broad Vulkan conformance;
- emulator-only success as a substitute for hardware validation;
- invasive architecture changes that are not required by the current milestone.

## Engineering principles

### 1. Prove the layer below first

Every milestone should have the smallest possible standalone hardware test. Do not debug an upper layer while the dependency below it is still assumed.

### 2. Evidence over plausibility

Classify claims as one of:

- **PROVEN_HW** — reproduced on real Switch/Tegra hardware with retained evidence.
- **PROVEN_HOST** — build/unit/static validation only.
- **IMPLEMENTED_UNPROVEN** — code exists but required hardware proof is missing.
- **HYPOTHESIS** — reasoned design that has not yet been implemented.
- **BLOCKED** — prerequisite or technical blocker identified.
- **REJECTED** — tested approach shown not to work or intentionally abandoned.

Do not upgrade a claim without evidence.

### 3. Minimal vertical slices

Prefer one small end-to-end capability over broad infrastructure. A milestone should normally add one new primitive and one test that proves it.

### 4. Preserve upstream knowledge

The existing `PLAN_*`, `RESUME_NVK.md`, `ROADMAP.md`, and `docs/knowledge/` files are historical evidence and debugging knowledge. Do not rewrite them to make the new project look cleaner. New project state belongs in the documents introduced by this fork.

### 5. Canonical source of truth

For the new fork:

- project intent and boundaries: `PROJECT.md`
- ordered gates: `MILESTONES.md`
- current capability status: `docs/status/CAPABILITY_MATRIX.md`
- architecture: `docs/architecture/ARCHITECTURE.md`
- agent workflow: `AGENTS.md`
- current behavioral capability specs: `openspec/specs/`
- proposed changes and implementation plans: `openspec/changes/`
- hardware proof requirements: `docs/testing/HARDWARE_EVIDENCE.md`
- experiments and unknowns: `docs/research/`
- architectural decisions: `docs/decisions/`

If an older upstream document conflicts with these files about the **new frame-generation effort**, these files win. Older documents remain authoritative for the historical upstream bring-up they describe.

## Scope boundary: research versus application integration

Keep the project layered:

```text
application / native-title integration research
                 │
                 ▼
external image + synchronization bridge
                 │
                 ▼
frame-generation pipeline
                 │
                 ▼
Switch NVK runtime / winsys / WSI
                 │
                 ▼
libnx nv + nwindow + VI + GM20B
```

Do not couple the core runtime to one game or one frame-generation algorithm. Game-specific and algorithm-specific adapters should sit above reusable memory, synchronization, compute, and presentation primitives.

## Legal and provenance constraints

- Do not commit proprietary Nintendo SDK code, proprietary game assets, copyrighted binaries, or redistributable material that the repository is not licensed to distribute.
- Prefer clean-room interfaces based on public/open-source headers, observed behavior, or user-supplied local assets where legally appropriate.
- Keep provenance for imported code, reverse-engineering references, shaders, and generated artifacts.
- The fork inherits upstream licensing complexity; resolve licensing questions before redistributing combined binaries or integrating code under incompatible terms.

## Definition of project maturity

The project is **research-ready** when documentation, build reproduction, milestone gates, and hardware evidence conventions are in place.

The project is **framegen-runtime-ready** when the synthetic frame-generation harness is fully asynchronous and hardware-proven.

The project is **native-interop-ready** when an externally allocated image can be consumed correctly by NVK with explicit synchronization.

The project is **native-framegen-ready** only when acquisition, interop, scheduling, generated-frame presentation, and pacing have all been proven independently and then together on hardware.
