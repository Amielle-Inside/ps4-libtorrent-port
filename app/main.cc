// PS4 libtorrent port — smoke test
// Links the ported libtorrent statically and initializes the engine.
#include <cstdio>
#include <torrent/torrent.h>
// PS4 kernel bootstrap
#include <orbis/libkernel.h>

int main() {
    std::printf("[ps4-torrent] boot\n");
    // initialize thread library + engine (exercises Poll::create() → epoll)
    torrent::initialize();
    std::printf("[ps4-torrent] engine initialized OK\n");
    torrent::cleanup();
    std::printf("[ps4-torrent] bye\n");
    return 0;
}
