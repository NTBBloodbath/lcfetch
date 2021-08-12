---
title: lcfetch
section: 1
header: lcfetch user manual
footer: lcfetch 0.1.0-beta2
date: August 12, 2021
---

# NAME

lcfetch - A fast and easy to configure alternative to neofetch.

# SYNOPSIS

**lcfetch** [*OPTIONS*] ...

# DESCRIPTION

**lcfetch** is a fast and easy to configure alternative to neofetch, written
in C and configuredusing Lua.

# OPTIONS

**-h**, **--help**
: Print help message and exit.

**-v**, **--version**
: Show lcfetch version.

**-c**, **--config**
: Specify a path to a custom config file.

# EXAMPLES

**lcfetch -h | lcfetch --help**
: Display help message and exit.

**lcfetch -v | lcfetch --version**
: Show lcfetch version.

**lcfetch --config ~/.config/lcfetch/circle-colors.lua**
: Use the `~/.config/lcfetch/circle-colors.lua` file as the configurations file.

# EXIT VALUES

**0**
: Success.

**1**
: Invalid option.

# CONFIGURATION

**lcfetch** uses the Lua scripting language as its configurations language. All the
configuration options has a **options.** prefix because they're a Lua table.

**enabled_fields**
: Enabled information fields, the data that will be printed.

    Available (working) fields:

    - "" (empty string) -> newline
    - OS
    - Kernel
    - Uptime
    - Shell
    - Terminal
    - Colors

    Type: table

**os_message**
: The OS field message.

    Type: string

**kernel_message**
: The Kernel field message.

    Type: string

**uptime_message**
: The Uptime field message.

    Type: string

**packages_message**
: The Packages field message.

    Type: string

**de_message**
: The DE field message.

    Type: string

**wm_message**
: The WM field message.

    Type: string

**shell_message**
: The Shell field message.

    Type: string

**terminal_message**
: The Terminal field message.

    Type: string

**cpu_message**
: The CPU field message.

    Type: string

**memory_message**
: The Memory field message.

    Type: string

**colors_style**
: The terminal colors style.

    Available styles:

    - classic
    - circles

**ascii_distro**
: ASCII distro logo to be printed (does nothing at the moment).

**display_logo**
: If the ASCII distro logo should be printed.

# AUTHORS

Written by NTBBloodbath.

# BUGS

Submit bug reports online at: <https://github.com/NTBBloodbath/lcfetch/issues>.

# SEE ALSO

Full documentation and sources at: <https://github.com/NTBBloodbath/lcfetch>.

# COPYRIGHT

Copyright (c) 2021 NTBBloodbath. License GPLv2: GNU GPL version 2
<https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
