# Makefile for the ls(1) re-implementation.

CC       ?= cc
CFLAGS   ?= -O2
CFLAGS   += -Wall -Wextra -pedantic -std=c99
CPPFLAGS += -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS  +=

# MinGW appends .exe to the linked binary even for a bare target name.
ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TARGET := ls$(EXE)
OBJDIR := build
SRCS   := $(wildcard src/*.c)
OBJS   := $(SRCS:src/%.c=$(OBJDIR)/%.o)
DEPS   := $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

-include $(DEPS)
