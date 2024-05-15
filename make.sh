#! /bin/sh -
##############################################################################
#                                                                            #
# make.sh                                                                    #
#                                                                            #
##############################################################################
#                                                                            #
# Copyright (C) 2023-2024, Shane Seelig                                      #
# SPDX-License-Identifier: GPL-3.0-or-later                                  #
#                                                                            #
##############################################################################

NPROC=$(nproc) || NPROC=1;	# MAYBE add a '-j' opt
readonly NPROC;

#----------------------------------------------------------------------------#

readonly LIBRARY='libttaR.so';
readonly LIB_BASE='ttaR';
readonly PROGRAM='ttaR';

#   clang produces a SIGNIFICANTLY faster binary than gcc. In the tta
# reference encoder (ttaenc), I fixed all (gcc) compiler warnings and made
# everything static (it is only one file) and gcc made it 10% slower! (clang
# can't compile the reference encoder.) I know my compilers are outdated
# because of Debian, but still.
#
#   main reasons that gcc is slower:
#       - gcc doesn't have a builtin memmove (this is extra funny considering
# the glibc memcpy backwards fiasco from last decade)
#       - clang is better at auto-SIMDing
#       - it can do dumb stuff if you don't use gcc-isms like the example in
# the previous paragraph
#
#	AMD Ryzen 7 1700
#	gcc (Debian 10.2.1-6) 10.2.1 20210110
#	Debian clang version 11.0.1-2

readonly CC='clang';
readonly LD="$CC";

#=============================================================================

readonly ROOT="$(realpath "$(dirname "$0")")";
# relative to ROOT
readonly SRC='./src';
readonly BUILD='./build';
readonly OBJ="$BUILD/obj";

#============================================================================#

CFLAGS_COMMON=;

CFLAGS_COMMON="$CFLAGS_COMMON -std=c99";

CFLAGS_COMMON="$CFLAGS_COMMON -march=native";
CFLAGS_COMMON="$CFLAGS_COMMON -mtune=native";

#CFLAGS_COMMON="$CFLAGS_COMMON -gdwarf";
CFLAGS_COMMON="$CFLAGS_COMMON -DNDEBUG";

#CFLAGS_COMMON="$CFLAGS_COMMON -fno-inline";

CFLAGS_COMMON="$CFLAGS_COMMON -Wall";
CFLAGS_COMMON="$CFLAGS_COMMON -Wextra";
CFLAGS_COMMON="$CFLAGS_COMMON -Wpedantic";

if [ "$CC" = 'gcc' ]; then
# gcc complaining about enum switches
CFLAGS_COMMON="$CFLAGS_COMMON -Wno-maybe-uninitialized";
fi

readonly CFLAGS_COMMON;

#----------------------------------------------------------------------------#

CFLAGS_CLI=;

#CFLAGS_CLI="$CFLAGS_CLI -D_POSIX_C_SOURCE=200809";
CFLAGS_CLI="$CFLAGS_CLI -D_GNU_SOURCE";	# reallocarray, memrchr, strchr
CFLAGS_CLI="$CFLAGS_CLI -D_FILE_OFFSET_BITS=64";

CFLAGS_CLI="$CFLAGS_CLI -O3";
CFLAGS_CLI="$CFLAGS_CLI -ffast-math";

readonly CFLAGS_CLI;

#----------------------------------------------------------------------------#

CFLAGS_LIB=;

CFLAGS_LIB="$CFLAGS_LIB -O3";

CFLAGS_LIB="$CFLAGS_LIB -fPIC";

# disable the unrolled mono/stereo codec loop. bit faster, but larger binary
#CFLAGS_LIB="$CFLAGS_LIB -DLIBTTAr_DISABLE_UNROLLED_1CH";
#CFLAGS_LIB="$CFLAGS_LIB -DLIBTTAr_DISABLE_UNROLLED_2CH";

# disable the multichannel/general decoder
#CFLAGS_LIB="$CFLAGS_LIB -DLIBTTAr_DISABLE_MCH";

# disable tzcnt builtin if no native tzcnt instruction
#CFLAGS_LIB="$CFLAGS_LIB -DLIBTTAr_NO_INSTRUCTION_TZCNT";

readonly CFLAGS_LIB;

#============================================================================#

LDFLAGS_COMMON=;
readonly LDFLAGS_COMMON;

#----------------------------------------------------------------------------#

LDFLAGS_CLI=;
readonly LDFLAGS_CLI;

# gcc needs these at the end of the command for some reason
LDFLAGS_CLI_END=;
LDFLAGS_CLI_END="$LDFLAGS_CLI_END -L$BUILD/";
LDFLAGS_CLI_END="$LDFLAGS_CLI_END -l$LIB_BASE";
LDFLAGS_CLI_END="$LDFLAGS_CLI_END -lpthread";
readonly LD_FLAGS_CLI_END;

