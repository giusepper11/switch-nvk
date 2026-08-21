# FG-2 compute-root address-reuse-distance host record — 2026-08-21

Status: `IMPLEMENTED_UNPROVEN` (`hardware-ready`; no device contacted)

This record covers source authority, causal isolation, implementation, and host validation for the
paired root-address-reuse-distance experiment. It is not hardware evidence. CPU-mapped equality does
not prove GPU visibility or consumption, and no source inspection below is `PROVEN_HW`.

## Baseline and retained evidence

- Branch: `master`.
- Starting HEAD and freshly fetched `origin/master`:
  `8e2b2882c31ce5be163fb3baa41a1aa6261969eb`.
- Starting worktree: unchanged except for the active untracked OpenSpec change
  `test-compute-root-address-reuse-distance`.
- All nine retained raw streams for the root-flush, QMD-identity, QMD-address, root-address, and QMD
  shader-constant-cache experiments match their recorded SHA256 values and line counts. A complete
  UTF-8 read found only the expected NVK non-conformance warning and no timeout, `ERRNOTIF`,
  `ERRINFO`, GPU-fault, assertion-failure, or incomplete-stream marker. The current constant-cache
  validator accepts both 64-record streams and rejects all 12 retained invalid mutations.
- The retained result chain is `specific_flush_insufficient`,
  `identical_payload_non_discriminating`, `specific_qmd_address_change_insufficient`,
  `behavior_changed_unresolved`, then
  `specific_qmd_shader_constant_cache_invalidate_insufficient`. Root reuse distance is the next
  separate discriminator. FG-2 remains `BLOCKED`; FG-3 and FG-4 are unchanged.

## Public research boundary

Public GM20B/Maxwell Compute B QMD material distinguishes QMD/SKED state and the named
`INVALIDATE_SHADER_CONSTANT_CACHE` field. The generated QMD v0.6 definition and the inspected NAK
builder path are respectively `PUBLICLY_DOCUMENTED` and `SOURCE_CODE_EVIDENCE`. They do not identify
the relevant root-state key, hidden launch-state lifetime, VA-versus-physical identity, replacement
policy, associativity, or TLB behavior. This change therefore tests only a behavioral first-revisit
boundary and does not name an undocumented cache architecture or convert public/source evidence into
`PROVEN_HW`.

## Source authority and mirrors

- The durable Mesa source of truth is the official Mesa 25.0.7 archive plus
  `patches/switch-nvk-mesa-25.0.7.patch`; gitignored `mesa-25/` is the disposable reconstructed build
  tree. The official archive passed `xz -t`; the complete patch dry-applied and applied in a fresh
  temporary tree; and all 32 patched/generated outputs are byte-identical to `mesa-25/`. A complete
  reverse dry-run against `mesa-25/` also succeeds. No unexplained source divergence was found.
- The tracked verbatim mirrors `winsys/mesa-edits/src/nouveau/vulkan/nvk_image.c`,
  `winsys/mesa-edits/src/nouveau/vulkan/nvk_queue.c`, and
  `winsys/mesa-edits/src/vulkan/wsi/wsi_common.c` are byte-identical to their reconstructed build-tree
  copies. `winsys/wsi/wsi_common_switch.c` is the intentionally older standalone copy documented in
  `REPRODUCE.md`; it is not build-authoritative and is not touched by this experiment.
- There is no tracked mirror of `nvk_cmd_dispatch.c`. Tracked experiment inputs are
  `winsys/smoke/nvk_render_compute.c`, its three shader sources and generated
  `winsys/smoke/shaders/tri_shaders.h`, `winsys/build-nro.sh`, the paired build recipe, host tests,
  validators, and evidence templates/records.

## Frozen workload before implementation

- Iterations and seed: 64 records; `(zero_based_iteration * 37 + 5) & 255`. The decisive seeds are
  5, 42, and 79.
- Exact oracle pixel/checksum pairs: seed 5 `0xfa47d33f` / `0xb7d223e5`; seed 42
  `0xf5031a17` / `0x0daf4ac5`; seed 79 `0xf0bf610f` / `0x1fcdf2e5`.
