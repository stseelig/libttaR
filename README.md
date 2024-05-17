# libttaR
Tau's True Audio (TTA) codec rewrite

## What is TTA?
TTA is a lossless audio codec designed around realtime hardware coding.

http://tausoft.org/wiki/True_Audio_Codec_Overview

## Building
edit then run make.sh

I recommend using clang for compiling.

## About
The library is just a collection of functions for reading from one buffer and writing to another with some support functions to calculate values. It does not allocate, print, or make any syscalls. It does not even need libc as long as your compiler has a builtin memset and memmove (ie, not gcc). So fairly bare-bones as far as codec libraries go.

The two codec functions are re-entrant, so you can code as few samples/bytes at a time with buffers as small as you want. ttaR (cli util) 1.0 did this, but I replaced it with doing the whole frame at once in 1.1 because it's a little faster and it simplified the source by sharing common functions among the single and multi-threaded versions.
