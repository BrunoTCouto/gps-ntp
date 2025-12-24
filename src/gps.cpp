#include <Arduino.h>
#include <TinyGPS++.h>
#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "state.h"

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);

void gpsSetup()
{
    gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("[GPS] Serial initialized at " + String(GPS_BAUD_RATE) + " baud");

    Serial.println("[SIM7000] Enabling GNSS and NMEA output...");
    gpsSerial.println("AT");
    delay(200);
    gpsSerial.println("AT+CGNSPWR=1");
    delay(500);
    gpsSerial.println("AT+CGNSTST=1");
    delay(500);
}

void gpsLoop()
{
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isUpdated())
    {
        gpsData.latitude = gps.location.lat();
        gpsData.longitude = gps.location.lng();
        gpsData.altitude = gps.altitude.meters();
        gpsData.satCount = gps.satellites.value();
        gpsData.gpsLocked = (gpsData.satCount >= SATELLITES_MIN);
        gpsData.lastGpsUpdate = millis();

        if (gpsData.gpsLocked && gps.date.isValid() && gps.time.isValid())
        {
            struct tm tm{};
            tm.tm_year = gps.date.year() - 1900;
            tm.tm_mon = gps.date.month() - 1;
            tm.tm_mday = gps.date.day();
            tm.tm_hour = gps.time.hour();
            tm.tm_min = gps.time.minute();
            tm.tm_sec = gps.time.second();
            tm.tm_isdst = 0;

            time_t gpsTime = mktime(&tm);
            timeval tv = {.tv_sec = gpsTime, .tv_usec = 0};
            settimeofday(&tv, nullptr);

            if (DEBUG_ENABLED)
            {
                Serial.printf("[GPS] Time synced: %04d-%02d-%02d %02d:%02d:%02d\n",
                              gps.date.year(), gps.date.month(), gps.date.day(),
                              gps.time.hour(), gps.time.minute(), gps.time.second());
            }
        }
    }
}
