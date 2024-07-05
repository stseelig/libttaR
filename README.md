# libttaR
Tau's True Audio (TTA) codec rewrite

## What is TTA?
TTA is a lossless audio codec designed around realtime hardware coding.

http://tausoft.org/wiki/True_Audio_Codec_Overview

## Building
edit then run ./make.sh

I recommend compiling with clang.

### Defines
NDEBUG\
	turns off debug assertions (!!!the library has assertions too!!!)

LIBTTAr_OPT_DISABLE_UNROLLED_1CH\
	disables the unrolled mono loop

LIBTTAr_OPT_DISABLE_UNROLLED_2CH\
	disables the unrolled stereo loop

LIBTTAr_OPT_DISABLE_MCH\
	disables the general/multichannel loop

LIBTTAr_OPT_PREFER_LOOKUP_TABLES\
	makes some operations use lookup tables (for weak CPUs like a Celeron)

## Basic Usage
$ ttaR encode file.(wav|w64)
$ ttaR decode file.tta

Read the man page (./man/ttaR.1) for a full list of the options.

## About
This project started as a fork of ttaenc 3.4.1-linux (2007).
Any code from ttaenc was put into the library, and the CLI program was
completely rewritten.

The library is just a collection of functions for reading from one buffer and
writing to another with some support functions to calculate values.
It does not allocate, print, nor make any other syscall.
(Though it can abort if NDEBUG is not defined!)
It does not even need libc as long as your compiler has a builtin memset and
memmove (ie, not gcc).
So fairly bare-bones as far as codec libraries go.

The two codec functions are reentrant, so you can code as few samples at a
time with buffers as small as you want.
ttaR (CLI util) 1.0 did this, but I replaced it with doing the whole frame at
once in 1.1 because it's a little faster and it simplified the source a bit.
