#include "network_html.h"

const char NETWORK_CONTENT[] =


"<div class='network-page'>"

/* =========================================================
 * ACCESS POINT BOX
 * ========================================================= */

"<div class='card'>"

"<h1>📶 Access Point</h1>"

"<div class='info'>"

"<p><b>SSID:</b> <span id='ap-ssid'>ESP32-Assistant</span></p>"
"<p><b>IP Address:</b> <span id='ap-ip'>192.168.4.1</span></p>"
"<p><b>Status:</b> <span id='ap-status'>Active</span></p>"
"<p><b>Clients:</b> <span id='ap-clients'>-</span></p>"

"</div>"

"</div>"


/* =========================================================
 * WIFI MANAGER BOX
 * ========================================================= */

"<div class='card'>"

"<h2>🌐 WiFi Manager</h2>"

"<div class='wifi-status-box'>"

"<p><b>Status:</b> "
"<span id='wifi-status' class='status-disconnected'>Disconnected</span>"
"</p>"

"<p><b>SSID:</b> "
"<span id='wifi-ssid'>Not Connected</span>"
"</p>"

"<p><b>IP Address:</b> "
"<span id='wifi-ip'>-</span>"
"</p>"

"<p><b>Signal:</b> "
"<span id='wifi-rssi'>-</span>"
"</p>"

"</div>"


"<button class='scan-btn' onclick='scanWifi()'>"
"🔍 Scan Networks"
"</button>"


"<div id='networks'>"
"<p>Press scan to find networks</p>"
"</div>"


"</div>"


"</div>"


/* =========================================================
 * CSS
 * ========================================================= */

"<style>"

".network-page{"
"display:flex;"
"flex-direction:column;"
"align-items:center;"
"gap:20px;"
"padding:15px;"
"}"


".card{"
"width:95%;"
"max-width:420px;"
"background:#ffffff;"
"border-radius:15px;"
"padding:20px;"
"box-shadow:0 4px 10px rgba(0,0,0,0.15);"
"}"


".info p,"
".wifi-status-box p{"
"margin:10px 0;"
"font-size:16px;"
"}"


".wifi-status-box{"
"background:#f5f5f5;"
"border-radius:10px;"
"padding:12px;"
"margin-bottom:15px;"
"}"


".status-connected{"
"color:green;"
"font-weight:bold;"
"}"


".status-disconnected{"
"color:red;"
"font-weight:bold;"
"}"


".scan-btn{"
"width:100%;"
"padding:14px;"
"font-size:18px;"
"border-radius:10px;"
"border:none;"
"background:#1976d2;"
"color:white;"
"cursor:pointer;"
"}"


".wifi-btn{"
"width:100%;"
"padding:12px;"
"margin-top:10px;"
"border-radius:10px;"
"border:1px solid #aaa;"
"background:#f5f5f5;"
"font-size:16px;"
"cursor:pointer;"
"}"


"</style>"


/* =========================================================
 * JAVASCRIPT
 * ========================================================= */

"<script>"


/* ---------------------------------------------------------
 * Load WiFi status when page opens
 * --------------------------------------------------------- */

"document.addEventListener('DOMContentLoaded', function(){"

"updateWifiStatus();"

"});"


/* ---------------------------------------------------------
 * Get current WiFi status
 * --------------------------------------------------------- */

"function updateWifiStatus(){"

"fetch('/api/wifi/status')"

".then(function(response){"

"return response.json();"

"})"

".then(function(data){"

"console.log('WiFi Status:', data);"


"if(data.connected === true){"

"document.getElementById('wifi-status').innerText='Connected';"

"document.getElementById('wifi-status').className='status-connected';"

"document.getElementById('wifi-ssid').innerText=data.ssid;"

"document.getElementById('wifi-ip').innerText=data.ip;"

"document.getElementById('wifi-rssi').innerText=data.rssi + ' dBm';"

"}"

"else{"

"document.getElementById('wifi-status').innerText='Disconnected';"

"document.getElementById('wifi-status').className='status-disconnected';"

"document.getElementById('wifi-ssid').innerText='Not Connected';"

"document.getElementById('wifi-ip').innerText='-';"

"document.getElementById('wifi-rssi').innerText='-';"

"}"

"})"

".catch(function(error){"

"console.error('WiFi status error:', error);"

"});"

"}"


/* ---------------------------------------------------------
 * Periodically update status
 * --------------------------------------------------------- */

"setInterval(updateWifiStatus, 5000);"


/* ---------------------------------------------------------
 * Scan WiFi
 * --------------------------------------------------------- */

"function scanWifi(){"

"document.getElementById('networks').innerHTML='Scanning...';"

"fetch('/api/wifi/scan')"

".then(function(response){"

"return response.json();"

"})"

".then(function(data){"

"let html='';"


"if(data.networks.length === 0){"

"html='<p>No networks found</p>';"

"}"


"data.networks.forEach(function(n){"

"html += '<button class=\"wifi-btn\" onclick=\"selectWifi(\\''"

"+ n.ssid.replace(/'/g, \"\\\\'\")"

"+ '\\')\">';"

"html += n.ssid + ' (' + n.rssi + ' dBm)';"

"html += '</button>';"

"});"


"document.getElementById('networks').innerHTML=html;"

"})"

".catch(function(error){"

"document.getElementById('networks').innerHTML='Scan failed';"

"console.error(error);"

"});"

"}"


/* ---------------------------------------------------------
 * Select WiFi
 * --------------------------------------------------------- */

"function selectWifi(ssid){"

"fetch('/api/wifi/select?ssid=' + encodeURIComponent(ssid))"

".then(function(response){"

"return response.json();"

"})"

".then(function(data){"

"if(data.status === 'selected'){"

"let pass = prompt('Enter WiFi password');"


"if(pass !== null){"

"connectWifi(pass);"

"}"

"}"

"});"

"}"


/* ---------------------------------------------------------
 * Connect WiFi
 * --------------------------------------------------------- */

"function connectWifi(password){"

"fetch('/api/wifi/connect',{"

"method:'POST',"

"headers:{"

"'Content-Type':'application/x-www-form-urlencoded'"

"},"

"body:'password=' + encodeURIComponent(password)"

"})"

".then(function(response){"

"return response.json();"

"})"

".then(function(data){"

"alert(JSON.stringify(data));"

"setTimeout(updateWifiStatus, 1000);"

"});"

"}"


"</script>";
