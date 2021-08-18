#include <lua.h>
#include <lualib.h>
#ifndef LCFETCH_H
#define LCFETCH_H

/* lcfetch version */
#define VERSION "0.1.0-beta3"

/* copyright notice */
#define COPYRIGHT                                                                                                      \
    "Copyright (c) 2021 NTBBloodbath. lcfetch is distributed under GPLv2 license.\n\n"                                 \
    "This program is free software; you can redistribute it and/or "                                                   \
    "modify it under the terms of the GNU General Public License "                                                     \
    "as published by the Free Software Foundation; either version 2 "                                                  \
    "of the License, or (at your option) any later version.\n\n"                                                       \
    "This program is distributed in the hope that it will be useful, "                                                 \
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "                                                  \
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "                                                   \
    "GNU General Public License for more details.\n"

/* lcfetch.c */
#define BUF_SIZE 150
#define COUNT(x) (int)(sizeof x / sizeof *x)

/* memory.c */
void *xmalloc(size_t size);
void xfree(void *ptr);

/* cli.c */
void version(void);
void help(void);

/* utils.c */
char *repeat_string(char *str, int times);
void truncate_whitespaces(char *str);
char *remove_substr(char *str, const char *sub);

/* lua_api.c */
void start_lua(const char *config_file_path);
void stop_lua(void);
char *get_configuration_file_path(void);
int get_table_size(const char *table);
int table_contains_string(const char *table, const char *key);

// Get options
int get_option_boolean(const char *opt);
const char *get_option_string(const char *opt);
lua_Number get_option_number(const char *opt);
const char *get_subtable_string(const char *table, int index);

// Set options
int set_table_boolean(const char *key, int value);
int set_table_string(const char *key, const char *value);
int set_table_number(const char *key, lua_Number value);
int set_table_subtable(const char *key);
int set_subtable_string(const char *table, const char *key);
void init_options(void);

#endif
