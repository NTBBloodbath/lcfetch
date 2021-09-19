#include "../include/lcfetch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
