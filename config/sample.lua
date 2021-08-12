-----[[-----------------------------------]]-----
----                                         ----
---           FIELDS CONFIGURATIONS           ---
----                                         ----
-----]]-----------------------------------[[-----

-- Enabled information fields, the data that will be printed
-- Available (working) fields:
--   * "" (empty string) -> newline
--   * OS
--   * Kernel
--   * Uptime
--   * Shell
--   * Terminal
--   * Colors
--
-- NOTE: fields are case-insensitive
--[[ options.enabled_fields = {
  "OS",
  "Kernel",
  "Uptime",
  "Packages",
  "",
  "DE",
  "WM",
  "",
  "Shell",
  "Terminal",
  "",
  "CPU",
  "Memory",
  "",
  "Colors",
} ]]

----- DEFAULT FIELDS MESSAGES -------------------
-------------------------------------------------
-- options.os_message = "OS"
-- options.kernel_message = "Kernel"
-- options.uptime_message = "Uptime"
-- options.packages_message = "Packages"
-- options.de_message = "DE"
-- options.wm_message = "WM"
-- options.shell_message = "Shell"
-- options.terminal_message = "Terminal"
-- options.cpu_message = "CPU"
-- options.memory_message = "Memory"


-----[[-----------------------------------]]-----
----                                         ----
---          DISPLAY CONFIGURATIONS           ---
----                                         ----
-----]]-----------------------------------[[-----

-- The terminal colors style
-- Available styles:
--   * classic
--   * circles
--
-- NOTE: default is "classic"
-- options.colors_style = "classic"

-- ASCII distro logo to be printed
--
-- NOTE: by default the ASCII distro logo is automatically detected (WIP)
-- options.ascii_distro = ""

-- If the ASCII distro logo should be printed
--
-- NOTE: default is true
-- options.display_logo = true
