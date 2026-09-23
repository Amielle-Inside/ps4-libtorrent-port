# ps4-libtorrent-port ★

Native BitTorrent client for PlayStation 4 (homebrew, jailbroken/exploited consoles).

Port of [libtorrent (rakshasa)](https://github.com/rakshasa/libtorrent) (C++17) using the
[OpenOrbis PS4 Toolchain](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain) — no official SDK.

## Architecture

```
ps4-libtorrent-port/
├── docker/           # OpenOrbis build image (podman/docker, nothing installed on host)
├── patches/          # patches applied to upstream rakshasa/libtorrent
│   └── libtorrent/   #   0001-configure: allow cross-compile without kqueue/epoll detection
│                     #   0002-add poll backend on sceNetEpoll (PS4 kernel epoll)
├── shim/             # PS4 platform shims linked before libtorrent
│   ├── ps4_poll.cc   #   Poll backend: sceNetEpollCreate/Control/Wait
│   ├── ps4_net.cc    #   socket()/connect()/getaddrinfo() glue (newlib → sceNet)
│   └── ps4_init.cc   #   sceNetPool/sceNetInit bootstrap
├── app/              # the actual .elf homebrew (SDL2 UI later; CLI log first)
├── scripts/          # fetch-deps.sh, build.sh, make-gpkg.sh
└── docs/             # study notes: SDK findings, syscall coverage, risks
```

## Phases

1. **[current]** Cross-compile libtorrent core for PS4 (no OpenSSL, no libcurl —
   HTTP tracker via raw sockets) + minimal test elf that initializes the engine.
2. Link ps4-openssl → MSE/encrypted peers.
3. SDL2/borealis UI: add magnet, list torrents, progress.

## Prerequisites

- podman or docker (build runs fully containerized)
- a PS4 with a homebrew enabler (GoldHEN etc.) to run the `.elf`/`.pkg`

## Build

```bash
./scripts/build.sh          # builds the docker image + libtorrent + app elf
```

## License

MIT (upstream libtorrent is GPL-2.0 — combined work distributed as GPL-2.0;
this repo's original shim/app code is MIT).
