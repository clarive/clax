PROGRAM=clax
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-Wall -Icontrib/mbedtls -Icontrib/multipart-parser-c -Icontrib/jsmn -Icontrib -D_ALL_SOURCE
LFLAGS=
LIBS=contrib/mbedtls/*.o contrib/jsmn/*.o contrib/http_parser/*.o contrib/multipart-parser-c/*.o

all: mbedtls jsmn http_parser multipart_parser_c $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

jsmn:
	$(MAKE) -C contrib/jsmn CFLAGS="-DJSMN_PARENT_LINKS"

mbedtls:
	$(MAKE) -C contrib/mbedtls

http_parser:
	$(MAKE) -C contrib/http_parser package

multipart_parser_c:
	$(MAKE) -C contrib/multipart-parser-c

check: $(PROGRAM)
	$(MAKE) -C tests check

test: check

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	$(MAKE) -C contrib/mbedtls clean
	$(MAKE) -C contrib/jsmn clean
	$(MAKE) -C contrib/http_parser clean