#----------------------------------------------------------------------------#

LDFLAGS_LIB=;
LDFLAGS_LIB="$LDFLAGS_LIB -shared";
# library only uses memmove & memset from libc, but those should be builtin
LDFLAGS_LIB="$LDFLAGS_LIB -nolibc";
readonly LDFLAGS_LIB;

LDFLAGS_LIB_END=;
readonly LDFLAGS_LIB_END;

#============================================================================#

#readonly STRIP=1;
STRIP_O='--strip-all';
STRIP_O="$STRIP_O --remove-section=.comment*";
STRIP_O="$STRIP_O --remove-section=.note*";
readonly STRIP_O;

##############################################################################

readonly DIR0="$BUILD";
readonly DIR1="$OBJ";
readonly DIR2="$OBJ/cli";
readonly DIR3="$OBJ/cli/formats";
readonly DIR4="$OBJ/cli/modes";
readonly DIR5="$OBJ/cli/opts";
readonly DIR6="$OBJ/lib";

readonly HEADER="$SRC/libttaR.h";

readonly L_C00='lib/crc32';
readonly L_C01='lib/misc';
readonly L_C02='lib/pcm_read';
readonly L_C03='lib/pcm_write';
readonly L_C04='lib/rice';
readonly L_C05='lib/tta_dec';
readonly L_C06='lib/tta_enc';

readonly P_C00='cli/cli';
readonly P_C01='cli/debug';
readonly P_C02='cli/formats/guid';
readonly P_C03='cli/formats/metatags_skip';
readonly P_C04='cli/formats/tta1_check';
readonly P_C05='cli/formats/tta_seek';
readonly P_C06='cli/formats/tta_seek_check';
readonly P_C07='cli/formats/tta_write';
readonly P_C08='cli/formats/w64_check';
readonly P_C09='cli/formats/w64_write';
readonly P_C10='cli/formats/wav_check';
readonly P_C11='cli/formats/wav_write';
readonly P_C12='cli/help';
readonly P_C13='cli/main';
readonly P_C14='cli/open';
readonly P_C15='cli/opts/common';
readonly P_C16='cli/opts/tta2dec';
readonly P_C17='cli/opts/tta2enc';
readonly P_C18='cli/optsget';
readonly P_C19='cli/modes/bufs';
readonly P_C20='cli/modes/mode_decode';
readonly P_C21='cli/modes/mode_decode_st';
readonly P_C22='cli/modes/mode_encode';
readonly P_C23='cli/modes/mode_encode_loop';
readonly P_C24='cli/modes/mt-struct';

#----------------------------------------------------------------------------#

readonly L_O00="$OBJ/$L_C00.o";
readonly L_O01="$OBJ/$L_C01.o";
readonly L_O02="$OBJ/$L_C02.o";
readonly L_O03="$OBJ/$L_C03.o";
readonly L_O04="$OBJ/$L_C04.o";
readonly L_O05="$OBJ/$L_C05.o";
readonly L_O06="$OBJ/$L_C06.o";

readonly P_O00="$OBJ/$P_C00.o";
readonly P_O01="$OBJ/$P_C01.o";
readonly P_O02="$OBJ/$P_C02.o";
readonly P_O03="$OBJ/$P_C03.o";
readonly P_O04="$OBJ/$P_C04.o";
readonly P_O05="$OBJ/$P_C05.o";
readonly P_O06="$OBJ/$P_C06.o";
readonly P_O07="$OBJ/$P_C07.o";
readonly P_O08="$OBJ/$P_C08.o";
readonly P_O09="$OBJ/$P_C09.o";
readonly P_O10="$OBJ/$P_C10.o";
readonly P_O11="$OBJ/$P_C11.o";
readonly P_O12="$OBJ/$P_C12.o";
readonly P_O13="$OBJ/$P_C13.o";
readonly P_O14="$OBJ/$P_C14.o";
readonly P_O15="$OBJ/$P_C15.o";
readonly P_O16="$OBJ/$P_C16.o";
readonly P_O17="$OBJ/$P_C17.o";
readonly P_O18="$OBJ/$P_C18.o";
readonly P_O19="$OBJ/$P_C19.o";
readonly P_O20="$OBJ/$P_C20.o";
readonly P_O21="$OBJ/$P_C21.o";
readonly P_O22="$OBJ/$P_C22.o";
readonly P_O23="$OBJ/$P_C23.o";
readonly P_O24="$OBJ/$P_C24.o";

##############################################################################

