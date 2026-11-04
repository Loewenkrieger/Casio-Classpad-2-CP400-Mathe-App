#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
image=ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a
docker run --rm --network none --cap-drop ALL --security-opt no-new-privileges \
  --mount "type=bind,source=$(pwd),target=/work" --workdir /work "$image" make -j2 hh3
python3 tools/check_binary.py
