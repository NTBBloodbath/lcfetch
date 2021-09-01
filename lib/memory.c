#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../third-party/log.c/src/log.h"

/**
 * A malloc() wrapper that checks the results and dies in case of error
 */
void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        log_fatal("allocating memory; %s\n", strerror(errno));
        exit(errno);
    }

    return ptr;
}

/**
 * A free() wrapper that dies if fed with NULL pointer
 */
void xfree(void *ptr) {
    if (ptr == NULL) {
        log_fatal("allocating memory; %s\n", strerror(errno));
        exit(errno);
    }

    free(ptr);
}
