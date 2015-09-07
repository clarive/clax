PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall \
	   -I../contrib/mbedtls -I../contrib/multipart-parser-c -I../contrib/jsmn -I../contrib -I../ \
	   -D_ALL_SOURCE -D_POSIX_SOURCE -D_XOPEN_SOURCE=700 -D_XOPEN_SOURCE_EXTENDED
LFLAGS=
LIBS=../clax_log.o ../clax_http.o ../clax_http_multipart.o ../clax_dispatcher.o ../clax_command.o ../clax_util.o ../popen2.o ../contrib/mbedtls/*.o ../contrib/http_parser/*.o ../contrib/multipart-parser-c/*.o

all: mbedtls http_parser $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

mbedtls:
	$(MAKE) -C ../contrib/mbedtls

http_parser:
	$(MAKE) -C ../contrib/http_parser package

check: $(PROGRAM)
	./test

clean:
	rm -f $(PROGRAM) $(OBJECTS)
