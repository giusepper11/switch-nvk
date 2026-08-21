# FG-2 compute-root address-reuse-distance hardware run — 2026-08-21

Status: paired run complete; `specific_root_address_reuse_distance_change_insufficient`.

```text
Date: 2026-08-21
Repository immutable experiment commit: a71baed9d432b3f168d26bf4a3edd4679b8461b7
Spec/milestone: test-compute-root-address-reuse-distance / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1 console at 192.168.15.13
Firmware/Atmosphere context: versions not captured; same intended console/configuration as the retained 2026-08-21 FG-2 experiments
Build type/toolchain: devkitA64 full Application NRO; switch-nvk-build image sha256:4e516b1f35766dbb6c6061e606613ce5d19d8c393e03dee750675cbd80190832; Meson 1.12.0; Ninja 1.11.1; devkitA64 GCC 15.2.0; glslang 12.0.0
Control artifact/version/SHA256: chain2-root-reuse-distance-control / 0.63.0-root-reuse-control / 6b7ccfa260868ee45e05fa53923467527e2f47798d375c8497e9fe6e421f1569
Variant artifact/version/SHA256: chain2-root-reuse-distance-variant / 0.63.0-root-reuse-variant / e1be1a3e0372cd4aa3376b158ba45218e256d33526e2cf3ec5108503b96ac549
Durable patch SHA256: 36322b38cb64da99ee6ccb6e97b3410f7edc3d6ee3bcecd8e3a9aea67c7b76e1
Harness SHA256: 74fa6ef0960b68a311c69af23b680b90b02add68f0af49c1059f16acfb10c9c5
Vertex/fragment/compute source SHA256: dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0 / 14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235 / 1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49
Generated tri_shaders.h SHA256: 21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14
Selectors: control ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1; variant ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1; every historical intervention selector disabled
Protocol: control first for 64 records; read the complete unfiltered stream and require the exact seed-42 onset gate; run variant for 64 records only after authorization; read its complete stream before filtering or classification
Actual allocation order: root A / QMD X / QMD Y / root B / QMD Z / QMD W / root C
Control CPU mappings: 0x884d6b000 / 0x884d6b800 / 0x884d6b900 / 0x884d6ba00 / 0x884d6c200 / 0x884d6c300 / 0x884d6c400
Variant CPU mappings: 0xc64d6b000 / 0xc64d6b800 / 0xc64d6b900 / 0xc64d6ba00 / 0xc64d6c200 / 0xc64d6c300 / 0xc64d6c400
Actual offsets: 0 / 2048 / 2304 / 2560 / 4608 / 4864 / 5120
Actual GPU VAs: 0xc7f40000 / 0xc7f40800 / 0xc7f40900 / 0xc7f40a00 / 0xc7f41200 / 0xc7f41300 / 0xc7f41400
Actual root/QMD sizes and alignments: root 2048/256; QMD 256/256 bytes
Actual command-upload identity: upload_mem=0x884ceefe0; backing_mem=0x884d24f50; BO handle 17; nvmap_identity=same_shim_bo_handle; one_backing=1
Actual footprint/remaining capacity: 7168 / 58368 bytes; later_consumer_displacement=0
Ownership/lifetime/cleanup: command-buffer-owned through completion; existing pool cleanup; final queue wait and cleanup complete
Control intended schedule: roots A/B; QMDs X/Y/Z/W; root first revisit 3 distance 2; QMD first revisit 5 distance 4
Variant intended schedule: roots A/B/C; QMDs X/Y/Z/W; root first revisit 4 distance 3; QMD first revisit 5 distance 4
Control raw log: docs/testing/raw/fg2_root_reuse_distance_control_2026-08-21.log; 534 lines; 156058 bytes; SHA256 9a876f4ddcf0518f9dee4d2dba796d5a064bf07f63f5e79cdf72ffc9f6f4654a
Control full-stream inspection: complete before filtering; only expected NVK non-conformance warning; no timeout, ERRNOTIF, ERRINFO, assertion, incomplete stream, or unexplained GPU fault
Control causal validation: VALID; 64 records; causal_evidence_complete=1
Control oracle: records 1-2 consume seeds 5 and 42 exactly; record 3 first fails on root A/QMD Z and records 3-64 retain seed-42 pixel 0xf5031a17 / checksum 0x0daf4ac5
Control authorization: PASS; variant authorized
Variant raw log: docs/testing/raw/fg2_root_reuse_distance_variant_2026-08-21.log; 534 lines; 156078 bytes; SHA256 b1f2ac8236bf95ea5318802204856743bbbd41d73a028dd4c3090dabdc7875d1
Variant full-stream inspection: complete before filtering; only expected NVK non-conformance warning; no timeout, ERRNOTIF, ERRINFO, assertion, incomplete stream, or unexplained GPU fault
Variant causal validation: VALID; 64 records; causal_evidence_complete=1
Variant oracle: records 1-2 consume seeds 5 and 42 exactly; record 3 uses fresh root C/QMD Z but retains seed-42 pixel 0xf5031a17 / checksum 0x0daf4ac5; records 3-64 retain the same signature
Classification: specific_root_address_reuse_distance_change_insufficient
Next discriminator: at most one later OpenSpec change may isolate compute-root backing identity; no such intervention is designed or implemented here
Non-promotion: FG-2 remains BLOCKED; FG-3/FG-4 unchanged; experiment ring is not production allocator policy
Selector state after capture: both selectors remain opt-in and disabled in ordinary execution
```

