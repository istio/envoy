#include "common/ssl/context_manager_impl.h"

#include <functional>
#include <shared_mutex>

#include "common/common/assert.h"
#include "common/ssl/context_impl.h"

namespace Envoy {
namespace Ssl {

ContextManagerImpl::~ContextManagerImpl() { ASSERT(contexts_.empty()); }

void ContextManagerImpl::releaseContext(Context* context) {
  std::unique_lock<std::shared_timed_mutex> lock(contexts_lock_);

  // context may not be found, in the case that a subclass of Context throws
  // in it's constructor. In that case the context did not get added, but
  // the destructor of Context will run and call releaseContext().
  contexts_.erase(context);
}

ClientContextPtr ContextManagerImpl::createSslClientContext(Stats::Scope& scope,
                                                            const ClientContextConfig& config) {
  if (!config.isValid()) {
    return nullptr;
  }

  ClientContextPtr context(new ClientContextImpl(*this, scope, config));
  std::unique_lock<std::shared_timed_mutex> lock(contexts_lock_);
  contexts_.emplace(context.get());
  return context;
}

Ssl::ClientContextPtr
ContextManagerImpl::updateSslClientContext(const Ssl::ClientContextPtr& client_context,
                                           Stats::Scope& scope, const ClientContextConfig& config) {
  std::unique_lock<std::shared_timed_mutex> lock(contexts_lock_);

  if (contexts_.erase(client_context.get()) == 0) {
    return nullptr;
  }

  ClientContextPtr context(new ClientContextImpl(*this, scope, config));
  contexts_.emplace(context.get());

  return context;
}

ServerContextPtr
ContextManagerImpl::createSslServerContext(Stats::Scope& scope, const ServerContextConfig& config,
                                           const std::vector<std::string>& server_names) {
  if (!config.isValid()) {
    return nullptr;
  }

  ServerContextPtr context(new ServerContextImpl(*this, scope, config, server_names, runtime_));
  std::unique_lock<std::shared_timed_mutex> lock(contexts_lock_);
  contexts_.emplace(context.get());
  return context;
}

ServerContextPtr
ContextManagerImpl::updateSslServerContext(const ServerContextPtr& server_context,
                                           Stats::Scope& scope, const ServerContextConfig& config,
                                           const std::vector<std::string>& server_names) {
  std::unique_lock<std::shared_timed_mutex> lock(contexts_lock_);

  if (contexts_.erase(server_context.get()) == 0) {
    return nullptr;
  }

  ServerContextPtr context(new ServerContextImpl(*this, scope, config, server_names, runtime_));
  contexts_.emplace(context.get());

  return context;
}

size_t ContextManagerImpl::daysUntilFirstCertExpires() const {
  std::shared_lock<std::shared_timed_mutex> lock(contexts_lock_);
  size_t ret = std::numeric_limits<int>::max();
  for (Context* context : contexts_) {
    ret = std::min<size_t>(context->daysUntilFirstCertExpires(), ret);
  }
  return ret;
}

void ContextManagerImpl::iterateContexts(std::function<void(const Context&)> callback) {
  std::shared_lock<std::shared_timed_mutex> lock(contexts_lock_);
  for (Context* context : contexts_) {
    callback(*context);
  }
}

} // namespace Ssl
} // namespace Envoy
