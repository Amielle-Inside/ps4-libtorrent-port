0001 — configure.ac: allow building with external sceNetEpoll poll backend
=========================================================================
rakshasa/libtorrent configure.ac:65 aborts unless kqueue or epoll is
detected. On PS4 neither exists; we supply our own Poll backend
(poll_scenetpoll.cc) compiled into the library. This patch adds the
--with-scenetpoll switch that:
  - skips the kqueue/epoll hard error
  - defines TORRENT_USE_SCENETPOLL
  - adds src/torrent/system/poll_scenetpoll.cc to the build

Also drops the libcurl hard requirement for the MVP (--without-curl),
since HTTP tracker access is done through raw sockets by the app.

Apply from repo root:  patch -p1 < patches/libtorrent/0001-*.patch
NOTE: this file documents the intent; the actual .patch is generated
against the cloned tree by scripts/build.sh (auto-generated on first
run and then frozen into this repo once it applies cleanly).
