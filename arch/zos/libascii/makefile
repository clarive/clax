#
#  Makefile to create archive "libascii.a"
#
 
 
# To build with debug option, uncomment the following
# DEBUG		= -g
 
 
# Define compiler options
#		CC			= Compiler name
#		CFLAGS		= Compiler flags
#
CC		= unset _CXX_OSUFFIX; cxx
CFLAGS	= -c $(DEBUG) -I./ -D_ALL_SOURCE

# Define list of programs to be processed
OBJS	= ctype_a.o ctypet_a.o dirent_a.o dll_a.o dynit_a.o \
		  fcntl_a.o grp_a.o iconv_a.o \
		  inet_a.o ipc_a.o langin_a.o locale_a.o init_a.o \
		  netdb_a.o netdb2_a.o nl_typ_a.o \
		  print_a.o pwd_a.o regex_a.o rexec_a.o scanf_a.o stat_a.o \
		  stdio_a.o stdlib_a.o string_a.o time_a.o trans_a.o \
		  unistd_a.o utime_a.o utsnam_a.o xdrfloa.o 


# Define archive dependencies and archive command
libascii.a	: $(OBJS)
#	cat license | more
	ar -rc libascii.a $(OBJS) 

# Define compile dependencies and command for each part
ctype_a.o	: ctype_a.c
	$(CC) $(CFLAGS) -D__STRING_CODE_SET__="ISO8859-1" ctype_a.c 
ctypet_a.o	: ctypet_a.c
	$(CC) $(CFLAGS) -D__STRING_CODE_SET__="ISO8859-1" ctypet_a.c 
dynit_a.o	: dynit_a.c
	$(CC) $(CFLAGS) -W "0,langlvl(extended)" dynit_a.c
.c.o		:
 
clean:
	rm -f *.a *.o

##### END OF FILE #####
