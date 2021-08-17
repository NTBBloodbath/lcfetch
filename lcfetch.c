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
#include "include/logos/linux.h"
// TODO: find the way to dynamically set the logo array in C
// #include "include/logos/gentoo.h"
// #include "include/logos/fedora.h"

struct utsname os_uname;
struct sysinfo sys_info;

Display *display;
int title_length, status;

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

static char *get_title(char *accent_color) {
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
    snprintf(title, BUF_SIZE, "%s%s\e[0m@%s%s\e[0m\n", accent_color, username,
             accent_color, hostname);

    return title;
}

static char *get_separator() {
    return repeat_string("-", title_length);
}

static char *get_os(int pretty_name) {
    char *os = xmalloc(BUF_SIZE);
    char *name = xmalloc(BUF_SIZE);
    char *line = NULL;
    size_t len;

    FILE *os_release = fopen("/etc/os-release", "r");
    while (getline(&line, &len, os_release) != -1) {
        // NOTE: the 'NAME' field will be used later for determining the
        // distribution ASCII logo and accent color
        if ((!pretty_name) && (sscanf(line, "NAME=%s", name) > 0)) {
            break;
        } else if ((!pretty_name) && (sscanf(line, "NAME=\"%[^\"]+", name) > 0)) {
            break;
        } else if ((pretty_name) && (sscanf(line, "PRETTY_NAME=\"%[^\"]+", name) > 0)) {
            break;
        }
    }
    xfree(line);
    fclose(os_release);
    if (pretty_name) {
        snprintf(os, BUF_SIZE, "%s %s", name, os_uname.machine);
    } else {
        snprintf(os, BUF_SIZE, "%s", name);
    }
    xfree(name);

    return os;
}

