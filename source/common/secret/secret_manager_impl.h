#pragma once

#include <shared_mutex>
#include <unordered_map>

#include "envoy/secret/secret_manager.h"
#include "envoy/server/instance.h"
#include "envoy/ssl/tls_certificate_config.h"

#include "common/common/logger.h"
#include "common/secret/sds_api.h"

namespace Envoy {
namespace Secret {

class SecretManagerImpl : public SecretManager, Logger::Loggable<Logger::Id::upstream> {
public:
  SecretManagerImpl(Server::Instance& server) : server_(server) {}

  void addStaticSecret(const envoy::api::v2::auth::Secret& secret) override;
  const Ssl::TlsCertificateConfigSharedPtr
  findStaticTlsCertificate(const std::string& name) const override;

  std::string addOrUpdateSdsService(const envoy::api::v2::core::ConfigSource& config_source,
                                    std::string config_name) override;

  virtual DynamicSecretProviderSharedPtr createDynamicSecretProvider(const envoy::api::v2::core::ConfigSource& config_source,
						     std::string config_name) override;

private:
  Server::Instance& server_;

  // map hash code of SDS config source and SdsApi object.
  std::unordered_map<std::string, std::weak_ptr<DynamicSecretProvider>> sds_apis_;
  mutable std::shared_timed_mutex sds_api_mutex_;

  // Manages pairs of name and Ssl::TlsCertificateConfig grouped by SDS config source hash.
  // If SDS config source hash is empty, it is a static secret.
  std::unordered_map<std::string, Ssl::TlsCertificateConfigSharedPtr>
    tls_certificate_secrets_;
  mutable std::shared_timed_mutex tls_certificate_secrets_mutex_;

  // callback functions for secret update
  // "config source hash": {
  //   "secret name":
  //      secret,
  //      [{callback}]
  //   ]
  // }
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::pair<Ssl::TlsCertificateConfigSharedPtr,
                                                               std::vector<SecretCallbacks*>>>>
      tls_certificate_secret_update_callbacks_;
  mutable std::shared_timed_mutex tls_certificate_secret_update_callbacks_mutex_;
};

} // namespace Secret
} // namespace Envoy
