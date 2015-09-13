PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall \
	   -I../contrib/mbedtls \
	   -I../contrib/multipart-parser-c \
	   -I../contrib/jsmn \
	   -I../contrib/base64 \
	   -I../contrib/inih \
	   -I../contrib/slre \
	   -I../contrib \
	   -I../ \
	   -I../tests/ \
	   -D_ALL_SOURCE -D_POSIX_SOURCE -D_XOPEN_SOURCE=700 -D_XOPEN_SOURCE_EXTENDED
LFLAGS=
LIBS=../clax_*.o \
	 ../popen2.o \
	 ../contrib/mbedtls/*.o \
	 ../contrib/http_parser/*.o \
	 ../contrib/multipart-parser-c/*.o \
	 ../contrib/inih/*.o \
	 ../contrib/base64/*.o \
	 ../contrib/slre/*.o

all: lib $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

depend: .depend

.depend: $(SOURCES)
	rm -rf ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

-include .depend

lib:
	$(MAKE) -C ../ lib

check: $(PROGRAM)
	./test

check-coverage:
	$(MAKE) clean
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage -O0 $(CFLAGS)" check

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	rm -f *.gcno *.gcda
	rm -f .depend
