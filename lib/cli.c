#include <stdio.h>
#include <stdlib.h>
#include "../include/lcfetch.h"

void version() {
    printf("lcfetch %s\n\n%s\n", VERSION, COPYRIGHT);
    exit(0);
}

void help() {
    const char *help_message =
        "Usage: lcfetch [OPTIONS]\n\n"
        "OPTIONS:\n"
        "\t-c, --config /path/to/config\tSpecify a path to a custom config "
        "file\n"
        "\t-h, --help\t\t\t\t\t\tPrint this message and exit\n"
        "\t-v, --version\t\t\t\t\tShow lcfetch version\n\n"
        "Report bugs to https://github.com/NTBBloodbath/lcfetch/issues\n";
    printf("%s", help_message);
}
