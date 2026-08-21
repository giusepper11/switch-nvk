# FG-2 reused root-upload CPU cache-flush host record

Status: `IMPLEMENTED_UNPROVEN`

This record covers source reconstruction, static review, and cross-build evidence for the controlled
`NVK_ROOT_UPLOAD_CACHE_FLUSH=1` experiment. It does not establish CPU/GPU cache behavior on Tegra.

## Source and reconstruction

- Immutable experiment source commit: `c79700438f42b588c6a1a9b7dcff296f8235349e`
  (base commit `6004cdc0c9e2cdd2d6b9718093efbb0eecc0bf10`).
- Upstream input: Mesa 25.0.7 archive from `https://archive.mesa3d.org/mesa-25.0.7.tar.xz`.
- The build-authoritative input is the ignored `mesa-25/` extraction; the durable reproducible input is
  `patches/switch-nvk-mesa-25.0.7.patch`.
- No tracked mirror exists for `nvk_cmd_dispatch.c` or `nvk_cmd_buffer.c`. The tracked
  `winsys/wsi/wsi_common_switch.c` is not authoritative for this change.
- The existing ignored WSI extraction differs from the durable patch in unrelated, previously known
  swapchain ownership/triple-buffer changes. It was preserved and was not folded into this experiment.
- The complete durable patch dry-ran and applied without rejects to a clean Mesa 25.0.7 extraction.
- Reconstructed and extracted `nvk_cmd_dispatch.c` matched byte-for-byte with SHA256
  `e2ea2f43da353e69c946efa01ae6294b9ae3a6c09d272905f7ce3061cff43c45`.
- Durable patch SHA256 after regeneration:
  `a98200af3f7cf8e480fb14cd580dbd9cd313d0f7d87c55b65d90c78e456b980f`.

## Frozen control review

`winsys/smoke/nvk_render_compute.c` retains the 16x16 RGBA8 source/destination images, shared
fragment/compute layout, `(iteration * 37 + 5) & 255` seed sequence, explicit image barriers,
same-command-buffer reset/rerecord flow, one direct dispatch, submit plus queue-idle wait, CPU readback,
and exact 64-iteration oracle. The retained stale signature remains seed 5 output when later iterations
are expected to use new seeds. The experiment adds only startup/path reporting and selector activation;
the command loop, resources, shaders, QMD construction/upload, synchronization, dispatch, and oracle are
unchanged.

Generated shader identities used by both artifacts:

| Shader | SHA256 |
|---|---|
| `render_compute.vert.spv` | `a5710f31551964e99298c1f94289c001c9d729a4305c0271129e92966018a3df` |
| `render_compute.frag.spv` | `bcccb45ad89425e16c5dc13d11d3465466eb5417db5381956957d4d1a79d7c20` |
| `render_compute.comp.spv` | `d2742455295117324b12268247857e4f4d19095b4eaf7c1e26817aa3832133d3` |

## Cache primitive and selector review

The pinned libnx header `/opt/devkitpro/libnx/include/switch/arm/cache.h` declares
`void armDCacheFlush(void *addr, size_t size)`. Its contract defines flush as clean plus invalidate and
states that the start and end are rounded to cache-line boundaries read from `CTR_EL0`. The experiment
therefore records the requested pointer and `sizeof(*root)` range and reports invocation only; it does
not fabricate a return value or claim byte-granular hardware action.

Only the exact Switch environment value `NVK_ROOT_UPLOAD_CACHE_FLUSH=1` enables the cached selector.
Absent, `0`, and malformed values do not invoke the primitive. Non-Switch compilation contains no cache
call. The selector is independent of `NVK_ROOT_TRACE`; trace remains bounded while experiment state
continues across resets of the same command buffer. A command-buffer identity change clears the prior
address, and a bounded record-order mismatch prevents invocation for that correlation.

## Build and static validation

- NVK cross-build command: `ninja -C mb src/nouveau/vulkan/libnvk.a` in
  `switch-nvk-build:latest`; result: pass.
- The same app source compiled successfully with `FG2_ROOT_UPLOAD_CACHE_FLUSH=0` and `=1`.
- The build wrapper rejected value `2` with exit code 2. Source review confirms strict runtime parsing
  through exact string comparison, covering absent, `0`, malformed, and `1` cases.
- Static review confirms first upload `no_prior`, equal preceding VA `reused`, different preceding VA
  `fresh_address`, command-buffer identity reset, and invalid bounded order skip paths.
- The only `armDCacheFlush` added to Mesa is guarded by `__SWITCH__`, occurs after root copy/mapped-byte
  inspection, targets exactly `root_desc_map, sizeof(*root)`, and precedes existing QMD construction.
  No QMD/upload-BO flush, GPU command, invalidation, barrier, wait, or allocation-policy change was added.
- `git diff --check`: pass.

Paired artifacts built from the same working source:

| Artifact | Build control | Version | SHA256 |
|---|---:|---|---|
| `nvk_render_compute_rootflush_control.nro` | `0` | `0.62.0-rootflush-control` | `1656f810a07b52a77e5848da3c549842fa591f0411fb2e495bf4cacc3235a8ba` |
| `nvk_render_compute_rootflush_enabled.nro` | `1` | `0.62.0-rootflush-enabled` | `5c2302d73c597d996aada097f9a7407d8d35b2af75788db83b4d39c7f882ca39` |

Both artifacts were rebuilt from clean experiment commit
`c79700438f42b588c6a1a9b7dcff296f8235349e` after it was created, and both SHA256 values reproduced
exactly before any hardware execution.

The artifact hashes differ as required by their build/version metadata, startup selector/reporting, and
enabled `setenv` call. Both link the same NVK archive and embed the same generated shaders. Source review
confirms that their GPU command path, resources, synchronization, QMD behavior, direct dispatch,
readback oracle, and 64-iteration contract are identical; the driver-side semantic branch is solely the
targeted reused-root cache call.

## Remaining hardware-only claims

Real Tegra execution is required to confirm address reuse, actual invocation on iteration 2, QMD/dispatch
correlation, output signature, observer effects, and GPU fault state. Until a valid paired hardware run is
retained and inspected in full, the cache-visibility hypothesis is unresolved and FG-2 remains `BLOCKED`.
