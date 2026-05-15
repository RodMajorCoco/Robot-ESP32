/************************************************************
 *  Fichier  : web_interface.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Interface web embarquée (HTML/CSS/JS) de la télécommande.
 *    Stockée en mémoire programme (PROGMEM).
 ************************************************************/

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" charset="utf-8">
    <style>
        body, html {
            margin: 0; padding: 0; height: 100%;
            background: #121212; font-family: sans-serif; color: white;
            overflow: hidden; touch-action: none;
            user-select: none; -webkit-user-select: none;
        }
        .camera-container {
            height: 50vh; background: #000;
            display: flex; align-items: center; justify-content: center;
            border-bottom: 2px solid #444;
        }
        .camera-placeholder { color: #00ffcc; font-size: 1.2rem; text-transform: uppercase; letter-spacing: 2px; }
        .battery-overlay {
            position: absolute; top: 10px; right: 10px;
            display: flex; align-items: center; gap: 6px;
            background: rgba(0,0,0,0.55); border-radius: 8px; padding: 4px 10px;
            backdrop-filter: blur(4px); font-size: 13px; font-weight: bold; color: #fff;
            pointer-events: none; z-index: 10;
        }
        .battery-bar-bg { width: 36px; height: 14px; border: 2px solid #fff; border-radius: 3px; position: relative; }
        .battery-bar-bg::after { content: ''; position: absolute; right: -5px; top: 3px; width: 3px; height: 6px; background: #fff; border-radius: 0 2px 2px 0; }
        .battery-bar-fill { height: 100%; border-radius: 1px; transition: width 0.5s, background 0.5s; }
        .battery-text { min-width: 34px; text-align: right; }
        .controls {
            height: 50vh; padding: 10px; box-sizing: border-box;
            display: flex; flex-direction: column; gap: 8px;
        }
        .layout {
            display: grid; width: 100%; flex: 1;
            grid-template-areas: "drv up scr" "left stop right" ". down .";
            grid-template-columns: 1fr 1.2fr 1fr; grid-template-rows: 1fr 1fr 1fr; gap: 12px;
        }
        .btn { background: #2a2a2a; border-radius: 20px; display: flex; align-items: center; justify-content: center; font-size: 40px; cursor: pointer; border: 1px solid #444; transition: all 0.1s; }
        .btn.is-active { background: #00ffcc !important; color: #000 !important; box-shadow: 0 0 25px #00ffcc; border-color: #fff; }
        .btn-up { grid-area: up; } .btn-left { grid-area: left; } .btn-right { grid-area: right; } .btn-down { grid-area: down; }
        .btn-stop { grid-area: stop; background: #ff3333; font-size: 24px; font-weight: bold; border-color: #800000; }
        .btn-stop.is-active { background: #ff0000 !important; box-shadow: 0 0 25px #ff0000; }
        .btn-screen { grid-area: scr; background: #333; font-size: 14px; color: #00ffcc; border: 1px dashed #00ffcc; border-radius: 20px; text-align: center; line-height: 1.1; flex-direction: column; }
        .btn-driver { grid-area: drv; background: #333; font-size: 14px; color: #00ff88; border: 1px dashed #00ff88; border-radius: 20px; text-align: center; line-height: 1.1; flex-direction: column; }
        .btn-driver.driver-off { color: #ff4444; border-color: #ff4444; }
        .sliders-zone {
            display: flex; gap: 12px; padding: 0 2px 2px;
        }
        .slider-group {
            flex: 1; display: flex; flex-direction: column; align-items: center; gap: 3px;
        }
        .slider-group .slabel {
            font-size: 10px; color: #888; text-transform: uppercase; letter-spacing: 1px;
        }
        .slider-group input[type=range] {
            width: 100%; accent-color: #00ffcc; cursor: pointer; margin: 0;
        }
        .slider-group .sval {
            font-size: 12px; color: #00ffcc; font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="camera-container" style="position:relative;">
        <div class="camera-placeholder">--- LIVE FEED ---</div>
        <div class="battery-overlay">
            <div class="battery-bar-bg"><div class="battery-bar-fill" id="batFill"></div></div>
            <span class="battery-text" id="batText">---%</span>
        </div>
    </div>
    <div class="controls">
        <div class="layout">
            <div class="btn btn-up"     data-dir="forward">▲</div>
            <div class="btn btn-screen" id="scrBtn">SCREEN<br>ON</div>
            <div class="btn btn-driver" id="drvBtn">DRIVER<br>ON</div>
            <div class="btn btn-left"   data-dir="left">◄</div>
            <div class="btn btn-stop"   data-dir="stop">STOP</div>
            <div class="btn btn-right"  data-dir="right">►</div>
            <div class="btn btn-down"   data-dir="backward">▼</div>
        </div>
        <div class="sliders-zone">
            <div class="slider-group">
                <span class="slabel">Croisière</span>
                <input type="range" min="135" max="255" step="5" value="200"
                       oninput="setSpdVal('vCru',this.value);sendSpeed('cruise',this.value)">
                <span class="sval" id="vCru">200</span>
            </div>
            <div class="slider-group">
                <span class="slabel">Rotation</span>
                <input type="range" min="135" max="255" step="5" value="180"
                       oninput="setSpdVal('vRot',this.value);sendSpeed('rotation',this.value)">
                <span class="sval" id="vRot">180</span>
            </div>
        </div>
    </div>
    <script>
        let screenState = true, controller;
        const scrBtn = document.getElementById('scrBtn');
        function toggleScreen(e) {
            if (e) e.preventDefault();
            screenState = !screenState;
            fetch(screenState ? '/display/on' : '/display/off');
            scrBtn.style.color = screenState ? "#00ffcc" : "#666";
            scrBtn.style.borderColor = screenState ? "#00ffcc" : "#666";
            scrBtn.innerHTML = screenState ? "SCREEN<br>ON" : "SCREEN<br>OFF";
        }
        scrBtn.addEventListener('touchstart', toggleScreen);
        scrBtn.addEventListener('click', toggleScreen);

        let driverState = true;
        const drvBtn = document.getElementById('drvBtn');
        function toggleDriver(e) {
            if (e) e.preventDefault();
            driverState = !driverState;
            fetch(driverState ? '/driver/on' : '/driver/off');
            drvBtn.classList.toggle('driver-off', !driverState);
            drvBtn.innerHTML = driverState ? "DRIVER<br>ON" : "DRIVER<br>OFF";
        }
        drvBtn.addEventListener('touchstart', toggleDriver);
        drvBtn.addEventListener('click', toggleDriver);

        function sendMove(dir) {
            if (controller) controller.abort();
            controller = new AbortController();
            fetch('/' + dir, { signal: controller.signal });
        }
        function updateBatteryUI(pct) {
            const fill = document.getElementById('batFill');
            const text = document.getElementById('batText');
            if (!fill || !text) return;
            fill.style.width = pct + '%';
            fill.style.background = pct > 50 ? '#00ff88' : pct > 20 ? '#ffaa00' : '#ff4444';
            text.textContent = pct + '%';
        }
        async function fetchBattery() {
            try { const r = await fetch('/battery'); if (r.ok) updateBatteryUI(parseInt(await r.text())); } catch(e) {}
        }
        fetchBattery();
        setInterval(fetchBattery, 30000);

        const speedTimers = {};
        function sendSpeed(type, value) {
            clearTimeout(speedTimers[type]);
            speedTimers[type] = setTimeout(() => fetch('/speed/' + type + '?value=' + value), 200);
        }
        function setSpdVal(id, v) { document.getElementById(id).textContent = v; }

        document.querySelectorAll('.btn').forEach(button => {
            if (button.id === 'scrBtn' || button.id === 'drvBtn') return;
            const start = (e) => { e.preventDefault(); button.classList.add('is-active'); sendMove(button.dataset.dir); };
            const end   = (e) => { e.preventDefault(); button.classList.remove('is-active'); if (button.dataset.dir !== 'stop') sendMove('stop'); };
            button.addEventListener('touchstart', start); button.addEventListener('touchend', end);
            button.addEventListener('mousedown',  start); button.addEventListener('mouseup',  end);
        });
        window.oncontextmenu = function(e) { e.preventDefault(); e.stopPropagation(); return false; };
    </script>
</body>
</html>)rawliteral";

#endif // WEB_INTERFACE_H
