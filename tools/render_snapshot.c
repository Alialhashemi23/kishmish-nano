#include "../src/game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: render-snapshot SCENE OUTPUT.ppm [seconds]\n");
        return 1;
    }
    remove("build/preview-never-created.sav");
    game_init("build/preview-never-created.sav");
    srand(7);
    if (!strcmp(argv[1], "sleep"))
        pet.sleeping = 1;
    if (!strcmp(argv[1], "chaos"))
        pet.chaos = 70;
    if (!strcmp(argv[1], "stars")) {
        game_key(1);
        game_key(2);
    }
    if (argc > 3)
        game_tick(atof(argv[3]));
    if (!strcmp(argv[1], "pause"))
        game_key(4);
    game_draw();
    FILE *f = fopen(argv[2], "wb");
    if (!f)
        return 1;
    fprintf(f, "P6\n240 240\n255\n");
    for (int i = 0; i < 240 * 240; i++) {
        fputc(pixels[i] >> 16, f);
        fputc(pixels[i] >> 8, f);
        fputc(pixels[i], f);
    }
    int status = fclose(f) ? 1 : 0;
    remove("build/preview-never-created.sav");
    return status;
}
