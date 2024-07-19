CC = gcc

CFLAGS = $(shell pkg-config -cflags gtk4)

TARGET = calc

SRCS = calc.c

OBJS = $(SRCS:.c=.o)

INCDIR = -I../inc

LIBSDIR  = $(shell pkg-config -libs gtk4)

LIBS =

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBSDIR) $(LIBS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(INCDIR) -c $(SRCS)

all: clean
	$(OBJS) $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.d