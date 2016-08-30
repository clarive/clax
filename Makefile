PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
COVERAGE_GRAPH=$(SOURCES:.c=.gcno)
COVERAGE_DATA=$(SOURCES:.c=.gcda)
CFLAGS += -Icontrib -Icontrib/mbedtls
LFLAGS=
LIBS=contrib/*/*.o

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

all: version lib $(PROGRAM)

version:
	OS=$(OS) ARCH=$(ARCH) WINDOWS=$(WINDOWS) sh util/makeversion.sh

lib: mbedtls http-parser multipart-parser-c inih base64 slre snprintf $(OBJECTS)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS)
	$(CP) $(EXE) $(PROGRAM)

ifneq ($(MVS),1)
depend: .depend

.depend: $(SOURCES)
	$(CC) $(CFLAGS) -MM $^ > .depend

-include .depend
endif

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

tests: lib
	$(MAKE) -C tests CC="$(CC)" CFLAGS="$(CFLAGS)"

tests_func: tests
	$(MAKE) -C tests_func CC="$(CC)" CFLAGS="$(CFLAGS)"

check: tests
	$(MAKE) -C tests check

check-all: tests tests_func $(PROGRAM)
	$(MAKE) -C tests check
	$(MAKE) -C tests_func check

check-valgrind: tests
	$(MAKE) -C tests check-valgrind

bench: lib
	$(MAKE) -C bench bench

check-coverage: coverage-prepare
	$(MAKE) -C tests check-coverage
	$(MAKE) coverage-report

check-coverage-all: coverage-prepare
	$(MAKE) -C tests check-coverage
	$(MAKE) -C tests_func check-coverage
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

archive: version
	sh util/makearchive.sh

dist: all
	sh util/makedist.sh WINDOWS=$(WINDOWS) WINDOWS_CMD=$(WINDOWS_CMD)

clean:
	$(RMF)  $(PROGRAM) $(OBJECTS)
	$(RMF)  *.exe
	$(RMF)  .depend
	$(RMF)  *.gcno *.gcda
	$(RMRF) coverage
	$(MAKE) -C tests clean
	$(MAKE) -C tests_func clean
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
