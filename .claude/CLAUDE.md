# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**RD_DSP** is a portable C++20 DSP library. Goal: a JUCE-free, dependency-free core that compiles on every major C++ DSP target — desktop (CMake/standalone), JUCE plugins, AUv3, and WebAssembly. Test-driven via Catch2 v3.

The sister project `C:/REPOS/PLUGIN_PROJECTS/RD` is a JUCE plugin that uses similar primitives (e.g. `juce::AudioBuffer`); RD_DSP types are conceptually equivalent but framework-agnostic. **Do not pull JUCE or any other library into the core**. Reference RD only for layout conventions and naming.

## Hard rules

- **Notes/instructions live under `.claude/`. Do not create or edit `README.md`.**
- **Core lib (`SOURCE/`) has zero third-party deps.** No JUCE, no Boost, no `<filesystem>` (WASM/emscripten quirks), no `std::thread`, no globals, no platform headers. Only the C++ standard library.
- **No allocations on the audio hot path.** Anything that runs inside a `processBlock` equivalent must be allocation-free. Sizing/init goes in `prepare()`-style methods.
- **No exceptions in DSP hot path.** Exceptions are fine in setup/factory methods but must not propagate through real-time code (incompatible with AUv3 and disabled in many WASM builds).
- **Allman braces** (opening brace on its own line at the parent's indent). Match the surrounding file only if it is overwhelmingly non-Allman.
- **Helper scripts and platform integration live outside `SOURCE/`** — `HELPER_SCRIPTS/` for Python build drivers, `STANDALONE/` for the CLI demo. Never let platform code leak into the core library.

## Build commands

```bash
# From repo root — Python drivers (cross-platform)
python HELPER_SCRIPTS/build_tests.py             # build Tests target (Debug)
python HELPER_SCRIPTS/build_tests.py --run       # build then run Catch2
python HELPER_SCRIPTS/build_standalone.py        # build CLI demo
python HELPER_SCRIPTS/rebuild_all.py             # wipe BUILD/, rebuild both
python HELPER_SCRIPTS/regenSource.py             # regen CMAKE/{SOURCES,TESTS}.cmake

# Or raw CMake
cmake -B BUILD -DBUILD_TESTS=ON -DBUILD_STANDALONE=ON
cmake --build BUILD --target Tests
cd BUILD && ctest
```

All build scripts call `regenSource.py` automatically before configuring. After adding/removing `.h`/`.cpp` files manually, re-run `regenSource.py` (or just run any build script).

### CMake options

- `BUILD_TESTS` (default ON) — Catch2 v3 fetched via FetchContent, builds `Tests` exe.
- `BUILD_STANDALONE` (default ON) — builds `RD_DSP_Standalone` CLI exe linking the static lib.

### Running a single test

After building, from `BUILD/` (or `BUILD/Debug` on MSVC multi-config):

```bash
./Tests --list-tests              # enumerate
./Tests "[RD_Buffer]"             # by tag
./Tests "exact test case name"    # by name
ctest -R <regex>                  # ctest filter
```

## Layout

| Path | Purpose |
|------|---------|
| `SOURCE/` | Core library. Zero deps. Public headers + impl. |
| `TESTS/` | Catch2 tests. `test_main.cpp` is the custom session entry point. |
| `STANDALONE/` | CLI demo executable (`main.cpp`) that links the static lib. |
| `CMAKE/SOURCES.cmake` | Auto-generated source list (do not edit by hand — run `regenSource.py`). |
| `CMAKE/TESTS.cmake` | Auto-generated test source list. |
| `HELPER_SCRIPTS/` | Python build drivers. Cross-platform; no shell scripts. |
| `BUILD/` | Out-of-source build dir. Gitignored. |
| `VERSION.txt` | Single source of truth for project version. |

`CMAKE/*.cmake` files are generated artifacts — edit `regenSource.py` if the discovery rules need to change, not the `.cmake` files themselves.

## Targets

- `RD_DSP` — static library, the core DSP code. Links nothing. Public include dir is `SOURCE/`.
- `Tests` — Catch2 v3 executable, linked to `RD_DSP` and `Catch2::Catch2`. Discovered via `catch_discover_tests` so `ctest` sees individual cases.
- `RD_DSP_Standalone` — CLI exe. Placeholder demo for ad-hoc experiments outside any plugin framework.

## Testing convention

Test files live directly under `TESTS/` for now (e.g. `TESTS/test_RD_Buffer.cpp`). When the suite grows, mirror RD's pattern: standalone-component tests get their own subdir, processor tests live under `TESTS/PROCESSORS/<NAME>/`.

Tag every `TEST_CASE` with a `[ComponentName]` tag so `./Tests "[Tag]"` works. Keep one test file per logical unit.

## Portability notes

This library must compile against multiple toolchains:

- **Desktop / CMake** — primary dev target.
- **JUCE plugins** — added as a CMake subdirectory; `target_link_libraries(YourPlugin PRIVATE RD_DSP)`.
- **AUv3** — Apple's app-extension sandbox bans many things. No file I/O in the lib, no global mutable state, no `dlopen`.
- **WebAssembly (emscripten)** — no threads (unless explicitly enabled), no `<filesystem>`, exceptions disabled by default, no `mmap`, no `dlopen`.

When in doubt, lean toward the most restrictive target (WASM ∩ AUv3). Anything platform-specific belongs in the consuming app, not in `SOURCE/`.

## Working with this repo

- TDD is the default. Write the failing Catch2 test first, confirm red, implement minimum code to go green, refactor.
- The `RD` repo (sibling dir) is a reference for naming and layout — not a dependency. Do not `#include` from it.
- Reference RD's JUCE submodule path only when discussing what RD does; never when generating code here.
