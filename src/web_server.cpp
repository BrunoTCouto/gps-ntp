#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include "config.h"
#include "state.h"

static AsyncWebServer server(API_PORT);
static AsyncWebSocket ws("/ws");
static unsigned long lastTimeBroadcast = 0;

void webServerSetup()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // Serve Swagger UI for API documentation
    server.on("/swagger", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/swagger.html", "text/html"); });

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

    ws.onEvent([](AsyncWebSocket *serverPtr, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
               {
                   if (type == WS_EVT_CONNECT)
                   {
                       time_t now = time(nullptr);
                       if (now > 0)
                       {
                           String payload = String("{\"epoch\":") + String((uint32_t)now) + "}";
                           client->text(payload);
                       }
                   }
                   else if (type == WS_EVT_DISCONNECT)
                   {
                       // no-op
                   } });

    server.addHandler(&ws);

    server.begin();
    Serial.println("[WebServer] Started on port " + String(API_PORT));
}

void webSocketLoop()
{
    ws.cleanupClients();

    unsigned long nowMillis = millis();
    if (nowMillis - lastTimeBroadcast < 1000)
    {
        return;
    }
    lastTimeBroadcast = nowMillis;

    time_t now = time(nullptr);
    if (now > 0)
    {
        String payload = String("{\"epoch\":") + String((uint32_t)now) + "}";
        ws.textAll(payload);
    }
}
