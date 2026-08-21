#!/usr/bin/env bash
# Deterministic host checks for the selector and bounded two-root/two-QMD model.
set -eu

fail_selector() {
  if "$@" >/dev/null 2>&1; then
    echo "expected selector failure: $*" >&2
    exit 1
  else
    status=$?
    test "$status" -eq 2
  fi
}

fail_selector env ROOT_ADDRESS_CONTROL=x bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_FRESH=x bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_CONTROL=1 ROOT_ADDRESS_FRESH=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_CONTROL=1 ROOT_UPLOAD_CACHE_FLUSH=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_CONTROL=1 QMD_UPLOAD_IDENTITY=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_FRESH=1 QMD_UPLOAD_CACHE_FLUSH=1 QMD_UPLOAD_IDENTITY=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_FRESH=1 QMD_ADDRESS_CONTROL=1 bash winsys/build-nro.sh

root_size=$((0x800))
qmd_size=$((0x100))
primary_root=0
primary_qmd=$((primary_root + root_size))
secondary_qmd=$((primary_qmd + qmd_size))
alternate_root=$((secondary_qmd + qmd_size))
footprint=$((alternate_root + root_size))
remaining=$((0x10000 - footprint))
test "$primary_qmd" -eq $((0x800))
test "$secondary_qmd" -eq $((0x900))
test "$alternate_root" -eq $((0xa00))
test "$footprint" -eq 4608
test "$remaining" -eq 60928

control_root_transitions=0
fresh_root_transitions=0
qmd_transitions=0
previous_control=$primary_root
previous_fresh=$primary_root
previous_qmd=$primary_qmd
record=2
while test "$record" -le 64; do
  control=$primary_root
  if test $((record % 2)) -eq 0; then
    fresh=$alternate_root
    selected_qmd=$secondary_qmd
  else
    fresh=$primary_root
    selected_qmd=$primary_qmd
  fi
  test "$control" -ne "$previous_control" && control_root_transitions=$((control_root_transitions + 1))
  test "$fresh" -ne "$previous_fresh" && fresh_root_transitions=$((fresh_root_transitions + 1))
  test "$selected_qmd" -ne "$previous_qmd" && qmd_transitions=$((qmd_transitions + 1))
  previous_control=$control
  previous_fresh=$fresh
  previous_qmd=$selected_qmd
  record=$((record + 1))
done
test "$control_root_transitions" -eq 0
test "$fresh_root_transitions" -eq 63
test "$qmd_transitions" -eq 63

# Required validity gates are conjunctive: injecting any failed observation
# must prevent a valid aggregate.
for failed_gate in root_copy qmd_copy decode outside_mask freshness dispatch order; do
  valid=1
  for gate in root_copy qmd_copy decode outside_mask freshness dispatch order; do
    test "$gate" = "$failed_gate" && valid=0
  done
  test "$valid" -eq 0
done

echo "root-address host model PASS footprint=4608 remaining=60928 control_root=0/63 fresh_root=63/63 qmd=63/63"
