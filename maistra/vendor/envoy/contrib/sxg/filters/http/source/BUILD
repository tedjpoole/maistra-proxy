load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_contrib_extension",
    "envoy_cc_library",
    "envoy_contrib_package",
)

licenses(["notice"])  # Apache 2

envoy_contrib_package()

envoy_cc_library(
    name = "sxg_lib",
    srcs = [
        "encoder.cc",
        "filter.cc",
        "filter_config.cc",
    ],
    hdrs = [
        "encoder.h",
        "filter.h",
        "filter_config.h",
    ],
    external_deps = ["libsxg"],
    deps = [
        "//envoy/server:filter_config_interface",
        "//source/common/config:datasource_lib",
        "//source/common/http:codes_lib",
        "//source/common/stats:symbol_table_lib",
        "//source/common/stats:utility_lib",
        "//source/extensions/filters/http/common:pass_through_filter_lib",
        "@openssl//:openssl-lib",
        "@envoy_api//contrib/envoy/extensions/filters/http/sxg/v3alpha:pkg_cc_proto",
    ],
)

envoy_cc_contrib_extension(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = [
        ":sxg_lib",
        "//envoy/registry",
        "//source/extensions/filters/http/common:factory_base_lib",
        "@envoy_api//contrib/envoy/extensions/filters/http/sxg/v3alpha:pkg_cc_proto",
    ],
)
