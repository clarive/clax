PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-Icontrib/mbedtls -Icontrib/jsmn -D_ALL_SOURCE
LFLAGS=
LIBS=contrib/mbedtls/*.o contrib/jsmn/*.o

all: mbedtls jsmn $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

jsmn:
	$(MAKE) -C contrib/jsmn CFLAGS="-DJSMN_PARENT_LINKS"

mbedtls:
	$(MAKE) -C contrib/mbedtls

check:
	$(MAKE) -C tests check

test: check

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	$(MAKE) -C contrib/mbedtls clean
	$(MAKE) -C contrib/jsmn clean
