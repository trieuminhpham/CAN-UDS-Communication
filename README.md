# STM32 CAN & UDS Diagnostic Services

![C/C++](https://img.shields.io/badge/Language-C%2FC%2B%2B-blue)
![Platform](https://img.shields.io/badge/Platform-STM32F4-lightgrey)
![Protocol](https://img.shields.io/badge/Protocol-CAN%20%7C%20UDS-green)
![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-blue)

## 📌 Overview
This repository contains the embedded firmware implementation of **CAN (Controller Area Network)** communication and **UDS (Unified Diagnostic Services - ISO 14229)** protocols on an STM32 microcontroller. 

This project was developed entirely from scratch in C/C++ as part of an Automotive Embedded Software internship training program. It demonstrates the ability to configure hardware peripherals, manage CAN frames, and handle automotive diagnostic requests.

## ⚙️ Key Features
* **CAN Communication:** Configured CAN peripheral for reliable transmission and reception of standard/extended frames.
* **UDS Protocol Stack:** Implemented core diagnostic services according to ISO 14229 standard.
* **Hardware Abstraction:** Utilized STM32 HAL library for hardware initialization and peripheral control.

## 🛠️ Implemented UDS Services
The firmware successfully handles the following UDS requests:
* `0x22 - Read Data By Identifier (RDBI):` Allows the client to request specific data from the MCU.
* `0x27 - Security Access:` Implemented Seed & Key algorithm to unlock protected services.
* `0x2E - Write Data By Identifier (WDBI):` Enables the client to write configuration data to the MCU (requires Security Access).

## 🧰 Hardware & Software Stack
* **Microcontroller:** STM32F405RGTx (ARM Cortex-M4)
* **Development IDE:** STM32CubeIDE
* **Framework:** STM32 HAL Library
* **Testing/Simulation Tools:** CANoe / CANalyzer / PCAN-View (Update with your specific tool)

## 📂 Project Structure

* `Core/Inc/` - Header files (.h) for UDS handlers and CAN configs
* `Core/Src/` - Source files (.c) for main logic and protocol implementation
* `Drivers/` - STM32 HAL Drivers & CMSIS
* `README.md` - Project overview

## 🚀 How to Build and Run

1. Clone this repository using the following command:
   `git clone https://github.com/trieuminhpham/CAN-UDS-Communication.git`
2. Open **STM32CubeIDE**.
3. Go to `File` > `Import` > `Existing Projects into Workspace`.
4. Select the cloned directory and click `Finish`.
5. Click the **Build** hammer icon to compile the project.
6. Connect your ST-Link to the STM32 board and click **Debug/Run** to flash the firmware.

---
*Note: The source code in this repository was independently developed based on training requirements.*
