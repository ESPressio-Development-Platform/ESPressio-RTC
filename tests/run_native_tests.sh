#!/usr/bin/env bash
set -euo pipefail
CXX="${CXX:-g++}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PLATFORM_SRC="${PLATFORM_SRC:-${ROOT}/../ESPressio-Platform/src}"
BUILD="${ROOT}/.native-test-build"
rm -rf "${BUILD}" && mkdir -p "${BUILD}"
"${CXX}" -std=gnu++17 -Wall -Wextra -Werror -pedantic \
  -I"${ROOT}/src" -I"${PLATFORM_SRC}" \
  "${ROOT}/tests/native/rtc_test.cpp" -o "${BUILD}/rtc_test"
"${BUILD}/rtc_test"
echo "ESPressio-RTC native tests passed"
