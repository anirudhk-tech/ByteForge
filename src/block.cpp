#include "byteforge/block.hpp"

#include <cassert>
#include <cstdint>

namespace {

std::uintptr_t align_up(std::uintptr_t x, std::size_t a) {
  return (x + (a - 1)) & ~(std::uintptr_t(a - 1)); // assuming a is power of two, go up and then bit mask off last integers to make multiple of 8
}

} // anonymous namespace

namespace byteforge {

Block::Block(std::uint8_t* base, std::size_t cap)
  : base_(base), cap_(cap), off_(0) {
  // base_: block starts here
  // cap_: total bytes available
  // off_: current position of next free slot
}

void* Block::allocate(std::size_t n, std::size_t alignment) {
  assert(alignment != 0);
  assert((alignment & (alignment - 1)) == 0);

  std::uintptr_t cur = std::uintptr_t(base_) + off_;

  std::uintptr_t aligned = align_up(cur, alignment);

  std::size_t padding = aligned - cur;

  if (off_ + padding + n > cap_) {
    return nullptr; // over allocated space, caller has to handle this
  }

  off_ += padding + n;

  return reinterpret_cast<void*>(aligned);
}

void Block::reset() {
  off_ = 0;
}

std::size_t Block::capacity() const {
  return cap_;
}

std::size_t Block::used() const {
  return off_;
}

} // namespace byteforge
