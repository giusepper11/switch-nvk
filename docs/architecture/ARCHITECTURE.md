# Architecture

## 1. Objective

This document describes the intended architecture of the frame-generation research fork. It is a boundary map, not a claim that every component is currently feasible.

The architecture must preserve one central property: each uncertain interface can be tested independently.

## 2. System layers

```text
┌──────────────────────────────────────────────────────────────┐
│ Native-title integration / controlled producer adapters      │
│ status: research boundary                                    │
└───────────────────────────┬──────────────────────────────────┘
                            │ external image + producer fence
                            ▼
┌──────────────────────────────────────────────────────────────┐
│ External interoperability layer                              │
│ - NvMap import                                               │
│ - external image metadata/layout                             │
│ - ownership/lifetime                                        │
│ - native fence import/export                                 │
└───────────────────────────┬──────────────────────────────────┘
                            │ VkImage-compatible source(s)
                            ▼
┌──────────────────────────────────────────────────────────────┐
│ Frame-generation pipeline                                   │
│ - temporal input images                                     │
│ - compute/interpolation                                     │
│ - generated output image                                    │
│ - quality/algorithm adapter                                  │
└───────────────────────────┬──────────────────────────────────┘
                            │ generated image + completion sync
                            ▼
┌──────────────────────────────────────────────────────────────┐
│ switch-nvk Vulkan runtime                                   │
│ - Mesa NVK                                                  │
│ - NAK                                                       │
│ - Vulkan runtime/WSI                                        │
│ - custom Horizon Nouveau/libdrm winsys                      │
└───────────────────────────┬──────────────────────────────────┘
                            │ libnx GPU/VI primitives
                            ▼
┌──────────────────────────────────────────────────────────────┐
│ Horizon / libnx / Tegra X1                                  │
│ - NvMap                                                     │
│ - NvAddressSpace                                            │
│ - NvGpuChannel / GPFIFO                                     │
│ - NvFence / NvMultiFence                                    │
│ - NWindow / NvGraphicBuffer / VI                            │
│ - GM20B                                                     │
└──────────────────────────────────────────────────────────────┘
```

## 3. Existing runtime path

The inherited implementation approximately maps Linux/Nouveau concepts expected by Mesa NVK onto Horizon/libnx primitives:

```text
Mesa NVK
  -> libdrm/Nouveau-style API
  -> winsys/drm_shim.c
  -> nvMap / nvAddressSpace / nvGpuChannel / nvFence
  -> GM20B
```

The WSI path adds:

```text
VkSwapchainKHR
  -> VK_NN_vi_surface
  -> switch WSI backend
  -> NvGraphicBuffer wrapping block-linear NVK image memory
  -> nwindow buffer queue
  -> VI compositor
```

The project should extend these boundaries rather than bypass them unless an experiment proves the abstraction itself is blocking required functionality.

## 4. Target frame-generation runtime path

The first target owns all source images:

```text
VkImage[N-1]      VkImage[N]
      \              /
       \            /
        compute/interpolation
                │
                ▼
        VkImage[generated]
                │
       zero-copy swapchain/WSI
                │
                ▼
              VI
```

The native-interoperability target later becomes:

```text
external producer image
        │
        ├── metadata: format/extent/tiling/kind/offset
        ├── memory: NvMap + GPU mapping/lifetime
        └── sync: producer-complete native fence
        │
        ▼
 external-image bridge
        │
        ▼
 NVK sampled/storage image view
        │
        ▼
 frame-generation compute
        │
        ▼
 generated NVK image
        │
        ├── completion native fence
        ▼
 presentation boundary / VI
```

## 5. Subsystem boundaries

### 5.1 `winsys` / DRM shim

Responsibilities:

- translate NVK's required Nouveau/libdrm-facing operations to Horizon primitives;
- manage GPU VA and BO lifetime;
- map Vulkan synchronization objects to native GPU completion state;
- expose narrowly scoped Switch-specific helpers required by WSI/external-memory integration.

Must not:

- contain frame-generation algorithm logic;
- contain game-specific hooks/addresses;
- silently block every submit merely to simplify synchronization once asynchronous semantics are implemented.

