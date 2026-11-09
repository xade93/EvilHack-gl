#!/usr/bin/env python3
"""Convert EvilHack's validated native tiles to glHack PNG atlases."""
from pathlib import Path
import re
import subprocess
import tempfile

from PIL import Image

root = Path(__file__).resolve().parents[2]
subprocess.run(["make", "-C", str(root / "util"), "tile2bmp"], check=True)
with tempfile.TemporaryDirectory() as tmp:
    bmp = Path(tmp) / "tiles.bmp"
    result = subprocess.run(
        ["./tile2bmp", str(bmp)], cwd=root / "util",
        capture_output=True, text=True, check=True,
    )
    report = result.stdout + result.stderr
    match = re.fullmatch(r"Total of (\d+) tiles written to .+\.\n", report)
    assert match, "Tile source validation failed:\n" + report
    count = int(match[1])
    mapping = (root / "src/tile.c").read_text()
    glyphs = [int(n) for n in re.search(
        r"glyph2tile\[MAX_GLYPH\] = \{(.*?)\};", mapping, re.S
    )[1].replace("\n", " ").split(",") if n.strip()]
    assert min(glyphs) >= 0 and max(glyphs) < count
    assert int(re.search(r"total_tiles_used = (\d+)", mapping)[1]) <= count
    atlas = Image.open(bmp).convert("RGBA")
    across = atlas.width // 16
    atlas = atlas.crop((0, 0, atlas.width, ((count + across - 1) // across) * 16))
    atlas.putdata([
        (r, g, b, 0 if (r, g, b) == (71, 108, 108) else 255)
        for r, g, b, _ in atlas.get_flattened_data()
    ])
    for size in (16, 32):
        scaled = atlas.resize(
            (atlas.width * size // 16, atlas.height * size // 16),
            Image.Resampling.NEAREST,
        )
        target = root / "dat" / f"gltile{size}.png"
        scaled.save(target)
        with Image.open(target) as check:
            assert check.size == scaled.size
    print(f"Validated {len(glyphs)} glyph mappings against {count} tiles.")
