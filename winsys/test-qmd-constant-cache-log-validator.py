#!/usr/bin/env python3
"""Mutation tests for validate-qmd-constant-cache-log.py."""

from __future__ import annotations

import importlib.util
from pathlib import Path

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location(
    "validator", HERE / "validate-qmd-constant-cache-log.py")
assert SPEC and SPEC.loader
validator = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(validator)


def valid_stream() -> str:
    lines: list[str] = []
    previous_root = "0x0000000000000000"
    previous_qmd = "0x0000000000000000"
    for record in range(1, 65):
        root = "0x00000000c7f40000" if record % 2 else "0x00000000c7f40a00"
        qmd = "0x00000000c7f40800" if record % 2 else "0x00000000c7f40900"
        fresh = "0" if record == 1 else "1"
        lines.append(
            f"FG2_QMD_CONSTANT_CACHE phase=correlation path=control record={record} "
            f"iteration={record} seed={(record * 37 + 5) & 255} "
            "qmd_shader_constant_cache_invalidate=0 qmd_constant_cache_field_match=1 "
            "one_bit_xor_match=1 field_bits=255:256 root_decode_match=1 "
            f"root_gpu_va={root} previous_root_gpu_va={previous_root} root_fresh={fresh} "
            f"selected_qmd_gpu_va={qmd} previous_qmd_gpu_va={previous_qmd} qmd_fresh={fresh} "
            "source_map_match=1 order_match=1 gpu_visible=UNPROVEN")
        lines.append(
            f"FG2_QMD_CONSTANT_CACHE phase=dispatch path=control record={record} "
            f"selected_root_gpu_va={root} selected_qmd_gpu_va={qmd} pcas=0x00c7f408 "
            f"dispatch_qmd_gpu_va={qmd} address_match=1 order_match=1")
        lines.append(
            f"FG2_QMD_CONSTANT_CACHE phase=result path=control record={record} iteration={record} "
            f"seed={(record * 37 + 5) & 255} pixel=0x0 checksum=0x0 expected_pixel=0x0 "
            "expected_checksum=0x0 oracle_match=0 mismatches=1 fault_state=INSPECT_COMPLETE_STREAM")
        previous_root, previous_qmd = root, qmd
    lines.append(
        "FG2_QMD_CONSTANT_CACHE phase=aggregate path=control records=64 "
        "root_transitions=63/63 qmd_transitions=63/63 root_copies=64/64 "
        "alternate_root_copies=64/64 qmd_copies=64/64 one_bit_comparisons=64/64 "
        "field_matches=64/64 root_decode_matches=64/64 dispatch_matches=64/64 "
        "footprint=4608 remaining=60928 ordering_complete=1 "
        "fault_state=NO_DRIVER_FAULT_REPORTED gpu_visible=UNPROVEN")
    lines.append(
        "FG2_QMD_CONSTANT_CACHE phase=oracle_aggregate path=control validations=2/64 "
        "mismatched_iterations=62 fault_state=INSPECT_COMPLETE_STREAM")
    lines.append(
        "FG2_QMD_CONSTANT_CACHE phase=teardown path=control queue_wait_complete=1 "
        "command_buffer_lifetime_complete=1 cleanup_begin=1 fault_state=INSPECT_COMPLETE_STREAM")
    lines.append(
        "FG2_QMD_CONSTANT_CACHE phase=teardown path=control cleanup_complete=1 "
        "fault_state=INSPECT_COMPLETE_STREAM")
    return "\n".join(lines) + "\n"


def main() -> None:
    baseline = valid_stream()
    assert validator.validate(baseline, "control") == []
    mutations = {
        "unexpected_bit": ("field_bits=255:256", "field_bits=254:256"),
        "field_decode": ("qmd_shader_constant_cache_invalidate=0", "qmd_shader_constant_cache_invalidate=1"),
        "root_decode": ("root_decode_match=1", "root_decode_match=0"),
        "mapped_copy": ("source_map_match=1", "source_map_match=0"),
        "root_transition": ("root_fresh=1", "root_fresh=0"),
        "qmd_transition": ("qmd_fresh=1", "qmd_fresh=0"),
        "pcas": ("address_match=1", "address_match=0"),
        "record_order": ("order_match=1", "order_match=0"),
        "aggregate": ("records=64", "records=63"),
        "teardown": ("cleanup_complete=1", "cleanup_complete=0"),
        "fault": ("fault_state=INSPECT_COMPLETE_STREAM", "fault_state=AMBIGUOUS"),
    }
    for name, (old, new) in mutations.items():
        changed = baseline.replace(old, new, 1)
        assert changed != baseline
        assert validator.validate(changed, "control"), f"mutation accepted: {name}"
    assert validator.validate(baseline + "GPU fault detected\n", "control")
    print(f"PASS: {len(mutations) + 1} invalid mutations rejected")


if __name__ == "__main__":
    main()
