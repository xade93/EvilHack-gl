# EvilHack SDL/OpenGL prototype

Based on EvilHack 0.9.3, commit `c444f6a3ab1e9f16d0676961dba86f628e91c6ba`.
The renderer comes from the adjacent SLASH'EM checkout's Andrew Apted
SDL/GL window port, with its original copyright and license notices retained.

Run from this directory:

```sh
./run-gl.sh
```

The launcher opens a 1280×800 window using OpenGL. For software rendering:

```sh
./run-gl.sh -wSDL
```

Page Up/Down zoom the map, Home centres on the player, and End switches
between tiles and ASCII. Inventory and gameplay commands use EvilHack's
normal keyboard controls.

The local installation, saves and settings are under `local/games/evilhackdir/`.
Edit `evilhack-gl.conf` there for gameplay and font options. Renderer options
can be supplied through `EVILHACKGLOPTIONS`, for example
`EVILHACKGLOPTIONS=windowed,mode:1280x800 ./run-gl.sh`.
Existing system installations and their saves are not used.

The default config enables gold-only autopickup (plus recovery of thrown
ammunition), prompts before becoming burdened, adds confirmations for risky
actions, and keeps inventory selection predictable. Resumed characters retain
saved gameplay options: use `O` to enable `autopickup` and set `pickup_types`
to `$` for an existing character. New characters use the config defaults.

Rebuild with `./build-gl.sh`. Dependencies are a C compiler, make, flex,
bison, groff, SDL 1.2-compatible development files (`sdl-config`), OpenGL,
libpng, ncurses, Python 3 and Pillow. The installation prefix in
`sys/unix/hints/linux-gl` is specific to this checkout; change it if moving
the project. The script uses `make update` for an existing installation;
do not use upstream `make install` on an installation containing saves,
because that target deletes its playground.

Tiles are generated from EvilHack's own `win/share` sources using its
native `tile2bmp` converter. This validates tile names against the engine.
`win/gl/make_assets.py` checks all 9,256 glyph mappings against the 1,975
converted tiles, including grayscale statues, and generates 16px and
nearest-neighbour 32px PNG atlases. No SLASH'EM monster/item atlas is used.
The runtime also checks tile-map bounds before play. This verifies coverage
and ordering, not that every upstream picture is visually distinctive.

The port uses EvilHack's supplied foreground/background glyphs and generic
status formatter, including variant-specific conditions. The character's name and title double as the HP bar, with a filled background
behind the text. HP uses muted sage above 50%, soft amber at 21–50%, and dusty red at 20%
or below. Exact HP and power values remain in the stats line. Power text is
amber at half capacity and red at one fifth; there is no separate MP bar.
The remaining status is plain text, without configurable highlighting. Legacy glHack's 64px
isometric artwork is not supplied. Message scrollback is session-local;
the generic message-history save hooks do not persist it.

Verified on a private Xvfb display: OpenGL startup with Mesa software GL,
character creation, tiled map, inventory, keyboard movement, status fields,
saving/restoring a test character, software rendering, ASCII toggle and zoom.
Rebuilding with `build-gl.sh` preserved the test save byte-for-byte.
This is a playable prototype, not a
full-game regression test or a hardware-GPU compatibility certification.

Cursor-description regression check: enter `;`, move repeatedly with `h`/`l`,
then move from the player to an empty floor square. Only the current
square's description should remain, without leftover characters or `[More]`
prompts. Escape returns to normal messages. These temporary descriptions
honour EvilHack's suppress-history flag and do not enter message scrollback.
Verified with the OpenGL port on Xvfb.

Status-bar regression test (no display required):

```sh
cc -std=gnu17 -DGL_GRAPHICS -DUSE_TILES -Iinclude tests/status_bars.c win/gl/gl_stat.c -o /tmp/evilhack-status-test
/tmp/evilhack-status-test
```

This checks empty/partial/full title bars, HP colour thresholds, preservation
of title text, changing maxima, power colours (including wrapped fields),
zero power capacity, and widths from 1 to 128 text cells. Appearance checks
for the revised layout are left to interactive testing.

Control-key regression test (no display required):

```sh
cc -std=gnu17 -DGL_GRAPHICS -DUSE_TILES -Iinclude -ffunction-sections -fdata-sections tests/key_input.c -Wl,--gc-sections -o /tmp/evilhack-key-test
/tmp/evilhack-key-test
```

The real SDL key converter must produce control codes even when the backend
supplies a printable Unicode letter. This covers both Ctrl keys, Shift,
pretranslated and absent Unicode, and unchanged plain/Alt-letter input.
Ctrl+D should prompt for a kick direction rather than opening the drop menu.
