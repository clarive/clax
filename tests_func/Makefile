PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall \
	   -I../contrib/mbedtls -I../contrib/multipart-parser-c -I../contrib/jsmn -I../contrib -I../ -I../tests/ \
	   -D_ALL_SOURCE -D_POSIX_SOURCE -D_XOPEN_SOURCE=700 -D_XOPEN_SOURCE_EXTENDED
LFLAGS=
LIBS=../clax_*.o ../popen2.o ../contrib/mbedtls/*.o ../contrib/http_parser/*.o ../contrib/multipart-parser-c/*.o

all: lib $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

depend: .depend

.depend: $(SOURCES)
	rm -rf ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

include .depend

lib:
	$(MAKE) -C ../ lib

check: $(PROGRAM)
	./test

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	rm -f .depend
