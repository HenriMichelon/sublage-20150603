CFLAGS=-std=c99 -fPIC -g -I../include
LDFLAGS=-L.. -lsublagert -export-dynamic
UNIT_TESTS=./tests

all: tests
	$(UNIT_TESTS)

tests: tests.o
	$(CC) -o $@ tests.o $(LDFLAGS)

clean:
	rm -f tests.o tests

*.o: *.c
	$(CC) -o $@ -c $< $(CFLAGS)
