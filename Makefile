LUA=lua
USE_SYSTEM_LUA=0
SHELL:=/bin/bash

PREFIX=$(HOME)/.local
CONFIG_DIR=$(HOME)/.config/lcfetch

TP_DIR=$(CURDIR)/third-party
BIN_DIR=$(CURDIR)/bin
LIB_DIR=$(CURDIR)/lib
INC_DIR=$(CURDIR)/include
LUA_INC_DIR=/usr/include/$(LUA)

CC=clang
CCF=clang-format
CFLAGS=-O2 -Wall -Wextra
LDFLAGS=$(shell pkg-config --libs $(LUA)) -lX11
ifeq (1,$(USE_SYSTEM_LUA))
	CFLAGS+=-I $(LUA_INC_DIR) -DUSE_SYSTEM_LUA
else
	CFLAGS+=-I $(TP_DIR)/lua-5.3.6/src
endif

PANDOC=pandoc

LOG_INFO=$(shell date +"%H:%M:%S") \e[1;32mINFO\e[0m

.PHONY: clean fmt build run docs
.SILENT: clean run
.DEFAULT_GOAL := all

all: build


_clone_deps:
	@echo -e "$(LOG_INFO) Downloading third-party dependencies if needed ...\n"
	@if [ ! -d "$(TP_DIR)/log.c" ]; then \
		echo -e "$(LOG_INFO) Cloning log.c ..."; \
		git clone --depth 1 https://github.com/rxi/log.c.git $(TP_DIR)/log.c; \
		echo ""; \
	fi
	@if [ ! -d "$(TP_DIR)/lua-5.3.6" ] && [ $(USE_SYSTEM_LUA) -eq 0 ]; then \
		echo -e "$(LOG_INFO) Downloading lua-5.3.6.tar.gz ..."; \
		pushd $(TP_DIR); \
		curl -R -O http://www.lua.org/ftp/lua-5.3.6.tar.gz; \
		tar xf lua-5.3.6.tar.gz && rm lua-5.3.6.tar.gz; \
		popd; \
		pushd $(TP_DIR)/lua-5.3.6; \
		make linux; \
		popd; \
		echo ""; \
	fi


build: clean _clone_deps lcfetch.c $(LIB_DIR)/lua_api.c $(LIB_DIR)/cli.c $(LIB_DIR)/memory.c $(LIB_DIR)/utils.c $(INC_DIR)/lcfetch.h
	@echo -e "$(LOG_INFO) Building lcfetch.c ..."
	$(CC) lcfetch.c $(LIB_DIR)/*.c $(TP_DIR)/log.c/src/log.c $(CFLAGS) -L $(TP_DIR)/lua-5.3.6/src $(LDFLAGS) -o $(BIN_DIR)/lcfetch -Wl,-E -DLOG_USE_COLOR
	strip $(BIN_DIR)/lcfetch


install: build
	@echo -e "\n$(LOG_INFO) Installing lcfetch under $(PREFIX)/bin directory ..."
	mkdir -p $(PREFIX)/bin $(PREFIX)/share/man/man1 $(CONFIG_DIR)
	install $(BIN_DIR)/lcfetch $(PREFIX)/bin/lcfetch
	cp $(CURDIR)/config/sample.lua $(CONFIG_DIR)/config.lua
	cp $(CURDIR)/man/lcfetch.1 $(PREFIX)/share/man/man1/lcfetch.1
	mandb -pu


uninstall:
	@echo -e "$(LOG_INFO) Uninstalling lcfetch ..."
	@if [ -f "$(PREFIX)/bin/lcfetch" ]; then \
		rm "$(PREFIX)/bin/lcfetch"; \
	fi
	@echo -e "$(LOG_INFO) Removing lcfetch man pages ..."
	@if [ -f "$(PREFIX)/share/man/man1/lcfetch.1" ]; then \
		rm "$(PREFIX)/share/man/man1/lcfetch.1"; \
		mandb -pu; \
	fi


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
