#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
static char sounddir[1024];
static void play_meow(int id) {
    const char *names[] = {"", "snack", "pet", "play", "secret"};
    char wav[1200];
    if (!game_sound_enabled()) {
        PlaySoundA(NULL, NULL, 0);
        return;
    }
    if (id < 1 || id > 4)
        return;
    snprintf(wav, sizeof wav, "%s%s.wav", sounddir, names[id]);
    PlaySoundA(wav, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}
static LRESULT CALLBACK proc(HWND w, UINT m, WPARAM k, LPARAM l) {
    if (m == WM_CLOSE) {
        if (!game_save())
            MessageBoxA(w, "Could not save Kishmish.", "Save failed", MB_OK);
        DestroyWindow(w);
        return 0;
    }
    if (m == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    if (m == WM_KEYDOWN && !(l & (1L << 30))) {
        if (k == VK_ESCAPE || k == 'P')
            game_key(4);
        if (k == VK_LEFT || k == 'A')
            game_key(-1);
        if (k == VK_UP)
            game_key(-2);
        if (k == VK_DOWN)
            game_key(5);
        if (k == VK_RIGHT || k == 'D')
            game_key(1);
        if (k == VK_RETURN || k == VK_SPACE || k == 'Z')
            game_key(2);
        if (k == 'X' || k == VK_BACK)
            game_key(3);
    }
    return DefWindowProc(w, m, k, l);
}
int main(int argc, char **argv) {
    char path[1024];
    GetModuleFileNameA(NULL, path, sizeof path);
    char *p = strrchr(path, '\\');
    if (p)
        strcpy(p + 1, "kishmish.sav");
    snprintf(sounddir, sizeof sounddir, "%s", path);
    char *sd = strrchr(sounddir, '\\');
    if (sd)
        strcpy(sd + 1, "sounds\\");
    game_init(path);
    srand((unsigned)time(NULL));
    if (argc > 1 && (!strcmp(argv[1], "--snapshot") || !strcmp(argv[1], "--stars") ||
                     !strcmp(argv[1], "--pause"))) {
        if (!strcmp(argv[1], "--stars")) {
            game_key(1);
            game_key(2);
        }
        if (!strcmp(argv[1], "--pause"))
            game_key(4);
        game_draw();
        FILE *f = fopen("preview.ppm", "wb");
        if (!f)
            return 1;
        fprintf(f, "P6\n240 240\n255\n");
        for (int i = 0; i < 240 * 240; i++) {
            fputc(pixels[i] >> 16, f);
            fputc(pixels[i] >> 8, f);
            fputc(pixels[i], f);
        }
        fclose(f);
        return 0;
    }
    WNDCLASSA c = {0};
    c.lpfnWndProc = proc;
    c.hInstance = GetModuleHandle(NULL);
    c.lpszClassName = "Kishmish";
    c.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&c);
    RECT r = {0, 0, 720, 720};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    HWND w = CreateWindowA("Kishmish", "Kishmish Nano - arrows / Z / X",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                           r.right - r.left, r.bottom - r.top, NULL, NULL, c.hInstance, NULL);
    if (!w)
        return 1;
    BITMAPINFO b = {0};
    b.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    b.bmiHeader.biWidth = 240;
    b.bmiHeader.biHeight = -240;
    b.bmiHeader.biPlanes = 1;
    b.bmiHeader.biBitCount = 32;
    b.bmiHeader.biCompression = BI_RGB;
    DWORD last = GetTickCount();
    MSG msg;
    for (;;) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        DWORD now = GetTickCount();
        if (game_should_quit()) {
            SendMessage(w, WM_CLOSE, 0, 0);
            continue;
        }
        game_tick((now - last) / 1000.0);
        play_meow(game_take_sound());
        last = now;
        game_draw();
        HDC dc = GetDC(w);
        GetClientRect(w, &r);
        SetStretchBltMode(dc, COLORONCOLOR);
        StretchDIBits(dc, 0, 0, r.right, r.bottom, 0, 0, 240, 240, pixels, &b, DIB_RGB_COLORS,
                      SRCCOPY);
        ReleaseDC(w, dc);
        Sleep(33);
    }
}
#else
#include <SDL/SDL.h>
#include <sys/stat.h>
static Uint8 *clips[4];
static Uint32 lengths[4], audio_pos, audio_len;
static Uint8 *audio_data;
static int audio_ready;
static void audio_callback(void *unused, Uint8 *stream, int len) {
    (void)unused;
    memset(stream, 0, (size_t)len);
    if (!audio_data)
        return;
    Uint32 n = audio_len - audio_pos;
    if (n > (Uint32)len)
        n = (Uint32)len;
    memcpy(stream, audio_data + audio_pos, n);
    audio_pos += n;
}
static void audio_init(void) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Audio unavailable: %s\n", SDL_GetError());
        return;
    }
    SDL_AudioSpec want;
    memset(&want, 0, sizeof want);
    want.freq = 22050;
    want.format = AUDIO_S16LSB;
    want.channels = 1;
    want.samples = 512;
    want.callback = audio_callback;
    if (SDL_OpenAudio(&want, NULL) < 0) {
        fprintf(stderr, "Audio open failed: %s\n", SDL_GetError());
        return;
    }
    const char *names[] = {"sounds/snack.wav", "sounds/pet.wav", "sounds/play.wav",
                           "sounds/secret.wav"};
    for (int i = 0; i < 4; i++) {
        SDL_AudioSpec spec;
        if (!SDL_LoadWAV(names[i], &spec, &clips[i], &lengths[i])) {
            fprintf(stderr, "Meow load failed: %s\n", SDL_GetError());
            continue;
        }
        if (spec.freq != 22050 || spec.format != AUDIO_S16LSB || spec.channels != 1) {
            SDL_FreeWAV(clips[i]);
            clips[i] = NULL;
            lengths[i] = 0;
        }
    }
    audio_ready = 1;
    SDL_PauseAudio(0);
    fprintf(stderr, "Meow audio ready\n");
}
static void play_meow(int id) {
    if (!audio_ready)
        return;
    if (!game_sound_enabled()) {
        SDL_LockAudio();
        audio_data = NULL;
        SDL_UnlockAudio();
        return;
    }
    if (id < 1 || id > 4)
        return;
    SDL_LockAudio();
    audio_data = clips[id - 1];
    audio_len = lengths[id - 1];
    audio_pos = 0;
    SDL_UnlockAudio();
}
static void audio_close(void) {
    if (audio_ready)
        SDL_CloseAudio();
    for (int i = 0; i < 4; i++)
        if (clips[i])
            SDL_FreeWAV(clips[i]);
}

