#include "byteforge/block.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace byteforge {

class Bundle {
  public:
    explicit Bundle(std::size_t initial_block_size);

    Bundle(const Bundle&) = delete;
    Bundle& operator=(const Bundle&) = delete;
 
    void* store(std::size_t n, std::size_t alignment);
    void reset();
    int used();
    int capacity();

  private:
    struct BlockStorage {
      std::unique_ptr<std::uint8_t[]> buffer;
      Block block;

      explicit BlockStorage(std::size_t block_size)
        : buffer(std::make_unique<std::uint8_t[]>(block_size)),
          block(buffer.get(), block_size)
        {}
    };
    
    std::vector<BlockStorage> blocks_;
    std::size_t block_size_;
    std::size_t cap_;
  };

} // namespace byteforge
