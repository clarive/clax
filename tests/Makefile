PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-I../ -I../contrib -I../contrib/mbedtls -D_ALL_SOURCE
LFLAGS=
LIBS=../clax_http.o ../clax_dispatcher.o ../clax_command.o ../popen2.o ../contrib/mbedtls/*.o ../contrib/http_parser/*.o

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
