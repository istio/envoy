#include "extensions/wasm/config.h"

#include <stdio.h>

#include "envoy/registry/registry.h"

#include "common/common/empty_string.h"
#include "common/config/datasource.h"
#include "common/protobuf/utility.h"

#include "extensions/common/wasm/wasm.h"

namespace Envoy {
namespace Extensions {
namespace Wasm {

static const std::string INLINE_STRING = "<inline>";

Server::WasmPtr WasmFactory::createWasm(const envoy::config::wasm::v2::WasmConfig& config,
                                        Server::Configuration::WasmFactoryContext& context) {
  auto wasm =
      Common::Wasm::createWasmUnique(config.id(), config.vm_config(), context.clusterManager(),
                                     context.dispatcher(), context.api());
  if (config.singleton()) {
    // Return the WASM VM which will be stored as a singleton by the Server.
    return wasm;
  }
  auto configuration = std::make_shared<std::string>(config.configuration());
  auto base_wasm = std::shared_ptr<Common::Wasm::Wasm>(wasm.release());
  // Per-thread WASM VM.
  // NB: the Slot set() call doesn't complete inline, so all arguments must outlive this call.
  // NB: no need to keep the resulting slot as Wasm is cached on each thread.
  context.threadLocal().allocateSlot()->set(
      [base_wasm, configuration](Event::Dispatcher& dispatcher) {
        auto wasm =
            Extensions::Common::Wasm::createThreadLocalWasm(*base_wasm, *configuration, dispatcher);
        return wasm;
      });
  // Do not return this WASM VM since this is per-thread. Returning it would indicate that this is a
  // singleton.
  return nullptr;
}

/**
 * Static registration for the wasm factory. @see RegistryFactory.
 */
static Registry::RegisterFactory<WasmFactory, Server::Configuration::WasmFactory> registered_;

} // namespace Wasm
} // namespace Extensions
} // namespace Envoy
