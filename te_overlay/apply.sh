#!/bin/sh
# Adds the boot-time SD swap to the TegraExplorer payload (idempotent).
# usage: te_overlay/apply.sh <path to TegraExplorer checkout>
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
TE=${1:-"$HERE/../TegraExplorer"}
MAIN="$TE/source/main.c"
[ -f "$MAIN" ] || { echo "apply.sh: $MAIN not found" >&2; exit 1; }

cp "$HERE/aiosu_swap.c" "$HERE/aiosu_swap.h" "$TE/source/"

if ! grep -q 'aiosu_swap.h' "$MAIN"; then
    sed -i '0,/#include "err.h"/s//#include "err.h"\n#include "aiosu_swap.h"/' "$MAIN"
fi
if ! grep -q 'aiosu_run_pending_swap' "$MAIN"; then
    sed -i 's|^\([[:space:]]*\)easy_rename("atmosphere/fusee-secondary.bin.aio"|\1aiosu_run_pending_swap();\n&|' "$MAIN"
fi

grep -q '#include "aiosu_swap.h"' "$MAIN" || { echo "apply.sh: include not inserted" >&2; exit 1; }
[ "$(grep -c 'aiosu_run_pending_swap();' "$MAIN")" = 1 ] || { echo "apply.sh: call not inserted exactly once" >&2; exit 1; }
echo "apply.sh: TegraExplorer patched"
