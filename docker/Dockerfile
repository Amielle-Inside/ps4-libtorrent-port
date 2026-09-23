FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential clang clang-tools lld autoconf automake libtool pkg-config \
    autoconf-archive make cmake git curl wget xz-utils ccache python3 \
    libtool-bin m4 texinfo && \
    rm -rf /var/lib/apt/lists/*

# OpenOrbis toolchain release
ARG OO_VERSION=0.5.4
RUN cd /opt && \
    curl -sL -o oo.tar.gz \
      https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/releases/download/v${OO_VERSION}/openorbis-ps4-toolchain-${OO_VERSION}.tar.gz && \
    tar xf oo.tar.gz && rm oo.tar.gz

ENV OO_PS4_TOOLCHAIN=/opt/OpenOrbis-PS4-Toolchain
ENV PATH=$OO_PS4_TOOLCHAIN/bin:$PATH

# Cross environment used by autotools builds
COPY cross-file-ps4.env /opt/cross-file-ps4.env

WORKDIR /src
