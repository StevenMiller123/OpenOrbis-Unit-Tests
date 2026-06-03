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

#define open_file(flags)                                                                                                                                       \
  fd = sceKernelOpen("/data/file_map_test.txt", flags, 0777);                                                                                                  \
  CHECK(fd > 0);

#define close_file(fd)                                                                                                                                         \
  result = sceKernelClose(fd);                                                                                                                                 \
  UNSIGNED_INT_EQUALS(0, result);

#define map_file(in_addr, fd, flags, prot)                                                                                                                     \
  addr   = in_addr;                                                                                                                                            \
  result = sceKernelMmap(addr, 0x4000, prot, flags, fd, 0, &addr);                                                                                             \
  UNSIGNED_INT_EQUALS(0, result);

#define trunc_file(fd, size)                                                                                                                                   \
  result = sceKernelFtruncate(fd, size);                                                                                                                       \
  UNSIGNED_INT_EQUALS(0, result);                                                                                                                              \
  result = sceKernelLseek(fd, 0, 0);                                                                                                                           \
  UNSIGNED_INT_EQUALS(0, result);

#define check_memory(addr, data)                                                                                                                               \
  result = memcmp((void*)addr, data, 0x4000);                                                                                                                  \
  UNSIGNED_INT_EQUALS(0, result);

#define write_memory(addr, data) memcpy((void*)addr, data, 0x4000);

#define protect_memory(addr, prot)                                                                                                                             \
  result = sceKernelMprotect(addr, 0x4000, prot);                                                                                                              \
  UNSIGNED_INT_EQUALS(0, result);

#define check_file(fd, data)                                                                                                                                   \
  result = sceKernelRead(fd, test_buf, 0x4000);                                                                                                                \
  UNSIGNED_INT_EQUALS(0x4000, result);                                                                                                                         \
  result = sceKernelLseek(fd, 0, 0);                                                                                                                           \
  UNSIGNED_INT_EQUALS(0, result);                                                                                                                              \
  result = memcmp(test_buf, data, 0x4000);                                                                                                                     \
  UNSIGNED_INT_EQUALS(0, result);

#define unmap_file(addr)                                                                                                                                       \
  result = sceKernelMunmap(addr, 0x4000);                                                                                                                      \
  UNSIGNED_INT_EQUALS(0, result);

#define write_file(fd, addr)                                                                                                                                   \
  result = sceKernelWrite(fd, addr, 0x4000);                                                                                                                   \
  UNSIGNED_INT_EQUALS(0x4000, result);                                                                                                                         \
  result = sceKernelLseek(fd, 0, 0);                                                                                                                           \
  UNSIGNED_INT_EQUALS(0, result);

