#include "byteforge/block.hpp"

#include <array>
#include <iostream>

int main () {
  std::array<std::uint8_t, 1024> buffer{};
  byteforge::Block block(buffer.data(), buffer.size());

  int* a = static_cast<int*>(block.allocate(sizeof(int), alignof(int))); 
  double* b = static_cast<double*>(block.allocate(sizeof(double), alignof(double)));
 
  *a = 32;
  *b = 345.94;

  void* raw = block.allocate(sizeof(std::string), alignof(std::string));
  auto* c = new (raw) std::string("Byteforge status - ok");

  std::cout << "pointer 1 address: " << a << ", " << "pointer 1 value: " << *a << "\n";
  std::cout << "pointer 2 address: " << b << ", " << "pointer 2 value: " << *b << "\n";
  std::cout << "pointer 3 address: " << c << ", " << "pointer 3 value: " << *c << "\n";

  std::cout << "used: " << block.used() << " / " << block.capacity() << "\n";

  block.reset();

  std::cout << "block has been reset!\n";
  
  std::cout << "used: " << block.used() << " / " << block.capacity() << "\n";

  return 0;
}
