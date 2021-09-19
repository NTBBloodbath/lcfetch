/* C stdlib */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
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
#include "third-party/log.c/src/log.h"
/* ASCII logos */
#include "include/logos/arch.h"
#include "include/logos/debian.h"
#include "include/logos/fedora.h"
#include "include/logos/gentoo.h"
#include "include/logos/linux.h"
#include "include/logos/nixos.h"
#include "include/logos/ubuntu.h"

struct utsname os_uname;
struct sysinfo sys_info;

Display *display;
int title_length;

static char *get_property(Display *disp, Window win, Atom xa_prop_type, char *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    // char *ret = NULL;

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

static char *get_title(char *accent_color) {
    char *title = xmalloc(BUF_SIZE);

    // reduce the maximum size for the title components so we don't over-fill
    // the string
    char hostname[BUF_SIZE / 3];
    gethostname(hostname, BUF_SIZE / 3);

    char *username = getenv("USER");

    // Calculate the length of hostname + @ + username
    title_length = strlen(hostname) + strlen(username) + 1;

    // Get the accent color
    snprintf(title, BUF_SIZE, "%s%s\e[0m@%s%s\e[0m\n", accent_color, username, accent_color, hostname);

    return title;
}

static char *get_separator() {
    const char *separator = get_option_string("separator");
    return repeat_string((char*)separator, title_length);
}

static char *get_os(int pretty_name) {
    char *os = xmalloc(BUF_SIZE);
    char *name = xmalloc(BUF_SIZE);
    char *line = NULL;
    int show_arch = get_option_boolean("show_arch");
    size_t len;

    FILE *os_release = fopen("/etc/os-release", "r");
    if (os_release == NULL) {
        log_fatal("Unable to open /etc/os-release");
        exit(1);
    }
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

    if (pretty_name && show_arch) {
        snprintf(os, BUF_SIZE, "%s %s", name, os_uname.machine);
    } else {
        snprintf(os, BUF_SIZE, "%s", name);
    }
    xfree(name);

    return os;
}

static char *get_kernel() { return os_uname.release; }

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
            len += snprintf(uptime + len, BUF_SIZE - len, "%d %s%s, ", n, units[i].name, n != 1 ? "s" : "");
        }
        seconds %= units[i].seconds;
    }

    // null-terminate at the trailing comma
    uptime[len - 2] = '\0';
    return uptime;
}

static char *get_wm() {
    char *wm_name = NULL;

    if (display != NULL) {
        Window *top_win = NULL;

        top_win = (Window *)get_property(display, DefaultRootWindow(display), XA_WINDOW, "_NET_SUPPORTING_WM_CHECK", NULL);
        if (!top_win) {
            top_win = (Window *)get_property(display, DefaultRootWindow(display), XA_CARDINAL, "_WIN_SUPPORTING_WM_CHECK", NULL);
            if (!top_win) {
                log_debug("Cannot get window manager required properties."
                          "(_NET_SUPPORTING_WM_CHECK or _WIN_SUPPORTING_WM_CHECK)\n",
                          stderr);
                XFree(top_win);
                return "lcfetch was not able to recognize your window manager";
            }
        }

        wm_name = get_property(display, *top_win, XInternAtom(display, "UTF8_STRING", 0), "_NET_WM_NAME", NULL);
        if (!wm_name) {
            wm_name = get_property(display, *top_win, XA_STRING, "_NET_WM_NAME", NULL);
            if (!wm_name) {
                log_debug("Cannot get name of the window manager (_NET_WM_NAME).\n");
                xfree(wm_name);
                return "lcfetch was not able to recognize your window manager";
            }
        }

        XFree(top_win);
    }

    if (wm_name == NULL) {
        snprintf(wm_name, BUF_SIZE, "lcfetch was not able to recognize your window manager");
    }

    return wm_name;
}

