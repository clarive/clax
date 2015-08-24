/*
** GGS 3/25/97 These macros are now in ascii_a.h. 
** This file should no longer be used, and should not be 
** merged into the IRIS code base.
*/

/****************************************************************/
/* Name: ctype_a.h                                              */
/*                                                              */
/* Function: Provides prototypes and #define for ascii library  */
/*           routines which support functions from ctype.h      */
/*           ctype.h                                            */
/*                                                              */
/****************************************************************/
/* ASCII versions of ctype macros */
#ifndef _CTYPE_A_H
#define _CTYPE_A_H

#ifdef __cplusplus
extern "C" {
#endif

extern const short *_ctype_a;
extern const short *_tolower_a;
extern const short *_toupper_a;

#define _ISUPPER_A  0x01
#define _ISLOWER_A  0x02
#define _ISDIGIT_A  0x04
#define _ISXDIGIT_A 0x08
#define _ISCNTRL_A  0x10
#define _ISSPACE_A  0x20
#define _ISPUNCT_A  0x40
#define _ISPRINT_A  0x80

/* inhibit sourcing of <ctype.h> */
#define __ctype_h

#ifdef isalnum
#undef isalnum
#endif
#define isalnum(c)    (int)(_ctype_a[(c)] & (_ISUPPER_A|_ISLOWER_A|_ISDIGIT_A))

#ifdef isalpha
#undef isalpha
#endif
#define isalpha(c)    (int)(_ctype_a[(c)] & (_ISUPPER_A|_ISLOWER_A))

#ifdef iscntrl
#undef iscntrl
#endif
#define iscntrl(c)    (int)(_ctype_a[(c)] & (_ISCNTRL_A))

#ifdef isdigit
#undef isdigit
#endif
#define isdigit(c)    (int)(_ctype_a[(c)] & (_ISDIGIT_A))

#ifdef isgraph
#undef isgraph
#endif
#define isgraph(c)    (int)(_ctype_a[(c)] & (_ISPUNCT_A|_ISUPPER_A|_ISLOWER_A|_ISDIGIT_A))

#ifdef islower
#undef islower
#endif
#define islower(c)    (int)(_ctype_a[(c)] & (_ISLOWER_A))

#ifdef isprint
#undef isprint
#endif
#define isprint(c)    (int)(_ctype_a[(c)] & (_ISPRINT_A))

#ifdef ispunct
#undef ispunct
#endif
#define ispunct(c)    (int)(_ctype_a[(c)] & (_ISPUNCT_A))

#ifdef isspace
#undef isspace
#endif
#define isspace(c)    (int)(_ctype_a[(c)] & (_ISSPACE_A))

#ifdef isupper
#undef isupper
#endif
#define isupper(c)    (int)(_ctype_a[(c)] & (_ISUPPER_A))

#ifdef isxdigit
#undef isxdigit
#endif
#define isxdigit(c)   (int)(_ctype_a[(c)] & (_ISXDIGIT_A))

#ifdef tolower
#undef tolower
#endif
#define tolower(c)    (int)(_tolower_a[(c)])

#ifdef toupper
#undef toupper
#endif
#define toupper(c)    (int)(_toupper_a[(c)])

#if defined (__cplusplus)
}
#endif
#endif
