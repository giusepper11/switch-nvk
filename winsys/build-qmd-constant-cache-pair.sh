#!/usr/bin/env bash
set -e

APP=nvk_render_compute \
OUTPUT=nvk_render_compute_qmd_constant_cache_control \
TITLE="NVK FG2 QMD Constant Cache Control" \
VERSION="0.62.0-qmd-constant-cache-control" \
QMD_SHADER_CONSTANT_CACHE_CONTROL=1 \
bash winsys/build-nro.sh

APP=nvk_render_compute \
OUTPUT=nvk_render_compute_qmd_constant_cache_invalidate \
TITLE="NVK FG2 QMD Constant Cache Invalidate" \
VERSION="0.62.0-qmd-constant-cache-invalidate" \
QMD_SHADER_CONSTANT_CACHE_INVALIDATE=1 \
bash winsys/build-nro.sh
