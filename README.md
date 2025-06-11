# Kartrack – Software embarqué

> **Dépôts associés** :  
> - [Kartrack_SW](https://github.com/erripe13/Kartrack_SW) – **Ce dépôt** – Code embarqué STM32 (FreeRTOS, GPS, LoRa, ICM...)  
> - [Kartrack_HW](https://github.com/erripe13/Kartrack_HW) – Schéma et PCB du système  
> - [Kartrack_APP](https://github.com/erripe13/Kartrack_APP) – Application d’analyse et visualisation des données

## Présentation du projet Kartrack

Kartrack est un système embarqué de chronométrage et de télémétrie pour karting, conçu pour être fixé sur le volant. Il enregistre en temps réel les données de course (position GPS, accélérations, orientation) et les transmet en LoRa à l'appli desktop et les enregistre sur carte microSD. L’objectif est de proposer un outil complet pour les pilotes amateurs ou en compétition : analyse post-course, visualisation en temps réel par l'équipe, appel aux stands, comparaisons de trajectoires.

---

## Contenu de ce dépôt : Kartrack_SW

Ce dépôt contient le **firmware embarqué** destiné à la carte **STM32F746G-DISCO**, reposant sur **FreeRTOS** et utilisant les périphériques STM32 (UART, SPI, I2C, DMA, timers, etc.).

### Branches principales

- **`main`**  
  Intègre la communication fonctionnelle avec les trois composants critiques :
  - **GPS Quectel L76** via UART
  - **LoRa RA-01 (Ai-Thinker)** via SPI
  - **ICM-45605** (accéléromètre/gyroscope) via I²C  
  La synchronisation, la récupération des données, et leur traitement sont assurés par des tâches FreeRTOS.

- **`interface`**  
  Branche dédiée à l’**interface utilisateur** développée sous **TouchGFX**. Elle permet d’exploiter l’écran TFT tactile de la Discovery pour afficher les informations temps réel (GPS, chrono, état de liaison LoRa, etc.).

- **`SD`**  
  Tentative d’intégration du système de fichiers **FatFs** avec gestion DMA via **SDMMC1**.  
  Ce travail n’a pas pu être finalisé à cause de problèmes récurrents avec l’initialisation incorrecte du **DMA** générée par **STM32CubeMX**. La pile progresse jusqu'au montage du volume, mais les écritures échouent (`FR_DISK_ERR`) du fait de l'absence de déclenchement de la callback `BSP_SD_WriteCpltCallback`.

---

- **GPS Quectel L76**  
  - Communication : UART6 avec réception en IT + traitement NMEA en tâche dédiée.
  - Format des données : NMEA GGA, RMC, GSV
  - Extraction : position, date, heure, vitesse, course, qualité du fix, satellites visibles.

- **ICM-45605 (accéléromètre / gyroscope)**  
  - Bus : I²C1  
  - Fréquences d’échantillonnage : 200 Hz (accel + gyro)
  - Plages : ±16 g et ±2000 dps  
  - Lecture directe des registres, sans FIFO.

- **LoRa RA-01 (Ai-Thinker)**  
  - Bus : SPI2  
  - Envoi périodique toutes les 500 ms d’un message contenant :
    ```
    gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, lat, lon, alt, sats, fix, speed
    ```
  - Fréquence : 433 MHz  
  - SF7, BW 125kHz, CR 4/5, puissance 20 dBm

#### Tâches FreeRTOS :

| Nom           | Périphérique | Priorité | Fonction |
|---------------|--------------|----------|----------|
| `LoRa_init`   | SPI2, GPIO   | Haute    | Init IMU + LoRa, acquisition + envoi périodique |
| `gpsTask`     | UART6        | Haute    | Réception NMEA avec buffer double et sémaphore |
| `Gyro_Init`   | I2C1         | Idle     | (réservée / vide pour l’instant implémentée dans LoRa_init pour des raisons de temps manquant) |

---

## Architecture logicielle

- **`main.c`**  
  Configure les horloges, initialise les périphériques, démarre FreeRTOS.

- **`freertos.c`**  
  Création des tâches, configuration de l’IMU, GPS et LoRa.

- **`gps.c`**  
  Traitement complet des trames NMEA avec parsing maison. Stockage dans `L76_GPS_Data_t`.

- **`inv_imu_driver.c/.h`**  
  Pilote du capteur ICM-45605 basé sur la lecture directe de registres. Configuration LN, sans FIFO.

- **`LoRa.c`**  
  Pilotage bas niveau du module RA-01 avec gestion de l’émission via SPI + contrôle GPIO.
