/* Exercise the real SDL input loop, WAV loading, callbacks, pause and quit on Linux.
 * Run from build/ with SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy and a test HOME.
 */
#define _POSIX_C_SOURCE 200809L
#define main kishmish_app_main
#include "../src/main.c"
#undef main
#include <assert.h>

static void press(SDLKey key) {
    SDL_Event event;
    memset(&event, 0, sizeof event);
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = key;
    assert(SDL_PushEvent(&event) == 0);
    SDL_Delay(50);
}
static void code(void) {
    press(SDLK_l);
    press(SDLK_r);
    press(SDLK_l);
    press(SDLK_r);
    press(SDLK_b);
    press(SDLK_a);
}
static int inputs(void *unused) {
    (void)unused;
    SDL_Delay(300);
    code();
    SDL_Delay(1400); /* Let the complete original vocal effect play. */
    press(SDLK_s);
    press(SDLK_r);
    press(SDLK_a);
    press(SDLK_b); /* Mute and resume. */
    press(SDLK_l);
    press(SDLK_a); /* Pet while muted. */
    press(SDLK_s);
    press(SDLK_r);
    press(SDLK_r);
    press(SDLK_r);
    press(SDLK_a);
    return 0;
}
int main(void) {
    mkdir("smoke-home", 0755);
    assert(setenv("HOME", "./smoke-home", 1) == 0);
    remove("smoke-home/.kishmish-nano/pet.sav");
    assert(SDL_Init(SDL_INIT_TIMER) == 0);
    SDL_Thread *thread = SDL_CreateThread(inputs, NULL);
    assert(thread);
    int result = kishmish_app_main();
    SDL_WaitThread(thread, NULL);
    assert(result == 0);
    for (int i = 0; i < 4; i++)
        assert(lengths[i] > 1000);
    assert(pet.affection == 1);
    assert(game_should_quit());
    assert(!game_sound_enabled());
    puts("PASS: SDL boot, four WAVs, secret playback, muted affection, Start pause and saved quit");
    return 0;
}
