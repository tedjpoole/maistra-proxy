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
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load(
    "//bazel:repositories.bzl",
    "docker_dependencies",
    "istioapi_dependencies",
)

istioapi_dependencies()


ENVOY_OPENSSL_ORG = "tedjpoole"
ENVOY_OPENSSL_REPO = "envoy-openssl"
ENVOY_OPENSSL_BRANCH = "OSSM-5001-integrate-into-proxy"
ENVOY_OPENSSL_COMMIT = "de50633d500c960944515f3b75a7ef96278dcab2"
ENVOY_OPENSSL_SHA256 = "d92c1a34a81e25f58407841511c11e558c6ef08f582393f330def86d9cf00c80"

git_repository(
    name = "envoy_openssl",
    remote = "https://github.com/" + ENVOY_OPENSSL_ORG + "/" + ENVOY_OPENSSL_REPO + ".git",
    commit = ENVOY_OPENSSL_COMMIT,
    init_submodules = True,
)

load("@envoy_openssl//:bazel/envoy_openssl_repositories.bzl", "envoy_openssl_repositories")
envoy_openssl_repositories()

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