static char *get_de() {
    char *de_name = xmalloc(BUF_SIZE);
    char *xdg_desktop = getenv("XDG_CURRENT_DESKTOP");
    char *desktop_session = getenv("DESKTOP_SESSION");
    char *session_name = strrchr(getenv("DESKTOP_SESSION"), '/');

    if (xdg_desktop != NULL) {
        snprintf(de_name, BUF_SIZE, "%s", xdg_desktop);
    }

    if (de_name == NULL) {
        if (session_name == NULL) {
            snprintf(de_name, BUF_SIZE, "%s", desktop_session);
        } else  {
            snprintf(de_name, BUF_SIZE, "%s", session_name + 1);
        }
    }

    // Get the WM name so if DE name is equal to WM name, we will skip DE
    char *wm_name = get_wm();
    if (strcasecmp(wm_name, de_name) == 0) {
        return NULL;
    }
    xfree(wm_name);

    return de_name;
}

static char *get_resolution() {
    char *res = xmalloc(BUF_SIZE);

    if (display != NULL) {
        Screen *screen = DefaultScreenOfDisplay(display);
        int display_refresh_rate = get_option_boolean("display_refresh_rate");

        snprintf(res, BUF_SIZE, "%dx%d", screen->width, screen->height);
        if (display_refresh_rate) {
            Window root = RootWindow(display, 0);
            XRRScreenConfiguration *conf = XRRGetScreenInfo(display, root);
            snprintf(res + strlen(res), BUF_SIZE, " @ %dHz", XRRConfigCurrentRate(conf));
        }
    } else {
        // If we were unable to detect the screen resolution then send an error message
        // in the information field
        snprintf(res, BUF_SIZE, "lcfetch was not able to recognize your screen resolution");
    }

    return res;
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
    unsigned char *property;
    char *terminal = xmalloc(BUF_SIZE);
    // Windows Terminal session, we will use it for WSL detection
    char *wt_session = getenv("WT_SESSION");
    // Get the TERM environment variable, we will use it for TTY detection
    char *environment_term = getenv("TERM");

    // Check if we are running in a TTY, a graphical X interface or WSL (on Windows Terminal)
    if (display != NULL) {
        // Get the current window
        unsigned long _, window = RootWindow(display, XDefaultScreen(display));
        // Get the active window and the window class name
        Atom a, active_win = XInternAtom(display, "_NET_ACTIVE_WINDOW", 1),
                win_class = XInternAtom(display, "WM_CLASS", 1);

        XGetWindowProperty(display, window, active_win, 0, 64, 0, 0, &a, (int *)&_, &_, &_, &property);
        window = (property[3] << 24) + (property[2] << 16) + (property[1] << 8) + property[0];
        xfree(property);

        XGetWindowProperty(display, window, win_class, 0, 64, 0, 0, &a, (int *)&_, &_, &_, &property);

        snprintf(terminal, BUF_SIZE, "%s", property);
        xfree(property);
    } else {
        // Check if we are running on WSL inside the Windows Terminal
        if (wt_session != NULL) {
            strncpy(terminal, "Windows Terminal", BUF_SIZE);
        } else {
            // In TTY, $TERM is simply returned as "linux" so we get the actual TTY name
            if (strcmp(environment_term, "linux") == 0) {
                strncpy(terminal, ttyname(STDIN_FILENO), BUF_SIZE);
            }
        }
    }

    return terminal;
}

