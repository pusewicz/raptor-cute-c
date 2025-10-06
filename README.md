# Raptor

Raptor is a small shoot em' up game made with Cute Framework and written in C.

https://github.com/user-attachments/assets/08e2b5d0-6ecd-49f0-ad4d-f82f6726f973

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

### Credits

#### Sprites

  - [GrafxKid](https://grafxkid.itch.io)

#### Sounds

  - @TheMaverickProgrammer

