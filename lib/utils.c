#include "../include/lcfetch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