- Root table: `struct nvk_root_descriptor_table`, 2,048 bytes, aligned to the existing GM20B minimum
  cbuf alignment. The complete current `desc->root` bytes are copied; the only per-record semantic
  change is the existing four-byte push seed at offset zero. Draw/compute fields, descriptor-set
  addresses, dynamic buffers, and padding remain current-source controlled.
- Resources and execution: two distinct 16x16 `VK_FORMAT_R8G8B8A8_UNORM` images; unchanged sampled
  and storage descriptors, nearest/clamp sampler, shared graphics/compute pipeline layout, explicit
  transfer/color/sample/compute/copy barriers and layouts, one direct compute dispatch of 2x2x1,
  command-buffer reset/rerecord per iteration, one queue submission, existing `vkQueueWaitIdle`, CPU
  readback cache operation, 256-pixel exact oracle, and current teardown.
- GPU method and QMD behavior: retained `INVALIDATE_SKED_CACHES`, direct `SEND_PCAS_A`/`PCAS`,
  `SEND_SIGNALING_PCAS_B`, normal QMD generation/copy, and ordinary cache/membar values including
  `INVALIDATE_SHADER_CONSTANT_CACHE=FALSE`. No cache, membar, timing, synchronization, backing, or
  allocator intervention is permitted.

Pre-implementation SHA256 identities:

| Input | SHA256 |
|---|---|
| `winsys/smoke/nvk_render_compute.c` | `c2fbcbebe5701fb393c4ee85bfd0a5e1fcc6c69b09bf87e5a4a213edad907085` |
| `render_compute.vert` | `dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0` |
| `render_compute.frag` | `14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235` |
| `render_compute.comp` | `1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49` |
| generated `tri_shaders.h` | `21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14` |
| durable Mesa patch | `34e4f0d155d12a1834246a1f2999088ca009f37cd5f2603824ebf48db15031f9` |
| reconstructed `nvk_cmd_dispatch.c` | `d5054831aecfcf2423777e5ee2bdf6dfa36c7c99b8b7ad4c617661bfb5aef2b2` |

## Command-upload ownership pre-audit

`NVK_CMD_MEM_SIZE` is 64 KiB. `nvk_cmd_buffer_upload_alloc` aligns within the current upload object,
returns the mapped CPU pointer and measured GPU VA, and allocates a new command-memory object only when
the current object lacks capacity. Command memory is owned by `nvk_cmd_buffer::owned_mem`; reset or
destruction returns it to the command pool, and command-pool trim/destruction releases the underlying
mapped memory. The harness waits for queue completion before reset, so the retained lifetime boundary
is unchanged.

The retained allocation order is root A (2,048 bytes), QMD X (256), QMD Y (256), root B (2,048), at
offsets `0x000`, `0x800`, `0x900`, and `0xa00`, for a 4,608-byte footprint. The proposed common order
appends QMD Z (256), QMD W (256), and root C (2,048), nominally ending at `0x1c00` (7,168 bytes) with
58,368 bytes remaining. These appended offsets and every GPU VA/backing identity remain expectations
until measured by the implemented host/runtime correlation; logic and evidence must not derive them
arithmetically. A second command-upload BO/NvMap, capacity rollover, later-consumer displacement, or
ownership/lifetime discrepancy is a hard stop before hardware.

## Implemented source and causal review

The driver now reserves the same seven candidates in both arms in this exact order: root A, QMD X,
QMD Y, root B, QMD Z, QMD W, root C. Runtime validation requires actual offsets `0x000`, `0x800`,
`0x900`, `0xa00`, `0x1200`, `0x1300`, and `0x1400`, a 7,168-byte footprint, 58,368 bytes remaining,
seven distinct aligned GPU VAs, and one command-upload backing identity. These are implementation
acceptance checks, not claims that hardware has yet emitted them. Root A/B/C receive complete copies
of the current root source in both arms. Both arms use QMD X/Y/Z/W; control alone selects roots
A/B/A/B and variant alone selects A/B/C/A/B/C. Full QMD comparison permits only generated root-address
fields to differ, requires `INVALIDATE_SHADER_CONSTANT_CACHE=FALSE`, decodes the selected root VA, and
correlates the selected mapped QMD VA to the unchanged direct PCAS dispatch.

