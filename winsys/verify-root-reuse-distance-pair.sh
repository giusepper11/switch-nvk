#!/usr/bin/env bash
# Source/artifact semantic review for the same-source root-reuse-distance pair.
set -eu

control=${1:-nvk_render_compute_root_reuse_distance_control.nro}
variant=${2:-nvk_render_compute_root_reuse_distance_variant.nro}
test -f "$control"
test -f "$variant"

source_file=mesa-25/src/nouveau/vulkan/nvk_cmd_dispatch.c
harness=winsys/smoke/nvk_render_compute.c

test "$(rg -c 'const uint32_t root_ring_size =' "$source_file")" -eq 1
test "$(rg -c 'root_reuse_distance_variant_enabled \? 3 : 2' "$source_file")" -eq 2
test "$(rg -c 'const uint32_t selected_qmd = \(trace.record - 1\) % 4' "$source_file")" -eq 1
test "$(rg -c 'root_a_match.*root_b_match.*root_c_match' "$source_file")" -ge 1
rg -q 'memcmp\(root, primary_root_map' "$source_file"
rg -q 'memcmp\(root, alternate_root_map' "$source_file"
rg -q 'memcmp\(root, third_root_map' "$source_file"
rg -q 'qmd_info.invalidate_shader_constant_cache =' "$source_file"
rg -q 'constant_cache_experiment &&' "$source_file"
rg -q 'qmd_cache_false=%u' "$source_file"
rg -q 'device_work_started=0' "$harness"

strings "$control" | rg -q 'chain2-root-reuse-distance-control'
strings "$variant" | rg -q 'chain2-root-reuse-distance-variant'
strings "$control" | rg -q 'root_schedule=%s qmd_schedule=X/Y/Z/W'
strings "$variant" | rg -q 'root_schedule=%s qmd_schedule=X/Y/Z/W'

for shader in render_compute.vert render_compute.frag render_compute.comp; do
  test -f "winsys/smoke/shaders/$shader"
done

echo "root-reuse-distance source/artifact semantic review PASS"
