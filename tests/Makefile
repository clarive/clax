PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=gnu99 -pedantic -Wall -O0\
	   -I../contrib -I../ \
	   -D_ALL_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
LFLAGS=
LIBS=../clax_*.o \
	 ../popen2.o \
	 ../contrib/*/*.o

ifeq ($(WINDOWS),1)
	LIBS+=-lws2_32
else
endif

all: lib $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS) -o $(PROGRAM)

depend: .depend

.depend: $(SOURCES)
	rm -rf ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

-include .depend

lib:
	$(MAKE) -C ../ lib

check: $(PROGRAM)
	./test

check-valgrind: $(PROGRAM)
	valgrind -v --leak-check=full --show-leak-kinds=all ./test

check-coverage:
	$(MAKE) clean
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage -O0 $(CFLAGS)" check

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	rm -f *.gcno *.gcda
	rm -f .depend