## Compact decisive comparison

| Record | Seed | Control root/QMD | Variant root/QMD | Control and variant output | Decision |
|---:|---:|---|---|---|---|
| 1 | 5 | A/X, first use | A/X, first use | `0xfa47d33f` / `0xb7d223e5` | both exact |
| 2 | 42 | B/Y, first use | B/Y, first use | `0xf5031a17` / `0x0daf4ac5` | both exact |
| 3 | 79 | A/Z, root distance 2 | C/Z, both new | `0xf5031a17` / `0x0daf4ac5` | both retain seed 42; variant does not consume seed 79 |
| 4 | 116 | B/W, root distance 2 | A/W, first root revisit distance 3 | `0xf5031a17` / `0x0daf4ac5` | no predicted one-record phase shift |
| 5 | 153 | A/X, root/QMD distances 2/4 | B/X, root/QMD distances 3/4 | `0xf5031a17` / `0x0daf4ac5` | no independent QMD conclusion |

All 64 control records report complete A/B/C root copies, selected QMD copies, authoritative root
decodes, equality outside the root-address mask, `INVALIDATE_SHADER_CONSTANT_CACHE=FALSE`, mapped-
QMD-to-direct-PCAS equality, and copy-before-dispatch ordering. The aggregate reports 64/64 for every
causal counter, one backing, 7,168-byte footprint, ownership through completion, and no driver fault.
CPU-mapped equality remains `UNPROVEN` for GPU visibility. The variant aggregate likewise reports
64/64 for every causal counter, one backing, the same footprint and capacity, ownership through
completion, and no driver fault. Because a fresh root C at record 3 still returned seed-42 output,
the failure boundary did not move with the tested distance increase. This rejects only that specific
three-root reuse-distance intervention as sufficient.

## Full paired causal table

`R/Q distance` is derived from exact dispatched GPU VAs; zero means first use. `PASS` means both
records passed root A/B/C copies, selected-QMD copy, generated-layout root decode, outside-root-mask
comparison, QMD cache-field freeze, mapped-QMD/direct-PCAS equality, copy-before-dispatch ordering,
one-backing, record join, fault-state, and teardown prerequisites. `STALE` is an oracle mismatch with
the known seed-42 signature, not a failed causal gate.

| Rec | Seed | Control root/QMD (R/Q distance) | Control pixel/checksum | Variant root/QMD (R/Q distance) | Variant pixel/checksum | Gates |
|---:|---:|---|---|---|---|---|
| 1 | 5 | A/X (0/0) | `0xfa47d33f` / `0xb7d223e5` (EXACT) | A/X (0/0) | `0xfa47d33f` / `0xb7d223e5` (EXACT) | PASS |
| 2 | 42 | B/Y (0/0) | `0xf5031a17` / `0x0daf4ac5` (EXACT) | B/Y (0/0) | `0xf5031a17` / `0x0daf4ac5` (EXACT) | PASS |
| 3 | 79 | A/Z (2/0) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Z (0/0) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 4 | 116 | B/W (2/0) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/W (3/0) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 5 | 153 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 6 | 190 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 7 | 227 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 8 | 8 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 9 | 45 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 10 | 82 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 11 | 119 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 12 | 156 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 13 | 193 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 14 | 230 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 15 | 11 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 16 | 48 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 17 | 85 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 18 | 122 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 19 | 159 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 20 | 196 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 21 | 233 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 22 | 14 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 23 | 51 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 24 | 88 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 25 | 125 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 26 | 162 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 27 | 199 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 28 | 236 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 29 | 17 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 30 | 54 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 31 | 91 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 32 | 128 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 33 | 165 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 34 | 202 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 35 | 239 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 36 | 20 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 37 | 57 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 38 | 94 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 39 | 131 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 40 | 168 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 41 | 205 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 42 | 242 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 43 | 23 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 44 | 60 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 45 | 97 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 46 | 134 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 47 | 171 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 48 | 208 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 49 | 245 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 50 | 26 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 51 | 63 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 52 | 100 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 53 | 137 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 54 | 174 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 55 | 211 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 56 | 248 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 57 | 29 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 58 | 66 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 59 | 103 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 60 | 140 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 61 | 177 | A/X (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/X (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 62 | 214 | B/Y (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | B/Y (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 63 | 251 | A/Z (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | C/Z (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
| 64 | 32 | B/W (2/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | A/W (3/4) | `0xf5031a17` / `0x0daf4ac5` (STALE) | PASS |
