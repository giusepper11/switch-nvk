# MILESTONES.md — Frame-Generation Research Gates

This roadmap is deliberately gate-based rather than date-based. Do not start a milestone because the previous one "looks close". Start it when its acceptance criteria are satisfied or when the next experiment is explicitly needed to resolve a blocker.

Status values: `NOT_STARTED`, `IN_PROGRESS`, `BLOCKED`, `PROVEN_HOST`, `PROVEN_HW`, `REJECTED`.

## M0 — Project foundation

**Status:** IN_PROGRESS

Goal: make the fork safe for repeatable agent-driven research.

Deliverables:

- `PROJECT.md`
- `MILESTONES.md`
- `AGENTS.md`
- architecture and capability matrix
- OpenSpec, ADR, research, and hardware-evidence conventions
- PR template

Acceptance:

- a new contributor/agent can identify the current goal, next milestone, evidence standard, and source-of-truth documents without reading the entire historical notebook;
- historical upstream documents remain intact.

---

## FG-1 — Compute + storage-image hardware proof

**Status:** PROVEN_HW

Goal: prove the exact GPU primitives required by a frame interpolation pipeline on this NVK/Horizon stack.

Implement the smallest dedicated test covering:

- `VkPipeline` compute creation;
- NAK compute shader execution;
- storage buffers;
- storage images;
- sampled image -> compute -> storage image;
- required image layouts/barriers;
- deterministic CPU readback validation.

Suggested test: `winsys/smoke/nvk_compute.c` or equivalent.

Acceptance:

- test passes on real Tegra hardware;
- known input produces deterministic expected output;
- no GPU fault/error notifier;
- evidence retained according to `docs/testing/HARDWARE_EVIDENCE.md`.

Current implementation status: `nvk_compute` passed on a real Nintendo Switch OLED: 64/64 exact
iterations, matching buffer/image checksums, complete device-log inspection, and no reported GPU fault.

Do not call compute "supported for framegen" until this passes.

---

## FG-2 — Render-to-texture and compute/image chaining

**Status:** NOT_STARTED

Goal: validate the image transitions used by temporal pipelines.

Prove:

1. render to offscreen image;
2. sample that image in a later pass;
3. consume it from compute;
4. write a second image from compute;
5. present or read back the result.

Acceptance:

- hardware-proven deterministic result;
- barriers/layout transitions are explicit and validation logic is documented;
- no hidden CPU copy is required for the processing chain.

---

## FG-3 — Truly asynchronous GPU submission

**Status:** NOT_STARTED

Goal: remove unconditional completion waits from the submission fast path.

Current research concern: the winsys `EXEC` path historically drains each submission with `nvFenceWait`, which serializes work and prevents a useful frame-generation scheduler.

Target behavior:

```text
submit GPU work
    -> associate native completion fence with Vulkan sync object
    -> return to caller
    -> wait only when Vulkan synchronization semantics require it
```

Acceptance:

- multiple dependent/independent submissions can remain in flight;
- Vulkan fences/semaphores preserve correctness;
- no unconditional per-submit `nvFenceWait` or `vkQueueWaitIdle` in the steady-state path;
- regression smoke tests continue to pass;
- hardware trace demonstrates overlap or at minimum non-blocking submit behavior.

---

## FG-4 — Native fence bridge + asynchronous VI present

**Status:** NOT_STARTED

Goal: bridge Vulkan completion directly to libnx/VI synchronization.

Research/implementation target:

- obtain or construct the correct `NvFence` / `NvMultiFence` payload corresponding to render completion;
- pass that payload to `nwindowQueueBuffer`;
- avoid CPU waiting before handing the image to VI;
- preserve a correctness-first fallback path.

Acceptance:

- VI waits on GPU completion without a CPU render-complete wait in the normal path;
- zero-copy scanout remains correct;
- stable frame pacing under triple buffering;
- fallback path is tested and documented.

---

## FG-5 — External NvMap memory import

**Status:** NOT_STARTED

Goal: make an existing `NvMap` allocation consumable by the NVK winsys without pretending it was allocated by NVK.

Design target:

```text
external nvmap id + size + mapping metadata
              ↓
      Switch winsys external BO
              ↓
         NVK memory object
```

Requirements:

- native Horizon/NvMap abstraction rather than assuming Linux dma-buf exists;
- explicit ownership/lifetime semantics;
- no double-free/unmap;
- clear distinction between CPU address, nvmap object, GPU VA, and Vulkan memory binding.

Acceptance:

- a controlled external allocation can be imported and read/written by GPU work on real hardware;
- original owner remains valid according to the defined lifetime contract;
- negative tests cover invalid metadata and premature release.

---

## FG-6 — External block-linear image import

**Status:** NOT_STARTED

Goal: correctly interpret externally supplied GM20B image memory as an NVK `VkImage`-compatible surface.

Inputs to characterize explicitly include:

- width/height/format;
- nvmap id / offset / size;
- pitch/stride;
- block-linear layout;
- GOB/block-height metadata;
- PTE kind / scanout kind where applicable;
- GPU VA mapping requirements.

