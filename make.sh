#!/bin/sh -

readonly ROOT="$(realpath "$(dirname "$0")")";
readonly SRC="$ROOT/src";
readonly BUILD="$ROOT/build";

readonly LIB="$BUILD/libttaR.so";
readonly CLI="$BUILD/ttaR";

readonly CC='clang';

CFLAGS_SHARED=;
CFLAGS_SHARED="$CFLAGS_SHARED -std=c99";
CFLAGS_SHARED="$CFLAGS_SHARED -Wall -Wextra -Wpedantic";
CFLAGS_SHARED="$CFLAGS_SHARED -DNDEBUG";
#CFLAGS_SHARED="$CFLAGS_SHARED -gdwarf";
CFLAGS_SHARED="$CFLAGS_SHARED -march=native -mtune=native";
readonly CFLAGS_SHARED;

CFLAGS_LIB="$CFLAGS_SHARED";
CFLAGS_LIB="$CFLAGS_LIB -O3";
CFLAGS_LIB="$CFLAGS_LIB -shared -fPIC";
CFLAGS_LIB="$CFLAGS_LIB -nolibc -ffreestanding";
#CFLAGS_LIB="$CFLAGS_LIB -DLIBTTAr_OPT_SLOW_CPU";
readonly CFLAGS_LIB;

CFLAGS_CLI="$CFLAGS_SHARED";
CFLAGS_CLI="$CFLAGS_CLI -O3 -ffast-math";
readonly CFLAGS_CLI;

LD_CLI="-L$BUILD -lpthread -lttaR";
readonly LD_CLI;

##############################################################################

if [ ! -e "$BUILD" ]; then
	mkdir -- "$BUILD" || exit $?;
fi
cp -- "$SRC/libttaR.h" "$BUILD/" &
"$CC" $CFLAGS_LIB "$SRC/build_lib.c" -o "$LIB";
"$CC" $CFLAGS_CLI "$SRC/build_cli.c" -o "$CLI" $LD_CLI;
wait;
