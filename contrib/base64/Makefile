SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=c99 -pedantic -Wall
LFLAGS=

all: $(OBJECTS)

$(OBJECTS): $(SOURCES)
	$(CC) -c $(CFLAGS) $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJECTS)
