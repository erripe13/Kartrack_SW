#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  Serial.println("LoRa Receiver");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Paramètres identiques à ceux du STM32
  LoRa.setSpreadingFactor(7);        // SF_7
  LoRa.setSignalBandwidth(125E3);    // 125 kHz
  LoRa.setCodingRate4(5);            // CR_4_5
  LoRa.enableCrc();                  // CRC activé
  LoRa.setPreambleLength(8);         // 8 symboles
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet '");

    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
