#!/usr/bin/env python3
"""Validate complete FG-2 compute-root reuse-distance experiment streams."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

PREFIX = "FG2_ROOT_REUSE_DISTANCE"
FATAL_TEXT = re.compile(r"\b(timeout|errnotif|errinfo|gpu fault|assertion failed)\b", re.I)


def fields(line: str) -> dict[str, str]:
    return dict(re.findall(r"([A-Za-z0-9_]+)=([^\s]+)", line))


def require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def validate(text: str, arm: str) -> list[str]:
    errors: list[str] = []
    tagged = [line for line in text.splitlines() if PREFIX in line]
    by_phase: dict[str, list[dict[str, str]]] = {}
    for line in tagged:
        item = fields(line)
        by_phase.setdefault(item.get("phase", ""), []).append(item)

    arms = by_phase.get("arm", [])
    allocations = by_phase.get("allocation", [])
    correlations = by_phase.get("correlation", [])
    payloads = by_phase.get("qmd_payload", [])
    dispatches = by_phase.get("dispatch", [])
    results = by_phase.get("result", [])
    aggregates = by_phase.get("aggregate", [])
    oracle_aggregates = by_phase.get("oracle_aggregate", [])
    teardowns = by_phase.get("teardown", [])

    require(len(arms) == 1, "pre-device arm marker count is not 1", errors)
    require(len(allocations) == 1, "allocation record count is not 1", errors)
    require(len(correlations) == 64, "correlation record count is not 64", errors)
    require(len(payloads) == 40, "detailed QMD payload line count is not 40", errors)
    require(len(dispatches) == 64, "dispatch record count is not 64", errors)
    require(len(results) == 64, "result record count is not 64", errors)
    require(len(aggregates) == 1, "driver aggregate count is not 1", errors)
    require(len(oracle_aggregates) == 1, "oracle aggregate count is not 1", errors)
    require(len(teardowns) == 2, "teardown record count is not 2", errors)

    expected_root_schedule = "A/B/C" if arm == "variant" else "A/B"
    expected_root_first = 4 if arm == "variant" else 3
    expected_root_distance = 3 if arm == "variant" else 2

    if len(arms) == 1:
        marker = arms[0]
        require(marker.get("arm") == arm, "pre-device marker: wrong arm", errors)
        require(marker.get("root_schedule") == expected_root_schedule,
                "pre-device marker: wrong root schedule", errors)
        require(marker.get("qmd_schedule") == "X/Y/Z/W",
                "pre-device marker: wrong QMD schedule", errors)
        require(marker.get("first_root_revisit") == str(expected_root_first),
                "pre-device marker: wrong first root revisit", errors)
        require(marker.get("first_qmd_revisit") == "5",
                "pre-device marker: wrong first QMD revisit", errors)
        require(marker.get("device_work_started") == "0",
                "pre-device marker occurred after device work", errors)

    root_vas: list[str] = []
    qmd_vas: list[str] = []
    if len(allocations) == 1:
        allocation = allocations[0]
        require(allocation.get("arm") == arm, "allocation: wrong arm", errors)
        expected = {
            "root_a_offset": "0", "qmd_x_offset": "2048",
            "qmd_y_offset": "2304", "root_b_offset": "2560",
            "qmd_z_offset": "4608", "qmd_w_offset": "4864",
            "root_c_offset": "5120", "root_size": "2048",
            "qmd_size": "256", "root_alignment": "256",
            "qmd_alignment": "256", "footprint": "7168",
            "remaining": "58368", "one_backing": "1",
            "later_consumer_displacement": "0", "gpu_visible": "UNPROVEN",
        }
        for key, value in expected.items():
            require(allocation.get(key) == value,
                    f"allocation: {key} is not {value}", errors)
        require(allocation.get("ownership") == "command_buffer_through_completion",
                "allocation: ownership mismatch", errors)
        require(allocation.get("cleanup") == "existing_pool_path",
                "allocation: cleanup path mismatch", errors)
        require(allocation.get("nvmap_identity") == "same_shim_bo_handle",
                "allocation: NvMap/BO identity is ambiguous", errors)
        root_vas = [allocation.get(key, "") for key in
                    ("root_a_va", "root_b_va", "root_c_va")]
        qmd_vas = [allocation.get(key, "") for key in
                   ("qmd_x_va", "qmd_y_va", "qmd_z_va", "qmd_w_va")]
        require(all(root_vas) and len(set(root_vas)) == 3,
                "allocation: root VAs are missing or aliased", errors)
        require(all(qmd_vas) and len(set(qmd_vas)) == 4,
                "allocation: QMD VAs are missing or aliased", errors)

    root_last: dict[str, int] = {}
    qmd_last: dict[str, int] = {}
    root_revisited: set[str] = set()
    qmd_revisited: set[str] = set()
    derived_root_first = 0
    derived_root_distance = 0
    derived_qmd_first = 0
    derived_qmd_distance = 0
    root_ring_size = 3 if arm == "variant" else 2
    root_names = "ABC"
    qmd_names = "XYZW"

    for record in range(1, 65):
        corr = next((x for x in correlations if x.get("record") == str(record)), None)
        disp = next((x for x in dispatches if x.get("record") == str(record)), None)
        result = next((x for x in results if x.get("record") == str(record)), None)
        require(corr is not None, f"missing correlation record {record}", errors)
        require(disp is not None, f"missing dispatch record {record}", errors)
        require(result is not None, f"missing result record {record}", errors)
        if not corr or not root_vas or not qmd_vas:
            continue

        expected_seed = ((record - 1) * 37 + 5) & 255
        root_idx = (record - 1) % root_ring_size
        qmd_idx = (record - 1) % 4
        root_va = root_vas[root_idx]
        qmd_va = qmd_vas[qmd_idx]
        root_distance = record - root_last[root_va] if root_va in root_last else 0
        qmd_distance = record - qmd_last[qmd_va] if qmd_va in qmd_last else 0
        root_first = root_va in root_last and root_va not in root_revisited
        qmd_first = qmd_va in qmd_last and qmd_va not in qmd_revisited
        if root_first:
            root_revisited.add(root_va)
            if derived_root_first == 0:
                derived_root_first, derived_root_distance = record, root_distance
        if qmd_first:
            qmd_revisited.add(qmd_va)
            if derived_qmd_first == 0:
                derived_qmd_first, derived_qmd_distance = record, qmd_distance

        require(corr.get("arm") == arm, f"record {record}: wrong arm", errors)
        require(corr.get("iteration") == str(record), f"record {record}: iteration mismatch", errors)
        require(corr.get("seed") == str(expected_seed), f"record {record}: seed mismatch", errors)
        require(corr.get("selected_root") == root_names[root_idx],
                f"record {record}: wrong selected root label", errors)
        require(corr.get("selected_root_va") == root_va,
                f"record {record}: selected root VA violates actual-VA schedule", errors)
        require(corr.get("selected_qmd") == qmd_names[qmd_idx],
                f"record {record}: wrong selected QMD label", errors)
        require(corr.get("selected_qmd_va") == qmd_va,
                f"record {record}: selected QMD VA violates actual-VA schedule", errors)
        require(corr.get("root_reused") == str(int(root_distance > 0)),
                f"record {record}: root reuse mismatch", errors)
        require(corr.get("root_reuse_distance") == str(root_distance),
                f"record {record}: root distance mismatch", errors)
        require(corr.get("root_first_revisit") == str(int(root_first)),
                f"record {record}: root first-revisit mismatch", errors)
        require(corr.get("qmd_reused") == str(int(qmd_distance > 0)),
                f"record {record}: QMD reuse mismatch", errors)
        require(corr.get("qmd_reuse_distance") == str(qmd_distance),
                f"record {record}: QMD distance mismatch", errors)
        require(corr.get("qmd_first_revisit") == str(int(qmd_first)),
                f"record {record}: QMD first-revisit mismatch", errors)
        for key in ("root_a_copy", "root_b_copy", "root_c_copy",
                    "selected_root_copy", "qmd_copy", "root_decode",
                    "outside_root_mask", "qmd_cache_false",
                    "copy_before_dispatch", "one_backing"):
            require(corr.get(key) == "1", f"record {record}: {key} failed", errors)
        require(corr.get("qmd_cache_bits") == "255:256",
                f"record {record}: unexpected cache-field layout", errors)
        require(corr.get("decoded_root_va") == root_va,
                f"record {record}: decoded root VA mismatch", errors)
        require(corr.get("pcas_pending_va") == qmd_va,
                f"record {record}: pending PCAS VA mismatch", errors)
        require(corr.get("gpu_visible") == "UNPROVEN",
                f"record {record}: CPU equality promoted improperly", errors)
        require(corr.get("fault_state") == "INSPECT_COMPLETE_STREAM",
                f"record {record}: ambiguous correlation fault state", errors)

        if disp:
            require(disp.get("arm") == arm, f"dispatch {record}: wrong arm", errors)
            require(disp.get("selected_root_va") == root_va,
                    f"dispatch {record}: root VA mismatch", errors)
            require(disp.get("selected_qmd_va") == qmd_va,
                    f"dispatch {record}: QMD VA mismatch", errors)
            require(disp.get("dispatch_qmd_va") == qmd_va,
                    f"dispatch {record}: selected/PCAS VA mismatch", errors)
            require(disp.get("address_match") == "1", f"dispatch {record}: PCAS mismatch", errors)
            require(disp.get("order_match") == "1", f"dispatch {record}: order mismatch", errors)
        if result:
            require(result.get("arm") == arm, f"result {record}: wrong arm", errors)
            require(result.get("iteration") == str(record), f"result {record}: iteration mismatch", errors)
            require(result.get("seed") == str(expected_seed), f"result {record}: seed mismatch", errors)
            require(result.get("fault_state") == "INSPECT_COMPLETE_STREAM",
                    f"result {record}: ambiguous fault state", errors)
        root_last[root_va] = record
        qmd_last[qmd_va] = record

    require((derived_root_first, derived_root_distance) ==
            (expected_root_first, expected_root_distance),
            "derived root first-revisit boundary is wrong", errors)
    require((derived_qmd_first, derived_qmd_distance) == (5, 4),
            "derived QMD first-revisit boundary is wrong", errors)

    payload_keys = {(item.get("record"), item.get("dword")) for item in payloads}
    expected_payload_keys = {(str(record), str(dword))
                             for record in range(1, 6)
                             for dword in range(0, 64, 8)}
    require(payload_keys == expected_payload_keys,
            "detailed payload coverage for records 1-5 is incomplete", errors)

    if len(aggregates) == 1:
        aggregate = aggregates[0]
        expected = {
            "arm": arm, "records": "64", "root_schedule": expected_root_schedule,
            "qmd_schedule": "X/Y/Z/W",
            "root_first_revisit_iteration": str(expected_root_first),
            "root_first_revisit_distance": str(expected_root_distance),
            "qmd_first_revisit_iteration": "5", "qmd_first_revisit_distance": "4",
            "root_a_copies": "64/64", "root_b_copies": "64/64",
            "root_c_copies": "64/64", "qmd_copies": "64/64",
            "root_decodes": "64/64", "outside_root_mask": "64/64",
            "dispatch_matches": "64/64", "footprint": "7168",
            "remaining": "58368", "one_backing": "1",
            "ownership_through_completion": "1", "ordering_complete": "1",
            "aggregate_valid": "1", "gpu_visible": "UNPROVEN",
            "fault_state": "NO_DRIVER_FAULT_REPORTED",
        }
        for key, value in expected.items():
            require(aggregate.get(key) == value,
                    f"aggregate: {key} is not {value}", errors)

    if len(oracle_aggregates) == 1:
        require(oracle_aggregates[0].get("arm") == arm,
                "oracle aggregate: wrong arm", errors)
        require(oracle_aggregates[0].get("fault_state") == "INSPECT_COMPLETE_STREAM",
                "oracle aggregate: ambiguous fault state", errors)

    if len(teardowns) == 2:
        require(teardowns[0].get("queue_wait_complete") == "1",
                "teardown: queue wait incomplete", errors)
        require(teardowns[0].get("command_buffer_lifetime_complete") == "1",
                "teardown: command-buffer lifetime incomplete", errors)
        require(teardowns[0].get("cleanup_begin") == "1",
                "teardown: cleanup did not begin", errors)
        require(teardowns[1].get("cleanup_complete") == "1",
                "teardown: cleanup incomplete", errors)
        for item in teardowns:
            require(item.get("arm") == arm, "teardown: wrong arm", errors)
            require(item.get("fault_state") == "INSPECT_COMPLETE_STREAM",
                    "teardown: ambiguous fault state", errors)

    unrelated = "\n".join(line for line in text.splitlines() if PREFIX not in line)
    require(FATAL_TEXT.search(unrelated) is None,
            "complete stream contains a fault/timeout marker", errors)
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--arm", choices=("control", "variant"), required=True)
    parser.add_argument("log", type=Path)
    args = parser.parse_args()
    errors = validate(args.log.read_text(errors="replace"), args.arm)
    if errors:
        for error in errors:
            print(f"INVALID: {error}", file=sys.stderr)
        return 2
    print(f"VALID: arm={args.arm} records=64 causal_evidence_complete=1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
