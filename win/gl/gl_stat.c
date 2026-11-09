/* NetHack may be freely redistributed. See license for details. */
#include "hack.h"

#if defined(GL_GRAPHICS) || defined(SDL_GRAPHICS)
#define WINGL_INTERNAL
#include "winGL.h"

static int hp, hpmax, power, powermax;

void Sdlgl_status_enablefield(int field, const char *name, const char *format,
                             BOOLEAN_P enabled)
{
  genl_status_enablefield(field, name,
                         field == BL_TITLE ? "[%-30.30s]" : format, enabled);
}

void Sdlgl_status_update(int field, genericptr_t value, int change,
                        int percent, int color, unsigned long *masks)
{
  struct TextWindow *win;
  char text[MAXCO];
  int i, width, filled, hp_percent, power_color, count;
  char *start, *end;
  genl_status_update(field, value, change, percent, color, masks);
  /* Percentage callbacks are optional unless a highlight rule needs them. */
  switch (field) {
    case BL_HP: hp = atoi((char *) value); break;
    case BL_HPMAX: hpmax = atoi((char *) value); break;
    case BL_ENE: power = atoi((char *) value); break;
    case BL_ENEMAX: powermax = atoi((char *) value); break;
  }
  if (field != BL_FLUSH && field != BL_RESET)
    return;
  if (WIN_STATUS == WIN_ERR || !(win = text_wins[WIN_STATUS]) || !win->base)
    return;

  hp_percent = hpmax > 0 ? max(0, min(100, (int) (100L * hp / hpmax))) : 0;
  color = hp_percent <= 20 ? STATUS_DANGER : hp_percent <= 50 ? STATUS_CAUTION : STATUS_HEALTHY;
  width = min(30, win->show_w - 1);
  filled = width * hp_percent / 100;
  if (hp_percent > 0 && filled == 0)
    filled = 1;
  if (hp_percent < 100 && filled == width)
    --filled;
  for (i = 0; i < width; ++i) {
    struct TilePair *tile = &win->base->tiles[(win->show_h - 1) * win->show_w + i + 1];
    int active = i < filled;
    sdlgl_store_char(win->base, i + 1, win->show_h - 1,
                     TILE_2_CHAR(tile->fg),
                     active ? (color << 8) | BLACK : L_GREY);
  }

  /* Find the rendered power field, including when it wraps onto another row. */
  count = min(MAXCO - 1, win->show_w * (win->show_h - 1));
  for (i = 0; i < count; ++i)
    text[i] = TILE_2_CHAR(win->base->tiles[
        (win->show_h - 2 - i / win->show_w) * win->show_w + i % win->show_w].fg);
  text[count] = '\0';
  start = strstr(text, "Pw:");
  end = start ? strchr(start, ')') : NULL;
  power_color = powermax > 0 && 100L * power <= 20L * powermax ? STATUS_DANGER
                : powermax > 0 && 100L * power <= 50L * powermax ? STATUS_CAUTION : L_GREY;
  if (end)
    for (i = (int) (start - text); i <= (int) (end - text); ++i)
      sdlgl_store_char(win->base, i % win->show_w,
                       win->show_h - 2 - i / win->show_w, text[i], power_color);
}
#endif
