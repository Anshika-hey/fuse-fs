CC      = gcc
CFLAGS  = -Wall -Wextra -g -I/opt/homebrew/include -D_FILE_OFFSET_BITS=64
LIBS    = -L/opt/homebrew/lib -lfuse
TARGET  = fusefs
SRCDIR  = src
SRCS    = $(SRCDIR)/main.c \
          $(SRCDIR)/inode.c \
          $(SRCDIR)/dir.c \
          $(SRCDIR)/fuse_ops.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "Build successful!"

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)