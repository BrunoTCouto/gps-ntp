# GPS NTP Server with React Dashboard

This project creates an NTP (Network Time Protocol) server using the **SIM7000G** GPS module paired with an **ESP32** microcontroller, complete with a web-based React dashboard.

## Features

- 📡 **GPS-based Time Synchronization**: Automatically sync time from GPS satellites
- 🕐 **NTP Server**: Serve accurate time to your network devices
- 🌐 **Web Dashboard**: Monitor GPS status, NTP activity, and system health in real-time
- 📱 **Responsive UI**: Works on desktop, tablet, and mobile devices
- 🔌 **WiFi Connected**: Connect via WiFi for both web access and NTP services

## Hardware Requirements

- **ESP32** Development Board
- **SIM7000G** GPS/GPRS Module
- USB cable for programming
- Micro USB power supply (5V)
- External GPS antenna (recommended)

## Wiring Diagram

### SIM7000G to ESP32

| SIM7000G | ESP32 Pin | Purpose |
|----------|-----------|---------|
| RX (TX) | GPIO 17 | Serial RX |
| TX (RX) | GPIO 16 | Serial TX |
| GND | GND | Ground |
| VCC | 3.3V | Power (with regulator recommended) |

**Note**: The SIM7000G requires ~2A at peak, so use an external power supply if possible.

## Software Setup

### Prerequisites

1. **PlatformIO** installed in VS Code
2. **Node.js** (v14+) for React build
3. **USB Driver** for ESP32 (CH340 or similar)

### Installation Steps

1. **Clone/Download the project**:
   ```bash
   git clone <repository-url>
   cd gps-ntp
   ```

2. **Create your WiFi `.env`** (loaded automatically by PlatformIO):
   ```bash
   cp .env.example .env
   # edit .env
   WIFI_SSID=YourWifiName
   WIFI_PASSWORD=YourWifiPassword
   ```

3. **Configure Serial Port in `platformio.ini`**:
   ```ini
   upload_port = COM3  ; Change to your ESP32 port
   ```

4. **Build & upload firmware + dashboard + SPIFFS** (one command):
   ```bash
   npm run deploy
   ```
   This runs the Vite build, copies it into `data/`, then flashes firmware and SPIFFS.

## Project Structure

```
gps-ntp/
├── src/
│   └── main.cpp              # ESP32 firmware
├── dashboard/
│   ├── src/
│   │   ├── App.js            # React main component
│   │   ├── App.css           # Dashboard styles
│   │   ├── index.js          # React entry point
│   │   └── index.css         # Global styles
│   ├── public/
│   │   └── index.html        # HTML template
│   └── package.json          # React dependencies
├── data/                     # SPIFFS payload (auto-generated)
│   ├── index.html
│   └── assets/
├── platformio.ini            # PlatformIO configuration
├── build.bat                 # Build script (Windows)
└── build.sh                  # Build script (Linux/macOS)
```

## API Endpoints

The ESP32 exposes REST API endpoints for the dashboard:

### GET `/api/gps`
Returns GPS data:
```json
{
  "locked": true,
  "satellites": 12,
  "latitude": -23.550520,
  "longitude": -46.633308,
  "altitude": 750.5,
  "lastUpdate": 1234567890
}
```

### GET `/api/ntp`
Returns NTP server status:
```json
{
  "requests": 42,
  "currentTime": 1703425234,
  "synchronized": true
}
```

### GET `/api/status`
Returns system status:
```json
{
  "uptime": 3600,
  "wifiConnected": true,
  "wifiRSSI": -55,
  "gpsLocked": true,
  "freeMem": 98304
}
```

## API Docs (Swagger)

- Open interactive docs at `http://<esp32-ip>/swagger`.
- The OpenAPI spec is served at `http://<esp32-ip>/openapi.json`.
- Note: The Swagger UI page loads assets from a public CDN. Internet access is required for the UI to render. If you need offline docs, we can bundle the assets into SPIFFS.

## Access the Dashboard

Once everything is set up:

1. **Find ESP32 IP**: Check your router or serial monitor output
2. **Open in browser**: `http://<esp32-ip>`
3. **Dashboard shows**:
   - GPS lock status and satellite count
   - Current coordinates and altitude
   - NTP synchronization status
   - Time information
   - System uptime and memory usage
   - WiFi connection strength

## Using the NTP Server

Configure your devices to use the ESP32 as an NTP server:

**Linux/macOS**:
```bash
sudo ntpdate <esp32-ip>
```

**Windows**:
```cmd
w32tm /config /manualpeerlist:"<esp32-ip>" /syncfromflags:manual /update
```

**Router/Network**:
Most modern routers allow setting a custom NTP server in their web interface.

## Serial Monitor Output

Monitor the device through PlatformIO's Serial Monitor to see:
- GPS status updates every 5 seconds
- WiFi connection status
- NTP requests handled
- Time synchronization events

```
[STARTUP] GPS NTP Server Starting...
[SPIFFS] Mounted successfully
[WiFi] Connecting to SSID: MyNetwork
[WiFi] Connected!
IP address: 192.168.1.100
[GPS] Serial initialized
[NTP] Server listening on port 123
[WebServer] Started on port 80
[GPS] Time synced: 2024-12-24 10:30:45
[GPS] Locked: YES, Satellites: 12, Lat: -23.550520, Lon: -46.633308
```

## Troubleshooting

### GPS Not Locking
- Check antenna connection
- Ensure clear view of sky
- GPS typically locks in 1-5 minutes
- Requires minimum 4 satellites for time sync

### Dashboard Not Loading
- Check ESP32 IP address
- Verify SPIFFS upload completed
- Check WiFi connection
- Clear browser cache and reload

### NTP Requests Not Working
- Firewall may be blocking port 123
- Ensure GPS is locked first
- Check router allows NTP traffic

### Serial Monitor Shows Garbage
- Wrong baud rate (should be 115200)
- USB cable issue
- Wrong COM port selected

## Customization

### Change NTP Port
In `src/main.cpp`, modify:
```cpp
#define NTP_PORT 123
```

### Change Dashboard Update Interval
In `dashboard/src/App.js`, modify:
```javascript
const interval = setInterval(fetchData, 2000); // milliseconds
```

### Add More GPS Data
Expand the `/api/gps` endpoint in `src/main.cpp` to include speed, course, etc.

## Performance Notes

- **Memory**: ESP32 has ~4MB total, ~1.2MB for SPIFFS by default
- **GPS**: Typical UART speed is 9600 baud
- **Update Rate**: Dashboard refreshes every 2 seconds
- **NTP Accuracy**: ±50-200ms depending on GPS quality

## License

This project is open source. Feel free to modify and distribute.

## Support

For issues or questions:
1. Check the troubleshooting section
2. Review serial monitor output for errors
3. Verify hardware connections
4. Ensure firmware and dashboard versions match

---

**Happy time syncing! 🕐**
