#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "config.h"

void setupWiFi()
{
    if (String(WIFI_SSID).length() == 0 || String(WIFI_PASSWORD).length() == 0)
    {
        Serial.println("[WiFi] WIFI_SSID or WIFI_PASSWORD is empty. Check your .env and rebuild.");
        return;
    }

    Serial.println("[WiFi] Connecting to SSID: " + String(WIFI_SSID));
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n[WiFi] Connected!");
        Serial.println("IP address: " + WiFi.localIP().toString());
        Serial.println("Gateway: " + WiFi.gatewayIP().toString());
        Serial.println("DNS: " + WiFi.dnsIP().toString());
    }
    else
    {
        Serial.println("\n[WiFi] Failed to connect");
    }
}

void setupSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("[SPIFFS] Failed to mount SPIFFS");
        return;
    }

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    Serial.println("[SPIFFS] Files:");
    while (file)
    {
        Serial.println("  - " + String(file.name()) + " (" + String(file.size()) + " bytes)");
        file = root.openNextFile();
    }
    Serial.println("[SPIFFS] Mounted successfully");
}
