## Why

FG-1 is the first hardware gate for the frame-generation runtime, but the current capability matrix only records compute and storage-image behavior as `IMPLEMENTED_UNPROVEN`. Existing graphics, WSI, and upstream NVK evidence cannot establish that this Horizon/NVK winsys executes the exact compute/image path required by a future interpolation pipeline. A deterministic, low-level proof is needed now so later synchronization and frame-generation work is not built on an unverified assumption.

## What Changes

- Add a dedicated compute/storage-image hardware smoke capability for the Switch NVK winsys.
- Define a minimal path that creates a compute pipeline, executes a NAK compute shader, reads and writes storage buffers, samples an image, writes a storage image, and uses explicit image layouts and barriers.
- Require deterministic output and CPU-visible readback validation over repeated iterations.
- Require unambiguous reporting that the intended GPU compute/image path executed, rather than a CPU or staging fallback.
- Require complete diagnostic evidence, including GPU fault/error-notifier inspection and the configuration needed to reproduce the run.
- Keep the result scoped to FG-1 primitives; do not claim asynchronous submission, external-memory interoperability, NVN interoperability, or frame-generation support.

## Capabilities

### New Capabilities

- `compute-storage-images`: Deterministic compute-pipeline, storage-buffer, sampled-image, storage-image, barrier/layout, readback, and fault-observability behavior required by FG-1.

### Modified Capabilities

<!-- No existing OpenSpec capability requirements are being modified. -->

## Impact

- Expected implementation/test area: `winsys/smoke/nvk_compute.c` or the repository-equivalent smoke-test location, plus the build/package wiring needed to produce the test artifact.
- Affected runtime layers: Horizon/NVK device setup, NAK compute pipeline creation and dispatch, Vulkan image/buffer resource usage, synchronization/layout transitions, and CPU-visible validation.
- Affected project state: the FG-1 rows in `docs/status/CAPABILITY_MATRIX.md` and `MILESTONES.md` can change only after the required hardware evidence exists; this planning change itself makes no capability claim.
- No external dependencies, proprietary artifacts, native-title integration, or changes to the frame-generation algorithm are in scope.
