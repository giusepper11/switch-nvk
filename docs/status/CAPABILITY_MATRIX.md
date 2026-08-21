# Capability Matrix

Baseline for the frame-generation fork. This file is the concise current-state source of truth for capabilities relevant to the new project.

Last reviewed: 2026-08-20

Status vocabulary is defined in `PROJECT.md` / `AGENTS.md`.

## Runtime / GPU

| Capability | Status | Evidence / location | Next gate |
|---|---|---|---|
| GM20B device discovery on Horizon | PROVEN_HW | historical M2 evidence in `RESUME_NVK.md`, `docs/knowledge/` | retain regression |
| GPU VA / VM_BIND path | PROVEN_HW | M2 smoke + winsys history | retain regression |
| GPFIFO submit / GPU execution | PROVEN_HW | `winsys/smoke/nvk_smoke.c` + hardware history | async semantics in FG-3 |
| CPU/GPU coherency for validated path | PROVEN_HW | CAFEBABE M2 smoke | revalidate for new external-memory paths |
| NAK vertex/fragment shaders | PROVEN_HW | triangle/scene tests | retain regression |
| Compute shader dispatch on this Horizon winsys | PROVEN_HW | `winsys/smoke/nvk_compute.c`, immutable-commit FG-1 hardware evidence | retain 64-iteration regression |
| Storage-buffer compute | PROVEN_HW | `winsys/smoke/nvk_compute.c`, immutable-commit FG-1 hardware evidence | retain 64-iteration regression |
| Storage-image compute | PROVEN_HW | `winsys/smoke/nvk_compute.c`, immutable-commit FG-1 hardware evidence | retain 64-iteration regression |
| Compute root upload/QMD diagnostics | PROVEN_HW | `docs/testing/FG2_ROOT_STATE_DIAGNOSTIC_RUN_2026-08-20.md`; current seed proven through CPU root/upload mapping and dispatched QMD encoding | retain observation-only diagnostic; do not infer GPU visibility |
| Render-to-texture pipeline | IMPLEMENTED_UNPROVEN | expected Vulkan/NVK behavior but no dedicated acceptance test in current baseline | FG-2 |
| Render → sampled image → compute → storage image chain | BLOCKED | initial/shared-layout failures plus `docs/testing/FG2_ROOT_STATE_DIAGNOSTIC_RUN_2026-08-20.md`; current seed reaches CPU upload mapping and dispatched QMD, but output retains the identical stale signature | controlled root-upload cache-visibility experiment; GPU consumption remains unresolved |

## Graphics feature baseline

| Capability | Status | Evidence / location |
|---|---|---|
| Triangle / graphics pipeline | PROVEN_HW | `winsys/smoke/nvk_tri.c` |
| Texture upload/sampling | PROVEN_HW | `winsys/smoke/nvk_logo.c` |
| 3D + depth | PROVEN_HW | `winsys/smoke/nvk_scene.c` |
| Indexed UINT16/UINT32 draw | PROVEN_HW | `winsys/smoke/nvk_indexed.c` |
| Multi-draw / pipeline switches | PROVEN_HW | `winsys/smoke/nvk_multi.c` |
| Alpha blending | PROVEN_HW | `winsys/smoke/nvk_multi.c` |
| Mipmaps | PROVEN_HW | `winsys/smoke/nvk_textures.c` |
| sRGB sampling | PROVEN_HW | `winsys/smoke/nvk_textures.c` |
| BC1 sampling | PROVEN_HW | `winsys/smoke/nvk_textures.c` |
| Cubemap / samplerCube | PROVEN_HW | `winsys/smoke/nvk_cubemap.c` |

## WSI / presentation

