#include "byteforge/bundle.hpp"

#include <cstddef>
#include <new>
#include <algorithm>

namespace byteforge {
  
Bundle::Bundle(std::size_t initial_block_size) 
  : block_size_(initial_block_size)
{
  blocks_.push_back(BlockStorage(initial_block_size));
}

void* Bundle::allocate_raw(std::size_t n, std::size_t alignment) {
  BlockStorage& last_storage = blocks_.back();

  void* result = last_storage.block.allocate(n, alignment);
  
  if (result != nullptr) {
    return result;
  } else {
      std::size_t new_block_size = std::max(n, block_size_);
      std::size_t needed = new_block_size + alignment - 1; // worst case alignment

      blocks_.emplace_back(needed);
      BlockStorage& new_block = blocks_.back();

      void* retry = new_block.block.allocate(n, alignment);

      if (retry == nullptr) {
        throw std::bad_alloc(); // n needs greater than block_size
      }

    return retry;
    }
}

void Bundle::reset() {
  for (auto& storage : blocks_) {
    storage.block.reset();
  }
}

std::size_t Bundle::used() {
  std::size_t total = 0;
  for (auto& storage : blocks_) {
    total += storage.block.used();
  }
  return total;
}

std::size_t Bundle::capacity() {
  std::size_t total = 0;
  for (auto& storage : blocks_) {
    total += storage.block.capacity();
  }
  return total;
}

} // namespace byteforge
