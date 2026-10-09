#include "../src/game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static const char *path = "build/test-pet.sav";
static void fresh(void) {
    remove(path);
    game_init(path);
}
static void care_and_saves(void) {
    fresh();
    assert(pet.food == 80);
    game_key(2);
    assert(pet.food == 98);
    assert(game_take_sound() == 1);
    assert(game_take_sound() == 0);
    game_key(1);
    game_key(1);
    game_key(2);
    assert(pet.sleeping);
    pet.energy = 50;
    game_tick(30);
    assert(pet.energy == 54);
    assert(game_save());
    game_init(path);
    assert(pet.sleeping && pet.energy == 54);
    FILE *f = fopen(path, "w");
    assert(f);
    fputs("corrupt", f);
    fclose(f);
    game_init(path);
    assert(pet.food == 80);
    f = fopen(path, "w");
    assert(f);
    fprintf(f, "KISHMISH1 100 100 100 0 0 0 %lld\n", (long long)time(NULL) - 999999);
    fclose(f);
    game_init(path);
    assert(pet.food == 0 && pet.joy == 0 && pet.energy == 0);
    f = fopen(path, "w");
    assert(f);
    fprintf(f, "KISHMISH1 50 50 50 0 0 0 %lld\n", (long long)time(NULL) + 3600);
    fclose(f);
    game_init(path);
    assert(pet.food == 50 && pet.energy == 50);
}
static void minigame(void) {
    fresh();
    game_key(1);
    game_key(2);
    game_draw();
    uint32_t menu[240 * 22];
    memcpy(menu, pixels + 197 * 240, sizeof menu);
    game_key(1);
    game_key(1);
    game_draw();
    assert(!memcmp(menu, pixels + 197 * 240, sizeof menu));
    game_key(3);
    game_key(2);
    game_key(2);
    assert(game_take_sound() == 3);
    game_key(1);
    game_key(1);
    game_key(2);
    assert(game_take_sound() == 2);
}
static void pause_and_quit(void) {
    fresh();
    Pet before = pet;
    game_key(4);
    game_tick(120);
    assert(pet.food == before.food && pet.joy == before.joy && pet.energy == before.energy);
    game_key(1);
    game_key(2);
    assert(!game_sound_enabled());
    game_key(1);
    game_key(2);
    FILE *f = fopen(path, "r");
    assert(f);
    fclose(f);
    game_key(3);
    game_key(2);
    assert(pet.food > before.food);
    assert(game_take_sound() == 0);
    game_key(4);
    game_key(1);
    game_key(1);
    game_key(1);
    game_key(2);
    assert(game_should_quit());
    game_init(path);
    assert(pet.food > before.food && !game_should_quit());
}
static void failed_save_keeps_game_open(void) {
    game_init("build/directory-that-does-not-exist/pet.sav");
    game_key(4);
    game_key(1);
    game_key(1);
    game_key(1);
    game_key(2);
    assert(!game_should_quit());
}
int main(void) {
    care_and_saves();
    minigame();
    pause_and_quit();
    failed_save_keeps_game_open();
    remove(path);
    puts("PASS: care, saves, corrupt recovery, offline clock bounds, minigame isolation, sounds, "
         "pause and save failure");
    return 0;
}
