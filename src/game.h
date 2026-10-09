#ifndef GAME_H
#define GAME_H
#include <stdint.h>
typedef struct {
    int food, joy, energy, chaos, sleeping, meals;
    long long stamp;
} Pet;
extern Pet pet;
extern uint32_t pixels[240 * 240];
void game_init(const char *path);
void game_tick(double dt);
void game_key(int key); /* -1 left, 1 right, 2 confirm, 3 back, 4 pause */
void game_draw(void);
int game_save(void);
int game_take_sound(void); /* 1 snack, 2 affection, 3 play; 0 none */
int game_should_quit(void);
int game_sound_enabled(void);
#endif
