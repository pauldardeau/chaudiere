# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = clang
CC_OPTS = -c

OBJS = adler32.o \
compress.o \
crc32.o \
deflate.o \
gzlib.o \
gzwrite.o \
inffast.o \
inflate.o \
inftrees.o \
trees.o \
zutil.o

all : $(OBJS)

clean :
	rm -f *.o

%.o : %.c
	$(CC) $(CC_OPTS) $< -o $@
