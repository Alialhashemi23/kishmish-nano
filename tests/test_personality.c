#include "../src/game.h"
#include "../src/personality.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *path = "build/personality-test.sav";
static const int code[] = {-1, 1, -1, 1, 3, 2};
static void fresh(void) {
    remove(path);
    game_init(path);
    personality_init(12345);
}
static void enter_code(void) {
    for (unsigned i = 0; i < sizeof code / sizeof *code; i++)
        game_key(code[i]);
}
static void secret_sequence(void) {
    fresh();
    Pet before = pet;
    enter_code();
    assert(game_take_sound() == 4);
    assert(pet.food == before.food && pet.joy == before.joy && pet.meals == before.meals);
    assert(personality_pose() == POSE_SECRET);
    assert(strstr(game_message(), "BURENYUU"));
    enter_code();
    assert(game_take_sound() == 0);
    assert(pet.food == before.food);
    game_tick(21);
    enter_code();
    assert(game_take_sound() == 4);
    /* Timeout, interruption and non-care contexts must never complete the code. */
    fresh();
    game_key(-1);
    game_tick(5);
    for (unsigned i = 1; i < sizeof code / sizeof *code; i++)
        game_key(code[i]);
    assert(game_take_sound() != 4);
    fresh();
    for (int i = 0; i < 5; i++)
        game_key(code[i]);
    game_key(4);
    game_key(4);
    game_key(2);
    assert(game_take_sound() != 4);
    fresh();
    game_key(1);
    game_key(2);
    enter_code();
    assert(game_take_sound() != 4);
    fresh();
    pet.sleeping = 1;
    enter_code();
    assert(game_take_sound() != 4);
    /* Up/down still navigate normally, but cannot impersonate left/right. */
    fresh();
    game_key(-2);
    game_key(5);
    game_key(-2);
    game_key(5);
    game_key(3);
    game_key(2);
    assert(game_take_sound() != 4);
    fresh();
    game_key(4);
    game_key(1);
    game_key(2);
    game_key(3);
    enter_code();
    assert(personality_pose() == POSE_SECRET);
    assert(game_take_sound() == 0);
}
static void ambient_and_reactions(void) {
    fresh();
    int seen[POSE_SECRET + 1] = {0}, lines = 0, neko = 0, last_neko = 0;
    Pet calm = pet;
    for (int i = 0; i < 72000; i++) {
        calm.chaos = i < 36000 ? 0 : 70;
        const char *line = personality_tick(.1, &calm, 0);
        seen[personality_pose()] = 1;
        if (line) {
            assert(strlen(line) <= 36);
            lines++;
            int is_neko = strstr(line, "NYA") || strstr(line, "MEOW") || strstr(line, "PURR");
            assert(!(last_neko && is_neko));
            last_neko = is_neko;
            neko += is_neko;
        }
    }
    for (int pose = POSE_BLINK; pose <= POSE_GREMLIN; pose++)
        assert(seen[pose]);
    assert(lines > 100 && lines < 260);
    assert(neko > 0 && neko < lines / 2);
    fresh();
    game_key(2);
    char reaction[40];
    snprintf(reaction, sizeof reaction, "%s", game_message());
    assert(personality_pose() == POSE_MUNCH);
    game_tick(10);
    assert(!strcmp(reaction, game_message()));
    game_key(-1);
    game_key(2);
    assert(personality_pose() == POSE_HAPPY);
    game_key(4);
    double frozen = personality_pose_time();
    game_tick(8);
    assert(personality_pose_time() == frozen);
}
static void save_compatibility_and_memory(void) {
    fresh();
    FILE *f = fopen(path, "w");
    assert(f);
    fprintf(f, "KISHMISH1 80 80 90 0 0 3 %lld\n", (long long)time(NULL));
    fclose(f);
    game_init(path);
    assert(pet.meals == 3 && pet.food == 80 && pet.affection == 0 && pet.visits == 1);
    game_key(-1);
    game_key(2);
    assert(pet.affection == 1);
    assert(game_save());
    game_init(path);
    assert(pet.affection == 1 && pet.visits == 2);
    pet.affection = 20;
    assert(game_save());
    game_init(path);
    assert(pet.affection == 20);
    assert(strstr(game_message(), "HUMAN"));
    /* The first eight fields retain the exact MVP meaning for old readers. */
    char magic[20];
    Pet legacy = {0};
    f = fopen(path, "r");
    assert(f);
    assert(fscanf(f, "%19s %d %d %d %d %d %d %lld", magic, &legacy.food, &legacy.joy,
                  &legacy.energy, &legacy.chaos, &legacy.sleeping, &legacy.meals,
                  &legacy.stamp) == 8);
    fclose(f);
    assert(!strcmp(magic, "KISHMISH1") && legacy.food == pet.food);
    /* A malformed extension cannot invalidate a valid legacy pet. */
    f = fopen(path, "w");
    assert(f);
    fprintf(f, "KISHMISH1 70 60 50 0 0 2 %lld -5 999999999\n", (long long)time(NULL));
    fclose(f);
    game_init(path);
    assert(pet.food == 70 && pet.affection == 0 && pet.visits == 1);
}
static void animation_rendering(void) {
    fresh();
    game_draw();
    uint32_t menu[240 * 22];
    memcpy(menu, pixels + 197 * 240, sizeof menu);
    uint32_t idle[240 * 240];
    memcpy(idle, pixels, sizeof idle);
    for (int pose = POSE_BLINK; pose <= POSE_SECRET; pose++) {
        personality_pose_start((CatPose)pose);
        personality_tick(.14, &pet, 0);
        game_draw();
        assert(!memcmp(menu, pixels + 197 * 240, sizeof menu));
        assert(memcmp(idle, pixels, sizeof idle));
        for (int i = 0; i < 240 * 240; i++)
            assert(pixels[i] == 0xe0edb3 || pixels[i] == 0xa9bf70 || pixels[i] == 0x52734b ||
                   pixels[i] == 0x203c36);
    }
}
int main(void) {
    secret_sequence();
    ambient_and_reactions();
    save_compatibility_and_memory();
    animation_rendering();
    remove(path);
    puts("PASS: secret isolation/timeout/cooldown/mute, ambient frequency, seven idle poses, "
         "pause, save compatibility and affection memory");
    return 0;
}
