PROGRAM=ini.o
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall
LFLAGS=

all: $(PROGRAM)

$(OBJECTS): $(SOURCES)
	$(CC) -c $(CFLAGS) $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJECTS)
