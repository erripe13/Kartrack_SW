// FORMAT :
// gyroX, gyroY, gyroZ,
// accelX, accelY, accelZ,
// lat, lon, alt, sats, fix, speed

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

    Serial.println("---- LoRa DATA ----");
    Serial.println(payload);  // Debug brut si besoin

    int gyro[3], accel[3], sats, fix;
    float lat, lon, alt, speed;

    char buffer[128];
    payload.toCharArray(buffer, sizeof(buffer));
    char *token = strtok(buffer, ",");
    int idx = 0;

    while (token != NULL) {
      switch (idx) {
        case 0: gyro[0] = atoi(token); break;
        case 1: gyro[1] = atoi(token); break;
        case 2: gyro[2] = atoi(token); break;
        case 3: accel[0] = atoi(token); break;
        case 4: accel[1] = atoi(token); break;
        case 5: accel[2] = atoi(token); break;
        case 6: lat = atof(token); break;
        case 7: lon = atof(token); break;
        case 8: alt = atof(token); break;
        case 9: sats = atoi(token); break;
        case 10: fix = atoi(token); break;
        case 11: speed = atof(token) * 1.852; break;  // conversion en km/h
      }
      token = strtok(NULL, ",");
      idx++;
    }

    Serial.print("Gyro XYZ      : ");
    Serial.print(gyro[0]); Serial.print(" ");
    Serial.print(gyro[1]); Serial.print(" ");
    Serial.println(gyro[2]);

    Serial.print("Accel XYZ     : ");
    Serial.print(accel[0]); Serial.print(" ");
    Serial.print(accel[1]); Serial.print(" ");
    Serial.println(accel[2]);

    Serial.print("Latitude      : "); Serial.println(lat, 5);
    Serial.print("Longitude     : "); Serial.println(lon, 5);
    Serial.print("Altitude (m)  : "); Serial.println(alt, 1);
    Serial.print("Satellites    : "); Serial.println(sats);
    Serial.print("Fix quality   : "); Serial.println(fix);
    Serial.print("Speed (km/h)  : "); Serial.println(speed, 2);
    Serial.print("RSSI          : "); Serial.println(LoRa.packetRssi());
    Serial.println("-----------------------------\n");
  }
}
