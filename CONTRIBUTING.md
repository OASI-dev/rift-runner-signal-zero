# Contributing

Thanks for helping with RIFT RUNNER: SIGNAL ZERO.

## Before a change

- Keep gameplay and engine code in C99 under `src/`.
- Keep third-party dependencies separate; do not copy raylib source into the repository.
- Add or update a regression test for changes to profile handling or game-state behavior.
- Do not claim graphics/audio are validated by the headless raylib stub.

## Local checks

```sh
./tools/release-check.sh
```

For a native build with raylib available:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Please describe your operating system, compiler and CMake version in bug reports. For display or audio issues, include whether the native build completed and whether the problem can be reproduced in a clean run.
