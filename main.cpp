#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

static std::uintptr_t align_up(std::uintptr_t x, std::size_t a) {
  return (x + (a - 1)) & ~(std::uintptr_t(a - 1)); // a - 1 reverses last 0s (multiple of 2), adding it aligns it UP instead of down, AND with a - 1 turns last numbers into 0s aligning x
};

struct Arena {
  std::byte* base = nullptr; // byte is a more precise alternative than char (traditionally used since it's 1 byte)
  std::size_t cap = 0; // size_t is an unsigned integer often used for sizes (allows bigger numbers than unsigned int type and traditionally used)
  std::size_t off = 0;

  void* alloc(std::size_t n, std::size_t alignment = alignof(std::max_align_t)) { // max_align_t default because that is the baseline alignment required for THIS system. Usually 8 or 16.
    assert(alignment != 0);
    assert((alignment & (alignment - 1)) == 0); // Power of two (alignment - 1 will be all 1s if alignment is power of two)
    
    std::uintptr_t cur = std::uintptr_t(base) + off; // starting addr + offset is current pointer
    
    std::uintptr_t aligned = align_up(cur, alignment);

    std::size_t padding = aligned - cur;

    if (off + padding + n > cap) { // off + padding + n because off is relative to the allocated space for buffer (base is relative to OS heap)
      return nullptr;
    };

    off += padding + n;
    
    return reinterpret_cast<void*>(aligned); // reinterpret_cast is a specialized function meant for int -> ptr and ptr -> int reinterpretations. static_cast generally does NOT allow int -> ptr
  };

  void reset() {
    off = 0; // Arena is just a pointer storage. It gives out pointers, caller pushes. So resetting the offset to 0, caller can push again into "empty" Arena (old content pushed gets overwritten)
  };
};

int main() {
  std::cout << "Main program ran!" << "\n";

  constexpr std::size_t N = 1024; // Prefer constexpr when values are needed at compile time (normal int N -> can't change, but still known at runtime, constexpr -> can't change, known before runtime)
  alignas(64) std::byte buffer[N]; // alignas is optional, it aligns start of buffer to an address divisible by 64 in this case. Useful for SIMD (Single Instruction, Multiple Data), not this case.

  Arena arena;
  arena.base = buffer;
  arena.cap = N;

  int* p = static_cast<int*>(arena.alloc(sizeof(int), alignof(int))); // alignof(int) is more deterministic because size of int may RARELY vary between different platforms
  assert(p != nullptr);
  *p = 42;

  void* block = arena.alloc(128, 64);
  assert(block != nullptr);

  std::cout << "p = " << p << ", value = " << *p << "\n";
  std::cout << "block = " << block << "\n";
  std::cout << "used bytes = " << arena.off << " / " << arena.cap << "\n";

  arena.reset();
  void* again = arena.alloc(500, 32);
  assert(again != nullptr);

  std::cout << "after reset, used bytes = " << arena.off << " / " << arena.cap << "\n";

  return 0;
};
