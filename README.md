<div align="center">

# lcfetch

![License](https://img.shields.io/github/license/NTBBloodbath/lcfetch?style=flat-square)

![lcfetch demo](https://user-images.githubusercontent.com/36456999/129203250-efb94aac-d301-4126-a4ae-b8b59f3ddc40.png)

</div>

A fast and easy to configure alternative to [neofetch](https://github.com/dylanaraps/neofetch)
written in C and configured using Lua (still in a very early stage)!

> **IMPORTANT**: I'm a newbie using C and the Lua C API so if you see something wrong
> please let me know!

## Installation

You can grab the latest release from our [releases](https://github.com/NTBBloodbath/lcfetch/releases/latest)
or if you want to live in the bleeding-edge with the latest features you can [build from source](#building-from-source).

### Building from source

#### Installing dependencies

> **NOTE**: lcfetch was tested only with Lua 5.3+ but should work as expected with earlier
> Lua versions.

##### Ubuntu

```sh
apt install lua5.3 liblua5.3-dev libx11-dev libreadline-dev
```

##### Fedora

```sh
dnf install lua lua-devel libX11-devel readline-devel
```

> **NOTE**: isn't your distro covered here but you know the exact packages names? Please
> send a Pull Request!

---

Now that you have the dependencies you can proceed to build and install lcfetch!

```sh
git clone --depth 1 --recursive-submodules https://github.com/NTBBloodbath/lcfetch.git \
    && cd lcfetch
```

For speeding up things, you can simply use our [Makefile](./Makefile).

```sh
# For only building lcfetch, produced binary will be located at 'bin/lcfetch'
make build

# For building and installing lcfetch, lcfetch will be installed at '~/.local/bin'
make install
```

> **IMPORTANT**: if you don't have clang installed you will need to change the `CC` variable
> value by adding `CC=gcc` in your make call.

#### Troubleshooting

1. If you're getting errors regarding to missing Lua headers (e.g. `lauxlib.h`)
    change the `LUA_INC_DIR` variable when running `make` to match your Lua `include/` path.

2. If you're getting errors regarding to `ld` and `-llua` you will need to change the
    `LUA_LIB_NAME` variable when running `make` to match with your `liblua*.so` file, e.g.
    `-llua5.3` for `liblua5.3.so`.

## Usage

For starting `lcfetch` you will only need to type `lcfetch` in your terminal.

`lcfetch` also provides a CLI with some options.

```
$ lcfetch --help
Usage: lcfetch [OPTIONS]

OPTIONS:
    -c, --config /path/to/config    Specify a path to a custom config file
    -h, --help                      Print this message and exit
    -v, --version                   Show lcfetch version

Report bugs to https://github.com/NTBBloodbath/lcfetch/issues
```

### Configuring lcfetch

`lcfetch` uses the [Lua scripting language](https://www.lua.org/) as its configuration
language.

When installing `lcfetch` with `make` we will automatically copy the default configurations
under the default configurations path for `lcfetch` (`~/.config/lcfetch/config.lua`).

> **NOTE**: if you didn't installed `lcfetch` with `make` you will need to copy the file
> by yourself under `~/.config/lcfetch` directory. The default configurations file is
> located under [config](./config) directory in the repository.

All the configuration options are self-documented and easy to understand.

## Acknowledgements

- [dylanaraps](https://github.com/dylanaraps), creator of `neofetch`. We _will_
  extract the ASCII distribution logos from `neofetch`.
- [rxi](https://github.com/rxi), `lcfetch` is using his [log.c](https://github.com/rxi/log.c)
  library for providing error logs in a fancy way.

## Contribute

1. Fork it (https://github.com/NTBBloodbath/lcfetch/fork)
2. Create your feature branch (<kbd>git checkout -b my-new-feature</kbd>)
3. Commit your changes (<kbd>git commit -am 'Add some feature'</kbd>)
4. Push to the branch (<kbd>git push origin my-new-feature</kbd>)
5. Create a new Pull Request

## Todo

- [ ] Support for some missing fields (e.g. `Packages` and `DE`/`WM`)
- [ ] Stop hardcoding the ASCII distro logo and accent color
- [ ] Add more configurations
- [ ] Strip content on non-wide terminals if we need to (like neofetch)

## License

`lcfetch` is distributed under [GPLv2 license](./LICENSE).
