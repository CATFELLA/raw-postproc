#include "timer.hpp"

uint64_t tick(void) {
  uint64_t ticks = 0;

#ifdef __amd64__
  uint32_t high, low;
  __asm__ __volatile__("rdtsc\n"
                       "movl %%edx, %0\n"
                       "movl %%eax, %1\n"
                       : "=r"(high), "=r"(low)::"%rax", "%rbx", "%rcx", "%rdx");

  ticks = (static_cast<uint64_t>(high) << 32) | low;
#endif

  return ticks;
}
