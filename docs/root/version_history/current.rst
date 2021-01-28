1.16.3 (Pending)
================

Incompatible Behavior Changes
-----------------------------
*Changes that are expected to cause an incompatibility if applicable; deployment changes are likely required*

Minor Behavior Changes
----------------------
*Changes that may cause incompatibilities for some users, but should not for most*

* adaptive concurrency: added a response body / grpc-message header for rejected requests.
* async_client: minor change to handling header only responses more similar to header-with-empty-body responses.
* build: an :ref:`Ubuntu based debug image <install_binaries>` is built and published in DockerHub.
* build: the debug information will be generated separately to reduce target size and reduce compilation time when build in compilation mode `dbg` and `opt`. Users will need to build dwp file to debug with gdb.
* compressor: always insert `Vary` headers for compressible resources even if it's decided not to compress a response due to incompatible `Accept-Encoding` value. The `Vary` header needs to be inserted to let a caching proxy in front of Envoy know that the requested resource still can be served with compression applied.
* decompressor: headers-only requests were incorrectly not advertising accept-encoding when configured to do so. This is now fixed.
* ext_authz filter: request timeout will now count from the time the check request is created, instead of when it becomes active. This makes sure that the timeout is enforced even if the ext_authz cluster's circuit breaker is engaged.
  This behavior can be reverted by setting runtime feature `envoy.reloadable_features.ext_authz_measure_timeout_on_check_created` to false. When enabled, a new `ext_authz.timeout` stat is counted when timeout occurs. See :ref:`stats <config_http_filters_ext_authz_stats>`.
* grpc reverse bridge: upstream headers will no longer be propagated when the response is missing or contains an unexpected content-type.
* http: added :ref:`contains <envoy_api_msg_type.matcher.StringMatcher>`, a new string matcher type which matches if the value of the string has the substring mentioned in contains matcher.
* http: added :ref:`contains <envoy_api_msg_route.HeaderMatcher>`, a new header matcher type which matches if the value of the header has the substring mentioned in contains matcher.
* http: added :ref:`headers_to_add <envoy_v3_api_field_extensions.filters.network.http_connection_manager.v3.ResponseMapper.headers_to_add>` to :ref:`local reply mapper <config_http_conn_man_local_reply>` to allow its users to add/append/override response HTTP headers to local replies.
* http: added HCM level configuration of :ref:`error handling on invalid messaging <envoy_v3_api_field_extensions.filters.network.http_connection_manager.v3.HttpConnectionManager.stream_error_on_invalid_http_message>` which substantially changes Envoy's behavior when encountering invalid HTTP/1.1 defaulting to closing the connection instead of allowing reuse. This can temporarily be reverted by setting `envoy.reloadable_features.hcm_stream_error_on_invalid_message` to false, or permanently reverted by setting the HTTP/1 configuration :ref:`override_stream_error_on_invalid_http_message <envoy_v3_api_field_config.core.v3.Http1ProtocolOptions.override_stream_error_on_invalid_http_message>` to true to restore prior HTTP/1.1 behavior (i.e. connection isn't terminated) and to retain prior HTTP/2 behavior (i.e. connection is terminated).
* http: added HCM level configuration of :ref:`error handling on invalid messaging <envoy_v3_api_field_extensions.filters.network.http_connection_manager.v3.HttpConnectionManager.stream_error_on_invalid_http_message>` which substantially changes Envoy's behavior when encountering invalid HTTP/1.1 defaulting to closing the connection instead of allowing reuse. This can temporarily be reverted by setting `envoy.reloadable_features.hcm_stream_error_on_invalid_message` to false, or permanently reverted by setting the :ref:`HCM option <envoy_v3_api_field_extensions.filters.network.http_connection_manager.v3.HttpConnectionManager.stream_error_on_invalid_http_message>` to true to restore prior HTTP/1.1 beavior and setting the *new* HTTP/2 configuration :ref:`override_stream_error_on_invalid_http_message <envoy_v3_api_field_config.core.v3.Http2ProtocolOptions.override_stream_error_on_invalid_http_message>` to false to retain prior HTTP/2 behavior.
* http: applying route level header modifications to local replies sent on that route. This behavior may be temporarily reverted by setting `envoy.reloadable_features.always_apply_route_header_rules` to false.
* http: changed Envoy to send GOAWAY to HTTP2 downstreams when the :ref:`disable_keepalive <config_overload_manager_overload_actions>` overload action is active. This behavior may be temporarily reverted by setting `envoy.reloadable_features.overload_manager_disable_keepalive_drain_http2` to false.
* http: changed Envoy to send error headers and body when possible. This behavior may be temporarily reverted by setting `envoy.reloadable_features.allow_response_for_timeout` to false.
* http: changed empty trailers encoding behavior by sending empty data with ``end_stream`` true (instead of sending empty trailers) for HTTP/2. This behavior can be reverted temporarily by setting runtime feature `envoy.reloadable_features.http2_skip_encoding_empty_trailers` to false.
* http: changed how local replies are processed for requests which transform from grpc to not-grpc, or not-grpc to grpc. Previously the initial generated reply depended on which filter sent the reply, but now the reply is consistently generated the way the downstream expects. This behavior can be temporarily reverted by setting `envoy.reloadable_features.unify_grpc_handling` to false.
* http: clarified and enforced 1xx handling. Multiple 100-continue headers are coalesced when proxying. 1xx headers other than {100, 101} are dropped.
* http: fixed a bug in access logs where early stream termination could be incorrectly tagged as a downstream disconnect, and disconnects after partial response were not flagged.
* http: fixed the 100-continue response path to properly handle upstream failure by sending 5xx responses. This behavior can be temporarily reverted by setting `envoy.reloadable_features.allow_500_after_100` to false.
* http: the per-stream FilterState maintained by the HTTP connection manager will now provide read/write access to the downstream connection FilterState. As such, code that relies on interacting with this might
  see a change in behavior.
