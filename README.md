# libttaR
Tau's True Audio (TTA) codec rewrite

## What is TTA?
TTA is a lossless audio codec designed around realtime hardware coding.

http://tausoft.org/wiki/True_Audio_Codec_Overview

## Building
edit then run make.sh

I recommend compiling with clang.

## About
This project started as a fork of ttaenc 3.4.1 linux version (2007). Any code 
from ttaenc was put into the library, and the cli program was completely 
rewritten. 

The library is just a collection of functions for reading from one buffer and 
writing to another with some support functions to calculate values. It does 
not allocate, print, nor make any other syscall. It does not even need libc 
as long as your compiler has a builtin memset and memmove (ie, not gcc). So 
fairly bare-bones as far as codec libraries go.

The two codec functions are re-entrant, so you can code as few samples at a 
time with buffers as small as you want. ttaR (cli util) 1.0 did this, but I 
replaced it with doing the whole frame at once in 1.1 because it's a little 
faster and it simplified the source a bit.
