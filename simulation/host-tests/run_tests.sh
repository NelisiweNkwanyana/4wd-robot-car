#!/usr/bin/env bash
# Build the real firmware against a mock Arduino core and run the logic tests.
# Needs only a C++ compiler (macOS: `xcode-select --install` provides clang++).
set -euo pipefail
cd "$(dirname "$0")"
FW=../../firmware/RobotCar
CXX=${CXX:-g++}
SRCS=$(ls $FW/*.cpp)
$CXX -std=gnu++11 -Wall -Wextra -Wno-unused-parameter -I mock -I $FW \
  mock/mock.cpp $SRCS tests.cpp -o robotcar_tests
./robotcar_tests
