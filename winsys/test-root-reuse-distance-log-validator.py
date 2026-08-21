#!/usr/bin/env python3
"""Mutation tests for validate-root-reuse-distance-log.py."""

from __future__ import annotations

import importlib.util
from pathlib import Path

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location(
    "validator", HERE / "validate-root-reuse-distance-log.py")
assert SPEC and SPEC.loader
validator = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(validator)


def valid_stream(arm: str = "control") -> str:
    lines: list[str] = []
    variant = arm == "variant"
    root_schedule = "A/B/C" if variant else "A/B"
    root_first_record = 4 if variant else 3
    root_first_distance = 3 if variant else 2
    roots = ["0x00000000c7f40000", "0x00000000c7f40a00", "0x00000000c7f41400"]
    qmds = ["0x00000000c7f40800", "0x00000000c7f40900",
            "0x00000000c7f41200", "0x00000000c7f41300"]
    lines.append(
        f"FG2_ROOT_REUSE_DISTANCE phase=arm experiment=test control_selector={int(not variant)} "
        f"variant_selector={int(variant)} arm={arm} root_schedule={root_schedule} "
        f"qmd_schedule=X/Y/Z/W first_root_revisit={root_first_record} "
        "first_qmd_revisit=5 records=64 device_work_started=0")
    lines.append(
        f"FG2_ROOT_REUSE_DISTANCE phase=allocation arm={arm} record=1 "
        f"root_a_map=0x1 root_a_offset=0 root_a_va={roots[0]} "
        f"qmd_x_map=0x2 qmd_x_offset=2048 qmd_x_va={qmds[0]} "
        f"qmd_y_map=0x3 qmd_y_offset=2304 qmd_y_va={qmds[1]} "
        f"root_b_map=0x4 root_b_offset=2560 root_b_va={roots[1]} "
        f"qmd_z_map=0x5 qmd_z_offset=4608 qmd_z_va={qmds[2]} "
        f"qmd_w_map=0x6 qmd_w_offset=4864 qmd_w_va={qmds[3]} "
        f"root_c_map=0x7 root_c_offset=5120 root_c_va={roots[2]} "
        "root_size=2048 qmd_size=256 root_alignment=256 qmd_alignment=256 "
        "upload_mem=0x8 backing_mem=0x9 bo_handle=1 nvmap_identity=same_shim_bo_handle "
        "upload_base=0x00000000c7f40000 footprint=7168 remaining=58368 one_backing=1 "
        "ownership=command_buffer_through_completion cleanup=existing_pool_path "
        "later_consumer_displacement=0 gpu_visible=UNPROVEN")

    root_ring_size = 3 if variant else 2
    root_last: dict[str, int] = {}
    qmd_last: dict[str, int] = {}
    root_revisited: set[str] = set()
    qmd_revisited: set[str] = set()
    previous_root = "0x0000000000000000"
    previous_qmd = "0x0000000000000000"
    for record in range(1, 65):
        root_idx = (record - 1) % root_ring_size
        qmd_idx = (record - 1) % 4
        root = roots[root_idx]
        qmd = qmds[qmd_idx]
        root_distance = record - root_last[root] if root in root_last else 0
        qmd_distance = record - qmd_last[qmd] if qmd in qmd_last else 0
        root_first = root in root_last and root not in root_revisited
        qmd_first = qmd in qmd_last and qmd not in qmd_revisited
        if root_first:
            root_revisited.add(root)
        if qmd_first:
            qmd_revisited.add(qmd)
        seed = ((record - 1) * 37 + 5) & 255
        lines.append(
            f"FG2_ROOT_REUSE_DISTANCE phase=correlation arm={arm} record={record} "
            f"iteration={record} seed={seed} root_a_va={roots[0]} root_b_va={roots[1]} "
            f"root_c_va={roots[2]} qmd_x_va={qmds[0]} qmd_y_va={qmds[1]} "
            f"qmd_z_va={qmds[2]} qmd_w_va={qmds[3]} selected_root={'ABC'[root_idx]} "
            f"previous_root_va={previous_root} selected_root_va={root} "
            f"root_reused={int(root_distance > 0)} root_reuse_distance={root_distance} "
            f"root_first_revisit={int(root_first)} selected_qmd={'XYZW'[qmd_idx]} "
            f"previous_qmd_va={previous_qmd} selected_qmd_va={qmd} "
            f"qmd_reused={int(qmd_distance > 0)} qmd_reuse_distance={qmd_distance} "
            f"qmd_first_revisit={int(qmd_first)} root_a_copy=1 root_b_copy=1 root_c_copy=1 "
            f"selected_root_copy=1 qmd_copy=1 decoded_root_va={root} root_decode=1 "
            f"outside_root_mask=1 qmd_cache_false=1 qmd_cache_bits=255:256 "
            f"pcas_pending_va={qmd} copy_before_dispatch=1 one_backing=1 footprint=7168 "
            "remaining=58368 gpu_visible=UNPROVEN fault_state=INSPECT_COMPLETE_STREAM")
        if record <= 5:
            for dword in range(0, 64, 8):
                lines.append(
                    f"FG2_ROOT_REUSE_DISTANCE phase=qmd_payload arm={arm} record={record} "
                    f"dword={dword} reference=0 selected=0 mapped=0")
        lines.append(
            f"FG2_ROOT_REUSE_DISTANCE phase=dispatch arm={arm} record={record} "
            f"selected_root_va={root} selected_qmd_va={qmd} pcas=0x0 "
            f"dispatch_qmd_va={qmd} address_match=1 order_match=1 "
            "fault_state=INSPECT_COMPLETE_STREAM")
        lines.append(
            f"FG2_ROOT_REUSE_DISTANCE phase=result arm={arm} record={record} "
            f"iteration={record} seed={seed} pixel=0x0 checksum=0x0 expected_pixel=0x0 "
            "expected_checksum=0x0 observed_behavior_seed=42 behavior_seed_known=1 "
            "oracle_match=0 mismatches=1 fault_state=INSPECT_COMPLETE_STREAM")
        root_last[root] = record
        qmd_last[qmd] = record
        previous_root, previous_qmd = root, qmd

    lines.append(
        f"FG2_ROOT_REUSE_DISTANCE phase=aggregate arm={arm} records=64 "
        f"root_schedule={root_schedule} qmd_schedule=X/Y/Z/W "
        f"root_first_revisit_iteration={root_first_record} "
        f"root_first_revisit_distance={root_first_distance} "
        "qmd_first_revisit_iteration=5 qmd_first_revisit_distance=4 "
        "root_a_copies=64/64 root_b_copies=64/64 root_c_copies=64/64 "
        "qmd_copies=64/64 root_decodes=64/64 outside_root_mask=64/64 "
        "dispatch_matches=64/64 footprint=7168 remaining=58368 one_backing=1 "
        "ownership_through_completion=1 ordering_complete=1 aggregate_valid=1 "
        "gpu_visible=UNPROVEN fault_state=NO_DRIVER_FAULT_REPORTED")
    lines.append(
        f"FG2_ROOT_REUSE_DISTANCE phase=oracle_aggregate arm={arm} validations=2/64 "
        "mismatched_iterations=62 fault_state=INSPECT_COMPLETE_STREAM")
    lines.append(
        f"FG2_ROOT_REUSE_DISTANCE phase=teardown arm={arm} queue_wait_complete=1 "
        "command_buffer_lifetime_complete=1 cleanup_begin=1 "
        "fault_state=INSPECT_COMPLETE_STREAM")
    lines.append(
        f"FG2_ROOT_REUSE_DISTANCE phase=teardown arm={arm} cleanup_complete=1 "
        "fault_state=INSPECT_COMPLETE_STREAM")
    return "\n".join(lines) + "\n"