* logging: added fine-grain logging for file level log control with logger management at administration interface. It can be enabled by option :option:`--enable-fine-grain-logging`.
* logging: changed default log format to `"[%Y-%m-%d %T.%e][%t][%l][%n] [%g:%#] %v"` and default value of :option:`--log-format-prefix-with-location` to `0`.
* logging: nghttp2 log messages no longer appear at trace level unless `ENVOY_NGHTTP2_TRACE` is set
  in the environment.
* lua: changed the response body returned by `httpCall()` API to raw data. Previously, the returned data was string.
* memory: switched to the `new tcmalloc <https://github.com/google/tcmalloc>`_ for linux_x86_64 builds. The `old tcmalloc <https://github.com/gperftools/gperftools>`_ can still be enabled with the `--define tcmalloc=gperftools` option.
* postgres: changed log format to tokenize fields of Postgres messages.
* router: added transport failure reason to response body when upstream reset happens. After this change, the response body will be of the form `upstream connect error or disconnect/reset before headers. reset reason:{}, transport failure reason:{}`.This behavior may be reverted by setting runtime feature `envoy.reloadable_features.http_transport_failure_reason_in_body` to false.
* router: now consumes all retry related headers to prevent them from being propagated to the upstream. This behavior may be reverted by setting runtime feature `envoy.reloadable_features.consume_all_retry_headers` to false.
* stats: the fake symbol table implemention has been removed from the binary, and the option `--use-fake-symbol-table` is now a no-op with a warning.
* thrift_proxy: special characters {'\0', '\r', '\n'} will be stripped from thrift headers.
* watchdog: replaced single watchdog with separate watchdog configuration for worker threads and for the main thread configured via :ref:`Watchdogs<envoy_v3_api_field_config.bootstrap.v3.Bootstrap.watchdogs>`. It works with :ref:`watchdog<envoy_v3_api_field_config.bootstrap.v3.Bootstrap.watchdog>` by having the worker thread and main thread watchdogs have same config.
* build: the Alpine based debug images are no longer built in CI, use Ubuntu based images instead.
* cluster manager: the cluster which can't extract secret entity by SDS to be warming and never activate. This feature is disabled by default and is controlled by runtime guard `envoy.reloadable_features.cluster_keep_warming_no_secret_entity`.
* expr filter: added `connection.termination_details` property support.
* ext_authz filter: disable `envoy.reloadable_features.ext_authz_measure_timeout_on_check_created` by default.
* ext_authz filter: the deprecated field :ref:`use_alpha <envoy_api_field_config.filter.http.ext_authz.v2.ExtAuthz.use_alpha>` is no longer supported and cannot be set anymore.
* grpc_web filter: if a `grpc-accept-encoding` header is present it's passed as-is to the upstream and if it isn't `grpc-accept-encoding:identity` is sent instead. The header was always overwriten with `grpc-accept-encoding:identity,deflate,gzip` before.
* watchdog: the watchdog action :ref:`abort_action <envoy_v3_api_msg_watchdog.v3alpha.AbortActionConfig>` is now the default action to terminate the process if watchdog kill / multikill is enabled.

Bug Fixes
---------
*Changes expected to improve the state of the world and are unlikely to have negative effects*

