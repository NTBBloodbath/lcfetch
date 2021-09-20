/* C stdlib */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Lua headers */
#ifdef USE_SYSTEM_LUA
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#else
#include "../third-party/lua-5.3.6/src/lauxlib.h"
#include "../third-party/lua-5.3.6/src/lua.h"
#include "../third-party/lua-5.3.6/src/lualib.h"
#endif
/* Custom headers */
#include "../include/lcfetch.h"

// Lua interpreter state
static lua_State *lua;

/**
 * Get the lcfetch configuration file path
 */
char *get_configuration_file_path() {
    char *config_directory = getenv("XDG_CONFIG_HOME");
    if (!config_directory) {
        config_directory = strncat(getenv("HOME"), "/.config", -1);
    }
    char *config_file_path = strncat(config_directory, "/lcfetch/config.lua", -1);

    return config_file_path;
}

/**
 * Print Lua API stack to stdout, for debugging purposes
 * NOTE: remove the '__attribute__((unused))' statement when using it
 */
static void __attribute__((unused)) dumpstack(lua_State *L) {
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) {
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i)) {
        case LUA_TNUMBER:
            printf("%g\n", lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            printf("%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        default:
            printf("%p\n", lua_topointer(L, i));
            break;
        }
    }
}

/**
 * Start the Lua interpreter and load the Lua interface functions
 */
void start_lua(const char *config_file_path) {
    char *config_file = (char *)config_file_path;

    // Get the default configuration path for lcfetch,
    // e.g. /home/user/.config/lcfetch/config.lua
    if (config_file_path == NULL) {
        config_file = get_configuration_file_path();
    }

    // Create a pointer to an empty Lua environment
    lua = luaL_newstate();
    // Load the Lua libraries to make the Lua environment usable
    luaL_openlibs(lua);
    // Set the stack top to a specific value (0)
    lua_settop(lua, 0);
    // Load the default configurations
    init_options();
    // Load the user configurations file
    luaL_loadfile(lua, config_file) || lua_pcall(lua, 0, 0, 0);
}

/**
 * Stop the Lua interpreter
 */
void stop_lua(void) { lua_close(lua); }

/**
 * Set default values to lcfetch configurations
 */
void init_options(void) {
    // Create a new empty table
    lua_newtable(lua);

    // Set the default basic types options (strings, numbers, booleans)
    set_table_string("accent_color", "");
    set_table_string("ascii_distro", "");
    set_table_string("colors_icon", "");
    set_table_string("colors_style", "classic");
    set_table_string("delimiter", ":");
    set_table_string("separator", "-");
    set_table_boolean("show_arch", 1);
    set_table_boolean("display_refresh_rate", 0);
    set_table_boolean("short_cpu_info", 1);
    set_table_boolean("memory_in_gib", 1);
    set_table_boolean("display_logo", 1);
    set_table_number("gap", 3);

    // Fields messages
    set_table_string("os_message", "OS");
    set_table_string("kernel_message", "Kernel");
    set_table_string("uptime_message", "Uptime");
    set_table_string("packages_message", "Packages");
    set_table_string("resolution_message", "Resolution");
    set_table_string("wm_message", "WM");
    set_table_string("shell_message", "Shell");
    set_table_string("terminal_message", "Terminal");
    set_table_string("cpu_message", "CPU");
    set_table_string("memory_message", "Memory");

    // Set the global "options" table
    lua_setglobal(lua, "options");

    // Create a new subtable in the "options" table
    set_table_subtable("enabled_fields");
    // Assign "options.enabled_fields" values
    set_subtable_string("enabled_fields", "User");
    set_subtable_string("enabled_fields", "Separator");
    set_subtable_string("enabled_fields", "OS");
    set_subtable_string("enabled_fields", "Kernel");
    set_subtable_string("enabled_fields", "Uptime");
    set_subtable_string("enabled_fields", "Packages");
    set_subtable_string("enabled_fields", ""); // Newline
    set_subtable_string("enabled_fields", "WM");
    set_subtable_string("enabled_fields", "Resolution");
    set_subtable_string("enabled_fields", ""); // Newline
    set_subtable_string("enabled_fields", "Shell");
    set_subtable_string("enabled_fields", "Terminal");
    set_subtable_string("enabled_fields", ""); // Newline
    set_subtable_string("enabled_fields", "CPU");
    set_subtable_string("enabled_fields", "Memory");
    set_subtable_string("enabled_fields", ""); // Newline
    set_subtable_string("enabled_fields", "Colors");
}

