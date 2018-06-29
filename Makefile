all: default Wifi101_FirmwareUpdater

deps: Wifi101_FirmwareUpdater

Wifi101_FirmwareUpdater: Wifi101_FirmwareUpdater_linux64.zip
	unzip Wifi101_FirmwareUpdater_linux64.zip

Wifi101_FirmwareUpdater_linux64.zip:
	wget https://github.com/arduino-libraries/WiFi101-FirmwareUpdater/releases/download/0.7.0/Wifi101_FirmwareUpdater_linux64.zip

default:
	mkdir -p build
	cd build && cmake ../
	cd build && make

clean:
	rm -rf build
