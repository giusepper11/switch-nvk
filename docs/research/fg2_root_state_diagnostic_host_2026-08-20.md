# FG-2 compute-root diagnostic host record — 2026-08-20

Status: `PROVEN_HOST` for compilation, reconstruction, and bounded trace-format checks. No GPU
visibility, consumption, or stale-boundary claim is made without the planned real-Tegra run.

## Authoritative source and baseline

- Baseline branch `codex/prove-render-compute-image-chain-shared-layout` was fast-forwarded into
  `master` at `1b7ce19`; it contains shared-layout artifact source `d9dfc65` and the archived
  hardware failure record.
- Official Mesa 25.0.7 copies of `nvk_cmd_buffer.c`, `nvk_cmd_buffer.h`, and
  `nvk_cmd_dispatch.c` were fetched from the `mesa-25.0.7` tag and compared byte-for-byte with
  the pre-edit extracted `mesa-25/` files. All three matched upstream exactly.
- None of those files appeared in the pre-edit durable
  `patches/switch-nvk-mesa-25.0.7.patch`. All three therefore had to be added to the durable
  patch. `git ls-files` found no intentional tracked mirror for any of them; `mesa-25/` is the
  ignored extracted build tree and the patch remains the reproducible source of truth.
- A fresh official Mesa 25.0.7 extraction accepted the complete 28-file patch in dry-run and
  actual modes. The three reconstructed diagnostic files then matched the extracted build tree
  byte-for-byte. No file under `pristine-25.0.7/` was modified.

## Direct state and artifact path

The diagnostic follows the existing direct-dispatch path without replacing it:

```text
nvk_CmdPushConstants2KHR
  -> nvk_push_constants(graphics descriptor root)
  -> nvk_push_constants(compute descriptor root)
  -> nvk_descriptor_state_set_root_array(... root.push ...)

nvk_CmdDispatchBase
  -> nvk_flush_compute_state
  -> nvk_cmd_flush_cs_qmd
  -> nvk_cmd_upload_qmd
       -> nvk_cmd_buffer_upload_alloc(sizeof(root), min_cbuf_alignment)
       -> memcpy(root_desc_map, compute root, sizeof(root))
       -> locate NVK_CBUF_TYPE_ROOT_DESC while building qmd_info.cbufs
       -> nak_fill_qmd
       -> nak_get_qmd_cbuf_desc_layout(device, root_cbuf_index)
       -> nvk_cmd_buffer_upload_data(qmd, 0x100 alignment)
  -> SEND_PCAS_A(qmd_addr >> 8)
```

The root-table client-push base is reported from `nvk_root_descriptor_offset(push)`, while the
API byte offset and size are reported separately. The root constant-buffer index is discovered
from the shader's actual `cbuf_map`, not assumed to be slot zero. On GM20B, NAK selects the
pre-Pascal `Qmd0_6` layout through `nak_get_qmd_cbuf_desc_layout`; the trace uses its returned
lower/upper bit ranges and a bounded extractor rather than hard-coded dword positions.

## Frozen observation-only contract

The diagnostic retains the `chain2` shader bytes, seed sequence `(iteration * 37 + 5) & 255`,
16x16 RGBA8 images, descriptor bindings, barriers/layouts, shared pipeline layout, combined
fragment+compute push, command order, one direct dispatch, submit/wait behavior, exact CPU
oracle, and all 64 iterations. The added work is CPU-side logging and post-readback seed
classification only. Source review found no added GPU allocation, GPU command, cache operation,
wait, synchronization primitive, command-buffer/root structure field, shader byte, or
replacement data path.

The upload record explicitly labels the inspected destination as the CPU mapping backing the
returned GPU VA and labels GPU visibility `UNPROVEN`.

## Host validation and immutable artifact

- Diagnostic source revision: `36cf3dd37ae3d25e2e47244472665da458113290`.
- Build container: `switch-nvk-build:latest` image `4e516b1f3576`.
- NVK check: `ninja -C mb src/nouveau/vulkan/libnvk.a` passed. Only unrelated pre-existing
  unused-variable warnings were observed in `nvk_cmd_copy.c` and `nvk_cmd_draw.c` during the
  first rebuild; the final incremental rebuild was clean.
- Diagnostic build: `APP=nvk_render_compute OUTPUT=nvk_render_compute_rootdiag1
  TITLE="NVK Root Diagnostic" VERSION="0.62.0-rootdiag1" ROOT_DIAG_LIMIT=2`.
- Artifact: `nvk_render_compute_rootdiag1.nro`.
- Artifact SHA256: `7e1e12b38eb487a81cd6909e6b351558102708ac2d24e9e4ee13bfac637eeff7`.
- The enabled artifact was rebuilt after committing the exact source revision; the hash remained
  the value above.
- Disabled control build: `ROOT_DIAG_LIMIT=0`, artifact
  `nvk_render_compute_rootdiag_disabled.nro`, SHA256
  `f7fdded7aeca0633a69fde8a278d8b53c8b7855ec258029ea1a24236f0f4cc7c`.
- Static string check: the enabled artifact contains build tag `chain2-rootdiag1`, marker/result
  schemas, and driver push/upload/QMD/dispatch schemas. The disabled artifact contains neither
  the app marker nor app result schemas; without `NVK_ROOT_TRACE`, the driver returns before its
  sink.
- Generated shader header SHA256 before and after:
  `21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14`.

Detailed records are limited to the configured first two records (maximum accepted control is
three). The application still executes and validates all 64 iterations. Any missing marker,
push, upload, QMD, upload-address, dispatch, or result record—or any record/order mismatch—must
invalidate hardware correlation rather than be repaired during analysis.
