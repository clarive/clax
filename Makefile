PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
COVERAGE_GRAPH=$(SOURCES:.c=.gcno)
COVERAGE_DATA=$(SOURCES:.c=.gcda)
CFLAGS=-std=gnu99 -pedantic -Wall \
	   -Icontrib \
	   -D_ALL_SOURCE
LFLAGS=
LIBS=contrib/*/*.o

ifeq ($(WINDOWS),1)
	PROGRAM=clax.exe
	LIBS+=-lws2_32
else
endif

all: lib $(PROGRAM)

lib: mbedtls jsmn http-parser multipart-parser-c inih base64 slre $(OBJECTS)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

depend: .depend

.depend: $(SOURCES)
	rm -rf ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

-include .depend

inih:
	$(MAKE) -C contrib/inih

jsmn:
	$(MAKE) -C contrib/jsmn CFLAGS="-DJSMN_PARENT_LINKS"

mbedtls:
	$(MAKE) -C contrib/mbedtls

http-parser:
	$(MAKE) -C contrib/http-parser package

multipart-parser-c:
	$(MAKE) -C contrib/multipart-parser-c

base64:
	$(MAKE) -C contrib/base64

slre:
	$(MAKE) -C contrib/slre

tests: lib
	$(MAKE) -C tests

tests_func: tests
	$(MAKE) -C tests_func

check: tests
	$(MAKE) -C tests check

check-all: tests $(PROGRAM)
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

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	rm -f *.exe
	rm -f .depend
	rm -f *.gcno *.gcda
	rm -rf coverage
	$(MAKE) -C tests clean
	$(MAKE) -C tests_func clean
	$(MAKE) -C bench clean

distclean:
	$(MAKE) clean
	$(MAKE) -C contrib/mbedtls clean
	$(MAKE) -C contrib/jsmn clean
	$(MAKE) -C contrib/http-parser clean
	$(MAKE) -C contrib/multipart-parser-c clean
	$(MAKE) -C contrib/inih clean
	$(MAKE) -C contrib/base64 clean
	$(MAKE) -C contrib/slre clean
