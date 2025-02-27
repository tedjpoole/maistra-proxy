#include "source/common/common/base64.h"
#include "source/common/crypto/utility.h"
#include "source/common/http/utility.h"
#include "source/common/protobuf/utility.h"
#include "source/extensions/filters/http/oauth2/filter.h"
#include "source/extensions/filters/http/oauth2/oauth_response.pb.h"

#include "test/integration/http_integration.h"

#include "absl/strings/escaping.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Oauth2 {
namespace {

class OauthIntegrationTest : public HttpIntegrationTest,
                             public Grpc::GrpcClientIntegrationParamTest {
public:
  OauthIntegrationTest()
      : HttpIntegrationTest(Http::CodecType::HTTP2, Network::Address::IpVersion::v4) {
    skip_tag_extraction_rule_check_ = true;
    enableHalfClose(true);
  }
  envoy::service::discovery::v3::DiscoveryResponse genericSecretResponse(absl::string_view name,
                                                                         absl::string_view value) {
    envoy::extensions::transport_sockets::tls::v3::Secret secret;
    secret.set_name(std::string(name));
    secret.mutable_generic_secret()->mutable_secret()->set_inline_string(std::string(value));

    envoy::service::discovery::v3::DiscoveryResponse response_pb;
    response_pb.add_resources()->PackFrom(secret);
    response_pb.set_type_url(
        envoy::extensions::transport_sockets::tls::v3::Secret::descriptor()->name());
    return response_pb;
  }

  void createLdsStream() {
    AssertionResult result =
        fake_upstreams_[1]->waitForHttpConnection(*dispatcher_, lds_connection_);
    EXPECT_TRUE(result);

    auto result2 = lds_connection_->waitForNewStream(*dispatcher_, lds_stream_);
    EXPECT_TRUE(result2);
    lds_stream_->startGrpcStream();
    ASSERT_NE(nullptr, lds_stream_);
  }

  void sendLdsResponse(const std::vector<envoy::config::listener::v3::Listener>& listener_configs,
                       const std::string& version) {
    envoy::service::discovery::v3::DiscoveryResponse response;
    response.set_version_info(version);
    response.set_type_url(Config::TypeUrl::get().Listener);
    for (const auto& listener_config : listener_configs) {
      response.add_resources()->PackFrom(listener_config);
    }
    ASSERT_NE(nullptr, lds_stream_);
    lds_stream_->sendGrpcMessage(response);
  }
  FakeUpstream& getLdsFakeUpstream() const { return *fake_upstreams_[1]; }

  void sendLdsResponse(const std::vector<std::string>& listener_configs,
                       const std::string& version) {
    std::vector<envoy::config::listener::v3::Listener> proto_configs;
    proto_configs.reserve(listener_configs.size());
    for (const auto& listener_blob : listener_configs) {
      proto_configs.emplace_back(
          TestUtility::parseYaml<envoy::config::listener::v3::Listener>(listener_blob));
    }
    sendLdsResponse(proto_configs, version);
  }

  void setUpGrpcLds() {
    config_helper_.addConfigModifier([this](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
      listener_config_.Swap(bootstrap.mutable_static_resources()->mutable_listeners(0));
      listener_config_.set_name(listener_name_);
      bootstrap.mutable_static_resources()->mutable_listeners()->Clear();
      auto* lds_config_source = bootstrap.mutable_dynamic_resources()->mutable_lds_config();
      lds_config_source->set_resource_api_version(envoy::config::core::v3::ApiVersion::V3);
      auto* lds_api_config_source = lds_config_source->mutable_api_config_source();
      lds_api_config_source->set_api_type(envoy::config::core::v3::ApiConfigSource::GRPC);
      lds_api_config_source->set_transport_api_version(envoy::config::core::v3::V3);
      envoy::config::core::v3::GrpcService* grpc_service =
          lds_api_config_source->add_grpc_services();
      setGrpcService(*grpc_service, "lds_cluster", getLdsFakeUpstream().localAddress());
    });
  }

  void waitForLdsAck() {
    envoy::service::discovery::v3::DiscoveryRequest sotw_request;
    EXPECT_TRUE(lds_stream_->waitForGrpcMessage(*dispatcher_, sotw_request));
    EXPECT_EQ(sotw_request.version_info(), "");
    EXPECT_TRUE(lds_stream_->waitForGrpcMessage(*dispatcher_, sotw_request));
    EXPECT_EQ(sotw_request.version_info(), "initial");

    EXPECT_EQ((*test_server_.get()).server().initManager().state(),
              Init::Manager::State::Initialized);
  }

  void initialize() override {
    use_lds_ = false; // required for grpc lds
    setUpstreamProtocol(Http::CodecType::HTTP2);
    config_helper_.addConfigModifier([](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
      // Add the static cluster to serve LDS.
      auto* lds_cluster = bootstrap.mutable_static_resources()->add_clusters();
      lds_cluster->MergeFrom(bootstrap.static_resources().clusters()[0]);
      lds_cluster->set_name("lds_cluster");
      ConfigHelper::setHttp2(*lds_cluster);

      // Add the static cluster to serve RDS.
      auto* rds_cluster = bootstrap.mutable_static_resources()->add_clusters();
      rds_cluster->MergeFrom(bootstrap.static_resources().clusters()[0]);
      rds_cluster->set_name("rds_cluster");
      ConfigHelper::setHttp2(*rds_cluster);
    });

    TestEnvironment::writeStringToFileForTest("token_secret.yaml", R"EOF(
resources:
  - "@type": "type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.Secret"
    name: token
    generic_secret:
      secret:
        inline_string: "token_secret")EOF",
                                              false);

    TestEnvironment::writeStringToFileForTest("token_secret_1.yaml", R"EOF(
resources:
  - "@type": "type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.Secret"
    name: token
    generic_secret:
      secret:
        inline_string: "token_secret_1")EOF",
                                              false);

