import { execSync } from 'node:child_process'
import { rmSync, mkdirSync, readFileSync, existsSync } from 'node:fs'
import { join } from 'node:path'

const ROOT = process.cwd()
const DASH = join(ROOT, 'dashboard')
const DATA = join(ROOT, 'data')

function loadEnvFile(path) {
  if (!existsSync(path)) return
  const lines = readFileSync(path, 'utf8').split(/\r?\n/)
  for (const line of lines) {
    if (!line || line.trim().startsWith('#')) continue
    const idx = line.indexOf('=')
    if (idx === -1) continue
    const key = line.slice(0, idx).trim()
    const value = line.slice(idx + 1).trim()
    if (!process.env[key]) {
      process.env[key] = value
    }
  }
}

function run(cmd, cwd = ROOT) {
  console.log(`[deploy] ${cmd}`)
  execSync(cmd, { stdio: 'inherit', cwd })
}

try {
  // Load .env so PlatformIO sees WIFI_SSID/WIFI_PASSWORD
  loadEnvFile(join(ROOT, '.env'))
  if (!process.env.WIFI_SSID || !process.env.WIFI_PASSWORD) {
    throw new Error('WIFI_SSID and WIFI_PASSWORD must be set in .env')
  }

  // Build dashboard with Vite
  run('npm install', DASH)
  run('npm run build', DASH)

  // Prepare data/ (SPIFFS root)
  rmSync(DATA, { recursive: true, force: true })
  mkdirSync(DATA, { recursive: true })
  run(`node -e "const fs=require('fs');const path=require('path');function cp(src,dst){const st=fs.statSync(src);if(st.isDirectory()){fs.mkdirSync(dst,{recursive:true});for(const f of fs.readdirSync(src)){cp(path.join(src,f), path.join(dst,f));}}else{fs.copyFileSync(src,dst);}}cp('dashboard/dist','data')"`)

  // PlatformIO upload
  const pio = process.platform === 'win32'
    ? '"%USERPROFILE%\\.platformio\\penv\\Scripts\\platformio.exe"'
    : 'pio'

  const portArgIndex = process.argv.indexOf('--upload-port')
  const portArg = portArgIndex > -1 ? ` --upload-port ${process.argv[portArgIndex+1]}` : ''

  run(`${pio} run -e esp32dev -t upload${portArg}`)
  run(`${pio} run -e esp32dev -t uploadfs${portArg}`)

  console.log('[deploy] Done. You can now start the serial monitor.')
} catch (err) {
  console.error('[deploy] Failed:', err?.message || err)
  process.exit(1)
}
