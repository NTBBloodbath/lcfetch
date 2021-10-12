#include "../include/lcfetch.h"
#include "../third-party/log.c/src/log.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
/* ASCII logos */
#include "../include/logos/arch.h"
#include "../include/logos/android.h"
#include "../include/logos/debian.h"
#include "../include/logos/fedora.h"
#include "../include/logos/gentoo.h"
#include "../include/logos/linux.h"
#include "../include/logos/nixos.h"
#include "../include/logos/ubuntu.h"

size_t utf8len(char *s) {
    size_t len = 0;
    for (; *s; s++) {
        if ((*s & 0xC0) != 0x80) {
            len++;
        }
    }
    return len;
}

char *repeat_string(char *str, int times) {
    if (times < 1) {
        return str;
    }

    char *result = xmalloc(BUF_SIZE);
    char *repeated = result;

    for (int i = 0; i < times; i++) {
        *(repeated++) = *str;
    }
    *repeated = '\0';

    return result;
}

void truncate_whitespaces(char *str) {
    int src = 0;
    int dst = 0;
    while (*(str + dst) == ' ') {
        dst++;
    }

    while (*(str + dst) != '\0') {
        *(str + src) = *(str + dst);

        if (*(str + (dst++)) == ' ') {
            while (*(str + dst) == ' ') {
                dst++;
            }
        }
        src++;
    }

    *(str + src) = '\0';
}

char *remove_substr(char *str, const char *sub) {
    char *p, *q, *r;

    if (*sub && (q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            while (p < r) {
                *q++ = *p++;
            }
        }
        while ((*q++ = *p++) != '\0') {
            continue;
        }
    }
    return str;
}

char *replace_string(char *str, char *pattern, char *new_pattern) {
    char *new_str;
    int i = 0, count = 0;
    int pattern_len = strlen(pattern);
    int new_pattern_len = strlen(new_pattern);

    // Count the number of times old pattern occurs in the string
    for (i = 0; str[i] != '\0'; i++) {
        if (strstr(&str[i], pattern) == &str[i]) {
            count++;

            i += pattern_len - 1;
        }
    }
    new_str = xmalloc(i + count * (new_pattern_len - pattern_len) + 1);

    i = 0;
    while (*str) {
        // Compare the substrings with our result
        if (strstr(str, pattern) == str) {
            strcpy(&new_str[i], new_pattern);
            i += new_pattern_len;
            str += pattern_len;
        } else {
            new_str[i++] = *str++;
        }
    }

    new_str[i] = '\0';
    return new_str;
}

char *str_to_lower(char *str) {
    for (char *c = str; *c; c++) {
        *c = tolower(*c);
    }

    return str;
}

bool is_android_device() {
    DIR *sys_app = opendir("/system/app");
    DIR *sys_priv_app = opendir("/system/priv-app");
    if (sys_app && sys_priv_app) {
        closedir(sys_app);
        closedir(sys_priv_app);
        
        return true;
    }

    return false;
}

char **get_distro_logo(char *distro) {
    char **logo;
    if ((strcasecmp(distro, "fedora") == 0) || (strstr(distro, "Fedora"))) {
        logo = fedora;
    } else if (strcasecmp(distro, "gentoo") == 0) {
        logo = gentoo;
    } else if ((strcasecmp(distro, "arch") == 0) || (strstr(distro, "Arch"))) {
        logo = arch;
    } else if (strcasecmp(distro, "debian") == 0) {
        logo = debian;
    } else if ((strcasecmp(distro, "ubuntu") == 0) || (strstr(distro, "Ubuntu"))) {
        logo = ubuntu;
    } else if (strcasecmp(distro, "nixos") == 0) {
        logo = nixos;
    } else if (is_android_device()) {
        logo = android;
    } else {
        logo = linux_logo;
    }
    return logo;
}

int get_distro_logo_rows(char *distro) {
    int rows;
    if ((strcasecmp(distro, "fedora") == 0) || (strstr(distro, "Fedora"))) {
        rows = LEN(fedora);
    } else if (strcasecmp(distro, "gentoo") == 0) {
        rows = LEN(gentoo);
    } else if ((strcasecmp(distro, "arch") == 0) || (strstr(distro, "Arch"))) {
        rows = LEN(arch);
    } else if (strcasecmp(distro, "debian") == 0) {
        rows = LEN(debian);
    } else if ((strcasecmp(distro, "ubuntu") == 0) || (strstr(distro, "Ubuntu"))) {
        rows = LEN(ubuntu);
    } else if (strcasecmp(distro, "nixos") == 0) {
        rows = LEN(nixos);
    } else if (is_android_device()) {
        rows = LEN(android);
    } else {
        rows = LEN(linux_logo);
    }
    return rows;
}

