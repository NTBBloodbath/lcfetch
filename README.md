<div align="center">

# lcfetch

![License](https://img.shields.io/github/license/NTBBloodbath/lcfetch?style=flat-square)
[![build](https://github.com/NTBBloodbath/lcfetch/actions/workflows/build.yml/badge.svg)](https://github.com/NTBBloodbath/lcfetch/actions/workflows/build.yml)

![lcfetch demo](https://user-images.githubusercontent.com/36456999/133947515-0116a6fe-8e1e-4c23-a181-8d055a0071e2.png)

</div>

A fast and easy to configure alternative to [neofetch](https://github.com/dylanaraps/neofetch)
written in C and configured using Lua!

> **IMPORTANT**: I'm a newbie using C and the Lua C API so if you see something wrong
> please let me know!

## Installation

You can grab the latest release from our [releases](https://github.com/NTBBloodbath/lcfetch/releases/latest)
or if you want to live in the bleeding-edge with the latest features you can [build from source](#building-from-source).

> **NOTE**: lcfetch was tested only with Lua `5.3+` but should work as expected with earlier
> Lua versions.

### Building from source

#### Installing dependencies

The packages listed below are optional because lcfetch will download `Lua 5.3.6` locally
by default in order to avoid installing extra stuff in your system and libraries if they
aren't installed in your system.

> **IMPORTANT**: xmake is not in all distributions repositories,
> please refer to xmake [installation guide](https://xmake.io/#/guide/installation).

##### Ubuntu

```sh
apt install lua5.3 liblua5.3-dev libx11-dev libxrandr-dev libreadline-dev
```

##### Fedora

```sh
dnf install lua lua-devel libX11-devel libXrandr-devel readline-devel
```

##### Arch

```sh
pacman -S lua53 libx11 libxrandr readline
```

##### Termux

```sh
apt install xmake lua53 liblua53 libx11 libxrandr readline xorgproto
```

> **NOTE**: isn't your distro covered here but you know the exact packages names? Please
> send a Pull Request!

---

Now that you have the system-wide dependencies you can proceed to build and install lcfetch!

```sh
git clone --depth 1 https://github.com/NTBBloodbath/lcfetch.git \
    && cd lcfetch
```

For speeding up things, you can simply use our [XMake file](./xmake.lua).

The `lcfetch` target (the default one) will automatically download the required
third-party dependencies for building lcfetch (`Lua 5.3.6` and system dependencies like `libxrandr` if needed).

```sh
# For only building lcfetch
xmake

# For building and installing lcfetch, lcfetch will be installed at '~/.local/bin'
xmake install
```

> **IMPORTANT**: if you don't have clang installed you will need to change the compiler
> by adding `--cc=gcc` in your xmake call.

#### Troubleshooting

1. If you're getting an error related to `Xatom.h` header during compilation you will
     need to install `xorgproto` package (don't know if the package name changes in some distros tho).

## Usage

For starting lcfetch you will only need to type `lcfetch` in your terminal.

lcfetch also provides a CLI with some options.

```
$ lcfetch --help
Usage: lcfetch [OPTIONS]

OPTIONS:
    -c, --config /path/to/config    Specify a path to a custom config file
    -h, --help                      Print this message and exit
    -v, --version                   Show lcfetch version

Report bugs to https://github.com/NTBBloodbath/lcfetch/issues
```

> See `man lcfetch` for more information about usage and configurations.

### Configuring lcfetch

lcfetch uses the [Lua scripting language](https://www.lua.org/) as its configuration
language.

When installing lcfetch with `XMake` we will automatically copy the default configurations
under the default configurations path for lcfetch (`~/.config/lcfetch/config.lua`).

> **NOTE**: if you didn't installed lcfetch with `xmake install` you will need to copy the file
> by yourself under `~/.config/lcfetch` directory. The default configurations file is
> located under [config](./config) directory in the repository.

All the configuration options are self-documented and easy to understand.

## Uninstalling

For uninstalling lcfetch you can simply run `xmake uninstall` in the lcfetch directory.

This command will remove the lcfetch binary from `~/.local/bin` directory and also
the lcfetch man pages from `~/.local/share/man/man1`.

> **NOTE**: you will need to refresh your `mandb` to completely remove lcfetch man pages.

## Acknowledgements

- [dylanaraps](https://github.com/dylanaraps), creator of `neofetch`. We _will_
  extract some ASCII distribution logos from `neofetch`.
- [rxi](https://github.com/rxi), lcfetch is using his [log.c](https://github.com/rxi/log.c)
  library for providing error logs in a fancy way.
- [wmctrl](http://tripie.sweb.cz/utils/wmctrl/) creator (I don't know who is),
  I extracted the window manager name logic from his awesome program.
- All the amazing people who helped me to test stuff :heart:.

## Contribute

1. Fork it (https://github.com/NTBBloodbath/lcfetch/fork)
2. Create your feature branch (<kbd>git checkout -b my-new-feature</kbd>)
3. Commit your changes (<kbd>git commit -am 'Add some feature'</kbd>)
4. Push to the branch (<kbd>git push origin my-new-feature</kbd>)
5. Create a new Pull Request

## Todo

- [x] Support for some missing essential fields (e.g. `Packages`, `WM`)
- [x] Stop hardcoding the ASCII distro logo and accent color
- [x] Add more configurations
- [x] Strip content on non-wide terminals if we need to (like neofetch)
- [ ] Add support for MacOS (probably the last thing to be done?)
- [ ] Add support for images (we can use icat, uberzug or kitty protocols)

## License

lcfetch is distributed under [GPLv2 license](./LICENSE).
