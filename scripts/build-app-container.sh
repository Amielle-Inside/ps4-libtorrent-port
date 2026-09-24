#!/usr/bin/env bash
# Runs INSIDE the container (see build.sh). Builds the test elf.
set -e
source /opt/cross-file-ps4.env
cd /src/app
make -f Makefile.oo
ls -la torrent_test.elf
