#!/bin/bash
# Copyright (C) 2020 Red Hat, Inc. All rights reserved.
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

set -x
set -e
set -u
set -o pipefail

# These can be overridden from the calling environment to
# explicitly force a particular org, repo and/or branch name.
ENVOY_OPENSSL_ORG="${ENVOY_OPENSSL_ORG:-envoyproxy}"
ENVOY_OPENSSL_REPO="${ENVOY_OPENSSL_REPO:-envoy-openssl}"
ENVOY_OPENSSL_BRANCH="${ENVOY_OPENSSL_BRANCH:-}"


function cleanup() {
  rm -rf "${WORKDIR}"
}

function init() {
  WORKDIR="$(mktemp -d)"
  trap cleanup EXIT
}

function get_envoy_sha() {
  local maistra_proxy_branch

  if [[ -z "${ENVOY_OPENSSL_BRANCH}" ]]; then
    maistra_proxy_branch="$(git symbolic-ref --quiet --short HEAD)"

    # Prior to the maistra-2.5 branch, we used to assume the same branch name existed in the
    # maistra/envoy repository. Starting from maistra-2.5 we use the upstream envoyproxy/envoy-openssl
    # repository, which doesn't contain matching branch names. Therefore, we have to explicitly map
    # from proxy's maistra-2.x branch name to upstream's corresponding release/v1.xx branch name.

    case ${maistra_proxy_branch} in
      maistra-2.5)
        ENVOY_OPENSSL_BRANCH=release/v1.26
      ;;
      *)
        echo "Unknown proxy branch ${maistra_proxy_branch}" 1>&2
        exit 1
      ;;
    esac
  fi

  pushd "${WORKDIR}" >/dev/null
  git clone --depth=1 -b "${ENVOY_OPENSSL_BRANCH}" "https://github.com/${ENVOY_OPENSSL_ORG}/${ENVOY_OPENSSL_REPO}.git"

  pushd "${ENVOY_OPENSSL_REPO}" >/dev/null
  SHA=$(git rev-parse HEAD)
  popd >/dev/null

  popd >/dev/null
}

function get_envoy_sha_256() {
  pushd "${WORKDIR}" >/dev/null
  curl -sfLO "https://github.com/${ENVOY_OPENSSL_ORG}/${ENVOY_OPENSSL_REPO}/archive/${SHA}.tar.gz"
  SHA256=$(sha256sum "${SHA}.tar.gz" | awk '{print $1}')
  popd >/dev/null
}

function main() {
  local today

  init

  today=$(date +%D)
  get_envoy_sha
  get_envoy_sha_256

  sed -i "s|^# Commit time: .*|# Commit time: ${today}|" WORKSPACE
  sed -i "s|^ENVOY_OPENSSL_ORG = .*|ENVOY_OPENSSL_ORG = \"${ENVOY_OPENSSL_ORG}\"|" WORKSPACE
  sed -i "s|^ENVOY_OPENSSL_REPO = .*|ENVOY_OPENSSL_REPO = \"${ENVOY_OPENSSL_REPO}\"|" WORKSPACE
  sed -i "s|^ENVOY_OPENSSL_BRANCH = .*|ENVOY_OPENSSL_BRANCH = \"${ENVOY_OPENSSL_BRANCH}\"|" WORKSPACE
  sed -i "s|^ENVOY_OPENSSL_COMMIT = .*|ENVOY_OPENSSL_COMMIT = \"${SHA}\"|" WORKSPACE
  sed -i "s|^ENVOY_OPENSSL_SHA256 = .*|ENVOY_OPENSSL_SHA256 = \"${SHA256}\"|" WORKSPACE
}

main
