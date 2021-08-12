/* C stdlib */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
/* Custom headers */
#include "include/lcfetch.h"

struct utsname os_uname;
struct sysinfo sys_info;

Display *display;
int title_length, status;

static char *get_title() {
    // reduce the maximum size for the title components so we don't over-fill
    // the string
    char hostname[BUF_SIZE / 3];
    status = gethostname(hostname, BUF_SIZE / 3);

    char username[BUF_SIZE / 3];
    status = getlogin_r(username, BUF_SIZE / 3);

    char *title = xmalloc(BUF_SIZE);
    // Calculate the length of hostname + @ + username
    title_length = strlen(hostname) + strlen(username) + 1;

    // Get the accent color
    const char *accent_color = get_option_string("accent_color");
    snprintf(title, BUF_SIZE, "%s%s\e[0m@%s%s\e[0m\n", accent_color, username,
             accent_color, hostname);

    return title;
}

static char *get_separator() {
    char *separator = xmalloc(BUF_SIZE);
    char *str = separator;

    for (int i = 0; i < title_length; i++) {
        *(str++) = '-';
    }
    *str = '\0';

    return separator;
}

static char *get_os() {
    char *os = xmalloc(BUF_SIZE);
    char *name = xmalloc(BUF_SIZE);
    char *line = NULL;
    size_t len;

    FILE *os_release = fopen("/etc/os-release", "r");
    while (getline(&line, &len, os_release) != -1) {
        if (sscanf(line, "PRETTY_NAME=\"%[^\"]+", name) > 0) {
            break;
        }
    }
    xfree(line);
    fclose(os_release);
    snprintf(os, BUF_SIZE, "%s %s", name, os_uname.machine);
    xfree(name);

    return os;
}

static char *get_kernel() {
    char *kernel = xmalloc(BUF_SIZE);
    strncpy(kernel, os_uname.release, BUF_SIZE);

    return kernel;
}

static char *get_uptime() {
    long seconds = sys_info.uptime;
    struct {
        char *name;
        int seconds;
    } units[] = {
        {"week", 60 * 60 * 24 * 7},
        {"day", 60 * 60 * 24},
        {"hour", 60 * 60},
        {"min", 60},
    };

    int n, len = 0;
    char *uptime = xmalloc(BUF_SIZE);
    for (int i = 0; i < 4; ++i) {
        if ((n = seconds / units[i].seconds) || i == 2) {
            len += snprintf(uptime + len, BUF_SIZE - len, "%d %s%s, ", n,
                            units[i].name, n != 1 ? "s" : "");
        }
        seconds %= units[i].seconds;
    }

    // null-terminate at the trailing comma
    uptime[len - 2] = '\0';
    return uptime;
}

static char *get_shell() {
    char *shell = xmalloc(BUF_SIZE);
    char *shell_path = getenv("SHELL");
    char *shell_name = strrchr(getenv("SHELL"), '/');

    // If the shell does not contains a separator in the path, e.g.
    // zsh instead of /usr/bin/zsh then write it directly
    if (shell_name == NULL) {
        strncpy(shell, shell_path, BUF_SIZE);
    } else {
        // Copy only the last '/'
        strncpy(shell, shell_name + 1, BUF_SIZE);
    }

    return shell;
}