/**
 * Get a table size
 */
int get_table_size(const char *table) {
    int table_length = 0;

    lua_getglobal(lua, "options");
    if (luaL_getsubtable(lua, -1, table)) {
        // Push the table length
        lua_len(lua, -1);
        table_length = luaL_checknumber(lua, -1);
        // Remove the table length from the stack
        lua_pop(lua, 1);
    }

    return table_length;
}

/**
 * Check if a given string is in the given table
 */
bool table_contains_string(const char *table, const char *key) {
    const char *value = NULL;

    lua_getglobal(lua, "options");
    if (luaL_getsubtable(lua, -1, table)) {
        int table_length = get_table_size(table);
        // Iterate over all the table elements
        for (int i = 1; i <= table_length; i++) {
            // Get and store the current index value in the table
            lua_rawgeti(lua, -1, i);
            value = luaL_checkstring(lua, -1);
            // Remove the value from the stack
            lua_pop(lua, 1);
            // If the wanted value is in the table (case-insensitive)
            // then let's return 1 and break the bucle
            if (strcasecmp(value, key) == 0) {
                return true;
            }
        }
    }
    lua_pop(lua, 2);

    return false;
}

/**
 * Get a boolean option from the configuration file
 */
bool get_option_boolean(const char *opt) {
    bool bool_opt;

    lua_getglobal(lua, "options");
    lua_getfield(lua, -1, opt);
    bool_opt = lua_toboolean(lua, -1);
    lua_pop(lua, -1);

    return bool_opt;
}

/**
 * Get a string option from the configuration file
 */
const char *get_option_string(const char *opt) {
    const char *str = NULL;

    lua_getglobal(lua, "options");
    lua_getfield(lua, -1, opt);
    str = lua_tostring(lua, -1);
    lua_pop(lua, -1);

    return str;
}

/**
 * Get a number option from the configuration file
 */
lua_Number get_option_number(const char *opt) {
    lua_Number number;

    lua_getglobal(lua, "options");
    lua_getfield(lua, -1, opt);
    number = lua_tonumber(lua, -1);
    lua_pop(lua, -1);

    return number;
}

/**
 * Get a table value from a subtable in the configuration file
 */
const char *get_subtable_string(const char *table, int index) {
    const char *value = NULL;

    lua_getglobal(lua, "options");
    if (luaL_getsubtable(lua, -1, table)) {
        int table_length = get_table_size(table);
        // If the wanted index is higher than the table length then return NULL
        if (table_length < index) {
            lua_pop(lua, 1);
            return value;
        }
        lua_rawgeti(lua, -1, index);
        value = luaL_checkstring(lua, -1);
        lua_pop(lua, 1);
    }
    lua_pop(lua, 2);

    return value;
}

/**
 * Set a boolean value in a table element
 */
int set_table_boolean(const char *key, bool value) {
    lua_pushstring(lua, key);
    lua_pushboolean(lua, value);
    lua_settable(lua, -3);

    return 0;
}

/**
 * Set a string value in a table element
 */
int set_table_string(const char *key, const char *value) {
    lua_pushstring(lua, key);
    lua_pushstring(lua, value);
    lua_settable(lua, -3);

    return 0;
}

/**
 * Set a number value in a table element
 */
int set_table_number(const char *key, lua_Number value) {
    lua_pushstring(lua, key);
    lua_pushnumber(lua, value);
    lua_settable(lua, -3);

    return 0;
}

/**
 * Set a subtable in a table
 */
int set_table_subtable(const char *key) {
    lua_getglobal(lua, "options");
    lua_setfield(lua, -1, key);

    return 0;
}

/**
 * Set a subtable string value in a table
 */
int set_subtable_string(const char *table, const char *key) {
    lua_getglobal(lua, "options");
    if (luaL_getsubtable(lua, -1, table)) {
        int table_length = get_table_size(table);
        // Push the new value
        lua_pushstring(lua, key);
        lua_rawseti(lua, -2, table_length + 1);
        lua_pop(lua, 2);
    }
    lua_pop(lua, 1);

    return 0;
}