static char *get_packages() {
    char *pkg_managers[] = {"apt", "dnf", "rpm", "nix", "pacman", "apk", "xbps-query", "flatpak"};
    char *packages = xmalloc(BUF_SIZE * 2);
    int apt, rpm, dnf, pacman, aur, nix, apk, xbps, flatpak = 0;

    // Add an initial empty string to our packages characters array to be able
    // to use snprintf() for append to it later
    snprintf(packages, BUF_SIZE, "");

    // Store a count of the displayed package managers so we can dynamically add the commas on them
    int displayed_pkg_managers = 0;
    for (int i = 0; i < COUNT(pkg_managers); i++) {
        char *pkg_manager = pkg_managers[i];
        // Set the which command to run later
        char *which_command = xmalloc(BUF_SIZE);
        snprintf(which_command, BUF_SIZE, "which %s >/dev/null 2>&1", pkg_manager);
        // If the package manager is installed
        if (system(which_command) == 0) {
            if (strcmp(pkg_manager, "apt") == 0) {
                FILE *dpkg_packages = popen("dpkg --list 2> /dev/null | grep -c ^ii", "r");
                fscanf(dpkg_packages, "%d", &apt);
                pclose(dpkg_packages);
                // If there are packages installed then let's print the packages count
                // NOTE: this is for avoiding values like "0 (foo)" because you can install
                // APT and others packages managers in almost any distro.
                if (apt > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", apt, "dpkg");
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", apt, "dpkg");
                    }
                    displayed_pkg_managers++;
                }
            } else if (strcmp(pkg_manager, "dnf") == 0) {
                // Using DNF package cache is much faster than RPM
                FILE *dnf_packages =
                    popen("sqlite3 /var/cache/dnf/packages.db 'SELECT count(pkg) FROM installed'", "r");
                fscanf(dnf_packages, "%d", &dnf);
                pclose(dnf_packages);
                if (dnf > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", dnf, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", dnf, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
            } else if ((strcmp(pkg_manager, "rpm") == 0) && (dnf == 0)) {
                // If the current package manager is RPM and DNF packages count is zero
                // because if we already scanned the packages with DNF it makes no sense
                // to scan them again
                FILE *rpm_packages = popen("rpm -qa 2> /dev/null | wc -l", "r");
                fscanf(rpm_packages, "%d", &rpm);
                pclose(rpm_packages);
                if (rpm > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", rpm, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", rpm, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
            } else if (strcmp(pkg_manager, "pacman") == 0) {
                FILE *pacman_packages = popen("pacman -Qq 2> /dev/null | wc -l", "r");
                fscanf(pacman_packages, "%d", &pacman);
                pclose(pacman_packages);
                FILE *aur_packages = popen("pacman -Qqm 2> /dev/null | wc -l", "r");
                fscanf(aur_packages, "%d", &aur);
                pclose(aur_packages);
                if (pacman > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", pacman, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", pacman, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
                if (aur > 0) {
                    snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", aur, "AUR");
                    displayed_pkg_managers++;
                }
            } else if (strcmp(pkg_manager, "nix") == 0) {
                FILE *nix_current_system = popen("nix-store -q --requisites /run/current-system/sw", "r");
                fscanf(nix_current_system, "%d", &nix);
                pclose(nix_current_system);
                FILE *nix_profile = popen("nix-store -q --requisites ~/.nix-profile", "r");
                fscanf(nix_profile, "%d", &nix);
                pclose(nix_profile);
                if (nix > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", nix, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", nix, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
            } else if (strcmp(pkg_manager, "apk") == 0) {
                FILE *apk_packages = popen("apk info 2> /dev/null | wc -l", "r");
                fscanf(apk_packages, "%d", &apk);
                pclose(apk_packages);
                if (apk > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", apk, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", apk, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
            } else if (strcmp(pkg_manager, "xbps-query") == 0) {
                FILE *xbps_packages = popen("xbps-query -l 2> /dev/null | wc -l", "r");
                fscanf(xbps_packages, "%d", &xbps);
                pclose(xbps_packages);
                if (xbps > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", xbps, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", xbps, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
            } else if (strcmp(pkg_manager, "flatpak") == 0) {
                // NOTE: it seems that flatpak does not like to be called from a popen so it fails in
                // a really stupid way sending a non-sense error, this is why we are not using 'flatpak list'
                // for getting the flatpak packages at the moment
                FILE *flatpak_packages = popen("echo \"$(( $(ls /var/lib/flatpak/app 2> /dev/null | wc -l) + $(ls "
                                               "/var/lib/flatpak/runtime 2> /dev/null | wc -l) ))\"",
                                               "r");
                fscanf(flatpak_packages, "%d", &flatpak);
                pclose(flatpak_packages);
                if (flatpak > 0) {
                    if (displayed_pkg_managers >= 1) {
                        snprintf(packages + strlen(packages), BUF_SIZE, ", %d (%s)", flatpak, pkg_manager);
                    } else {
                        snprintf(packages + strlen(packages), BUF_SIZE, "%d (%s)", flatpak, pkg_manager);
                    }
                    displayed_pkg_managers++;
                }
            }
        }
        xfree(which_command);
    }

    // If the packages weren't calculated because the package manager is not supported then
    // return an error message that actually makes sense
    if (strcmp(packages, "") == 0) {
        strncat(packages, "lcfetch was not able to recognize your system package manager", BUF_SIZE);
    }

    return packages;
}

static char *get_cpu() {
    char *line = NULL;
    char *cpu = xmalloc(BUF_SIZE);
    char *cpu_model = xmalloc(BUF_SIZE / 2);
    int num_cores = 0;
    int cpu_freq = 0;
    int prec = 3;
    double freq;
    char freq_unit[] = "GHz";
    size_t len;

    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo == NULL) {
        log_fatal("Unable to open /proc/cpuinfo");
        exit(1);
    }
    while (getline(&line, &len, cpuinfo) != -1) {
        num_cores += sscanf(line, "model name : %[^\n@]", cpu_model);
    }
    fclose(cpuinfo);
    xfree(line);

    line = NULL;
    FILE *cpufreq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
    // If /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq file exists
    // then read the CPU frequency from it. Otherwise, fallback to /proc/cpuinfo
    if (cpufreq) {
        // cpuinfo_max_freq has only a line so we don't need a while loop
        if (getline(&line, &len, cpufreq) != -1) {
            sscanf(line, "%d", &cpu_freq);
            // KHz / 1000 = MHz
            cpu_freq /= 1000;
        }
    } else {
        cpufreq = fopen("/proc/cpuinfo", "r");
        if (cpufreq == NULL) {
            log_fatal("Unable to open /proc/cpuinfo");
            exit(1);
        }
        while (getline(&line, &len, cpufreq) != -1) {
            if (sscanf(line, "cpu MHz: %lf", &freq) > 0) {
                break;
            }
        }
        // Convert frequency to integer, e.g. 1483
        cpu_freq = (int)freq;
    }
    fclose(cpufreq);
    xfree(line);

    // If the cpu frequency is lower than 1000 then it makes no sense
    // to render it as GHz so we will change it to MHz
    if (cpu_freq < 1000) {
        freq = (double)cpu_freq;
        freq_unit[0] = 'M';
        prec = 0; // Show freq as integer
    } else {
        // MHz / 1000 = GHz
        freq = cpu_freq / 1000.0;

        while (cpu_freq % 10 == 0) {
            prec--;
            cpu_freq /= 10;
        }
    }

    // e.g. Intel i5 760 (4) @ 2.8GHz
    snprintf(cpu, BUF_SIZE, "%s (%d) @ %.*f%s", cpu_model, num_cores, prec, freq, freq_unit);
    xfree(cpu_model);

    // Remove unneeded information
    int short_cpu_info = get_option_boolean("short_cpu_info");
    if (short_cpu_info) {
        cpu = remove_substr(remove_substr(cpu, "(R)"), "Core(TM)");
    }
    cpu = remove_substr(cpu, "CPU");

    // Remove the annoying whitespaces between characters in the string
    truncate_whitespaces(cpu);

    if (num_cores == 0) {
        *cpu = '\0';
    }

    return cpu;
}

static char *get_memory() {
    char *line = NULL;
    char *memory = xmalloc(BUF_SIZE);
    int memory_in_gib = get_option_boolean("memory_in_gib");

    int total_memory, used_memory;
    int total, shared, memfree, buffers, cached, reclaimable;
    size_t len;

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL) {
        log_fatal("Unable to open /proc/meminfo");
        exit(1);
    }
    while (getline(&line, &len, meminfo) != -1) {
        /* if sscanf doesn't find a match, pointer is untouched */
        sscanf(line, "MemTotal: %d", &total);
        sscanf(line, "Shmem: %d", &shared);
        sscanf(line, "MemFree: %d", &memfree);
        sscanf(line, "Buffers: %d", &buffers);
        sscanf(line, "Cached: %d", &cached);
        sscanf(line, "SReclaimable: %d", &reclaimable);
    }
    fclose(meminfo);
    xfree(line);

    // we're using same calculation as neofetch
    // KiB / 1024 = MiB
    used_memory = (total + shared - memfree - buffers - cached - reclaimable) / 1024;
    total_memory = total / 1024;
    if (memory_in_gib) {
        // MiB / 1024 = GiB
        float used_memory_gib = (double)used_memory / (double)1024;
        float total_memory_gib = (double)total_memory / (double)1024;
        snprintf(memory, BUF_SIZE, "%.2fGiB / %.2fGiB", used_memory_gib, total_memory_gib);
    } else {
        snprintf(memory, BUF_SIZE, "%dMiB / %dMiB", used_memory, total_memory);
    }

    return memory;
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
    // The delimiter shown between the field message and the information, e.g.
    // OS: Fedora 34 (KDE Plasma) x86_64
    //   ^
    // delimiter
    const char *delimiter = get_option_string("delimiter");

    // Get the accent color and logo for the current distro
    char **logo;
    int logo_rows;
    char *accent_color = xmalloc(BUF_SIZE);
    char *current_distro = get_os(0);
    const char *custom_distro_logo = get_option_string("ascii_distro");

    // Compare the current distribution first so we can override the information later
    // with the custom ascii distro logo
    // TODO: find a better way, this one is not the most efficient and can have a high cost
    if (strstr(current_distro, "fedora") == 0) {
        logo = fedora;
        logo_rows = LEN(fedora);
        strncpy(accent_color, fedora_accent, BUF_SIZE);
    } else if (strcasecmp(current_distro, "gentoo") == 0) {
        logo = gentoo;
        logo_rows = LEN(gentoo);
        strncpy(accent_color, gentoo_accent, BUF_SIZE);
    } else if (strstr(current_distro, "Arch")) {
        logo = arch;
        logo_rows = LEN(arch);
        strncpy(accent_color, arch_accent, BUF_SIZE);
    } else if (strcasecmp(current_distro, "debian") == 0) {
        logo = debian;
        logo_rows = LEN(debian);
        strncpy(accent_color, debian_accent, BUF_SIZE);
    } else if (strstr(current_distro, "ubuntu") == 0) {
        logo = ubuntu;
        logo_rows = LEN(ubuntu);
        strncpy(accent_color, ubuntu_accent, BUF_SIZE);
    } else if (strcasecmp(current_distro, "nixos") == 0) {
        logo = nixos;
        logo_rows = LEN(nixos);
        strncpy(accent_color, nixos_accent, BUF_SIZE);
    } else {
        logo = linux_logo;
        logo_rows = LEN(linux_logo);
        strncpy(accent_color, linux_accent, BUF_SIZE);
    }
    if (strcasecmp(custom_distro_logo, "fedora") == 0) {
        logo = fedora;
        logo_rows = LEN(fedora);
        strncpy(accent_color, fedora_accent, BUF_SIZE);
    } else if (strcasecmp(custom_distro_logo, "gentoo") == 0) {
        logo = gentoo;
        logo_rows = LEN(gentoo);
        strncpy(accent_color, gentoo_accent, BUF_SIZE);
    } else if (strcasecmp(custom_distro_logo, "arch") == 0) {
        logo = arch;
        logo_rows = LEN(arch);
        strncpy(accent_color, arch_accent, BUF_SIZE);
    } else if (strcasecmp(custom_distro_logo, "debian") == 0) {
        logo = debian;
        logo_rows = LEN(debian);
        strncpy(accent_color, debian_accent, BUF_SIZE);
    } else if (strcasecmp(custom_distro_logo, "ubuntu") == 0) {
        logo = ubuntu;
        logo_rows = LEN(ubuntu);
        strncpy(accent_color, ubuntu_accent, BUF_SIZE);
    } else if (strcasecmp(custom_distro_logo, "nixos") == 0) {
        logo = nixos;
        logo_rows = LEN(nixos);
        strncpy(accent_color, nixos_accent, BUF_SIZE);
    } else if (strcasecmp(custom_distro_logo, "tux") == 0) {
        logo = linux_logo;
        logo_rows = LEN(linux_logo);
        strncpy(accent_color, linux_accent, BUF_SIZE);
    }
    xfree(current_distro);

    // Get the amount of enabled information fields
    int enabled_fields = get_table_size("enabled_fields");
    if (display_logo) {
        // Get the logo length, substracting the ANSI escapes length
        // NOTE: this should be refactored once we manage to dynamically change the logo (maybe)
        int logo_length = (strlen(logo[0]) - strlen("\e[1;00m"));
        // Get the gap that should be between the logo and the information
        int gap_size = get_option_number("gap");
        char *gap_logo_info = repeat_string(" ", gap_size);
        // This gap is specially used when there's more information but the
        // logo is already complete
        char *gap_logo = repeat_string(" ", logo_length);

        // Store the amount of displayed information so we can determine if there is still information
        // that needs to be rendered after the logo finishes
        int displayed_info = 0;
        for (int i = 0; i < logo_rows; i++) {
            // Count two extra fields for (user@host and the separator)
            if (i >= enabled_fields + 2) {
                // If we've run out of information to show then we will
                // just print the next logo line
                printf("%s%s%s\n", accent_color, logo[i], "\e[0m");
            } else {
                if (i == 0) {
                    char *title = get_title(accent_color);
                    printf("%s%s%s", logo[i], gap_logo_info, title);
                    xfree(title);
                } else if (i == 1) {
                    char *separator = get_separator();
                    printf("%s%s%s%s\n", logo[i], "\e[0m", gap_logo_info, separator);
                    xfree(separator);
                } else {
                    displayed_info++;

                    const char *field = get_subtable_string("enabled_fields", i - 1);
                    if (strcasecmp(field, "Colors") == 0) {
                        char *dark_colors = get_colors_dark();
                        char *bright_colors = get_colors_bright();
                        printf("%s%s%s\n", logo[i], gap_logo_info, dark_colors);
                        printf("%s%s%s\n", logo[i + 1], gap_logo_info, bright_colors);
                        xfree(dark_colors);
                        xfree(bright_colors);
                        i++;
                    } else {
                        // Set the function that will be used for getting the field
                        // value
                        char *function = NULL;
                        const char *field_message = NULL;
                        // If the field should be ignored, used for fields that can return
                        // empty values like DE, we don't want to print an empty DE field
                        // if the end user is runnning a TWM
                        int skip_field = 0;

                        // If we should draw an empty line as a separator
                        if (strcmp(field, "") == 0) {
                            printf("%s%s\n", logo[i], "\e[0m");
                        } else {
                            char *message = xmalloc(BUF_SIZE);
                            if (strcasecmp(field, "OS") == 0) {
                                function = get_os(1);
                                field_message = get_option_string("os_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "Kernel") == 0) {
                                function = get_kernel();
                                field_message = get_option_string("kernel_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            } else if (strcasecmp(field, "Uptime") == 0) {
                                function = get_uptime();
                                field_message = get_option_string("uptime_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "Packages") == 0) {
                                function = get_packages();
                                field_message = get_option_string("packages_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "DE") == 0) {
                                function = get_de();
                                field_message = get_option_string("de_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                if (strlen(function) == 0) {
                                    skip_field = 1;
                                }
                                xfree(function);
                            } else if (strcasecmp(field, "WM") == 0) {
                                function = get_wm();
                                field_message = get_option_string("wm_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "Resolution") == 0) {
                                function = get_resolution();
                                field_message = get_option_string("resolution_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "Shell") == 0) {
                                function = get_shell();
                                field_message = get_option_string("shell_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "Terminal") == 0) {
                                function = get_terminal();
                                field_message = get_option_string("terminal_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "CPU") == 0) {
                                function = get_cpu();
                                field_message = get_option_string("cpu_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else if (strcasecmp(field, "Memory") == 0) {
                                function = get_memory();
                                field_message = get_option_string("memory_message");
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                                xfree(function);
                            } else {
                                function = "Not implemented yet (maybe?)";
                                field_message = (char *)field;
                                snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            }
                            if (skip_field == 0) {
                                printf("%s%s%s%s\n", logo[i], gap_logo_info, accent_color, message);
                            }
                            skip_field = 0;
                            xfree(message);
                        }
                    }
                }
            }
        }
        // If there's still information that needs to be rendered then let's render them
        // leaving a padding from the logo
        if (displayed_info < enabled_fields) {
            for (int i = displayed_info + 2; i <= (enabled_fields + 1); i++) {
                const char *field = get_subtable_string("enabled_fields", i - 1);
                if (strcasecmp(field, "colors") == 0) {
                    char *dark_colors = get_colors_dark();
                    char *bright_colors = get_colors_bright();
                    printf("%s%s%s\n", gap_logo, gap_logo_info, dark_colors);
                    printf("%s%s%s\n", gap_logo, gap_logo_info, bright_colors);
                    xfree(dark_colors);
                    xfree(bright_colors);
                } else {
                    // Set the function that will be used for getting the field
                    // value
                    char *function = NULL;
                    const char *field_message = NULL;
                    // If the field should be ignored, used for fields that can return
                    // empty values like DE, we don't want to print an empty DE field
                    // if the end user is runnning a TWM
                    int skip_field = 0;

                    // If we should draw an empty line as a separator
                    if (strcmp(field, "") == 0) {
                        printf("%s\n", gap_logo);
                    } else {
                        char *message = xmalloc(BUF_SIZE);
                        if (strcasecmp(field, "OS") == 0) {
                            function = get_os(1);
                            field_message = get_option_string("os_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Kernel") == 0) {
                            function = get_kernel();
                            field_message = get_option_string("kernel_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                        } else if (strcasecmp(field, "Uptime") == 0) {
                            function = get_uptime();
                            field_message = get_option_string("uptime_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Packages") == 0) {
                            function = get_packages();
                            field_message = get_option_string("packages_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "DE") == 0) {
                            function = get_de();
                            field_message = get_option_string("de_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            if (strlen(function) == 0) {
                                skip_field = 1;
                            }
                            xfree(function);
                        } else if (strcasecmp(field, "WM") == 0) {
                            function = get_wm();
                            field_message = get_option_string("wm_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Resolution") == 0) {
                            function = get_resolution();
                            field_message = get_option_string("resolution_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Shell") == 0) {
                            function = get_shell();
                            field_message = get_option_string("shell_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Terminal") == 0) {
                            function = get_terminal();
                            field_message = get_option_string("terminal_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "CPU") == 0) {
                            function = get_cpu();
                            field_message = get_option_string("cpu_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Memory") == 0) {
                            function = get_memory();
                            field_message = get_option_string("memory_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else {
                            function = "Not implemented yet (maybe?)";
                            field_message = (char *)field;
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                        }
                        if (skip_field == 0) {
                            printf("%s%s%s%s\n", gap_logo, gap_logo_info, accent_color, message);
                        }
                        skip_field = 0;
                        xfree(message);
                    }
                }
            }
        }

        // If the gap between the logo and the information was higher than 0
        // then we will need to free it
        if (gap_size >= 1) {
            xfree(gap_logo_info);
        }
        xfree(gap_logo);
    } else {
        // Get the gap that should be between the left terminal border and the information
        int gap_size = get_option_number("gap");
        char *gap_term_info = repeat_string(" ", gap_size);

        // Do not add gaps if gap_size is 0
        if (gap_size == 0) {
            gap_term_info = "";
        }

        for (int i = 0; i <= (enabled_fields + 1); i++) {
            // Count two extra fields for (user@host and the separator)
            if (i == 0) {
                char *title = get_title(accent_color);
                printf("%s%s", gap_term_info, title);
                xfree(title);
            } else if (i == 1) {
                char *separator = get_separator();
                printf("%s%s\n", gap_term_info, separator);
                xfree(separator);
            } else {
                const char *field = get_subtable_string("enabled_fields", i - 1);
                if (strcasecmp(field, "colors") == 0) {
                    char *dark_colors = get_colors_dark();
                    char *bright_colors = get_colors_bright();
                    printf("%s%s\n", gap_term_info, dark_colors);
                    printf("%s%s\n", gap_term_info, bright_colors);
                    xfree(dark_colors);
                    xfree(bright_colors);
                } else {
                    // Set the function that will be used for getting the field
                    // value
                    char *function = NULL;
                    const char *field_message = NULL;
                    // If the field should be ignored, used for fields that can return
                    // empty values like DE, we don't want to print an empty DE field
                    // if the end user is runnning a TWM
                    int skip_field = 0;

                    // If we should draw an empty line as a separator
                    if (strcmp(field, "") == 0) {
                        printf("\n");
                    } else {
                        char *message = xmalloc(BUF_SIZE);
                        if (strcasecmp(field, "OS") == 0) {
                            function = get_os(1);
                            field_message = get_option_string("os_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Kernel") == 0) {
                            function = get_kernel();
                            field_message = get_option_string("kernel_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            // xfree(function);
                        } else if (strcasecmp(field, "Uptime") == 0) {
                            function = get_uptime();
                            field_message = get_option_string("uptime_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Packages") == 0) {
                            function = get_packages();
                            field_message = get_option_string("packages_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "DE") == 0) {
                            function = get_de();
                            field_message = get_option_string("de_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            if (strlen(function) == 0) {
                                skip_field = 1;
                            }
                            xfree(function);
                        } else if (strcasecmp(field, "WM") == 0) {
                            function = get_wm();
                            field_message = get_option_string("wm_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Resolution") == 0) {
                            function = get_resolution();
                            field_message = get_option_string("resolution_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Shell") == 0) {
                            function = get_shell();
                            field_message = get_option_string("shell_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Terminal") == 0) {
                            function = get_terminal();
                            field_message = get_option_string("terminal_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "CPU") == 0) {
                            function = get_cpu();
                            field_message = get_option_string("cpu_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else if (strcasecmp(field, "Memory") == 0) {
                            function = get_memory();
                            field_message = get_option_string("memory_message");
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                            xfree(function);
                        } else {
                            function = "Not implemented yet (maybe?)";
                            field_message = (char *)field;
                            snprintf(message, BUF_SIZE, "%s%s%s %s", field_message, "\e[0m", delimiter, function);
                        }
                        if (skip_field == 0) {
                            printf("%s%s%s\n", gap_term_info, accent_color, message);
                        }
                        skip_field = 0;
                        xfree(message);
                    }
                }
            }
        }
    }
    xfree(accent_color);
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

    // Disable line wrapping so we can keep the logo intact on small terminals
    printf("\e[?7l");
    // Print all stuff (logo, information)
    print_info();
    // Re-enable line wrapping again
    printf("\e[?7h");

    if (display != NULL) {
        XCloseDisplay(display);
    }

    // Close our Lua environment and release resources
    stop_lua();
    return 0;
}
