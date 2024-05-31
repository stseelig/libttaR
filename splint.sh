#! /bin/sh -
##############################################################################
#                                                                            #
# splint.sh                                                                  #
#                                                                            #
##############################################################################
#                                                                            #
# Copyright (C) 2023-2024, Shane Seelig                                      #
# SPDX-License-Identifier: GPL-3.0-or-later                                  #
#                                                                            #
##############################################################################

#readonly EXPECTED=;	# Splint 3.1.2 --- 21 Feb 2021

#----------------------------------------------------------------------------#

OPTS='-nof';
#OPTS="$OPTS -expect $EXPECTED";
OPTS="$OPTS +forcehints";
readonly OPTS;

FLAGS='-posixlib';
readonly FLAGS;

MODES='-strict';

#MODES="$MODES -controlnestdepth 3";

MODES="$MODES -boundsread";	# bugged
MODES="$MODES -boundswrite";	# bugged
MODES="$MODES -cppnames";
MODES="$MODES -declundef";
MODES="$MODES -enummemuse";
MODES="$MODES -exportconst";
MODES="$MODES -exportfcn";
MODES="$MODES -exportheader";
MODES="$MODES -exporttype";
MODES="$MODES -exportvar";
MODES="$MODES -fielduse";	# bugged
MODES="$MODES -formatcode"	# %z
MODES="$MODES -globs";		# selective checking
MODES="$MODES -incondefs";	# bugged
MODES="$MODES -mods";
MODES="$MODES -protoparamname";
MODES="$MODES -sizeoftype";
MODES="$MODES -strictops";
MODES="$MODES -sysdirerrors";	# too much garbage in the system headers
MODES="$MODES -typeuse";	# bugged

MODES="$MODES +charint";
MODES="$MODES +enumint";

readonly MODES;

##############################################################################

printf -- "splint %s %s %s %s\n" "$OPTS" "$FLAGS" "$FILES" "$@";

splint $OPTS $FLAGS $MODES "$@";

## EOF #######################################################################
