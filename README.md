# cpp23_starter_coodle

Bare-minimum C++23 scaffold for quickly verifying ideas. Not for everyone — but it works for me.

## Requirements
- cmake 3.20+
- Ninja
- A C++23-capable compiler (e.g., clang++ 17+ or g++ 13+)

## Quick Start
- Build: `./build.sh`
- Run: `./run.sh [args]`
- Build output: `./build/` (also exports `compile_commands.json`)

## Layout
- `CMakeLists.txt` — minimal CMake config (C++23, single target)
- `src/main.cpp` — your playground
- `build.sh` — configure + build with Ninja
- `run.sh` — run the built binary

## Notes
- Edit `src/main.cpp` and rebuild to iterate.
- Expand by adding sources and updating `add_executable` in `CMakeLists.txt`.

