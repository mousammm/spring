BINARY=spring_wm
BINDIR=bin
SRCDIR=src

CC=gcc
INCS = -I./src/includes
LIBS = -lutil -lX11 

CFLAGS=-Wall -Wextra -g $(LIBS)
LDLIBS = $(LIBS)

SRCS=$(wildcard $(SRCDIR)/*.c)

OBJS=$(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(SRCS))

all: $(BINDIR)/$(BINARY)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/$(BINARY): $(OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	@echo "Compiling $< -> $@"
	$(CC) $(CFLAGS) -c -o $@ $< 

run: $(BINDIR)/$(BINARY)
	@echo "Starting Xephyr..."
	@Xephyr :1 -screen 960x540 -ac &
	@sleep 1
	@echo "Running WM..."
	@DISPLAY=:1 ./$(BINDIR)/$(BINARY) || true
	@echo "Cleaning up..."
	@pkill Xephyr 2>/dev/null || true
	

clean:
	rm -rf $(BINDIR)

.PHONY: all run clean
