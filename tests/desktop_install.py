#!/usr/bin/env python3
"""Check an existing scratch install and verify an installer rerun preserves data.

Usage: python3 tests/desktop_install.py PREFIX XDG_DATA_HOME
"""
from pathlib import Path
import hashlib
import os
import subprocess
import sys
from PIL import Image

root = Path(__file__).resolve().parents[1]
prefix, data = (Path(p).resolve() for p in sys.argv[1:])
game = prefix / 'games/evilhackdir'
launcher = prefix / 'bin/evilhack-gl'
entry = data / 'applications/evilhack-gl.desktop'
icon = data / 'icons/hicolor/64x64/apps/evilhack-gl.png'
env = dict(os.environ, EVILHACK_PREFIX=str(prefix), XDG_DATA_HOME=str(data))

def snapshot():
    paths = [game / 'sysconf', game / 'evilhack-gl.conf']
    paths += [p for p in (game / 'var/save').rglob('*') if p.is_file()]
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}

before = snapshot()
with open('/tmp/evilhack-desktop-regression.log', 'w') as log:
    subprocess.run([str(root / 'install-desktop-gl.sh')], cwd=root, env=env,
                   stdout=log, stderr=subprocess.STDOUT, check=True)
assert before == snapshot(), 'Installer changed saves or configuration'
subprocess.run(['desktop-file-validate', str(entry)], check=True)
with Image.open(icon) as image:
    assert image.size == (64, 64) and image.getbbox()
text = entry.read_text()
assert f'Exec={launcher}\n' in text
assert f'Exec={launcher} -wSDL\n' in text
assert f'Icon={icon}\n' in text
paths = subprocess.check_output([str(launcher), '-showpaths'], text=True, env=env)
assert str(game / 'sysconf') in paths
assert str(game / 'var') in paths
subprocess.run([str(launcher), '-s'], check=True, stdout=subprocess.DEVNULL, env=env)
print('Desktop entry, icon, compiled paths and save/config preservation passed.')
