#!/bin/sh
set -eu
cd "$(dirname "$0")"
export EVILHACK_PREFIX=${EVILHACK_PREFIX:-$HOME/.local}
playground=$EVILHACK_PREFIX/games/evilhackdir
first_install=false
if [ ! -e "$playground" ]; then
    first_install=true
fi
./build-gl.sh
# Only seed configuration on a new installation; saves stay independent.
if "$first_install" && [ "$playground" != "$PWD/local/games/evilhackdir" ]; then
    if [ -f local/games/evilhackdir/evilhack-gl.conf ]; then
        cp local/games/evilhackdir/evilhack-gl.conf "$playground/evilhack-gl.conf"
    fi
fi
install -Dm755 sys/unix/evilhack-gl.sh "$EVILHACK_PREFIX/bin/evilhack-gl"
data_home=${XDG_DATA_HOME:-$HOME/.local/share}
icon=$data_home/icons/hicolor/64x64/apps/evilhack-gl.png
mkdir -p "$(dirname "$icon")"
python3 - "$icon" <<'PY_ICON'
from PIL import Image
from io import BytesIO
from pathlib import Path
import sys
# Pillow expects palette rows immediately after the XPM dimensions.
rows = [line for line in Path("win/Qt/knh.xpm").read_bytes().splitlines() if line.startswith(b'"')]
xpm = b"/* XPM */\nstatic char *icon[] = {\n" + b"\n".join(rows) + b"\n};"
with Image.open(BytesIO(xpm)) as icon:
    icon.convert("RGBA").resize((64, 64), Image.Resampling.NEAREST).save(sys.argv[1])
PY_ICON
mkdir -p "$data_home/applications"
entry=$data_home/applications/evilhack-gl.desktop
cat > "$entry" <<EOF_DESKTOP
[Desktop Entry]
Type=Application
Name=EvilHack GL
Comment=EvilHack with SDL/OpenGL tiles
Exec=$EVILHACK_PREFIX/bin/evilhack-gl
Icon=$icon
Terminal=false
Categories=Game;RolePlaying;
Keywords=NetHack;roguelike;tiles;
Actions=Software;

[Desktop Action Software]
Name=Software Rendering
Exec=$EVILHACK_PREFIX/bin/evilhack-gl -wSDL
EOF_DESKTOP
chmod 644 "$entry"
if command -v desktop-file-validate >/dev/null 2>&1; then
    desktop-file-validate "$entry"
fi
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$data_home/applications"
fi
printf 'Installed: %s\nDesktop entry: %s\n' "$EVILHACK_PREFIX/bin/evilhack-gl" "$entry"
