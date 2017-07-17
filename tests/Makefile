PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
override CFLAGS += -I../contrib -I../contrib/mbedtls -I../
LFLAGS=
LIBS=../clax_*.o ../popen2.o ../scandir.o ../contrib/*/*.o

RM = rm
RMF = rm -f
RMRF = rm -rf
MV = mv
CP = cp
EXE = a.out

ifeq ($(OS),Windows_NT)
	WINDOWS=1
ifneq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
	WINDOWS_CMD=1
endif
endif

ifeq ($(WINDOWS),1)
	EXE     = a.exe
	PROGRAM =  clax.exe
	LIBS    += -lws2_32
	CFLAGS  += -std=gnu99 -pedantic -Wall
ifeq ($(WINDOWS_CMD),1)
	RM      = del
	RMF     = del /f
	RMRF    = rd /s /q
	MV      = move
	CP      = copy
endif
else
ifeq ($(MVS),1)
	CC      =  c99
	CFLAGS  += -DMVS -D_ALL_SOURCE -DMBEDTLS_NO_DEFAULT_ENTROPY_SOURCES -DMBEDTLS_NO_PLATFORM_ENTROPY
else
	CFLAGS  += -std=gnu99 -pedantic -Wall
endif
endif

all: lib $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS)
	$(CP) $(EXE) $(PROGRAM)

ifneq ($(MVS),1)
depend: .depend

.depend: $(SOURCES)
	$(CC) $(CFLAGS) -MM $^ > ./.depend

-include .depend
endif

lib:
	$(MAKE) -C ../ OS=$(OS) ARCH=$(ARCH)

check: $(PROGRAM)
	./$(PROGRAM)

check-valgrind: $(PROGRAM)
	valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(PROGRAM)

check-coverage:
	$(MAKE) clean
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage -O0 $(CFLAGS)" check

clean:
	$(RMF) $(PROGRAM) $(OBJECTS)
	$(RMF) a.out
	$(RMF) *.gcno *.gcda
	$(RMF) .depend
