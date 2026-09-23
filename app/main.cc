// PS4 libtorrent port — smoke test
// Links the ported libtorrent statically and initializes the engine.
#include <cstdio>
#include <torrent/torrent.h>
#include <torrent/download.h>
// PS4 kernel bootstrap
#include <orbis/libkernel.h>

int main() {
    std::printf("[ps4-torrent] boot\n");
    // initialize thread library + engine (this exercises Poll::create() → our sceNetEpoll shim)
    torrent::initialize();
    std::printf("[ps4-torrent] engine initialized, libtorrent %s\n",
                torrent::version());
    torrent::cleanup();
    std::printf("[ps4-torrent] bye\n");
    return 0;
}
