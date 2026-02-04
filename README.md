# ByteForge

A high-performance bump-pointer arena allocator for C++20.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [Performance](#performance)
- [Design](#design)
- [Limitations](#limitations)
- [Platform Support](#platform-support)
- [License](#license)

## Overview

ByteForge is a linear (bump-pointer) arena allocator that provides fast, contiguous memory allocation with O(1) bulk deallocation. Memory is allocated by advancing a pointer through fixed-size blocks; individual objects cannot be freed. When a phase of work completes, a single `reset()` call reclaims all memory instantly.

This allocation strategy is ideal for:

- **Compilers and parsers** — AST nodes share a common lifetime
- **Game engines** — per-frame scratch allocators
- **Request handlers** — allocate freely, reset between requests
- **Graph algorithms** — temporary node/edge storage

## Features

- **O(1) allocation** — bump pointer advance; no free-list traversal
- **O(1) reset** — rewind offsets; no per-object cleanup
- **Automatic growth** — new blocks allocated via `mmap(2)` when needed
- **Type-safe construction** — `store<T>(args...)` handles alignment and placement-new
- **Zero dependencies** — header-only templates, minimal `.cpp` implementation
- **Modern C++20** — move semantics, `constexpr`-friendly design

## Installation

### CMake (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(
  byteforge
  GIT_REPOSITORY https://github.com/youruser/byteforge.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(byteforge)

target_link_libraries(your_target PRIVATE byteforge)
```

### Manual

```bash
git clone https://github.com/youruser/byteforge.git
cd byteforge
mkdir build && cd build
cmake ..
cmake --build .
```

Link against the `byteforge` library and add `include/` to your include path.

## Quick Start

```cpp
#include "byteforge/bundle.hpp"

int main() {
    byteforge::Bundle arena(4096);  // 4 KB initial block

    // Allocate and construct objects
    int* x = arena.store<int>(42);
    auto* str = arena.store<std::string>("hello, arena");

    // Use allocated memory...
    std::cout << *x << " " << *str << "\n";

    // Bulk deallocation — all pointers invalidated
    arena.reset();
}
```

## API Reference

### `byteforge::Bundle`

The primary user-facing arena allocator.

#### Constructor

```cpp
explicit Bundle(std::size_t initial_block_size);
```

Creates an arena with one block of the specified size (in bytes). Blocks are allocated via `mmap(2)`.

#### `store<T>(Args&&... args) -> T*`

```cpp
template <typename T, typename... Args>
T* store(Args&&... args);
```

Allocates `sizeof(T)` bytes with `alignof(T)` alignment, then constructs a `T` in-place via placement-new. Returns `nullptr` only if the system fails to allocate a new block.

#### `reset()`

```cpp
void reset();
```

Rewinds all blocks to offset zero. All previously returned pointers become **invalid**. Does not call destructors.

#### `used() -> std::size_t`

```cpp
std::size_t used();
```

Returns total bytes currently allocated across all blocks.

#### `capacity() -> std::size_t`

```cpp
std::size_t capacity();
```

Returns total bytes reserved across all blocks.

---

### `byteforge::Block`

Low-level building block. Most users should prefer `Bundle`.

```cpp
Block(std::uint8_t* buffer, std::size_t size);

void* allocate(std::size_t size, std::size_t alignment);
void  reset();

std::size_t capacity() const;
std::size_t used() const;
```

`Block` operates on externally-owned memory. It does not allocate or free the underlying buffer.

## Performance

ByteForge significantly outperforms `new`/`delete` for batch allocation workloads. The included benchmark (`examples/benchmark.cpp`) allocates 1,000,000 objects across 50 frames:

| Allocator       | Time (ms) | Speedup |
|-----------------|-----------|---------|
| `new`/`delete`  | ~350      | 1.0x    |
| ByteForge arena | ~15       | ~23x    |

*Results vary by platform and compiler. Run `./benchmark` to measure on your system.*

### Why it's fast

1. **No metadata overhead** — bump allocators don't store per-object headers
2. **No free-list search** — allocation is a pointer increment
3. **Cache-friendly** — objects are allocated contiguously
4. **Bulk reset** — single offset rewind vs. N deallocations

## Design

### Memory Layout

```
Block 0                Block 1                Block N
┌──────────────────┐   ┌──────────────────┐   ┌──────────────────┐
│ obj obj obj ░░░░ │ → │ obj obj ░░░░░░░░ │ → │ ░░░░░░░░░░░░░░░░ │
└──────────────────┘   └──────────────────┘   └──────────────────┘
        ↑ offset              ↑ offset              ↑ offset = 0
```

Each `Block` maintains a bump pointer (`off_`). When a block is exhausted, `Bundle` allocates a new one. `reset()` sets all offsets to zero without releasing memory back to the OS.

### Alignment

All allocations respect the requested alignment:

```cpp
std::uintptr_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
```

Debug builds assert that alignment is a power of two.

### Memory Acquisition

Blocks are allocated via `mmap(2)` with `MAP_PRIVATE | MAP_ANONYMOUS`. This bypasses the C++ heap entirely, reducing fragmentation in long-running processes.

### Resource Management

- `Bundle` is **move-only** (non-copyable)
- Blocks are freed via `munmap(2)` on destruction
- `BlockStorage` uses RAII to ensure cleanup

## Limitations

| Limitation | Explanation |
|------------|-------------|
| No individual free | Only `reset()` deallocates; this is inherent to bump allocation. |
| Destructors not called | `reset()` does not invoke `~T()`. Manually destruct non-trivial types if needed, or only store trivially-destructible types. |
| Not thread-safe | Concurrent `store()` / `reset()` requires external synchronization. |
| POSIX only | Uses `mmap(2)`. Windows support would require `VirtualAlloc`. |

### When NOT to use ByteForge

- Objects have varying lifetimes requiring individual deallocation
- You need a general-purpose allocator
- Thread-safety is required without external locking

## Platform Support

| Platform | Status |
|----------|--------|
| Linux    | Supported |
| macOS    | Supported |
| Windows  | Not yet supported (requires `VirtualAlloc` backend) |

### Requirements

- C++20 compiler (GCC 10+, Clang 10+, Apple Clang 12+)
- CMake 3.16+
- POSIX-compliant OS

## Building Examples

```bash
mkdir build && cd build
cmake ..
cmake --build .

# Run examples
./block      # Low-level Block demo
./bundle     # Bundle usage demo
./benchmark  # Performance comparison
```

## License

MIT License. See [LICENSE](LICENSE) for details.

---

**ByteForge** — Allocate fast. Reset faster.
