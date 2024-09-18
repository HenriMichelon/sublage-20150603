DEBUG=-g
RELEASE=-O3
ifeq ($(MODE),RELEASE)
    MODE=$(RELEASE)
else
    MODE=$(DEBUG)
endif

SUBLAGE_SRC=../../sublage
BINARIES=../binaries
CFLAGS=$(MODE) -I$(SUBLAGE_SRC)/include -std=gnu99 -Wno-enum-conversion -Wno-format -Werror
LDFLAGS=-shared -L$(SUBLAGE_SRC) -L$(BINARIES) -lsublagert
UNAME=$(shell uname)
ifeq ($(UNAME), FreeBSD)
    CFLAGS+=-fPIC
    LDFLAGS+=-export-dynamic -pthread
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../sublage
endif
ifeq ($(UNAME), Linux)
    CFLAGS+=-fPIC
    LDFLAGS+=-lpthread
endif
ifeq ($(UNAME), Darwin)	
    CFLAGS+=-fPIC
    LDFLAGS+=-lpthread
endif
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
    LDFLAGS+=-lws2_32
	ifeq ($(findstring WOW64,$(UNAME)),WOW64)
		CC=x86_64-w64-mingw32-gcc
	else
		CC=i686-pc-mingw32-gcc
	endif
endif

all: $(BINARIES) \
    $(BINARIES)/console.library.native \
    $(BINARIES)/array.library.native \
    $(BINARIES)/thread.library.native \
    $(BINARIES)/math.library.native \
    $(BINARIES)/mutex.library.native \
    $(BINARIES)/socket.library.native \
    $(BINARIES)/io.library.native \
    $(BINARIES)/string.library.native

$(BINARIES):
	mkdir $(BINARIES)

$(BINARIES)/string.library.native: string.library.o
	$(CC) -o $@ string.library.c $(CFLAGS) $(LDFLAGS)

$(BINARIES)/io.library.native: io.library.o utils.o
	$(CC) -o $@ io.library.c utils.o $(CFLAGS) $(LDFLAGS)

$(BINARIES)/socket.library.native: socket.library.o
	$(CC) -o $@ socket.library.c $(CFLAGS) $(LDFLAGS)

$(BINARIES)/math.library.native: math.library.o
	$(CC) -o $@ math.library.c $(CFLAGS) $(LDFLAGS)

$(BINARIES)/array.library.native: array.library.o
	$(CC) -o $@ array.library.c $(CFLAGS) $(LDFLAGS)

$(BINARIES)/console.library.native: console.library.o utils.o
	$(CC) -o $@ console.library.c utils.o $(CFLAGS) $(LDFLAGS)

$(BINARIES)/thread.library.native: thread.library.o
	$(CC) -o $@ thread.library.c $(CFLAGS) $(LDFLAGS)

$(BINARIES)/mutex.library.native: mutex.library.o
	$(CC) -o $@ mutex.library.c $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) -f *.o $(BINARIES)/*.library.native

*.o: *.c
	$(CC) -o $@ -c $< $(CFLAGS)
