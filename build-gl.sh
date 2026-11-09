#!/bin/sh
set -eu
cd "$(dirname "$0")"
EVILHACK_PREFIX=${EVILHACK_PREFIX:-$PWD/local}
# Upstream makefiles embed these paths into compiler flags without shell quoting.
case "$EVILHACK_PREFIX" in
    /*) ;;
    *) echo 'EVILHACK_PREFIX must be an absolute path.' >&2; exit 1 ;;
esac
case "$EVILHACK_PREFIX" in
    *[!a-zA-Z0-9_./-]*) echo 'EVILHACK_PREFIX must contain only letters, digits, _, ., / or -.' >&2; exit 1 ;;
esac
export EVILHACK_PREFIX
make_gl() { make PREFIX="$EVILHACK_PREFIX" "$@"; }
sh sys/unix/setup.sh sys/unix/hints/linux-gl
mkdir -p local
# Changing the prefix changes compiled-in paths, so cached objects cannot be reused.
if [ ! -f local/.gl-build-prefix ] || [ "$(cat local/.gl-build-prefix)" != "$EVILHACK_PREFIX" ]; then
    make_gl clean
    rm -f local/.gl-build-prefix
fi
make_gl -C src ../include/pm.h ../include/onames.h ../include/vis_tab.h ../include/date.h
make_gl -C src -j"$(getconf _NPROCESSORS_ONLN)"
make_gl all
python3 win/gl/make_assets.py
playground=$EVILHACK_PREFIX/games/evilhackdir
# 'install' deletes the playground; subsequent builds must use 'update'.
if [ -d "$playground" ]; then
    make_gl update
else
    make_gl install
fi
cp dat/gl*.png "$playground/"
if [ ! -f "$playground/evilhack-gl.conf" ]; then
    cp dat/evilhack-gl.conf "$playground/"
fi
printf '%s\n' "$EVILHACK_PREFIX" > local/.gl-build-prefix
