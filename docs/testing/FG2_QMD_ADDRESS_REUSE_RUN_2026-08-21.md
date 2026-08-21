# FG-2 QMD address-reuse hardware run — 2026-08-21

```text
Date: 2026-08-21
Repository experiment-source commit: c83aca57c680fd6cf53275f96852d53191b7c443
Evidence/provenance commit before execution: 0aa1a54
Spec/milestone: openspec/changes/test-compute-qmd-address-reuse / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1 console at 192.168.15.13:28280
Firmware/Atmosphere/libnx context when relevant: firmware/Atmosphere versions not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application .nro, paired QMD-address control/fresh artifacts
Diagnostics enabled: NVK_QMD_ADDRESS_CONTROL=1 or NVK_QMD_ADDRESS_FRESH=1; root/QMD cache selectors absent; complete combined nxlink/application/driver stream
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM optimal-tiled images
Swapchain/buffer count/present mode when relevant: N/A — headless artifact
Clock/OC state when relevant: not changed/captured
Control artifact: nvk_render_compute_qmdaddress_control.nro
Control artifact SHA256: 97cca217c20071a802e676205495062c54ba3c50c28f49861efd744d5f94d530
Fresh-address artifact: nvk_render_compute_qmdaddress_fresh.nro
Fresh-address artifact SHA256: d760e826c38217db0f6cc147e4def272cc4ef18407c21b131670e2167f5436cc
Generated shader-source SHA256: vertex dbda7a3cbff7bd04e8514530d91c3a525a4369eee5cd87f84341ca2e2aaa7cd0; fragment 14660d60798626e636d5b04593243d1c002f2cb8b2598baeec3a842bd281f235; compute 1060e03a1648afcedc4d4d3d64db891cbeb1b99ae1c45687997a65a096150f49; generated tri_shaders.h 21b2d465433f85e46c400de74c2ff3c96b5ab0bd878aa027f5e221168773fa14
Run duration/iteration count: control and fresh-address variant each completed all 64 submit/wait/readback iterations
Expected result: valid control reproduces the stale signature; variant changes selected QMD address on every consecutive transition and passes only if all 64 oracle validations pass
Observed result: both artifacts passed iteration 1 exactly and reproduced the exact established stale pixel/checksum on iterations 2-64; variant achieved 63/63 fresh transitions
Deterministic validation/checksum: iteration 1 pixel[0]=0xfa47d33f and checksum 0xb7d223e5 exact; iterations 2-64 pixel[0]=0xfab61a38 and checksum 0xc17a35a5; each artifact RESULT FAIL with 63/64 iterations mismatched
GPU error notifier/error info: both complete 497-line streams inspected before filtering; no timeout, ERRNOTIF, ERRINFO, GPU fault, or unexplained driver warning; expected NVK non-conformance warning only; no fault-triggered notifier capture was applicable
Relevant timing summary: N/A — correctness-only address experiment
Control raw evidence: docs/testing/raw/FG2_QMD_ADDRESS_CONTROL_NXLINK_2026-08-21.txt; 497 lines; SHA256 2019d7df4ae1b64a616d9989f09bcb30a5ae5dd78bf003d4a778a2b7c4cfe056
Fresh raw evidence: docs/testing/raw/FG2_QMD_ADDRESS_FRESH_NXLINK_2026-08-21.txt; 497 lines; SHA256 0634a7acb8c71328551aacd062bbb5557feca736bfea05548ca58fa2aca7341c
Instrumentation observer effect: none detected; control exactly reproduced the established iteration-1 pass and iterations-2-64 stale signature
Conclusion/status: PROVEN_HW for the executed two-slot address intervention and its unchanged stale result; classification specific_qmd_address_change_insufficient; FG-2 remains BLOCKED
```

## Paired correlation

Both complete streams were read before filtering. Equality decisions below come from full 256-byte
comparisons; hashes are compact labels only. The eight bounded payload lines (all 64 dwords) are exactly
identical across control and variant and show identical source and selected mappings.

| Path | Iteration / seed | Root seed / VA | QMD identity and exact equality | Primary / secondary / selected / previous VA | Fresh / dispatch-PCAS / order | Pixel / checksum / oracle | Artifact and fault state |
|---|---|---|---|---|---|---|---|
| control | 1 / 5 | source=mapped=5 / `0xc7f40000` | source=mapped=`d4d9b1921f9202ad`; copy exact | `0xc7f40800` / `0xc7f40900` / `0xc7f40800` / none | 0 / match / complete | `0xfa47d33f` / `0xb7d223e5` / exact | control hash above; none reported |
| control | 2-64 / fixed sequence | source=mapped=current seed / reused `0xc7f40000` | source=mapped=`d4d9b1921f9202ad`; 63/63 equal to first and previous; 64/64 copies exact | same slots / primary every record / previous primary | 0/63 / 64/64 / complete | always `0xfab61a38` / `0xc17a35a5` / mismatch | control hash above; none reported |
| fresh | 1 / 5 | source=mapped=5 / `0xc7f40000` | source=mapped=`d4d9b1921f9202ad`; copy and cross-artifact payload exact | `0xc7f40800` / `0xc7f40900` / primary / none | 0 / match / complete | `0xfa47d33f` / `0xb7d223e5` / exact | fresh hash above; none reported |
| fresh | 2-64 / fixed sequence | source=mapped=current seed / reused `0xc7f40000` | source=mapped=`d4d9b1921f9202ad`; 63/63 equal to first and previous; 64/64 copies exact | same slots / alternate secondary-primary / prior selected slot | 63/63 / 64/64 / complete | always `0xfab61a38` / `0xc17a35a5` / mismatch | fresh hash above; none reported |

The variant aggregate is `exact_copies=64/64`, `first_equal=63/63`,
`previous_equal=63/63`, `fresh_transitions=63/63`, `dispatch_matches=64/64`,
`slot_valid=1`, and `ordering_complete=1`. Its independent oracle aggregate is `validations=1/64` and
`mismatched_iterations=63`. Control differs only in selecting the primary slot for every dispatch and
therefore reports `fresh_transitions=0/63`.

## Decision and classification

Classification: `specific_qmd_address_change_insufficient`.

The exact established stale signature survived all 63 consecutive transitions between two distinct,
aligned QMD GPU addresses while the generated payload, root behavior, direct-dispatch address,
resources, synchronization, and oracle remained controlled. Therefore this specific two-slot,
one-intervening-dispatch address intervention was insufficient.

This does not reject other QMD, launch-state, cache, allocator, or GPU-consumption hypotheses. Address
reuse after one intervening dispatch remains possible by construction. The opt-in selectors are not a
production fix, and FG-2 remains `BLOCKED`.
