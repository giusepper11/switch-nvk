#!/usr/bin/env python3
"""Validate complete FG-2 QMD constant-cache experiment streams."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

PREFIX = "FG2_QMD_CONSTANT_CACHE"
FATAL_TEXT = re.compile(r"\b(timeout|gpu fault|error notifier|error-info)\b", re.I)


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

    correlations = by_phase.get("correlation", [])
    dispatches = by_phase.get("dispatch", [])
    results = by_phase.get("result", [])
    aggregates = by_phase.get("aggregate", [])
    oracle_aggregates = by_phase.get("oracle_aggregate", [])
    teardowns = by_phase.get("teardown", [])
    expected_field = "1" if arm == "invalidate" else "0"

    require(len(correlations) == 64, "correlation record count is not 64", errors)
    require(len(dispatches) == 64, "dispatch record count is not 64", errors)
    require(len(results) == 64, "result record count is not 64", errors)
    require(len(aggregates) == 1, "driver aggregate count is not 1", errors)
    require(len(oracle_aggregates) == 1, "oracle aggregate count is not 1", errors)
    require(len(teardowns) == 2, "teardown record count is not 2", errors)

    for record in range(1, 65):
        corr = next((x for x in correlations if x.get("record") == str(record)), None)
        disp = next((x for x in dispatches if x.get("record") == str(record)), None)
        result = next((x for x in results if x.get("record") == str(record)), None)
        require(corr is not None, f"missing correlation record {record}", errors)
        require(disp is not None, f"missing dispatch record {record}", errors)
        require(result is not None, f"missing result record {record}", errors)
        if corr:
            require(corr.get("path") == arm, f"record {record}: wrong arm", errors)
            require(corr.get("iteration") == str(record), f"record {record}: iteration mismatch", errors)
            require(corr.get("qmd_shader_constant_cache_invalidate") == expected_field,
                    f"record {record}: wrong field decode", errors)
            for key in ("qmd_constant_cache_field_match", "one_bit_xor_match",
                        "root_decode_match", "source_map_match", "order_match"):
                require(corr.get(key) == "1", f"record {record}: {key} failed", errors)
            require(corr.get("field_bits") == "255:256",
                    f"record {record}: unexpected QMD bit layout", errors)
            if record > 1:
                require(corr.get("root_fresh") == "1", f"record {record}: root transition failed", errors)
                require(corr.get("qmd_fresh") == "1", f"record {record}: QMD transition failed", errors)
        if disp:
            require(disp.get("path") == arm, f"dispatch {record}: wrong arm", errors)
            require(disp.get("address_match") == "1", f"dispatch {record}: PCAS mismatch", errors)
            require(disp.get("order_match") == "1", f"dispatch {record}: order mismatch", errors)
            require(disp.get("selected_qmd_gpu_va") == disp.get("dispatch_qmd_gpu_va"),
                    f"dispatch {record}: selected/dispatch VA mismatch", errors)
        if result:
            require(result.get("path") == arm, f"result {record}: wrong arm", errors)
            require(result.get("iteration") == str(record), f"result {record}: iteration mismatch", errors)
            require(result.get("fault_state") == "INSPECT_COMPLETE_STREAM",
                    f"result {record}: ambiguous fault state", errors)

    if len(aggregates) == 1:
        aggregate = aggregates[0]
        expected = {
            "path": arm, "records": "64", "root_transitions": "63/63",
            "qmd_transitions": "63/63", "root_copies": "64/64",
            "alternate_root_copies": "64/64", "qmd_copies": "64/64",
            "one_bit_comparisons": "64/64", "field_matches": "64/64",
            "root_decode_matches": "64/64", "dispatch_matches": "64/64",
            "footprint": "4608", "remaining": "60928", "ordering_complete": "1",
            "fault_state": "NO_DRIVER_FAULT_REPORTED",
        }
        for key, value in expected.items():
            require(aggregate.get(key) == value, f"aggregate: {key} is not {value}", errors)

    if len(oracle_aggregates) == 1:
        require(oracle_aggregates[0].get("path") == arm, "oracle aggregate: wrong arm", errors)
        require(oracle_aggregates[0].get("fault_state") == "INSPECT_COMPLETE_STREAM",
                "oracle aggregate: ambiguous fault state", errors)

    if len(teardowns) == 2:
        require(teardowns[0].get("queue_wait_complete") == "1", "teardown: queue wait incomplete", errors)
        require(teardowns[0].get("command_buffer_lifetime_complete") == "1",
                "teardown: command-buffer lifetime incomplete", errors)
        require(teardowns[0].get("cleanup_begin") == "1", "teardown: cleanup did not begin", errors)
        require(teardowns[1].get("cleanup_complete") == "1", "teardown: cleanup incomplete", errors)
        for item in teardowns:
            require(item.get("path") == arm, "teardown: wrong arm", errors)
            require(item.get("fault_state") == "INSPECT_COMPLETE_STREAM",
                    "teardown: ambiguous fault state", errors)

    unrelated = "\n".join(line for line in text.splitlines() if PREFIX not in line)
    require(FATAL_TEXT.search(unrelated) is None, "complete stream contains a fault/timeout marker", errors)
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--arm", choices=("control", "invalidate"), required=True)
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
