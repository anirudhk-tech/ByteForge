#pragma once

#include <cstddef>
#include <cstdint>

namespace byteforge {

class Block {
public:
  Block(std::uint8_t* buffer,
        std::size_t   size);

  void* allocate(std::size_t size,
                std::size_t alignment);

  void reset();

  std::size_t capacity() const;
  std::size_t used() const;

private:
  std::uint8_t* base_;
  std::size_t cap_;
  std::size_t off_;
};

} // namespace byteforge