readonly T_RESET='\e[0m';
readonly T_B_BLACK='\e[1;30m';
readonly T_RED='\e[31m';
readonly T_B_RED='\e[1;31m';
readonly T_GREEN='\e[32m';
readonly T_B_GREEN='\e[1;32m';
readonly T_YELLOW='\e[33m';
readonly T_BLUE='\e[34m';
readonly T_B_BLUE='\e[1;34m';
readonly T_PURPLE='\e[35m';
readonly T_B_PURPLE='\e[1;35m';
readonly T_B_CYAN='\e[1;36m';

##############################################################################

CMD_START=;
PRINT=;

#----------------------------------------------------------------------------#

_timestamp(){
	date +%s.%N;
}

_timefmt(){
# $1: time
	printf -- "%um%.3fs" $(dc -e "$1 60 / p") $(dc -e "$1 60 % p");
}

_size(){
# $1: file
	du -h --apparent-size "$1" | cut -f 1;
}

_ret_ok(){
# $1: time
# $2: file
	PRINT="${PRINT}(${T_GREEN}0: ok";
	if [ $# -ge 1 ]; then
		PRINT="${PRINT}${T_RESET}, ${T_BLUE}";
		PRINT="${PRINT}$(_timefmt $1)";
	fi
	if [ $# -ge 2 ]; then
		PRINT="${PRINT}${T_RESET}, ${T_B_BLACK}$(_size "$2")";
	fi
	printf -- "${PRINT}${T_RESET})\n";
	PRINT=;
}

_ret_fail(){
# $1: status
	printf -- "${PRINT}(${T_RED}${1}: failed${T_RESET})\n";
	kill -TERM $$;	# trapped
}

_exit(){
# $1: status
# $2: time
	PRINT="[${T_B_PURPLE}EXIT${T_RESET}]\n";
	if [ $1 -eq 0 ]; then
		PRINT="${PRINT}(${T_B_GREEN}0: ok${T_RESET}";
		if [ $# -gt 1 ]; then
			PRINT="${PRINT}, ${T_B_BLUE}$(_timefmt $2)${T_RESET}";
		fi
		printf -- "${PRINT})\n";
	else
		printf -- "${PRINT}(${T_B_RED}${1}: failed${T_RESET})\n";
		exec 1<&- && exec 2<&-;	# close stdout and stderr
		sleep 0.05;		# let anything that got through print
	fi
	exit $1
}

_flags_print(){
# $1: name
# $2-$#: flags
	PRINT="[${T_YELLOW}FLAGS${T_RESET}]\t${1}='";
	shift;
	while [ $# -gt 0 ]; do
		PRINT="${PRINT}${1}";
		shift;
	done
	printf -- "${PRINT}'\n";
	PRINT=;
}

_cd(){
# $1: directory
	if [ "$1" != "$(pwd)" ]; then
		PRINT="[${T_PURPLE}CD${T_RESET}]\tcd ${1}\n";
		cd -- "$1" || _ret_fail $?;
		_ret_ok;
	fi
}

_mkdir(){
# $@: directories
	while [ $# -gt 0 ]; do
		if [ ! -e "$1" ]; then
			PRINT="[${T_PURPLE}MKDIR${T_RESET}]\tmkdir ${1}\n";
			mkdir -- "$1" || _ret_fail $?;
			_ret_ok;
		fi
		shift;
	done
}

_cc(){
# $1: CC opts
# $2: C-file
	PRINT="[${T_B_CYAN}CC${T_RESET}]\t${CC} -c";
	PRINT="${PRINT}${1} -o ${OBJ}/${2}.o ${SRC}/${2}.c\n";
	CMD_START=$(_timestamp);
	 "$CC" -c $1 -o "$OBJ/$2.o" "$SRC/$2.c" || _ret_fail $?;
	_ret_ok $(dc -e "$(_timestamp) $CMD_START - p") "$OBJ/$2.o";
}

_cc_mp(){
# $1: CC opts
# $2-$#: C-files
	CC_OPTS="$1"; shift;
	while [ $# -gt 0 ]; do
		_cc "$CC_OPTS" "$1"&
		while [ $(pgrep -P $$ -c) -ge $NPROC ]; do :; done
		shift;
	done
}

_ld(){
# $1: LD opts
# $2: LD end opts (gcc bs)
# $3: out-file
# $4-$#: in-files
	LD_OPTS="$1"; shift;
	LD_OPTS_END="$1"; shift;
	PRINT="[${T_B_CYAN}LD${T_RESET}]\t${LD}";
	if [ -n "LD_OPTS" ]; then
		PRINT="${PRINT}${LD_OPTS}";
	fi
	if [ -n "$LDFLAGS" ]; then
		PRINT="${PRINT}${LDFLAGS}";
	fi
	PRINT="${PRINT} -o ${*}";
	if [ -n "LD_OPTS_END" ]; then
		PRINT="${PRINT}${LD_OPTS_END}";
	fi
	PRINT="${PRINT}\n";
	CMD_START=$(_timestamp);
	"$LD" $LD_OPTS -o "$@" $LD_OPTS_END || _ret_fail $?;
	_ret_ok $(dc -e "$(_timestamp) $CMD_START - p") "$1";
}

_strip(){
# $@: files
	while [ $# -gt 0 ]; do
		PRINT="[${T_B_CYAN}STRIP${T_RESET}]\tstrip ${STRIP_O} ${1}\n";
		CMD_START=$(_timestamp);
		strip $STRIP_O -- "$1" || _ret_fail $?;
		_ret_ok $(dc -e "$(_timestamp) $CMD_START - p") "$1";
		shift;
	done
}

_cp2build(){
# $@: files to copy
	while [ $# -gt 0 ]; do
		PRINT="[${T_PURPLE}CP${T_RESET}]\tcp ${1} ${BUILD}\n";
		cp -- "$1" "${BUILD}" || _ret_fail $?;
		_ret_ok;
		shift;
	done
}

##############################################################################

trap 'pkill -P $$; _exit 1;' ABRT HUP INT QUIT TERM;	# filicide

##############################################################################

readonly MAIN_START=$(_timestamp);

#----------------------------------------------------------------------------#

if [ -n "$CFLAGS_COMMON" ]; then
	_flags_print 'CFLAGS_COMMON' "$CFLAGS_COMMON";
fi
if [ -n "$CFLAGS_CLI" ]; then
	_flags_print 'CFLAGS_CLI' "$CFLAGS_CLI";
fi
if [ -n "$CFLAGS_LIB" ]; then
	_flags_print 'CFLAGS_LIB' "$CFLAGS_LIB";
fi
if [ -n "$LDFLAGS_COMMON" ]; then
	_flags_print 'LDFLAGS_COMMON' "$LDFLAGS_COMMON";
fi
if [ -n "$LDFLAGS_CLI" ]; then
	_flags_print 'LDFLAGS_CLI' "$LDFLAGS_CLI";
fi
if [ -n "$LDFLAGS_CLI_END" ]; then
	_flags_print 'LDFLAGS_CLI_END' "$LDFLAGS_CLI_END";
fi
if [ -n "$LDFLAGS_LIB" ]; then
	_flags_print 'LDFLAGS_LIB' "$LDFLAGS_LIB";
fi
if [ -n "$LDFLAGS_LIB_END" ]; then
	_flags_print 'LDFLAGS_LIB_END' "$LDFLAGS_LIB_END";
fi

_cd "$ROOT";

_mkdir "$DIR0" "$DIR1" "$DIR2" "$DIR3" "$DIR4" "$DIR5" "$DIR6";

_cc_mp	"$CFLAGS_COMMON $CFLAGS_LIB" \
	"$L_C00" "$L_C01" "$L_C02" "$L_C03" "$L_C04" "$L_C05" "$L_C06";
_cc_mp	"$CFLAGS_COMMON $CFLAGS_CLI" \
	"$P_C00" "$P_C01" "$P_C02" "$P_C03" "$P_C04" "$P_C05" "$P_C06" \
	"$P_C07" "$P_C08" "$P_C09" "$P_C10" "$P_C11" "$P_C12" "$P_C13" \
	"$P_C14" "$P_C15" "$P_C16" "$P_C17" "$P_C18" "$P_C19" "$P_C20" \
	"$P_C21" "$P_C22" "$P_C23" "$P_C24";
wait;

_ld "$LDFLAGS_LIB" "$LDFLAGS_LIB_END" "$BUILD/$LIBRARY" \
	"$L_O00" "$L_O01" "$L_O02" "$L_O03" "$L_O04" "$L_O05" "$L_O06";
_ld "$LDFLAGS_CLI" "$LDFLAGS_CLI_END" "$BUILD/$PROGRAM" \
	"$P_O00" "$P_O01" "$P_O02" "$P_O03" "$P_O04" "$P_O05" "$P_O06" \
	"$P_O07" "$P_O08" "$P_O09" "$P_O10" "$P_O11" "$P_O12" "$P_O13" \
	"$P_O14" "$P_O15" "$P_O16" "$P_O17" "$P_O18" "$P_O19" "$P_O20" \
	"$P_O21" "$P_O22" "$P_O23" "$P_O24";

if [ -n "$STRIP" ] && [ $STRIP -ne 0 ]; then
	_strip "$BUILD/$PROGRAM";
	_strip "$BUILD/$LIBRARY";
fi

_cp2build "$HEADER";

_exit 0 $(dc -e "$(_timestamp) $MAIN_START - p");

## EOF #######################################################################
