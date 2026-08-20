# FG-2 shared-layout controlled variant host build — 2026-08-20

Status: `PROVEN_HOST`

This record covers only cross-build, package, and source-review evidence for the controlled
shared-pipeline-layout variant. FG-2 remains `BLOCKED` until the complete deterministic chain passes
on real Switch/Tegra hardware.

```text
Date: 2026-08-20
Repository commit: d9dfc6568dd0580884a2352a0622bb3bbb166b6f
Branch: codex/prove-render-compute-image-chain-shared-layout
Spec/milestone: openspec/changes/prove-render-compute-image-chain / FG-2
Build environment: switch-nvk-build:latest, image 4e516b1f3576
Build command: APP=nvk_render_compute TITLE="NVK Render Compute" VERSION=0.61.0-chain2 bash winsys/build-nro.sh
Artifact: nvk_render_compute.nro
Artifact size: 12675269 bytes
Artifact SHA256: 505586148e0f2282ea71dfd063a3255ccecf9dd433c9b50e76f24a58fec7f30e
Embedded identifiers: title "NVK Render Compute"; version "0.61.0-chain2"; application log tag "BUILD chain2"
Conclusion/status: PROVEN_HOST for controlled-variant compilation and packaging only; FG-2 remains BLOCKED pending real-hardware acceptance
```

Source review confirmed that the controlled variant:

- creates one `VkPipelineLayout` containing the compute descriptor-set layout;
- defines one four-byte push-constant range for fragment and compute stages;
- uses that same pipeline layout for graphics and compute pipeline creation;
- issues one combined-stage `vkCmdPushConstants` before the draw and no later compute-only push;
- retains the existing shaders, two-image chain, barriers, dispatch, CPU oracle, exact pixel checks,
  checksum checks, and stale-seed detection.

The build completed without compiler or linker diagnostics. This host result cannot determine whether
the shared-layout variant fixes the stale compute seed on GM20B.

## Initial hardware-run availability

After packaging, nxlink could not connect to the previously recorded Switch endpoint at
`192.168.15.13`, and a read-only FTP availability check could not connect to port 5000. No artifact
was uploaded and no code executed on the device. At that point, tasks 8.4, 8.5, and 9.1 remained
incomplete.

This availability note was superseded later on 2026-08-20 when the endpoint came online and the
controlled artifact completed its full hardware run. See
`docs/testing/FG2_NVK_RENDER_COMPUTE_SHARED_LAYOUT_FAILURE_2026-08-20.md`.
