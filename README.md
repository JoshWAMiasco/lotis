LoTIS
Logger Terminal with IoT and SMS

LoTIS is an open-source remote agriculture logging system built for low-connectivity and off-grid environments. It combines embedded firmware, GSM-based SMS notifications, SD card logging, and a Flutter Android mobile interface using BLE communication.

This repository contains two main projects:

lotis → ESP32-S3 firmware (ESP-IDF based)

lotis_manager_app → Flutter Android mobile application

🌾 Project Purpose

LoTIS is designed for agricultural deployments in remote areas where:

Internet connectivity is unreliable or unavailable

SMS is the most dependable communication channel

Offline data logging is critical

Field configuration must work locally via Bluetooth

The system logs agricultural inventory data, stores structured logs locally, and sends SMS alerts when required.

📂 Repository Structure
LoTIS/
│
├── lotis/                 # ESP32-S3 Firmware (ESP-IDF)
├── lotis_manager_app/     # Flutter Android BLE App
├── LICENSE
└── README.md
1️⃣ lotis (Firmware – ESP-IDF)


🔐 License

This project is licensed under the MIT License.
See the LICENSE file for details.

⚠️ Disclaimer

This project is provided "as is" without warranty of any kind.

The author is not responsible for agricultural loss, data corruption, hardware damage, or improper deployment.

LoTIS is intended for research, educational, and controlled field use.
