#!/usr/bin/env bash
# Fetch upstream deps: rakshasa libtorrent source
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEPS="$ROOT/deps"
mkdir -p "$DEPS"

if [ ! -d "$DEPS/libtorrent/.git" ]; then
  git clone --depth 1 https://github.com/rakshasa/libtorrent "$DEPS/libtorrent"
else
  git -C "$DEPS/libtorrent" fetch --depth 1 origin || true
fi
echo "deps ready: $DEPS/libtorrent @ $(git -C "$DEPS/libtorrent" rev-parse --short HEAD)"
