#ifndef H_ASCII_LITERALS_H
#define H_ASCII_LITERALS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// ascii-literals.h                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// raison d'etre: a character literal in C is not guaranteed to be ASCII    //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#define ASCII_NUL	UINT8_C(0x00)	/* '\0' nul character             */

#define ASCII_SOH	UINT8_C(0x01)	/*      start of heading          */
#define ASCII_STX	UINT8_C(0x02)	/*      start of text             */
#define ASCII_ETX	UINT8_C(0x03)	/*      end of text               */
#define ASCII_EOT	UINT8_C(0x04)	/*      end of transmission       */
#define ASCII_ENQ	UINT8_C(0x05)	/*      enquiry                   */
#define ASCII_ACK	UINT8_C(0x06)	/*      acknowledgement           */
#define ASCII_BEL	UINT8_C(0x07)	/* '\a' bell (alert)              */
#define ASCII_BS	UINT8_C(0x08)	/* '\b' backspace                 */
#define ASCII_HT	UINT8_C(0x09)	/* '\t' horizontal tab            */
#define ASCII_LF	UINT8_C(0x0A)	/* '\n' line feed (new line)      */
#define ASCII_VT	UINT8_C(0x0B)	/* '\v' vertical tab              */
#define ASCII_FF	UINT8_C(0x0C)	/* '\f' form feed                 */
#define ASCII_CR	UINT8_C(0x0D)	/* '\r' carriage return           */
#define ASCII_SO	UINT8_C(0x0E)	/*      shift out                 */
#define ASCII_SI	UINT8_C(0x0F)	/*      shift in                  */
#define ASCII_DLE	UINT8_C(0x10)	/*      data link escape          */
#define ASCII_DC1	UINT8_C(0x11)	/*      device control 1          */
#define ASCII_DC2	UINT8_C(0x12)	/*      device control 2          */
#define ASCII_DC3	UINT8_C(0x13)	/*      device control 3          */
#define ASCII_DC4	UINT8_C(0x14)	/*      device control 4          */
#define ASCII_NAK	UINT8_C(0x15)	/*      negative acknowledgement  */
#define ASCII_SYN	UINT8_C(0x16)	/*      synchronous idle          */
#define ASCII_ETB	UINT8_C(0x17)	/*      end of transmission block */
#define ASCII_CAN	UINT8_C(0x18)	/*      cancel                    */
#define ASCII_EM	UINT8_C(0x19)	/*      end of medium             */
#define ASCII_SUB	UINT8_C(0x1A)	/*      substitute                */
#define ASCII_ESC	UINT8_C(0x1B)	/* '\e' escape                    */
#define ASCII_FS	UINT8_C(0x1C)	/*      file seperator            */
#define ASCII_GS	UINT8_C(0x1D)	/*      group seperator           */
#define ASCII_RS	UINT8_C(0x1E)	/*      record seperator          */
#define ASCII_US	UINT8_C(0x1F)	/*      unit seperator            */

#define ASCII_SP	UINT8_C(0x20)	/* ' '  space                     */

#define ASCII_EXCLAM	UINT8_C(0x21)	/* '!'  */
#define ASCII_DQUOTE	UINT8_C(0x22)	/* '"'  */
#define ASCII_POUND	UINT8_C(0x23)	/* '#'  */
#define ASCII_DOLLAR	UINT8_C(0x24)	/* '$'  */
#define ASCII_PERCENT	UINT8_C(0x25)	/* '%'  */
#define ASCII_AMPERSAND	UINT8_C(0x26)	/* '&'  */
#define ASCII_SQUOTE	UINT8_C(0x27)	/* '''  */
#define ASCII_LPAREN	UINT8_C(0x28)	/* '('  */
#define ASCII_RPAREN	UINT8_C(0x29)	/* ')'  */
#define ASCII_ASTERISK	UINT8_C(0x2A)	/* '*'  */
#define ASCII_PLUS	UINT8_C(0x2B)	/* '+'  */
#define ASCII_COMMA	UINT8_C(0x2C)	/* ','  */
#define ASCII_HYPHEN	UINT8_C(0x2D)	/* '-'  */
#define ASCII_PERIOD	UINT8_C(0x2E)	/* '.'  */
#define ASCII_SLASH	UINT8_C(0x2F)	/* '/'  */

#define ASCII_0		UINT8_C(0x30)	/* '0'  */
#define ASCII_1		UINT8_C(0x31)	/* '1'  */
#define ASCII_2		UINT8_C(0x32)	/* '2'  */
#define ASCII_3		UINT8_C(0x33)	/* '3'  */
#define ASCII_4		UINT8_C(0x34)	/* '4'  */
#define ASCII_5		UINT8_C(0x35)	/* '5'  */
#define ASCII_6		UINT8_C(0x36)	/* '6'  */
#define ASCII_7		UINT8_C(0x37)	/* '7'  */
#define ASCII_8		UINT8_C(0x38)	/* '8'  */
#define ASCII_9		UINT8_C(0x39)	/* '9'  */

#define ASCII_COLON	UINT8_C(0x3A)	/* ':'  */
#define ASCII_SEMICOLON	UINT8_C(0x3B)	/* ';'  */
#define ASCII_LESSER	UINT8_C(0x3C)	/* '<'  */
#define ASCII_EQUALS	UINT8_C(0x3D)	/* '='  */
#define ASCII_GREATER	UINT8_C(0x3E)	/* '>'  */
#define ASCII_QMARK	UINT8_C(0x3F)	/* '?'  */
#define ASCII_ATSIGN	UINT8_C(0x40)	/* '@'  */