The semantic verifier found no arm-dependent change outside the root-ring selection and mechanically
required QMD root encoding. Shader sources, generated shader header, pipeline/resources,
layouts/barriers, cache/membar fields and methods, command-buffer lifecycle, submission/waits, CPU
readback operation, synchronization, timing, backing policy, oracle, and teardown remain unchanged.
Absent and explicit-zero selectors produced byte-identical ordinary artifacts. Every new-plus-old,
both-new, malformed, and contradictory selector combination failed before device work as required.

## Final host/build validation

- Immutable experiment-source revision: `a71baed9d432b3f168d26bf4a3edd4679b8461b7`. The archive and both
  NROs were rebuilt after this commit, all recorded hashes reproduced, and the final source/artifact
  semantic review passed before device contact.
- Container: `switch-nvk-build`, image
  `sha256:4e516b1f35766dbb6c6061e606613ce5d19d8c393e03dee750675cbd80190832`.
- Toolchain: Meson 1.12.0, Ninja 1.11.1, devkitA64 GCC 15.2.0, glslang 12.0.0.
- Archive and pair build:
  `docker run --rm -v /home/gro/projects/switch-nvk:/work -w /work switch-nvk-build bash -lc
  'export NATIVE_PREFIX=/work/native-prefix; export PATH=/work/native-prefix/bin:$PATH;
  ninja -C mb src/nouveau/vulkan/libnvk.a && bash winsys/build-root-address-reuse-distance-pair.sh'`.
- Patch reconstruction: official Mesa 25.0.7 archive passed `xz -t`; the complete durable patch
  dry-applied and applied; all 32 resulting files compare byte-for-byte with `mesa-25/`.
- Schedule model: 64 records pass with control root first revisit record/distance 3/2, variant 4/3,
  and common QMD 5/4. QMD first reuse is strictly later than both decisive root revisits.
- Validator: both valid schedules accepted and 16 independent invalid mutations rejected. The prior
  QMD constant-cache validator still accepts both retained 64-record streams and rejects all 12
  retained invalid mutations.
- Compile/static checks: strict C11 selector and prior compute-class tests pass; all changed shell
  files pass `bash -n`; source/artifact semantic review, complete root/QMD/PCAS correlation checks,
  allocation/capacity checks, observer symmetry, aggregate checks, and teardown checks pass.
- Ordinary-path check: absent-selector and explicit-zero artifacts compare byte-identically at
  `6107fa873bc235f43821e32efad8bf53808f5d36d03f69bbba6dc81c71a9bb67`.

Final SHA256 identities:

| Input or artifact | SHA256 |
|---|---|
| durable Mesa patch | `36322b38cb64da99ee6ccb6e97b3410f7edc3d6ee3bcecd8e3a9aea67c7b76e1` |
| implemented `nvk_cmd_dispatch.c` | `4f57cb6884a86229222f1599d68b2636ff2fe796440ad6cdcee1243c697f56bb` |
| root-reuse validation header | `f84982ca9da76d8c353a5feb8a644102cbcd28c30aadbda47ecfd214afa18646` |
| implemented harness | `74fa6ef0960b68a311c69af23b680b90b02add68f0af49c1059f16acfb10c9c5` |
| `render_compute.vert` | `dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0` |
| `render_compute.frag` | `14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235` |
| `render_compute.comp` | `1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49` |
| generated `tri_shaders.h` | `21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14` |
| control NRO | `6b7ccfa260868ee45e05fa53923467527e2f47798d375c8497e9fe6e421f1569` |
| variant NRO | `e1be1a3e0372cd4aa3376b158ba45218e256d33526e2cf3ec5108503b96ac549` |

This is host/build evidence only. No Switch, Tegra GPU, emulator, or other device was contacted during
this implementation session. GPU consumption, backing identity, reuse behavior, and the experiment's
causal hypothesis remain unproven until the immutable-revision control gate and authorized real-
hardware protocol are completed.
