#!/bin/sh
set -eu
cd "$(dirname "$0")/../games/evilhackdir"
export EVILHACKOPTIONS="${EVILHACKOPTIONS:-$PWD/evilhack-gl.conf}"
export EVILHACKGLOPTIONS="${EVILHACKGLOPTIONS:-windowed,mode:1280x800}"
exec ./evilhack "$@"
