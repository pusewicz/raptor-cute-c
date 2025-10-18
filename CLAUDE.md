# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Building the project
```bash
rake compile
```
This creates a debug build in `build/debug/` using CMake with Ninja generator.

### Running the project
```bash
rake run
```
Runs the built executable from the build directory. Will fail if build hasn't been run first.

### Auto-compile on file changes
```bash
rake watch
```
Watches source files for changes and automatically recompiles the project causing the running game process to reload the gamelib.

## Architecture

This is a C23 game called "Raptor" built on top of the Cute Framework (https://github.com/RandyGaul/cute_framework).

### Core Structure

The project uses a **hot-reloadable game library architecture**:

- **Main executable** (`src/main.c`): Entry point that loads the game library and runs the main loop
- **Game library** (`src/game/game.c`): Contains the actual game logic, compiled as a shared library (`gamelib`)
- **Platform layer** (`src/platform/`): Abstracts platform-specific functionality
- **Engine layer** (`src/engine/`): Core engine utilities and data structures

### Key Components

1. **Memory Management**: Custom arena allocators with three tiers:
   - Permanent arena (64MB) - long-lived allocations
   - Stage arena (64MB) - level/scene-specific allocations
   - Scratch arena (64MB) - temporary allocations

2. **Game Library Interface**: Function pointers for hot reloading:
   - `game_init()` - Initialize game state
   - `game_update()` - Main game loop logic
   - `game_render()` - Rendering
   - `game_shutdown()` - Cleanup
   - `game_state()` - Returns current game state pointer
   - `game_hot_reload()` - Restores game state after library reload

3. **Platform Abstraction**: Platform-specific code isolated in `platform/` directory with functions for:
   - Memory allocation/deallocation
   - Game library loading/reloading/unloading
   - Frame timing (begin/end frame)

4. **Coordinate System**: Uses a centered Cartesian coordinate system:
   - Origin `(0, 0)` is at the **center** of the screen
   - Positive X goes **right**, negative X goes **left**
   - Positive Y goes **up**, negative Y goes **down**
   - X range: `[-canvas_width/2, +canvas_width/2]`
   - Y range: `[-canvas_height/2, +canvas_height/2]`
   - Default canvas size: 180×320 (portrait orientation)
   - Example: Top-right corner is at `(90, 160)`, bottom-left is at `(-90, -160)`

### Dependencies

- **Cute Framework**: Main framework dependency, fetched automatically via CMake FetchContent
- **Standard**: C23 standard with strict prototypes enabled

### Build Configuration

- Debug builds include runtime checks (`/RTCcsu` on MSVC, `-g` on GCC/Clang)
- Release builds strip debug info
- Assets folder is symlinked to build directory during build
- Uses Ninja build system for faster incremental builds

### Instructions

- Always format and compile after changes: `rake format compile`.
