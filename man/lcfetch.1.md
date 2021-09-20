---
title: lcfetch
section: 1
header: lcfetch user manual
footer: lcfetch 0.1.0-beta6
date: September 19, 2021
---

# NAME

lcfetch - A fast and easy to configure alternative to neofetch.

# SYNOPSIS

**lcfetch** [*OPTIONS*] ...

# DESCRIPTION

**lcfetch** is a fast and easy to configure alternative to neofetch, written
in C and configured using Lua.

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

The **lcfetch** configurations resides under `~/.config/lcfetch` directory and a
`config.lua` file by default.

**enabled_fields**
: Enabled information fields, the data that will be printed.

    Available (working) fields:

    - "" (empty string) -> newline
    - User (JohnDoe@myhost)
    - Separator
    - OS
    - Kernel
    - Uptime
    - Packages
    - WM
    - Resolution
    - Shell
    - Terminal
    - CPU
    - Memory
    - Colors

    Type: table

**os_message**
: The OS field message.

    Type: string

    Default: "OS"

**kernel_message**
: The Kernel field message.

    Type: string

    Default: "Kernel"

**uptime_message**
: The Uptime field message.

    Type: string

    Default: "Uptime"

**packages_message**
: The Packages field message.

    Type: string

    Default: "Packages"

**resolution_message**
: The Resolution field message.

    Type: string

    Default: "Resolution"

**wm_message**
: The WM field message.

    Type: string

    Default: "WM"

**shell_message**
: The Shell field message.

    Type: string

    Default: "Shell"

**terminal_message**
: The Terminal field message.

    Type: string

    Default: "Terminal"

**cpu_message**
: The CPU field message.

    Type: string

    Default: "CPU"

**memory_message**
: The Memory field message.

    Type: string

    Default: "Memory"

**delimiter**
: The delimiter shown between the field message and the information

    Type: string

    Default: ":"

**separator**
:The separator shown between your USERNAME@HOSTNAME message 

    Type: string

    Default: "-"

**colors_style**
: The terminal colors style.

    Available styles:

    - classic
    - circles

    Default: "classic"

**accent_color**
: Accent color for the fields.

    Type: string

    Available colors:

    - black
    - red
    - green
    - yellow
    - blue
    - purple
    - cyan
    - white

    Default: ""

**ascii_distro**
: ASCII distro logo to be printed.

    Type: string

    Available logos:

    - tux
    - arch
    - nixos
    - fedora
    - gentoo
    - debian
    - ubuntu

    Default: ""

**display_logo**
: If the ASCII distro logo should be printed.

    Type: boolean

    Default: true

**gap**
: The gap between the ASCII distro logo and the information fields.

    Type: number

    Default: 3

**display_refresh_rate**
: If the screen refresh rate should be shown when displaying the screen resolution

    Type: boolean

    Default: false

**short_cpu_info**
: If the CPU information should be short or include extra information.

    Type: boolean

    Default: true

**memory_in_gib**
: If the memory should be printed as GiB instead of MiB.

    Type: boolean

    Default: true

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
