#include "common/secret/secret_manager_impl.h"

#include "envoy/common/exception.h"

#include "common/ssl/tls_certificate_config_impl.h"

namespace Envoy {
namespace Secret {
namespace {

std::string configSourceHash(const envoy::api::v2::core::ConfigSource& config_source) {
  std::string jsonstr;
  if (Protobuf::util::MessageToJsonString(config_source, &jsonstr).ok()) {
    auto obj = Json::Factory::loadFromString(jsonstr);
    if (obj.get() != nullptr) {
      return std::to_string(obj->hash());
    }
  }
  throw EnvoyException(
      fmt::format("Invalid ConfigSource message: {}", config_source.DebugString()));
}

} // namespace

void SecretManagerImpl::addStaticSecret(const envoy::api::v2::auth::Secret& secret) {
  switch (secret.type_case()) {
  case envoy::api::v2::auth::Secret::TypeCase::kTlsCertificate: {
    std::unique_lock<std::shared_timed_mutex> lhs(static_tls_certificate_secrets_mutex_);
    static_tls_certificate_secrets_[secret.name()] =
        std::make_unique<Ssl::TlsCertificateConfigImpl>(secret.tls_certificate());
  } break;
  default:
    throw EnvoyException("Secret type not implemented");
  }
}

const Ssl::TlsCertificateConfig*
SecretManagerImpl::findStaticTlsCertificate(const std::string& name) const {
  std::shared_lock<std::shared_timed_mutex> lhs(static_tls_certificate_secrets_mutex_);

  auto secret = static_tls_certificate_secrets_.find(name);
  return (secret != static_tls_certificate_secrets_.end()) ? secret->second.get() : nullptr;
}

DynamicSecretProviderSharedPtr SecretManagerImpl::findOrCreateDynamicSecretProvider(
    const envoy::api::v2::core::ConfigSource& sds_config_source, std::string config_name) {
  auto hash = configSourceHash(sds_config_source);
  std::string map_key = hash + config_name;

  std::unique_lock<std::shared_timed_mutex> lhs(dynamic_secret_providers_mutex_);
  auto dynamic_secret_provider = dynamic_secret_providers_[map_key].lock();
  if (!dynamic_secret_provider) {
    dynamic_secret_provider = std::make_shared<SdsApi>(server_, sds_config_source, config_name);
    dynamic_secret_providers_[map_key] = dynamic_secret_provider;
  }

  return dynamic_secret_provider;
}

} // namespace Secret
} // namespace Envoy
