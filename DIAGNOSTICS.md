# Build and browser diagnostics

## The browser build fails during raylib compilation with an EM_ASM C-standard error

The browser target must compile raylib and `main.c` in GNU C99 mode. `CMakeLists.txt` enables C extensions and applies `-std=gnu99` to the raylib target when CMake is run through `emcmake`. Verify that the repository root contains the current `CMakeLists.txt`, then rerun the **Build and publish browser game** workflow.

## The compiler says `DrawCircleGradient` has too many arguments

raylib 6.0 uses `DrawCircleGradient(Vector2 center, float radius, Color inner, Color outer)`. Pass the center as a `Vector2`; do not use the older x/y-style argument list.

## The site loads but the game area is blank

Open the newest **Build and publish browser game** run. Both **Compile web game** and **Publish public play link** must pass. If they pass, open the Pages URL in a fresh tab and inspect the browser console for missing `.wasm`, `.js`, or `.data` resources. The deployed `dist/` folder must contain `index.html`, `game.html`, `rift_runner.js`, `rift_runner.wasm`, and `rift_runner.data`.

## Sound doesn't start until a click

Browsers can restrict audio before a user gesture. Click inside the game or press a key to start the game/audio experience, then verify the browser isn't muted.

## Scores disappear or fail to persist

The browser uses Emscripten IDBFS at `/persist`. Check that `web/idbfs-pre.js` is passed as `--pre-js`, the IDBFS filesystem is mounted before the runtime starts, and the browser allows site storage. Score history is local to each browser profile/device.

## Desktop build fails to open a window

This project uses raylib, which needs the graphics/audio development dependencies for the operating system. CI's native build workflow shows the compiler output; it does not prove that a GUI/audio session has been play-tested.