static char *get_terminal() {
    char *terminal = xmalloc(BUF_SIZE);
    int* terminal_pid = xmalloc(BUF_SIZE);
    const char *terminal_pid_path = xmalloc(BUF_SIZE);
    const char *terminal_child_pid_path = xmalloc(BUF_SIZE);
    char* line = NULL;
    size_t len;

    // Check if we are running in a TTY or a graphical X interface
    if (display != NULL) {
        pid_t ppid;
        // Get parent lcfetch process ID (shell)
        ppid = getppid();
        snprintf((char*)terminal_child_pid_path, BUF_SIZE, "/proc/%d/status", ppid);

        // Get parent shell process ID (terminal)
        FILE *terminal_child_pid = fopen(terminal_child_pid_path, "r");
        while (getline(&line, &len, terminal_child_pid) != -1) {
            if (sscanf(line, "PPid: %d", terminal_pid) > 0) {
                break;
            }
        }

        // Get terminal name
        snprintf((char*)terminal_pid_path, BUF_SIZE, "/proc/%d/status", *terminal_pid);
        FILE *terminal_name = fopen(terminal_pid_path, "r");
        while (getline(&line, &len, terminal_name) != -1) {
            if (sscanf(line, "Name: %s", terminal) > 0) {
                break;
            }
        }
    } else {
        // In TTY, $TERM is simply returned as "linux" so we get the actual TTY name
        if (strcmp(terminal, "linux") == 0) {
            strncpy(terminal, ttyname(STDIN_FILENO), BUF_SIZE);
        }
    }

    xfree(line);
    xfree(terminal_pid);
    xfree((char*)terminal_pid_path);
    xfree((char*)terminal_child_pid_path);

    return terminal;
}

static char *get_colors_dark() {
    char *dark_colors = xmalloc(BUF_SIZE);
    char *str = dark_colors;
    const char *colors_style = get_option_string("colors_style");

    for (int i = 0; i < 8; i++) {
        if (strcasecmp(colors_style, "circles") == 0) {
            sprintf(str, "\e[3%dm⬤  ", i);
            str += 10;
        } else if (strcasecmp(colors_style, "classic") == 0) {
            sprintf(str, "\e[4%dm   ", i);
            str += 8;
        }
    }
    snprintf(str, 5, "\e[0m");

    return dark_colors;
}

static char *get_colors_bright() {
    char *bright_colors = xmalloc(BUF_SIZE);
    char *str = bright_colors;
    const char *colors_style = get_option_string("colors_style");

    for (int i = 8; i < 16; i++) {
        if (strcasecmp(colors_style, "circles") == 0) {
            sprintf(str, "\e[38;5;%dm⬤  ", i);
            str += 14 + (i >= 10 ? 1 : 0);
        } else if (strcasecmp(colors_style, "classic") == 0) {
            sprintf(str, "\e[48;5;%dm   ", i);
            str += 12 + (i >= 10 ? 1 : 0);
        }
    }
    snprintf(str, 5, "\e[0m");

    return bright_colors;
}

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

