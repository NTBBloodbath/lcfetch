PREFIX=$(HOME)/.local
CONFIG_DIR=$(HOME)/.config/lcfetch

TP_DIR=$(CURDIR)/third-party
BIN_DIR=$(CURDIR)/bin
LIB_DIR=$(CURDIR)/lib
INC_DIR=$(CURDIR)/include
LUA_INC_DIR=/usr/include/lua

CC=clang
CCF=clang-format
CFLAGS=-I$(LUA_INC_DIR) -O2 -Wall -Wextra -llua -lX11

PANDOC=pandoc

LOG_INFO=$(shell date +"%H:%M:%S") \e[1;32mINFO\e[0m

.PHONY: clean fmt build run docs
.SILENT: clean run
.DEFAULT_GOAL := all

all: build


build: clean lcfetch.c $(LIB_DIR)/lua_api.c $(LIB_DIR)/cli.c $(LIB_DIR)/memory.c $(INC_DIR)/lcfetch.h
	@echo -e "$(LOG_INFO) Cloning third-party dependencies ..."
	git clone --depth 1 git@github.com:rxi/log.c.git $(TP_DIR)/log.c
	@echo -e "\n$(LOG_INFO) Building lcfetch.c ..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/lcfetch lcfetch.c $(LIB_DIR)/*.c $(TP_DIR)/log.c/src/log.c -DLOG_USE_COLOR
	strip $(BIN_DIR)/lcfetch


install: build
	@echo -e "\n$(LOG_INFO) Installing lcfetch under $(PREFIX)/bin directory ..."
	mkdir -p $(PREFIX)/bin $(PREFIX)/share/man/man1 $(CONFIG_DIR)
	install $(BIN_DIR)/lcfetch $(PREFIX)/bin/lcfetch
	cp $(CURDIR)/config/sample.lua $(CONFIG_DIR)/config.lua
	cp $(CURDIR)/man/lcfetch.1 $(PREFIX)/share/man/man1/lcfetch.1
	mandb -pu


clean:
ifneq (,$(wildcard $(BIN_DIR)/lcfetch))
	rm $(BIN_DIR)/lcfetch
endif


fmt:
	@echo -e "$(LOG_INFO) Formatting source code ..."
	$(CCF) -style=file --sort-includes -i lcfetch.c $(INC_DIR)/*.h $(LIB_DIR)/*.c


docs:
	@echo -e "$(LOG_INFO) Generating man pages ..."
	$(PANDOC) $(CURDIR)/man/lcfetch.1.md -s -t man -o $(CURDIR)/man/lcfetch.1


run: build
	echo -e "\n$(LOG_INFO) Running lcfetch ..."
	$(BIN_DIR)/lcfetch
