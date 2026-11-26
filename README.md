# Keyple Plugin PC/SC C++ Library

## Overview

The **Keyple Plugin PC/SC C++ Library** is an add-on to allow an application using Keyple to interact with PC/SC readers.

## Documentation & Contribution Guide

The full documentation, including the **user guide**, **download information** and **contribution guide**, is available on the Keyple website [keyple.org](https://keyple.org).

## API documentation

API documentation & class diagram is available online: [docs.keyple.org/keyple-plugin-pcsc-cpp-lib](https://docs.keyple.org/keyple-plugin-cpp-java-lib)

## Examples

Examples of implementation are available in the following repository: [github.com/eclipse/keyple-cpp-example](https://github.com/eclipse/keyple-cpp-example)

## Building with Conan

This repository is packaged as a [Conan](https://conan.io/) recipe (`conanfile.py`), providing the
`Keyple::Plugin::Pcsc` CMake target as `keyple-plugin-pcsc-cpp-lib`. It requires `keyple-common-cpp-api/2.0.2`,
`keyple-plugin-cpp-api/2.3.2` and `keyple-util-cpp-lib/2.4.1` (all declared with `transitive_headers=True`, since
this library's public headers include their headers directly), resolved via Conan instead of `FetchContent`.
Downstream keyple recipes should consume this package the same way, with
`self.requires("keyple-plugin-pcsc-cpp-lib/2.6.3")`. The `shared` option (default `True`) controls whether a shared
or static library is built, and `fPIC` is available for static, non-Windows builds. PC/SC itself remains a system
library outside Conan's package graph (the PC/SC framework on macOS, `libpcsclite-dev` on Linux, `winscard.lib`
on Windows) — see `package_info()` for how it's declared to downstream consumers.

This library has no unit test suite (PC/SC readers require real hardware), so there is no `with_tests` option.

### Local development

Use `conan build` to configure/compile in-tree, in a regular `build/` folder you can inspect, rerun and debug
against (and which generates a `CMakeUserPresets.json` for IDE integration). The build type defaults to `Release`;
pass `-s build_type=Debug` for a debug build, which lands in its own `build/Debug` subfolder alongside `build/Release`:

```bash
conan build . --build=missing
conan build . -s build_type=Debug --build=missing
```

### Packaging to the local cache

Use `conan create` to export the recipe and package to your local Conan cache, for consumption by other
recipes/repos (no remote upload involved):

```bash
# Build and export the package to the local cache
conan create . --build=missing

# Package a static build instead (default is shared)
conan create . -o shared=False --build=missing
```

### Conan profile

Builds use your default Conan profile. If you don't have one yet, create it with:

```bash
conan profile detect
```

## About the source code

The code is built with **CLang / GCC / MSVC** in order to address a wide range of applications.
