#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

struct GpsData
{
    bool gpsLocked = false;
    int satCount = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    uint32_t ntpRequests = 0;
    unsigned long lastGpsUpdate = 0;
};

extern GpsData gpsData;

#endif
