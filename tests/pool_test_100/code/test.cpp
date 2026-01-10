#include "test.h"

#include "CppUTest/TestHarness.h"

#include <list>
#include <stdio.h>
#include <string>

TEST_GROUP (PoolTests) {
  void setup() { // Before each test, call mem_scan to print out memory map information.
    // This will provide an indicator of how the memory map looks, which can help with debugging strange behavior during tests.
    printf("Before test:\n");
    mem_scan();
  }

  void teardown() {
    printf("After test:\n");
    mem_scan();
  }
};

TEST(PoolTests, PoolTest) {
  // Goal here is to see how backing memory behaves when fragmented.
  // Start by preparing memory for the pool.
  uint64_t page_size      = getpagesize();
  uint64_t pool_size      = 0x200000;
  int64_t  pool_phys_addr = 0;
  // PS4 kernel reserves a page for internal use, so expand needs to consume more than pool_size.
  int32_t result = sceKernelMemoryPoolExpand(0, sceKernelGetDirectMemorySize(), pool_size + 0x20000, 0, &pool_phys_addr);
  CHECK_EQUAL(0, result);
  uint64_t pool_addr = 0;
  result             = sceKernelMemoryPoolReserve(pool_addr, pool_size, 0, 0, &pool_addr);
  CHECK_EQUAL(0, result);

  // First, commit all pool memory at once, then label each physical page with an address.
  result = sceKernelMemoryPoolCommit(pool_addr, pool_size, 0, 3, 0);
  CHECK_EQUAL(0, result);

  for (uint64_t address = pool_addr; address < pool_addr + pool_size; address += page_size) {
    uint64_t* memory_ptr  = (uint64_t*)address;
    uint64_t  pool_offset = address - pool_addr;
    // This will write the physical address of this physical page into the pool memory.
    *memory_ptr = pool_phys_addr + pool_offset;
    // Log physical addresses for comparing.
    printf("address 0x%012lX -> phys_address 0x%012lX\n", address, *memory_ptr);
  }

  // Now, decommit the main pool memory
  result = sceKernelMemoryPoolDecommit(pool_addr, pool_size, 0);
  CHECK_EQUAL(0, result);

  printf("Test 1:\n");

  // Simple test, re-commit in smaller chunks and see how physical data compares.
  uint64_t commit_size = 0x20000;
  for (uint64_t address = pool_addr; address < pool_addr + pool_size; address += commit_size) {
    result = sceKernelMemoryPoolCommit(address, commit_size, 0, 1, 0);
    CHECK_EQUAL(0, result);
  }

  // In theory, the physical addresses should all match the prior logs, due to the way the memory was committed
  for (uint64_t address = pool_addr; address < pool_addr + pool_size; address += page_size) {
    uint64_t* memory_ptr = (uint64_t*)address;
    printf("address 0x%012lX -> phys_address 0x%012lX\n", address, *memory_ptr);
  }

  // Now, decommit the main pool memory
  result = sceKernelMemoryPoolDecommit(pool_addr, pool_size, 0);
  CHECK_EQUAL(0, result);

  printf("Test 2:\n");

  // Recommit, but in reverse.
  commit_size = 0x10000;
  for (uint64_t address = pool_addr + pool_size - commit_size; address >= pool_addr; address -= commit_size) {
    result = sceKernelMemoryPoolCommit(address, commit_size, 0, 1, 0);
    CHECK_EQUAL(0, result);
  }

  for (uint64_t address = pool_addr; address < pool_addr + pool_size; address += page_size) {
    uint64_t* memory_ptr = (uint64_t*)address;
    printf("address 0x%012lX -> phys_address 0x%012lX\n", address, *memory_ptr);
  }

  // Now, decommit the main pool memory
  result = sceKernelMemoryPoolDecommit(pool_addr, pool_size, 0);
  CHECK_EQUAL(0, result);

  printf("Test 3:\n");

  // Recommit, but intentionally try to fragment the pool space.
  commit_size = 0x40000;
  for (uint64_t address = pool_addr; address < pool_addr + pool_size; address += commit_size) {
    result = sceKernelMemoryPoolCommit(address + commit_size / 2, commit_size / 2, 0, 1, 0);
    CHECK_EQUAL(0, result);
    result = sceKernelMemoryPoolCommit(address, commit_size / 2, 0, 1, 0);
    CHECK_EQUAL(0, result);
    result = sceKernelMemoryPoolDecommit(address + commit_size / 4, commit_size / 2, 0);
    CHECK_EQUAL(0, result);
    result = sceKernelMemoryPoolCommit(address + commit_size / 4, commit_size / 2, 0, 1, 0);
    CHECK_EQUAL(0, result);
  }

  for (uint64_t address = pool_addr; address < pool_addr + pool_size; address += page_size) {
    uint64_t* memory_ptr = (uint64_t*)address;
    printf("address 0x%012lX -> phys_address 0x%012lX\n", address, *memory_ptr);
  }

  // Run a mem_scan to give an idea of what the address space looks like.
  mem_scan();

  // Now, decommit the main pool memory
  result = sceKernelMemoryPoolDecommit(pool_addr, pool_size, 0);
  CHECK_EQUAL(0, result);
}