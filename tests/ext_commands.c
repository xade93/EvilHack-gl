/* Compile with -ffunction-sections -fdata-sections -Wl,--gc-sections. */
#include "../win/gl/gl_emul.c"

struct flag flags;
struct instance_flags iflags;
winid WIN_MESSAGE = 0;
struct TextWindow *text_wins[MAXWIN];
int sdlgl_alt_prev;
struct ext_func_tab extcmdlist[] = {
  {0, "ride", "ride", 0, AUTOCOMPLETE},
  {0, "rub", "rub", 0, AUTOCOMPLETE},
  {0, "pray", "pray", 0, AUTOCOMPLETE},
  {0, "prayer", "prayer", 0, AUTOCOMPLETE},
  {0, "drop", "drop", 0, 0},
  {0, "wizwish", "wizard", 0, AUTOCOMPLETE | WIZMODECMD},
  {0, "wait", "wait", 0, AUTOCOMPLETE},
  {0, "waste", "unavailable", 0, AUTOCOMPLETE | CMD_NOT_AVAILABLE},
  {0, "abcdefghijklmnopqrstuvwx", "long command", 0, AUTOCOMPLETE},
  {0, NULL}
};
static const char *keys;
static int errors, menu_calls, key_index;
static char suggestion[BUFSZ];
static int cursor_columns[BUFSZ];

void sdlgl_flush(void) {}
int sdlgl_get_key(int f)
{
  assert(*keys);
  cursor_columns[key_index++] = text_wins[0]->write_x;
  return (unsigned char) *keys++;
}
void sdlgl_set_cursor(struct TileWindow *w, int x, int y, int z) {}
void sdlgl_store_char(struct TileWindow *w, int x, int y, char c, rgbcol_t col) {}
int sdlgl_store_str(struct TileWindow *w, int x, int y, const char *s, int n, rgbcol_t c)
{
  assert(n >= 0 && x + n <= w->total_w);
  snprintf(suggestion, sizeof suggestion, "%.*s", n, s);
  return min(n, (int) strlen(s));
}
void sdlgl_blank_area(struct TileWindow *w, int x, int y, int a, int b) {}
void sdlgl_copy_area(struct TileWindow *w, int x, int y, int a, int b, int c, int d) { abort(); }
void sdlgl_transfer_area(struct TileWindow *w, int x, int y, int a, int b,
                         struct TileWindow *v, int c, int d) { abort(); }
void Sdlgl_nhbell(void) {}
int Sdlgl_doprev_message(void) { return 0; }
void sdlgl_remove_scrollback(void) {}
int extcmd_via_menu(void) { ++menu_calls; return 77; }
void pline(const char *fmt, ...) { ++errors; }
void sdlgl_more(struct TextWindow *w) {}
void Sdlgl_clear_nhwindow(winid w) {}
void sdlgl_reset_message(struct TextWindow *w) {}

static int enter(struct TextWindow *win, const char *input)
{
  keys = input;
  key_index = 0;
  suggestion[0] = '\0';
  return do_get_ext_cmd(win);
}

int main(void)
{
  struct TileWindow base = {0};
  struct TextWindow win = {0};
  base.total_w = win.show_w = 80;
  base.total_h = win.show_h = 3;
  win.base = &base;
  text_wins[0] = &win;
  assert(match_ext_cmd("") == -1);
  assert(match_ext_cmd("r") == -1);
  assert(match_ext_cmd("wa") == 6);
  assert(match_ext_cmd("wiz") == -1);
  assert(match_ext_cmd("waste") == -1);
  assert(match_ext_cmd("dr") == -1);
  assert(enter(&win, "ri\n") == 0);
  assert(!strcmp(suggestion, "de"));
  assert(cursor_columns[2] == 4); /* '# ri', not '# ride'. */
  assert(enter(&win, "ride\n") == 0);
  assert(enter(&win, "RIDE\n") == 0);
  assert(enter(&win, "ri\bub\n") == 1);
  assert(enter(&win, "ri\177ub\n") == 1);
  assert(enter(&win, "rid\025rub\n") == 1);
  assert(enter(&win, "pray\n") == 2); /* Exact match wins over 'prayer'. */
  assert(enter(&win, "drop\n") == 4);
  assert(enter(&win, "dr\n") == -1 && errors == 1);
  assert(enter(&win, "ri\033") == -1 && errors == 1);
  assert(enter(&win, "\b") == -1 && errors == 1);
  assert(enter(&win, "\n") == -1 && errors == 1);
  assert(enter(&win, "abcdefghijklmnopqrstuvwx\n") == 8);
  assert(enter(&win, "?") == 77 && menu_calls == 1);
  iflags.extmenu = TRUE;
  assert(Sdlgl_get_ext_cmd() == 77 && menu_calls == 2);
  extcmdlist[0].flags &= ~AUTOCOMPLETE;
  assert(match_ext_cmd("ri") == -1);
  assert(enter(&win, "ride\n") == 0);
  extcmdlist[0].flags |= AUTOCOMPLETE;
  base.total_w = win.show_w = 8;
  assert(enter(&win, "a\n") == 8); /* A long suggestion is clipped to the row. */
  assert(strlen(suggestion) == 4);
  wizard = TRUE;
  assert(match_ext_cmd("wiz") == 5);
  return 0;
}
