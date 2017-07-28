SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=gnu99 -pedantic -Wall
LFLAGS=

all: $(OBJECTS)

$(OBJECTS): $(SOURCES)
	$(CC) -c $(CFLAGS) $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJECTS)
	rm -f *.gch
