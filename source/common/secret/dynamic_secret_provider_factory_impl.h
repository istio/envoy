#pragma once

#include <functional>

#include "envoy/init/init.h"
#include "envoy/secret/dynamic_secret_provider.h"
#include "envoy/secret/dynamic_secret_provider_factory.h"
#include "envoy/secret/secret_manager.h"

#include "common/secret/sds_api.h"

namespace Envoy {
namespace Secret {

class DynamicTlsCertificateSecretProviderFactoryImpl
    : public DynamicTlsCertificateSecretProviderFactory {
public:
  DynamicTlsCertificateSecretProviderFactoryImpl(const LocalInfo::LocalInfo& local_info,
                                                 Event::Dispatcher& dispatcher,
                                                 Runtime::RandomGenerator& random,
                                                 Stats::Store& stats,
                                                 Upstream::ClusterManager& cluster_manager,
                                                 Secret::SecretManager& secret_manager,
                                                 Init::Manager& init_manager)
      : local_info_(local_info), dispatcher_(dispatcher), random_(random), stats_(stats),
        cluster_manager_(cluster_manager), secret_manager_(secret_manager),
        init_manager_(init_manager) {}

  DynamicTlsCertificateSecretProviderSharedPtr
  findOrCreate(const envoy::api::v2::core::ConfigSource& sds_config,
               std::string sds_config_name) override {
    auto secret_provider =
        secret_manager_.findDynamicTlsCertificateSecretProvider(sds_config, sds_config_name);
    if (!secret_provider) {
      secret_provider = std::make_shared<Secret::SdsApi>(local_info_, dispatcher_, random_, stats_,
                                                         cluster_manager_, init_manager_,
                                                         sds_config, sds_config_name);
      secret_manager_.setDynamicTlsCertificateSecretProvider(sds_config, sds_config_name,
                                                             secret_provider);
    }
    return secret_provider;
  }

private:
  const LocalInfo::LocalInfo& local_info_;
  Event::Dispatcher& dispatcher_;
  Runtime::RandomGenerator& random_;
  Stats::Store& stats_;
  Upstream::ClusterManager& cluster_manager_;
  Secret::SecretManager& secret_manager_;
  Init::Manager& init_manager_;
};

} // namespace Secret
} // namespace Envoy