int main(void) {
    char path[1024];
    const char *home = getenv("HOME");
    if (!home)
        home = "/tmp";
    snprintf(path, sizeof path, "%s/.kishmish-nano", home);
    mkdir(path, 0755);
    snprintf(path, sizeof path, "%s/.kishmish-nano/pet.sav", home);
    game_init(path);
    srand((unsigned)time(NULL));
    fprintf(stderr, "Kishmish: starting SDL\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Surface *s = SDL_SetVideoMode(240, 240, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
    if (!s) {
        fprintf(stderr, "Video mode failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    fprintf(stderr, "Video: %dx%d %d bpp pitch %d\n", s->w, s->h, s->format->BitsPerPixel,
            s->pitch);
    if (s->w < 240 || s->h < 240) {
        fprintf(stderr, "Unsupported surface size\n");
        SDL_Quit();
        return 1;
    }
    SDL_ShowCursor(0);
    audio_init();
    int run = 1;
    Uint32 last = SDL_GetTicks();
    while (run) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                run = 0;
            if (e.type == SDL_KEYDOWN) {
                SDLKey k = e.key.keysym.sym;
                if (k == SDLK_ESCAPE || k == SDLK_q || k == SDLK_s)
                    game_key(4);
                if (k == SDLK_LEFT || k == SDLK_l)
                    game_key(-1);
                if (k == SDLK_UP || k == SDLK_u)
                    game_key(-2);
                if (k == SDLK_DOWN || k == SDLK_d)
                    game_key(5);
                if (k == SDLK_RIGHT || k == SDLK_r)
                    game_key(1);
                if (k == SDLK_LCTRL || k == SDLK_RETURN || k == SDLK_a)
                    game_key(2);
                if (k == SDLK_LALT || k == SDLK_b)
                    game_key(3);
            }
        }
        if (game_should_quit())
            run = 0;
        Uint32 now = SDL_GetTicks();
        game_tick((now - last) / 1000.0);
        play_meow(game_take_sound());
        last = now;
        game_draw();
        if (SDL_MUSTLOCK(s) && SDL_LockSurface(s) < 0)
            break;
        for (int y = 0; y < 240; y++)
            for (int x = 0; x < 240; x++) {
                uint32_t c = pixels[y * 240 + x];
                Uint32 v = SDL_MapRGB(s->format, c >> 16, c >> 8, c);
                unsigned char *dst =
                    (unsigned char *)s->pixels + y * s->pitch + x * s->format->BytesPerPixel;
                switch (s->format->BytesPerPixel) {
                case 1:
                    *dst = (unsigned char)v;
                    break;
                case 2: {
                    Uint16 v16 = (Uint16)v;
                    memcpy(dst, &v16, 2);
                    break;
                }
                case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    dst[0] = (v >> 16) & 255;
                    dst[1] = (v >> 8) & 255;
                    dst[2] = v & 255;
#else
                    dst[0] = v & 255;
                    dst[1] = (v >> 8) & 255;
                    dst[2] = (v >> 16) & 255;
#endif
                    break;
                case 4:
                    memcpy(dst, &v, 4);
                    break;
                }
            }
        if (SDL_MUSTLOCK(s))
            SDL_UnlockSurface(s);
        SDL_Flip(s);
        SDL_Delay(33);
    }
    int ok = game_save();
    audio_close();
    SDL_Quit();
    return ok ? 0 : 1;
}
#endif
