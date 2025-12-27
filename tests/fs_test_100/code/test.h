#include "CppUTest/TestHarness.h"

#define UNSIGNED_INT_EQUALS(expected, actual) UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, NULLPTR, __FILE__, __LINE__)

struct OrbisKernelTimespec {
  int64_t tv_sec;
  int64_t tv_nsec;
};

struct OrbisKernelStat {
  uint32_t            st_dev;
  uint32_t            st_ino;
  uint16_t            st_mode;
  uint16_t            st_nlink;
  uint32_t            st_uid;
  uint32_t            st_gid;
  uint32_t            st_rdev;
  OrbisKernelTimespec st_atim;
  OrbisKernelTimespec st_mtim;
  OrbisKernelTimespec st_ctim;
  int64_t             st_size;
  int64_t             st_blocks;
  uint32_t            st_blksize;
  uint32_t            st_flags;
  uint32_t            st_gen;
  int32_t             st_lspare;
  OrbisKernelTimespec st_birthtim;
};

struct PfsDirent {
  int32_t d_fileno;
  int32_t d_type;
  int32_t d_namlen;
  int32_t d_reclen;
  char    d_name[256];
};

struct FolderDirent {
  uint32_t d_fileno;
  uint16_t d_reclen;
  uint8_t  d_type;
  uint8_t  d_namlen;
  char     d_name[256];
};

struct OrbisAppContentMountPoint {
  char data[16];
};

struct OrbisSaveDataDirName {
  char data[32];
};

struct OrbisSaveDataMount2 {
  int32_t user_id;
  int32_t : 32;
  const OrbisSaveDataDirName* dir_name;
  uint64_t                    blocks;
  uint32_t                    mount_mode;
  char                        reserved_2[32];
  int32_t : 32;
};

struct OrbisSaveDataMountPoint {
  char data[16];
};

struct OrbisSaveDataMountResult {
  OrbisSaveDataMountPoint mount_point;
  uint64_t                required_blocks;
  uint32_t                unused;
  uint32_t                mount_status;
  char                    reserved[32];
  int32_t : 32;
};

// Function definitions (with modified types to improve testability)
extern "C" {
int32_t sceKernelOpen(const char* path, int32_t flags, int32_t mode);
int64_t sceKernelRead(int32_t fd, void* buffer, uint64_t size);
int32_t sceKernelClose(int32_t fd);
int32_t sceKernelFstat(int32_t fd, OrbisKernelStat* stat);
int64_t sceKernelGetdents(int32_t fd, void* buffer, uint64_t size);
int64_t sceKernelGetdirentries(int32_t fd, void* buffer, uint64_t size, int64_t* offset);
int64_t sceKernelLseek(int32_t fd, int64_t offset, int32_t whence);

int32_t sceAppContentTemporaryDataMount2(int32_t option, OrbisAppContentMountPoint* mount_point);

int32_t sceSaveDataMount2(const OrbisSaveDataMount2* option, OrbisSaveDataMountResult* result);

int32_t sceUserServiceGetInitialUser(int32_t* user_id);
}

