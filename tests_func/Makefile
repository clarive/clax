PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-std=gnu99 -pedantic -Wall \
	   -I../contrib \
	   -I../tests/ \
	   -I../
LFLAGS=
LIBS=../clax_*.o \
	 ../popen2.o \
	 ../contrib/*/*.o \
	 ../tests/u_util.o

ifeq ($(WINDOWS),1)
	PROGRAM=test.exe
	LIBS+=-lws2_32
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
	$(MAKE) -C ../

check: $(PROGRAM)
	./$(PROGRAM)

check-coverage:
	$(MAKE) clean
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage -O0 $(CFLAGS)" check

clean:
	rm -f $(PROGRAM) $(OBJECTS) *.exe
	rm -f *.gcno *.gcda
	rm -f .depend
