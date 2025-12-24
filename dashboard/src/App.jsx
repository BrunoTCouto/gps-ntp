import { useEffect, useMemo, useState } from 'react'
import './App.css'

const API_BASE = import.meta.env.VITE_API_BASE || window.location.origin

const formatTime = (epoch) => {
  if (!epoch) return '—'
  const d = new Date(epoch * 1000)
  return d.toLocaleTimeString(undefined, { hour12: false })
}

const formatDate = (epoch) => {
  if (!epoch) return '—'
  const d = new Date(epoch * 1000)
  return d.toLocaleString(undefined, { dateStyle: 'medium', timeStyle: 'short' })
}

function App() {
  const endpoint = useMemo(() => API_BASE.replace(/\/$/, ''), [])
  const [status, setStatus] = useState(null)
  const [gps, setGps] = useState(null)
  const [ntp, setNtp] = useState(null)
  const [liveTime, setLiveTime] = useState(null)
  const [socketState, setSocketState] = useState('disconnected')

  const wsUrl = useMemo(() => {
    const url = new URL(endpoint)
    url.protocol = url.protocol === 'https:' ? 'wss:' : 'ws:'
    url.pathname = '/ws'
    return url.toString()
  }, [endpoint])


  useEffect(() => {
    let mounted = true

    const fetchAll = async () => {
      try {
        const [s, g, n] = await Promise.all([
          fetch(`${endpoint}/api/status`).then((r) => r.json()),
          fetch(`${endpoint}/api/gps`).then((r) => r.json()),
          fetch(`${endpoint}/api/ntp`).then((r) => r.json()),
        ])
        if (mounted) {
          setStatus(s)
          setGps(g)
          setNtp(n)
          if (n?.currentTime) setLiveTime(n.currentTime)
        }
      } catch (err) {
        console.warn('Fetch failed', err)
      }
    }

    fetchAll()
    const id = setInterval(fetchAll, 4000)
    return () => {
      mounted = false
      clearInterval(id)
    }
  }, [endpoint])

  useEffect(() => {
    let ws
    let retryTimer

    const connect = () => {
      ws = new WebSocket(wsUrl)
      ws.onopen = () => setSocketState('connected')
      ws.onclose = () => {
        setSocketState('disconnected')
        retryTimer = setTimeout(connect, 2000)
      }
      ws.onerror = () => setSocketState('error')
      ws.onmessage = (event) => {
        try {
          const payload = JSON.parse(event.data)
          if (payload?.epoch) setLiveTime(payload.epoch)
        } catch (err) {
          // ignore parse errors
        }
      }
    }

    connect()

    return () => {
      if (retryTimer) clearTimeout(retryTimer)
      if (ws) ws.close()
    }
  }, [wsUrl])

  const uptime = status?.uptime ? `${Math.floor(status.uptime / 3600)}h ${Math.floor((status.uptime % 3600) / 60)}m` : '—'
  const satellites = gps?.satellites ?? '—'
  const rssi = status?.wifiRSSI ? `${status.wifiRSSI} dBm` : '—'
  const connected = status?.wifiConnected

  const timeString = liveTime ? formatTime(liveTime) : '—'

  return (
    <div className="page">
      <section className="hero">
        <div className="badge-row">
          <span className="badge">GPS NTP / ESP32</span>
          <span className="badge">Live Dashboard</span>
          <span className="badge">Socket.IO Time Stream</span>
        </div>
        <div className="hero-title">GPS NTP Server</div>
        <div className="hero-subtitle">
          Real-time view of GPS lock, NTP health, and system vitals. Time sync flows over Socket.IO; metrics auto-refresh every few seconds.
        </div>
        <div className="cta-row">
          <button className="button" onClick={() => window.location.reload()}>Refresh now</button>
          <span className="pill pulse">Socket: {socketState}</span>
        </div>
      </section>

      <section className="grid">
        <div className="card">
          <h3>Universal Time</h3>
          <div className="value glow">{timeString}</div>
          <div className="muted">{formatDate(liveTime)}</div>
          <div className="status-row">
            <span className="pill">Source: {gps?.gpsLocked ? 'GPS' : 'Syncing...'}</span>
            <span className="pill">Socket: {socketState}</span>
          </div>
        </div>

        <div className="card">
          <h3>GPS Lock</h3>
          <div className="value">{gps?.locked ? 'Locked' : 'Searching'}</div>
          <div className="muted">Satellites: {satellites}</div>
          <div className="status-row">
            <span className="pill">Lat: {gps?.latitude?.toFixed?.(6) ?? '—'}</span>
            <span className="pill">Lon: {gps?.longitude?.toFixed?.(6) ?? '—'}</span>
          </div>
        </div>

        <div className="card">
          <h3>WiFi Link</h3>
          <div className="value">{connected ? 'Online' : 'Offline'}</div>
          <div className="muted">RSSI: {rssi}</div>
          <div className="status-row">
            <span className="pill">IP: {status?.localIP ?? '—'}</span>
            <span className="pill">Gateway: {status?.gateway ?? '—'}</span>
          </div>
        </div>

        <div className="card">
          <h3>NTP Engine</h3>
          <div className="value">{ntp?.requests ?? 0} req</div>
          <div className="muted">Synced: {gps?.gpsLocked ? 'Yes' : 'No'}</div>
          <div className="status-row">
            <span className="pill">Port 123</span>
            <span className="pill">Packet: 48 bytes</span>
          </div>
        </div>

        <div className="card">
          <h3>System Health</h3>
          <div className="value">{status?.freeMem ? `${Math.round(status.freeMem / 1024)} KB` : '—'}</div>
          <div className="muted">Uptime: {uptime}</div>
          <div className="sparkline" aria-hidden />
        </div>

        <div className="card">
          <h3>Recent Metrics</h3>
          <div className="list">
            <div className="list-item">
              <span className="muted">Last GPS update</span>
              <span>{gps?.lastUpdate ? `${gps.lastUpdate}s` : '—'}</span>
            </div>
            <div className="list-item">
              <span className="muted">Altitude</span>
              <span>{gps?.altitude ? `${gps.altitude.toFixed?.(1)} m` : '—'}</span>
            </div>
            <div className="list-item">
              <span className="muted">Socket status</span>
              <span>{socketState}</span>
            </div>
          </div>
        </div>
      </section>

      <div className="footer">
        Data from `{endpoint}` · Live via Socket.IO when available, fallback to periodic REST.
      </div>
    </div>
  )
}

export default App
