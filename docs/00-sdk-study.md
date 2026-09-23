docs/00-sdk-study.md — findings from the FOSS PS4 SDK study (sources verified 2026-09-23)

# PS4 FOSS SDK study — verified findings

## Toolchains
1. **OpenOrbis PS4 Toolchain** (chosen): clang+binutils, releases v0.1–v0.5.4,
   C++17 threading supported (std::thread/std::mutex, release notes #182).
   https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain
2. **OrbisDev**: alternative (fjtrujy), less active (~2022). Not used.

## Network stack (verified in headers + shadPS4 implementation)
- `Net.h` exposes: sceNetSocket/Bind/Connect/Listen/Accept/Send/Recv/
  Sendto/Recvfrom/Sendmsg/Recvmsg/Getsockopt/Setsockopt/Shutdown/Close,
  **sceNetEpoll{Create,Control,Wait,Destroy,Abort}** (lines 177–185),
  sceNetResolver*, sceNetInetPton/Ntop, sceNetHtonl/s...
- newlib layer exposes BSD API directly: `socket()`, `connect()`,
  `getaddrinfo()` via <netdb.h> — proven in production homebrew
  `garlic-worker/src/ps4/http.c` (438 lines, no sceNetInit needed).
- UDP works (SOCK_DGRAM + sendto/recvfrom) → uTP viable.

## libtorrent rakshasa port analysis (source: git clone master)
- Poll abstraction: `src/torrent/system/poll.h` — `Poll::create()` factory,
  backends `poll_epoll.cc` / `poll_kqueue.cc` in separate TUs → clean seam
  for a `poll_scenetpoll.cc` backend.
- `configure.ac:65` errors if neither kqueue nor epoll detected → patch needed
  to accept our backend (`--with-scenetpoll`).
- `configure.ac:70` checks fallocate/accept4/pipe2/kqueue1/inotify_init1 —
  none exist on PS4; answered via `config.site-ps4`.
- Deps: libcurl + zlib + pthreads (all in ps4-openorbis-portlibs);
  OpenSSL optional (`--without-openssl` → no MSE, fine for MVP).
- DNS: getaddrinfo used in src/torrent/net/address_info.cc ✓.

## Crypto (phase 2)
- PacBrew ps4-openorbis-portlibs ships: mbedtls, libsodium.
- OpenSSL 1.1.1 port exists: cy33hc/ps4-openssl (branch OpenSSL_1_1_1-ps4),
  build documented with pacbrew.

## Portlibs available (verified via GitHub API, ps4-openorbis-portlibs)
SDL2(+image/mixer/ttf), ffmpeg, libmpv, libcurl, mbedtls, libsodium, zlib,
bzip2, libjson-c, tinyxml2, freetype, libpng, glm, libarchive, freegnm, ...

## Roadmap
1. MVP: no OpenSSL, sceNetEpoll poll backend, HTTP tracker via raw sockets.
2. MSE via ps4-openssl.
3. UI: SDL2 + borealis (has PS4 target in wiki, GLES2).
