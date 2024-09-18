DEBUG=-g
RELEASE=-O3
ifeq ($(MODE),RELEASE)
    MODE=$(RELEASE)
else
    MODE=$(DEBUG)
endif
MODE=$(DEBUG)

CFLAGS=$(MODE) -Iinclude -std=gnu99 -Wno-enum-conversion -Wno-format -Werror
LDFLAGS=-L. -lsublagert
UNAME=$(shell uname)
ifeq ($(UNAME),Linux)
    CFLAGS+=-fPIC
    LDFLAGS+=-ldl -lpthread
    RT=libsublagert.so
endif
ifeq ($(UNAME),FreeBSD)
    CFLAGS+=-fPIC
    LDFLAGS+=-lpthread
    RT=libsublagert.so 
endif
ifeq ($(UNAME),Darwin)
    CFLAGS+=-fPIC
    LDFLAGS+=-ldl
    RT=libsublagert.dylib
endif
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
    LDFLAGS+=-lws2_32 -mconsole
    RT=sublagert.dll
    ifeq ($(findstring WOW64,$(UNAME)),WOW64)
        CC=x86_64-w64-mingw32-gcc
    else
        CC=i686-pc-mingw32-gcc
    endif
endif

SRC=src
SRC_COMMON=$(wildcard $(SRC)/common/*.c)
SRC_RUNTIME=$(wildcard $(SRC)/runtime/*.c)
SRC_ASSEMBLER=$(wildcard $(SRC)/assembler/*.c)
SRC_INTERNALS=$(wildcard $(SRC)/runtime/internals/*.c)

OBJ_COMMON=$(SRC_COMMON:.c=.o)
OBJ_RUNTIME=$(SRC_RUNTIME:.c=.o)
OBJ_ASSEMBLER=$(SRC_ASSEMBLER:.c=.o)
OBJ_INTERNALS=$(SRC_INTERNALS:.c=.o)

all: sublagec sublaged sublage binarydump debugdump

debugdump: $(RT) $(SRC)/debugdump.c
	$(CC) -o $@ $(SRC)/debugdump.c $(CFLAGS) $(LDFLAGS)

binarydump: $(RT) $(SRC)/binarydump.c
	$(CC) -o $@ $(SRC)/binarydump.c $(OBJ_ASSEMBLER) $(CFLAGS) $(LDFLAGS)

sublagec: $(RT) $(OBJ_ASSEMBLER) $(SRC)/sublagec.c
	$(CC) -o $@ $(SRC)/sublagec.c $(OBJ_ASSEMBLER) $(CFLAGS) $(LDFLAGS)

sublaged: $(RT) $(SRC)/sublaged.c
	$(CC) -o $@ $(SRC)/sublaged.c $(CFLAGS) $(LDFLAGS)

sublage: $(RT) $(SRC)/sublage.c
	$(CC) -o $@ $(SRC)/sublage.c $(CFLAGS) $(LDFLAGS)

libsublagert.so: $(OBJ_RUNTIME) $(OBJ_INTERNALS) $(OBJ_COMMON)
	$(CC) -shared -o $@ $(OBJ_RUNTIME) $(OBJ_INTERNALS) $(OBJ_COMMON)

libsublagert.dylib: $(OBJ_RUNTIME) $(OBJ_INTERNALS) $(OBJ_COMMON)
	$(CC) -dynamiclib -o $@ $(OBJ_RUNTIME) $(OBJ_INTERNALS) $(OBJ_COMMON)

sublagert.dll: $(OBJ_RUNTIME) $(OBJ_INTERNALS) $(OBJ_COMMON)
	$(CC) -shared -o sublagert.dll -Wl,--out-implib,libsublagert.a -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc $(OBJ_RUNTIME) $(OBJ_INTERNALS) $(OBJ_COMMON)

clean:
	$(RM) -f sublagec sublaged sublage binarydump debugdump \
	    libsublagert.a $(RT) $(OBJ_COMMON) $(OBJ_RUNTIME) \
	    $(OBJ_ASSEMBLER) $(OBJ_INTERNALS)
ifeq ($(UNAME),Darwin)
	$(RM) -rf *.dSYM
endif
	$(MAKE) -C tests clean

test:
	$(MAKE) -C tests
	
*.o: *.c
	$(CC) -o $@ -c $< $(CFLAGS)
