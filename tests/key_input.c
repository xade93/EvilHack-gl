/* Compile with -ffunction-sections -fdata-sections -Wl,--gc-sections so
 * only the real key converter and its dependencies are linked.
 */
#include "../win/gl/gl_main.c"

struct instance_flags iflags;
boolean letter(CHAR_P ch)
{
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '@';
}

int main(void)
{
  int ch;
  for (ch = 'a'; ch <= 'z'; ++ch) {
    assert(convert_keysym(ch, ch, KMOD_LCTRL) == C(ch));
    assert(convert_keysym(ch, ch, KMOD_RCTRL) == C(ch));
    assert(convert_keysym(ch, ch - 'a' + 'A', KMOD_CTRL | KMOD_SHIFT) == C(ch));
    assert(convert_keysym(ch, C(ch), KMOD_CTRL) == C(ch));
    assert(convert_keysym(ch, 0, KMOD_CTRL) == C(ch));
    assert(convert_keysym(ch, ch, 0) == ch);
    assert(convert_keysym(ch, ch, KMOD_ALT) == M(ch));
  }
  assert(convert_keysym(SDLK_d, 'd', KMOD_CTRL) == 4); /* kick, not drop */
  assert(convert_keysym(SDLK_UP, 0, 0) == 'k');
  assert(convert_keysym(SDLK_UP, 0, KMOD_SHIFT) == 'K');
  assert(convert_keysym(SDLK_LCTRL, 0, KMOD_CTRL) == 0);
  return 0;
}
