#include "CppUTest/TestHarness.h"

#define UNSIGNED_INT_EQUALS(expected, actual) UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, NULLPTR, __FILE__, __LINE__)

struct OrbisUserServiceInitializeParams {
  int32_t priority;
};

#define ORBIS_SYSMODULE_NP_MATCHING2 (0x1c)

struct OrbisNpMatching2InitializeParameter {
  uint64_t pool_size;
  uint64_t cpu_mask;
  int32_t  thread_priority;
  uint8_t  padding[4];
  uint64_t thread_stack_size;
  uint64_t struct_size;
  uint64_t ssl_pool_size;
};

struct OrbisNpMatching2CreateContextParamA {
  int32_t  user_id;
  uint32_t np_service_label;
  uint64_t struct_size;
};

typedef void (*OrbisNpMatching2ContextCallback)(uint16_t context_id, uint16_t event, uint8_t event_cause, int32_t error_code, void* user_arg);

extern "C" {
int32_t sceKernelUsleep(uint32_t micros);

int32_t sceUserServiceInitialize(const OrbisUserServiceInitializeParams* params);
int32_t sceUserServiceGetInitialUser(int32_t* user_id);
int32_t sceUserServiceTerminate();

int32_t sceSysmoduleLoadModule(uint32_t module_id);
int32_t sceSysmoduleUnloadModule(uint32_t module_id);

int32_t sceNpMatching2Initialize(const OrbisNpMatching2InitializeParameter* param);
int32_t sceNpMatching2CreateContextA(const OrbisNpMatching2CreateContextParamA* param, uint16_t* context_id);
int32_t sceNpMatching2ContextStart(uint16_t context_id, uint64_t timeout);
int32_t sceNpMatching2RegisterContextCallback(OrbisNpMatching2ContextCallback cb_func, void* user_arg);
int32_t sceNpMatching2ContextStop(uint16_t context_id);
int32_t sceNpMatching2DestroyContext(uint16_t context_id);
int32_t sceNpMatching2Terminate();
}