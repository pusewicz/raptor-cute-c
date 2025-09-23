# Raptor

Raptor is a small shoot em' up game made with Cute Framework and written in C.

<img width="652" height="1104" alt="Screenshot 2025-09-24 at 00 24 52" src="https://github.com/user-attachments/assets/5a00f416-9c09-4ca1-a157-9b59b961545d" />

## Controls

- Arrow Keys/WASD: Move the ship
- Space/Mouse Button 1: Shoot

## Building

### Prerequisites

- [CMake](https://cmake.org/)

### macOS

1. Install dependencies using Homebrew:
   ```sh
   brew install cmake
   ```
2. Clone the repository:
   ```sh
   git clone https://github.com/pusewicz/raptor-cute-c.git
   cd raptor-cute-c
   ```
3. Build and run the project:
   ```sh
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   cd build
   ./Raptor
   ```
