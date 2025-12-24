#ifndef CONFIG_H
#define CONFIG_H

// ============ WiFi Configuration ============
// Values injected via build flags from environment variables (see README/.env.example)
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

#define WIFI_TIMEOUT 10000 // ms

// ============ GPS Configuration ============
#define GPS_RX_PIN 26 // ESP32 RX connected to SIM7000 TX (MODEM_TX)
#define GPS_TX_PIN 27 // ESP32 TX connected to SIM7000 RX (MODEM_RX)
#define GPS_BAUD_RATE 115200
#define SATELLITES_MIN 4 // Minimum satellites for lock

// ============ NTP Configuration ============
#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_STRATUM 1 // Stratum 1 (directly connected to time source)

// ============ API Configuration ============
#define API_PORT 80
#define API_UPDATE_INTERVAL 2000 // ms, for dashboard polling
#define GPS_PRINT_INTERVAL 5000  // ms, for serial output

// ============ Debug Configuration ============
#define DEBUG_ENABLED true
#define DEBUG_BAUD 115200

#endif
