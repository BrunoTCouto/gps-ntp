#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include "config.h"
#include "state.h"

static AsyncWebServer server(API_PORT);

void webServerSetup()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.on("/api/gps", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = "{";
        json += "\"locked\":" + String(gpsData.gpsLocked ? "true" : "false") + ",";
        json += "\"satellites\":" + String(gpsData.satCount) + ",";
        json += "\"latitude\":" + String(gpsData.latitude, 6) + ",";
        json += "\"longitude\":" + String(gpsData.longitude, 6) + ",";
        json += "\"altitude\":" + String(gpsData.altitude, 2) + ",";
        json += "\"lastUpdate\":" + String(gpsData.lastGpsUpdate);
        json += "}";
        request->send(200, "application/json", json); });

    server.on("/api/ntp", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        time_t now = time(nullptr);
        String json = "{";
        json += "\"requests\":" + String(gpsData.ntpRequests) + ",";
        json += "\"currentTime\":" + String(now) + ",";
        json += "\"synchronized\":" + String(gpsData.gpsLocked ? "true" : "false");
        json += "}";
        request->send(200, "application/json", json); });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = "{";
        json += "\"uptime\":" + String(millis() / 1000) + ",";
        json += "\"wifiConnected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
        json += "\"wifiRSSI\":" + String(WiFi.RSSI()) + ",";
        json += "\"gpsLocked\":" + String(gpsData.gpsLocked ? "true" : "false") + ",";
        json += "\"freeMem\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"localIP\":\"" + WiFi.localIP().toString() + "\"";
        json += "}";
        request->send(200, "application/json", json); });

    server.on("/api/health", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "OK"); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      {
        const String url = request->url();

        // Let API endpoints return a standard 404
        if (url.startsWith("/api"))
        {
            request->send(404, "text/plain", "Not Found");
            return;
        }

        // SPA fallback: serve index.html so the client router can handle the path
        if (SPIFFS.exists("/index.html"))
        {
            request->send(SPIFFS, "/index.html", "text/html");
        }
        else
        {
            request->send(500, "text/plain", "index.html not found");
        } });

    server.begin();
    Serial.println("[WebServer] Started on port " + String(API_PORT));
}
