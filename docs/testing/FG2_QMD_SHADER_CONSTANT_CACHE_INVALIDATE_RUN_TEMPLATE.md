# FG-2 QMD shader-constant-cache invalidation paired hardware record

Status: `IMPLEMENTED_UNPROVEN`

```text
Date:
Immutable repository commit:
Spec/milestone: test-compute-qmd-shader-constant-cache-invalidate / FG-2
Hardware/model:
Firmware/Atmosphere/libnx context:
Build type/toolchain:
Control artifact/version/SHA256:
Variant artifact/version/SHA256:
Durable patch/shader/generated-header SHA256:
Selectors and startup paths:
Expected VAs/allocation footprint/backing:
Control raw complete-log location/SHA256:
Control full-log review:
Control authorization: yes/no
Variant raw complete-log location/SHA256:
Variant full-log review:
Notifier/error-info/GPU fault review:
Classification: qmd_shader_constant_cache_invalidate_supported_experiment_only | specific_qmd_shader_constant_cache_invalidate_insufficient | behavior_changed_unresolved | inconclusive
FG-2 status: BLOCKED
```

## Required causal table

Record all 64 iterations for each executed arm: artifact/selector, record/seed, root bytes and A/B address, selected/counterfactual/mapped complete-payload decisions, decoded bit 255, decoded root VA, X/Y QMD and direct `PCAS` VA, copy/dispatch/order decisions, pixel/checksum/oracle, teardown, and notifier/error state.

The control must run and receive explicit authorization first. It is valid only if iteration 1 is `0xfa47d33f`/`0xb7d223e5`, iteration 2 is `0xf5031a17`/`0x0daf4ac5`, iterations 3-64 retain the iteration-2 values, bit 255 is zero, all causal aggregates are complete, and the full unfiltered stream contains no unexplained fault. If any prerequisite fails, classify `inconclusive` and do not run or interpret the variant.

CPU-mapped equality and successful typed field construction remain `UNPROVEN` for GPU visibility or consumption.
