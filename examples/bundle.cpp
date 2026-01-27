#include "byteforge/bundle.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>

int main() {
  byteforge::Bundle bundle(1024);

  std::cout << "Bundle started. Base alignment is: " << alignof(std::max_align_t) << "\n";
  
  void* p1 = bundle.store(100, alignof(std::max_align_t));
  void* p2 = bundle.store(900, alignof(std::max_align_t));

  assert(p1 != nullptr && p2 != nullptr);
  
  std::cout << "Space used: " << bundle.used() << " / " << bundle.capacity() << "\n";

  bundle.reset();
  
  void* p3 = bundle.store(100, alignof(std::max_align_t));
  void* p4 = bundle.store(1025, alignof(std::max_align_t));
  
  assert(p3 != nullptr);
  assert(p4 != nullptr);

  std::cout << "Space used: " << bundle.used() << " / " << bundle.capacity() << "\n";
}
