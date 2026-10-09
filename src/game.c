#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
Pet pet;
uint32_t pixels[240 * 240];
static const uint32_t pal[] = {0xe0edb3, 0xa9bf70, 0x52734b, 0x203c36};
static char savepath[1024], message[40] = "HELLO. I AM KISHMISH.";
static int selected, star_selected, frame, playing, target = 1, pending_sound;
static int paused, pause_selected, quit_requested, sound_enabled = 1;
int game_should_quit(void) {
    return quit_requested;
}
int game_sound_enabled(void) {
    return sound_enabled;
}
int game_take_sound(void) {
    int s = pending_sound;
    pending_sound = 0;
    return sound_enabled ? s : 0;
}
static double elapsed, anim, autosave;
static int clamp(int n) {
    return n < 0 ? 0 : n > 100 ? 100 : n;
}
static void rect(int x, int y, int w, int h, int c) {
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++)
            if (i >= 0 && j >= 0 && i < 240 && j < 240)
                pixels[j * 240 + i] = pal[c];
}
/* Five-column bitmap glyphs, rendered without external fonts or assets. */
static const unsigned char font[][5] = {
    {126, 17, 17, 17, 126}, {127, 73, 73, 73, 54}, {62, 65, 65, 65, 34},  {127, 65, 65, 34, 28},
    {127, 73, 73, 73, 65},  {127, 9, 9, 9, 1},     {62, 65, 73, 73, 122}, {127, 8, 8, 8, 127},
    {0, 65, 127, 65, 0},    {32, 64, 65, 63, 1},   {127, 8, 20, 34, 65},  {127, 64, 64, 64, 64},
    {127, 2, 12, 2, 127},   {127, 4, 8, 16, 127},  {62, 65, 65, 65, 62},  {127, 9, 9, 9, 6},
    {62, 65, 81, 33, 94},   {127, 9, 25, 41, 70},  {70, 73, 73, 73, 49},  {1, 1, 127, 1, 1},
    {63, 64, 64, 64, 63},   {31, 32, 64, 32, 31},  {63, 64, 56, 64, 63},  {99, 20, 8, 20, 99},
    {7, 8, 112, 8, 7},      {97, 81, 73, 69, 67},  {62, 81, 73, 69, 62},  {0, 66, 127, 64, 0},
    {98, 81, 73, 73, 70},   {34, 65, 73, 73, 54},  {24, 20, 18, 127, 16}, {39, 69, 69, 69, 57},
    {60, 74, 73, 73, 48},   {1, 113, 9, 5, 3},     {54, 73, 73, 73, 54},  {6, 73, 73, 41, 30}};
