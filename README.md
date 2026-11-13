# Tanks Game Engine

A C++17 8-bit-style tanks game with a console version and an optional SFML
sprite renderer. Both versions read the same game engine state.

## Build and run

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

Run `build/tanks_cli` on Linux/macOS or `build/Debug/tanks_cli.exe` on Windows
with a multi-configuration generator. With a single-configuration Windows
generator, the executable is `build/tanks_cli.exe`.

For the graphical version, enable SFML when configuring CMake:

```text
cmake -S . -B build-render -DTANKS_BUILD_RENDERER=ON
cmake --build build-render --config Release --target tanks_render
```

On Windows, run `build-render/Release/tanks_render.exe`. CMake downloads SFML
2.6.2 when needed. The pixel textures are created in code, so no image files
are needed.

## Controls

| Input | Action |
| --- | --- |
| Enter | Start from Ready; wait one simulation tick during battle |
| W, A, S, D | Move up, left, down, right |
| I, J, K, L | Turn up, left, down, right without moving |
| F | Fire in the current direction |
| R | Restart after a win or loss |
| Q | Quit |

On Windows, press one key for each action; you do not need to press Enter after it.
On Linux/macOS, type a command and press Enter.
In the graphical window, hold WASD to keep moving, use arrow keys or IJKL to
aim, F or Space to fire, and Esc to quit.

`P` is the player, `B` a bot with 2 HP, `b` a damaged bot with 1 HP,
`#` a wall with 2 durability, `+` a damaged wall with 1 durability,
`~` water, `*` a shot, and `.` floor. The HUD shows when the player's
shot is ready or reloading. Tanks cannot enter walls, water, or another tank's cell.
Shots pass over water and can damage walls and tanks.
Tanks move one cell per simulation tick. Projectiles move two cells per tick,
checking collisions in each cell so they cannot pass through a wall or tank.

## Design notes

`GameObject` is the base class for tiles, walls, tanks, and projectiles.
`Damageable` gives walls and tanks the same damage contract. A `GameMap`
owns three layers, and each layer owns its objects with `std::unique_ptr`.
`GameMap::generate(seed)` is a factory function that returns a complete map.
The engine has no dependency on `Renderer`. The graphical app is split into
`GameDisplayData` (map objects to console-style symbols and status),
`PixelTextures` (creates the small images), `SpriteRenderer` (draws them),
`WindowInput` (keyboard events), and `app/graphics/main.cpp` (window loop).

## Game rules

Each new battle creates a deterministic 13 x 13 map from a seed. The player
starts with 3 health, each of three bots with 2 health, and walls with 2
durability. A shot deals 1 damage. Bots fire when they have a clear row or
column toward the player and patrol otherwise. The engine updates at a fixed
0.25-second step and resolves all shots before checking for victory or defeat.
