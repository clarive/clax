#ifndef CLAX_UTIL_H
#define CLAX_UTIL_H

#define sizeof_struct_member(type, member) sizeof(((type *)0)->member)

#define TRY if ((
#define GOTO ) < 0) {goto error;}

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#endif