int main(int argc, char *argv[]) {
    char *LOGO[] = {
        "\e[1;35m         -/oyddmdhs+:.                ",
        "     -o\e[1;37mdNMMMMMMMMNNmhy+\e[1;35m-`             ",
        "   -y\e[1;37mNMMMMMMMMMMMNNNmmdhy\e[1;35m+-           ",
        " `o\e[1;37mmMMMMMMMMMMMMNmdmmmmddhhy\e[1;35m/`        ",
        " om\e[1;37mMMMMMMMMMMMN\e[1;35mhhyyyo\e[1;37mhmdddhhhd\e[1;35mo`      ",
        ".y\e[1;37mdMMMMMMMMMMd\e[1;35mhs++so/s\e[1;37mmdddhhhhdm\e[1;35m+`    ",
        " oy\e[1;37mhdmNMMMMMMMN\e[1;35mdyooy\e[1;37mdmddddhhhhyhN\e[1;35md.   ",
        "  :o\e[1;37myhhdNNMMMMMMMNNNmmdddhhhhhyym\e[1;35mMh   ",
        "    .:\e[1;37m+sydNMMMMMNNNmmmdddhhhhhhmM\e[1;35mmy   ",
        "       /m\e[1;37mMMMMMMNNNmmmdddhhhhhmMNh\e[1;35ms:   ",
        "    `o\e[1;37mNMMMMMMMNNNmmmddddhhdmMNhs\e[1;35m+`    ",
        "  `s\e[1;37mNMMMMMMMMNNNmmmdddddmNMmhs\e[1;35m/.      ",
        " /N\e[1;37mMMMMMMMMNNNNmmmdddmNMNdso\e[1;35m:`        ",
        "+M\e[1;37mMMMMMMNNNNNmmmmdmNMNdso\e[1;35m/-           ",
        "yM\e[1;37mMNNNNNNNmmmmmNNMmhs+/\e[1;35m-`             ",
        "/h\e[1;37mMMNNNNNNNNMNdhs++/\e[1;35m-`                ",
        "`/\e[1;37mohdmmddhys+++/:\e[1;35m.`                   ",
        "  `-//////:--.\n                                      ",
    };
    // Command-line arguments (CLI)
    int c;
    char *config_file_path;
    while (1) {
        static struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"version", no_argument, NULL, 'v'},
            {"config", required_argument, NULL, 'c'},
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "hvc:", long_options, &option_index);

        // Detect the end of the command-line options
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v':
            version();
        case 'h':
            help();
            exit(0);
        case 'c':
            config_file_path = optarg;
            break;
        default:
            help();
            exit(1);
        }
    }

    // Start our Lua environment
    start_lua(config_file_path);

    // Get the accent color
    // NOTE: delete this after setting the dynamic coloring using the ASCII
    // distro
    const char *accent_color = get_option_string("accent_color");

    // populate the os_uname struct
    uname(&os_uname);
    // populate the sys_info struct
    sysinfo(&sys_info);
    display = XOpenDisplay(NULL);

    // Print ASCII distro logo
    int enabled_fields = get_table_size("enabled_fields");
    for (int i = 0; i < COUNT(LOGO); i++) {
        // Count two extra fields for (user@host and the separator)
        if (i >= enabled_fields + 2) {
            // If we've run out of information to show then we will
            // just print the next logo line
            printf("%s%s%s\n", accent_color, LOGO[i], "\e[0m");
        } else {
            if (i == 0) {
                printf("%s %s", LOGO[i], get_title());
            } else if (i == 1) {
                printf("%s%s %s\n", LOGO[i], "\e[0m", get_separator());
            } else {
                const char *field =
                    get_subtable_string("enabled_fields", i - 1);
                if (strcasecmp(field, "colors") == 0) {
                    printf("%s %s\n", LOGO[i], get_colors_dark());
                    printf("%s %s\n", LOGO[i], get_colors_bright());
                } else {
                    // Set the function that will be used for getting the field
                    // value
                    const char *function = NULL;
                    const char *field_message = NULL;
                    // If we should draw an empty line as a separator
                    if (strcmp(field, "") == 0) {
                        printf("%s%s\n", LOGO[i], "\e[0m");
                    } else {
                        // strncpy(field_message, field, -1);
                        // printf("%s", field_message);
                        if (strcasecmp(field, "OS") == 0) {
                            function = get_os();
                            field_message = get_option_string("os_message");
                        } else if (strcasecmp(field, "Kernel") == 0) {
                            function = get_kernel();
                            field_message = get_option_string("kernel_message");
                        } else if (strcasecmp(field, "Uptime") == 0) {
                            function = get_uptime();
                            field_message = get_option_string("uptime_message");
                        } else if (strcasecmp(field, "Shell") == 0) {
                            function = get_shell();
                            field_message = get_option_string("shell_message");
                        } else if (strcasecmp(field, "Terminal") == 0) {
                            function = get_terminal();
                            field_message = get_option_string("terminal_message");
                        } else {
                            function = "Not implemented yet (maybe?)";
                            field_message = (char*)field;
                        }
                        printf("%s %s%s: %s\n", LOGO[i], field_message, "\e[0m", function);
                    }
                }
            }
        }
    }

    if(display != NULL) { 
        XCloseDisplay(display);
    }

    // Close our Lua environment and release resources
    stop_lua();
    return 0;
}
