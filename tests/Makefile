PROGRAM=test
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -I../contrib -I../contrib/mbedtls -I../
LFLAGS=
LIBS=../clax_*.o ../popen2.o ../contrib/*/*.o

include ../Makefile.vars

all: lib $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) $(LIBS)
	$(CP) $(EXE) $(PROGRAM)

ifneq ($(MVS),1)
depend: .depend

.depend: $(SOURCES)
	$(RMRF) ./.depend
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
	$(RMF) $(PROGRAM) $(OBJECTS)
	$(RMF) *.gcno *.gcda
	$(RMF) .depend
