# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################
#
workspace(name = "io_istio_proxy")

# http_archive is not a native function since bazel 0.19
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load(
    "//bazel:repositories.bzl",
    "docker_dependencies",
    "googletest_repositories",
    "istioapi_dependencies",
)

googletest_repositories()

istioapi_dependencies()

new_local_repository(
    name = "openssl",
    path = "/usr/lib64/",
    build_file = "openssl.BUILD"
)

# 1. Determine SHA256 `wget https://github.com/envoyproxy/envoy/archive/$COMMIT.tar.gz && sha256sum $COMMIT.tar.gz`
# 2. Update .bazelversion, envoy.bazelrc and .bazelrc if needed.
#
# Note: this is needed by release builder to resolve envoy dep sha to tag.
# Commit date: 2023-07-05
ENVOY_SHA = "25ee871f0c935a5cc39b7ba1d75bb81ed10942e9"

ENVOY_SHA256 = "92540f679d570f69e20d4f43255fce354e375396b05b7d2d7e9c6edb2fccdd86"

ENVOY_ORG = "maistra"

ENVOY_REPO = "envoy"

# To override with local envoy, just pass `--override_repository=envoy=/PATH/TO/ENVOY` to Bazel or
# persist the option in `user.bazelrc`.
http_archive(
    name = "envoy",
    sha256 = ENVOY_SHA256,
    strip_prefix = ENVOY_REPO + "-" + ENVOY_SHA,
    url = "https://github.com/" + ENVOY_ORG + "/" + ENVOY_REPO + "/archive/" + ENVOY_SHA + ".tar.gz",
)

load("@envoy//bazel:api_binding.bzl", "envoy_api_binding")
envoy_api_binding()

local_repository(
    name = "envoy_build_config",
    # Relative paths are also supported.
    path = "bazel/extension_config",
)

load("@envoy//bazel:api_repositories.bzl", "envoy_api_dependencies")
envoy_api_dependencies()

load("@envoy//bazel:repositories.bzl", "envoy_dependencies", "BUILD_ALL_CONTENT")
envoy_dependencies()

# Added for OSSM-1931: emscripten is in /opt/emsdk
new_local_repository(
    name = "emscripten_bin_linux",
    path = "/opt/emsdk/",
    build_file_content = BUILD_ALL_CONTENT,
)

# Added for OSSM-1931: find npm in /lib
new_local_repository(
    name = "emscripten_npm_linux",
    path = "/lib/node_modules/npm",
    build_file_content = BUILD_ALL_CONTENT,
)

local_repository(
    name = "local-toolchain",
    path = "maistra/local",
)

load("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra")
envoy_dependencies_extra()

load("@envoy//bazel:python_dependencies.bzl", "envoy_python_dependencies")

envoy_python_dependencies()

load("@base_pip3//:requirements.bzl", "install_deps")
install_deps()

load("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports")
envoy_dependency_imports()

# Bazel @rules_pkg

http_archive(
    name = "rules_pkg",
    sha256 = "aeca78988341a2ee1ba097641056d168320ecc51372ef7ff8e64b139516a4937",
    urls = [
        "https://github.com/bazelbuild/rules_pkg/releases/download/0.2.6-1/rules_pkg-0.2.6.tar.gz",
        "https://mirror.bazel.build/github.com/bazelbuild/rules_pkg/releases/download/0.2.6/rules_pkg-0.2.6.tar.gz",
    ],
)

load("@rules_pkg//:deps.bzl", "rules_pkg_dependencies")
rules_pkg_dependencies()

# Docker dependencies

docker_dependencies()
load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)
container_repositories()
load("@io_bazel_rules_docker//repositories:deps.bzl", container_deps = "deps")
container_deps()

# End of docker dependencies

load("//bazel:wasm.bzl", "wasm_dependencies")
wasm_dependencies()
