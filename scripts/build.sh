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
$ENGINE build -t "$IMAGE" docker/

echo "== [2/4] fetching deps =="
"$ROOT/scripts/fetch-deps.sh"

echo "== [3/4] configuring + building libtorrent (PS4 target) =="
$ENGINE run --rm -v "$ROOT":/src -w /src "$IMAGE" bash -c '
  set -e
  source /opt/cross-file-ps4.env
  cd /src/deps/libtorrent
  # apply port patches
  for p in /src/patches/libtorrent/*.patch; do
    echo "applying $p"; patch -p1 --forward < "$p" || true
  done
  ./autogen.sh
  # rakshasa needs an explicit host triple for cross builds
  ./configure --host=x86_64-scei-ps4 --prefix=/src/build/oo \
      --without-openssl --disable-shared --enable-static \
      --with-scenetpoll \
      --disable-debug \
      CC="$CC" CXX="$CXX" AR="$AR" RANLIB="$RANLIB" \
      CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS"
  make -j"$(nproc)" || make -j2
  make install
  echo "LIBTORRENT BUILD OK"
'

echo "== [4/4] building test elf =="
$ENGINE run --rm -v "$ROOT":/src -w /src "$IMAGE" bash -c '
  set -e
  source /opt/cross-file-ps4.env
  cd /src/app
  make -f Makefile.oo
  ls -la torrent_test.elf && file torrent_test.elf || true
'
echo "BUILD COMPLETE"
