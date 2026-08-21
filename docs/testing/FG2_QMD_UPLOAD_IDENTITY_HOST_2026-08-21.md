# FG-2 QMD upload identity and conditional cache experiment — host record

Status: `IMPLEMENTED_UNPROVEN`

This record covers source reconstruction, static review, and cross-build evidence for the bounded
QMD identity control and conditionally eligible `NVK_QMD_UPLOAD_CACHE_FLUSH=1` artifact. It does not
establish QMD identity, address reuse, CPU/GPU visibility, or cache behavior on Tegra.

## Source and frozen baseline

- Immutable experiment source commit: `6828d6e23d5562801dbc0f6e1060e0b0b9b2c7a0` (base commit
  `b07b62c7bf868716d6db5af067a5f2e4bcd85817`). Both recorded artifacts were rebuilt from this exact
  revision after the commit and reproduced the hashes below.
- Upstream input: Mesa 25.0.7 from `https://archive.mesa3d.org/mesa-25.0.7.tar.xz`.
- Durable Mesa source: `patches/switch-nvk-mesa-25.0.7.patch`; extracted build tree:
  `mesa-25/`; intentionally tracked consumer source: `winsys/smoke/nvk_render_compute.c`; generated
  shader container: `winsys/smoke/shaders/tri_shaders.h`. There is no tracked mirror of
  `nvk_cmd_dispatch.c`.
- The complete patch dry-ran against pristine Mesa 25.0.7. Applying it reconstructed
  `nvk_cmd_dispatch.c` byte-for-byte; both reconstructed and extracted files have SHA256
  `5fd5d87ef5a2328da6e62b945a136342699315483e7b9aaaa81dd6f81d8d3a8d`.
- Durable patch SHA256: `9e6ce9d55bae5ec26a982a8ca51f75acbf723c72e1bc2a3346c1947357bc3df5`.

The retained smoke contract is unchanged: distinct 16x16 RGBA8 source/destination images, shared
fragment/compute pipeline layout, `(iteration * 37 + 5) & 255` seeds, explicit barriers, reset and
rerecord of one command buffer, direct QMD dispatch, queue submit plus queue-idle wait, readback, and
the exact 64-iteration oracle. Retained hardware evidence establishes iteration 1 seed 5 as exact and
iterations 2-64 as the stable seed-5 stale signature. The existing root-upload selector remains
independent and disabled in both new artifacts.

Generated shader-source hashes are identical for both artifacts:

| Input | SHA256 |
|---|---|
| `render_compute.vert` | `dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0` |
| `render_compute.frag` | `14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235` |
| `render_compute.comp` | `1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49` |
| generated `tri_shaders.h` | `21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14` |

## Implementation and false-positive controls

The direct QMD upload helper was expanded locally into its existing operations: one
`nvk_cmd_buffer_upload_alloc` for 256 bytes at `0x100` alignment followed by the same 256-byte
`memcpy`. Both artifacts execute this same allocation/copy path. The bounded diagnostic computes
64-bit FNV-1a identities over all source and mapped bytes, while full `memcmp` results—not hashes—
control source/map and previous/current equality decisions. State is fixed-size and keyed to command-
buffer identity; it performs no dynamic allocation or whole-QMD logging.

Only exact Switch values `NVK_QMD_UPLOAD_IDENTITY=1` and `NVK_QMD_UPLOAD_CACHE_FLUSH=1` enable their
respective selectors. Absent, zero, malformed, and non-Switch cases do not invoke a cache operation.
The build wrapper rejected malformed selectors and cache-without-identity with exit 2. Static review
confirmed explicit `no_prior`, `fresh_address`, `identical_payload`, copy-failure, order-failure, and
`eligible` classifications.

The sole new semantic operation is guarded by all eligibility checks and `__SWITCH__`, occurs after
copy and exact identity validation, and requests
`armDCacheFlush(qmd_map, sizeof(qmd))` before unchanged direct dispatch. The libnx declaration is
`void armDCacheFlush(void *addr, size_t size)`; its documented implementation may round start/end to
cache-line boundaries. Logs therefore claim only invocation over the requested 256-byte mapping and
explicitly retain `gpu_visible=UNPROVEN`.

Source review confirms no QMD nonce or field modification, fresh-allocation policy, root/upload-arena
flush, descriptor/push flush, GPU invalidation, GPU command, barrier, wait, shader, submission,
readback, or oracle change was added.

## Build and artifact evidence

- Driver command in `switch-nvk-build:latest`:
  `ninja -C mb src/nouveau/vulkan/libnvk.a` — pass.
- Control build: `APP=nvk_render_compute OUTPUT=nvk_render_compute_qmdidentity_control
  TITLE="NVK QMD Identity" VERSION="0.63.0-qmdidentity-control" ROOT_DIAG_LIMIT=2
  QMD_UPLOAD_IDENTITY=1 QMD_UPLOAD_CACHE_FLUSH=0 bash winsys/build-nro.sh` — pass.
- Enabled build: `APP=nvk_render_compute OUTPUT=nvk_render_compute_qmdcache_enabled
  TITLE="NVK QMD Cache" VERSION="0.63.0-qmdcache-enabled" ROOT_DIAG_LIMIT=2
  QMD_UPLOAD_IDENTITY=1 QMD_UPLOAD_CACHE_FLUSH=1 bash winsys/build-nro.sh` — pass.
- `git diff --check` — pass.

| Artifact | Identity selector | Cache selector | SHA256 |
|---|---:|---:|---|
| `nvk_render_compute_qmdidentity_control.nro` | 1 | 0 | `e78804a5692d9c5a699fddc1051470b7c177b99486c5b86f968ae95ab59b8c60` |
| `nvk_render_compute_qmdcache_enabled.nro` | 1 | 1 | `4c538c22cadb1a93052d9c5a307e62a54edf6f2d21af6fe0a1c24788f3db048d` |

The artifacts differ only in build/version reporting, cache selector activation, and the runtime
branch that can invoke the QMD-only CPU cache operation after every eligibility prerequisite passes.
Their resources, generated shaders, QMD generation/copy, GPU commands, synchronization, direct
dispatch, submission/wait behavior, readback, oracle, and iteration count are the same.

## Remaining gate

All hardware-dependent claims remain `IMPLEMENTED_UNPROVEN`. Run and inspect the complete identity
control first. An identical reused payload forbids the enabled run; only a valid, naturally changed
reused payload authorizes it. FG-2 remains `BLOCKED` regardless of host success.
