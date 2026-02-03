#include "byteforge/block.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <sys/mman.h>

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
      std::uint8_t* mem;
      std::size_t cap;
      Block block;

      static std::uint8_t* map_or_throw(std::size_t cap_bytes) {
        void* p = ::mmap(
          nullptr,
          cap_bytes,
          PROT_READ | PROT_WRITE,
          MAP_PRIVATE | MAP_ANONYMOUS,
          -1,
          0
        );

        if (p == MAP_FAILED) {
          throw std::bad_alloc();
        }

        return static_cast<std::uint8_t*>(p);
      }
  
     explicit BlockStorage(std::size_t cap_bytes)
      : mem(map_or_throw(cap_bytes)),
        cap(cap_bytes),
        block(mem, cap)
      {}
    
      ~BlockStorage() noexcept {
        if (mem) {
          ::munmap(mem, cap);  
          }    
        }
      
      BlockStorage(const BlockStorage&) = delete;

      BlockStorage(BlockStorage&& other) noexcept 
      : mem(other.mem),
        cap(other.cap),
        block(std::move(other.block))
      {
        other.mem = nullptr;
        other.cap = 0;
      }

      BlockStorage& operator=(BlockStorage&& other) noexcept {
        if (this == &other) return *this;
        
        if (mem) {
          ::munmap(mem, cap);
        }

        mem = other.mem;
        cap = other.cap;
        block = std::move(other.block);

        other.mem = nullptr;
        other.cap = 0;

        return *this;
      }
    };

    void* allocate_raw (std::size_t n, std::size_t alignment);

    std::vector<BlockStorage> blocks_;
    std::size_t block_size_;
    std::size_t cap_;
  };

} // namespace byteforge
