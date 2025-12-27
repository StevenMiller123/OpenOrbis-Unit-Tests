#include "test.h"

#include "CppUTest/TestHarness.h"

#include <list>
#include <stdio.h>
#include <string>

TEST_GROUP (Test) {
  void setup() {}

  void teardown() {}
};

void PrintStatInfo(OrbisKernelStat* info) {
  printf("info.st_dev = %u\n", info->st_dev);
  printf("info.st_ino = %u\n", info->st_ino);
  printf("info.st_mode = %u\n", info->st_mode);
  printf("info.st_nlink = %u\n", info->st_nlink);
  printf("info.st_uid = %u\n", info->st_uid);
  printf("info.st_gid = %u\n", info->st_gid);
  printf("info.st_rdev = %u\n", info->st_rdev);
  printf("info.st_atim.tv_sec = %li\n", info->st_atim.tv_sec);
  printf("info.st_atim.tv_nsec = %li\n", info->st_atim.tv_nsec);
  printf("info.st_mtim.tv_sec = %li\n", info->st_mtim.tv_sec);
  printf("info.st_mtim.tv_nsec = %li\n", info->st_mtim.tv_nsec);
  printf("info.st_ctim.tv_sec = %li\n", info->st_ctim.tv_sec);
  printf("info.st_ctim.tv_nsec = %li\n", info->st_ctim.tv_nsec);
  printf("info.st_size = %li\n", info->st_size);
  printf("info.st_blocks = %li\n", info->st_blocks);
  printf("info.st_blksize = %u\n", info->st_blksize);
  printf("info.st_flags = %u\n", info->st_flags);
  printf("info.st_gen = %u\n", info->st_gen);
  printf("info.st_lspare = %d\n", info->st_lspare);
  printf("info.st_birthtim.tv_sec = %li\n", info->st_birthtim.tv_sec);
  printf("info.st_birthtim.tv_nsec = %li\n", info->st_birthtim.tv_nsec);
}

TEST(Test, Test) {
  // Open assets dir, this helps test PFS directories.
  int32_t fd = sceKernelOpen("/app0/assets/misc", 0, 0511);
  CHECK_FALSE(fd < 0);

  // Run stat on PFS directory
  OrbisKernelStat info {};
  int32_t         result = sceKernelFstat(fd, &info);
  UNSIGNED_INT_EQUALS(0, result);

  // Log info from stat call.
  // TODO: checks for some important fields, like blocks, blksize, and size.
  PrintStatInfo(&info);

  // Run read on PFS directory
  char dirent_buf[0x400];
  memset(dirent_buf, 0, sizeof(dirent_buf));
  int64_t bytes = sceKernelRead(fd, dirent_buf, 0x200);

  // Read on directories always returns the inputted byte count.
  CHECK_EQUAL(0x200, bytes);

  // The raw data of a directory, as returned by read, is a list of dirent objects describing each file.
  int32_t num_entries   = 0;
  char*   current_entry = dirent_buf;
  while (current_entry < dirent_buf + bytes) {
    PfsDirent* entry = (PfsDirent*)current_entry;

    // Read will return the size of the buffer, regardless of contents.
    // To avoid garbage entries, if we are close enough to the end of the buffer, break the loop.
    if (current_entry > dirent_buf + bytes - 40) {
      // By time this is hit, we should have seen 17 entries.
      CHECK_EQUAL(17, num_entries);
      break;
    }

    if (num_entries == 0) {
      // First entry should be the entry for .
      int32_t comp = strncmp(".", entry->d_name, 1);
      CHECK_EQUAL(0, comp);
      CHECK_EQUAL(4, entry->d_type);
    } else if (num_entries == 1) {
      // Second entry should be the entry for ..
      int32_t comp = strncmp("..", entry->d_name, 2);
      CHECK_EQUAL(0, comp);
      CHECK_EQUAL(5, entry->d_type);
    } else {
      // Everything else are basic files, these use type 2.
      CHECK_EQUAL(2, entry->d_type);
    }

    // By this point, we can safely assume the entry data is valid.
    // Check for proper namlen based on name
    int32_t expected_namlen = strlen(entry->d_name);
    CHECK_EQUAL(expected_namlen, entry->d_namlen);

    // Check for proper reclen based on namlem.
    // For a PFS directory entry, this will be four ints + name length, aligned up to the nearest 8 bytes.
    int32_t expected_reclen = ((entry->d_namlen / 8) * 8) + 24;
    CHECK_EQUAL(expected_reclen, entry->d_reclen);

    printf("name = %s\n", entry->d_name);
    printf("    fileno = %u\n", entry->d_fileno);
    printf("    reclen = %u\n", entry->d_reclen);
    printf("    type = %u\n", entry->d_type);
    printf("    namlen = %u\n", entry->d_namlen);

    // d_reclen is the length of the returned record, use it to jump to the next one.
    current_entry += entry->d_reclen;

    // Keep track of returned entries.
    num_entries++;
  }

  // One more read will read the rest of the directory contents, starting from the current file position (0x200)
  // To make sure data is completely valid, store this data at buffer index 0x200.
  bytes = sceKernelRead(fd, &dirent_buf[0x200], 0x200);

  // Read on directories always returns the inputted byte count.
  CHECK_EQUAL(0x200, bytes);

  // The raw data of a directory, as returned by read, is a list of dirent objects describing each file.
  while (current_entry < &dirent_buf[0x200] + bytes) {
    // Current entry still points to the partial entry from last read.
    // If this read worked properly, this entry will be complete now.
    PfsDirent* entry = (PfsDirent*)current_entry;

    // This read shouldn't result in any partial data, so once we're past the end of the directory data
    // We will have an entry of all zeros.
    if (entry->d_reclen == 0) {
      // By time this is hit, we should have seen 20 entries.
      CHECK_EQUAL(20, num_entries);
      break;
    }

    // Everything we see should be a basic file, which uses type 2.
    CHECK_EQUAL(2, entry->d_type);

    // By this point, we can safely assume the entry data is valid.
    // Check for proper namlen based on name
    int32_t expected_namlen = strlen(entry->d_name);
    CHECK_EQUAL(expected_namlen, entry->d_namlen);

    // Check for proper reclen based on namlem.
    // For a PFS directory entry, this will be four ints + name length, aligned up to the nearest 8 bytes.
    int32_t expected_reclen = ((entry->d_namlen / 8) * 8) + 24;
    CHECK_EQUAL(expected_reclen, entry->d_reclen);

    printf("name = %s\n", entry->d_name);
    printf("    fileno = %u\n", entry->d_fileno);
    printf("    reclen = %u\n", entry->d_reclen);
    printf("    type = %u\n", entry->d_type);
    printf("    namlen = %u\n", entry->d_namlen);

    // d_reclen is the length of the returned record, use it to jump to the next one.
    current_entry += entry->d_reclen;

    // Keep track of returned entries.
    num_entries++;
  }

  result = sceKernelClose(fd);
  UNSIGNED_INT_EQUALS(0, result);

  // TODO: Test /savedata0, /temp0, /download0 results.
}
