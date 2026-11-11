/* No display needed: feed mouse events through the real input loop. */
#include "../win/gl/gl_main.c"
#include "../win/gl/gl_map.c"

struct instance_flags iflags;
winid WIN_MESSAGE = WIN_ERR;
int sdlgl_height = 800;
void hangup(int sig) { abort(); }
void clearlocks(void) { abort(); }
void nh_terminate(int status) { abort(); }
void sdlgl_free_mouse_cursors(void) {}
void sdlgl_win_shutdown(void) {}
void sdlgl_emul_shutdown(void) {}
void sdlgl_font_shutdown(void) {}
void sdlgl_tile_shutdown(void) {}
void SDL_Quit(void) {}
struct TextWindow *text_wins[MAXWIN];
int sdlgl_map_win = 0, sdlgl_top_win = WIN_ERR;
struct MouseLocation sdlgl_mouseloc;
int sdlgl_key_repeat, sdlgl_alt_prev;
static SDL_Event events[16];
static int event_count, event_pos;
static int mouse_window;

int sgn(int n) { return (n > 0) - (n < 0); }
boolean letter(CHAR_P c) { return (c >= 'a' && c <= 'z'); }
int SDL_PollEvent(SDL_Event *ev)
{
  assert(event_pos < event_count);
  *ev = events[event_pos++];
  return 1;
}
void sdlgl_update_mouse_location(int lost) { sdlgl_mouseloc.window = lost ? WIN_ERR : mouse_window; }
void sdlgl_internal_motion_handler(SDL_MouseMotionEvent *ev) {}
int sdlgl_internal_button_handler(SDL_MouseButtonEvent *ev) { return 0; }
int sdlgl_internal_key_handler(SDL_keysym *k, int repeat) { return 0; }
void sdlgl_remove_scrollback(void) {}
void sdlgl_adjust_scrollback(struct TextWindow *w, int dy) {}
void sdlgl_flush(void) {}
static void set_pan(struct TileWindow *w, int x, int y) { w->pan_x = x; w->pan_y = y; }

static void button(int type, int which)
{
  SDL_Event *e = &events[event_count++];
  memset(e, 0, sizeof *e);
  e->type = type;
  e->button.button = which;
  e->button.x = 100;
  e->button.y = 500;
}
static void motion(int x, int y, int held)
{
  SDL_Event *e = &events[event_count++];
  memset(e, 0, sizeof *e);
  e->type = SDL_MOUSEMOTION;
  e->motion.xrel = x;
  e->motion.yrel = y;
  e->motion.state = held ? SDL_BUTTON(SDL_BUTTON_RIGHT) : 0;
}
static void run(void)
{
  int x, y, mod;
  cur_key_sym = SDLK_LAST;
  SDL_Event *e = &events[event_count++];
  memset(e, 0, sizeof *e);
  e->type = SDL_KEYDOWN;
  e->key.keysym.sym = SDLK_ESCAPE;
  e->key.keysym.unicode = SDLK_ESCAPE;
  assert(sdlgl_get_poskey(0, &x, &y, &mod) == SDLK_ESCAPE);
  assert(event_pos == event_count);
  event_pos = event_count = 0;
}
int main(void)
{
  struct TileWindow base = {0};
  sdlgl_rend_procs.set_pan = set_pan;
  struct TextWindow win = {0};
  base.scale_w = base.scale_h = 32;
  base.total_w = 80; base.total_h = 21;
  base.scr_w = 1280; base.scr_h = 400;
  win.base = &base;
  win.map_px = 300; win.map_py = 100;
  text_wins[0] = &win;

  button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
  motion(13, 7, 1);
  button(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT);
  motion(20, 20, 1);
  run();
  assert(win.map_px == 287 && win.map_py == 107);
  assert(base.pan_x == 287 && base.pan_y == 107);

  button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
  motion(20, 20, 0);
  motion(20, 20, 1);
  run();
  assert(win.map_px == 287 && win.map_py == 107);

  button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
  events[event_count].type = SDL_ACTIVEEVENT;
  events[event_count++].active.gain = 0;
  motion(20, 20, 1);
  run();
  assert(win.map_px == 287 && win.map_py == 107);

  sdlgl_pan_map_pixels(-9999, -9999);
  assert(win.map_px == -64 && win.map_py == -64);
  sdlgl_pan_map_pixels(9999, 9999);
  assert(win.map_px == 1344 && win.map_py == 336);
  win.map_px = win.map_py = 0;
  sdlgl_pan_map_window(2, 2);
  assert(win.map_px == 192 && win.map_py == 128);
  {
    int x, y, mod;
    button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_MIDDLE);
    assert(sdlgl_get_poskey(0, &x, &y, &mod) == 0 && mod == CLICK_2);
    event_pos = event_count = 0;
    sdlgl_top_win = 0;
    button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
    assert(sdlgl_get_poskey(0, &x, &y, &mod) == 0 && mod == CLICK_2);
    event_pos = event_count = 0;
    sdlgl_top_win = WIN_ERR;
    mouse_window = WIN_ERR;
    button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
    assert(sdlgl_get_poskey(0, &x, &y, &mod) == 0 && mod == CLICK_2);
    event_pos = event_count = 0;
    motion(30, 30, 1);
    run();
    assert(win.map_px == 192 && win.map_py == 128);
  }
  base.is_text = 1;
  sdlgl_pan_map_pixels(25, 25);
  assert(win.map_px == 192 && win.map_py == 128);
  return 0;
}
