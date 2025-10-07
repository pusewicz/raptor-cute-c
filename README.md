# 🚀 Raptor

A classic arcade-style shoot 'em up that brings retro gaming action to modern systems! Built with [Cute Framework](https://github.com/RandyGaul/cute_framework) and written in pure C23, Raptor features smooth gameplay, hot-reloadable game logic for rapid development, and beautiful pixel art.

![Screenshot](./assets/screenshot.gif)

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
```

Now edit `src/game/game.c` and watch your changes appear instantly in the running game!

## 🙏 Credits

This project wouldn't be possible without the amazing work of:

- **Sprites**: [GrafxKid](https://grafxkid.itch.io) - Gorgeous pixel art assets
- **Sounds**: [TheMaverickProgrammer](https://github.com/TheMaverickProgrammer) - Crisp sound effects
- **Framework**: [Cute Framework](https://github.com/RandyGaul/cute_framework) by Randy Gaul - Powerful and lightweight C game framework

## 📝 License

See the [LICENSE](LICENSE) file for details.

---

Made with ❤️ and C23. Pull requests welcome!

