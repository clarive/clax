PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
COVERAGE_GRAPH=$(SOURCES:.c=.gcno)
COVERAGE_DATA=$(SOURCES:.c=.gcda)
CFLAGS += -Icontrib -Icontrib/mbedtls -Iinclude -Isrc
LFLAGS += -pthread
LIBS   += $(wildcard contrib/*/*.o) $(wildcard contrib/*/*.a) $(wildcard contrib/*/.libs/*.a)

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
else
	UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -DHAVE_SNPRINTF
endif

endif

ifeq ($(WINDOWS),1)
	EXE     = a.exe
	PROGRAM =  clax.exe
	LIBS    += -liphlpapi -lpsapi -lws2_32 -luserenv
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

clax_version.h:
	OS=$(OS) ARCH=$(ARCH) WINDOWS=$(WINDOWS) sh util/makeversion.sh

lib: clax_version.h mbedtls http-parser multipart-parser-c inih base64 slre snprintf libuv $(OBJECTS)

$(OBJECTS): $(SOURCES)
	$(CC) -c $(CFLAGS) $^ $(LFLAGS)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(EXE)
	$(CP) $(EXE) $(PROGRAM)

inih:
	$(MAKE) -C contrib/inih CC="$(CC)" CFLAGS="$(CFLAGS)"

mbedtls:
	$(MAKE) -C contrib/mbedtls -f Makefile.clax CC="$(CC)" CFLAGS="$(CFLAGS) -D__STRING_CODE_SET__=\"ISO8859-1\""

http-parser:
	$(MAKE) -C contrib/http-parser -f Makefile.clax CC="$(CC)" CFLAGS="$(CFLAGS) -D__STRING_CODE_SET__=\"ISO8859-1\""

multipart-parser-c:
	$(MAKE) -C contrib/multipart-parser-c -f Makefile.clax CC="$(CC)" CFLAGS="$(CFLAGS) -D__STRING_CODE_SET__=\"ISO8859-1\""

base64:
	$(MAKE) -C contrib/base64 CC="$(CC)" CFLAGS="$(CFLAGS)"

slre:
	$(MAKE) -C contrib/slre CC="$(CC)" CFLAGS="$(CFLAGS)"

snprintf:
	$(MAKE) -C contrib/snprintf CC="$(CC)" CFLAGS="$(CFLAGS)"

libuv:
	$(MAKE) -C contrib/libuv -f Makefile.clax CC="$(CC)" AR="$(AR)" CFLAGS="$(CFLAGS)" WINDOWS="$(WINDOWS)"

tests: lib
	$(MAKE) -C tests CC="$(CC)" CFLAGS="$(CFLAGS)"

check: tests
	$(MAKE) -C tests check

check-valgrind: tests
	$(MAKE) -C tests check-valgrind

bench: lib
	$(MAKE) -C bench bench

check-coverage: coverage-prepare
	$(MAKE) -C tests check-coverage
	$(MAKE) coverage-report

check-coverage-all: coverage-prepare
	$(MAKE) -C tests check-coverage
	$(MAKE) coverage-report

coverage-report:
	lcov --rc lcov_branch_coverage=1 --no-external -c -d . -o coverage/coverage-test.info
	lcov --rc lcov_branch_coverage=1 -a coverage/coverage-base.info -a coverage/coverage-test.info -o coverage/coverage.info
	genhtml --branch-coverage coverage/coverage.info -o coverage

coverage-prepare:
	$(MAKE) clean
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage -O0 $(CFLAGS)"
	mkdir coverage
	lcov --rc lcov_branch_coverage=1 --no-external -c -i -d . -o coverage/coverage-base.info

archive: clax_version.h
	sh util/makearchive.sh

dist: all
	OS=$(OS) ARCH=$(ARCH) WINDOWS=$(WINDOWS) WINDOWS_CMD=$(WINDOWS_CMD) sh util/makedist.sh

clean:
	$(RMF)  $(PROGRAM) $(OBJECTS)
	$(RMF)  *.gch
	$(RMF)  a.out *.exe
	$(RMF)  clax_version.h OS ARCH
	$(RMF)  *.gcno *.gcda
	$(RMRF) coverage
	$(MAKE) -C tests clean
	$(MAKE) -C bench clean

distclean:
	$(MAKE) clean
	$(MAKE) -C contrib/mbedtls clean
	$(MAKE) -C contrib/http-parser clean
	$(MAKE) -C contrib/multipart-parser-c clean
	$(MAKE) -C contrib/inih clean
	$(MAKE) -C contrib/base64 clean
	$(MAKE) -C contrib/slre clean
	$(MAKE) -C contrib/snprintf clean
	$(MAKE) -C contrib/libuv clean