#define ASCII_A_UP	UINT8_C(0x41)	/* 'A'  */
#define ASCII_B_UP	UINT8_C(0x42)	/* 'B'  */
#define ASCII_C_UP	UINT8_C(0x43)	/* 'C'  */
#define ASCII_D_UP	UINT8_C(0x44)	/* 'D'  */
#define ASCII_E_UP	UINT8_C(0x45)	/* 'E'  */
#define ASCII_F_UP	UINT8_C(0x46)	/* 'F'  */
#define ASCII_G_UP	UINT8_C(0x47)	/* 'G'  */
#define ASCII_H_UP	UINT8_C(0x48)	/* 'H'  */
#define ASCII_I_UP	UINT8_C(0x49)	/* 'I'  */
#define ASCII_J_UP	UINT8_C(0x4A)	/* 'J'  */
#define ASCII_K_UP	UINT8_C(0x4B)	/* 'K'  */
#define ASCII_L_UP	UINT8_C(0x4C)	/* 'L'  */
#define ASCII_M_UP	UINT8_C(0x4D)	/* 'M'  */
#define ASCII_N_UP	UINT8_C(0x4E)	/* 'N'  */
#define ASCII_O_UP	UINT8_C(0x4F)	/* 'O'  */
#define ASCII_P_UP	UINT8_C(0x50)	/* 'P'  */
#define ASCII_Q_UP	UINT8_C(0x51)	/* 'Q'  */
#define ASCII_R_UP	UINT8_C(0x52)	/* 'R'  */
#define ASCII_S_UP	UINT8_C(0x53)	/* 'S'  */
#define ASCII_T_UP	UINT8_C(0x54)	/* 'T'  */
#define ASCII_U_UP	UINT8_C(0x55)	/* 'U'  */
#define ASCII_V_UP	UINT8_C(0x56)	/* 'V'  */
#define ASCII_W_UP	UINT8_C(0x57)	/* 'W'  */
#define ASCII_X_UP	UINT8_C(0x58)	/* 'X'  */
#define ASCII_Y_UP	UINT8_C(0x59)	/* 'Y'  */
#define ASCII_Z_UP	UINT8_C(0x5A)	/* 'Z'  */

#define ASCII_LBRACKET	UINT8_C(0x5B)	/* '['  */
#define ASCII_BSLASH	UINT8_C(0x5C)	/* '\\' */
#define ASCII_RBRACKET	UINT8_C(0x5D)	/* ']'  */
#define ASCII_CARET	UINT8_C(0x5E)	/* '^'  */
#define ASCII_USCORE	UINT8_C(0x5F)	/* '_'  */
#define ASCII_BTICK	UINT8_C(0x60)	/* '`'  */

#define ASCII_A_LO	UINT8_C(0x61)	/* 'a'  */
#define ASCII_B_LO	UINT8_C(0x62)	/* 'b'  */
#define ASCII_C_LO	UINT8_C(0x63)	/* 'c'  */
#define ASCII_D_LO	UINT8_C(0x64)	/* 'd'  */
#define ASCII_E_LO	UINT8_C(0x65)	/* 'e'  */
#define ASCII_F_LO	UINT8_C(0x66)	/* 'f'  */
#define ASCII_G_LO	UINT8_C(0x67)	/* 'g'  */
#define ASCII_H_LO	UINT8_C(0x68)	/* 'h'  */
#define ASCII_I_LO	UINT8_C(0x69)	/* 'i'  */
#define ASCII_J_LO	UINT8_C(0x6A)	/* 'j'  */
#define ASCII_K_LO	UINT8_C(0x6B)	/* 'k'  */
#define ASCII_L_LO	UINT8_C(0x6C)	/* 'l'  */
#define ASCII_M_LO	UINT8_C(0x6D)	/* 'm'  */
#define ASCII_N_LO	UINT8_C(0x6E)	/* 'n'  */
#define ASCII_O_LO	UINT8_C(0x6F)	/* 'o'  */
#define ASCII_P_LO	UINT8_C(0x70)	/* 'p'  */
#define ASCII_Q_LO	UINT8_C(0x71)	/* 'q'  */
#define ASCII_R_LO	UINT8_C(0x72)	/* 'r'  */
#define ASCII_S_LO	UINT8_C(0x73)	/* 's'  */
#define ASCII_T_LO	UINT8_C(0x74)	/* 't'  */
#define ASCII_U_LO	UINT8_C(0x75)	/* 'u'  */
#define ASCII_V_LO	UINT8_C(0x76)	/* 'v'  */
#define ASCII_W_LO	UINT8_C(0x77)	/* 'w'  */
#define ASCII_X_LO	UINT8_C(0x78)	/* 'x'  */
#define ASCII_Y_LO	UINT8_C(0x79)	/* 'y'  */
#define ASCII_Z_LO	UINT8_C(0x7A)	/* 'z'  */

#define ASCII_LBRACE	UINT8_C(0x7B)	/* '{'  */
#define ASCII_VBAR	UINT8_C(0x7C)	/* '|'  */
#define ASCII_RBRACE	UINT8_C(0x7D)	/* '}'  */
#define ASCII_TILDE	UINT8_C(0x7E)	/* '~'  */

#define ASCII_DEL	UINT8_C(0x7F)	/*      delete                    */

/* ======================================================================== */

#define ASCII_CASE_DIFF	UINT8_C(0x20)

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_ASCII_LITERALS_H */