| Capability | Status | Evidence / location | Notes / next gate |
|---|---|---|---|
| `VK_NN_vi_surface` | PROVEN_HW | `winsys/wsi/wsi_common_switch.c`, `nvk_vi_swapchain` history | inherited baseline |
| Vulkan swapchain -> nwindow/VI | PROVEN_HW | hardware history in `RESUME_NVK.md` | inherited baseline |
| zero-copy block-linear scanout | PROVEN_HW | WSI code + 2026-06-02 history | inherited baseline |
| swapchain recreate after extent change | PROVEN_HW | upstream hardware-tested commit/history | regression needed when WSI changes |
| triple-buffer capability | PROVEN_HW | durable Mesa patch changed minimum to 3 after HW profiling | tracked mirror may still contain stale value; canonicalization needed when touched |
| native GPU fence passed to VI | IMPLEMENTED_UNPROVEN | current WSI fast path historically CPU-waits render fence then queues with null native fence | FG-4 |
| asynchronous present dependency chain | BLOCKED | depends on native fence bridge / submit semantics | FG-3, FG-4 |

## Synchronization

| Capability | Status | Evidence / location | Next gate |
|---|---|---|---|
| libdrm-style sync objects backed by `NvFence` | PROVEN_HW | `winsys/drm_shim.c` + M2 history | retain regression |
| timeline-related shim behavior used by NVK | PROVEN_HW | upstream Dawn bring-up history | needs regression under async changes |
| non-blocking `EXEC` submit | BLOCKED | current inherited `nouveau_exec` drains with `nvFenceWait` | FG-3 |
| native `NvFence`/`NvMultiFence` export from Vulkan completion | IMPLEMENTED_UNPROVEN | historical plan/issue describes missing fast path | FG-4 |
| cross-context/native-producer fence import | NOT_STARTED | new project capability | NI-3 |

## External memory / image interoperability

| Capability | Status | Evidence / location | Next gate |
|---|---|---|---|
| expose NVK BO's nvmap id for WSI | PROVEN_HW | zero-copy WSI helper path | inherited, NVK-owned BO only |
| import arbitrary existing NvMap allocation as NVK memory | NOT_STARTED | no generic import contract | FG-5 |
| Linux dma-buf PRIME interop | REJECTED | shim explicitly returns `ENOSYS`; Horizon-native path preferred | do not use as capability claim |
| externally described block-linear image -> NVK image | NOT_STARTED | no generic external-image contract | FG-6 |
| borrowed external-resource lifetime semantics | NOT_STARTED | must be designed/tested | FG-5 |

## Frame generation

| Capability | Status | Next gate |
|---|---|---|
| deterministic interpolation compute shader | NOT_STARTED | FG-7 after FG-1/2 |
| zero-copy synthetic source -> generated -> VI pipeline | NOT_STARTED | FG-7 |
| asynchronous synthetic frame-generation pipeline | NOT_STARTED | FG-3/4/7 |
| production frame-generation algorithm integration | NOT_STARTED | FG-8 |

## Native NVN/title interoperability

| Capability | Status | Next gate |
|---|---|---|
| observe native final-frame/presentation boundary | NOT_STARTED | NI-1 |
| identify/import native producer's image memory | NOT_STARTED | NI-1/NI-2 |
| interpret native NVN image layout in NVK | NOT_STARTED | NI-2 |
| native producer -> NVK synchronization | NOT_STARTED | NI-3 |
| generated-frame insertion/presentation interposition | NOT_STARTED | NI-4 |
| title-specific validation | NOT_STARTED | VAL-1 |

## Integration abstractions

| Capability | Status | Evidence / location |
|---|---|---|
| packaged static Vulkan/NVK consumer library | PROVEN_HOST | `package-nvk.sh`, historical link validation |
| Dawn -> NVK -> Switch WSI | PROVEN_HW | Dawn clear hardware bring-up history |

## Known project-level risks

### Source-of-truth drift

The inherited repository contains durable Mesa patch files plus selected tracked source copies and local extracted trees. At least one historical WSI value (triple-buffer minimum image count) has differed between the durable patch and the standalone tracked WSI copy. Any change in these areas must identify and update the build-authoritative representation.

### Licensing

The fork inherits `GPL-2.0-or-later` plus an upstream `FORK_POLICY.md`; upstream has received a compatibility challenge regarding extra restrictions. Treat distribution/licensing questions as unresolved until reviewed. This does not change technical experiment status.

### Performance baseline

Historical performance results are useful context but are not automatically the baseline for this fork. New scheduling/frame-generation claims require measurements tied to an exact fork commit and hardware configuration.
