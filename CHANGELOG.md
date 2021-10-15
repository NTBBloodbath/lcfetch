# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2021-10-15

### Added

- Add `colors_icon` option, now users can use their own color palette themes!
- Add `ghosts` color palette style (_requires a patched font like nerd fonts_)
- Add `custom_ascii_logo` option, set your own ASCII logo!
- Add support for Android devices (Termux)
- Add debugging build targets in Makefile (`make dev` and `make valgrind`)
- Add `USER_FLAGS` variable to Makefile to handle extra preprocessors that are optionals (e.g. `-DUSE_PWD_SHELL`)
- ASCII logos:
  - Android
  - Manjaro

### Changed

- Shell is now obtained in a more robust and accurate method by default instead of relying in `SHELL` environment variable
- If a field cannot be obtained then print line without field information

### Fixed

- Fix CPU model if `/proc/cpuinfo` does not have a `model name` field
- Fix getting username in some distributions (and WSL)
- Fix ninja bug where the configuration files were ignored
- Logos with special characters were having a weird length
- Proper conditional to get Debian logo and accent
- Do not override lcfetch configuration file when installing

## [0.1.2] - 2021-09-19

### Fixed

- Properly detect some logos with case-insensitive conditionals

## [0.1.1] - 2021-09-19

### Added

- Add `print_colors` function to handle colors field

### Changed

- Small Lua API stack changes

### Fixed

- Properly detect some logos with `strstr`

## [0.1.0] - 2021-09-19

### Added

- Support for WM field
- Support for `separator` configuration option
- Support for `accent_color` configuration option

### Changed

- Increased buffer size to 256
- `user@hostname` and `separator` (`----------`) are now fields that can be enabled or disabled in your configurations
- Use `-Ofast` instead of `-O2` when building to improve performance
- Use `<stdbool.h>` header for boolean values instead of `0` and `1` (should improve code readability)
- Refactor printing logic with a faster implementation
- Use `getenv` for getting the username (fixes issues on WSL)
- Use `log_fatal` instead of `log_error` when we were unable to open system files
- Memory field small visual tweaks
- Changed Ubuntu logo detection
- Changed Fedora logo detection to also detect incoming F35 changes to `/etc/os-release` file

### Fixed

- Small bugs on Lua API stack
- Properly exit on errors (ERROR and FATAL logs) 
- Proper conditional for Nix packages
- Proper terminal detection on WSL (only WT at the moment)

## [0.1.0-beta6] - 2021-08-25

### Added

- ASCII logos:
  - Ubuntu

### Fixed

- Reverted commit [f7f715e](https://github.com/NTBBloodbath/lcfetch/commit/f7f715e19f7274526052632f37912f185a259327), configurations should
    work as expected now
- Fixed Arch logo detection
- Use the current distro accent when the logo is not displayed
- Logo lines will not be duplicated anymore when rendering the color palette

## [0.1.0-beta5] - 2021-08-21

### Added

- Support for resolution field
- Support for dynamic distribution ASCII logos
- Option to set a custom delimiter between the field message and the information
- Option to choose if the OS architecture should be shown at the right side of the OS name or not
- Support for Nix package manager
- ASCII logos:
  - Arch
  - Debian
  - NixOS (seems to be buggy for me)

### Fixed

- packages field will not try to use non installed managers anymore

## [0.1.0-beta4] - 2021-08-20

### Added

- Support for packages field
- Option to choose whether showing extra information when printing CPU field
  or keep it short

### Changed

- Releases now have Lua embedded
- Lua 5.3.6 will now be optionally downloaded when building
- Allow using gap when using minimal mode (not displaying logo)
- Updated default configurations

## [0.1.0-beta3] - 2021-08-18

### Added

- Support for CPU field
- Support for memory field
- Option to choose whether showing memory in GiB or MiB
- Better error handling when opening system files like `/proc/cpuinfo`

### Changed

- Updated default configurations
- Third party dependencies are now handled by our Makefile, no more submodules!

### Fixed

- Fixed a segfault caused if the gap option was equal to 0
- Logo and information will not wrap anymore on small terminals or resize
- If the logo was completely rendered but there is missing information (still not rendered)
    lcfetch will also render this information, as it should be

## [0.1.0-beta2] - 2021-08-17

### Added

- Man pages, you can use `man lcfetch` now to see lcfetch documentation!
- Option to choose if the ASCII distro logo should be printed or not
- Option for custom spacing between the ASCII distro logo and the information

### Changed

- Default distro logo is now a Tux, should make more sense
- Some default configurations
- Copyright notice is now a lot better

### Fixed

- Set `dumpstack` debugging function as unused
- We had some memory leaks, and then poof, they disappeared

## [0.1.0-beta1] - 2021-08-12

- Initial release

[Unreleased]: https://github.com/NTBBloodbath/lcfetch/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.2..v0.2.0
[0.1.2]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.1..v0.1.2
[0.1.1]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0..v0.1.1
[0.1.0]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0-beta6..v0.1.0
[0.1.0-beta6]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0-beta5..v0.1.0-beta6
[0.1.0-beta5]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0-beta4..v0.1.0-beta5
[0.1.0-beta4]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0-beta3..v0.1.0-beta4
[0.1.0-beta3]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0-beta2..v0.1.0-beta3
[0.1.0-beta2]: https://github.com/NTBBloodbath/lcfetch/compare/v0.1.0-beta1..v0.1.0-beta2
[0.1.0-beta1]: https://github.com/NTBBloodbath/lcfetch/releases/tag/v0.1.0-beta1
