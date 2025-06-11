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

