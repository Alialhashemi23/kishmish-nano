# Third-party runtime notices

The original Kishmish code and generated assets are MIT licensed. The ARM OPK additionally carries unmodified shared libraries from the official FunKey SDK 2.3.0:

| Component | Version | License | Files |
| --- | --- | --- | --- |
| SDL | 1.2.15, FunKey/Buildroot patches | LGPL 2.1 or later | `lib/libSDL-1.2.so.0` |
| musl libc / dynamic loader | 1.2.1 | MIT and notices in COPYRIGHT | `lib/libc.so`, `lib/ld-musl-armhf.so.1` |

Full license texts are in `licenses/` and are also included inside release OPKs. musl's loader and libc are the same underlying SDK file copied under two names. These components keep their upstream copyrights. The game links dynamically to SDL; you may rebuild or replace the private shared library and repackage the OPK with the included build script.

## Matching source and build recipes

The release asset **`kishmish-runtime-sources.zip`** contains the upstream SDL 1.2.15 and musl 1.2.1 source archives, plus the FunKey OS and Buildroot source trees used by the SDK release (including SDL patches and build configuration).

- [SDL 1.2.15 source](https://www.libsdl.org/release/SDL-1.2.15.tar.gz)
- [musl 1.2.1 source](https://musl.libc.org/releases/musl-1.2.1.tar.gz)
- [FunKey OS 2.3.0 build tree](https://github.com/FunKey-Project/FunKey-OS/tree/FunKey-OS-2.3.0)
- [Matching Buildroot revision](https://github.com/FunKey-Project/buildroot/tree/1f59bd3b4809a4196f34bd4daf4f092d4c4911c8)
- [Official SDK download](https://github.com/FunKey-Project/FunKey-OS/releases/tag/FunKey-OS-2.3.0)

Buildroot's `package/sdl` and `package/musl` contain component build recipes and package patches. FunKey OS's `FunKey/board/funkey/patches/sdl` adds handheld-specific patches; its configuration and README describe rebuilding the SDK. The SDK files are not committed to this repository. Keep source and license materials available alongside redistributions of binary releases.
