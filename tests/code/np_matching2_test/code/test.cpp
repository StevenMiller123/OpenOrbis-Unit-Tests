#include "test.h"

#include "CppUTest/TestHarness.h"

TEST_GROUP (NpMatching2Test) {
  void setup() {}
  void teardown() {}
};

static bool g_context_started = false;

TEST(NpMatching2Test, Test) {
  // Init libSceUserService
  OrbisUserServiceInitializeParams user_service_init_params{};
  user_service_init_params.priority = 0x2ff;
  int32_t result = sceUserServiceInitialize(&user_service_init_params);
  UNSIGNED_INT_EQUALS(0, result);

  // Get user_id
  int32_t user_id = 0;
  result = sceUserServiceGetInitialUser(&user_id);
  UNSIGNED_INT_EQUALS(0, result);

  // Load libSceNpMatching2
  result = sceSysmoduleLoadModule(ORBIS_SYSMODULE_NP_MATCHING2);
  UNSIGNED_INT_EQUALS(0, result);

  // Initialize libSceNpMatching2
  OrbisNpMatching2InitializeParameter init_params{};
  memset(&init_params, 0, sizeof(init_params));
  init_params.struct_size = sizeof(init_params);

  result = sceNpMatching2Initialize(&init_params);
  UNSIGNED_INT_EQUALS(0, result);
  printf("sceNpMatching2Initialize returns 0x%08x\n", result);

  // Create NpMatching2 context
  OrbisNpMatching2CreateContextParamA ctx_params{};
  memset(&ctx_params, 0, sizeof(ctx_params));
  ctx_params.struct_size = sizeof(ctx_params);
  ctx_params.user_id = user_id;

  uint16_t ctx_id = 0;
  result = sceNpMatching2CreateContextA(&ctx_params, &ctx_id);
  UNSIGNED_INT_EQUALS(0, result);
  printf("sceNpMatching2CreateContextA returns 0x%08x\n", result);

  // Start the context
  result = sceNpMatching2ContextStart(ctx_id, 0);
  UNSIGNED_INT_EQUALS(0, result);
  printf("sceNpMatching2ContextStart returns 0x%08x\n", result);

  // Register a callback, this will fire when the context is ready, or if context creation fails.
  OrbisNpMatching2ContextCallback cb_func = [](uint16_t context_id, uint16_t event, uint8_t event_cause, int32_t error_code, void* user_arg) {
    printf("Context callback called!\n");
    printf("context_id = 0x%04x, event = 0x%04x, cause = 0x%02x, error_code = 0x%08x\n", context_id, event, event_cause, error_code);
    g_context_started = true;

    int32_t* error_out = (int32_t*)user_arg;
    *error_out = error_code;
  };
  int32_t error_code = 0;
  result = sceNpMatching2RegisterContextCallback(cb_func, &error_code);
  UNSIGNED_INT_EQUALS(0, result);
  printf("sceNpMatching2RegisterContextCallback returns 0x%08x\n", result);

  // Wait as long as necessary for the result.
  while (!g_context_started) {
    sceKernelUsleep(100000);
  }

  // Context creation succeeded, stop before destroying
  if (error_code >= 0) {
    result = sceNpMatching2ContextStop(ctx_id);
    printf("sceNpMatching2ContextStop returns 0x%08x\n", result);
  }

  // Destroy context
  result = sceNpMatching2DestroyContext(ctx_id);
  printf("sceNpMatching2DestroyContext returns 0x%08x\n", result);

  // Terminate libSceNpMatching2
  result = sceNpMatching2Terminate();
  printf("sceNpMatching2Terminate returns 0x%08x\n", result);

  // Unload libSceNpMatching2
  result = sceSysmoduleUnloadModule(ORBIS_SYSMODULE_NP_MATCHING2);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceUserServiceTerminate();
  UNSIGNED_INT_EQUALS(0, result);
}