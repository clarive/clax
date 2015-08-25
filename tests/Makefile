PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-I../contrib/mbedtls -D_ALL_SOURCE
LFLAGS=
LIBS=../contrib/mbedtls/*.o

all: mbedtls $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

mbedtls:
	$(MAKE) -C ../contrib/mbedtls

check: $(PROGRAM)
	./test

clean:
	rm -f $(PROGRAM) $(OBJECTS)
