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

    // Set SO_REUSEADDR to allow rebinding after restart
    int opt = 1;
    if (setsockopt(ntpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Serial.println("[NTP] Failed to set SO_REUSEADDR");
        close(ntpSocket);
        ntpSocket = -1;
        return;
    }

    // Set larger receive buffer for incoming NTP packets
    int rcvbuf = 4096;
    setsockopt(ntpSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    if (bind(ntpSocket, (struct sockaddr *)&ntpAddr, sizeof(ntpAddr)) < 0)
    {
        Serial.println("[NTP] Failed to bind socket on port " + String(NTP_PORT));
        Serial.println("[NTP] Check firewall/permissions and ensure port is not already in use");
        close(ntpSocket);
        ntpSocket = -1;
        return;
    }

    // Set non-blocking mode
    int flags = fcntl(ntpSocket, F_GETFL, 0);
    if (fcntl(ntpSocket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        Serial.println("[NTP] Failed to set non-blocking mode");
        close(ntpSocket);
        ntpSocket = -1;
        return;
    }

    Serial.println("[NTP] Server listening on port " + String(NTP_PORT));
    Serial.println("[NTP] UDP socket ready for incoming requests");
}

void ntpLoop()
{
    if (ntpSocket < 0)
        return;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    uint8_t buffer[NTP_PACKET_SIZE];

    int ret = recvfrom(ntpSocket, (char *)buffer, NTP_PACKET_SIZE, 0,
                       (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (ret > 0)
    {
        // Validate request is valid NTP (Mode=3 client or Mode=4 server)
        uint8_t mode = buffer[0] & 0x7;
        if (!(mode == 3 || mode == 4))
        {
            if (DEBUG_ENABLED)
                Serial.printf("[NTP] Invalid request from %s (Mode=%d)\n",
                              inet_ntoa(clientAddr.sin_addr), mode);
            return;
        }

        // Only respond if GPS is locked; Windows/Linux will reject unsync'd sources
        if (!gpsData.gpsLocked)
        {
            if (DEBUG_ENABLED)
                Serial.printf("[NTP] GPS not locked; rejecting request from %s\n",
                              inet_ntoa(clientAddr.sin_addr));
            return;
        }

        // Clear buffer and build response: LI=0, VN=4, Mode=4 (server)
        memset(buffer, 0, NTP_PACKET_SIZE);
        buffer[0] = (0 << 6) | (4 << 3) | 4;
        buffer[1] = 1;   // Stratum 1 (primary source)
        buffer[2] = 6;   // Poll interval: 6 (64 seconds)
        buffer[3] = 236; // Precision: -20 (~1µs)
        // Root delay & dispersion: 0 (direct GPS source)
        buffer[8] = 0;
        buffer[9] = 0;
        buffer[10] = 0;
        buffer[11] = 16; // 1ms dispersion
        // Reference ID: "GPS\0"
        buffer[12] = 'G';
        buffer[13] = 'P';
        buffer[14] = 'S';
        buffer[15] = 0;

        time_t now = time(nullptr);
        uint32_t secsSince1900 = now + 2208988800UL;

        // Reference Timestamp (Ref, bytes 16-23)
        buffer[16] = (secsSince1900 >> 24) & 0xFF;
        buffer[17] = (secsSince1900 >> 16) & 0xFF;
        buffer[18] = (secsSince1900 >> 8) & 0xFF;
        buffer[19] = secsSince1900 & 0xFF;

        // Origin Timestamp (Orig, bytes 24-31): copy client's transmit (bytes 40-47)
        for (int i = 0; i < 8; i++)
            buffer[24 + i] = buffer[40 + i];

        // Receive Timestamp (Rx, bytes 32-39): now
        buffer[32] = (secsSince1900 >> 24) & 0xFF;
        buffer[33] = (secsSince1900 >> 16) & 0xFF;
        buffer[34] = (secsSince1900 >> 8) & 0xFF;
        buffer[35] = secsSince1900 & 0xFF;

        // Transmit Timestamp (Tx, bytes 40-47): now
        buffer[40] = (secsSince1900 >> 24) & 0xFF;
        buffer[41] = (secsSince1900 >> 16) & 0xFF;
        buffer[42] = (secsSince1900 >> 8) & 0xFF;
        buffer[43] = secsSince1900 & 0xFF;

        sendto(ntpSocket, (const char *)buffer, NTP_PACKET_SIZE, 0,
               (struct sockaddr *)&clientAddr, clientAddrLen);

        gpsData.ntpRequests++;

        if (DEBUG_ENABLED)
        {
            Serial.printf("[NTP] Response sent to %s (GPS locked, %d requests total)\n",
                          inet_ntoa(clientAddr.sin_addr), gpsData.ntpRequests);
        }
    }
}
