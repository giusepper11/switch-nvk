# FG-2 compute-root address-reuse hardware run — 2026-08-21

```text
Date: 2026-08-21
Repository immutable experiment commit: f542b44e01579d8dd736da4686f5a0b6b91bd350
Spec/milestone: test-compute-root-address-reuse / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1 console at 192.168.15.13:28280
Firmware/Atmosphere/libnx context: firmware/Atmosphere versions not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application .nro, paired root-address artifacts
Diagnostics/selectors: NVK_ROOT_ADDRESS_CONTROL=1 or NVK_ROOT_ADDRESS_FRESH=1; all root-cache and QMD experiment selectors absent
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM images
Clock/OC state: not changed/captured
Control artifact/version/SHA256: chain2-root-address-control / 0.61.0-root-address-control / 1b1bdedcc0529785f6c22f33dfa73aea76ffafa4fe65ff0f7895ef21be07355f
Fresh-root artifact/version/SHA256: chain2-root-address-fresh / 0.61.0-root-address-fresh / d0e8c3edfc2aa3d69142540855f35edf92a422400c9fb90f9b95152f50e432e0
Durable patch SHA256: 7e515c726856cfbd6f825dd5ee01a7caa863e943417b10b68a5afec5f23931fe
Vertex/fragment/compute/generated-header SHA256: dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0 / 14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235 / 1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49 / 21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14
Run duration/iterations: control and variant each completed 64; variant ran only after valid control review
Allocation footprint/lifetime: 4,608 bytes; 60,928 remain; +2,048 live; command-buffer-owned through wait/reset
Control observed: iteration 1 exact; iterations 2-64 exact retained stale signature; root 0/63; QMD 63/63
Control raw log/SHA256: docs/testing/raw/FG2_ROOT_ADDRESS_CONTROL_NXLINK_2026-08-21.txt / 4b47c3aa386e7f00eee15f69e7433cd6f96ae95d792e82ac2d0c4b24731d1477
Control full-log review: complete 497-line stream read before filtering; expected NVK non-conformance warning only; no timeout, ERRNOTIF, ERRINFO, GPU fault, incomplete teardown, or unexplained warning
Variant authorized: yes; every control prerequisite passed
Variant correlation: root 63/63; QMD 63/63; root copies 64/64 each; QMD copies 64/64; decode/outside-mask/PCAS 64/64
Variant observed: iterations 1 and 2 exact; iterations 3-64 retained iteration-2/seed-42 output pixel 0xf5031a17 and checksum 0x0daf4ac5
Variant raw log/SHA256: docs/testing/raw/FG2_ROOT_ADDRESS_FRESH_NXLINK_2026-08-21.txt / 840ed55105dd1fde24d1c30dfbdb9e05d7ac84a0b977bb029a83e60c570f2696
Variant full-log review: complete 504-line stream read before filtering; expected NVK non-conformance warning only; no timeout, ERRNOTIF, ERRINFO, GPU fault, incomplete teardown, or unexplained warning
Exact oracle: control 1/64; variant 2/64; variant changed behavior without passing the complete oracle
Classification: behavior_changed_unresolved
FG-2 status: BLOCKED
FG-3 status: unchanged/out of scope
Production promotion: forbidden; selectors remain opt-in and disabled in ordinary execution
```

## Paired causal table

| Path | Records | Root selection / transitions | QMD selection / transitions | Copy, decode, mask, dispatch | Output/oracle | Fault state |
|---|---:|---|---|---|---|---|
| control | 64 | primary `0xc7f40000`; 0/63 | `0xc7f40800`/`0xc7f40900`; 63/63 | primary+alternate root copies 64/64; QMD 64/64; decode/mask/PCAS 64/64 | iteration 1 exact; iterations 2-64 `0xfab61a38`/`0xc17a35a5`; 1/64 | none reported |
| fresh | 64 | primary `0xc7f40000`/alternate `0xc7f40a00`; 63/63 | same QMD slots; 63/63 | primary+alternate root copies 64/64; QMD 64/64; decode/mask/PCAS 64/64 | iterations 1-2 exact; iterations 3-64 seed-42 `0xf5031a17`/`0x0daf4ac5`; 2/64 | none reported |

The reference and selected QMD payloads differ only in the layout-decoded root-address field when
the alternate root is selected. Complete in-process comparisons, not hashes, establish the copy and
outside-mask decisions. CPU-mapped equality is not evidence of GPU visibility.

## Classification

`behavior_changed_unresolved`

Changing the selected compute-root GPU VA on every adjacent dispatch changed the retained failure
signature and allowed iteration 2 to pass, but it did not make later iterations consume their current
root contents. This supports neither a production fix nor FG-2 acceptance. It also does not justify
combining another intervention in this change. FG-2 remains `BLOCKED`.
