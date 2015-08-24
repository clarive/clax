/********************************************************************/
/*																	*/
/* Name		: 	ctypet_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/*              US Government Users Restricted Rights - Use,        */
/*              duplication or disclosure restricted by GSA ADP     */
/*              Schedule Contract with IBM Corp.                    */
/* 																	*/
/* Function :	Contains tables used by ctype functions				*/
/* 																	*/
/* Compile	:	None												*/
/* Options															*/
/*								DLL									*/
/*																	*/
/* Notes	:	None												*/
/*																	*/ 
/********************************************************************/

/* Need to include _Ascii_a.h to force definition of _ISCNTRL_A, functions */

#include <ctype.h>
#include "_Ascii_a.h"

/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII-EBCDIC translate tables for ctype_a.h functions			*/
/*																	*/
/********************************************************************/

/* Character Classes 												*/
static short _ctype_a_tbl[] =
	{
	/* -1 */    0,
	/* 00 */    _ISCNTRL_A,
	/* 01 */    _ISCNTRL_A,
	/* 02 */    _ISCNTRL_A,
	/* 03 */    _ISCNTRL_A,
	/* 04 */    _ISCNTRL_A,
	/* 05 */    _ISCNTRL_A,
	/* 06 */    _ISCNTRL_A,
	/* 07 */    _ISCNTRL_A,
	/* 08 */    _ISCNTRL_A,
	/* 09 */    _ISCNTRL_A|_ISSPACE_A,
	/* 0A */    _ISCNTRL_A|_ISSPACE_A,
	/* 0B */    _ISCNTRL_A|_ISSPACE_A,
	/* 0C */    _ISCNTRL_A|_ISSPACE_A,
	/* 0D */    _ISCNTRL_A|_ISSPACE_A,
	/* 0E */    _ISCNTRL_A,
	/* 0F */    _ISCNTRL_A,
	/* 10 */    _ISCNTRL_A,
	/* 11 */    _ISCNTRL_A,
	/* 12 */    _ISCNTRL_A,
	/* 13 */    _ISCNTRL_A,
	/* 14 */    _ISCNTRL_A,
	/* 15 */    _ISCNTRL_A,
	/* 16 */    _ISCNTRL_A,
	/* 17 */    _ISCNTRL_A,
	/* 18 */    _ISCNTRL_A,
	/* 19 */    _ISCNTRL_A,
	/* 1A */    _ISCNTRL_A,
	/* 1B */    _ISCNTRL_A,
	/* 1C */    _ISCNTRL_A,
	/* 1D */    _ISCNTRL_A,
	/* 1E */    _ISCNTRL_A,
	/* 1F */    _ISCNTRL_A,
	/* 20 */    _ISPRINT_A|_ISSPACE_A,
	/* 21 */    _ISPRINT_A|_ISPUNCT_A,
	/* 22 */    _ISPRINT_A|_ISPUNCT_A,
	/* 23 */    _ISPRINT_A|_ISPUNCT_A,
	/* 24 */    _ISPRINT_A|_ISPUNCT_A,
	/* 25 */    _ISPRINT_A|_ISPUNCT_A,
	/* 26 */    _ISPRINT_A|_ISPUNCT_A,
	/* 27 */    _ISPRINT_A|_ISPUNCT_A,
	/* 28 */    _ISPRINT_A|_ISPUNCT_A,
	/* 29 */    _ISPRINT_A|_ISPUNCT_A,
	/* 2A */    _ISPRINT_A|_ISPUNCT_A,
	/* 2B */    _ISPRINT_A|_ISPUNCT_A,
	/* 2C */    _ISPRINT_A|_ISPUNCT_A,
	/* 2D */    _ISPRINT_A|_ISPUNCT_A,
	/* 2E */    _ISPRINT_A|_ISPUNCT_A,
	/* 2F */    _ISPRINT_A|_ISPUNCT_A,
	/* 30 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 31 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 32 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 33 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 34 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 35 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 36 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 37 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 38 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 39 */    _ISPRINT_A|_ISDIGIT_A|_ISXDIGIT_A,
	/* 3A */    _ISPRINT_A|_ISPUNCT_A,
	/* 3B */    _ISPRINT_A|_ISPUNCT_A,
	/* 3C */    _ISPRINT_A|_ISPUNCT_A,
	/* 3D */    _ISPRINT_A|_ISPUNCT_A,
	/* 3E */    _ISPRINT_A|_ISPUNCT_A,
	/* 3F */    _ISPRINT_A|_ISPUNCT_A,
	/* 40 */    _ISPRINT_A|_ISPUNCT_A,
	/* 41 */    _ISPRINT_A|_ISUPPER_A|_ISXDIGIT_A,
	/* 42 */    _ISPRINT_A|_ISUPPER_A|_ISXDIGIT_A,
	/* 43 */    _ISPRINT_A|_ISUPPER_A|_ISXDIGIT_A,
	/* 44 */    _ISPRINT_A|_ISUPPER_A|_ISXDIGIT_A,
	/* 45 */    _ISPRINT_A|_ISUPPER_A|_ISXDIGIT_A,
	/* 46 */    _ISPRINT_A|_ISUPPER_A|_ISXDIGIT_A,
	/* 47 */    _ISPRINT_A|_ISUPPER_A,
	/* 48 */    _ISPRINT_A|_ISUPPER_A,
	/* 49 */    _ISPRINT_A|_ISUPPER_A,
	/* 4A */    _ISPRINT_A|_ISUPPER_A,
	/* 4B */    _ISPRINT_A|_ISUPPER_A,
	/* 4C */    _ISPRINT_A|_ISUPPER_A,
	/* 4D */    _ISPRINT_A|_ISUPPER_A,
	/* 4E */    _ISPRINT_A|_ISUPPER_A,
	/* 4F */    _ISPRINT_A|_ISUPPER_A,
	/* 50 */    _ISPRINT_A|_ISUPPER_A,
	/* 51 */    _ISPRINT_A|_ISUPPER_A,
	/* 52 */    _ISPRINT_A|_ISUPPER_A,
	/* 53 */    _ISPRINT_A|_ISUPPER_A,
	/* 54 */    _ISPRINT_A|_ISUPPER_A,
	/* 55 */    _ISPRINT_A|_ISUPPER_A,
	/* 56 */    _ISPRINT_A|_ISUPPER_A,
	/* 57 */    _ISPRINT_A|_ISUPPER_A,
	/* 58 */    _ISPRINT_A|_ISUPPER_A,
	/* 59 */    _ISPRINT_A|_ISUPPER_A,
	/* 5A */    _ISPRINT_A|_ISUPPER_A,
	/* 5B */    _ISPRINT_A|_ISPUNCT_A,
	/* 5C */    _ISPRINT_A|_ISPUNCT_A,
	/* 5D */    _ISPRINT_A|_ISPUNCT_A,
	/* 5E */    _ISPRINT_A|_ISPUNCT_A,
	/* 5F */    _ISPRINT_A|_ISPUNCT_A,
	/* 60 */    _ISPRINT_A|_ISPUNCT_A,
	/* 61 */    _ISPRINT_A|_ISLOWER_A|_ISXDIGIT_A,
	/* 62 */    _ISPRINT_A|_ISLOWER_A|_ISXDIGIT_A,
	/* 63 */    _ISPRINT_A|_ISLOWER_A|_ISXDIGIT_A,
	/* 64 */    _ISPRINT_A|_ISLOWER_A|_ISXDIGIT_A,
	/* 65 */    _ISPRINT_A|_ISLOWER_A|_ISXDIGIT_A,
	/* 66 */    _ISPRINT_A|_ISLOWER_A|_ISXDIGIT_A,
	/* 67 */    _ISPRINT_A|_ISLOWER_A,
	/* 68 */    _ISPRINT_A|_ISLOWER_A,
	/* 69 */    _ISPRINT_A|_ISLOWER_A,
	/* 6A */    _ISPRINT_A|_ISLOWER_A,
	/* 6B */    _ISPRINT_A|_ISLOWER_A,
	/* 6C */    _ISPRINT_A|_ISLOWER_A,
	/* 6D */    _ISPRINT_A|_ISLOWER_A,
	/* 6E */    _ISPRINT_A|_ISLOWER_A,
	/* 6F */    _ISPRINT_A|_ISLOWER_A,
	/* 70 */    _ISPRINT_A|_ISLOWER_A,
	/* 71 */    _ISPRINT_A|_ISLOWER_A,
	/* 72 */    _ISPRINT_A|_ISLOWER_A,
	/* 73 */    _ISPRINT_A|_ISLOWER_A,
	/* 74 */    _ISPRINT_A|_ISLOWER_A,
	/* 75 */    _ISPRINT_A|_ISLOWER_A,
	/* 76 */    _ISPRINT_A|_ISLOWER_A,
	/* 77 */    _ISPRINT_A|_ISLOWER_A,
	/* 78 */    _ISPRINT_A|_ISLOWER_A,
	/* 79 */    _ISPRINT_A|_ISLOWER_A,
	/* 7A */    _ISPRINT_A|_ISLOWER_A,
	/* 7B */    _ISPRINT_A|_ISPUNCT_A,
	/* 7C */    _ISPRINT_A|_ISPUNCT_A,
	/* 7D */    _ISPRINT_A|_ISPUNCT_A,
	/* 7E */    _ISPRINT_A|_ISPUNCT_A,
	/* 7F */    0,
	/* 80 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 90 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* A0 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* B0 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* C0 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* D0 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* E0 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* F0 */    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

/*%PAGE																*/
/* toupper table	 												*/
static short _toupper_a_tbl[] =
	{
	/* -1 */    -1,
	/* 00 */    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	/* 10 */    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	/* 20 */    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	/* 30 */    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	/* 40 */    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	/* 50 */    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	/* 60 */    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	/* 70 */    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	/* 80 */    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	/* 90 */    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	/* A0 */    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	/* B0 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	/* C0 */    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	/* D0 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	/* E0 */    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	/* F0 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
	};

/*%PAGE																*/
/* tolower table 													*/
static short _tolower_a_tbl[] =
	{
	/* -1 */    -1,
	/* 00 */    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	/* 10 */    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	/* 20 */    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	/* 30 */    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	/* 40 */    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	/* 50 */    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	/* 60 */    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	/* 70 */    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	/* 80 */    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	/* 90 */    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	/* A0 */    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	/* B0 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	/* C0 */    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	/* D0 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	/* E0 */    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	/* F0 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
	};

const short* _ctype_a   = &_ctype_a_tbl[1];
const short* _toupper_a = &_toupper_a_tbl[1];
const short* _tolower_a = &_tolower_a_tbl[1];

