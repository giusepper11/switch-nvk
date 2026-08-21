#!/usr/bin/env bash
# Build the same-source FG-2 compute-root address control/fresh artifact pair.
set -e

APP=nvk_render_compute \
OUTPUT=nvk_render_compute_root_address_control \
TITLE="NVK Root Address Control" \
VERSION="0.61.0-root-address-control" \
ROOT_ADDRESS_CONTROL=1 ROOT_DIAG_LIMIT=64 \
bash winsys/build-nro.sh

APP=nvk_render_compute \
OUTPUT=nvk_render_compute_root_address_fresh \
TITLE="NVK Root Address Fresh" \
VERSION="0.61.0-root-address-fresh" \
ROOT_ADDRESS_FRESH=1 ROOT_DIAG_LIMIT=64 \
bash winsys/build-nro.sh
