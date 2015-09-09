PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall -Icontrib/mbedtls -Icontrib/multipart-parser-c -Icontrib/jsmn -Icontrib -D_ALL_SOURCE -D_POSIX_SOURCE -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED
LFLAGS=
LIBS=contrib/mbedtls/*.o contrib/jsmn/*.o contrib/http_parser/*.o contrib/multipart-parser-c/*.o

all: lib $(PROGRAM)

lib: mbedtls jsmn http_parser multipart_parser_c $(OBJECTS)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

depend: .depend

.depend: $(SOURCES)
	rm -rf ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

include .depend

jsmn:
	$(MAKE) -C contrib/jsmn CFLAGS="-DJSMN_PARENT_LINKS"

mbedtls:
	$(MAKE) -C contrib/mbedtls

http_parser:
	$(MAKE) -C contrib/http_parser package

multipart_parser_c:
	$(MAKE) -C contrib/multipart-parser-c

tests: lib
	$(MAKE) -C tests

check: tests
	$(MAKE) -C tests check

check-valgrind: tests
	$(MAKE) -C tests check-valgrind

bench: lib
	$(MAKE) -C bench bench

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	rm -f .depend
	$(MAKE) -C contrib/mbedtls clean
	$(MAKE) -C contrib/jsmn clean
	$(MAKE) -C contrib/http_parser clean
	$(MAKE) -C contrib/multipart-parser-c clean
	$(MAKE) -C tests clean
	$(MAKE) -C bench clean
