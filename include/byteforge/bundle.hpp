#include "byteforge/block.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace byteforge {

class Bundle {
  public:
    explicit Bundle(std::size_t initial_block_size);

    Bundle(const Bundle&) = delete;
    Bundle& operator=(const Bundle&) = delete;
    
    template <typename T, typename... Args> // template must be in .hpp
    T* store(Args&&... args) { // compiler needs to generate function itself, it can't just link assuming function already exists (it generates -> links rather than just link)
      void* result = allocate_raw(sizeof(T), alignof(T));

      if (result == nullptr) {
        return nullptr;
      }

      return new (result) T(std::forward<Args>(args)...);
    }
 
    void reset();
    std::size_t used();
    std::size_t capacity();

  private:
    struct BlockStorage {
      std::unique_ptr<std::uint8_t[]> buffer;
      Block block;

      explicit BlockStorage(std::size_t block_size)
        : buffer(std::make_unique<std::uint8_t[]>(block_size)),
          block(buffer.get(), block_size)
        {}
    };
   
    void* allocate_raw (std::size_t n, std::size_t alignment);

    std::vector<BlockStorage> blocks_;
    std::size_t block_size_;
    std::size_t cap_;
  };

} // namespace byteforge
