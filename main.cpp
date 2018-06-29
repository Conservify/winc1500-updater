/*
  FirmwareUpdate.h - Firmware Updater for WiFi101 / WINC1500.
  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <WiFi101.h>

#include <spi_flash/include/spi_flash.h>

bool isBigEndian();
uint32_t fromNetwork32(uint32_t from);
uint16_t fromNetwork16(uint16_t from);
uint32_t toNetwork32(uint32_t to);
uint16_t toNetwork16(uint16_t to);

typedef struct __attribute__((__packed__)) {
    uint8_t command;
    uint32_t address;
    uint32_t arg1;
    uint16_t payloadLength;
    // payloadLenght bytes of data follows...
} UartPacket;

static const int MAX_PAYLOAD_SIZE = 1024;

#define CMD_READ_FLASH        0x01
#define CMD_WRITE_FLASH       0x02
#define CMD_ERASE_FLASH       0x03
#define CMD_MAX_PAYLOAD_SIZE  0x50
#define CMD_HELLO             0x99

void testVersion() {
    // Print a welcome message
    Serial.println("WiFi101 firmware check.");
    Serial.println();

    // Check for the presence of the shield
    Serial.print("WiFi101 shield: ");
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("NOT PRESENT");
        return; // don't continue
    }
    Serial.println("DETECTED");

    // Print firmware version on the shield
    String fv = WiFi.firmwareVersion();
    String latestFv;
    Serial.print("Firmware version installed: ");
    Serial.println(fv);

    if (REV(GET_CHIPID()) >= REV_3A0) {
        // model B
        latestFv = WIFI_FIRMWARE_LATEST_MODEL_B;
    } else {
        // model A
        latestFv = WIFI_FIRMWARE_LATEST_MODEL_A;
    }

    // Print required firmware version
    Serial.print("Latest firmware version available : ");
    Serial.println(latestFv);

    // Check if the latest version is installed
    Serial.println();
    if (fv == latestFv) {
        Serial.println("Check result: PASSED");
    } else {
        Serial.println("Check result: NOT PASSED");
        Serial.println(" - The firmware version on the shield do not match the");
        Serial.println("   version required by the library, you may experience");
        Serial.println("   issues or failures.");
    }

}

void setup() {
    Serial.begin(115200);

    #ifdef FK_NATURALIST
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 16;
    static constexpr uint8_t WIFI_PIN_RST = 15;
    static constexpr uint8_t WIFI_PIN_EN = 38;
    #endif

    #ifdef FK_CORE
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 9;
    static constexpr uint8_t WIFI_PIN_RST = 10;
    static constexpr uint8_t WIFI_PIN_EN = 11;
    #endif

    #ifdef ADAFRUIT_FEATHER
    static constexpr uint8_t WIFI_PIN_CS = 8;
    static constexpr uint8_t WIFI_PIN_IRQ = 7;
    static constexpr uint8_t WIFI_PIN_RST = 4;
    static constexpr uint8_t WIFI_PIN_EN = 2;
    #endif

    WiFi.setPins(WIFI_PIN_CS, WIFI_PIN_IRQ, WIFI_PIN_RST, WIFI_PIN_EN);
    if (WiFi.status() == WL_NO_SHIELD) {
        pinMode(13, OUTPUT);
        while (true) {
            delay(100);
            digitalWrite(13, HIGH);
            delay(100);
            digitalWrite(13, LOW);
        }
    }

    while (!Serial && millis() < 2000) {

    }

    if (Serial) {
        testVersion();

        while (true) {
        }
    }

    nm_bsp_init();
    if (m2m_wifi_download_mode() != M2M_SUCCESS) {
        Serial.println("Failed to put the WiFi module in download mode");

        while (true) {
        }
    }
}

void receivePacket(UartPacket *pkt, uint8_t *payload) {
    // Read command
    uint8_t *p = reinterpret_cast<uint8_t *>(pkt);
    uint16_t l = sizeof(UartPacket);
    while (l > 0) {
        int c = Serial.read();
        if (c == -1)
            continue;
        *p++ = c;
        l--;
    }

    // Convert parameters from network byte order to cpu byte order
    pkt->address = fromNetwork32(pkt->address);
    pkt->arg1 = fromNetwork32(pkt->arg1);
    pkt->payloadLength = fromNetwork16(pkt->payloadLength);

    // Read payload
    l = pkt->payloadLength;
    while (l > 0) {
        int c = Serial.read();
        if (c == -1)
            continue;
        *payload++ = c;
        l--;
    }
}

static UartPacket pkt;
static uint8_t payload[MAX_PAYLOAD_SIZE];

void loop() {
    receivePacket(&pkt, payload);

    if (pkt.command == CMD_HELLO) {
        if (pkt.address == 0x11223344 && pkt.arg1 == 0x55667788)
            Serial.print("v10000");
    }

    if (pkt.command == CMD_MAX_PAYLOAD_SIZE) {
        uint16_t res = toNetwork16(MAX_PAYLOAD_SIZE);
        Serial.write(reinterpret_cast<uint8_t *>(&res), sizeof(res));
    }

    if (pkt.command == CMD_READ_FLASH) {
        uint32_t address = pkt.address;
        uint32_t len = pkt.arg1;
        if (spi_flash_read(payload, address, len) != M2M_SUCCESS) {
            Serial.println("ER");
        } else {
            Serial.write(payload, len);
            Serial.print("OK");
        }
    }

    if (pkt.command == CMD_WRITE_FLASH) {
        uint32_t address = pkt.address;
        uint32_t len = pkt.payloadLength;
        if (spi_flash_write(payload, address, len) != M2M_SUCCESS) {
            Serial.print("ER");
        } else {
            Serial.print("OK");
        }
    }

    if (pkt.command == CMD_ERASE_FLASH) {
        uint32_t address = pkt.address;
        uint32_t len = pkt.arg1;
        if (spi_flash_erase(address, len) != M2M_SUCCESS) {
            Serial.print("ER");
        } else {
            Serial.print("OK");
        }
    }
}


