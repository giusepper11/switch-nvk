#!/usr/bin/env bash
# Build the headless NVK smoke-test .nro for Sphaira (full Application mode).
#   docker run --rm -v "D:\switch-nvk:/work" -w /work switch-nvk-build bash winsys/build-nro.sh
# Output: /work/nvk_smoke.nro  (=> D:\switch-nvk\nvk_smoke.nro)
set -e

DKP=/opt/devkitpro
GCC=$DKP/devkitA64/bin/aarch64-none-elf-gcc
GXX=$DKP/devkitA64/bin/aarch64-none-elf-g++
STRIP=$DKP/devkitA64/bin/aarch64-none-elf-strip
MB=/work/mb
OBJ=/tmp/nvk-nro
mkdir -p "$OBJ"

# Which app to build (default = the passing M2 smoke). Override e.g.:
#   APP=nvk_tri TITLE="NVK Tri" VERSION="0.33.0-tri1a" bash winsys/build-nro.sh
# FG-1 compute proof:
#   APP=nvk_compute TITLE="NVK Compute" VERSION="0.60.0-compute1" bash winsys/build-nro.sh
# FG-2 render -> sampled image -> compute -> storage image proof:
#   APP=nvk_render_compute TITLE="NVK Render Compute" VERSION="0.61.0-chain1" bash winsys/build-nro.sh
APP="${APP:-nvk_smoke}"
OUTPUT="${OUTPUT:-$APP}"
TITLE="${TITLE:-NVK Smoke}"
VERSION="${VERSION:-0.32.0-fencecmdlist}"
ROOT_DIAG_LIMIT="${ROOT_DIAG_LIMIT:-0}"
ROOT_UPLOAD_CACHE_FLUSH="${ROOT_UPLOAD_CACHE_FLUSH:-0}"
QMD_UPLOAD_IDENTITY="${QMD_UPLOAD_IDENTITY:-0}"
QMD_UPLOAD_CACHE_FLUSH="${QMD_UPLOAD_CACHE_FLUSH:-0}"
QMD_ADDRESS_CONTROL="${QMD_ADDRESS_CONTROL:-0}"
QMD_ADDRESS_FRESH="${QMD_ADDRESS_FRESH:-0}"
ROOT_ADDRESS_CONTROL="${ROOT_ADDRESS_CONTROL:-0}"
ROOT_ADDRESS_FRESH="${ROOT_ADDRESS_FRESH:-0}"
case "$ROOT_DIAG_LIMIT" in
  0|1|2|3|64) ;;
  *) echo "ROOT_DIAG_LIMIT must be 0, 1, 2, 3, or 64" >&2; exit 2 ;;
esac
case "$ROOT_UPLOAD_CACHE_FLUSH" in
  0|1) ;;
  *) echo "ROOT_UPLOAD_CACHE_FLUSH must be 0 or 1" >&2; exit 2 ;;
esac
case "$QMD_UPLOAD_IDENTITY" in
  0|1) ;;
  *) echo "QMD_UPLOAD_IDENTITY must be 0 or 1" >&2; exit 2 ;;
esac
case "$QMD_UPLOAD_CACHE_FLUSH" in
  0|1) ;;
  *) echo "QMD_UPLOAD_CACHE_FLUSH must be 0 or 1" >&2; exit 2 ;;
esac
if [ "$QMD_UPLOAD_CACHE_FLUSH" = 1 ] && [ "$QMD_UPLOAD_IDENTITY" != 1 ]; then
  echo "QMD_UPLOAD_CACHE_FLUSH=1 requires QMD_UPLOAD_IDENTITY=1" >&2
  exit 2
fi
case "$QMD_ADDRESS_CONTROL" in 0|1) ;; *) echo "QMD_ADDRESS_CONTROL must be 0 or 1" >&2; exit 2 ;; esac
case "$QMD_ADDRESS_FRESH" in 0|1) ;; *) echo "QMD_ADDRESS_FRESH must be 0 or 1" >&2; exit 2 ;; esac
if [ "$QMD_ADDRESS_CONTROL" = 1 ] && [ "$QMD_ADDRESS_FRESH" = 1 ]; then
  echo "QMD address selectors are mutually exclusive" >&2; exit 2
fi
if { [ "$QMD_ADDRESS_CONTROL" = 1 ] || [ "$QMD_ADDRESS_FRESH" = 1 ]; } &&
   { [ "$QMD_UPLOAD_CACHE_FLUSH" = 1 ] || [ "$ROOT_UPLOAD_CACHE_FLUSH" = 1 ]; }; then
  echo "QMD address experiment requires all cache selectors disabled" >&2; exit 2
fi
case "$ROOT_ADDRESS_CONTROL" in 0|1) ;; *) echo "ROOT_ADDRESS_CONTROL must be 0 or 1" >&2; exit 2 ;; esac
case "$ROOT_ADDRESS_FRESH" in 0|1) ;; *) echo "ROOT_ADDRESS_FRESH must be 0 or 1" >&2; exit 2 ;; esac
if [ "$ROOT_ADDRESS_CONTROL" = 1 ] && [ "$ROOT_ADDRESS_FRESH" = 1 ]; then
  echo "root address selectors are mutually exclusive" >&2; exit 2
