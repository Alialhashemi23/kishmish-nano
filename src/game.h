#ifndef GAME_H
#define GAME_H
#include <stdint.h>
typedef struct {
    int food, joy, energy, chaos, sleeping, meals;
    long long stamp;
    int affection, visits; /* Optional save extension; no additional care meter. */
} Pet;
extern Pet pet;
extern uint32_t pixels[240 * 240];
void game_init(const char *path);
void game_tick(double dt);
void game_key(int key); /* -1 left, 1 right, 2 confirm, 3 back, 4 pause, -2 up, 5 down */
void game_draw(void);
int game_save(void);
int game_take_sound(void); /* 1 snack, 2 affection, 3 play, 4 secret homage; 0 none */
const char *game_message(void);
int game_should_quit(void);
int game_sound_enabled(void);
#endif
