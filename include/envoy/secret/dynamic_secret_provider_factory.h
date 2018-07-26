#pragma once

#include "envoy/api/v2/core/config_source.pb.h"
#include "envoy/event/dispatcher.h"
#include "envoy/local_info/local_info.h"
#include "envoy/runtime/runtime.h"
#include "envoy/secret/dynamic_secret_provider.h"
#include "envoy/stats/stats.h"
#include "envoy/upstream/cluster_manager.h"

namespace Envoy {
namespace Secret {

/**
 * Factory for creating dynamic TlsCertificate secret provider.
 */
class DynamicTlsCertificateSecretProviderFactory {
public:
  virtual ~DynamicTlsCertificateSecretProviderFactory() {}

  /**
   * Finds and returns a secret provider associated to SDS config. Create a new one
   * if such provider does not exist.
   *
   * @param config_source a protobuf message object contains SDS config source.
   * @param config_name a name that uniquely refers to the SDS config source.
   * @return the dynamic tls certificate secret provider.
   */
  virtual DynamicTlsCertificateSecretProviderSharedPtr
  findOrCreate(const envoy::api::v2::core::ConfigSource& sds_config,
               std::string sds_config_name) PURE;
};

typedef std::unique_ptr<DynamicTlsCertificateSecretProviderFactory>
    DynamicTlsCertificateSecretProviderFactoryPtr;

} // namespace Secret
} // namespace Envoy