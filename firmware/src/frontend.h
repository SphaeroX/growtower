#ifndef FRONTEND_H
#define FRONTEND_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GrowTower Controller</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; color: #fff; padding: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        h1 { text-align: center; margin-bottom: 30px; font-size: 2rem; background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text; }
        .status-card { background: rgba(255, 255, 255, 0.1); backdrop-filter: blur(10px); border-radius: 20px; padding: 25px; margin-bottom: 20px; border: 1px solid rgba(255, 255, 255, 0.2); }
        .status-title { font-size: 1.1rem; color: #94a3b8; margin-bottom: 15px; text-transform: uppercase; letter-spacing: 1px; }
        .status-value { display: flex; align-items: center; justify-content: space-between; margin-bottom: 12px; }
        .status-value:last-child { margin-bottom: 0; }
        .status-label { color: #cbd5e1; }
        .status-indicator { display: flex; align-items: center; gap: 8px; font-weight: 600; }
        .status-indicator.on { color: #4ade80; }
        .status-indicator.off { color: #f87171; }
        .dot { width: 12px; height: 12px; border-radius: 50%; background: currentColor; box-shadow: 0 0 10px currentColor; }
        .control-group { margin-bottom: 25px; }
        .control-label { display: block; margin-bottom: 10px; color: #cbd5e1; font-size: 0.95rem; }
        .slider-container { display: flex; align-items: center; gap: 15px; }
        input[type="range"] { flex: 1; -webkit-appearance: none; appearance: none; height: 8px; background: rgba(255, 255, 255, 0.2); border-radius: 4px; outline: none; }
        input[type="range"]::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 24px; height: 24px; background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%); border-radius: 50%; cursor: pointer; box-shadow: 0 2px 10px rgba(74, 222, 128, 0.5); transition: transform 0.2s; }
        input[type="range"]::-webkit-slider-thumb:hover { transform: scale(1.1); }
        input[type="range"]::-moz-range-thumb { width: 24px; height: 24px; background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%); border-radius: 50%; cursor: pointer; border: none; box-shadow: 0 2px 10px rgba(74, 222, 128, 0.5); }
        .slider-value { min-width: 50px; text-align: right; font-weight: 600; color: #4ade80; }
        .button-group { display: flex; gap: 10px; }
        button { flex: 1; padding: 15px 25px; border: none; border-radius: 12px; font-size: 1rem; font-weight: 600; cursor: pointer; transition: all 0.3s; text-transform: uppercase; letter-spacing: 1px; }
        .btn-on { background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%); color: #1a1a2e; }
        .btn-on:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(74, 222, 128, 0.4); }
        .btn-off { background: linear-gradient(135deg, #f87171 0%, #ef4444 100%); color: #1a1a2e; }
        .btn-off:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(248, 113, 113, 0.4); }
        button:disabled { opacity: 0.5; cursor: not-allowed; transform: none !important; }
        .time-inputs { display: flex; align-items: center; gap: 15px; }
        .time-input { flex: 1; }
        input[type="number"] { width: 100%; padding: 12px 15px; border: 2px solid rgba(255, 255, 255, 0.2); border-radius: 10px; background: rgba(255, 255, 255, 0.1); color: #fff; font-size: 1.1rem; text-align: center; transition: border-color 0.3s; }
        input[type="number"]:focus { outline: none; border-color: #4ade80; }
        input[type="text"], textarea { width: 100%; padding: 12px 15px; border: 2px solid rgba(255, 255, 255, 0.2); border-radius: 10px; background: rgba(255, 255, 255, 0.1); color: #fff; font-size: 1rem; transition: border-color 0.3s; font-family: inherit; }
        input[type="text"]:focus, textarea:focus { outline: none; border-color: #4ade80; }
        .time-separator { color: #94a3b8; font-weight: 600; }
        .save-btn { margin-top: 15px; width: 100%; padding: 15px; background: linear-gradient(135deg, #60a5fa 0%, #3b82f6 100%); color: white; border: none; border-radius: 12px; font-size: 1rem; font-weight: 600; cursor: pointer; transition: all 0.3s; text-transform: uppercase; }
        .toggle-container { display: flex; align-items: center; justify-content: space-between; margin-bottom: 15px; padding: 12px; background: rgba(255, 255, 255, 0.05); border-radius: 10px; }
        .toggle-label { color: #cbd5e1; font-size: 0.95rem; }
        .toggle-switch { position: relative; width: 50px; height: 26px; }
        .toggle-switch input { opacity: 0; width: 0; height: 0; }
        .toggle-slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #475569; transition: 0.4s; border-radius: 26px; }
        .toggle-slider:before { position: absolute; content: ""; height: 20px; width: 20px; left: 3px; bottom: 3px; background-color: white; transition: 0.4s; border-radius: 50%; }
        .toggle-switch input:checked + .toggle-slider { background-color: #4ade80; }
        .toggle-switch input:checked + .toggle-slider:before { transform: translateX(24px); }
        .toggle-status { font-size: 0.85rem; margin-top: 5px; text-align: center; }
        .toggle-status.active { color: #4ade80; }
        .toggle-status.inactive { color: #f87171; }
        .reset-section { margin-top: 30px; border-top: 2px solid rgba(255, 255, 255, 0.1); padding-top: 20px; }
        .btn-reset { width: 100%; padding: 15px; background: linear-gradient(135deg, #f87171 0%, #ef4444 100%); color: white; border: none; border-radius: 12px; font-size: 1rem; font-weight: 600; cursor: pointer; transition: all 0.3s; }
        .btn-reset:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(248, 113, 113, 0.4); }
        .save-btn:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(59, 130, 246, 0.4); }
        .message { position: fixed; bottom: 20px; right: 20px; padding: 15px 25px; border-radius: 10px; font-weight: 600; animation: slideIn 0.3s ease; z-index: 1000; }
        @keyframes slideIn { from { transform: translateX(100%); opacity: 0; } to { transform: translateX(0); opacity: 1; } }
        .message.success { background: #4ade80; color: #1a1a2e; }
        .message.error { background: #f87171; color: white; }
        .current-time { text-align: center; color: #94a3b8; font-size: 0.9rem; margin-top: 20px; }
        .section-title { font-size: 1.3rem; color: #fff; margin-bottom: 20px; padding-bottom: 10px; border-bottom: 2px solid rgba(255, 255, 255, 0.1); }
        .log-entry { background: rgba(255, 255, 255, 0.05); border-radius: 10px; padding: 15px; margin-bottom: 10px; position: relative; }
        .log-entry-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
        .log-entry-time { font-size: 0.8rem; color: #60a5fa; font-weight: 600; }
        .log-entry-delete { background: transparent; border: none; color: #f87171; cursor: pointer; padding: 5px 10px; font-size: 1.2rem; opacity: 0.6; transition: opacity 0.2s; }
        .log-entry-delete:hover { opacity: 1; }
        .log-entry-text { color: #e2e8f0; font-size: 0.95rem; line-height: 1.4; }
        .log-empty { text-align: center; color: #64748b; padding: 20px; font-style: italic; }
        .log-input-group { display: flex; gap: 10px; margin-bottom: 15px; }
        .log-input-group input { flex: 1; }
        .log-input-group button { flex: 0 0 auto; padding: 12px 20px; }
        .log-actions { display: flex; gap: 10px; margin-top: 15px; }
        .btn-clear-log { background: linear-gradient(135deg, #f87171 0%, #ef4444 100%); color: white; }
        @media (max-width: 480px) { h1 { font-size: 1.5rem; } .status-card { padding: 20px; } button { padding: 12px 20px; font-size: 0.9rem; } }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 GrowTower Controller</h1>
        <div class="status-card">
            <div class="status-title">Current Status</div>
            <div class="status-value"><span class="status-label">Light:</span><span class="status-indicator" id="lightStatus"><span class="dot"></span><span id="lightText">-</span></span></div>
            <div class="status-value"><span class="status-label">Fan:</span><span class="status-indicator"><span id="fanValue">-</span>%</span></div>
            <div class="status-value"><span class="status-label">Fan Range:</span><span class="status-indicator"><span id="fanMin">-</span>% - <span id="fanMax">-</span>%</span></div>
            <div class="status-value"><span class="status-label">Light Timer:</span><span class="status-indicator"><span id="lightOn">-</span>:00 - <span id="lightOff">-</span>:00</span></div>
            <div class="status-value"><span class="status-label">Device:</span><span class="status-indicator" id="hostnameDisplay">-</span></div>
        </div>
        <div class="status-card">
            <div class="section-title">Light Control</div>
            <div class="button-group"><button class="btn-on" onclick="setLight(true)">ON</button><button class="btn-off" onclick="setLight(false)">OFF</button></div>
        </div>
        <div class="status-card">
            <div class="section-title">Fan Control</div>
            <div class="control-group"><label class="control-label">Speed: <span id="fanPercent">30</span>%</label><div class="slider-container"><input type="range" id="fanSlider" min="0" max="100" value="30" oninput="updateFanLabel(this.value)"><span class="slider-value" id="fanDisplay">30%</span></div></div>
            <button class="save-btn" onclick="setFan()">Set Fan</button>
        </div>
        <div class="status-card">
            <div class="section-title">Fan Range</div>
            <div class="control-group"><label class="control-label">Minimum: <span id="fanMinLabel">0</span>%</label><div class="slider-container"><input type="range" id="fanMinSlider" min="0" max="100" value="0" oninput="updateFanMinLabel(this.value)"><span class="slider-value" id="fanMinDisplay">0%</span></div></div>
            <div class="control-group"><label class="control-label">Maximum: <span id="fanMaxLabel">100</span>%</label><div class="slider-container"><input type="range" id="fanMaxSlider" min="0" max="100" value="100" oninput="updateFanMaxLabel(this.value)"><span class="slider-value" id="fanMaxDisplay">100%</span></div></div>
            <button class="save-btn" onclick="setFanRange()">Save Range</button>
        </div>
        <div class="status-card">
            <div class="section-title">Light Timer</div>
            <div class="toggle-container">
                <span class="toggle-label">Automatic Timer</span>
                <label class="toggle-switch">
                    <input type="checkbox" id="timerToggle" onchange="toggleTimer()">
                    <span class="toggle-slider"></span>
                </label>
            </div>
            <div class="toggle-status" id="timerStatus">Active</div>
            <div id="timerSettings">
                <div class="control-group"><label class="control-label">On Time</label><div class="time-inputs"><input type="number" id="onHour" min="0" max="23" value="18" class="time-input" oninput="updateLightDuration()"></div></div>
                <div class="control-group"><label class="control-label">Duration (hours)</label><div class="time-inputs"><input type="number" id="durationHours" min="1" max="24" value="18" class="time-input" oninput="updateLightDuration()"></div></div>
                <div style="margin: 15px 0; padding: 12px; background: rgba(74, 222, 128, 0.2); border-radius: 10px; text-align: center; border: 1px solid rgba(74, 222, 128, 0.3);"><span style="color: #4ade80; font-weight: 600;">Light Period: <span id="lightOnCalc">-</span>:00 - <span id="lightOffCalc">-</span>:00 (<span id="lightDuration">-</span>h)</span></div>
                <button class="save-btn" onclick="setLightTimer()">Save Timer</button>
            </div>
        </div>
        <div class="status-card">
            <div class="section-title">Plant Tracker</div>
            <div class="control-group">
                <div style="display: flex; gap: 10px; margin-bottom: 15px;">
                    <button id="btnSeedling" onclick="setPhase('seedling')" style="flex:1; padding:12px; background:rgba(139,92,246,0.3); border:2px solid rgba(139,92,246,0.5); border-radius:8px; color:white; cursor:pointer; font-weight:600;">Seedling</button>
                    <button id="btnVeg" onclick="setPhase('veg')" style="flex:1; padding:12px; background:rgba(16,185,129,0.3); border:2px solid rgba(16,185,129,0.5); border-radius:8px; color:white; cursor:pointer; font-weight:600;">Veg</button>
                    <button id="btnFlower" onclick="setPhase('flower')" style="flex:1; padding:12px; background:rgba(245,158,11,0.3); border:2px solid rgba(245,158,11,0.5); border-radius:8px; color:white; cursor:pointer; font-weight:600;">Flower</button>
                </div>
                <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin-bottom: 15px;">
                    <div style="background:rgba(139,92,246,0.2); padding:15px; border-radius:10px; text-align:center;"><div style="font-size:0.8rem; color:#94a3b8;">Seedling</div><div id="seedlingDays" style="font-size:1.5rem; font-weight:bold; color:#8b5cf6;">0</div><div style="font-size:0.75rem; color:#94a3b8;">days</div></div>
                    <div style="background:rgba(16,185,129,0.2); padding:15px; border-radius:10px; text-align:center;"><div style="font-size:0.8rem; color:#94a3b8;">Veg</div><div id="vegDays" style="font-size:1.5rem; font-weight:bold; color:#10b981;">0</div><div style="font-size:0.75rem; color:#94a3b8;">days</div></div>
                    <div style="background:rgba(245,158,11,0.2); padding:15px; border-radius:10px; text-align:center;"><div style="font-size:0.8rem; color:#94a3b8;">Flower</div><div id="flowerDays" style="font-size:1.5rem; font-weight:bold; color:#f59e0b;">0</div><div style="font-size:0.75rem; color:#94a3b8;">days</div></div>
                </div>
                <div style="background:rgba(74,222,128,0.2); padding:15px; border-radius:10px; text-align:center; margin-bottom:15px;"><div style="font-size:0.9rem; color:#94a3b8;">Total Days</div><div id="totalDays" style="font-size:2rem; font-weight:bold; color:#4ade80;">0</div><div style="font-size:0.8rem; color:#94a3b8;">days old</div></div>
                <div style="display:flex; gap: 10px;"><select id="resetPhaseSelect" style="flex:1; padding:10px; background:rgba(255,255,255,0.1); border:1px solid rgba(255,255,255,0.2); border-radius:8px; color:white;"><option value="seedling">Seedling</option><option value="veg">Veg</option><option value="flower">Flower</option><option value="all">All</option></select><button onclick="resetPhase()" style="padding:10px 20px; background:#ef4444; border:none; border-radius:8px; color:white; cursor:pointer; font-weight:600;">Reset</button></div>
            </div>
        </div>
        <div class="status-card">
            <div class="section-title">Watering</div>
            <div style="display: flex; gap: 10px; margin-bottom: 15px;">
                <input type="text" id="waterNote" placeholder="Optional: Note..." maxlength="100" style="flex: 1;">
                <button onclick="waterPlant()" style="padding: 12px 20px; background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%); border: none; border-radius: 10px; color: white; font-weight: 600; cursor: pointer; white-space: nowrap;">Water</button>
            </div>
        </div>
        <div class="status-card">
            <div class="section-title">Grow Logbook</div>
            <div class="log-input-group" style="flex-direction: column;">
                <textarea id="logInput" rows="3" placeholder="What did you do? e.g. watered, gave nutrients..." maxlength="200" style="resize: vertical;" onkeydown="if(event.key==='Enter' && !event.shiftKey){event.preventDefault();addLogEntry()}"></textarea>
                <button class="save-btn" style="margin-top:0;" onclick="addLogEntry()">Add Entry</button>
            </div>
            <div id="logEntries"></div>
            <div class="log-actions">
                <button class="btn-clear-log" onclick="clearLogbook()" style="width:100%; padding:12px; background:linear-gradient(135deg,#f87171 0%,#ef4444 100%); border:none; border-radius:10px; color:white; font-weight:600; cursor:pointer;">Clear Logbook</button>
            </div>
        </div>
        <div class="status-card">
            <div class="section-title">Network Settings</div>
            <div class="control-group"><label class="control-label">Device Name (for growtower.local)</label><input type="text" id="hostnameInput" placeholder="growtower" maxlength="31"></div>
            <button class="save-btn" onclick="setHostname()">Save & Restart</button>
        </div>
        <div class="status-card reset-section">
            <div class="section-title">Factory Settings</div>
            <p style="color: #94a3b8; font-size: 0.9rem; margin-bottom: 15px;">Reset all settings to default values. This will erase all user settings and restart the device.</p>
            <button class="btn-reset" onclick="resetToDefaults()">Reset to Factory Settings</button>
        </div>
        <div class="current-time">Last Update: <span id="lastUpdate">-</span></div>
    </div>
    <script>
        let currentStatus = {};
        function showMessage(text, type = 'success') { const msg = document.createElement('div'); msg.className = `message ${type}`; msg.textContent = text; document.body.appendChild(msg); setTimeout(() => msg.remove(), 3000); }
        function updateFanLabel(value) { document.getElementById('fanPercent').textContent = value; document.getElementById('fanDisplay').textContent = value + '%'; }
        function updateFanMinLabel(value) { document.getElementById('fanMinLabel').textContent = value; document.getElementById('fanMinDisplay').textContent = value + '%'; }
        function updateFanMaxLabel(value) { document.getElementById('fanMaxLabel').textContent = value; document.getElementById('fanMaxDisplay').textContent = value + '%'; }
        function updateLightDuration() { const onHour = parseInt(document.getElementById('onHour').value) || 0; const duration = parseInt(document.getElementById('durationHours').value) || 1; let offHour = onHour + duration; if (offHour >= 24) offHour -= 24; document.getElementById('lightOnCalc').textContent = String(onHour).padStart(2, '0'); document.getElementById('lightOffCalc').textContent = String(offHour).padStart(2, '0'); document.getElementById('lightDuration').textContent = duration; }
        function updateUI(status) {
            currentStatus = status;
            const lightStatus = document.getElementById('lightStatus'); const lightText = document.getElementById('lightText');
            if (status.light) { lightStatus.className = 'status-indicator on'; lightText.textContent = 'ON'; } else { lightStatus.className = 'status-indicator off'; lightText.textContent = 'OFF'; }
            document.getElementById('fanValue').textContent = status.fan;
            document.getElementById('fanMin').textContent = status.fanMin;
            document.getElementById('fanMax').textContent = status.fanMax;
            document.getElementById('lightOn').textContent = String(status.lightOn).padStart(2, '0');
            let calculatedOff = status.lightOn + status.lightDuration;
            if (calculatedOff >= 24) calculatedOff -= 24;
            document.getElementById('lightOff').textContent = String(calculatedOff).padStart(2, '0');
            document.getElementById('hostnameDisplay').textContent = status.hostname + '.local';
            updatePhaseUI(status);
            if (document.activeElement !== document.getElementById('fanSlider')) { document.getElementById('fanSlider').value = status.fan; updateFanLabel(status.fan); }
            if (document.activeElement !== document.getElementById('fanMinSlider')) { document.getElementById('fanMinSlider').value = status.fanMin; updateFanMinLabel(status.fanMin); }
            if (document.activeElement !== document.getElementById('fanMaxSlider')) { document.getElementById('fanMaxSlider').value = status.fanMax; updateFanMaxLabel(status.fanMax); }
            if (document.activeElement !== document.getElementById('onHour')) { document.getElementById('onHour').value = status.lightOn; }
            if (document.activeElement !== document.getElementById('durationHours')) { document.getElementById('durationHours').value = status.lightDuration; updateLightDuration(); }
            if (document.activeElement !== document.getElementById('timerToggle')) { document.getElementById('timerToggle').checked = status.timerEnabled; document.getElementById('timerStatus').textContent = status.timerEnabled ? 'Active' : 'Disabled'; document.getElementById('timerStatus').className = 'toggle-status ' + (status.timerEnabled ? 'active' : 'inactive'); document.getElementById('timerSettings').style.opacity = status.timerEnabled ? '1' : '0.5'; }
            updateLightDuration();
            if (document.activeElement !== document.getElementById('hostnameInput')) { document.getElementById('hostnameInput').value = status.hostname; }
            const now = new Date(); document.getElementById('lastUpdate').textContent = now.toLocaleTimeString('en-US');
        }
        async function fetchStatus() { try { const response = await fetch('/api/status'); const status = await response.json(); updateUI(status); } catch (error) { console.error('Error fetching status:', error); } }
        async function setLight(on) { try { const response = await fetch(`/api/light?state=${on ? 1 : 0}`); const result = await response.json(); if (result.success) { showMessage(on ? 'Light turned on' : 'Light turned off'); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error switching light', 'error'); } }
        async function setFan() { const value = document.getElementById('fanSlider').value; try { const response = await fetch(`/api/fan?speed=${value}`); const result = await response.json(); if (result.success) { showMessage(`Fan set to ${value}%`); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error setting fan', 'error'); } }
        async function setFanRange() { const min = document.getElementById('fanMinSlider').value; const max = document.getElementById('fanMaxSlider').value; try { const response = await fetch(`/api/fanrange?min=${min}&max=${max}`); const result = await response.json(); if (result.success) { showMessage(`Fan range: ${min}%-${max}%`); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error saving', 'error'); } }
        async function setLightTimer() { const on = document.getElementById('onHour').value; const duration = document.getElementById('durationHours').value; try { const response = await fetch(`/api/timer?on=${on}&duration=${duration}`); const result = await response.json(); if (result.success) { showMessage(`Timer: ${String(on).padStart(2, '0')}:00 - ${result.offHour}:00 (${duration}h)`); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error saving', 'error'); } }
        async function toggleTimer() { const enabled = document.getElementById('timerToggle').checked; try { const response = await fetch(`/api/timerenable?enabled=${enabled ? 1 : 0}`); const result = await response.json(); if (result.success) { showMessage(enabled ? 'Timer enabled' : 'Timer disabled'); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error toggling timer', 'error'); } }
        async function resetToDefaults() { if (!confirm('Reset all settings to factory defaults? The device will restart.')) { return; } try { const response = await fetch('/api/reset'); const result = await response.json(); if (result.success) { showMessage('Resetting to factory defaults...'); setTimeout(() => { window.location.href = 'http://growtower.local'; }, 5000); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error resetting', 'error'); } }
        async function setHostname() { const hostname = document.getElementById('hostnameInput').value.trim(); if (!hostname) { showMessage('Please enter a name', 'error'); return; } if (!/^[a-zA-Z0-9-]+$/.test(hostname)) { showMessage('Only letters, numbers and hyphens allowed', 'error'); return; } try { const response = await fetch(`/api/hostname?name=${encodeURIComponent(hostname)}`); const result = await response.json(); if (result.success) { showMessage('Device restarting...'); setTimeout(() => { window.location.href = `http://${hostname}.local`; }, 5000); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error saving', 'error'); } }
        let currentPhaseStatus = { seedling: {active:false}, veg: {active:false}, flower: {active:false} };
        async function setPhase(phase) { var phaseToSet = phase; if (phase === 'seedling' && currentPhaseStatus.seedling.active) phaseToSet = 'none'; else if (phase === 'veg' && currentPhaseStatus.veg.active) phaseToSet = 'none'; else if (phase === 'flower' && currentPhaseStatus.flower.active) phaseToSet = 'none'; try { const response = await fetch(`/api/phase?phase=${phaseToSet}`); const result = await response.json(); if (result.success) { const phaseNames = { seedling: 'Seedling', veg: 'Veg', flower: 'Flowering', none: 'All cancelled' }; showMessage(`${phaseNames[phaseToSet] || phaseToSet}`); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error setting phase', 'error'); } }
        async function resetPhase() { const phase = document.getElementById('resetPhaseSelect').value; try { const response = await fetch(`/api/phasereset?phase=${phase}`); const result = await response.json(); if (result.success) { showMessage(`${phase === 'all' ? 'All' : phase} reset`); fetchStatus(); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error resetting', 'error'); } }
        let logEntries = [];
        async function fetchLogbook() { try { const response = await fetch('/api/logbook'); const data = await response.json(); logEntries = data.entries || []; renderLogEntries(); } catch (error) { console.error('Error fetching logbook:', error); } }
        function renderLogEntries() { const container = document.getElementById('logEntries'); if (logEntries.length === 0) { container.innerHTML = '<div class="log-empty">No entries yet</div>'; return; } let html = ''; for (const entry of logEntries) { html += `<div class="log-entry"><div class="log-entry-header"><span class="log-entry-time">${entry.time}</span><button class="log-entry-delete" onclick="deleteLogEntry(${entry.index})" title="Delete">×</button></div><div class="log-entry-text">${escapeHtml(entry.text)}</div></div>`; } container.innerHTML = html; }
        function escapeHtml(text) { const div = document.createElement('div'); div.textContent = text; return div.innerHTML; }
        async function addLogEntry() { const input = document.getElementById('logInput'); const text = input.value.trim(); if (!text) { showMessage('Please enter text', 'error'); return; } try { const response = await fetch('/api/logbook/add', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: `text=${encodeURIComponent(text)}` }); const result = await response.json(); if (result.success) { input.value = ''; fetchLogbook(); showMessage('Entry added'); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error adding entry', 'error'); } }
        async function deleteLogEntry(index) { if (!confirm('Delete this entry?')) { return; } try { const response = await fetch('/api/logbook/delete', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: `index=${index}` }); const result = await response.json(); if (result.success) { fetchLogbook(); showMessage('Entry deleted'); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error deleting', 'error'); } }
        async function clearLogbook() { if (!confirm('Clear entire logbook? This cannot be undone!')) { return; } try { const response = await fetch('/api/logbook/clear', { method: 'POST' }); const result = await response.json(); if (result.success) { fetchLogbook(); showMessage('Logbook cleared'); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error clearing', 'error'); } }
        async function waterPlant() { const note = document.getElementById('waterNote').value.trim(); const text = note ? `Watered: ${note}` : 'Watered'; try { const response = await fetch('/api/logbook/add', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: `text=${encodeURIComponent(text)}` }); const result = await response.json(); if (result.success) { document.getElementById('waterNote').value = ''; fetchLogbook(); showMessage('Watered!'); } else { showMessage('Error: ' + result.error, 'error'); } } catch (error) { showMessage('Error watering', 'error'); } }
        function updatePhaseUI(status) {
            if (status.seedling !== undefined) { document.getElementById('seedlingDays').textContent = status.seedling.days || 0; currentPhaseStatus.seedling = { active: status.seedling.active }; var btnS = document.getElementById('btnSeedling'); if (status.seedling.active) { btnS.style.background = '#8b5cf6'; btnS.style.borderColor = '#a78bfa'; btnS.style.boxShadow = '0 0 15px rgba(139,92,246,0.6)'; } else { btnS.style.background = 'rgba(139,92,246,0.3)'; btnS.style.borderColor = 'rgba(139,92,246,0.5)'; btnS.style.boxShadow = 'none'; } }
            if (status.veg !== undefined) { document.getElementById('vegDays').textContent = status.veg.days || 0; currentPhaseStatus.veg = { active: status.veg.active }; var btnV = document.getElementById('btnVeg'); if (status.veg.active) { btnV.style.background = '#10b981'; btnV.style.borderColor = '#34d399'; btnV.style.boxShadow = '0 0 15px rgba(16,185,129,0.6)'; } else { btnV.style.background = 'rgba(16,185,129,0.3)'; btnV.style.borderColor = 'rgba(16,185,129,0.5)'; btnV.style.boxShadow = 'none'; } }
            if (status.flower !== undefined) { document.getElementById('flowerDays').textContent = status.flower.days || 0; currentPhaseStatus.flower = { active: status.flower.active }; var btnF = document.getElementById('btnFlower'); if (status.flower.active) { btnF.style.background = '#f59e0b'; btnF.style.borderColor = '#fbbf24'; btnF.style.boxShadow = '0 0 15px rgba(245,158,11,0.6)'; } else { btnF.style.background = 'rgba(245,158,11,0.3)'; btnF.style.borderColor = 'rgba(245,158,11,0.5)'; btnF.style.boxShadow = 'none'; } }
            if (status.totalDays !== undefined) { document.getElementById('totalDays').textContent = status.totalDays || 0; }
        }
        fetchStatus(); fetchLogbook(); setInterval(fetchStatus, 2000);
    </script>
</body>
</html>
)rawliteral";

#endif
