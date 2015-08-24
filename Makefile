PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-Icontrib/mbedtls -D_ALL_SOURCE
LFLAGS=
LIBS=contrib/mbedtls/*.o

all: mbedtls $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

mbedtls:
	$(MAKE) -C contrib/mbedtls

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	$(MAKE) -C contrib/mbedtls clean
