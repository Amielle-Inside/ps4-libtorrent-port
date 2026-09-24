#!/usr/bin/env bash
set -e
source /opt/cross-file-ps4.env
cd /tmp
cat > t.cc <<'EOF'
#include <cstdint>
#include <atomic>
int main() { std::atomic<uint64_t> x(0); return x.load(); }
EOF
ps4-clang++ $CXXFLAGS -D_GNU_SOURCE t.cc $LDFLAGS -o t.elf && echo ATOMIC_OK_GNU