    TestEnvironment::writeStringToFileForTest("hmac_secret.yaml", R"EOF(
resources:
  - "@type": "type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.Secret"
    name: hmac
    generic_secret:
      secret:
        inline_string: "hmac_secret")EOF",
                                              false);

    TestEnvironment::writeStringToFileForTest("hmac_secret_1.yaml", R"EOF(
resources:
  - "@type": "type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.Secret"
    name: hmac
    generic_secret:
      secret:
        inline_string: "hmac_secret_1")EOF",
                                              false);

    setOauthConfig();

    // Add the OAuth cluster.
    config_helper_.addConfigModifier([&](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
      *bootstrap.mutable_static_resources()->add_clusters() =
          config_helper_.buildStaticCluster("oauth", 0, "127.0.0.1");
    });

    setUpstreamCount(2);
    setUpGrpcLds();
    HttpIntegrationTest::initialize();
    waitForLdsAck();
    registerTestServerPorts({"http"});
  }

  void createUpstreams() override {
    HttpIntegrationTest::createUpstreams();
    // Create the LDS upstream (fake_upstreams_[1]).
    addFakeUpstream(Http::CodecType::HTTP2);
    // Create the RDS upstream (fake_upstreams_[2]).
    addFakeUpstream(Http::CodecType::HTTP2);
  }

  virtual void setOauthConfig() {
    // This config is same as when the 'auth_type: "URL_ENCODED_BODY"' is set as it's the default
    // value
    config_helper_.prependFilter(TestEnvironment::substitute(R"EOF(
name: oauth
typed_config:
  "@type": type.googleapis.com/envoy.extensions.filters.http.oauth2.v3.OAuth2
  config:
    token_endpoint:
      cluster: oauth
      uri: oauth.com/token
      timeout: 3s
    authorization_endpoint: https://oauth.com/oauth/authorize/
    redirect_uri: "%REQ(x-forwarded-proto)%://%REQ(:authority)%/callback"
    redirect_path_matcher:
      path:
        exact: /callback
    signout_path:
      path:
        exact: /signout
    credentials:
      client_id: foo
      token_secret:
        name: token
        sds_config:
          path_config_source:
            path: "{{ test_tmpdir }}/token_secret.yaml"
          resource_api_version: V3
      hmac_secret:
        name: hmac
        sds_config:
          path_config_source:
            path: "{{ test_tmpdir }}/hmac_secret.yaml"
          resource_api_version: V3
    auth_scopes:
    - user
    - openid
    - email
    resources:
    - oauth2-resource
    - http://example.com
    - https://example.com
)EOF"));
  }

  bool validateHmac(const Http::ResponseHeaderMap& headers, absl::string_view host,
                    absl::string_view hmac_secret) {
    std::string expires =
        Http::Utility::parseSetCookieValue(headers, default_cookie_names_.oauth_expires_);
    std::string token =
        Http::Utility::parseSetCookieValue(headers, default_cookie_names_.bearer_token_);
    std::string hmac =
        Http::Utility::parseSetCookieValue(headers, default_cookie_names_.oauth_hmac_);

    Http::TestRequestHeaderMapImpl validate_headers{{":authority", std::string(host)}};

    validate_headers.addReferenceKey(Http::Headers::get().Cookie,
                                     absl::StrCat(default_cookie_names_.oauth_hmac_, "=", hmac));
    validate_headers.addReferenceKey(
        Http::Headers::get().Cookie,
        absl::StrCat(default_cookie_names_.oauth_expires_, "=", expires));
    validate_headers.addReferenceKey(Http::Headers::get().Cookie,
                                     absl::StrCat(default_cookie_names_.bearer_token_, "=", token));

    OAuth2CookieValidator validator{api_->timeSource(), default_cookie_names_};
    validator.setParams(validate_headers, std::string(hmac_secret));
    return validator.isValid();
  }

  virtual void checkClientSecretInRequest(absl::string_view token_secret) {
    std::string request_body = upstream_request_->body().toString();
    const auto query_parameters = Http::Utility::parseFromBody(request_body);
    auto it = query_parameters.find("client_secret");

    ASSERT_TRUE(it != query_parameters.end());
    EXPECT_EQ(it->second, token_secret);
  }

  void doAuthenticationFlow(absl::string_view token_secret, absl::string_view hmac_secret) {
    codec_client_ = makeHttpConnection(lookupPort("http"));

    Http::TestRequestHeaderMapImpl headers{
        {":method", "GET"},
        {":path", "/callback?code=foo&state=http%3A%2F%2Ftraffic.example.com%2Fnot%2F_oauth"},
        {":scheme", "http"},
        {"x-forwarded-proto", "http"},
        {":authority", "authority"},
        {"authority", "Bearer token"}};

    auto encoder_decoder = codec_client_->startRequest(headers);
    request_encoder_ = &encoder_decoder.first;
    auto response = std::move(encoder_decoder.second);

    waitForNextUpstreamRequest(std::vector<uint64_t>({0, 1, 2, 3}));

    ASSERT_TRUE(upstream_request_->waitForHeadersComplete());

    checkClientSecretInRequest(token_secret);

    upstream_request_->encodeHeaders(
        Http::TestRequestHeaderMapImpl{{":status", "200"}, {"content-type", "application/json"}},
        false);

    envoy::extensions::http_filters::oauth2::OAuthResponse oauth_response;
    oauth_response.mutable_access_token()->set_value("bar");
    oauth_response.mutable_expires_in()->set_value(DateUtil::nowToSeconds(api_->timeSource()) + 10);

    Buffer::OwnedImpl buffer(MessageUtil::getJsonStringFromMessageOrDie(oauth_response));
    upstream_request_->encodeData(buffer, true);

    // We should get an immediate redirect back.
    response->waitForHeaders();

    EXPECT_TRUE(
        validateHmac(response->headers(), headers.Host()->value().getStringView(), hmac_secret));

    EXPECT_EQ("302", response->headers().getStatusValue());

    RELEASE_ASSERT(response->waitForEndStream(), "unexpected timeout");
    codec_client_->close();
  }

  const CookieNames default_cookie_names_{"BearerToken", "OauthHMAC", "OauthExpires"};
  envoy::config::listener::v3::Listener listener_config_;
  std::string listener_name_{"http"};
  FakeHttpConnectionPtr lds_connection_;
  FakeStreamPtr lds_stream_{};
};

