#include "system_html.h"

const char SYSTEM_CONTENT[] =
"<div class='network-page'>"

/* --- SYSTEM PROFILE --- */
"<div class='card'>"
"<h1>📊 Core Properties</h1>"
"<div class='info'>"
"<div class='stat-row'><b>Silicon Chip:</b> <span class='ready'>ESP32-S3</span></div>"
"<div class='stat-row'><b>CPU Core Clock:</b> <span>240 MHz</span></div>"
"<div class='stat-row'><b>IDF Framework:</b> <span>v5.5 Stable</span></div>"
"<div class='stat-row'><b>Device Uptime:</b> <span id='sys-uptime' class='online'>0s</span></div>"
"</div>"
"</div>"

/* --- METRICS & TELEMETRY --- */
"<div class='card'>"
"<h2>🌡️ Live Telemetry</h2>"
"<div class='wifi-status-box'>"
"<div class='stat-row'><b>Core Temp:</b> <span id='sys-temp'>-- °C</span></div>"
"<div class='stat-row'><b>Free SRAM Heap:</b> <span id='sys-heap' class='ready'>-- KB</span></div>"
"<div class='stat-row'><b>Min Free Ever:</b> <span id='sys-minheap'>-- KB</span></div>"
"<div class='stat-row'><b>Est. Current draw:</b> <span id='sys-power' style='color:#79bbff;'>-- mA</span></div>"
"</div>"
"<button class='scan-btn' onclick='fetchTelemetry()'>🔄 Force Refresh</button>"
"</div>"

"</div>"

/* --- ASYNCHRONOUS UPDATE ENGINE --- */
"<script>"
"function fetchTelemetry() {"
"  fetch('/api/system/status')"
"    .then(res => res.json())"
"    .then(data => {"
"      document.getElementById('sys-uptime').innerText = data.uptime;"
"      document.getElementById('sys-heap').innerText = (data.free_heap / 1024).toFixed(1) + ' KB';"
"      document.getElementById('sys-minheap').innerText = (data.min_heap / 1024).toFixed(1) + ' KB';"
"      document.getElementById('sys-temp').innerText = data.temperature.toFixed(1) + ' °C';"
"      document.getElementById('sys-power').innerText = data.est_current + ' mA';"
"    })"
"    .catch(err => console.error('Telemetry update drop:', err));"
"}"
"document.addEventListener('DOMContentLoaded', fetchTelemetry);"
"setInterval(fetchTelemetry, 1000);" // Dynamic refresh loop every 1 second
"</script>";

