// NOLINT(namespace-envoy)
#include <string>

#include "proxy_wasm_intrinsics.h"

static int *badptr = nullptr;

extern "C" EMSCRIPTEN_KEEPALIVE void proxy_onStart() {
  logError("before badptr");
  *badptr = 1;
  logError("after badptr");
}
