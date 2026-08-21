# FG-2 compute-root address-reuse-distance paired hardware run — template

Status: `IMPLEMENTED_UNPROVEN` until the gated real-Tegra protocol is executed and fully reviewed.

```text
Date:
Repository immutable experiment commit:
Spec/milestone: test-compute-root-address-reuse-distance / FG-2
Hardware/model:
Firmware/Atmosphere/libnx context:
Build type/toolchain:
Control artifact/version/SHA256:
Variant artifact/version/SHA256:
Durable patch SHA256:
Harness/vertex/fragment/compute/generated-header SHA256:
Selectors: control=ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1; variant=ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1; all historical interventions=0
Actual allocation order: root A / QMD X / QMD Y / root B / QMD Z / QMD W / root C
Actual CPU mappings/offsets/GPU VAs:
Actual root/QMD sizes and alignments:
Actual command-upload BO handle/NvMap identity:
Actual footprint/remaining capacity:
Ownership/lifetime/cleanup and later-consumer displacement:
Control intended schedule: roots A/B; QMDs X/Y/Z/W; root first revisit 3 distance 2; QMD first revisit 5 distance 4
Variant intended schedule: roots A/B/C; QMDs X/Y/Z/W; root first revisit 4 distance 3; QMD first revisit 5 distance 4
Run duration/iterations: control 64; variant 64 only if control gate passes
Complete control raw-log path/line count/SHA256:
Complete variant raw-log path/line count/SHA256 (or NOT AUTHORIZED):
Complete-stream inspection before filtering:
Root A/B/C full-copy decisions:
QMD X/Y/Z/W full-copy/outside-mask/root-decode/PCAS/order decisions:
QMD cache/membar state including INVALIDATE_SHADER_CONSTANT_CACHE=FALSE:
Control oracle and exact retained signature:
Variant oracle and exact retained signature:
GPU notifier/error-info/timeout/fault review:
Teardown/lifetime result:
Classification: root_address_reuse_distance_hypothesis_supported_experiment_only | specific_root_address_reuse_distance_change_insufficient | behavior_changed_unresolved | inconclusive
Non-promotion: FG-2 remains BLOCKED; FG-3/FG-4 unchanged; experiment ring is not production allocator policy
```

## Decisive iterations 1–5

| Record | Seed | Control root/QMD | Control pixel/checksum/oracle | Variant root/QMD | Variant pixel/checksum/oracle | Root/QMD reuse distances | Causal/fault decision |
|---:|---:|---|---|---|---|---|---|
| 1 | 5 | A/X | | A/X | | root first / QMD first | |
| 2 | 42 | B/Y | | B/Y | | root first / QMD first | |
| 3 | 79 | A/Z | | C/Z | | control root distance 2 / variant root first | |
| 4 | 116 | B/W | | A/W | | control root distance 2 / variant root distance 3 | |
| 5 | 153 | A/X | | B/X | | QMD distance 4 first revisit | |

## Full 64-record paired causal table

Populate one row per record after reading each complete unfiltered stream. Include actual selected root
and QMD VAs, last-use distances and first-revisit flags derived from those VAs, all copy/decode/mask/
PCAS/order decisions, observed and expected pixel/checksum, behavior seed, oracle result, and fault state.

CPU-mapped equality remains `UNPROVEN` for GPU visibility. Public/source evidence is not hardware
proof. Do not interpret record 5 or later as an independent QMD-reuse experiment.
