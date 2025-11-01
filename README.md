# 🚀 Raptor

A classic arcade-style shoot 'em up that brings retro gaming action to modern systems! Built with [Cute Framework](https://github.com/RandyGaul/cute_framework) and written in pure C23, Raptor features smooth gameplay, hot-reloadable game logic for rapid development, and beautiful pixel art.

![Screenshot](./assets/screenshot.gif)

## 🕹️ Play on Itch.io

You can play the game directly on [itch.io](https://itch.io/embed-upload/15297409?color=333333).

## ✨ Features

- **Fast-paced arcade action** with smooth scrolling and responsive controls
- **Hot-reloadable game library** - modify code and see changes instantly without restarting
- **Custom memory management** with arena allocators for optimal performance
- **Beautiful pixel art** from renowned artist GrafxKid
- **Pure C23** codebase showcasing modern C development practices

## Controls

- Arrow Keys/WASD: Move the ship
- Space/Mouse Button 1: Shoot

## 🎮 Quick Start

### Prerequisites

- [CMake](https://cmake.org/) 3.15 or higher
- A C23-compatible compiler (GCC, Clang, or MSVC)
- [Ninja build system](https://ninja-build.org/) (recommended)

### macOS

```sh
# Install dependencies
brew install cmake ninja

# Clone and enter the project
git clone https://github.com/pusewicz/raptor-cute-c.git
cd raptor-cute-c

# Build and play!
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build
./Raptor
```

### 🔥 Developer Mode (Hot Reload)

Want to modify the game while it's running? Use the watch mode:

```sh
rake watch  # Auto-recompiles on file changes
rake run    # Run the game in another terminal
rake build  # Build the game in debug mode with hot-reload enabled
```

Now edit `src/game/game.c` and watch your changes appear instantly in the running game!

### 🧪 Testing

The project includes a comprehensive test suite for the hot reload functionality:

```sh
# Run all tests
rake test

# Or use the test runner script
./tests/run_tests.sh

# Build with tests enabled manually
cmake -B build -DBUILD_TESTS=ON
cmake --build build
./build/hot_reload_tests
```

The test suite verifies:
- Platform layer memory management
- Library loading/unloading cycles
- State preservation across reloads
- Crash resistance during hot reload
- Memory integrity and leak prevention

See [tests/README.md](tests/README.md) for detailed documentation.

## 🙏 Credits

This project wouldn't be possible without the amazing work of:

- **Sprites**: [GrafxKid](https://grafxkid.itch.io) - Gorgeous pixel art assets
- **Sounds**: [TheMaverickProgrammer](https://github.com/TheMaverickProgrammer) - Crisp sound effects
- **Framework**: [Cute Framework](https://github.com/RandyGaul/cute_framework) by Randy Gaul - Powerful and lightweight C game framework

## 📝 License

See the [LICENSE](LICENSE) file for details.

---

Made with ❤️ and C23. Pull requests welcome!

