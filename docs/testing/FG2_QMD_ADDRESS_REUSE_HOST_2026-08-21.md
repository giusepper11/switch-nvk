# FG-2 QMD address-reuse experiment — host record

Status: `IMPLEMENTED_UNPROVEN`

This record covers source authority, frozen-workload review, implementation, and host validation for
the paired QMD-address control/fresh-address experiment. It does not establish GPU consumption or
any Tegra behavior; those claims require the separately gated hardware runs.

## Source authority and frozen baseline

- Upstream input is the official Mesa 25.0.7 tag/archive. The clean tagged
  `src/nouveau/vulkan/nvk_cmd_dispatch.c` was compared with the extracted tree before edits.
- `patches/switch-nvk-mesa-25.0.7.patch` is the durable Mesa representation and must be updated
  first. `mesa-25/` is the gitignored regenerated build tree consumed by the build. There is no
  tracked mirror of `nvk_cmd_dispatch.c`.
- The tracked experiment-side sources are `winsys/smoke/nvk_render_compute.c` and
  `winsys/build-nro.sh`; generated shader bytes remain in the tracked
  `winsys/smoke/shaders/tri_shaders.h` container and must be hash-compared for the pair.
- `REPRODUCE.md` confirms that a fresh Mesa 25.0.7 extraction plus the complete durable patch
  reconstructs `mesa-25/`; standalone winsys WSI sources are unrelated to this QMD path.

The frozen artifact remains one 64-iteration, 16x16 RGBA8 render-to-sampled-image-to-compute-to-
storage-image chain. Its seed is `(iteration * 37 + 5) & 255`; shaders, two distinct images,
descriptors, shared pipeline layout, explicit layout/barrier sequence, one primary command buffer,
reset/rerecord, direct `SEND_PCAS_A` dispatch, queue submit, queue-idle wait, readback, and exact
256-pixel oracle are unchanged. Retained hardware evidence is the complete 108-line QMD identity
stream: iteration 1 seed 5 gives pixel/checksum `0xfa47d33f`/`0xb7d223e5`; iterations 2-64 retain
`0xfab61a38`/`0xc17a35a5`. Records 1-2 prove exact 256-byte generated/mapped and cross-iteration QMD
equality at reused root/QMD VAs `0xc7f40000`/`0xc7f40800`, with direct-PCAS correlation and no
reported GPU fault. The prior reused-root-only CPU flush was invoked and insufficient; the QMD-only
flush was forbidden as non-discriminating.

## Upload allocator gate

`NVK_CMD_MEM_SIZE` is 64 KiB. `nvk_cmd_buffer_upload_alloc` aligns the current offset and advances it
by the requested size while the allocation fits. After the unchanged 2048-byte root allocation, the
ordinary 256-byte/`0x100` primary QMD remains at offset `0x800`; an immediately following identical
allocation is at offset `0x900`. Both mappings are non-null on success, both VAs are aligned and
distinct, and 61.5 KiB remains in the same upload BO. The second reservation therefore does not
shift the primary QMD address or require another BO.

Upload memory is added to the command buffer's `owned_mem` list. The artifact submits and completes
`vkQueueWaitIdle` before `vkResetCommandBuffer`; reset returns that list to the pool, then rerecording
reacquires pooled memory in the existing lifecycle. Two-slot primary/secondary selection can thus
alternate all 63 consecutive transitions without changing ownership or reset timing. The task-1.4
hard stop is not triggered.

## Implementation and validation

The dedicated build/runtime selectors are `QMD_ADDRESS_CONTROL` / `NVK_QMD_ADDRESS_CONTROL` and
`QMD_ADDRESS_FRESH` / `NVK_QMD_ADDRESS_FRESH`. Only exact value `1` enables either runtime selector.
Malformed, absent, zero, contradictory, cache-experiment, and non-experiment configurations retain
the ordinary one-slot path. Wrapper checks reject malformed values, contradictory selectors, and an
address experiment combined with either root/QMD cache selector.

Both experiment paths allocate the unchanged primary 256-byte, `0x100`-aligned slot first and an
identical secondary slot immediately afterward. Control always selects primary; variant selects
primary for odd records and secondary for even records. It copies the naturally generated QMD once
to the selected mapping. Full 256-byte `memcmp` decisions cover generated/mapped, current/first, and
current/previous equality. Fixed command-buffer-keyed state counts exact copies, equality, all 63
consecutive freshness transitions, and all 64 selected/direct-PCAS matches without loop allocation.
One complete source/mapped payload is emitted in eight fixed records; detailed inherited root/QMD
structure remains limited to records 1-2; correlation/result lines are capped at 64 plus one final
aggregate. Addresses are explicitly allowed to recur after one intervening dispatch.

Source review found no added CPU cache operation, GPU invalidation, barrier, synchronization, wait,
submission change, payload nonce, unrelated QMD field, root-address intervention, shader/resource
change, alternate dispatch method, or ordinary allocation policy. The direct `SEND_PCAS_A` structure
is unchanged; only the selected address operand differs in the variant. The artifact preserves the
same images, descriptors, pipeline layout, barriers/layouts, reset/rerecord, submission/wait,
readback, and independent oracle. The pair does not claim raw GPU-command-byte identity.

Host validation:

- complete durable patch dry-applied and then applied to pristine Mesa 25.0.7; all 27 reconstructed
  files matched the extracted tree byte-for-byte;
- `ninja -C mb src/nouveau/vulkan/libnvk.a` passed in `switch-nvk-build:latest`;
- control and fresh-address NRO cross-builds passed with `ROOT_DIAG_LIMIT=64`;
- malformed, contradictory, and cache-plus-address selector checks returned the required exit 2;
- static sequence review proves control primary reuse and variant odd/even primary/secondary
  alternation, yielding 63/63 consecutive changes when the validated distinct slots are returned;
- `git diff --check` passed;
- generated shader inputs/container retain the prior identity hashes.

| Artifact/input | SHA256 |
|---|---|
| `nvk_render_compute_qmd_address_control.nro` | `97cca217c20071a802e676205495062c54ba3c50c28f49861efd744d5f94d530` |
| `nvk_render_compute_qmd_address_fresh.nro` | `d760e826c38217db0f6cc147e4def272cc4ef18407c21b131670e2167f5436cc` |
| `render_compute.vert` | `dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0` |
| `render_compute.frag` | `14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235` |
| `render_compute.comp` | `1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49` |
| generated `tri_shaders.h` | `21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14` |
| durable Mesa patch | `37b85d38601b7de621ff9dd79ff941e62c9a22e8c82325283d8f8519b559bade` |

Immutable experiment source commit: `c83aca57c680fd6cf53275f96852d53191b7c443`. Both artifacts were
rebuilt after that commit and reproduced the recorded hashes exactly; the shader hashes also remained
identical. Host evidence supports only `IMPLEMENTED_UNPROVEN`; FG-2 remains `BLOCKED`.
