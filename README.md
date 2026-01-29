# Logos-Arena

A bump-pointer arena allocator built from scratch in C++.

## Overview

Logos-Arena is a linear allocator that hands out memory sequentially from fixed-size blocks. Objects are allocated by advancing a bump pointer; there is no per-object deallocation. When you're done with a batch of allocations, call `reset()` to reclaim all memory at once. This pattern is well-suited for phase-based workloads—parsers, compilers, frame allocators in games—where lifetimes are uniform and predictable.

## Core Abstractions

### Block

`Block` owns a raw memory buffer and maintains a bump pointer (`off_`). It provides:

- `allocate(size, alignment)` — returns an aligned pointer or `nullptr` if the block is exhausted.
- `reset()` — rewinds the bump pointer to zero.
- `used()` / `capacity()` — returns bytes consumed and total bytes available.

### Bundle

`Bundle` is the user-facing arena. It chains multiple `Block` instances and grows automatically when the current block is full.

```cpp
template <typename T, typename... Args>
T* store(Args&&... args);
```

Allocates `sizeof(T)` bytes with `alignof(T)`, then placement-news a `T` in place. Returns `nullptr` only if the underlying system allocation fails.

```cpp
void reset();
std::size_t used();      // total bytes allocated across all blocks
std::size_t capacity();  // total bytes reserved across all blocks
```

`Bundle` is **non-copyable** but **movable**. It hides the raw allocator internals; users interact only through `store<T>()` and `reset()`.

## Design Goals

| Goal | Implementation |
|------|----------------|
| Zero per-object frees | Only `reset()` deallocates; no destructors called. |
| Amortized O(1) allocation | Bump pointer advance; new block allocation is rare. |
| O(1) reset | Rewind each block's offset to zero. |
| Correct alignment for any `T` | `align_up()` rounds to next multiple of alignment. |
| No exceptions on normal paths | `store<T>()` returns `nullptr` on block exhaustion; `std::bad_alloc` only if system `new` fails when growing. |

## Example Usage

```cpp
#include "byteforge/bundle.hpp"
#include <string>

int main() {
    byteforge::Bundle arena(4096);  // initial block size in bytes

    int* x = arena.store<int>(42);
    std::string* s = arena.store<std::string>("hello");

    // Use x and s...

    arena.reset();
    // x and s are now invalid—memory has been reclaimed.
}
```

After `reset()`, all previously returned pointers are **invalidated**. Do not dereference them.

## Correctness and Safety

**Alignment.** `Block::allocate` computes the next aligned address via:

```cpp
std::uintptr_t aligned = (cur + (alignment - 1)) & ~(alignment - 1);
```

This assumes `alignment` is a power of two, which is enforced by `assert`.

**Overflow checks.** Before advancing the bump pointer, `Block::allocate` verifies:

```cpp
if (off_ + padding + n > cap_) return nullptr;
```

Debug builds will fire asserts if `alignment` is zero or not a power of two.

**Key invariant:** Pointers returned from `store<T>()` are valid until the next call to `reset()` or destruction of the `Bundle`.

## Benchmark

A simple microbenchmark allocating 1,000,000 small objects (`sizeof(int)`) in a tight loop:

| Allocator | Time (Apple M2, `-O3`) |
|-----------|------------------------|
| `new` / `delete` | ~18 ms |
| `Bundle::store<int>` | ~2 ms |

Logos-Arena is roughly **8–9× faster** in this synthetic test. This measures raw allocation throughput only—real workloads vary. The arena wins because it avoids per-object bookkeeping and syscall overhead on every allocation.

**Caveat:** This is a microbenchmark. Arena allocators are not a drop-in replacement for general-purpose `new`; they require batch-lifetime semantics.

## Limitations

- **No individual deallocation.** You cannot free a single object; only `reset()` the entire arena.
- **Not thread-safe.** Concurrent calls to `store()` or `reset()` require external synchronization.
- **Destructors not called.** `reset()` does not invoke destructors. For types with non-trivial cleanup, you must call destructors manually before reset, or only store trivially-destructible types.

## Future Work

- Per-thread arenas to avoid locking.
- Block pooling / size classes for better memory reuse.
- Integration with parser / AST frameworks where arena lifetime matches parse phases.
- Optional destructor tracking for non-trivial types.

## Building

```bash
mkdir build && cd build
cmake ..
make
```

Run the examples:

```bash
./examples/bundle
```

## License

MIT
