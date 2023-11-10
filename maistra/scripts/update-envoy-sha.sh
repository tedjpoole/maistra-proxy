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

ENVOY_ORG=envoyproxy
ENVOY_REPO=envoy


function cleanup() {
  rm -rf "${WORKDIR}"
}

function init() {
  WORKDIR="$(mktemp -d)"
  trap cleanup EXIT
}

function get_envoy_sha() {
  local proxy_branch
  local envoy_branch

  proxy_branch="${BRANCH:-$(git symbolic-ref --quiet --short HEAD)}"

  # Prior to the maistra-2.5 branch, we used to assume the same branch name existed in the
  # maistra/envoy repository. Starting from maistra-2.5 we use the upstream envoyproxy/envoy
  # repository, which doesn't contain matching branch names. Therefore, we have to explicitly map
  # from proxy's maistra-2.x branch name to upstream's corresponding release/v1.xx branch name.

  case ${proxy_branch} in
    maistra-2.5)
      envoy_branch=release/v1.26
    ;;
    *)
      echo "Unknown proxy branch ${proxy_branch}" 1>&2
      exit 1
    ;;
  esac

  pushd "${WORKDIR}" >/dev/null
  git clone --depth=1 -b "${envoy_branch}" "https://github.com/${ENVOY_ORG}/${ENVOY_REPO}.git"

  pushd "${ENVOY_REPO}" >/dev/null
  SHA=$(git rev-parse HEAD)
  popd >/dev/null

  popd >/dev/null
}

function get_envoy_sha_256() {
  pushd "${WORKDIR}" >/dev/null
  curl -sfLO "https://github.com/${ENVOY_ORG}/${ENVOY_REPO}/archive/${SHA}.tar.gz"
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
  sed -i "s|^ENVOY_SHA = .*|ENVOY_SHA = \"${SHA}\"|" WORKSPACE
  sed -i "s|^ENVOY_SHA256 = .*|ENVOY_SHA256 = \"${SHA256}\"|" WORKSPACE
  sed -i "s|^ENVOY_ORG = .*|ENVOY_ORG = \"${ENVOY_ORG}\"|" WORKSPACE
  sed -i "s|^ENVOY_REPO = .*|ENVOY_REPO = \"${ENVOY_REPO}\"|" WORKSPACE
}

main
