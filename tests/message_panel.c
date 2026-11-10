/* Compile with -ffunction-sections -fdata-sections -Wl,--gc-sections. */
#include <strings.h>
#include "../win/gl/gl_win.c"
#include "../win/gl/gl_emul.c"

struct instance_flags iflags;
winid WIN_MESSAGE = 0;
char toplines[TBUFSZ];
int sdlgl_alt_prev, sdlgl_msg_dim = 1, sdlgl_prev_step = 1, sdlgl_width, sdlgl_height;
static int more_keys;

int sgn(int n) { return (n > 0) - (n < 0); }
void sdlgl_insert_text_item(struct TextWindow *w, struct TextItem *id, int glyph, int attr, const char *s) { abort(); }
void sdlgl_flush(void) {}
int sdlgl_get_key(int f) { ++more_keys; return ' '; }
void Sdlgl_nhbell(void) { abort(); }
void sdlgl_remove_extrashapes(struct TileWindow *w, short x, short y) {}
void sdlgl_set_cursor(struct TileWindow *w, int x, int y, int n) {}
void sdlgl_map_tilewin(struct TileWindow *w, int x, int y, int a, int b, int d) {}
void sdlgl_unmap_tilewin(struct TileWindow *w) {}
struct rendering_procs sdlgl_rend_procs;
int strncmpi(const char *a, const char *b, int n) { return strncasecmp(a, b, n); }
char *strstri(const char *a, const char *b) { return (char *) strstr(a, b); }

void sdlgl_store_char(struct TileWindow *w, int x, int y, char c, rgbcol_t col)
{
  assert(x >= 0 && x < w->total_w && y >= 0 && y < w->total_h);
  w->tiles[y * w->total_w + x].fg = CHAR_2_TILE(c);
  w->tiles[y * w->total_w + x].u.col = col;
}
int sdlgl_store_str(struct TileWindow *w, int x, int y, const char *s, int n, rgbcol_t col)
{
  int i;
  for (i = 0; i < n && s[i]; ++i)
    sdlgl_store_char(w, x + i, y, s[i], col);
  return i;
}
void sdlgl_blank_area(struct TileWindow *w, int x, int y, int a, int b)
{
  int i, j;
  for (j = y; j < y + b; ++j)
    for (i = x; i < x + a; ++i)
      w->tiles[j * w->total_w + i].fg = TILE_EMPTY;
}
void sdlgl_transfer_area(struct TileWindow *s, int x, int y, int a, int b,
                          struct TileWindow *d, int dx, int dy)
{
  int j;
  for (j = b - 1; j >= 0; --j)
    memmove(d->tiles + (dy + j) * d->total_w + dx,
            s->tiles + (y + j) * s->total_w + x, a * sizeof *s->tiles);
}
void sdlgl_copy_area(struct TileWindow *w, int x, int y, int a, int b, int dx, int dy)
{
  sdlgl_transfer_area(w, x, y, a, b, w, dx, dy);
}
void sdlgl_transfer_line(struct TileWindow *w, int x, int y, int n,
                          struct TilePair *buffer, int save)
{
  struct TilePair *line = w->tiles + y * w->total_w + x;
  memcpy(save ? buffer : line, save ? line : buffer, n * sizeof *line);
}

int main(void)
{
  struct TilePair front[80 * 5], past[80 * 20];
  struct TileWindow base = {0}, history = {0};
  struct TextWindow win = {0};
  int i, row, archived;
  base.total_w = history.total_w = win.show_w = 80;
  base.total_h = win.show_h = DEF_VARY_MSGCOUNT;
  history.total_h = 20;
  base.tiles = front;
  base.is_text = history.is_text = 1;
  history.tiles = past;
  win.type = NHW_MESSAGE;
  win.base = &base;
  win.scrollback = &history;
  win.scrollback_enable = 1;
  win.write_col = L_GREY;
  text_wins[0] = &win;
  sdlgl_blank_area(&base, 0, 0, 80, 5);
  sdlgl_home(&win);

  for (i = 0; i < 8; ++i) {
    char message[] = "Message A";
    message[8] += i;
    Sdlgl_putstr(0, 0, message);
    assert(front[win.write_y * 80 + 8].fg == 'A' + i);
    assert(front[win.write_y * 80].u.col == L_GREY);
    Sdlgl_clear_nhwindow(0);
    assert(front[win.write_y * 80].u.col == GREY);
    Sdlgl_clear_nhwindow(0); /* Idle commands do not consume a row. */
  }
  assert(more_keys == 0 && win.scrollback_size == 3);
  for (row = 0; row < 5; ++row)
    assert(front[row * 80 + 8].fg == 'H' - row);
  assert(past[8].fg == 'C');

  /* Fresh output still pauses before pushing unread lines off-screen. */
  for (i = 0; i < 6; ++i)
    Sdlgl_putstr(0, 0, "A message long enough that two messages cannot share a row.");
  assert(more_keys == 1);

  /* A wrapped message uses several rows without losing old lines. */
  Sdlgl_clear_nhwindow(0);
  Sdlgl_putstr(0, 0, "A long message with enough words to wrap across two rows while keeping the recent messages visible above it.");
  assert(win.fresh_lines == 2 && more_keys == 1);

  /* Descriptions replace each other and never pollute recall. */
  Sdlgl_putstr(0, ATR_NOHISTORY, "A long temporary description.");
  archived = win.scrollback_size;
  Sdlgl_putstr(0, ATR_NOHISTORY, "Floor.");
  assert(front[4 * 80].fg == 'F' && front[4 * 80 + 6].fg == TILE_EMPTY);
  Sdlgl_clear_nhwindow(0);
  Sdlgl_putstr(0, 0, "Back to play.");
  assert(win.scrollback_size == archived && win.scrollback_enable);

  /* Prompt takeovers archive messages, then discard their input. */
  sdlgl_reset_message(&win);
  archived = win.scrollback_size;
  win.scrollback_enable = 0;
  sdlgl_puts(&win, "Prompt: input");
  Sdlgl_clear_nhwindow(0);
  assert(win.scrollback_size == archived);
  for (i = 0; i < 80 * 5; ++i)
    assert(front[i].fg == TILE_EMPTY);
  assert(!win.want_new_line && !win.fresh_lines);
  win.scrollback_enable = 1;
  sdlgl_msg_dim = 0;
  Sdlgl_putstr(0, 0, "Legacy clearing.");
  Sdlgl_clear_nhwindow(0);
  assert(front[4 * 80].fg == TILE_EMPTY);
  return 0;
}
