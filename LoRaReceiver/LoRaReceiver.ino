#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  LoRa.setPreambleLength(8);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String payload = "";
    while (LoRa.available()) {
      payload += (char)LoRa.read();
    }

    Serial.println("---- GPS DATA ----");
    //Serial.println(payload);

    float lat, lon, alt, speed;
    int sats, fix;

    int idx = 0;
    char *token;
    char buffer[128];
    payload.toCharArray(buffer, sizeof(buffer));
    token = strtok(buffer, ",");

    while (token != NULL) {
      switch (idx) {
        case 0: lat = atof(token); break;
        case 1: lon = atof(token); break;
        case 2: alt = atof(token); break;
        case 3: sats = atoi(token); break;
        case 4: fix = atoi(token); break;
        case 5: speed = atof(token) * 1.852; break; // convert to km/h
      }
      token = strtok(NULL, ",");
      idx++;
    }

    Serial.print("Latitude       : "); Serial.println(lat, 5);
    Serial.print("Longitude      : "); Serial.println(lon, 5);
    Serial.print("Altitude (m)   : "); Serial.println(alt, 1);
    Serial.print("Satellites     : "); Serial.println(sats);
    Serial.print("Fix quality    : "); Serial.println(fix);
    Serial.print("Speed (km/h)   : "); Serial.println(speed, 2);
    Serial.print("RSSI           : "); Serial.println(LoRa.packetRssi());
    Serial.println("----------------------------\n");
  }
}

