-- project
set_project("lcfetch")

-- version
set_version("0.2.0", { build = "%Y-%m-%d_%H:%M:%S" })

-- set xmake min version
set_xmakever("2.2.3")

-- '-Wall,-Wextra' equivalent
set_warnings("allextra")

-- set C standards
set_languages("c99")

-- build modes
add_rules("mode.debug", "mode.release", "mode.valgrind")
if is_mode("release") then
  -- '-Ofast' equivalent
  set_optimize("aggresive")
  -- Strip unneeded debugging symbols
  set_strip("all")
  -- Hide symbols
  set_symbols("hidden")
end

-- preprocessor variables
if is_plat("macosx") then
  add_defines("USE_PWD_SHELL", "MACOS")
else
  add_defines("USE_PWD_SHELL")
end

-- third-party dependencies
add_requires("lua >= 5.3.6", "libx11", "libxrandr", "xorgproto", "log.c")

-- headers directories
add_includedirs("src/include")

-- log.c library
package("log.c")
  set_homepage("https://github.com/rxi/log.c")
  set_description("A simple logging library implemented in C99")
  set_sourcedir(path.join(os.scriptdir(), "third-party/log.c/src"))

  on_install(function(package)
    -- Create directories
    os.mkdir(package:installdir("lib"), package:installdir("include"))
    -- Generate static log.c library
    os.run("gcc -DLOG_USE_COLOR -c -o log.o log.c")
    os.run("ar rcs liblog.a log.o")
    -- Copy libraries and headers
    os.cp("*.h", package:installdir("include"))
    os.cp("*.a", package:installdir("lib"))
  end)
  on_test(function(package)
    assert(package:check_csnippets(
      {
        test = [[
          #include <log.h>
          void test(int argc, char** argv) {
            log_info("log.c working? - %s", "yes");
          }
        ]],
      },
      { configs = {
        languages = "c99",
      } }
    ))
  end)
package_end()

-- default target
target("lcfetch")
  set_kind("binary")
  set_default(true)

  -- Source files
  add_files("src/*.c", "src/lib/*.c")

  -- Add third-party dependencies
  add_packages("lua", "libx11", "libxrandr", "xorgproto", "log.c")

  -- Add MacOS dynamic libraries that doesn't follow the 'libfoo.*' pattern
  if is_plat("macosx") then
    add_links("libXrandr.2.dylib")
  end

  -- Precompile main lcfetch header to optimize compile time
  -- set_pcheader("src/include/lcfetch.h")

  ----- HOOKS -------------------------
  after_build(function(target)
    import("core.project.config")
    local targetfile = target:targetfile()
    os.cp(targetfile, path.join(config.buildir(), path.filename(targetfile)))
    cprint("${bright green}[INFO]:${clear} Produced binary location: %s", targetfile)
  end)
  after_install(function(target)
    cprint(
      "${bright green}[INFO]:${clear} please run 'mandb -pu' if your system uses mandb to update man pages database"
    )
  end)
  on_install(function(target)
    import("core.project.config")
    -- Setup directories
    local local_dirs = { "bin", "share/man/man1" }
    for _, local_dir in ipairs(local_dirs) do
      os.run(string.format("mkdir -p %s/.local/%s", os.getenv("HOME"), local_dir))
    end

    -- Install produced binary
    cprint(
      "${bright green}[ 30%%]:${clear} installing lcfetch under %s",
      os.getenv("HOME") .. "/.local/" .. local_dirs[1] .. " ..."
    )
    local targetfile = target:targetfile()
    os.run(
      string.format(
        "install %s %s",
        path.join(config.buildir(), path.filename(targetfile)),
        os.getenv("HOME") .. "/.local/bin/" .. path.filename(targetfile)
      )
    )

    -- Install default configurations
    local skipped_config_copy = true
    local config_dir = string.format("%s/.config/lcfetch", os.getenv("HOME"))
    os.run(string.format("mkdir -p %s", config_dir))
    if not os.exists(string.format("%s/config.lua", config_dir)) then
      cprint("${bright green}[ 50%%]:${clear} copying default configs to %s", config_dir)
      os.cp("config/sample.lua", string.format("%s/config.lua", config_dir))
      skipped_config_copy = false
    end

    -- Install documentation
    cprint(
      "${bright green}[ %d%%]:${clear} copying man pages to %s",
      skipped_config_copy and 77 or 75,
      os.getenv("HOME") .. "/local/" .. local_dirs[2] .. " ..."
    )
    os.cp("man/lcfetch.1", string.format("%s/.local/%s", os.getenv("HOME"), local_dirs[2]))
  end)
  on_uninstall(function(_)
    local bin_dir = os.getenv("HOME") .. "/.local/bin"
    local man_dir = os.getenv("HOME") .. "/.local/share/man/man1"
    cprint("${bright green}[ 77%%]:${clear} uninstalling lcfetch ...")
    os.rm(bin_dir .. "/lcfetch", man_dir .. "/lcfetch.1")
  end)

-- Format source code
target("fmt")
  set_default(false)

  on_run(function(_)
    cprint("${bright green}[ 56%%]:${clear} formatting lcfetch source code ...")
    -- os.run("clang-format -style=file --sort-includes -i src/**/*.c src/include/**/*.h")
    local inc_files = os.files("$(projectdir)/src/include/**/*.h")
    local src_files = table.join(
      os.files("$(projectdir)/src/*.c"),
      os.files("$(projectdir)/src/**/*.c")
    )
    table.join2(src_files, inc_files)
    for _, filepath in ipairs(src_files) do
      os.run("clang-format -style=file --verbose --sort-includes -i " .. filepath)
    end
    cprint("${bright green}[100%%]: format ok!${clear}")
  end)
  on_uninstall(function(_)
    return nil
  end)

-- Generate man pages
target("docs")
  set_default(false)

  on_run(function(_)
    cprint("${bright green}[ 77%%]:${clear} Generating man pages ...")
    os.run("pandoc man/lcfetch.1.md -s -t man -o man/lcfetch.1")
    cprint("${bright green}[100%%]: docs ok!${clear}")
  end)
  on_uninstall(function(_)
    return nil
  end)

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
