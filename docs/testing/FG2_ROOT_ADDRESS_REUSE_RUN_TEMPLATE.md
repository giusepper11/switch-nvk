# FG-2 compute-root address-reuse hardware run — template

Do not classify the variant unless the control satisfies every prerequisite.

```text
Date:
Repository immutable experiment commit:
Spec/milestone: test-compute-root-address-reuse / FG-2
Hardware/model:
Firmware/Atmosphere/libnx context:
Build type:
Diagnostics/selectors:
Resolution/format: two distinct 16x16 VK_FORMAT_R8G8B8A8_UNORM images
Clock/OC state:
Control artifact/version/SHA256:
Fresh-root artifact/version/SHA256:
Durable patch SHA256:
Vertex/fragment/compute/generated-header SHA256:
Run duration/iterations: 64 each, variant gated on valid control
Allocation footprint/lifetime: 4,608 bytes; 60,928 remain; +2,048 live; command-buffer-owned through wait/reset
Expected control: iteration 1 exact, iterations 2-64 exact retained stale signature; root 0/63; QMD 63/63
Observed control:
Control complete raw-log location/SHA256:
Control full-log warnings/fault/notifier review:
Variant authorized (yes/no and exact gate):
Expected variant correlation: root 63/63; QMD 63/63; root copies 64/64 each; QMD copies 64/64; decode/outside-mask/PCAS 64/64
Observed variant:
Variant complete raw-log location/SHA256:
Variant full-log warnings/fault/notifier review:
Exact per-iteration pixels/checksums/oracle:
Paired causal table location:
Classification: root_address_reuse_hypothesis_supported_experiment_only | specific_root_address_change_insufficient | behavior_changed_unresolved | inconclusive
FG-2 status: BLOCKED
FG-3 status: unchanged/out of scope
Production promotion: forbidden; selectors remain opt-in or are removed after evidence capture
```