Acceptance:

- externally created known-pattern image is sampled correctly by NVK;
- NVK writes can be observed correctly by the external consumer where bidirectional use is intended;
- tiling mismatches fail safely or are detected;
- test is hardware-proven.

This milestone proves external-image interoperability. It does **not** prove NVN interoperability.

---

## FG-7 — Synthetic frame-generation harness

**Status:** NOT_STARTED

Goal: prove the complete frame-generation runtime while the project owns all source images and synchronization.

Pipeline:

```text
source frame N-1 ─┐
                  ├─> interpolation compute -> generated image -> WSI -> VI
source frame N   ─┘
```

Initially use a simple deterministic interpolation shader. Algorithm quality is secondary; pipeline correctness and latency are primary.

Required measurements:

- GPU interpolation time;
- submit CPU time;
- acquire time;
- present/queue time;
- frame interval distribution;
- memory bandwidth estimate where practical;
- resolution and clocks/configuration;
- dropped/repeated frames.

Acceptance:

- real-hardware output is visually and numerically correct;
- zero CPU image copies in steady state;
- no unconditional GPU-wide idle wait in steady state;
- generated frames are paced predictably;
- performance data retained.

**Gate:** do not begin native-title integration work before FG-7 unless a narrowly scoped research experiment is necessary to de-risk FG-5/FG-6.

---

## NI-1 — Native presentation/frame acquisition reconnaissance

**Status:** NOT_STARTED

Goal: determine what can be observed about a native NVN application's final render/presentation path without assuming a specific interception architecture.

Questions to answer:

- where is the final presentable image represented;
- which process/context owns its memory;
- whether relevant `NvMap` identifiers/metadata can be observed;
- what synchronization accompanies presentation;
- what VI/nwindow/buffer-queue boundaries exist;
- what ownership and security boundaries Horizon enforces.

Deliverable: research report and a minimal probe, not a game-specific frame generator.

Acceptance:

- observation points and limitations documented from hardware evidence;
- at least one concrete candidate integration boundary identified or the approach marked blocked/rejected.

---

## NI-2 — NVN -> NVK external-image proof

**Status:** NOT_STARTED

Goal: consume an image produced outside the NVK context using the external-memory primitives from FG-5/FG-6.

The first target should be a controlled producer if possible. A commercial game is a late-stage validation target, not the first interop test.

Acceptance:

- producer renders a known frame;
- NVK waits for safe access;
- NVK samples/processes the exact producer image without CPU copy;
- ownership is returned safely;
- repeated operation is stable.

---

## NI-3 — Cross-context GPU synchronization

**Status:** NOT_STARTED

Goal: prove a safe synchronization chain across the producer, NVK frame-generation work, and presentation consumer.

Target conceptual sequence:

```text
producer render complete
       ↓ native fence
NVK wait / frame-generation submit
       ↓ native fence
VI or downstream consumer wait
```

Acceptance:

- no CPU polling/spin is required in the steady-state dependency chain;
- no tearing/use-before-complete;
- repeated multi-frame run is stable;
- ownership transitions are documented.

---

## NI-4 — Generated-frame presentation/insertion proof

**Status:** NOT_STARTED

Goal: determine whether generated frames can be scheduled between producer frames at the chosen presentation boundary.

This is a presentation research milestone, not an image-quality milestone.

Acceptance:

- alternating real/generated presentation is observable and measurable;
- cadence is intentional rather than accidental queue behavior;
- latency and frame pacing are captured;
- source application remains stable;
- failure/recovery behavior is understood.

---

## FG-8 — Frame-generation algorithm integration

**Status:** NOT_STARTED

Goal: replace the synthetic interpolation shader with a legitimate, native implementation of the selected frame-generation algorithm/assets.

Requirements:

- provenance and licensing documented;
- no assumption that a Windows binary itself can run on Horizon;
- algorithm inputs/outputs and resource requirements explicitly specified;
- correctness tests separate algorithm quality from runtime/presentation correctness.

Acceptance:

- algorithm runs natively through the proven GPU/runtime path;
- performance budget measured at target resolutions;
- representative artifact/quality analysis documented.

---

## VAL-1 — Late-stage native game validation

**Status:** NOT_STARTED

Goal: validate the architecture against a demanding native title only after the reusable layers are proven.

A title such as *The Legend of Zelda: Tears of the Kingdom* may be used as a late-stage validation target, but the core runtime must not become title-specific.

Acceptance criteria must be defined by a dedicated OpenSpec change at that time and include:

- stability;
- correct acquisition and ownership;
- frame pacing;
- added latency;
- generated-frame quality;
- GPU/CPU cost;
- memory cost;
- behavior under dynamic resolution/performance changes;
- reproducible rollback/failure behavior.

---

## Milestone rule

Every milestone gets:

1. one OpenSpec change under `openspec/changes/` before substantial implementation;
2. the smallest practical automated/host validation;
3. a real-hardware acceptance run where hardware behavior is material;
4. a capability-matrix update;
5. retained evidence and any negative findings;
6. an ADR when the work introduces a durable architectural choice.
