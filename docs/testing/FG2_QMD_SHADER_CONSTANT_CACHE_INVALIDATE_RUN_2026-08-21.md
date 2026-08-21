# FG-2 QMD shader-constant-cache invalidation hardware run — 2026-08-21

```text
Date: 2026-08-21
Repository immutable experiment commit: 57d8075b386a1c1f24b6d38a7d5645d43403ffed
Spec/milestone: test-compute-qmd-shader-constant-cache-invalidate / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1 console at 192.168.15.13
Firmware/Atmosphere/libnx context: firmware/Atmosphere versions not captured; devkitA64/libnx 4.12.0 build environment
Build type/toolchain: devkitA64 full Application NRO; switch-nvk-build:latest; Rust nightly 2026-05-25 (1.98.0-nightly, 423e3d252)
Control artifact/version/SHA256: chain2-qmd-constant-cache-control / 0.62.0-qmd-constant-cache-control / 7009fe4aab55a14c7d10e504e000f035d3ed1ebf9a22baf50f3f679b6c18f983
Variant artifact/version/SHA256: chain2-qmd-constant-cache-invalidate / 0.62.0-qmd-constant-cache-invalidate / 304d68c163b98250255799f1b0192971f6ba1c87a03d9b4c66f037c193f2bc5f
Durable patch SHA256: 34e4f0d155d12a1834246a1f2999088ca009f37cd5f2603824ebf48db15031f9
Vertex/fragment/compute generated-header SHA256: 21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14
Selectors: control NVK_QMD_SHADER_CONSTANT_CACHE_CONTROL=1; variant NVK_QMD_SHADER_CONSTANT_CACHE_INVALIDATE=1; every historical intervention selector disabled
Expected VAs/allocation: root A 0xc7f40000; QMD X 0xc7f40800; QMD Y 0xc7f40900; root B 0xc7f40a00; 4,608-byte footprint; 60,928 bytes remaining; one command-upload BO/NvMap
Control raw complete log/SHA256: docs/testing/raw/fg2_qmd_constant_cache_control_2026-08-21.log / ea69867a3f2a4355ef2955d8113bcdd22aa0010888d15675f8c33204f1e93b28
Control full-log review: complete 612-line/178,631-byte stream read before filtering; expected NVK non-conformance warning only; no timeout, error notifier/error-info event, GPU fault, incomplete aggregate, or incomplete teardown
Control authorization: yes; iterations 1-2 exact, iterations 3-64 retained seed-42 output, and every causal prerequisite passed
Variant raw complete log/SHA256: docs/testing/raw/fg2_qmd_constant_cache_invalidate_2026-08-21.log / 031a2b12a2b638aa2bc63ef3c90676a024e68104e21746a9112b64b2384b1fb2
Variant full-log review: complete 612-line/178,756-byte stream read before filtering; expected NVK non-conformance warning only; no timeout, error notifier/error-info event, GPU fault, incomplete aggregate, or incomplete teardown
Classification: specific_qmd_shader_constant_cache_invalidate_insufficient
FG-2 status: BLOCKED
FG-3/FG-4 status: unchanged/out of scope
Production promotion: forbidden; selectors remain opt-in and ordinary typed field value remains false
```

## Paired causal table

| Path | Bit 255 | Records and schedule | Copy/decode/dispatch evidence | Output/oracle | Fault and teardown |
|---|---:|---|---|---|---|
| control | 0 for 64/64 | 64 records; root A/B 63/63; QMD X/Y 63/63; exact retained VAs and footprint | root copies 64/64 each; QMD copies, one-bit counterfactual comparisons, field/root decodes, and direct-`PCAS` matches 64/64; order complete | iterations 1-2 exact; iterations 3-64 seed-42 `0xf5031a17` / `0x0daf4ac5`; 2/64 current-seed oracles | no driver fault reported; queue wait, command-buffer lifetime, cleanup begin, and cleanup complete recorded |
| invalidate | 1 for 64/64 | same 64 records, seeds, roots, QMD slots, VAs, footprint, and 63/63 transitions | same 64/64 copy/decode/dispatch/order decisions; corresponding generated QMDs differ from control only at typed bit 255 | byte-for-byte same per-iteration pixels/checksums as control; iterations 3-64 retain seed-42 output; 2/64 current-seed oracles | no driver fault reported; queue wait, command-buffer lifetime, cleanup begin, and cleanup complete recorded |

The paired validator accepted all 64 joined control/variant records: corresponding seeds, root VAs,
selected QMD VAs, direct dispatch VAs, pixels, and checksums matched, while the decoded field was zero
in control and one in variant. Complete generated/counterfactual comparisons established the exclusive
bit-255 difference. CPU-mapped equality alone is not evidence of GPU visibility or consumption.

## Classification

`specific_qmd_shader_constant_cache_invalidate_insufficient`

Setting QMD v0.6 `INVALIDATE_SHADER_CONSTANT_CACHE` was insufficient to change the retained
two-root/two-QMD failure signature. This is a narrow negative result for this controlled launch
pattern. It does not establish an undocumented cache key, physical-backing behavior, cache lifetime,
a generic GM20B defect, or a general NVK defect. Root reuse distance is the next separately specified
discriminator; it is not implemented or combined here. FG-2 remains `BLOCKED`.
