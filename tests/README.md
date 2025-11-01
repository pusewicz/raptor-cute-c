# Hot Reload Test Suite

This directory contains comprehensive tests for the hot code reload functionality of the Raptor game engine.

## Overview

The test suite verifies that the hot reload system works correctly and doesn't cause crashes. It tests state preservation, library loading/unloading, and the complete reload workflow.

## Test Structure

### Test Framework (`test_framework.h`)
A lightweight, zero-dependency test framework with the following features:
- Simple macro-based test definitions
- Clear pass/fail reporting
- Assertion helpers (equality, null checks, string comparison)
- Test statistics and summary

### Test Suites

#### 1. Platform Layer Tests (`test_platform.c`)
Tests the low-level platform abstraction layer:
- ✅ Memory allocation and deallocation
- ✅ Large memory allocations (64MB arenas)
- ✅ Platform struct creation and function pointers
- ✅ Game library loading and unloading
- ✅ Multiple load/unload cycles (leak detection)

#### 2. Hot Reload Integration Tests (`test_hot_reload.c`)
Tests the complete hot reload workflow:
- ✅ Basic hot reload cycle (load → save state → unload → reload → restore)
- ✅ State pointer preservation across reloads
- ✅ Function pointer updates after reload
- ✅ Multiple consecutive reloads (3+ cycles)
- ✅ Error handling (reload without initialization)
- ✅ All library functions loaded correctly
- ✅ GameState validity after reload

#### 3. State Preservation Tests (`test_state_preservation.c`)
Tests that game data survives reloads:
- ✅ GameState structure integrity
- ✅ Game variables (score, lives, game over flag)
- ✅ Canvas size and scale preservation
- ✅ Platform pointer preservation
- ✅ Memory arena pointers and sizes
- ✅ State validity across 5+ reloads
- ✅ Player data preservation

## Building and Running

### Prerequisites
- CMake 3.22 or higher
- C23-compatible compiler (GCC 13+, Clang 14+)
- Ninja build system
- OpenGL development libraries

### Build with Tests Enabled

```bash
# Using CMake directly
mkdir -p .build/Debug-reloadable
cd .build/Debug-reloadable
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DRELOADABLE=ON -DBUILD_TESTS=ON ../..
ninja

# Using Rake (if available)
rake test
```

### Run Tests

```bash
# From build directory
./.build/Debug-reloadable/hot_reload_tests

# Using Rake
rake test
```

### Disable Tests

```bash
cmake -DBUILD_TESTS=OFF ../..
```

## Expected Output

When all tests pass, you should see output like:

```
========================================
  Hot Reload Test Suite
========================================

=== Running Test Suite: test_platform ===
  Running: test_platform_allocate_memory
    ✓ PASS
  Running: test_platform_free_memory
    ✓ PASS
  Running: test_platform_large_allocation
    ✓ PASS
  Running: test_platform_struct_creation
    ✓ PASS
  Running: test_load_game_library
    ✓ PASS
  Running: test_unload_game_library
    ✓ PASS
  Running: test_multiple_load_unload_cycles
    ✓ PASS

=== Running Test Suite: test_hot_reload ===
  Running: test_basic_hot_reload
    ✓ PASS
  Running: test_state_pointer_preservation
    ✓ PASS
  Running: test_function_pointer_updates
    ✓ PASS
  Running: test_multiple_consecutive_reloads
    ✓ PASS
  Running: test_reload_without_init
    ✓ PASS
  Running: test_all_library_functions_loaded
    ✓ PASS
  Running: test_reload_preserves_state_validity
    ✓ PASS

=== Running Test Suite: test_state_preservation ===
  Running: test_game_state_structure_preserved
    ✓ PASS
  Running: test_game_variables_survive_reload
    ✓ PASS
  Running: test_canvas_size_preserved
    ✓ PASS
  Running: test_platform_pointer_preserved
    ✓ PASS
  Running: test_memory_arenas_preserved
    ✓ PASS
  Running: test_state_validity_after_multiple_reloads
    ✓ PASS
  Running: test_player_data_preserved
    ✓ PASS

=== Test Summary ===
Total:  21
Passed: 21
Failed: 0
Success Rate: 100.0%

✅ ALL TESTS PASSED
```

## What These Tests Verify

### Crash Prevention
- No segmentation faults during library load/unload cycles
- No memory leaks from repeated reloads
- No double-free errors
- Safe handling of NULL pointers

### State Integrity
- All game data survives reloads (score, lives, player position)
- Memory arenas remain valid and accessible
- Platform function pointers remain accessible
- Canvas configuration is preserved

### Reload Workflow
- Library loads with all required functions
- State pointer remains stable across reloads
- Function pointers are updated correctly
- Multiple consecutive reloads work without issues
- GameState validation passes after reload

### Memory Safety
- 64MB arena allocations succeed
- Memory can be written to and read from after reload
- No corruption of memory arena pointers
- Platform allocator functions remain valid

## Architecture Tested

```
┌─────────────────────────────────────────┐
│         Main Executable                 │
│  ┌────────────────────────────────┐    │
│  │   Platform Layer               │    │
│  │  - Memory allocation           │    │
│  │  - Library loading/unloading   │    │
│  └────────────────────────────────┘    │
└──────────────┬──────────────────────────┘
               │ Loads/Reloads
               ▼
┌─────────────────────────────────────────┐
│      Game Library (gamelib.so)          │
│  ┌────────────────────────────────┐    │
│  │   Game State                   │    │
│  │  - Preserved across reloads    │    │
│  │  - Memory arenas              │    │
│  │  - Game data (score, lives)   │    │
│  │  - Player, enemies, bullets   │    │
│  └────────────────────────────────┘    │
│                                         │
│  Interface Functions:                  │
│  - game_init()                         │
│  - game_update()                       │
│  - game_render()                       │
│  - game_state() → Returns GameState*   │
│  - game_hot_reload(GameState*)         │
│  - game_shutdown()                     │
└─────────────────────────────────────────┘
```

## Integration with CI/CD

These tests can be integrated into continuous integration pipelines:

```yaml
# Example GitHub Actions
- name: Build and test
  run: |
    cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DRELOADABLE=ON -DBUILD_TESTS=ON -B build
    cmake --build build
    ./build/hot_reload_tests
```

## Troubleshooting

### Tests fail with "Library not loaded"
Ensure the game library is built before running tests:
```bash
cmake --build . --target gamelib
```

### Tests fail with "Could NOT find OpenGL"
Install OpenGL development libraries:
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev

# Fedora
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# macOS
# OpenGL is included with Xcode Command Line Tools
```

### Tests crash with segmentation fault
This indicates a real bug in the hot reload system. Check:
1. GameState structure alignment and packing
2. Memory arena initialization
3. Platform function pointer validity
4. Null pointer checks in hot_reload function

## Future Enhancements

Potential additions to the test suite:
- [ ] Stress tests (1000+ reloads)
- [ ] Concurrent reload tests (threading)
- [ ] Asset reload verification (sprites, audio)
- [ ] Coroutine state preservation tests
- [ ] Benchmark reload performance
- [ ] Memory leak detection with Valgrind
- [ ] Platform-specific tests (Windows, macOS, Linux)

## Contributing

When adding new hot reload features:
1. Add corresponding tests to verify the feature
2. Update this README with test descriptions
3. Ensure tests pass before committing
4. Run tests with `rake test` as part of development workflow