// Some error codes
enum OrbisError : int32_t {
  ORBIS_KERNEL_ERROR_UNKNOWN         = int(0x80020000),
  ORBIS_KERNEL_ERROR_EPERM           = int(0x80020001),
  ORBIS_KERNEL_ERROR_ENOENT          = int(0x80020002),
  ORBIS_KERNEL_ERROR_ESRCH           = int(0x80020003),
  ORBIS_KERNEL_ERROR_EINTR           = int(0x80020004),
  ORBIS_KERNEL_ERROR_EIO             = int(0x80020005),
  ORBIS_KERNEL_ERROR_ENXIO           = int(0x80020006),
  ORBIS_KERNEL_ERROR_E2BIG           = int(0x80020007),
  ORBIS_KERNEL_ERROR_ENOEXEC         = int(0x80020008),
  ORBIS_KERNEL_ERROR_EBADF           = int(0x80020009),
  ORBIS_KERNEL_ERROR_ECHILD          = int(0x8002000A),
  ORBIS_KERNEL_ERROR_EDEADLK         = int(0x8002000B),
  ORBIS_KERNEL_ERROR_ENOMEM          = int(0x8002000C),
  ORBIS_KERNEL_ERROR_EACCES          = int(0x8002000D),
  ORBIS_KERNEL_ERROR_EFAULT          = int(0x8002000E),
  ORBIS_KERNEL_ERROR_ENOTBLK         = int(0x8002000F),
  ORBIS_KERNEL_ERROR_EBUSY           = int(0x80020010),
  ORBIS_KERNEL_ERROR_EEXIST          = int(0x80020011),
  ORBIS_KERNEL_ERROR_EXDEV           = int(0x80020012),
  ORBIS_KERNEL_ERROR_ENODEV          = int(0x80020013),
  ORBIS_KERNEL_ERROR_ENOTDIR         = int(0x80020014),
  ORBIS_KERNEL_ERROR_EISDIR          = int(0x80020015),
  ORBIS_KERNEL_ERROR_EINVAL          = int(0x80020016),
  ORBIS_KERNEL_ERROR_ENFILE          = int(0x80020017),
  ORBIS_KERNEL_ERROR_EMFILE          = int(0x80020018),
  ORBIS_KERNEL_ERROR_ENOTTY          = int(0x80020019),
  ORBIS_KERNEL_ERROR_ETXTBSY         = int(0x8002001A),
  ORBIS_KERNEL_ERROR_EFBIG           = int(0x8002001B),
  ORBIS_KERNEL_ERROR_ENOSPC          = int(0x8002001C),
  ORBIS_KERNEL_ERROR_ESPIPE          = int(0x8002001D),
  ORBIS_KERNEL_ERROR_EROFS           = int(0x8002001E),
  ORBIS_KERNEL_ERROR_EMLINK          = int(0x8002001F),
  ORBIS_KERNEL_ERROR_EPIPE           = int(0x80020020),
  ORBIS_KERNEL_ERROR_EDOM            = int(0x80020021),
  ORBIS_KERNEL_ERROR_ERANGE          = int(0x80020022),
  ORBIS_KERNEL_ERROR_EAGAIN          = int(0x80020023),
  ORBIS_KERNEL_ERROR_EWOULDBLOCK     = int(0x80020023),
  ORBIS_KERNEL_ERROR_EINPROGRESS     = int(0x80020024),
  ORBIS_KERNEL_ERROR_EALREADY        = int(0x80020025),
  ORBIS_KERNEL_ERROR_ENOTSOCK        = int(0x80020026),
  ORBIS_KERNEL_ERROR_EDESTADDRREQ    = int(0x80020027),
  ORBIS_KERNEL_ERROR_EMSGSIZE        = int(0x80020028),
  ORBIS_KERNEL_ERROR_EPROTOTYPE      = int(0x80020029),
  ORBIS_KERNEL_ERROR_ENOPROTOOPT     = int(0x8002002A),
  ORBIS_KERNEL_ERROR_EPROTONOSUPPORT = int(0x8002002B),
  ORBIS_KERNEL_ERROR_ESOCKTNOSUPPORT = int(0x8002002C),
  ORBIS_KERNEL_ERROR_ENOTSUP         = int(0x8002002D),
  ORBIS_KERNEL_ERROR_EOPNOTSUPP      = int(0x8002002D),
  ORBIS_KERNEL_ERROR_EPFNOSUPPORT    = int(0x8002002E),
  ORBIS_KERNEL_ERROR_EAFNOSUPPORT    = int(0x8002002F),
  ORBIS_KERNEL_ERROR_EADDRINUSE      = int(0x80020030),
  ORBIS_KERNEL_ERROR_EADDRNOTAVAIL   = int(0x80020031),
  ORBIS_KERNEL_ERROR_ENETDOWN        = int(0x80020032),
  ORBIS_KERNEL_ERROR_ENETUNREACH     = int(0x80020033),
  ORBIS_KERNEL_ERROR_ENETRESET       = int(0x80020034),
  ORBIS_KERNEL_ERROR_ECONNABORTED    = int(0x80020035),
  ORBIS_KERNEL_ERROR_ECONNRESET      = int(0x80020036),
  ORBIS_KERNEL_ERROR_ENOBUFS         = int(0x80020037),
  ORBIS_KERNEL_ERROR_EISCONN         = int(0x80020038),
  ORBIS_KERNEL_ERROR_ENOTCONN        = int(0x80020039),
  ORBIS_KERNEL_ERROR_ESHUTDOWN       = int(0x8002003A),
  ORBIS_KERNEL_ERROR_ETOOMANYREFS    = int(0x8002003B),
  ORBIS_KERNEL_ERROR_ETIMEDOUT       = int(0x8002003C),
  ORBIS_KERNEL_ERROR_ECONNREFUSED    = int(0x8002003D),
  ORBIS_KERNEL_ERROR_ELOOP           = int(0x8002003E),
  ORBIS_KERNEL_ERROR_ENAMETOOLONG    = int(0x8002003F),
  ORBIS_KERNEL_ERROR_EHOSTDOWN       = int(0x80020040),
  ORBIS_KERNEL_ERROR_EHOSTUNREACH    = int(0x80020041),
  ORBIS_KERNEL_ERROR_ENOTEMPTY       = int(0x80020042),
  ORBIS_KERNEL_ERROR_EPROCLIM        = int(0x80020043),
  ORBIS_KERNEL_ERROR_EUSERS          = int(0x80020044),
  ORBIS_KERNEL_ERROR_EDQUOT          = int(0x80020045),
  ORBIS_KERNEL_ERROR_ESTALE          = int(0x80020046),
  ORBIS_KERNEL_ERROR_EREMOTE         = int(0x80020047),
  ORBIS_KERNEL_ERROR_EBADRPC         = int(0x80020048),
  ORBIS_KERNEL_ERROR_ERPCMISMATCH    = int(0x80020049),
  ORBIS_KERNEL_ERROR_EPROGUNAVAIL    = int(0x8002004A),
  ORBIS_KERNEL_ERROR_EPROGMISMATCH   = int(0x8002004B),
  ORBIS_KERNEL_ERROR_EPROCUNAVAIL    = int(0x8002004C),
  ORBIS_KERNEL_ERROR_ENOLCK          = int(0x8002004D),
  ORBIS_KERNEL_ERROR_ENOSYS          = int(0x8002004E),
  ORBIS_KERNEL_ERROR_EFTYPE          = int(0x8002004F),
  ORBIS_KERNEL_ERROR_EAUTH           = int(0x80020050),
  ORBIS_KERNEL_ERROR_ENEEDAUTH       = int(0x80020051),
  ORBIS_KERNEL_ERROR_EIDRM           = int(0x80020052),
  ORBIS_KERNEL_ERROR_ENOMSG          = int(0x80020053),
  ORBIS_KERNEL_ERROR_EOVERFLOW       = int(0x80020054),
  ORBIS_KERNEL_ERROR_ECANCELED       = int(0x80020055),
  ORBIS_KERNEL_ERROR_EILSEQ          = int(0x80020056),
  ORBIS_KERNEL_ERROR_ENOATTR         = int(0x80020057),
  ORBIS_KERNEL_ERROR_EDOOFUS         = int(0x80020058),
  ORBIS_KERNEL_ERROR_EBADMSG         = int(0x80020059),
  ORBIS_KERNEL_ERROR_EMULTIHOP       = int(0x8002005A),
  ORBIS_KERNEL_ERROR_ENOLINK         = int(0x8002005B),
  ORBIS_KERNEL_ERROR_EPROTO          = int(0x8002005C),
  ORBIS_KERNEL_ERROR_ENOTCAPABLE     = int(0x8002005D),
  ORBIS_KERNEL_ERROR_ECAPMODE        = int(0x8002005E),
  ORBIS_KERNEL_ERROR_ENOBLK          = int(0x8002005F),
  ORBIS_KERNEL_ERROR_EICV            = int(0x80020060),
  ORBIS_KERNEL_ERROR_ENOPLAYGOENT    = int(0x80020061)
};