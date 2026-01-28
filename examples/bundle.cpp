#include "byteforge/bundle.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

int main() {
  byteforge::Bundle bundle(1024);

  std::cout << "Bundle started. Base alignment is: " << alignof(std::max_align_t) << "\n";
  
  void* p1 = bundle.store<int>(100);
  void* p2 = bundle.store<std::string>("Hello!");

  assert(p1 != nullptr && p2 != nullptr);
  
  std::cout << "Space used: " << bundle.used() << " / " << bundle.capacity() << "\n";

  bundle.reset();
  
  void* p3 = bundle.store<std::array<int, 100>>();
  void* p4 = bundle.store<int>(1025);
  
  assert(p3 != nullptr);
  assert(p4 != nullptr);

  std::cout << "Space used: " << bundle.used() << " / " << bundle.capacity() << "\n";
}