### 5.2 External memory

Required conceptual object:

```text
ExternalMemoryDescriptor
  nvmap identity
  size
  offset
  GPU VA or mapping requirements
  ownership mode
  lifetime token/owner
  memory kind/attributes needed by the mapping layer
```

The exact public/internal API must be defined by an implementation spec. Avoid committing to a C ABI before the first controlled import works.

Ownership modes should be explicit, for example conceptually:

- borrowed: importer must never free underlying allocation;
- shared/ref-counted where the Horizon primitive supports it;
- owned: only for allocations intentionally transferred to NVK.

### 5.3 External images

Memory import and image interpretation are separate layers.

An image descriptor may require:

```text
format
extent
mip/layer information
base offset
row stride/pitch
block-linear layout
GOB/block-height parameters
PTE/memory kind
scanout/display kind where relevant
```

Do not infer image metadata solely from the NvMap allocation size.

### 5.4 Synchronization

The desired steady-state model is dependency-driven:

```text
producer completion
    -> GPU/native fence
    -> NVK waits as dependency
    -> interpolation work
    -> GPU/native fence
    -> VI/consumer waits
```

Avoid:

```text
submit -> CPU wait -> submit -> CPU wait -> queue
```

CPU waits remain acceptable as bring-up fallbacks and diagnostics, but a fallback must be observable so measurements cannot accidentally benchmark the slow path as the intended architecture.

### 5.5 Frame-generation pipeline

The runtime-facing frame-generation API should eventually be algorithm-neutral. Conceptually:

```text
FrameInputs
  previous image
  current image
  optional motion/depth/metadata
  timing information

FrameOutput
  generated image
  completion sync
  timing/quality diagnostics
```

Do not design the final abstraction before the synthetic interpolation harness exposes the real resource/synchronization requirements.

### 5.6 Native-title adapter

A native-title adapter or research probe may discover/acquire producer resources, but it must return them through the reusable external-memory/image/sync interfaces.

Game-specific knowledge must not leak into:

- Mesa patches;
- general winsys BO allocation;
- general WSI;
- generic interpolation code.

## 6. Data ownership model

Every low-level resource must have an explicit owner.

At minimum distinguish:

1. allocation owner;
2. GPU VA mapping owner;
3. Vulkan wrapper/view owner;
4. presentation queue ownership/state;
5. fence/sync object owner.

A wrapper's destruction must not imply destruction of borrowed external storage.

Any cross-context experiment that cannot state who owns each resource before, during, and after the operation is not ready to implement.

## 7. Source-of-truth problem inherited from upstream

Upstream retains multiple forms of Mesa changes:

- durable patch files;
- selected tracked Mesa source overrides/mirrors;
- generated/extracted Mesa trees used by the local build.

A future cleanup milestone may normalize this, but no cleanup should precede reproducibility. Until then, every code spec touching Mesa must identify:

- the build-time authoritative source;
- the durable source that survives clean extraction;
- any tracked mirror that intentionally duplicates the change.

## 8. Observability architecture

Low-level experiments need structured phase timing and error capture.

Prefer counters/timestamps such as:

```text
acquire_start/acquire_end
submit_start/submit_return
producer_wait_start/producer_wait_end
fg_gpu_begin/fg_gpu_end
present_queue_start/present_queue_end
frame_interval
```

Do not add high-frequency SD writes directly to performance-critical paths. Buffer or aggregate instrumentation and emit outside the measured critical path where possible.

## 9. Architectural invariants

1. Real hardware is authoritative for hardware-sensitive behavior.
2. External memory import does not imply external image correctness.
3. External image correctness does not imply synchronization correctness.
4. Synchronization correctness does not imply presentation insertion feasibility.
5. Core runtime remains independent of one commercial title.
6. Core runtime remains independent of one frame-generation algorithm.
7. Fast path must have a correctness fallback where practical.
8. Ownership/lifetime is explicit at every external-resource boundary.
9. Negative results are retained.
10. No subsystem gets marked proven from code inspection alone.