* csrf: fixed issues with regards to origin and host header parsing.
* dynamic_forward_proxy: only perform DNS lookups for routes to Dynamic Forward Proxy clusters since other cluster types handle DNS lookup themselves.
* fault: fixed an issue with `active_faults` gauge not being decremented for when abort faults were injected.
* fault: made the HeaderNameValues::prefix() method const.
* grpc-web: fixed an issue with failing HTTP/2 requests on some browsers. Notably, WebKit-based browsers (https://bugs.webkit.org/show_bug.cgi?id=210108), Internet Explorer 11, and Edge (pre-Chromium).
* http: fixed CVE-2020-25018 by rolling back the ``GURL`` dependency to previous state (reverted: ``2d69e30``, ``d828958``, and ``c9c4709`` commits) due to potential of crashing when Unicode URIs are present in requests.
* http: fixed bugs in datadog and squash filter's handling of responses with no bodies.
* http: made the HeaderValues::prefix() method const.
* jwt_authn: supports jwt payload without "iss" field.
* listener: fixed crash at listener inplace update when connetion load balancer is set.
* rocketmq_proxy: fixed an issue involving incorrect header lengths. In debug mode it causes crash and in release mode it causes underflow.
* thrift_proxy: fixed crashing bug on request overflow.
* udp_proxy: fixed a crash due to UDP packets being processed after listener removal.
* tls: fix detection of the upstream connection close event.
Bug Fixes
---------
*Changes expected to improve the state of the world and are unlikely to have negative effects*

* aggregate cluster: fixed a crash due to a TLS initialization issue.
* lua: fixed crash when Lua script contains streamInfo():downstreamSslConnection().
* tls: fix detection of the upstream connection close event.

Removed Config or Runtime
-------------------------
*Normally occurs at the end of the* :ref:`deprecation period <deprecated>`

New Features
------------
* config: added new runtime feature `envoy.features.enable_all_deprecated_features` that allows the use of all deprecated features.
* grpc: implemented header value syntax support when defining :ref:`initial metadata <envoy_v3_api_field_config.core.v3.GrpcService.initial_metadata>` for gRPC-based `ext_authz` :ref:`HTTP <envoy_v3_api_field_extensions.filters.http.ext_authz.v3.ExtAuthz.grpc_service>` and :ref:`network <envoy_v3_api_field_extensions.filters.network.ext_authz.v3.ExtAuthz.grpc_service>` filters, and :ref:`ratelimit <envoy_v3_api_field_config.ratelimit.v3.RateLimitServiceConfig.grpc_service>` filters.
* hds: added support for delta updates in the :ref:`HealthCheckSpecifier <envoy_v3_api_msg_service.health.v3.HealthCheckSpecifier>`, making only the Endpoints and Health Checkers that changed be reconstructed on receiving a new message, rather than the entire HDS.
* health_check: added option to use :ref:`no_traffic_healthy_interval <envoy_v3_api_field_config.core.v3.HealthCheck.no_traffic_healthy_interval>` which allows a different no traffic interval when the host is healthy.
* http: added HCM :ref:`timeout config field <envoy_v3_api_field_extensions.filters.network.http_connection_manager.v3.HttpConnectionManager.request_headers_timeout>` to control how long a downstream has to finish sending headers before the stream is cancelled.
* http: added frame flood and abuse checks to the upstream HTTP/2 codec. This check is off by default and can be enabled by setting the `envoy.reloadable_features.upstream_http2_flood_checks` runtime key to true.
* jwt_authn: added support for :ref:`per-route config <envoy_v3_api_msg_extensions.filters.http.jwt_authn.v3.PerRouteConfig>`.
* listener: added an optional :ref:`default filter chain <envoy_v3_api_field_config.listener.v3.Listener.default_filter_chain>`. If this field is supplied, and none of the :ref:`filter_chains <envoy_v3_api_field_config.listener.v3.Listener.filter_chains>` matches, this default filter chain is used to serve the connection.
* log: added a new custom flag ``%_`` to the log pattern to print the actual message to log, but with escaped newlines.
* lua: added `downstreamDirectRemoteAddress()` and `downstreamLocalAddress()` APIs to :ref:`streamInfo() <config_http_filters_lua_stream_info_wrapper>`.
* mongo_proxy: the list of commands to produce metrics for is now :ref:`configurable <envoy_v3_api_field_extensions.filters.network.mongo_proxy.v3.MongoProxy.commands>`.
* network: added a :ref:`timeout <envoy_v3_api_field_config.listener.v3.FilterChain.transport_socket_connect_timeout>` for incoming connections completing transport-level negotiation, including TLS and ALTS hanshakes.
* overload: add :ref:`envoy.overload_actions.reduce_timeouts <config_overload_manager_overload_actions>` overload action to enable scaling timeouts down with load.
* ratelimit: added support for use of various :ref:`metadata <envoy_v3_api_field_config.route.v3.RateLimit.Action.metadata>` as a ratelimit action.
* ratelimit: added :ref:`disable_x_envoy_ratelimited_header <envoy_v3_api_msg_extensions.filters.http.ratelimit.v3.RateLimit>` option to disable `X-Envoy-RateLimited` header.
* tcp: added a new :ref:`envoy.overload_actions.reject_incoming_connections <config_overload_manager_overload_actions>` action to reject incoming TCP connections.
* tls: added support for RSA certificates with 4096-bit keys in FIPS mode.
* tracing: added SkyWalking tracer.
* xds: added support for resource TTLs. A TTL is specified on the :ref:`Resource <envoy_api_msg_Resource>`. For SotW, a :ref:`Resource <envoy_api_msg_Resource>` can be embedded
  in the list of resources to specify the TTL.

Deprecated
----------
