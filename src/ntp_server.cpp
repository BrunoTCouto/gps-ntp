#include <Arduino.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "config.h"
#include "state.h"

static int ntpSocket = -1;
static struct sockaddr_in ntpAddr;

void ntpSetup()
{
    ntpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ntpSocket < 0)
    {
        Serial.println("[NTP] Failed to create socket");
        return;
    }

    memset(&ntpAddr, 0, sizeof(ntpAddr));
    ntpAddr.sin_family = AF_INET;
    ntpAddr.sin_port = htons(NTP_PORT);
    ntpAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(ntpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(ntpSocket, (struct sockaddr *)&ntpAddr, sizeof(ntpAddr)) < 0)
    {
        Serial.println("[NTP] Failed to bind socket");
        return;
    }

    fcntl(ntpSocket, F_SETFL, O_NONBLOCK);
    Serial.println("[NTP] Server listening on port " + String(NTP_PORT));
}

void ntpLoop()
{
    if (ntpSocket < 0)
        return;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    uint8_t buffer[NTP_PACKET_SIZE];

    int ret = recvfrom(ntpSocket, buffer, NTP_PACKET_SIZE, 0,
                       (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (ret > 0)
    {
        memset(buffer, 0, NTP_PACKET_SIZE);
        buffer[0] = (0 << 6) | (3 << 3) | 4;
        buffer[1] = NTP_STRATUM;
        buffer[2] = 6;
        buffer[3] = 0xEC;
        buffer[12] = 'G';
        buffer[13] = 'P';
        buffer[14] = 'S';
        buffer[15] = 0;

        time_t now = time(nullptr);
        uint32_t secsSince1900 = now + 2208988800UL;

        buffer[40] = (secsSince1900 >> 24) & 0xFF;
        buffer[41] = (secsSince1900 >> 16) & 0xFF;
        buffer[42] = (secsSince1900 >> 8) & 0xFF;
        buffer[43] = secsSince1900 & 0xFF;

        sendto(ntpSocket, buffer, NTP_PACKET_SIZE, 0,
               (struct sockaddr *)&clientAddr, clientAddrLen);

        gpsData.ntpRequests++;

        if (DEBUG_ENABLED)
        {
            Serial.printf("[NTP] Request from %s - Total: %d\n",
                          inet_ntoa(clientAddr.sin_addr), gpsData.ntpRequests);
        }
    }
}