INSTANTIATE_TEST_SUITE_P(IpVersionsAndGrpcTypes, OauthIntegrationTest,
                         GRPC_CLIENT_INTEGRATION_PARAMS);

// Regular request gets redirected to the login page.
TEST_P(OauthIntegrationTest, UnauthenticatedFlow) {
  on_server_init_function_ = [&]() {
    createLdsStream();
    sendLdsResponse({MessageUtil::getYamlStringFromMessage(listener_config_)}, "initial");
  };
  initialize();

  codec_client_ = makeHttpConnection(lookupPort("http"));
  Http::TestRequestHeaderMapImpl headers{{":method", "GET"},
                                         {":path", "/lua/per/route/default"},
                                         {":scheme", "http"},
                                         {":authority", "authority"}};
  auto encoder_decoder = codec_client_->startRequest(headers);

  request_encoder_ = &encoder_decoder.first;
  auto response = std::move(encoder_decoder.second);

  // We should get an immediate redirect back.
  response->waitForHeaders();
  EXPECT_EQ("302", response->headers().getStatusValue());
}

TEST_P(OauthIntegrationTest, AuthenticationFlow) {
  on_server_init_function_ = [&]() {
    createLdsStream();
    sendLdsResponse({MessageUtil::getYamlStringFromMessage(listener_config_)}, "initial");
  };

  initialize();

  // 1. Do one authentication flow.
  doAuthenticationFlow("token_secret", "hmac_secret");

  // 2. Reload secrets.
  EXPECT_EQ(test_server_->counter("sds.token.update_success")->value(), 1);
  EXPECT_EQ(test_server_->counter("sds.hmac.update_success")->value(), 1);
  TestEnvironment::renameFile(TestEnvironment::temporaryPath("token_secret_1.yaml"),
                              TestEnvironment::temporaryPath("token_secret.yaml"));
  test_server_->waitForCounterEq("sds.token.update_success", 2, std::chrono::milliseconds(5000));
  TestEnvironment::renameFile(TestEnvironment::temporaryPath("hmac_secret_1.yaml"),
                              TestEnvironment::temporaryPath("hmac_secret.yaml"));
  test_server_->waitForCounterEq("sds.hmac.update_success", 2, std::chrono::milliseconds(5000));
  // 3. Do another one authentication flow.
  doAuthenticationFlow("token_secret_1", "hmac_secret_1");
}

