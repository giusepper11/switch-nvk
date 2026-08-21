## 1. Confirm the authoritative path and baseline contract

- [x] 1.1 Starting from the latest `master` tree that contains the archived FG-2 shared-layout result, compare `mesa-25/src/nouveau/vulkan/nvk_cmd_buffer.c`, `nvk_cmd_buffer.h`, and `nvk_cmd_dispatch.c` with clean Mesa 25.0.7 and the durable `patches/switch-nvk-mesa-25.0.7.patch`; identify and preserve unrelated ignored-tree edits, confirm which files must be added to the durable patch, and record whether any intentional tracked mirror exists.
- [x] 1.2 Trace and document the direct artifact path from `nvk_CmdPushConstants2KHR` through compute descriptor/root state, `nvk_cmd_upload_qmd`, NAK QMD construction, QMD upload, and `nvk_CmdDispatchBase`/`SEND_PCAS_A`; confirm the root-table push offset, root cbuf index discovery, and device-specific QMD address-field layout used on GM20B.
- [x] 1.3 Freeze the observation-only baseline: retain the existing shader bytes, seed sequence, images, descriptors, barriers, command order, one direct dispatch, submission/wait behavior, CPU oracle, and 64-iteration validation; record that the diagnostic adds no GPU allocation/command, cache operation, wait, structure-layout field, or replacement data path.

## 2. Add iteration-correlated push/root diagnostics

- [x] 2.1 Add a disabled-by-default bounded diagnostic control and a single-line `FG2_ROOT_DIAG` sink using the existing Switch shim logging convention, with ordinary builds/runs emitting no diagnostic records and captured records limited to iterations 1-2 or optionally 1-3.
- [x] 2.2 Add marker and result records to `winsys/smoke/nvk_render_compute.c` for record sequence, command-buffer handle, iteration, expected seed, exact output/checksum, and uniquely decodable observed behavior seed; do not change shaders, GPU commands, synchronization, dispatch, or validation.
- [x] 2.3 Instrument immediately after `nvk_CmdPushConstants2KHR` updates both consumers to capture the input bytes, stage mask/range, graphics descriptor/root identity and seed, and compute descriptor/root identity and seed; validate correlation and report unavailable ranges without unsafe reads.

## 3. Add compute upload and QMD/dispatch diagnostics

- [x] 3.1 Instrument `nvk_cmd_upload_qmd` to capture the compute root source seed before copy and the CPU-mapped uploaded seed after copy, together with source/descriptor/mapping identities, allocation size/alignment, GPU virtual address, and address reuse versus the prior captured record.
- [x] 3.2 Identify the `NVK_CBUF_TYPE_ROOT_DESC` slot placed in `qmd_info`, then decode the generated QMD's device-specific lower/upper root-address fields with `nak_get_qmd_cbuf_desc_layout`; log only the intersecting QMD dwords, decoded address, current upload address/size, and match result.
- [x] 3.3 Correlate `nvk_cmd_flush_cs_qmd` and `nvk_CmdDispatchBase` so the trace reports the uploaded QMD GPU address, the QMD address/PCAS value actually selected for direct dispatch, and its associated root address without changing the emitted command stream.
- [x] 3.4 Review the implementation for bounds safety, format stability, disabled-path behavior, counter/order mismatches, and an explicit distinction between CPU-mapped bytes backing a GPU VA and bytes proven visible to the GPU.

## 4. Build and host-validate an immutable diagnostic artifact

- [x] 4.1 Update/regenerate the durable Mesa 25.0.7 patch and the extracted build tree (plus only any confirmed intentional mirror), then verify a clean reconstruction applies the patch and contains the exact diagnostics without modifying pristine upstream references.
- [x] 4.2 Build/package `nvk_render_compute` with a distinct diagnostic build/version tag, run applicable compile/static checks, and verify the disabled trace path and bounded enabled trace format without claiming a hardware boundary.
- [x] 4.3 Review the source and built shader artifacts to prove there is no added GPU command, allocation, cache operation, synchronization, command-buffer/root layout change, or shader-byte change; commit the exact diagnostic source revision and record the artifact SHA256 and diagnostic flags.

## 5. Run the observation-only real-hardware experiment

- [ ] 5.1 Run the immutable artifact for the unchanged 64 iterations on real Switch/Tegra hardware with detailed capture limited to iteration 1 (seed 5), iteration 2 (seed 42), and optionally iteration 3; do not substitute an emulator and do not introduce a flush, invalidation, wait, or semantic variant.
- [ ] 5.2 Retain and inspect the complete application/driver streams before filtering, record missing configured log files, and capture supported `ERRNOTIF`/`ERRINFO` data after any timeout or GPU fault.
- [ ] 5.3 Record commit, artifact SHA256/version, hardware/model, diagnostic flags, inspected iterations, all root/upload/QMD/dispatch values, exact observed output, address reuse, observer-effect assessment, GPU fault/error state, and raw-log references according to `docs/testing/HARDWARE_EVIDENCE.md`.

## 6. Correlate the first stale boundary and update research state

- [ ] 6.1 Build the iteration 1 versus iteration 2 table correlating expected seed, graphics CPU root, compute CPU root, upload source, uploaded CPU mapping, upload GPU VA, QMD root VA, dispatched QMD VA, and observed behavior seed; reject the correlation if marker/order/identity checks do not line up.
- [ ] 6.2 Compare the diagnostic run with both retained FG-2 failure signatures and classify exactly one justified result: first stale boundary identified, unresolved after current CPU/QMD state before GPU consumption, observer effect detected, or specified ambiguous interval; stop without implementing a fix.
- [ ] 6.3 Add the detailed evidence-backed finding under `docs/research/`, retain the full hardware record/raw logs under `docs/testing/`, keep the shared-layout hypothesis `REJECTED`, and keep FG-2 `BLOCKED` unless the original full render-compute-image-chain acceptance contract independently passes.
- [ ] 6.4 Update `docs/status/CAPABILITY_MATRIX.md` only if real hardware establishes a durable fact, add an ADR only for a persistent architectural decision, and name one smallest follow-up OpenSpec change targeted solely at the identified or unresolved boundary.
