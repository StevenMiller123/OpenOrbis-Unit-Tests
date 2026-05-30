#include "test.h"

#include <CppUTest/TestHarness.h>
#include <cstdio>

TEST_GROUP (MemoryTests) {
  void setup() { // Before each test, call mem_scan to print out memory map information.
    // This will provide an indicator of how the memory map looks, which can help with debugging strange behavior during tests.
    printf("Before test:\n");
    mem_scan();
  }

  void teardown() {}
};

// Test behavior that changed in firmware 5.50
TEST(MemoryTests, FW550Test) {
  // Starting with firmware 5.50, memory mappings performed with different call addresses will now merge.
  // This ignores the anon names, but not other memory names.
  int64_t phys_addr;
  int32_t result = sceKernelAllocateDirectMemory(0x100000, 0x1a0000, 0xa0000, 0, 0, &phys_addr);

  uint64_t base_addr = 0x2000000000;
  uint64_t addr      = base_addr;
  result             = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x100000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping1");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x80000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x180000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping5");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x20000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x120000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping2");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x60000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x160000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping4");
  UNSIGNED_INT_EQUALS(0, result);

  addr   = base_addr + 0x40000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x140000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelSetVirtualRangeName(addr, 0x20000, "Mapping3");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // There should be five observable mappings.
  uint64_t start_addr;
  uint64_t end_addr;
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0x20000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x20000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x20000, start_addr);
  LONGS_EQUAL(base_addr + 0x40000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x40000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x40000, start_addr);
  LONGS_EQUAL(base_addr + 0x60000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x60000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x60000, start_addr);
  LONGS_EQUAL(base_addr + 0x80000, end_addr);

  result = sceKernelQueryMemoryProtection(base_addr + 0x80000, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr + 0x80000, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Now call sceKernelSetVirtualRangeName
  result = sceKernelSetVirtualRangeName(base_addr, 0xa0000, "Mapping");
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  // Mappings all merge together.
  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Unmap testing memory.
  result = sceKernelReleaseDirectMemory(phys_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);

  // Additionally, mappings with different anon names also end up merging.
  // The memory name preserved will always be the name of the newest mapping.
  result = sceKernelAllocateDirectMemory(0x100000, 0x1a0000, 0xa0000, 0, 0, &phys_addr);
  addr   = base_addr;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x100000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x80000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x180000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x20000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x120000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x60000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x160000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  addr   = base_addr + 0x40000;
  result = sceKernelMapDirectMemory(&addr, 0x20000, 0x33, 0x10, 0x140000, 0);
  UNSIGNED_INT_EQUALS(0, result);

  mem_scan();

  result = sceKernelQueryMemoryProtection(base_addr, &start_addr, &end_addr, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  LONGS_EQUAL(base_addr, start_addr);
  LONGS_EQUAL(base_addr + 0xa0000, end_addr);

  // Unmap testing memory.
  result = sceKernelReleaseDirectMemory(phys_addr, 0xa0000);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, TLOU2Test) {
  // Check for something that came up in fpPS4.
  // I'll probably rewrite this test at some point.
  int64_t first_phys_addr = 0;
  int32_t result          = sceKernelAllocateMainDirectMemory(0x100000000, 0x10000, 0, &first_phys_addr);
  UNSIGNED_INT_EQUALS(0, result);

  uint64_t first_addr = 0x1600000000;
  result              = sceKernelMapDirectMemory(&first_addr, 0x1D200000, 3, 0, first_phys_addr, 0x200000);
  UNSIGNED_INT_EQUALS(0, result);

  uint64_t second_addr = 0x3700000000;
  for (int32_t i = 0; i < 48; ++i) {
    uint64_t unmap_addr = first_addr + (i * 0x100000);
    result              = sceKernelMunmap(unmap_addr, 0x100000);
    UNSIGNED_INT_EQUALS(0, result);

    uint64_t map_addr = second_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + (i * 0x100000), 0x100000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  first_addr  = 0x1603600000;
  second_addr = 0x4100000000;

  for (int32_t i = 0; i < 10; ++i) {
    uint64_t unmap_addr = first_addr + (i * 0x100000);
    result              = sceKernelMunmap(unmap_addr, 0x100000);

    uint64_t map_addr = second_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + 0x3600000 + (i * 0x100000), 0x100000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  result = sceKernelMunmap(second_addr, 0xa00000);
  UNSIGNED_INT_EQUALS(0, result);

  for (int32_t i = 0; i < 10; ++i) {
    uint64_t map_addr = first_addr + (i * 0x100000);
    result            = sceKernelMapDirectMemory(&map_addr, 0x100000, 3, 0x90, first_phys_addr + 0x3600000 + (i * 0x100000), 0x100000);
    UNSIGNED_INT_EQUALS(0, result);
  }

  result = sceKernelCheckedReleaseDirectMemory(first_phys_addr, 0x100000000);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(MemoryTests, TestFileMappings) {
  char zeros_buf[0x4000];
  memset(zeros_buf, 0, sizeof(zeros_buf));
  char ones_buf[0x4000];
  memset(ones_buf, 1, sizeof(ones_buf));
  char twos_buf[0x4000];
  memset(twos_buf, 2, sizeof(twos_buf));
  int32_t MAP_NOFLAGS = 0;
  int32_t MAP_SHARED = 1;
  int32_t MAP_PRIVATE = 2;

  auto open_file = [](int32_t flags) {
    int32_t fd = sceKernelOpen("/data/file_map_test.txt", flags, 0777);
    CHECK(fd > 0);
    return fd;
  };

  auto close_file = [](int32_t fd) {
    int32_t result = sceKernelClose(fd);
    UNSIGNED_INT_EQUALS(0, result);
  };

  auto map_file = [](uint64_t addr, int32_t fd, int32_t flags, int32_t prot) {
    // Map the file to memory
    uint64_t out_addr = addr;
    int32_t  result   = sceKernelMmap(out_addr, 0x4000, prot, flags, fd, 0, &out_addr);
    UNSIGNED_INT_EQUALS(0, result);
    return out_addr;
  };

  auto unmap_file = [](uint64_t addr) {
    int32_t result = sceKernelMunmap(addr, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  };

  auto write_file = [](int32_t fd, void* addr) {
    // Write contents from addr to file
    int64_t result = sceKernelWrite(fd, addr, 0x4000);
    UNSIGNED_INT_EQUALS(0x4000, result);
    // Reset file pointer
    result = sceKernelLseek(fd, 0, 0);
    UNSIGNED_INT_EQUALS(0, result);
  };

  auto trunc_file = [](int32_t fd, uint64_t size) {
    // Truncate file to requested size
    int64_t result = sceKernelFtruncate(fd, size);
    UNSIGNED_INT_EQUALS(0, result);
    // Reset file pointer
    result = sceKernelLseek(fd, 0, 0);
    UNSIGNED_INT_EQUALS(0, result);
  };

  auto check_memory = [](uint64_t addr, void* data) {
    int32_t result = memcmp((void*)addr, data, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  };

  auto write_memory = [](uint64_t addr, void* data) {
    memcpy((void*)addr, data, 0x4000);
  };

  auto protect_memory = [](uint64_t addr, int32_t prot) {
    int32_t result = sceKernelMprotect(addr, 0x4000, prot);
    UNSIGNED_INT_EQUALS(0, result);
  };

  auto check_file = [](int32_t fd, void* data) {
    // Read data to addr
    char addr[0x4000];
    int64_t result = sceKernelRead(fd, addr, 0x4000);
    UNSIGNED_INT_EQUALS(0x4000, result);

    // Reset file pointer
    result = sceKernelLseek(fd, 0, 0);
    UNSIGNED_INT_EQUALS(0, result);

    // Check memory contents
    result = memcmp(addr, data, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  };

  // Open and prepare file
  int32_t fd = open_file(0x602);
  trunc_file(fd, 0x4000);

  // File data starts zeroed after ftruncate
  check_file(fd, zeros_buf);

  // Write ones for testing purposes
  write_file(fd, ones_buf);

  // Close file and begin tests.
  close_file(fd);

  // A couple combinations of file perms and mmap prots need testing here.
  // First, we want the basic case. Open file as read-write, mmap as read-write with MAP_SHARED specified.
  fd = open_file(2);
  uint64_t addr = map_file(0, fd, MAP_SHARED, 3);

  // What we should have here is a shared mapping.
  // All writes to the memory will be visible in the file, and all writes to the file will appear in memory.
  // As-is, both memory and file should be full of ones.
  check_memory(addr, ones_buf);
  check_file(fd, ones_buf);

  // Now write to the memory.
  write_memory(addr, twos_buf);

  // Now reads should show 2s
  check_file(fd, twos_buf);

  // Memory should (naturally) also contain twos
  check_memory(addr, twos_buf);

  // Writing directly to the file should update the memory
  write_file(fd, ones_buf);

  // Now reads should (naturally) show 1s
  check_file(fd, ones_buf);

  // Memory should also contain ones
  check_memory(addr, ones_buf);

  // Unmap and close file
  unmap_file(addr);
  close_file(fd);

  fd = open_file(2);
  addr = map_file(0, fd, MAP_PRIVATE, 3);

  // What we should have here is a private mapping, where file contents and memory contents aren't synchronized
  // As-is, both memory and file should be full of ones.
  check_memory(addr, ones_buf);
  check_file(fd, ones_buf);

  // Writing directly to the file should not update the memory
  write_file(fd, twos_buf);

  // Now reads should show twos
  check_file(fd, twos_buf);

  // Memory should still contain ones
  check_memory(addr, ones_buf);

  // Unmap file
  unmap_file(addr);

  addr = map_file(0, fd, MAP_PRIVATE, 3);

  // Now memory should contain twos
  check_memory(addr, twos_buf);

  // Writes to memory won't update the file either
  write_memory(addr, ones_buf);

  // Memory should contain ones
  check_memory(addr, ones_buf);

  // File should still contain twos
  check_file(fd, twos_buf);

  // Write ones
  write_file(fd, ones_buf);

  // Unmap and close file
  unmap_file(addr);
  close_file(fd);

  // Open file as write-only.
  fd = open_file(1);

  // Read-write mmap is impossible.
  int32_t result = sceKernelMmap(0, 0x4000, 3, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(0, 0x4000, 3, MAP_PRIVATE, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(0, 0x4000, 3, MAP_NOFLAGS, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Write-only mmap is impossible
  result = sceKernelMmap(0, 0x4000, 2, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(0, 0x4000, 2, MAP_PRIVATE, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(0, 0x4000, 2, MAP_NOFLAGS, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // Read-only mmap is impossible
  result = sceKernelMmap(0, 0x4000, 1, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(0, 0x4000, 1, MAP_PRIVATE, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);
  result = sceKernelMmap(0, 0x4000, 1, MAP_NOFLAGS, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, result);

  // No-access mmap is possible.
  addr = map_file(0, fd, MAP_SHARED, 0);

  // We can protect this to get access to data
  protect_memory(addr, 3);

  // Now memory should contain ones if it is properly backed
  check_memory(addr, ones_buf);

  // Since this is a shared mapping, we can write twos to the file and see the change
  write_file(fd, twos_buf);
  check_memory(addr, twos_buf);

  // Bring back ones before unmapping
  write_file(fd, ones_buf);
  
  // Unmap and close file
  unmap_file(addr);
  close_file(fd);
}