char *get_distro_accent(char *distro) {
    char *accent_color = xmalloc(BUF_SIZE);
    if ((strcasecmp(distro, "fedora") == 0) || (strstr(distro, "Fedora"))) {
        strncpy(accent_color, fedora_accent, BUF_SIZE);
    } else if (strcasecmp(distro, "gentoo") == 0) {
        strncpy(accent_color, gentoo_accent, BUF_SIZE);
    } else if ((strcasecmp(distro, "arch") == 0) || (strstr(distro, "Arch"))) {
        strncpy(accent_color, arch_accent, BUF_SIZE);
    } else if (strcasecmp(distro, "debian") == 0) {
        strncpy(accent_color, debian_accent, BUF_SIZE);
    } else if ((strcasecmp(distro, "ubuntu") == 0) || (strstr(distro, "Ubuntu"))) {
        strncpy(accent_color, ubuntu_accent, BUF_SIZE);
    } else if (strcasecmp(distro, "nixos") == 0) {
        strncpy(accent_color, nixos_accent, BUF_SIZE);
    } else if (is_android_device()) {
        strncpy(accent_color, android_accent, BUF_SIZE);
    } else {
        strncpy(accent_color, linux_accent, BUF_SIZE);
    }
    return accent_color;
}

char *get_custom_accent(char *color) {
    /*
     * This wraps human-readable colors to terminal ANSI colors, where
     * black  = 0
     * red    = 1
     * green  = 2
     * yellow = 3
     * blue   = 4
     * purple = 5
     * cyan   = 6
     * white  = 7
     * */
    char *accent_color = xmalloc(BUF_SIZE);
    if (strcasecmp(color, "black") == 0) {
        strncpy(accent_color, "\e[1;30m", BUF_SIZE);
    } else if (strcasecmp(color, "red") == 0) {
        strncpy(accent_color, "\e[1;31m", BUF_SIZE);
    } else if (strcasecmp(color, "green") == 0) {
        strncpy(accent_color, "\e[1;32m", BUF_SIZE);
    } else if (strcasecmp(color, "yellow") == 0) {
        strncpy(accent_color, "\e[1;33m", BUF_SIZE);
    } else if (strcasecmp(color, "blue") == 0) {
        strncpy(accent_color, "\e[1;34m", BUF_SIZE);
    } else if (strcasecmp(color, "purple") == 0) {
        strncpy(accent_color, "\e[1;35m", BUF_SIZE);
    } else if (strcasecmp(color, "cyan") == 0) {
        strncpy(accent_color, "\e[1;36m", BUF_SIZE);
    } else if (strcasecmp(color, "white") == 0) {
        strncpy(accent_color, "\e[1;37m", BUF_SIZE);
    }
    return accent_color;
}

custom_ascii_logo get_custom_logo() {
    // NOTE: the current colors implementation is very tricky, should have a refactor later
    char *colors[] = {"black", "red", "green", "yellow", "blue", "purple", "cyan", "white"};
    bool has_accent = false;
    const char *custom_accent_color = get_option_string("accent_color");

    custom_ascii_logo logo;
    int custom_logo_size = get_table_size("custom_ascii_logo");
    if (custom_logo_size > 0) {
        logo.cols = custom_logo_size;
        logo.arr = (char **)xmalloc(BUF_SIZE * logo.cols);
        for (int i = 1; i <= custom_logo_size; i++) {
            char *logo_line = (char *)get_subtable_string("custom_ascii_logo", i);
            if (strlen(custom_accent_color) > 0) {
                char *accent_color = get_custom_accent((char *)custom_accent_color);
                char *logo_line_fmt = xmalloc(BUF_SIZE);
                snprintf(logo_line_fmt, BUF_SIZE, "%s%s", accent_color, logo_line);
                logo.arr[i - 1] = logo_line_fmt;
                xfree(accent_color);
                has_accent = true;
            } else {
                // Iterate over all possible colors and replace them
                for (int j = 0; j < LEN(colors); j++) {
                    char *accent_color = get_custom_accent(colors[j]);
                    char *logo_line_fmt = replace_string(logo_line, colors[j], accent_color);
                    logo.arr[i - 1] = logo_line_fmt;
                    unsigned long logo_fmt_len = strlen(logo_line_fmt);
                    xfree(accent_color);
                    if (strlen(logo_line) != logo_fmt_len) {
                        has_accent = true;
                        break;
                    }
                }
            }
        }
        logo.rows = utf8len(logo.arr[0]);
        logo.rows -= has_accent ? strlen("\e[0;00m") : 0;
        return logo;
    }

    logo.cols = logo.rows = 0;
    return logo;
}

