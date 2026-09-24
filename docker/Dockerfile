# OpenOrbis PS4 build image — based on the official toolchain image
FROM openorbisofficial/toolchain:latest

USER root

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf automake libtool autoconf-archive pkg-config \
    ca-certificates curl wget xz-utils m4 llvm \
    && rm -rf /var/lib/apt/lists/*

ENV OO_PS4_TOOLCHAIN=/lib/OpenOrbisSDK
ENV PATH=$OO_PS4_TOOLCHAIN/bin/linux:$PATH

# Cross-compiler wrappers for autotools (CC/CXX must be a single command).
# -nostdlib: we always provide crt1.o/link.x/libs explicitly via LDFLAGS
# (recipe from toolchain samples/net_http/Makefile).
RUN printf '#!/bin/sh\nexec clang --target=x86_64-pc-freebsd12-elf -nostdlib -fPIC -funwind-tables -isysroot %s -isystem %s/include "$@"\n' \
      "$OO_PS4_TOOLCHAIN" "$OO_PS4_TOOLCHAIN" > /usr/local/bin/ps4-clang && \
    printf '#!/bin/sh\nexec clang++ --target=x86_64-pc-freebsd12-elf -nostdlib -fPIC -funwind-tables -isysroot %s -isystem %s/include -isystem %s/include/c++/v1 "$@"\n' \
      "$OO_PS4_TOOLCHAIN" "$OO_PS4_TOOLCHAIN" "$OO_PS4_TOOLCHAIN" > /usr/local/bin/ps4-clang++ && \
    chmod +x /usr/local/bin/ps4-clang /usr/local/bin/ps4-clang++

# Cross environment used by autotools builds (libtorrent)
COPY docker/cross-file-ps4.env /opt/cross-file-ps4.env

# Cross-build zlib (needed by libtorrent's configure via pkg-config)
RUN cd /tmp && curl -sL -o zlib.tar.gz https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz \
    && tar xf zlib.tar.gz && cd zlib-1.3.1 \
    && . /opt/cross-file-ps4.env \
    && CHOST=x86_64-pc-freebsd12-elf ./configure --static --prefix=/opt/ps4deps \
    && make -j"$(nproc)" libz.a \
    && mkdir -p /opt/ps4deps/lib/pkgconfig /opt/ps4deps/include \
    && cp libz.a /opt/ps4deps/lib/ && cp zconf.h zlib.h /opt/ps4deps/include/ \
    && printf 'prefix=/opt/ps4deps\nexec_prefix=${prefix}\nlibdir=${exec_prefix}/lib\nincludedir=${prefix}/include\n\nName: zlib\nDescription: zlib compression library (PS4 cross)\nVersion: 1.3.1\n\nRequires:\nLibs: -L${libdir} -lz\nCflags: -I${includedir}\n' > /opt/ps4deps/lib/pkgconfig/zlib.pc \
    && rm -rf /tmp/zlib*

ENV PKG_CONFIG_PATH=/opt/ps4deps/lib/pkgconfig

# Cross-build libcurl 7.81 (HTTP only, no TLS — MVP trackers via HTTP; TLS in
# phase 2 via ps4-openssl). 7.x is what the PS4 scene historically cross-built;
# 8.x cf-socket.c uses Linux-only SOL_IP.
# Stub osreldate.h: our --host triple is FreeBSD-flavored but the SDK has no
# such header; curl only wants __FreeBSD_version.
RUN printf '#ifndef _OSRELDATE_H_PS4STUB\n#define _OSRELDATE_H_PS4STUB\n#define __FreeBSD_version 1200000\n#endif\n' \
      > "$OO_PS4_TOOLCHAIN/include/osreldate.h"
# curl's connect.c uses Linux-only SOL_IP; FreeBSD equivalent is IPPROTO_IP
RUN printf '#ifndef _NETINET_IP_PS4STUB_SOLIP\n#define _NETINET_IP_PS4STUB_SOLIP\n#define SOL_IP IPPROTO_IP\n#endif\n' \
      > "$OO_PS4_TOOLCHAIN/include/ps4_compat.h"
# Minimal OpenSSL headers: rakshasa's utils/sha1.h + rc4.h include OpenSSL
# headers unconditionally. The API is backed by the REAL SHA-1/RC4 in
# /src/shim/ps4_crypto.c (piece hashing requires working SHA-1).
# Phase 2 replaces this with real ps4-openssl.
COPY shim/ps4_crypto.h /src-shim/ps4_crypto.h
RUN mkdir -p "$OO_PS4_TOOLCHAIN/include/openssl" \
    && cp /src-shim/ps4_crypto.h "$OO_PS4_TOOLCHAIN/include/ps4_crypto.h" \
    && printf '#ifndef PS4_STUB_OPENSSL_EVPH\n#define PS4_STUB_OPENSSL_EVPH\n#include "ps4_crypto.h"\n#endif\n' \
      > "$OO_PS4_TOOLCHAIN/include/openssl/evp.h" \
    && printf '#ifndef PS4_STUB_OPENSSL_RC4H\n#define PS4_STUB_OPENSSL_RC4H\n#include "ps4_crypto.h"\n#endif\n' \
      > "$OO_PS4_TOOLCHAIN/include/openssl/rc4.h"
RUN cd /tmp && curl -sL -o curl.tar.bz2 https://curl.se/download/curl-7.81.0.tar.bz2 \
    && tar xf curl.tar.bz2 && cd curl-7.81.0 \
    && . /opt/cross-file-ps4.env \
    && ./configure --build=x86_64-pc-linux-gnu --host=x86_64-unknown-freebsd12.0 \
        curl_cv_sizeof_curl_off_t=8 curl_cv_sizeof_long=8 \
        ac_cv_func_gethostbyname=yes ac_cv_func_getaddrinfo=yes \
        --prefix=/opt/ps4deps --disable-shared --enable-static \
        --without-ssl --without-libpsl --without-libidn2 --without-brotli --without-zstd \
        --disable-ftp --disable-ftps --disable-gopher --disable-imap --disable-ldap --disable-ldaps \
        --disable-pop3 --disable-rtsp --disable-smb --disable-smtp --disable-telnet --disable-tftp \
        --disable-mqtt --without-nghttp2 --without-librtmp --without-libssh2 --disable-docs --disable-manual \
        CC="$CC" CXX="$CXX" AR="$AR" RANLIB="$RANLIB" \
        CFLAGS="$CFLAGS -include ps4_compat.h" LDFLAGS="$LDFLAGS -L/opt/ps4deps/lib" \
    && make -C lib -j"$(nproc)" && make -C lib install \
    && mkdir -p /opt/ps4deps/include/curl /opt/ps4deps/lib/pkgconfig \
    && cp include/curl/*.h /opt/ps4deps/include/curl/ \
    && sed -e 's|/tmp/curl-7.81.0|/opt/ps4deps|g; s|curl-7.81.0/lib|lib|g' lib/libcurl.la > /dev/null; \
       if [ -f libcurl.pc ]; then cp libcurl.pc /opt/ps4deps/lib/pkgconfig/; \
       elif [ -f lib/libcurl.pc ]; then cp lib/libcurl.pc /opt/ps4deps/lib/pkgconfig/; \
       else printf 'prefix=/opt/ps4deps\nexec_prefix=${prefix}\nlibdir=${exec_prefix}/lib\nincludedir=${prefix}/include\n\nName: libcurl\nDescription: libcurl (PS4 cross, HTTP only)\nVersion: 7.81.0\n\nRequires:\nLibs: -L${libdir} -lcurl\nCflags: -I${includedir}\n' > /opt/ps4deps/lib/pkgconfig/libcurl.pc; fi \
    && rm -rf /tmp/curl*

WORKDIR /src
