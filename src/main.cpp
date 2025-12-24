#include <Arduino.h>
// #include minimal includes and use modularized components
#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "system.h"
#include "gps.h"
#include "ntp_server.h"
#include "web_server.h"

void setup()
{
    Serial.begin(DEBUG_BAUD);
    delay(1000);

    Serial.println("\n\n╔═══════════════════════════════════════╗");
    Serial.println("║   GPS NTP Server - ESP32 + SIM7000G  ║");
    Serial.println("╚═══════════════════════════════════════╝\n");

    setupSPIFFS();
    gpsSetup();
    setupWiFi();
    ntpSetup();
    webServerSetup();

    Serial.println("\n[STARTUP] System ready!");
    Serial.println("[STARTUP] Waiting for GPS lock...\n");
}

void loop()
{
    gpsLoop();
    ntpLoop();
    webSocketLoop();

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > API_UPDATE_INTERVAL)
    {
        lastPrint = millis();
        if (DEBUG_ENABLED)
        {
            Serial.printf("[GPS] Locked: %s | Satellites: %2d | Lat: %9.6f | Lon: %10.6f | Alt: %7.1f m\n",
                          gpsData.gpsLocked ? "Y" : "N",
                          gpsData.satCount,
                          gpsData.latitude,
                          gpsData.longitude,
                          gpsData.altitude);
        }
    }

    delay(10);
}
