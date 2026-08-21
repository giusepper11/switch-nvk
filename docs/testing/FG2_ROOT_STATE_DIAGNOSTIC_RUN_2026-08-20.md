# FG-2 compute-root state diagnostic hardware run — 2026-08-20

```text
Date: 2026-08-20
Repository commit: 36cf3dd37ae3d25e2e47244472665da458113290 (exact diagnostic source revision)
Spec/milestone: openspec/changes/diagnose-stale-compute-root-state / FG-2
Hardware/model: Nintendo Switch OLED, real Tegra X1, netloaded at 192.168.15.13:28280
Firmware/Atmosphere/libnx context when relevant: firmware/CFW version not captured; devkitA64/libnx 4.12.0 build environment
Build type: intended devkitA64 full Application .nro
Diagnostics enabled: ROOT_DIAG_LIMIT=2; runtime NVK_ROOT_TRACE=2; complete combined nxlink/application/driver stream
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM optimal-tiled images
Swapchain/buffer count/present mode when relevant: N/A — headless artifact
Clock/OC state when relevant: not changed/captured
Test artifact/version: APP=nvk_render_compute, OUTPUT=nvk_render_compute_rootdiag1, BUILD=chain2-rootdiag1, VERSION=0.62.0-rootdiag1
Artifact SHA256: 7e1e12b38eb487a81cd6909e6b351558102708ac2d24e9e4ee13bfac637eeff7
Run duration/iteration count: all 64 submit/wait/readback iterations executed; detailed diagnostic capture limited to iterations 1-2
Expected result: exact CPU-oracle match for all 256 pixels on all 64 changing-seed iterations
Observed result: iteration 1 seed 5 exact; iterations 2-64 each mismatched all 256 pixels with stable observed pixel[0]=0xfab61a38 and checksum 0xc17a35a5
Deterministic validation/checksum: iteration 1 observed/expected 0xb7d223e5; iteration 2 observed 0xc17a35a5 versus expected 0x0daf4ac5; RESULT FAIL with 63/64 iterations mismatched
GPU error notifier/error info: complete combined stream inspected; no timeout, ERRNOTIF, ERRINFO, GPU fault, or unexplained driver warning; expected NVK non-conformance warning only
Relevant timing summary: N/A — correctness-only artifact
Raw evidence location/reference: docs/testing/raw/FG2_ROOT_DIAG_NXLINK_2026-08-20.txt (104-line unfiltered combined stream)
Raw evidence SHA256: 0f319e41e2502a8b22c79ca2ff95d74ee249f7c78bc3013956625c1af3fd8b31
Configured device logs: sdmc:/nvk_render_compute.log and sdmc:/nvk_render_compute_mesa.log were configured but not separately retrieved; the installed shim sink is present in the retained combined nxlink stream
Instrumentation observer effect: none detected; the 64-iteration output signature exactly matches both retained pre-instrumentation FG-2 failures
Conclusion/status: BLOCKED; current seed and address state is proven through the CPU mapping backing the root GPU VA and the generated/dispatched QMD, but the stale boundary remains unresolved after that CPU/QMD state and before or at GPU constant-buffer consumption
```

## Correlated diagnostic records

Every marker, push, upload, QMD, QMD-upload, dispatch, and result record appeared exactly once and
in order for records 1 and 2. The command-buffer identity was `0x8d732f040` throughout. Descriptor
and root identities were stable within and across both records, which is expected because the
primary command buffer was reset and rerecorded rather than reallocated.

| Iteration | Expected seed | Graphics CPU root | Compute CPU root | Upload source | Uploaded CPU mapping | Upload GPU VA | QMD root VA | Dispatched QMD VA | Observed behavior seed |
|---|---:|---:|---:|---:|---:|---|---|---|---|
| 1 | 5 | 5 | 5 | 5 | 5 | `0xc7f40000` | `0xc7f40000` | `0xc7f40800` | 5 |
| 2 | 42 | 42 | 42 | 42 | 42 | `0xc7f40000` | `0xc7f40000` | `0xc7f40800` | `UNKNOWN` |

For both records, the root constant buffer was shader cbuf index 0. NAK reported GM20B address
ranges `928:960` and `960:968`; the decoded root address matched the current upload. The QMD was
submitted as `PCAS=0x00c7f408`. Record 2 reported both root and QMD GPU-address reuse, with current
seed 42 visible in the CPU mapping backing that root VA. `order_match=1`, `decode_ok=1`, and
`address_match=1` for both records.

`observed_behavior_seed=UNKNOWN` on iteration 2 is the correct result of the artifact's exhaustive
single-seed classifier: the stale output does not equal the oracle image for any one seed because
the graphics and compute inputs are not jointly represented by a fresh single-seed oracle. The
exact pixel/checksum signature, rather than that classifier alone, matches both earlier failures.

## Full-log review

The complete 104-line retained stream was inspected before filtering. It begins with nxlink
transfer/server lines, includes device enumeration and all setup/path indicators, contains the two
complete diagnostic record sets, reports every iteration result through 64, then reports
`RESULT FAIL`, clean Vulkan teardown, and nxlink exit. No line reports a timeout, GPU fault,
`ERRNOTIF`, or `ERRINFO`; therefore no fault-triggered notifier capture was applicable.

The only warning is NVK's expected non-conformance warning. The application and Mesa SD paths were
configured, but no separate FTP/SD retrieval was available after this run; their absence is
explicit and does not hide the complete combined stream routed through the installed shim sink.
