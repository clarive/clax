/********************************************************************/
/*                                                                  */
/* Name		: 	_Ascii_a.h	                                        */
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/*                   	                                        	*/
/* Function	: 	Provides prototypes and #define for ascii library  	*/
/*           	routines which support ascii input characters.     	*/
/*                   	                                        	*/
/********************************************************************/
#ifndef _Ascii_a
 #define _Ascii_a

/* Define __CTYPE_MACROS__ to use isxxxxx() macros.                */
/* The macros have better performance than the function calls      */
/* such as __isalnum_a() .                                         */
#define __CTYPE_MACROS__

#ifdef __cplusplus
   extern "C" {
#endif

/* Functions defined in CTYPE.H										*/
/* if the isaxxx calls have been defined and using ASCII code,		*/
/* undefine them now												*/
#ifdef __ctype__
#ifdef __STRING_CODE_SET__
#ifndef __CTYPE_MACROS__
  #ifdef isalnum
   #undef isalnum
   #undef isalpha
   #undef iscntrl
   #undef isdigit
   #undef isgraph
   #undef islower
   #undef isprint
   #undef ispunct
   #undef isspace
   #undef isupper
   #undef isxdigit
   #undef tolower
   #undef toupper
  #endif
 #define isalnum(a)  			__isalnum_a(a)
 #define isalpha(a)  			__isalpha_a(a)
 #define iscntrl(a)  			__iscntrl_a(a) 
 #define isdigit(a)  			__isdigit_a(a)
 #define isgraph(a)  			__isgraph_a(a)
 #define islower(a)  			__islower_a(a)
 #define isprint(a)  			__isprint_a(a)
 #define ispunct(a)  			__ispunct_a(a)
 #define isspace(a)  			__isspace_a(a)
 #define isupper(a)  			__isupper_a(a)
 #define isxdigit(a) 			__isxdigit_a(a)
 #define tolower(a)  			__tolower_a(a)
 #define toupper(a)  			__toupper_a(a)
 int	__isalnum_a (int);
 int	__isalpha_a (int);        
 int	__iscntrl_a (int);
 int	__isdigit_a (int);
 int	__isgraph_a (int);        
 int	__islower_a (int);
 int	__ispunct_a (int);        
 int	__isprint_a (int);
 int	__isspace_a (int);
 int	__isupper_a (int);
 int	__isxdigit_a (int);
 int	__tolower_a (int);
 int	__toupper_a (int);
#else
/* __CTYPE_MACROS__ specified */
/* ASCII versions of ctype macros */

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

#endif		/* __CTYPE_MACROS__ */
#endif      /* __STRING_CODE_SET__ */
#endif      /* __ctype__ */


/* Functions defined in DLL.H										*/
#ifdef __dll
 #ifdef __STRING_CODE_SET__
   #define dllload(a) 			__dllload_a(a)
   #define dllqueryfn(a,b) 		__dllqueryfn_a(a,b)
 #endif
  dllhandle* __dllload_a(const char * dllname);
  void (* __dllqueryfn_a(dllhandle *dllHandle, char *funcName)) ();
#endif

/* Functions defined in DIRENT.H									*/
#ifdef __dirent
 #ifdef __STRING_CODE_SET__
   #define opendir(a)			__opendir_a(a)
   #define readdir(a)			__readdir_a(a)
 #endif
 DIR *		__opendir_a(const char *);
 struct dirent *__readdir_a(DIR *);
#endif

/* Functions defined in DYNIT.H										*/
#ifdef __dynit 
 #ifdef __STRING_CODE_SET__
   #define dynalloc(a)			__dynalloc_a(a)
 #endif
 int		__dynalloc_a(__dyn_t *);
#endif

/* Functions defined in FCNTL.H										*/
#ifdef __fcntl
 #ifdef __STRING_CODE_SET__
   #define creat(a,b)			__creat_a(a,b)
   #define open					__open_a
 #endif
 int		__creat_a(const char *, mode_t);
 int 		__open_a(const char *, int, ...);
#endif
  
/* Functions defined in GRP.H										*/
#ifdef __grp
 #ifdef __STRING_CODE_SET__
   #define getgrnam(a)			__getgrnam_a(a)
 #endif
 struct group * __getgrnam_a(const char *);
#endif

/* Functions defined in ICONV.H										*/
#ifdef __iconv__
 #ifdef __STRING_CODE_SET__
   #define iconv_open(a,b)		__iconv_open_a(a,b)
 #endif
 iconv_t	__iconv_open_a(const char *, const char *);
#endif

/* Functions defined in ARPA/INET.H									*/
#ifdef __arpa_inet
 #ifdef __STRING_CODE_SET__
   #define inet_addr(a)			__inet_addr_a(a)
   #define inet_ntoa(a)			__inet_ntoa_a(a)
 #endif
 in_addr_t	__inet_addr_a(const char *);
 char *		__inet_ntoa_a(struct in_addr);
#endif

/* Functions defined in LANGINFO.H									*/
#ifdef __langinfo
 #ifdef __STRING_CODE_SET__
   #define nl_langinfo(a) 		__nl_langinfo_a(a)
 #endif
  extern char *__nl_langinfo_a(nl_item);
#endif

/* Functions defined in LOCALE.H									*/
#ifdef __locale                                             
 #ifdef __STRING_CODE_SET__
   #define localeconv       	__localeconv_a
   #define setlocale(a,b)   	__setlocale_a(a,b)
 #endif
 struct lconv *__localeconv_a(void);                      
 char *		__setlocale_a(int, const char *);                     
#endif                                                      

/* Functions defined in NETDB.H										*/
#ifdef __netdb
 #ifdef __STRING_CODE_SET__
   #ifdef _OE_SOCKETS
     #define gethostbyaddr(a,b,c)	__gethostbyaddr_OE_a(a,b,c)
     #define gethostbyname(a) 		__gethostbyname_OE_a(a)
     #define getservbyname(a,b) 	__getservbyname_OE_a(a,b)
   #else
     #define gethostbyaddr(a,b,c)	__gethostbyaddr_a(a,b,c)
     #define gethostbyname(a) 		__gethostbyname_a(a)
     #define getservbyname(a,b) 	__getservbyname_a(a,b)
   #endif
 #endif
   #ifdef _OE_SOCKETS
     struct hostent *__gethostbyaddr_OE_a(char *, int, int);
     struct hostent *__gethostbyname_OE_a(char *);
     struct servent *__getservbyname_OE_a(char *, char *);
   #else
     struct hostent *__gethostbyaddr_a(const void *, size_t, int);
     struct hostent *__gethostbyname_a(const char *);
     struct servent *__getservbyname_a(const char *, const char *);
   #endif
#endif

/* Functions defined in PWD.H										*/
#ifdef __pwd
 #ifdef __STRING_CODE_SET__
   #define getpwnam(a)			__getpwnam_a(a)
   #define getpwuid(a)			__getpwuid_a(a)
 #endif
 struct passwd * __getpwnam_a(const char *);
 struct passwd * __getpwuid_a(uid_t);
#endif

/* Functions defined in REGEX.H										*/
#ifdef __regex
 #ifdef __STRING_CODE_SET__
   #define regcomp(a,b,c) 		__regcomp_a(a,b,c)
 #endif
 int 		__regcomp_a(regex_t *, const char *, int);
#endif
 
/* Functions defined in REXEC.H										*/
#ifdef __rexec
 #ifdef __STRING_CODE_SET__
   #define rexec(a,b,c,d,e,f) 	__rexec_a(a,b,c,d,e,f)
 #endif
 int		__rexec_a(char **, int, char *, char *, char *, int *);
#endif

/* Functions defined in SYS/STAT.H									*/
#ifdef __stat
 #ifdef __STRING_CODE_SET__
   #define chmod(a,b)  			__chmod_a(a,b)
   #define mkdir(a,b)  			__mkdir_a(a,b)
   #define stat(a,b)   			__stat_a(a,b)
   #define mknod(a,b,c)			__mknod_a(a,b,c) 
 #endif
 int 		__chmod_a(const char *, mode_t);
 int 		__mkdir_a(const char *, mode_t);
 int 		__stat_a(const char *, struct stat *);
 int 		__mknod_a(const char *, mode_t, rdev_t); 
#endif

/* Functions defined in SYS/STATVFS.H								*/
#ifdef __statvfs
 #ifdef __STRING_CODE_SET__
   #define statvfs(a,b) 		__statvfs_a(a,b)
 #endif
 int 		__statvfs_a(const char *, struct statvfs *);
#endif

/*%PAGE																*/
/* Functions defined in STDIO.H										*/
#ifdef __stdio
 #ifdef __STRING_CODE_SET__
   #ifdef getc
     #undef getc
     #undef getchar
   #endif
   #ifdef putc
     #undef putc
   #endif
   #ifdef putchar
     #undef putchar
   #endif
   #ifdef fputs
     #undef fputs
   #endif
   #define fdopen(a,b)			__fdopen_a(a,b)
   #define fgets(a,b,c)			__fgets_a(a,b,c)
   #define fopen(a,b)     		__fopen_a(a,b)
   #define fprintf        		__fprintf_a
   #define fputc(a,b)			__fputc_a(a,b)
   #define fputs(a,b)			__fputs_a(a,b)
   #define fread(a,b,c,d)		__fread_a(a,b,c,d)
   #define freopen(a,b,c) 		__freopen_a(a,b,c)
   #define fscanf         		__fscanf_a
   #define fwrite(a,b,c,d)		__fwrite_a(a,b,c,d)
   #define fwrite_allascii(a,b,c,d)	__fwrite_allascii_a(a,b,c,d) 
   #define getc(a)        		__getc_a(a)
   #define getc_ascii(a)		__getc_ascii_a(a)
   #define getchar()      		__getc_a(stdin)
   #define getopt(a,b,c)		__getopt_a(a,b,c)
   #define gets(a)        		__gets_a(a)
   #define perror(a)      		__perror_a(a)
   #define popen(a,b)			__popen_a(a,b)
   #define printf         		__printf_a
   #define putc(a,b)      		__putc_a(a,b)
   #define putchar(a)     		__putc_a((a),stdout) 
   #define puts(a)				__puts_a(a)
   #define remove(a)      		__remove_a(a)
   #define rename(a,b)   		__rename_a(a,b)
   #define scanf          		__scanf_a
   #define setvbuf(a,b,c,d)		__setvbuf_a(a,b,c,d)
   #define sscanf         		__sscanf_a
   #define stat(a,b)			__stat_a(a,b)
   #define statvfs(a,b)			__statvfs_a(a,b)
   #define sprintf       		__sprintf_a
   #define tempnam(a,b)   		__tempnam_a(a,b)
   #define tmpnam(a)   			__tmpnam_a(a)
   #define vfprintf       		__vfprintf_a
   #define vprintf        		__vprintf_a
   #define vsprintf       		__vsprintf_a
 #endif
 FILE *		__fdopen_a(int, const char *);
 char *		__fgets_a(char *, int, FILE *);
 FILE *		__fopen_a(const char *, const char *);
 int		__fputc_a(int, FILE *);
 int		__fputs_a(char *, FILE *);   
 int  		__fprintf_a (FILE *, const char *, ...);
 int		__fread_a(void *, size_t, size_t, FILE *);
 FILE *		__freopen_a(const char *, const char *, FILE *);
 int  		__fscanf_a (FILE *, const char *, ...);
 size_t		__fwrite_a(const void *, size_t, size_t, FILE *);
 size_t		__fwrite_allascii_a(const void *, size_t, size_t, FILE *);
 int 		__getc_a(FILE *);
 int 		__getc_ascii_a(FILE *);
 /* Note that getopt prototype changed from C run-time definition	*/
 /* See stdio_a.c for additional information.						*/
 /* int		__getopt_a(int, char * const[], const char *);			*/
 int		__getopt_a(int, char *[], const char *);
 char *		__gets_a(char *);
 void 		__perror_a (const char *);
 FILE *		__popen_a(const char *, const char *);
 int  		__printf_a (const char *, ...);
 int 		__putc_a(int, FILE *);
 int		__puts_a(char *);
 int 		__remove_a(const char *);
 int 		__rename_a(const char *, const char *);
 int  		__scanf_a (const char *, ...);
 int  		__sscanf_a (const char *, const char *, ...);
 int		__setvbuf_a(FILE *, char *, int, size_t);
 int  		__sprintf_a (char *, const char *, ...);
 int		__stat_a(const char *, struct stat *);
 int		__statvfs_a(const char *, struct statvfs *);
 char  *	__tempnam_a(const char *, const char *);
 char  *	__tmpnam_a(char *);
 int  		__vfprintf_a (FILE *, const char *, va_list);
 int  		__vprintf_a (const char *, va_list);
 int  		__vsprintf_a (char *, const char *, va_list);
#endif

/*%PAGE																*/
/* Functions defined in STDIO.H										*/
#ifdef __stdlib
 #ifdef __STRING_CODE_SET__
   #define atof(a)				__atof_a(a)
   #define atoi(a)				__atoi_a(a)
   #define atol(a)				__atol_a(a)
   #define ecvt(a,b,c,d)		__ecvt_a(a,b,c,d)
   #define fcvt(a,b,c,d)		__fcvt_a(a,b,c,d)
   #define gcvt(a,b,c)			__gcvt_a(a,b,c)
   #define getenv(a)			__getenv_a(a)
   #define mbtow(a,b,c)			__mbtow_a(a,b,c)
   #define mbstowcs(a,b,c)		__mbstowcs_a(a,b,c)
   #define mktemp(a)			__mktemp_a(a)
   #define ptsname(a)			__ptsname_a(a)
   #define putenv(a)			__putenv_a(a)
   #define setenv(a,b,c)		__la_setenv_a(a,b,c)
   #define setkey(a)			__setkey_a(a)
   #define strtod(a,b)			__strtod_a(a,b)
   #define strtol(a,b,c)		__strtol_a(a,b,c)
   #define strtoul(a,b,c)		__strtoul_a(a,b,c)
   #define system(a)			__system_a(a)
 #endif
 double 	__atof_a(const char *);
 int 		__atoi_a(const char *);
 long int 	__atol_a(const char *);
 char *		__ecvt_a(double, int, int *, int *);
 char *		__fcvt_a(double, int, int *, int *);
 char *		__gcvt_a(double, int, char *);
 char *		__getenv_a(const char *);
 int		__mbtow_a(wchar_t *, const char *, size_t);
 int		__mbstowcs_a(wchar_t *, const char *, size_t);
 char *		__mktemp_a(char *);
 char *		__ptsname_a(int);
 int		__putenv_a(const char *);
 int		__la_setenv_a(char *,char *, int);
 void		__setkey_a(const char *);
 double 	__strtod_a(const char *, char **);                  
 long int 	__strtol_a(const char *, char **, int);           
 unsigned long int __strtoul_a(const char *, char **, int); 
 int 		__system_a(const char *);                             
#endif

/*%PAGE																*/
/* Functions defined in STRING.H									*/
#ifdef __string
 #ifdef __STRING_CODE_SET__
   #define strerror(a)			__strerror_a(a)
 #endif
 char *			__strerror_a(int);
#endif

/* Functions defined in STRINGS.H									*/
#ifdef __strings_h
 #ifdef __STRING_CODE_SET__
   #define strcasecmp(a,b)		__strcasecmp_a(a,b)
   #define strncasecmp(a,b,c)	__strncasecmp_a(a,b,c)
 #endif
 int  		__strcasecmp_a (const char *, const char *);
 int  		__strncasecmp_a (const char *, const char *, size_t);
#endif

/* Functions defined in TIME.H										*/
#ifdef __time
 #ifdef __STRING_CODE_SET__
   #define asctime(a)			__asctime_a(a)
   #ifdef ctime
	   #undef ctime
   #endif
   #define ctime(a)				__ctime_a(a)
 #endif
 char *		__asctime_a(const struct tm *);
 char *		__ctime_a(const time_t *);
#endif

/* Functions defined in SYS/IPC.H									*/
#ifdef __sys_ipc
 #ifdef __STRING_CODE_SET__
   #define ftok(a,b)			__ftok_a(a,b)
 #endif
 key_t		__ftok_a(const char *, int);
#endif

/* Functions defined in SYS/TIMES.H and SYS/SYS_TIME.H				*/
#ifdef __sys_time
 #ifdef __STRING_CODE_SET__
   #define utimes(a,b)			__utimes_a(a,b)
 #endif
 int 		__utimes_a(const char *, const struct timeval [2]); 
#endif

/*%PAGE																*/
/* Functions defined in UNISTD.H									*/
#ifdef __unistd
 #ifdef __STRING_CODE_SET__
   #define access(a,b) 			__access_a(a,b)
   #define chdir(a)    			__chdir_a(a)
   #define chmod(a,b)			__chmod_a(a,b)
   #define chown(a,b,c)			__chown_a(a,b,c)
   #define execv(a,b) 			__execv_a(a,b)
   #define execve(a,b,c)		__execve_a(a,b,c)
   #define execvp(a,b)  		__execvp_a(a,b)
   #define getcwd(a,b) 			__getcwd_a(a,b)
   #define gethostname(a,b)		__gethostname_a(a,b)
   #define getlogin()			__getlogin_a()
   #define getpass(a)			__getpass_a(a)
   #define getwd(a)				__getwd_a(a)
   #define link(a,b)			__link_a(a,b)
   #define mkdir(a,b)			__mkdir_a(a,b)
   #define rmdir(a)    			__rmdir_a(a)
   #define unlink(a)   			__unlink_a(a)
 #endif
 int 		__access_a(const char *, int);
 int 		__chdir_a(const char *);
 int		__chmod_a(const char *, mode_t);
 int		__chown_a(const char *, uid_t, gid_t);
 int   		__execv_a(const char *, char *const []);
 int   		__execve_a(const char *, char *const [], char * const []);
 int   		__execvp_a(const char *, char *const []);
 char *		__getcwd_a(char *, size_t);
 int		__gethostname_a(char *, size_t);
 char *		__getlogin_a(void);
 char *		__getpass_a(const char *);
 char *		__getwd_a(char *);
 int		__link_a(const char *, const char *);
 int		__mkdir_a(const char *, mode_t);
 int   		__rmdir_a(const char *);
 int   		__unlink_a(const char *);
#endif 	

/*%PAGE																*/
/* Functions defined in UTIME.H										*/
#ifdef __utime
 #ifdef __STRING_CODE_SET__
   #define utime(a,b)			__utime_a(a,b)
 #endif
 int 		__utime_a(const char *, const struct utimbuf *);
#endif

/* Functions defined in SYS/UTSNAME.H								*/
#ifdef __utsname
   #define uname(a)				__uname_a(a)
 #ifdef __STRING_CODE_SET__
 #endif
 int		__uname_a(struct utsname *);
#endif

/* functions defined in nl_types.h                                  */
#ifdef __nl_types
 #ifdef __STRING_CODE_SET__
    #define catopen(a,b) __catopen_a(a,b)                           
    #define catgets(a,b,c,d) __catgets_a(a,b,c,d)
 #endif
 nl_catd   __catopen_a(const char *, int);
 char *    __catgets_a(nl_catd, int, int, const char *);
#endif

 int 		__argvtoascii_a(int, char *[]);
 int 		__argvtoebcdic_a(int, char *[]);
 char *		__getAstring1_a(const char *);
 char *		__getAstring2_a(const char *);
 char *		__getEstring1_a(const char *);
 char *		__getEstring2_a(const char *);
 char *		__getEstring3_a(const char *);
 char *		__getEstring4_a(const char *);
 void 		__toascii_a(char *, const char *);
 void 		__toasciilen_a(char *, const char *, int);
 void 		__toebcdic_a(char *, const char *);
 void 		__panic_a(char *);
 struct ATHD * getathdp();

/* Functions to convert between IEEE Floating Point and 
   S390 Hex Floating Point */

 void ConvertFloatToIEEE(void *source, void *destination);
 void ConvertDoubleToIEEE(void *source, void *destination);
 void ConvertIEEEToFloat(void *source, void *destination);
 void ConvertIEEEToDouble(void *source, void *destination);

 #ifdef __cplusplus
 }
 #endif
#endif
