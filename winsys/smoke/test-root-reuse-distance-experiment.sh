#!/usr/bin/env bash
# Deterministic host checks for root-reuse-distance selection, layout, and schedules.
set -eu

fail_selector() {
  if "$@" >/dev/null 2>&1; then
    echo "expected selector failure: $*" >&2
    exit 1
  else
    selector_status=$?
    test "$selector_status" -eq 2
  fi
}

fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=x bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=2 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 ROOT_UPLOAD_CACHE_FLUSH=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1 QMD_UPLOAD_IDENTITY=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 QMD_UPLOAD_IDENTITY=1 QMD_UPLOAD_CACHE_FLUSH=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1 QMD_ADDRESS_CONTROL=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 QMD_ADDRESS_FRESH=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1 ROOT_ADDRESS_CONTROL=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 ROOT_ADDRESS_FRESH=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1 QMD_SHADER_CONSTANT_CACHE_CONTROL=1 bash winsys/build-nro.sh
fail_selector env ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 QMD_SHADER_CONSTANT_CACHE_INVALIDATE=1 bash winsys/build-nro.sh

root_size=$((0x800))
qmd_size=$((0x100))
upload_size=$((0x10000))
root_a=0
qmd_x=$((root_a + root_size))
qmd_y=$((qmd_x + qmd_size))
root_b=$((qmd_y + qmd_size))
qmd_z=$((root_b + root_size))
qmd_w=$((qmd_z + qmd_size))
root_c=$((qmd_w + qmd_size))
footprint=$((root_c + root_size))
remaining=$((upload_size - footprint))
test "$qmd_x" -eq $((0x800))
test "$qmd_y" -eq $((0x900))
test "$root_b" -eq $((0xa00))
test "$qmd_z" -eq $((0x1200))
test "$qmd_w" -eq $((0x1300))
test "$root_c" -eq $((0x1400))
test "$footprint" -eq 7168
test "$remaining" -eq 58368

first_revisit() {
  ring=$1
  expected_record=$2
  expected_distance=$3
  seen_a=0
  seen_b=0
  seen_c=0
  previous_a=0
  previous_b=0
  previous_c=0
  record=1
  found_record=0
  found_distance=0
  while test "$record" -le 64; do
    case "$ring:$(((record - 1) % ring))" in
      *:0) va=$((0xc7f40000)); seen=$seen_a; last=$previous_a ;;
      *:1) va=$((0xc7f40a00)); seen=$seen_b; last=$previous_b ;;
      3:2) va=$((0xc7f41400)); seen=$seen_c; last=$previous_c ;;
      *) exit 1 ;;
    esac
    if test "$seen" -eq 1 && test "$found_record" -eq 0; then
      found_record=$record
      found_distance=$((record - last))
    fi
    case "$va" in
      $((0xc7f40000))) seen_a=1; previous_a=$record ;;
      $((0xc7f40a00))) seen_b=1; previous_b=$record ;;
      $((0xc7f41400))) seen_c=1; previous_c=$record ;;
      *) exit 1 ;;
    esac
    record=$((record + 1))
  done
  test "$found_record" -eq "$expected_record"
  test "$found_distance" -eq "$expected_distance"
}

first_revisit 2 3 2
first_revisit 3 4 3

qmd_seen_x=0
qmd_seen_y=0
qmd_seen_z=0
qmd_seen_w=0
qmd_first_record=0
qmd_first_distance=0
record=1
while test "$record" -le 64; do
  slot=$(((record - 1) % 4))
  case "$slot" in
    0) seen=$qmd_seen_x; last=${qmd_last_x:-0}; qmd_seen_x=1; qmd_last_x=$record ;;
    1) seen=$qmd_seen_y; last=${qmd_last_y:-0}; qmd_seen_y=1; qmd_last_y=$record ;;
    2) seen=$qmd_seen_z; last=${qmd_last_z:-0}; qmd_seen_z=1; qmd_last_z=$record ;;
    3) seen=$qmd_seen_w; last=${qmd_last_w:-0}; qmd_seen_w=1; qmd_last_w=$record ;;
  esac
  if test "$seen" -eq 1 && test "$qmd_first_record" -eq 0; then
    qmd_first_record=$record
    qmd_first_distance=$((record - last))
  fi
  record=$((record + 1))
done
test "$qmd_first_record" -eq 5
test "$qmd_first_distance" -eq 4

for failed_gate in root_a_copy root_b_copy root_c_copy qmd_copy root_decode outside_mask \
                   schedule reuse_history qmd_pcas order aggregate teardown fault_state; do
  causal_valid=1
  for gate in root_a_copy root_b_copy root_c_copy qmd_copy root_decode outside_mask \
              schedule reuse_history qmd_pcas order aggregate teardown fault_state; do
    test "$gate" = "$failed_gate" && causal_valid=0
  done
  test "$causal_valid" -eq 0
done

echo "root-reuse-distance host model PASS footprint=7168 remaining=58368 control_root_first=3/2 variant_root_first=4/3 qmd_first=5/4"