void print_colors(char *logo_part, char *next_logo_part, char *gap_logo, char *gap_info) {
    char *dark_colors = get_colors_dark();
    char *bright_colors = get_colors_bright();
    // if we should add padding instead of the next distro logo line
    if ((strlen(logo_part) == 0) || (strlen(logo_part) - strlen("\e[1;00m") == 0)) {
        printf("%s%s%s\n", gap_logo, gap_info, dark_colors);
    } else {
        printf("%s%s%s\n", logo_part, gap_info, dark_colors);
    }
    if ((strlen(next_logo_part) == 0) || (strlen(next_logo_part) - strlen("\e[1;00m") == 0)) {
        printf("%s%s%s\n", gap_logo, gap_info, bright_colors);
    } else {
        printf("%s%s%s\n", next_logo_part, gap_info, bright_colors);
    }
    xfree(dark_colors);
    xfree(bright_colors);
}

void print_field(char *logo_part, char *gap, const char *delimiter, char *accent, const char *field_name) {
    // NOTE: colors field requires a special treatment so we don't use print_info on it

    // User information requires a special treatment
    bool is_user_title = false;
    bool is_separator = false;

    char *message = xmalloc(BUF_SIZE);
    // Set the function that will be used for getting the field value
    char *field_function = NULL;
    char *field_message = xmalloc(BUF_SIZE);

    // Get the fields data
    if (strcasecmp(field_name, "os") == 0) {
        field_function = get_os(1);
    } else if (strcasecmp(field_name, "kernel") == 0) {
        field_function = get_kernel();
    } else if (strcasecmp(field_name, "uptime") == 0) {
        field_function = get_uptime();
    } else if (strcasecmp(field_name, "packages") == 0) {
        field_function = get_packages();
    } else if (strcasecmp(field_name, "wm") == 0) {
        field_function = get_wm();
    } else if (strcasecmp(field_name, "resolution") == 0) {
        field_function = get_resolution();
    } else if (strcasecmp(field_name, "shell") == 0) {
        field_function = get_shell();
    } else if (strcasecmp(field_name, "terminal") == 0) {
        field_function = get_terminal();
    } else if (strcasecmp(field_name, "cpu") == 0) {
        field_function = get_cpu();
    } else if (strcasecmp(field_name, "memory") == 0) {
        field_function = get_memory();
    } else if (strcasecmp(field_name, "user") == 0) {
        field_function = get_title(accent);
        is_user_title = true;
    } else if (strcasecmp(field_name, "separator") == 0) {
        field_function = get_separator();
        is_separator = true;
    } else {
        log_error("Field '%s' doesn't exists", field_name);
        xfree(field_message);
        xfree(message);
        exit(1);
    }
    snprintf(field_message, BUF_SIZE, "%s_message", str_to_lower((char *)field_name));
    snprintf(field_message, BUF_SIZE, "%s", get_option_string(field_message));
    if (is_user_title || is_separator) {
        snprintf(message, BUF_SIZE, "%s%s", "\e[0m", field_function);
    } else {
        snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, field_function);
    }
    xfree(field_message);

    if ((strcasecmp(field_name, "kernel") != 0) && (field_function != NULL)) {
        xfree(field_function);
    }

    // Print field information
    if (logo_part == NULL) {
        // When using minimal mode (without displaying logo)
        if (is_user_title) {
            printf("%s%s%s", gap, accent, message);
        } else {
            if (field_function != NULL) {
                printf("%s%s%s\n", gap, accent, message);
            } else {
                printf("%s%s\n", gap, accent);
            }
        }
    } else {
        if (is_user_title) {
            printf("%s%s%s%s", logo_part, gap, accent, message);
        } else {
            if (field_function != NULL) {
                printf("%s%s%s%s\n", logo_part, gap, accent, message);
            } else {
                printf("%s%s%s\n", logo_part, gap, accent);
            }
        }
    }
    xfree(message);
}

char *get_property(Display *disp, Window win, Atom xa_prop_type, char *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;

    xa_prop_name = XInternAtom(disp, prop_name, 0);

    if (XGetWindowProperty(disp, win, xa_prop_name, 0, BUF_SIZE, 0, xa_prop_type, &xa_ret_type, &ret_format,
                           &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        log_warn("Cannot get %s property.\n", prop_name);
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        log_warn("Invalid type of %s property.\n", prop_name);
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / (64 / sizeof(long))) * ret_nitems;
    char *ret = xmalloc(tmp_size + 1);
    memmove(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}
