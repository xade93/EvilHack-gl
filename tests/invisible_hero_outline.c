/* Run the map renderer without a display and record its cursor draws. */
#include "../win/gl/gl_tile.c"

struct you u;
struct rendering_procs sdlgl_rend_procs;
rgbcol_t sdlgl_text_colors[256];

int isok(int x, int y)
{
  return x >= 1 && x < COLNO && y >= 0 && y < ROWNO;
}

static int dirty_count, cursor_count;
static int dirty_x[2], dirty_y[2], cursor_x[2], cursor_y[2];

static void clip(int x, int y, int w, int h) {}
static void finish(void) {}
static void background(int x, int y, int w, int h, rgbcol_t color,
                       int start, int depth) {}
static void begin(int blending, int overlap) {}

static void dirty(int x, int y, int w, int h, int depth)
{
  assert(dirty_count < 2);
  dirty_x[dirty_count] = x;
  dirty_y[dirty_count++] = y;
}

static void cursor(struct TileWindow *win, int x, int y, int width)
{
  assert(cursor_count < 2 && !win->curs_block && width == 1);
  cursor_x[cursor_count] = x;
  cursor_y[cursor_count++] = y;
}

static void draw(struct TileWindow *win)
{
  dirty_count = cursor_count = 0;
  draw_tilewindow(win);
}

int main(void)
{
  struct TileSet set = {0};
  struct TilePair tiles[8 * 8];
  struct TileWindow win = {
    .set = &set, .tiles = tiles, .is_map = 1,
    .total_w = 8, .total_h = 8, .scr_w = 80, .scr_h = 80,
    .scale_w = 10, .scale_h = 10, .scale_full_w = 10, .scale_full_h = 10,
    .curs_x = -1, .hero_outline_x = -1, .hero_outline_y = -1
  };
  int i;

  for (i = 0; i < SIZE(tiles); ++i)
    tiles[i] = (struct TilePair) {
      .fg = TILE_EMPTY, .mg = TILE_EMPTY, .u.bg = TILE_EMPTY
    };

  sdlgl_rend_procs = (struct rendering_procs) {
    .enable_clipper = clip, .disable_clipper = finish,
    .draw_background = background, .begin_tile_draw = begin,
    .finish_tile_draw = finish, .mark_dirty = dirty, .draw_cursor = cursor
  };
  u.ux = 2;
  u.uy = 3;
  draw(&win);
  assert(cursor_count == 0 && dirty_count == 0);

  HInvis = 1;
  draw(&win);
  assert(cursor_count == 1 && cursor_x[0] == 2 && cursor_y[0] == 4);
  assert(dirty_count == 1 && dirty_x[0] == 20 && dirty_y[0] == 40);

  HSee_invisible = 1;
  win.curs_x = 5;
  win.curs_y = 2;
  win.curs_w = 1;
  win.pan_x = 3;
  win.pan_y = 6;
  u.ux = 3;
  draw(&win);
  assert(cursor_count == 2 && cursor_x[0] == 3 && cursor_y[0] == 4);
  assert(cursor_x[1] == 5 && cursor_y[1] == 2);
  assert(dirty_count == 2 && dirty_x[0] == 17 && dirty_x[1] == 27);
  assert(dirty_y[0] == 34 && dirty_y[1] == 34);

  draw(&win);
  assert(cursor_count == 2 && dirty_count == 0);

  BInvis = 1;
  draw(&win);
  assert(cursor_count == 1 && cursor_x[0] == 5 && cursor_y[0] == 2);
  assert(dirty_count == 1 && dirty_x[0] == 27 && dirty_y[0] == 34);

  win.is_text = 1;
  for (i = 0; i < SIZE(tiles); ++i)
    tiles[i].u.col = 0;
  BInvis = 0;
  draw(&win);
  assert(cursor_count == 2 && cursor_x[0] == 3 && cursor_y[0] == 4);
  return 0;
}
