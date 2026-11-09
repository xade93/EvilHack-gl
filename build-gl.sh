#!/bin/sh
set -eu
cd "$(dirname "$0")"
sh sys/unix/setup.sh sys/unix/hints/linux-gl
make -C src ../include/pm.h ../include/onames.h ../include/vis_tab.h ../include/date.h
make -C src -j"$(getconf _NPROCESSORS_ONLN)"
make all
python3 win/gl/make_assets.py
# 'install' deletes the playground; subsequent builds must use 'update'.
if [ -d local/games/evilhackdir ]; then
    make update
else
    make install
fi
cp dat/gl*.png local/games/evilhackdir/
if [ ! -f local/games/evilhackdir/evilhack-gl.conf ]; then
    cp dat/evilhack-gl.conf local/games/evilhackdir/
fi