// Regression test(issue #22678) where (incorrectly)using server's init manager(initialized state)
// to add init target by the secret manager led to the assertion failure.
TEST_P(OauthIntegrationTest, LoadListenerAfterServerIsInitialized) {
  on_server_init_function_ = [&]() {
    createLdsStream();
    envoy::config::listener::v3::Listener listener =
        TestUtility::parseYaml<envoy::config::listener::v3::Listener>(R"EOF(
      name: fake_listener
      address:
        socket_address:
          address: 127.0.0.1
          port_value: 0
      filter_chains:
        - filters:
          - name: envoy.filters.network.http_connection_manager
            typed_config:
              "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
              codec_type: HTTP2
              stat_prefix: config_test
              route_config:
                name: route_config_0
                virtual_hosts:
                  - name: integration
                    domains:
                      - "*"
                    routes:
                      - match:
                          prefix: /
                        route:
                          cluster: cluster_0
              http_filters:
                - name: envoy.filters.http.router
        )EOF");

    // dummy listener is being sent so that lds api gets marked as ready, which would
    // led to server's init manager reach initialized state
    sendLdsResponse({listener}, "initial");
  };

  initialize();

  // add listener with oauth2 filter and sds configs
  sendLdsResponse({MessageUtil::getYamlStringFromMessage(listener_config_)}, "delayed");
  test_server_->waitForCounterGe("listener_manager.lds.update_success", 2);
  test_server_->waitForGaugeEq("listener_manager.total_listeners_warming", 0);

  doAuthenticationFlow("token_secret", "hmac_secret");
}
class OauthIntegrationTestWithBasicAuth : public OauthIntegrationTest {
  void setOauthConfig() override {
    config_helper_.prependFilter(TestEnvironment::substitute(R"EOF(
name: oauth
typed_config:
  "@type": type.googleapis.com/envoy.extensions.filters.http.oauth2.v3.OAuth2
  config:
    token_endpoint:
      cluster: oauth
      uri: oauth.com/token
      timeout: 3s
    authorization_endpoint: https://oauth.com/oauth/authorize/
    redirect_uri: "%REQ(x-forwarded-proto)%://%REQ(:authority)%/callback"
    redirect_path_matcher:
      path:
        exact: /callback
    signout_path:
      path:
        exact: /signout
    credentials:
      client_id: foo
      token_secret:
        name: token
        sds_config:
          path_config_source:
            path: "{{ test_tmpdir }}/token_secret.yaml"
          resource_api_version: V3
      hmac_secret:
        name: hmac
        sds_config:
          path_config_source:
            path: "{{ test_tmpdir }}/hmac_secret.yaml"
          resource_api_version: V3
    auth_scopes:
    - user
    - openid
    - email
    resources:
    - oauth2-resource
    - http://example.com
    - https://example.com
    auth_type: "BASIC_AUTH"
)EOF"));
  }

  void checkClientSecretInRequest(absl::string_view token_secret) override {
    EXPECT_FALSE(
        upstream_request_->headers().get(Http::CustomHeaders::get().Authorization).empty());
    const std::string basic_auth_token = absl::StrCat("foo:", token_secret);
    const std::string encoded_token =
        Base64::encode(basic_auth_token.data(), basic_auth_token.size());
    const auto token_secret_expected = absl::StrCat("Basic ", encoded_token);
    EXPECT_EQ(token_secret_expected, upstream_request_->headers()
                                         .get(Http::CustomHeaders::get().Authorization)[0]
                                         ->value()
                                         .getStringView());
  }
};

INSTANTIATE_TEST_SUITE_P(IpVersionsAndGrpcTypes, OauthIntegrationTestWithBasicAuth,
                         GRPC_CLIENT_INTEGRATION_PARAMS);

// Do OAuth flow with Basic auth header in access token request.
TEST_P(OauthIntegrationTestWithBasicAuth, AuthenticationFlow) {
  on_server_init_function_ = [&]() {
    createLdsStream();
    sendLdsResponse({MessageUtil::getYamlStringFromMessage(listener_config_)}, "initial");
  };
  initialize();
  doAuthenticationFlow("token_secret", "hmac_secret");
}

// Verify the behavior when the callback param is missing the state query param.
TEST_P(OauthIntegrationTest, MissingStateParam) {
  on_server_init_function_ = [&]() {
    createLdsStream();
    sendLdsResponse({MessageUtil::getYamlStringFromMessage(listener_config_)}, "initial");
  };
  initialize();

  codec_client_ = makeHttpConnection(lookupPort("http"));
  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"}, {":path", "/callback"}, {":scheme", "http"}, {":authority", "authority"}};
  auto encoder_decoder = codec_client_->startRequest(headers);

  request_encoder_ = &encoder_decoder.first;
  auto response = std::move(encoder_decoder.second);

  // We should get an 401 back since the request looked like a redirect request but did not
  // contain the state param.
  response->waitForHeaders();
  EXPECT_EQ("401", response->headers().getStatusValue());
}

} // namespace
} // namespace Oauth2
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
