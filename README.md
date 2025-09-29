# Raptor

Raptor is a small shoot em' up game made with Cute Framework and written in C.

https://github.com/user-attachments/assets/64c2d78a-bbe7-4369-bc9e-3f9fd9a70b36

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
