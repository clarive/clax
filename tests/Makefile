PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -I../contrib -I../contrib/mbedtls -I../
LFLAGS=
LIBS=../clax_*.o ../popen2.o ../contrib/*/*.o

ifeq ($(WINDOWS),1)
	PROGRAM =  test.exe
	LIBS    += -lws2_32
	CFLAGS  += -std=gnu99 -pedantic -Wall
else
ifeq ($(MVS),1)
	CC      =  c99
	CFLAGS  += -DMVS -D_ALL_SOURCE
else
	CFLAGS  += -std=gnu99 -pedantic -Wall
endif
endif


all: lib $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS)
	mv a.out $(PROGRAM)

ifneq ($(MVS),1)
depend: .depend

.depend: $(SOURCES)
	rm -rf ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

-include .depend
endif

lib:
	$(MAKE) -C ../

check: $(PROGRAM)
	./$(PROGRAM)

check-valgrind: $(PROGRAM)
	valgrind -v --leak-check=full --show-leak-kinds=all ./$(PROGRAM)

check-coverage:
	$(MAKE) clean
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage -O0 $(CFLAGS)" check

clean:
	rm -f $(PROGRAM) $(OBJECTS)
	rm -f *.gcno *.gcda
	rm -f .depend
