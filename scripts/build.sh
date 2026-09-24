#!/usr/bin/env bash
# Build ps4-libtorrent-port inside the OpenOrbis docker image.
# Nothing is installed on the host (immutable-distro friendly).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

ENGINE="${ENGINE:-podman}"
command -v "$ENGINE" >/dev/null || ENGINE=docker

IMAGE=openorbis-libtorrent:latest

echo "== [1/4] building container image =="
$ENGINE build -f docker/Dockerfile -t "$IMAGE" .

echo "== [2/4] fetching deps =="
"$ROOT/scripts/fetch-deps.sh"

echo "== [3/4] configuring + building libtorrent (PS4 target) =="
$ENGINE run --rm -v "$ROOT":/src -w /src "$IMAGE" bash /src/scripts/build-libtorrent-container.sh

echo "== [4/4] building test elf =="
$ENGINE run --rm -v "$ROOT":/src -w /src "$IMAGE" bash /src/scripts/build-app-container.sh
echo "BUILD COMPLETE"