def main() -> None:
    baseline = valid_stream("control")
    variant = valid_stream("variant")
    assert validator.validate(baseline, "control") == []
    assert validator.validate(variant, "variant") == []
    mutations = {
        "root_a_copy": ("root_a_copy=1", "root_a_copy=0"),
        "root_b_copy": ("root_b_copy=1", "root_b_copy=0"),
        "root_c_copy": ("root_c_copy=1", "root_c_copy=0"),
        "qmd_copy": ("qmd_copy=1", "qmd_copy=0"),
        "root_decode": ("root_decode=1", "root_decode=0"),
        "outside_mask": ("outside_root_mask=1", "outside_root_mask=0"),
        "cache_field": ("qmd_cache_false=1", "qmd_cache_false=0"),
        "unexpected_alias": ("root_c_va=0x00000000c7f41400", "root_c_va=0x00000000c7f40000"),
        "reuse_distance": ("root_reuse_distance=2", "root_reuse_distance=1"),
        "qmd_pcas": ("address_match=1", "address_match=0"),
        "ordering": ("copy_before_dispatch=1", "copy_before_dispatch=0"),
        "missing_record": ("record=64 iteration=64", "record=63 iteration=64"),
        "aggregate": ("aggregate_valid=1", "aggregate_valid=0"),
        "teardown": ("cleanup_complete=1", "cleanup_complete=0"),
        "fault_state": ("fault_state=INSPECT_COMPLETE_STREAM", "fault_state=AMBIGUOUS"),
    }
    for name, (old, new) in mutations.items():
        changed = baseline.replace(old, new, 1)
        assert changed != baseline
        assert validator.validate(changed, "control"), f"mutation accepted: {name}"
    assert validator.validate(baseline + "GPU fault detected\n", "control")
    print(f"PASS: {len(mutations) + 1} invalid mutations rejected; both schedules accepted")


if __name__ == "__main__":
    main()
