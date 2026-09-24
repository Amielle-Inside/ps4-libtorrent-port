#!/usr/bin/env bash
# Runs INSIDE the container (see build.sh). Cross-builds rakshasa/libtorrent.
set -e
source /opt/cross-file-ps4.env
cd /src/deps/libtorrent

# apply port patches
find /src/patches/libtorrent -name "*.patch" | sort | while read -r p; do
  echo "applying $p"
  patch -p1 --forward < "$p" || true
done

autoreconf -ivf
# PS4 triple isn't in config.sub; use a recognized x86_64-unknown-freebsd
# triple — the toolchain clang targets x86_64 PS4 (FreeBSD-based) anyway.
# PS4 libc provides real epoll — rakshasa's stock epoll backend works as-is.
./configure --host=x86_64-unknown-freebsd12.0 --prefix=/src/build/oo \
    --disable-shared --enable-static \
    --disable-debug \
    CC="$CC" CXX="$CXX" AR="$AR" RANLIB="$RANLIB" \
    CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS"
make -j"$(nproc)" || make -j2
make install
# compile PS4 shim (real SHA-1/RC4/base64) into the INSTALLED static lib
ps4-clang -O2 -D_GNU_SOURCE -include stdint.h -I/src/shim -c /src/shim/ps4_crypto.cpp -o shim_ps4_crypto.o
llvm-ar r /src/build/oo/lib/libtorrent.a shim_ps4_crypto.o
llvm-ranlib /src/build/oo/lib/libtorrent.a
echo "LIBTORRENT BUILD OK"