TEST(MemoryTests, TestFileMappings) {
  char zeros_buf[0x4000];
  memset(zeros_buf, 0, sizeof(zeros_buf));
  char ones_buf[0x4000];
  memset(ones_buf, 1, sizeof(ones_buf));
  char twos_buf[0x4000];
  memset(twos_buf, 2, sizeof(twos_buf));
  char test_buf[0x4000];
  memset(test_buf, 0, sizeof(test_buf));
  int32_t  MAP_NOFLAGS = 0;
  int32_t  MAP_SHARED  = 1;
  int32_t  MAP_PRIVATE = 2;
  int32_t  fd;
  int64_t  result;
  uint64_t addr     = 0;
  uint64_t out_addr = 0;

  // Open and prepare file
  open_file(0x602);
  trunc_file(fd, 0x4000);

  // File data starts zeroed after ftruncate
  check_file(fd, zeros_buf);

  // Write ones for testing purposes
  write_file(fd, ones_buf);

  // Close file and begin tests.
  close_file(fd);

  // A couple combinations of file perms and mmap prots need testing here.
  // First, we want the basic case. Open file as read-write, mmap as read-write with MAP_SHARED specified.
  open_file(2);
  map_file(0, fd, MAP_SHARED, 3);

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
  map_file(0, fd, MAP_PRIVATE, 3);

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

  map_file(0, fd, MAP_PRIVATE, 3);

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
  int32_t check_result = sceKernelMmap(0, 0x4000, 3, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  check_result = sceKernelMmap(0, 0x4000, 3, MAP_PRIVATE, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  check_result = sceKernelMmap(0, 0x4000, 3, MAP_NOFLAGS, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);

  // Write-only mmap is impossible, as write-only is treated as read-write.
  check_result = sceKernelMmap(0, 0x4000, 2, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  check_result = sceKernelMmap(0, 0x4000, 2, MAP_PRIVATE, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  check_result = sceKernelMmap(0, 0x4000, 2, MAP_NOFLAGS, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);

  // Read-only mmap is impossible
  check_result = sceKernelMmap(0, 0x4000, 1, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  check_result = sceKernelMmap(0, 0x4000, 1, MAP_PRIVATE, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  check_result = sceKernelMmap(0, 0x4000, 1, MAP_NOFLAGS, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);

  // No-access mmap is possible.
  map_file(0, fd, MAP_SHARED, 0);

  // We can protect this to get access to data
  protect_memory(addr, 3);

  // Now memory should contain ones if it is properly backed
  check_memory(addr, ones_buf);

  // Since this is a shared mapping, we can write twos to the file and see the change
  write_file(fd, twos_buf);
  check_memory(addr, twos_buf);

  // While we can't see it through file reads, we can still write through memory too
  write_memory(addr, ones_buf);

  // Unmap memory
  unmap_file(addr);

  // Remap file
  map_file(0, fd, MAP_SHARED, 0);
  protect_memory(addr, 3);

  // The memory should contain the ones written earlier.
  check_memory(addr, ones_buf);

  // Unmap memory
  unmap_file(addr);
  close_file(fd);

  // Re-open and setup the same mapping, this time as private.
  open_file(1);
  map_file(0, fd, MAP_PRIVATE, 0);
  protect_memory(addr, 3);

  // Now memory should contain ones if it is properly backed
  check_memory(addr, ones_buf);

  // Under these circumstances, this private mapping behaves as a copy-on-write mapping.
  // The contents of the memory update appropriately.
  write_file(fd, twos_buf);
  check_memory(addr, twos_buf);

  // Writes to memory trigger a copy
  write_memory(addr, ones_buf);
  check_memory(addr, ones_buf);

  // Verify that the memory is no longer identical
  write_file(fd, zeros_buf);
  check_memory(addr, ones_buf);

  // Remap to confirm write occurred properly.
  unmap_file(addr);
  map_file(0, fd, MAP_PRIVATE, 0);
  protect_memory(addr, 3);
  check_memory(addr, zeros_buf);

  // Write ones and close.
  write_file(fd, ones_buf);
  unmap_file(addr);
  close_file(fd);

  // Open file as read-only.
  open_file(0);

  // Read-write mmap is impossible with MAP_SHARED.
  check_result = sceKernelMmap(0, 0x4000, 3, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);

  // You can, however, perform a read-write mmap with MAP_PRIVATE or no flags.
  map_file(0, fd, MAP_PRIVATE, 3);
  unmap_file(addr);
  map_file(0, fd, MAP_NOFLAGS, 3);
  unmap_file(addr);

  // Same applies to write-only, as write-only is treated as read-write.
  check_result = sceKernelMmap(0, 0x4000, 2, MAP_SHARED, fd, 0, &addr);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_EACCES, check_result);
  map_file(0, fd, MAP_PRIVATE, 3);
  unmap_file(addr);
  map_file(0, fd, MAP_NOFLAGS, 3);
  unmap_file(addr);

  // Read-only mmap is possible with all flag combinations
  map_file(0, fd, MAP_SHARED, 1);
  unmap_file(addr);
  map_file(0, fd, MAP_PRIVATE, 1);
  unmap_file(addr);
  map_file(0, fd, MAP_NOFLAGS, 1);
  unmap_file(addr);

  // We can take advantage of mprotect again
  map_file(0, fd, MAP_SHARED, 1);
  protect_memory(addr, 3);

  // Now we have read-write access to a read-only file.
  check_memory(addr, ones_buf);

  // Write to our memory, this will update the file contents.
  write_memory(addr, twos_buf);
  check_file(fd, twos_buf);
  write_memory(addr, ones_buf);

  // Unmap and close file.
  unmap_file(addr);
  close_file(fd);

  // Open file as read-only.
  open_file(0);

  // Make sure MAP_PRIVATE behaves as expected.
  map_file(0, fd, MAP_PRIVATE, 1);
  protect_memory(addr, 3);

  // Now we have read-write access to a read-only file.
  check_memory(addr, ones_buf);

  // Write to our memory, this will not update the file contents.
  write_memory(addr, twos_buf);
  check_file(fd, ones_buf);

  // Unmap and close file.
  unmap_file(addr);
  close_file(fd);
}

#undef open_file
#undef close_file
#undef map_file
#undef trunc_file
#undef check_memory
#undef protect_memory
#undef check_file
#undef unmap_file
#undef write_file