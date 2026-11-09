/* cc -std=gnu17 -DGL_GRAPHICS -DUSE_TILES -Iinclude
 * tests/status_bars.c win/gl/gl_stat.c -o /tmp/evilhack-status-test
 */
#include "hack.h"
#define WINGL_INTERNAL
#include "winGL.h"

winid WIN_STATUS = 0;
struct TextWindow *text_wins[MAXWIN];
static struct TilePair tiles[3 * 128];
static const char title[] = "[Test the Adventurer            ]";
static const char stats[] = "HP:14(16) Pw:2(10) AC:5";
static const char *title_format;
static int calls;

void genl_status_enablefield(int f, const char *n, const char *fmt, BOOLEAN_P e)
{
  title_format = fmt;
}

void genl_status_update(int f, genericptr_t v, int c, int p, int col,
                       unsigned long *m)
{
  int i, width = text_wins[0]->show_w;
  if (f != BL_FLUSH && f != BL_RESET)
    return;
  memset(tiles, 0, sizeof tiles);
  for (i = 0; i < width * 3; ++i)
    tiles[i].fg = ' ';
  for (i = 0; i < (int) strlen(title) && i < width; ++i)
    tiles[2 * width + i].fg = title[i];
  for (i = 0; i < (int) strlen(stats) && i < width * 2; ++i)
    tiles[(1 - i / width) * width + i % width].fg = stats[i];
}

void sdlgl_store_char(struct TileWindow *w, int x, int y, char ch, rgbcol_t col)
{
  assert(x >= 0 && x < text_wins[0]->show_w && y >= 0 && y < 3);
  assert(TILE_2_CHAR(tiles[y * text_wins[0]->show_w + x].fg) == (unsigned char) ch);
  tiles[y * text_wins[0]->show_w + x].u.col = col;
  ++calls;
}

int main(void)
{
  struct TileWindow base = {0};
  struct TextWindow win = {0};
  int width, hp, i;
  char value[16];
  base.tiles = tiles;
  win.base = &base;
  win.show_h = 3;
  text_wins[0] = &win;
  Sdlgl_status_enablefield(BL_TITLE, "title", "%s", TRUE);
  assert(!strcmp(title_format, "[%-30.30s]"));
  Sdlgl_status_update(BL_HPMAX, "100", 0, 0, 0, 0);
  Sdlgl_status_update(BL_ENEMAX, "100", 0, 0, 0, 0);
  for (width = 1; width <= 128; ++width) {
    win.show_w = width;
    for (hp = 0; hp <= 100; ++hp) {
      int filled = 0, expected_color = hp <= 20 ? STATUS_DANGER : hp <= 50 ? STATUS_CAUTION : STATUS_HEALTHY;
      sprintf(value, "%d", hp);
      calls = 0;
      Sdlgl_status_update(BL_HP, value, 0, 0, 0, 0);
      Sdlgl_status_update(BL_ENE, value, 0, 0, 0, 0);
      assert(calls == 0);
      Sdlgl_status_update(BL_FLUSH, 0, 0, 0, 0, 0);
      for (i = 0; i < width; ++i)
        if (tiles[2 * width + i].u.col >> 8) {
          assert((tiles[2 * width + i].u.col >> 8) == expected_color);
          ++filled;
        }
      if (width >= 32) {
        assert((filled == 0) == (hp == 0));
        assert((filled == 30) == (hp == 100));
      }
      if (width * 2 >= (int) strlen(stats)) {
        for (i = 10; i <= 17; ++i)
          assert(tiles[(1 - i / width) * width + i % width].u.col
                 == (hp <= 20 ? STATUS_DANGER : hp <= 50 ? STATUS_CAUTION : L_GREY));
      }
    }
  }
  Sdlgl_status_update(BL_HPMAX, "200", 0, 0, 0, 0);
  Sdlgl_status_update(BL_ENEMAX, "0", 0, 0, 0, 0);
  Sdlgl_status_update(BL_RESET, 0, 0, 0, 0, 0);
  assert((tiles[257].u.col >> 8) == STATUS_CAUTION);
  assert((tiles[271].u.col >> 8) == STATUS_CAUTION && !(tiles[272].u.col >> 8));
  assert(tiles[138].u.col == L_GREY);
  return 0;
}
