#!/bin/bash

PORT=$1
BOARD=$2

if [[ -z "$PORT" ]]; then
    echo "Usage: upgrade.sh PORT (fkcore|fkn|feather)"
    exit 2
fi

if [[ -z "$BOARD" ]]; then
    echo "Usage: upgrade.sh PORT (fkcore|fkn|feather)"
    exit 2
fi

FK_CORE=OFF
FK_NATURALIST=OFF
ADAFRUIT_FEATHER=OFF

case "$BOARD" in
    fkcore)
        FK_CORE=ON
    ;;
    fkn)
        FK_NATURALIST=ON
    ;;
    feather)
        ADAFRUIT_FEATHER=ON
    ;;
esac

set -xe

make deps
mkdir -p build
cd build
cmake ../ -DFK_CORE=$FK_CORE -DFK_NATURALIST=$FK_NATURALIST -DADAFRUIT_FEATHER=$ADAFRUIT_FEATHER
cd ..
cmake --build build

sudo chmod 777 $PORT
flasher --binary build/winc-firmware.bin --port $PORT

sleep 5
sudo chmod 777 $PORT
./Wifi101_FirmwareUpdater/winc1500-uploader --port $PORT --firmware m2m_aio_3a0.bin

flasher --binary build/winc-firmware.bin --port $PORT --tail
