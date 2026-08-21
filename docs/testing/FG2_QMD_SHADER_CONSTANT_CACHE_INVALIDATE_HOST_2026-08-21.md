# FG-2 QMD shader-constant-cache invalidation host record — 2026-08-21

Status: `IN_PROGRESS`

## Baseline and authority

- Branch: `master`
- Starting HEAD and fetched `origin/master`: `c147c8adc58d19450803676c177641e1a20cf9e1`
- Starting worktree: clean except for the active untracked OpenSpec change
- Active change: `test-compute-qmd-shader-constant-cache-invalidate`
- Archived predecessor: `openspec/changes/archive/2026-08-21-test-compute-root-address-reuse`
- Predecessor result: `behavior_changed_unresolved`; iterations 1/seed 5 and 2/seed 42 were exact, while iterations 3-64 retained seed-42 pixel `0xf5031a17` and checksum `0x0daf4ac5`; FG-2 remains `BLOCKED`.
- Durable Mesa source: pristine Mesa 25.0.7 plus `patches/switch-nvk-mesa-25.0.7.patch`.
- Disposable build representation: `mesa-25/`; no tracked mirrors of `nak.h`, `qmd.rs`, `hw_runner.rs`, or `nvk_cmd_dispatch.c` exist.

The complete root-flush, QMD-identity, QMD-address, and root-address run records and matching research findings were reviewed. They retain `specific_flush_insufficient`, `identical_payload_non_discriminating`, `specific_qmd_address_change_insufficient`, and `behavior_changed_unresolved` respectively, without promoting FG-2.

## Retained bit-zero precheck

The complete payload lines in both retained root-address raw streams were inspected mechanically. Every available `reference`, `selected`, and `mapped` dword-0 group has dwords 0-7 equal to zero. Therefore dword 7 is `0x00000000`, dword 7 bit 31 is `0`, and QMD bit 255 is `0` in the retained failure-producing payloads.

The generated Mesa 25.0.7 `QMDV00_06_INVALIDATE_SHADER_CONSTANT_CACHE` definition is `MW(255:255)`. This is `PUBLICLY_DOCUMENTED`; the inspected NAK construction path is `SOURCE_CODE_EVIDENCE`. Neither is hardware proof of GPU visibility or consumption.

## Frozen workload and allocation baseline

- Seed formula: `(iteration * 37 + 5) & 255`, 64 iterations.
- Root A/QMD X/QMD Y/root B allocation order: 2,048/256/256/2,048 bytes, 256-byte alignment.
- Expected VAs: root A `0xc7f40000`, QMD X `0xc7f40800`, QMD Y `0xc7f40900`, root B `0xc7f40a00`.
- Footprint/remainder: 4,608/60,928 bytes in the same command-upload BO; resources remain command-buffer-owned through submit/wait/reset and teardown.
- Odd records select A/X; even records select B/Y, giving 63/63 adjacent root and QMD transitions.
- Existing `INVALIDATE_SKED_CACHES`, direct `SEND_PCAS_A`, `SEND_SIGNALING_PCAS_B`, image resources, descriptors, layouts/barriers, reset/rerecord, submission/waits, cache maintenance, readback, and exact CPU oracle remain unchanged.

Frozen hashes before experiment integration were: vertex `dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0`; fragment `14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235`; compute `1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49`; generated header `21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14`.

## Implementation and build evidence

- Typed `nak_qmd_info.invalidate_shader_constant_cache` defaults false at ordinary C designated initializers and is explicitly false in the Rust hardware runner.
- Every typed QMD builder uses its generated named field; NVK obtains the generated layout through `nak_get_qmd_shader_constant_cache_invalidate_layout`.
- Experiment enablement is restricted to exact `MAXWELL_COMPUTE_B`; malformed, contradictory, and historical-selector combinations are rejected.
- Control and variant both use the retained A/B-X/Y schedule. A fixed-stack nondispatched counterfactual is generated only in experiment mode. Full 256-byte XOR must contain exactly generated-layout bit 255 before the selected payload is copied.
- No direct `qmd[7]` mutation, permanent QMD default, other invalidation bit, QMD membar, wait, sleep, cache operation, additional root/QMD slot, or alternate backing was added.

Strict OpenSpec validation passed before implementation. The updated durable patch dry-applied and applied to pristine Mesa 25.0.7, and all four touched reconstructed sources matched `mesa-25/` byte-for-byte.

Build toolchain: `switch-nvk-build:latest`, with Rust nightly `2026-05-25` (`1.98.0-nightly`, commit `423e3d252`) selected to match the retained custom Horizon sysroot. `libnak_rs.a`, `libnvk.a`, and both NROs built successfully. Existing generated-bindings and lifetime warnings were non-fatal; no new C warning remained.

Current identities:

- Durable patch: `34e4f0d155d12a1834246a1f2999088ca009f37cd5f2603824ebf48db15031f9`
- Harness source: `c2fbcbebe5701fb393c4ee85bfd0a5e1fcc6c69b09bf87e5a4a213edad907085`
- Frozen vertex SPIR-V: `a5710f31551964e99298c1f94289c001c9d729a4305c0271129e92966018a3df`
- Frozen fragment SPIR-V: `bcccb45ad89425e16c5dc13d11d3465466eb5417db5381956957d4d1a79d7c20`
- Frozen compute SPIR-V: `d2742455295117324b12268247857e4f4d19095b4eaf7c1e26817aa3832133d3`
- Control NRO: `7009fe4aab55a14c7d10e504e000f035d3ed1ebf9a22baf50f3f679b6c18f983`
- Invalidate NRO: `304d68c163b98250255799f1b0192971f6ba1c87a03d9b4c66f037c193f2bc5f`

Selector tests returned exit 2 for non-binary values, both new selectors, and new-selector combinations with root flush, QMD identity, QMD address, and root address. The exact NVK class-gate helper accepted selector-absent and `MAXWELL_COMPUTE_B` cases and rejected an enabled wrong-class case. Selector-absent and explicit-zero builds produced identical application objects (`921ee452e17fd9ddd61a7cfba38a293183449532516a6ed8f5f1bc5828af7676`) and byte-identical NROs.

The native Mesa `nak` test target passed all five subtests, including the new 64-record generated-QMD test proving explicit false equals the retained default and control/variant XOR equals only bit 255 for both alternating root addresses. The complete-stream validator rejected 12 injected failures covering unexpected field layout/bits, wrong field decode, root decode, source/mapped copy, root/QMD transition, QMD-to-PCAS address, record order, incomplete aggregate, incomplete teardown, ambiguous fault state, and an explicit GPU-fault marker.

The durable patch is now consolidated rather than sequential: one complete dry run and one apply against pristine Mesa 25.0.7 reconstructed all 31 authoritative files byte-for-byte. `bash -n`, patch reconstruction, the NAK/NVK archive builds, both NRO builds, strict OpenSpec validation, and `git diff --check` passed.

Host/build evidence does not prove GPU visibility or consumption. Hardware status remains `IMPLEMENTED_UNPROVEN`; real Tegra was not contacted by this host record.