static char *get_kernel() {
    return os_uname.release;
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
    for (int i = 0; i < 4; i++) {
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
    char *terminal_pid_path = xmalloc(BUF_SIZE);
    char *terminal_child_pid_path = xmalloc(BUF_SIZE);
    char* line = NULL;
    size_t len;

    // Check if we are running in a TTY or a graphical X interface
    if (display != NULL) {
        pid_t ppid;
        // Get parent lcfetch process ID (shell)
        ppid = getppid();
        snprintf(terminal_child_pid_path, BUF_SIZE, "/proc/%d/status", ppid);

        // Get parent shell process ID (terminal)
        FILE *terminal_child_pid = fopen(terminal_child_pid_path, "r");
        while (getline(&line, &len, terminal_child_pid) != -1) {
            if (sscanf(line, "PPid: %d", terminal_pid) > 0) {
                break;
            }
        }
        fclose(terminal_child_pid);

        // Get terminal name
        snprintf(terminal_pid_path, BUF_SIZE, "/proc/%d/status", *terminal_pid);
        FILE *terminal_name = fopen(terminal_pid_path, "r");
        while (getline(&line, &len, terminal_name) != -1) {
            if (sscanf(line, "Name: %s", terminal) > 0) {
                break;
            }
        }
        fclose(terminal_name);
    } else {
        // In TTY, $TERM is simply returned as "linux" so we get the actual TTY name
        if (strcmp(terminal, "linux") == 0) {
            strncpy(terminal, ttyname(STDIN_FILENO), BUF_SIZE);
        }
    }

    xfree(line);
    xfree(terminal_pid);
    xfree(terminal_pid_path);
    xfree(terminal_child_pid_path);

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

void print_info() {
    // If the ASCII distro logo should be printed
    int display_logo = get_option_boolean("display_logo");

    // Get the accent color
    // NOTE: delete this after setting the dynamic coloring using the ASCII
    // distro
    // const char *accent_color = get_option_string("accent_color");

    // Get the amount of enabled information fields
    int enabled_fields = get_table_size("enabled_fields");
    if (display_logo) {
        // Get the gap that should be between the logo and the information
        int gap_size = get_option_number("gap");
        char *gap = repeat_string(" ", gap_size);

        for (int i = 0; i < COUNT(linux_logo); i++) {
            // Count two extra fields for (user@host and the separator)
            if (i >= enabled_fields + 2) {
                // If we've run out of information to show then we will
                // just print the next logo line
                printf("%s%s%s\n", linux_accent, linux_logo[i], "\e[0m");
            } else {
                if (i == 0) {
                    char *title = get_title(linux_accent);
                    printf("%s%s%s", linux_logo[i], gap, title);
                    xfree(title);
                } else if (i == 1) {
                    char *separator = get_separator();
                    printf("%s%s%s%s\n", linux_logo[i], "\e[0m", gap, separator);
                    xfree(separator);
                } else {
                    const char *field =
                        get_subtable_string("enabled_fields", i - 1);
                    if (strcasecmp(field, "colors") == 0) {
                        char *dark_colors = get_colors_dark();
                        char *bright_colors = get_colors_bright();
                        printf("%s%s%s\n", linux_logo[i], gap, dark_colors);
                        printf("%s%s%s\n", linux_logo[i], gap, bright_colors);
                        xfree(dark_colors);
                        xfree(bright_colors);
                    } else {
                        // Set the function that will be used for getting the field
                        // value
                        char *function = NULL;
                        const char *field_message = NULL;
                        // If we should draw an empty line as a separator
                        if (strcmp(field, "") == 0) {
                            printf("%s%s\n", linux_logo[i], "\e[0m");
                        } else {
                            char *message = xmalloc(BUF_SIZE);
                            if (strcasecmp(field, "OS") == 0) {
                                function = get_os(1);
                                field_message = get_option_string("os_message");
                                snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                                xfree(function);
                            } else if (strcasecmp(field, "Kernel") == 0) {
                                function = get_kernel();
                                field_message = get_option_string("kernel_message");
                                snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                                // xfree(function);
                            } else if (strcasecmp(field, "Uptime") == 0) {
                                function = get_uptime();
                                field_message = get_option_string("uptime_message");
                                snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                                xfree(function);
                            } else if (strcasecmp(field, "Shell") == 0) {
                                function = get_shell();
                                field_message = get_option_string("shell_message");
                                snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                                xfree(function);
                            } else if (strcasecmp(field, "Terminal") == 0) {
                                function = get_terminal();
                                field_message = get_option_string("terminal_message");
                                snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                                xfree(function);
                            } else {
                                function = "Not implemented yet (maybe?)";
                                field_message = (char*)field;
                                snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                            }
                            printf("%s%s%s%s\n", linux_logo[i], gap, linux_accent, message);
                            xfree(message);
                        }
                    }
                }
            }
        }
        xfree(gap);
    } else {
        for (int i = 0; i <= (enabled_fields + 1); i++) {
            // Count two extra fields for (user@host and the separator)
            if (i == 0) {
                char *title = get_title(linux_accent);
                printf("%s", title);
                xfree(title);
            } else if (i == 1) {
                char *separator = get_separator();
                printf("%s\n", separator);
                xfree(separator);
            } else {
                const char *field =
                    get_subtable_string("enabled_fields", i - 1);
                if (strcasecmp(field, "colors") == 0) {
                    char *dark_colors = get_colors_dark();
                    char *bright_colors = get_colors_bright();
                    printf("%s\n", dark_colors);
                    printf("%s\n", bright_colors);
                    xfree(dark_colors);
                    xfree(bright_colors);
                } else {
                    // Set the function that will be used for getting the field
                    // value
                    char *function = NULL;
                    const char *field_message = NULL;
                    // If we should draw an empty line as a separator
                    if (strcmp(field, "") == 0) {
                        printf("\n");
                    } else {
                        char *message = xmalloc(BUF_SIZE);
                        if (strcasecmp(field, "OS") == 0) {
                            function = get_os(1);
                            field_message = get_option_string("os_message");
                            snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                            xfree(function);
                        } else if (strcasecmp(field, "Kernel") == 0) {
                            function = get_kernel();
                            field_message = get_option_string("kernel_message");
                            snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                            // xfree(function);
                        } else if (strcasecmp(field, "Uptime") == 0) {
                            function = get_uptime();
                            field_message = get_option_string("uptime_message");
                            snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                            xfree(function);
                        } else if (strcasecmp(field, "Shell") == 0) {
                            function = get_shell();
                            field_message = get_option_string("shell_message");
                            snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                            xfree(function);
                        } else if (strcasecmp(field, "Terminal") == 0) {
                            function = get_terminal();
                            field_message = get_option_string("terminal_message");
                            snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                            xfree(function);
                        } else {
                            function = "Not implemented yet (maybe?)";
                            field_message = (char*)field;
                            snprintf(message, BUF_SIZE, "%s%s: %s", field_message, "\e[0m", function);
                        }
                        printf("%s%s\n", linux_accent, message);
                        xfree(message);
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
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

    // populate the os_uname struct
    uname(&os_uname);
    // populate the sys_info struct
    sysinfo(&sys_info);

    display = XOpenDisplay(NULL);

    // Print all stuff (logo, information)
    print_info();

    if(display != NULL) { 
        XCloseDisplay(display);
    }

    // Close our Lua environment and release resources
    stop_lua();
    return 0;
}