fi
if { [ "$ROOT_ADDRESS_CONTROL" = 1 ] || [ "$ROOT_ADDRESS_FRESH" = 1 ]; } &&
   { [ "$ROOT_UPLOAD_CACHE_FLUSH" = 1 ] || [ "$QMD_UPLOAD_IDENTITY" = 1 ] ||
     [ "$QMD_UPLOAD_CACHE_FLUSH" = 1 ] || [ "$QMD_ADDRESS_CONTROL" = 1 ] ||
     [ "$QMD_ADDRESS_FRESH" = 1 ]; }; then
  echo "root address experiment excludes root-cache and QMD experiments" >&2; exit 2
fi
echo "=== building app=$APP -> /work/$OUTPUT.nro (title='$TITLE' ver=$VERSION) ==="

ARCH="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"
INC="-I$DKP/libnx/include -I/opt/switch-cross-include -Imesa-25/include -Imesa-25/src/nouveau/drm -Iwinsys -Icompat"
DEFS="-D__SWITCH__ -D_GNU_SOURCE -D_DEFAULT_SOURCE -include /work/compat/switch_compat.h"

echo "=== compiling app + shims ==="
$GCC -c "winsys/smoke/$APP.c"     -o "$OBJ/$APP.o"           $ARCH -D__SWITCH__ -D_GNU_SOURCE -DFG2_ROOT_DIAG_LIMIT="$ROOT_DIAG_LIMIT" -DFG2_ROOT_UPLOAD_CACHE_FLUSH="$ROOT_UPLOAD_CACHE_FLUSH" -DFG2_QMD_UPLOAD_IDENTITY="$QMD_UPLOAD_IDENTITY" -DFG2_QMD_UPLOAD_CACHE_FLUSH="$QMD_UPLOAD_CACHE_FLUSH" -DFG2_QMD_ADDRESS_CONTROL="$QMD_ADDRESS_CONTROL" -DFG2_QMD_ADDRESS_FRESH="$QMD_ADDRESS_FRESH" -DFG2_ROOT_ADDRESS_CONTROL="$ROOT_ADDRESS_CONTROL" -DFG2_ROOT_ADDRESS_FRESH="$ROOT_ADDRESS_FRESH" -Imesa-25/include -I$DKP/libnx/include -O2 -Wall
$GCC -c winsys/drm_shim.c         -o "$OBJ/drm_shim.o"       $ARCH $DEFS $INC -O2 ${DRM_SHIM_DEBUG:+-DDRM_SHIM_DEBUG}
$GCC -c winsys/switch_libc_shim.c -o "$OBJ/switch_libc_shim.o" $ARCH $DEFS $INC -O2
$GCC -c compat/compat.c           -o "$OBJ/compat.o"         $ARCH $DEFS $INC -O2

# Archive set + order from mb/build.ninja's ICD .so link.
ARCHIVES="
  src/nouveau/codegen/libnouveau_codegen.a
  src/util/libmesa_util.a src/util/libmesa_util_sse41.a src/util/blake3/libblake3.a
  src/c11/impl/libmesa_util_c11.a
  src/nouveau/compiler/libnak.a src/nouveau/compiler/libnak_rs.a
  src/compiler/rust/libcompiler_c_helpers.a
  src/nouveau/headers/libnvidia_headers_c.a
  src/nouveau/nil/liblibnil.a src/nouveau/nil/liblibnil_format_table.a
  src/compiler/nir/libnir.a src/compiler/libcompiler.a
  src/nouveau/mme/libnouveau_mme.a src/nouveau/winsys/libnouveau_ws.a
  src/vulkan/util/libvulkan_util.a src/compiler/spirv/libvtn.a
  src/util/libxmlconfig.a"
PORTLIBS="$DKP/portlibs/switch/lib/libz.a $DKP/portlibs/switch/lib/libzstd.a $DKP/portlibs/switch/lib/libexpat.a"

echo "=== linking ELF ==="
cd "$MB"
$GXX -specs="$DKP/libnx/switch.specs" $ARCH \
  -L$DKP/libnx/lib -L$DKP/portlibs/switch/lib \
  -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=stat -Wl,--wrap=lstat \
  -o "$OBJ/$APP.elf" \
  "$OBJ/$APP.o" "$OBJ/drm_shim.o" "$OBJ/switch_libc_shim.o" "$OBJ/compat.o" \
  -Wl,--whole-archive src/nouveau/vulkan/libnvk.a -Wl,--no-whole-archive \
  -Wl,--start-group \
    $ARCHIVES $PORTLIBS \
    -lnx -lc -lm -ldl -pthread \
  -Wl,--end-group

echo "=== packaging NRO ==="
"$STRIP" "$OBJ/$APP.elf" -o "$OBJ/$OUTPUT.stripped.elf"
"$DKP/tools/bin/nacptool" --create "$TITLE" "switch-nvk" "$VERSION" "$OBJ/$OUTPUT.nacp"
"$DKP/tools/bin/elf2nro" "$OBJ/$APP.elf" "/work/$OUTPUT.nro" \
  --icon="$DKP/libnx/default_icon.jpg" --nacp="$OBJ/$OUTPUT.nacp"

echo "=== DONE -> /work/$OUTPUT.nro ==="
ls -la "/work/$OUTPUT.nro"
