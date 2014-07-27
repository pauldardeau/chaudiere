# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = clang
CC_OPTS = -c
ARCHIVE_CMD = ar
ARCHIVE_OPTS = rs

LIB_NAME = libz.a

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

all : $(LIB_NAME)

clean :
	rm -f *.o
	rm -f $(LIB_NAME)

$(LIB_NAME) : $(OBJS)
	$(ARCHIVE_CMD) $(ARCHIVE_OPTS) $(LIB_NAME) $(OBJS)

%.o : %.c
	$(CC) $(CC_OPTS) $< -o $@