static void text(int x, int y, const char *s, int c) {
    for (; *s; s++, x += 6) {
        int k = *s >= 'A' && *s <= 'Z' ? *s - 'A' : *s >= '0' && *s <= '9' ? 26 + *s - '0' : -1;
        if (k >= 0) {
            for (int i = 0; i < 5; i++)
                for (int j = 0; j < 7; j++)
                    if (font[k][i] & (1 << j))
                        rect(x + i, y + j, 1, 1, c);
        } else if (*s == '.')
            rect(x + 2, y + 6, 1, 1, c);
        else if (*s == '!') {
            rect(x + 2, y, 1, 5, c);
            rect(x + 2, y + 6, 1, 1, c);
        } else if (*s == '-')
            rect(x, y + 3, 5, 1, c);
    }
}
static void say(const char *s) {
    snprintf(message, sizeof message, "%s", s);
}
int game_save(void) {
    char tmp[1100];
    snprintf(tmp, sizeof tmp, "%s.tmp", savepath);
    FILE *f = fopen(tmp, "w");
    if (!f)
        return 0;
    pet.stamp = (long long)time(NULL);
    int ok = fprintf(f, "KISHMISH1 %d %d %d %d %d %d %lld\n", pet.food, pet.joy, pet.energy,
                     pet.chaos, pet.sleeping, pet.meals, pet.stamp) > 0;
    if (fclose(f))
        ok = 0;
    if (!ok)
        return 0;
#ifdef _WIN32
    remove(savepath);
#endif
    return rename(tmp, savepath) == 0;
}
void game_init(const char *path) {
    snprintf(savepath, sizeof savepath, "%s", path);
    pet = (Pet){80, 80, 90, 0, 0, 0, (long long)time(NULL)};
    FILE *f = fopen(path, "r");
    Pet p;
    char magic[20];
    if (f) {
        int n = fscanf(f, "%19s %d %d %d %d %d %d %lld", magic, &p.food, &p.joy, &p.energy,
                       &p.chaos, &p.sleeping, &p.meals, &p.stamp);
        fclose(f);
        if (n == 8 && !strcmp(magic, "KISHMISH1") && p.food >= 0 && p.food <= 100 && p.joy >= 0 &&
            p.joy <= 100 && p.energy >= 0 && p.energy <= 100 && p.chaos >= 0 && p.chaos <= 100 &&
            (p.sleeping == 0 || p.sleeping == 1) && p.meals >= 0) {
            pet = p;
            long long gap = (long long)time(NULL) - pet.stamp;
            if (gap < 0)
                gap = 0;
            if (gap > 28800)
                gap = 28800;
            pet.food = clamp(pet.food - (int)(gap / 180));
            pet.joy = clamp(pet.joy - (int)(gap / 240));
            pet.energy = clamp(pet.energy + (pet.sleeping ? 1 : -1) * (int)(gap / 120));
            say("YOU CAME BACK. GOOD.");
        }
    }
    elapsed = anim = autosave = 0;
    frame = selected = star_selected = playing = pending_sound = paused = pause_selected =
        quit_requested = 0;
    sound_enabled = 1;
}
void game_tick(double dt) {
    if (paused)
        return;
    anim += dt;
    frame = (int)(anim * 6);
    elapsed += dt;
    autosave += dt;
    while (elapsed >= 30) {
        elapsed -= 30;
        pet.food = clamp(pet.food - 1);
        pet.joy = clamp(pet.joy - 1);
        pet.energy = clamp(pet.energy + (pet.sleeping ? 4 : -1));
        if (pet.food < 25 || pet.joy < 25)
            pet.chaos = clamp(pet.chaos + 2);
    }
    if (autosave >= 30) {
        autosave = 0;
        if (!game_save())
            say("SAVE FAILED.");
    }
}
void game_key(int key) {
    if (key == 4) {
        paused = !paused;
        pause_selected = 0;
        pending_sound = 0;
        return;
    }
    if (paused) {
        if (key == 3) {
            paused = 0;
            return;
        }
        if (key == -1 || key == 1)
            pause_selected = (pause_selected + (key < 0 ? 3 : 1)) % 4;
        if (key == 2) {
            if (pause_selected == 0)
                paused = 0;
            else if (pause_selected == 1) {
                sound_enabled = !sound_enabled;
                pending_sound = 0;
            } else if (pause_selected == 2) {
                if (game_save())
                    say("PROGRESS SAVED.");
                else
                    say("SAVE FAILED.");
            } else {
                if (game_save())
                    quit_requested = 1;
                else
                    say("SAVE FAILED.");
            }
        }
        return;
    }
    if (playing) {
        if (key == 3) {
            playing = 0;
            return;
        }
        if (key == -1 || key == 1)
            star_selected = (star_selected + (key < 0 ? 3 : 1)) % 4;
        if (key == 2) {
            if (star_selected == target) {
                pet.joy = clamp(pet.joy + 12);
                pet.energy = clamp(pet.energy - 3);
                pet.chaos = clamp(pet.chaos - 2);
                say(pet.chaos > 30 ? "I SPARED THE MOON." : "YOU FOUND MY STAR!");
            } else {
                pet.joy = clamp(pet.joy + 2);
                say("A VERY GOOD WRONG STAR.");
            }
            pending_sound = 3;
            playing = 0;
            game_save();
        }
        return;
    }
    if (key == -1 || key == 1)
        selected = (selected + (key < 0 ? 3 : 1)) % 4;
    if (key != 2)
        return;
    if (selected == 2) {
        pet.sleeping = !pet.sleeping;
        say(pet.sleeping ? "DO NOT TRUST THE MOON." : "I DREAMED OF SNACKS.");
    } else if (pet.sleeping)
        say("ZZZ. COME BACK SOON.");
    else if (selected == 0) {
        if (pet.food > 94)
            say("FULL. EMOTIONALLY EMPTY.");
        else {
            pet.food = clamp(pet.food + 18);
            pet.meals++;
            pending_sound = 1;
            pet.chaos = clamp(pet.chaos + 1);
            say(pet.chaos > 30 ? "ANOTHER OFFERING. GOOD." : "A SNACK! FOR ME?!");
        }
    } else if (selected == 1) {
        playing = 1;
        star_selected = 0;
        target = rand() % 4;
        say("FIND THE TWINKLING STAR!");
    } else {
        pending_sound = 2;
        pet.joy = clamp(pet.joy + 5);
        pet.chaos = clamp(pet.chaos - 1);
        say(pet.chaos > 30 ? "YOUR SECRETS ARE SAFE." : "YOU ARE MY FAVORITE.");
    }
    game_save();
}
static void bar(int y, const char *s, int value) {
    text(12, y, s, 3);
    rect(57, y, 54, 7, 3);
    rect(58, y + 1, 52, 5, 0);
    rect(58, y + 1, value * 52 / 100, 5, 2);
}
void game_draw(void) {
    rect(0, 0, 240, 240, 0);
    rect(5, 5, 230, 230, 3);
    rect(7, 7, 226, 226, 0);
    text(12, 13, "KISHMISH NANO", 3);
    text(174, 13, pet.sleeping ? "ASLEEP" : "AWAKE", 2);
    rect(12, 26, 216, 1, 2);
    bar(33, "FOOD", pet.food);
    bar(44, "JOY", pet.joy);
    bar(55, "REST", pet.energy);
    text(132, 34,
         pet.chaos > 60   ? "FERAL KITTY"
         : pet.chaos > 25 ? "ODD LITTLE GUY"
                          : "SWEET LITTLE GUY",
         2);
    text(132, 48, "DAY ONE FRIEND", 2);
    /* Code-drawn Siamese kitten: pointed ears, seal mask, cream bib and curled tail. */
    rect(13, 72, 214, 100, 1);
    rect(17, 76, 206, 92, 0);
    rect(28, 84, 28, 30, 3);
    rect(30, 86, 24, 26, 1);
    rect(41, 86, 2, 26, 3);
    rect(30, 98, 24, 2, 3);
    rect(188, 125, 14, 27, 2);
    rect(181, 123, 28, 7, 2);
    rect(192, 117, 5, 9, 3);
    for (int x = 21; x < 220; x += 16)
        rect(x, 160, 8, 1, 1);
    int bob = pet.sleeping ? 2 : (frame % 8 < 4 ? 0 : -2), y = 83 + bob;
    rect(82, 157, 88, 3, 1);
    /* Curled dark tail behind a small seated body. */
    rect(143, y + 49, 19, 8, 3);
    rect(157, y + 34, 8, 20, 3);
    rect(153, y + 29, 10, 8, 3);
    rect(149, y + 30, 6, 5, 2);
    rect(160, y + 39, 3, 9, 2);
    rect(99, y + 44, 42, 29, 3);
    rect(103, y + 44, 34, 26, 1);
    rect(108, y + 47, 24, 22, 0);
    /* Short triangular ears, stepped at the same pixel scale as the face. */
    for (int j = 0; j < 10; j++) {
        rect(88, y + j * 2, 4 + j * 2, 2, 3);
        rect(148 - j * 2, y + j * 2, 4 + j * 2, 2, 3);
    }
    for (int j = 0; j < 6; j++) {
        rect(92, y + 6 + j * 2, 2 + j * 2, 2, 1);
        rect(146 - j * 2, y + 6 + j * 2, 2 + j * 2, 2, 1);
    }
    /* Broad kitten cheeks, pale outer fur, pointed Siamese face mask. */
    rect(94, y + 15, 52, 39, 3);
    rect(86, y + 23, 68, 23, 3);
    rect(90, y + 21, 60, 25, 1);
    rect(96, y + 18, 48, 33, 1);
    rect(105, y + 19, 30, 6, 2);
    rect(99, y + 25, 42, 20, 2);
    rect(105, y + 43, 30, 8, 2);
    rect(112, y + 47, 16, 6, 2);
    /* Large bright eyes with dark pupils and tiny catchlights. */
    if (pet.sleeping) {
        rect(101, y + 33, 12, 2, 3);
        rect(127, y + 33, 12, 2, 3);
        rect(101, y + 31, 2, 2, 3);
        rect(137, y + 31, 2, 2, 3);
        text(161, y + 5, "Z", 3);
        text(174, y - 4, "Z", 2);
    } else {
        for (int x = 101; x <= 127; x += 26) {
            rect(x, y + 27, 12, 13, 3);
            rect(x + 2, y + 29, 8, 9, 0);
            rect(x + 6, y + 30, 3, 8, 3);
            rect(x + 3, y + 29, 3, 3, 0);
        }
    }
    /* Two pale muzzle pads, triangle nose and a little W mouth. */
    rect(110, y + 40, 10, 8, 0);
    rect(120, y + 40, 10, 8, 0);
    rect(117, y + 40, 6, 2, 3);
    rect(119, y + 42, 2, 3, 3);
    rect(115, y + 46, 4, 2, 3);
    rect(121, y + 46, 4, 2, 3);
    /* Whiskers extend past the cheek silhouette. */
    rect(76, y + 35, 17, 2, 3);
    rect(79, y + 42, 14, 2, 3);
    rect(147, y + 35, 17, 2, 3);
    rect(147, y + 42, 14, 2, 3);
    /* Seal-point front paws, pale toes, and a cream chest. */
    rect(101, y + 64, 16, 10, 3);
    rect(123, y + 64, 16, 10, 3);
    rect(104, y + 65, 10, 5, 2);
    rect(126, y + 65, 10, 5, 2);
    rect(108, y + 71, 2, 3, 1);
    rect(130, y + 71, 2, 3, 1);
    rect(118, y + 56, 4, 14, 2);
    text(12, 179, message, 3);
    const char *items[] = {"FEED", "PLAY", "SLEEP", "PET"};
    for (int i = 0; i < 4; i++) {
        int x = 12 + i * 55;
        rect(x, 197, 51, 21, i == selected ? 3 : 1);
        text(x + 7, 204, items[i], i == selected ? 0 : 3);
    }
    if (playing) {
        rect(19, 79, 202, 87, 3);
        rect(21, 81, 198, 83, 0);
        text(31, 88, "PICK THE TWINKLING STAR", 3);
        static const char *star[] = {".......#.......", "......###......", "......###......",
                                     ".....#####.....", "###############", ".#############.",
                                     "..###########..", "...#########...", "....#######....",
                                     "...#########...", "...####.####...", "..####...####..",
                                     "..###.....###..", ".##.........##.", ".#...........#."};
        for (int i = 0; i < 4; i++) {
            int x = 36 + i * 46;
            rect(x - 6, 108, 30, 38, i == star_selected ? 1 : 0);
            if (i == star_selected) {
                rect(x - 6, 108, 30, 1, 3);
                rect(x - 6, 145, 30, 1, 3);
            }
            for (int sy = 0; sy < 15; sy++)
                for (int sx = 0; sx < 15; sx++)
                    if (star[sy][sx] == '#')
                        rect(x + sx, 117 + sy, 1, 1, (i == target && frame % 4 < 2) ? 2 : 3);
            if (i == target && frame % 4 < 2) {
                rect(x - 4, 113, 2, 2, 3);
                rect(x + 20, 135, 2, 2, 3);
                rect(x + 19, 111, 1, 4, 2);
                rect(x + 18, 112, 3, 1, 2);
            }
        }
        text(42, 151, "A PICK     B BACK", 2);
    }
    text(16, 224, "L R MOVE  A PICK  START PAUSE", 2);
    if (paused) {
        rect(24, 74, 192, 101, 3);
        rect(26, 76, 188, 97, 0);
        text(99, 83, "PAUSED", 3);
        const char *options[] = {"RESUME", sound_enabled ? "SOUND ON" : "SOUND OFF", "SAVE NOW",
                                 "SAVE AND QUIT"};
        for (int i = 0; i < 4; i++) {
            rect(38, 97 + i * 17, 164, 15, i == pause_selected ? 3 : 0);
            text(49, 101 + i * 17, options[i], i == pause_selected ? 0 : 3);
        }
    }
}
