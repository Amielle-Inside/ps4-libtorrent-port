#!/usr/bin/env bash
# Runs INSIDE the container: packages torrent_test.elf into a PS4 .pkg
set -e
cd /src/app
TOOL=/lib/OpenOrbisSDK/bin/linux
TITLE_ID=BREW00123
CONTENT_ID=IV0000-BREW00123_00-TORRENT000000000

$TOOL/create-fself -in=torrent_test.elf -out=torrent_test.oelf --eboot "eboot.bin" --paid 0x3800000000000011

mkdir -p sce_sys/about
cp /lib/OpenOrbisSDK/samples/_common/sce_sys/about/right.sprx sce_sys/about/ 2>/dev/null || \
  find /lib/OpenOrbisSDK -name right.sprx -exec cp {} sce_sys/about/ \;

# param.sfo
$TOOL/PkgTool.Core sfo_new sce_sys/param.sfo
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo APP_TYPE      --type Integer --maxsize 4   --value 1
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo APP_VER       --type Utf8    --maxsize 8   --value 1.00
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo ATTRIBUTE     --type Integer --maxsize 4   --value 0
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo CATEGORY      --type Utf8    --maxsize 4   --value gd
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo CONTENT_ID    --type Utf8    --maxsize 48  --value $CONTENT_ID
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo SYSTEM_VER    --type Integer --maxsize 4   --value 0
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo TITLE         --type Utf8    --maxsize 128 --value "PS4 libtorrent smoke test"
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo TITLE_ID      --type Utf8    --maxsize 12  --value $TITLE_ID
$TOOL/PkgTool.Core sfo_setentry sce_sys/param.sfo VERSION       --type Utf8    --maxsize 8   --value 1.00

# icon0 (any valid png; generate tiny one via clang? use sample's if present)
find /lib/OpenOrbisSDK/samples -name icon0.png -exec cp {} sce_sys/ \; 2>/dev/null || true

$TOOL/create-gp4 -out pkg.gp4 --content-id=$CONTENT_ID --files "eboot.bin sce_sys/param.sfo sce_sys/about/right.sprx"
$TOOL/PkgTool.Core pkg_build pkg.gp4 .
ls -la *.pkg
echo "PKG BUILD OK"
