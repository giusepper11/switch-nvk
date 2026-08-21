#!/usr/bin/env bash
# Build the same-source FG-2 root-address-reuse-distance control/variant pair.
set -e

APP=nvk_render_compute \
OUTPUT=nvk_render_compute_root_reuse_distance_control \
TITLE="NVK Root Reuse Control" \
VERSION="0.63.0-root-reuse-control" \
ROOT_ADDRESS_REUSE_DISTANCE_CONTROL=1 ROOT_DIAG_LIMIT=64 \
bash winsys/build-nro.sh

APP=nvk_render_compute \
OUTPUT=nvk_render_compute_root_reuse_distance_variant \
TITLE="NVK Root Reuse Variant" \
VERSION="0.63.0-root-reuse-variant" \
ROOT_ADDRESS_REUSE_DISTANCE_VARIANT=1 ROOT_DIAG_LIMIT=64 \
bash winsys/build-nro.sh
