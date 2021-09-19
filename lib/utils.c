#include "../include/lcfetch.h"
#include "../third-party/log.c/src/log.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
/* ASCII logos */
#include "../include/logos/arch.h"
#include "../include/logos/debian.h"
#include "../include/logos/fedora.h"
#include "../include/logos/gentoo.h"
#include "../include/logos/linux.h"
#include "../include/logos/nixos.h"
#include "../include/logos/ubuntu.h"

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

char *str_to_lower(char *str) {
    for (char *c = str; *c; c++) {
        *c = tolower(*c);
    }

    return str;
}

char **get_distro_logo(char *distro) {
    char **logo;
    if (strstr(distro, "fedora") == 0) {
        logo = fedora;
    } else if (strcasecmp(distro, "gentoo") == 0) {
        logo = gentoo;
    } else if (strstr(distro, "Arch")) {
        logo = arch;
    } else if (strcasecmp(distro, "debian") == 0) {
        logo = debian;
    } else if (strstr(distro, "ubuntu") == 0) {
        logo = ubuntu;
    } else if (strcasecmp(distro, "nixos") == 0) {
        logo = nixos;
    } else {
        logo = linux_logo;
    }
    return logo;
}

int get_distro_logo_rows(char *distro) {
    int rows;
    if (strstr(distro, "fedora") == 0) {
        rows = LEN(fedora);
    } else if (strcasecmp(distro, "gentoo") == 0) {
        rows = LEN(gentoo);
    } else if (strstr(distro, "Arch")) {
        rows = LEN(arch);
    } else if (strcasecmp(distro, "debian") == 0) {
        rows = LEN(debian);
    } else if (strstr(distro, "ubuntu") == 0) {
        rows = LEN(ubuntu);
    } else if (strcasecmp(distro, "nixos") == 0) {
        rows = LEN(nixos);
    } else {
        rows = LEN(linux_logo);
    }
    return rows;
}

char *get_distro_accent(char *distro) {
    char *accent_color = xmalloc(BUF_SIZE);
    if (strstr(distro, "fedora") == 0) {
        strncpy(accent_color, fedora_accent, BUF_SIZE);
    } else if (strcasecmp(distro, "gentoo") == 0) {
        strncpy(accent_color, gentoo_accent, BUF_SIZE);
    } else if (strstr(distro, "Arch")) {
        strncpy(accent_color, arch_accent, BUF_SIZE);
    } else if (strcasecmp(distro, "debian") == 0) {
        strncpy(accent_color, debian_accent, BUF_SIZE);
    } else if (strstr(distro, "ubuntu") == 0) {
        strncpy(accent_color, ubuntu_accent, BUF_SIZE);
    } else if (strcasecmp(distro, "nixos") == 0) {
        strncpy(accent_color, nixos_accent, BUF_SIZE);
    } else {
        strncpy(accent_color, linux_accent, BUF_SIZE);
    }
    return accent_color;
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

void print_field(char *logo_part, char *gap, const char* delimiter, char *accent, const char *field_name) {
    // NOTE: colors field requires a special treatment so we don't use print_info on it

    char *message = xmalloc(BUF_SIZE);
    // Set the function that will be used for getting the field value
    char *field_function = NULL;
    char *field_message = xmalloc(BUF_SIZE);
    // If the field should be ignored, used for fields that can return
    // empty values like DE, we don't want to print an empty DE field
    // if the end user is runnning a TWM
    int skip_field = 0;


    // Get the fields data
    if (strcasecmp(field_name, "os") == 0) {
        field_function = get_os(1);
    } else if (strcasecmp(field_name, "kernel") == 0) {
        field_function = get_kernel();
    } else if (strcasecmp(field_name, "uptime") == 0) {
        field_function = get_uptime();
    } else if (strcasecmp(field_name, "packages") == 0) {
        field_function = get_packages();
    } else if (strcasecmp(field_name, "de") == 0) {
        field_function = get_de();
        if (strlen(field_function) == 0) {
            skip_field = 1;
        }
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
    } else {
        log_error("Field '%s' doesn't exists", field_name);
        xfree(field_message);
        xfree(message);
        exit(1);
    }
    snprintf(field_message, BUF_SIZE, "%s_message", str_to_lower((char *)field_name));
    snprintf(field_message, BUF_SIZE, "%s", get_option_string(field_message));
    snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, field_function);
    xfree(field_message);

    if (strcasecmp(field_name, "kernel") != 0) {
        xfree(field_function);
    }

    // Print field information
    if (logo_part == NULL) {
        // When using minimal mode (without displaying logo)
        if (skip_field == 0) {
            printf("%s%s%s\n", gap, accent, message);
        }
    } else {
        if (skip_field == 0) {
            printf("%s%s%s%s\n", logo_part, gap, accent, message);
        }
    }
    if (skip_field) {
        skip_field = 0;
    }
    xfree(message);
}
