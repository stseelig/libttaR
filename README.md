# libttaR

Tau's True Audio (TTA) codec rewrite


## What is TTA?

TTA is a lossless audio codec designed around realtime hardware coding.
The codec has no tunable parameters, and each input produces a unique output.
It supports 8, 16, and 24-bit PCM.

http://tausoft.org/wiki/True_Audio_Codec_Overview


## Blog

How ttaR is faster than ttaenc (2024-12-28)
https://gist.github.com/stseelig/0b9afe795a547b5798587db996694f97


## Platforms

The library is platform agnostic.
The program is POSIX native
and has a mostly native (mostly working) Win32 port via MinGW.


### Processors

2's complement integers are assumed.

While developing, I mostly tested on:

* AMD Ryzen 7 1700    (primary tuning)

* Intel Celeron N2830 (secondary tuning)

honorable mentions:

* IBM PowerPC 7400 (intrinsics)

* ARM Cortex-A53 (intrinsics)


## Building

Unix
```
$ clang -O3 -DNDEBUG -nolibc -ffreestanding -fPIC -shared ./src/build_lib.c -o libttaR.so
$ clang -O3 -DNDEBUG ./src/build_cli.c -o ttaR -L./ -lpthread -lttaR
```

Windows
```
$ mingw32-clang -O3 -DNDEBUG -ffreestanding -fPIC -shared ./src/build_lib.c -o libttaR.dll
$ mingw32-clang -O3 -DNDEBUG ./src/build_cli.c -o ttaR.exe -L./ -lttaR
```

gcc(1) would also work, but it produces a much slower binary than clang(1).


### Defines

NDEBUG

* disables all debug assertions

LIBTTAr_OPT_DISABLE_UNROLLED_1CH

* disables the unrolled mono loop

LIBTTAr_OPT_DISABLE_UNROLLED_2CH

* disables the unrolled stereo loop

LIBTTAr_OPT_DISABLE_MCH

 * disables the general/multichannel loop

LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS

* disables SIMD intrinsics

LIBTTAr_OPT_SLOW_CPU

* for weak and/or old CPUs (specifically the Intel Celeron N2830)

* fine-grained suboptions in 'src/lib/common.h'


## Basic Usage

```
$ ttaR encode file.(wav|w64)
$ ttaR decode file.tta
```

By default, ttaR will multithread with the number of coder threads equal to
the number of online processors.

Read the man page (./man/ttaR.1) for a full list of the options.


## About

This project started as a fork of ttaenc 3.4.1-linux (2007).
Any code from ttaenc was put into the library, and the CLI program was
completely rewritten.

The library prioritizes speed.
So, its memory usage is a bit bloated to avoid some bounds checks.
(The safer alternative is maybe 10-20% slower.)

The library is just a collection of functions for reading from one buffer and
writing to another with some support functions to calculate values.
It does not allocate, print, nor make any other syscall.
(Though it can abort, but should not, if NDEBUG is not defined.)
It does not even need libc as long as your compiler
has a builtin memset and memmove (both, ie, not gcc).
So fairly bare-bones as far as codec libraries go.

The two codec functions are reentrant, so you can code as few samples at a
time with buffers as small as you want (with some padding).
ttaR (CLI util) 1.0 did this, but I replaced it with doing the whole frame at
once in 1.1 because it's a little faster and it simplified the source a bit.
