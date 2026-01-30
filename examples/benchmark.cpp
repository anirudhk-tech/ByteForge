#include <chrono>
#include <cstdint>
#include <iostream>
#include "byteforge/bundle.hpp"

static volatile std::uintptr_t sink = 0;

struct BigPod {
  int x;
  double y;
  char buf[100];

  BigPod(int v) : x(v), y(v * 0.5) {
    for (int i = 0; i < 100; ++i) buf[i] = 'a';
  }
};

std::int64_t bench_arena(byteforge::Bundle& bundle, int frames, int per_frame) {
  using clock = std::chrono::steady_clock;
  auto start = clock::now();

  for (int f = 0; f < frames; ++f) {
    for (int i = 0; i < per_frame; ++i) {
      BigPod* p = bundle.store<BigPod>(i);
      sink ^= reinterpret_cast<std::uintptr_t>(p);
    }
    bundle.reset();
  }

  auto end = clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

std::int64_t bench_new_delete(int frames, int per_frame) {
  using clock = std::chrono::steady_clock;
  auto start = clock::now();

  for (int f = 0; f < frames; ++f) {
    for (int i = 0; i < per_frame; ++i) {
      BigPod* p = new BigPod(i);
      sink ^= reinterpret_cast<std::uintptr_t>(p);
      delete p;
    }
  }

  auto end = clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main() {
  const int frames    = 50;
  const int per_frame = 20'000;   // total 1,000,000 objects

  byteforge::Bundle bundle(64 * 1024 * 1024);

  auto arena_ms = bench_arena(bundle, frames, per_frame);
  auto new_ms   = bench_new_delete(frames, per_frame);

  std::cout << "Arena BigPod:     " << arena_ms << " ms\n";
  std::cout << "new/delete BigPod:" << new_ms   << " ms\n";
}

