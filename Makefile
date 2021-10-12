LUA=lua
USE_SYSTEM_LUA=0
SHELL:=/usr/bin/env bash

PREFIX=$(HOME)/.local
CONFIG_DIR=$(HOME)/.config/lcfetch

TP_DIR=$(CURDIR)/third-party
BIN_DIR=$(CURDIR)/bin
LIB_DIR=$(CURDIR)/lib
INC_DIR=$(CURDIR)/include
LUA_INC_DIR=/usr/include/$(LUA)

CC=clang
CCF=clang-format
CFLAGS=-Ofast -Wall -Wextra
LDFLAGS=$(shell pkg-config --libs $(LUA) || echo "-llua -lm -ldl") -lX11 -lXrandr
ifeq (1,$(USE_SYSTEM_LUA))
	CFLAGS+=-I $(LUA_INC_DIR) -DUSE_SYSTEM_LUA
else
	CFLAGS+=-I $(TP_DIR)/lua-5.3.6/src -L $(TP_DIR)/lua-5.3.6/src 
endif

PANDOC=pandoc

LOG_INFO=$(shell date +"%H:%M:%S") \e[1;32mINFO\e[0m

.PHONY: _echo_info clean fmt build run install docs
.SILENT: _echo_info clean run
.DEFAULT_GOAL := all

all: build


_echo_info:
	echo -e "$(LOG_INFO) Displaying build configuration ..."
	echo -e "\e[1mLUA\e[0m=$(LUA)"
	echo -e "\e[1mUSE_SYSTEM_LUA\e[0m=$(USE_SYSTEM_LUA)"
	echo -e "\e[1mSHELL\e[0m=$(SHELL)"
	echo -e "\e[1mPREFIX\e[0m=$(PREFIX)"
	echo -e "\e[1mLUA_INC_DIR\e[0m=$(LUA_INC_DIR)"
	echo -e "\e[1mCC\e[0m=$(CC)"
	echo -e "\e[1mCFLAGS\e[0m=$(CFLAGS)"
	echo -e "\e[1mLDFLAGS\e[0m=$(LDFLAGS)\n"


_clone_deps: _echo_info
	@if [[ -z "$(shell ls $(TP_DIR))" || "$(shell ls $(TP_DIR) | wc -l)" -lt 2 ]]; then \
		echo -e "$(LOG_INFO) Downloading third-party dependencies ..."; \
	fi
	@if [[ ! -d "$(TP_DIR)/log.c" ]]; then \
		echo -e "$(LOG_INFO) Cloning log.c ..."; \
		git clone --depth 1 https://github.com/rxi/log.c.git $(TP_DIR)/log.c; \
		echo ""; \
	fi
	@if [[ ! -d "$(TP_DIR)/lua-5.3.6" && $(USE_SYSTEM_LUA) -eq 0 ]]; then \
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


build: _clone_deps lcfetch.c $(LIB_DIR)/lua_api.c $(LIB_DIR)/cli.c $(LIB_DIR)/memory.c $(LIB_DIR)/utils.c $(INC_DIR)/lcfetch.h
	@echo -e "$(LOG_INFO) Building lcfetch.c ..."
	$(CC) lcfetch.c $(LIB_DIR)/*.c $(TP_DIR)/log.c/src/log.c $(CFLAGS) $(LDFLAGS) -o $(BIN_DIR)/lcfetch -Wl,-E -DLOG_USE_COLOR
	strip --strip-unneeded $(BIN_DIR)/lcfetch


dev: _clone_deps lcfetch.c $(LIB_DIR)/lua_api.c $(LIB_DIR)/cli.c $(LIB_DIR)/memory.c $(LIB_DIR)/utils.c $(INC_DIR)/lcfetch.h
	@echo -e "$(LOG_INFO) Building lcfetch.c for debugging purposes ..."
	$(CC) lcfetch.c $(LIB_DIR)/*.c $(TP_DIR)/log.c/src/log.c $(CFLAGS) -g3 $(LDFLAGS) -o $(BIN_DIR)/lcfetch -Wl,-E -DLOG_USE_COLOR


valgrind: dev
	@echo -e "$(LOG_INFO) Running valgrind on lcfetch binary ..."
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all --show-error-list=yes $(BIN_DIR)/lcfetch


install: build
	@echo -e "\n$(LOG_INFO) Installing lcfetch under $(PREFIX)/bin directory ..."
	mkdir -p $(PREFIX)/bin $(PREFIX)/share/man/man1 $(CONFIG_DIR)
	install $(BIN_DIR)/lcfetch $(PREFIX)/bin/lcfetch
	cp $(CURDIR)/config/sample.lua $(CONFIG_DIR)/config.lua
	cp $(CURDIR)/man/lcfetch.1 $(PREFIX)/share/man/man1/lcfetch.1
	mandb -pu


uninstall:
	@echo -e "$(LOG_INFO) Uninstalling lcfetch ..."
	@if [[ -f "$(PREFIX)/bin/lcfetch" ]]; then \
		rm "$(PREFIX)/bin/lcfetch"; \
	fi
	@echo -e "$(LOG_INFO) Removing lcfetch man pages ..."
	@if [[ -f "$(PREFIX)/share/man/man1/lcfetch.1" ]]; then \
		rm "$(PREFIX)/share/man/man1/lcfetch.1"; \
		mandb -pu; \
	fi


clean:
	if [[ -f "$(BIN_DIR)/lcfetch" ]]; then \
		rm "$(BIN_DIR)/lcfetch"; \
	fi
	if [[ -d "$(TP_DIR)/log.c" ]]; then \
		rm -rf "$(TP_DIR)/log.c"; \
	fi
	if [[ -d "$(TP_DIR)/lua-5.3.6" ]]; then \
		rm -rf "$(TP_DIR)/lua-5.3.6"; \
	fi


fmt:
	@echo -e "$(LOG_INFO) Formatting source code ..."
	$(CCF) -style=file --sort-includes -i lcfetch.c $(INC_DIR)/*.h $(LIB_DIR)/*.c


docs:
	@echo -e "$(LOG_INFO) Generating man pages ..."
	$(PANDOC) $(CURDIR)/man/lcfetch.1.md -s -t man -o $(CURDIR)/man/lcfetch.1


run: build
	echo -e "\n$(LOG_INFO) Running lcfetch ..."
	$(BIN_DIR)/lcfetch
