PROGRAM=benchmark
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall \
	   -I../contrib/mbedtls -I../contrib/multipart-parser-c -I../contrib -I../ \
	   -D_ALL_SOURCE -D_POSIX_SOURCE -D_XOPEN_SOURCE=700 -D_XOPEN_SOURCE_EXTENDED
LFLAGS=
LIBS=../clax_*.o ../popen2.o ../contrib/mbedtls/*.o ../contrib/http-parser/*.o ../contrib/multipart-parser-c/*.o ../contrib/inih/*.o ../contrib/base64/*.o

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

clax:
	$(MAKE) -C ../

bench: clax $(PROGRAM)
	./benchmark

clean:
	rm -f $(PROGRAM) $(OBJECTS)
