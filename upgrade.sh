#!/bin/bash

PORT=$1

set -xe

make

sudo chmod 777 $PORT
flasher --binary build/winc-firmware.bin --port $PORT

sleep 5
sudo chmod 777 $PORT
./Wifi101_FirmwareUpdater/winc1500-uploader --port $PORT --firmware m2m_aio_3a0.bin

flasher --binary build/winc-firmware.bin --port $PORT --tail
