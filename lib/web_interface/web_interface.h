/************************************************************
 *  Fichier  : web_interface.h
 *  Projet   : Robot ESP32-S3
 *  Auteur   : 
 *  Date     : 2026-05-01
 *  Version  : 1.1
 * ----------------------------------------------------------
 *  Description :
 *    Interface web embarquée (HTML/CSS/JS) stockée en
 *    mémoire programme (PROGMEM). Fournit une télécommande
 *    tactile responsive avec boutons de direction, arrêt
 *    d'urgence, contrôle de l'écran OLED et du driver
 *    moteur. Compatible mobile et PC.
 * ----------------------------------------------------------
 *  Historique :
 *    1.0 - 2026-05-01 : Création
 *    1.1 - 2026-06-01 : Ajout de la lecture de la batterie et endpoint /battery
 ************************************************************/

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" charset="utf-8">
<style>
    body, html { 
        margin: 0; padding: 0; height: 100%; background: #121212; 
        font-family: sans-serif; color: white; overflow: hidden; 
        touch-action: none; /* Empêche le zoom/scroll accidentel sur mobile */
        user-select: none; -webkit-user-select: none;
    }
    
    .camera-container { height: 50vh; background: #000; display: flex; align-items: center; justify-content: center; border-bottom: 2px solid #444; }
    .camera-placeholder { color: #00ffcc; font-size1,: 1.2rem; text-transform: uppercase; letter-spacing: 2px; }

    .battery-overlay {
        position: absolute; top: 10px; right: 10px;
        display: flex; align-items: center; gap: 6px;
        background: rgba(0,0,0,0.55); border-radius: 8px;
        padding: 4px 10px; backdrop-filter: blur(4px);
        font-size: 13px; font-weight: bold; color: #fff;
        pointer-events: none; /* ne pas bloquer le futur flux vidéo */
        z-index: 10;
    }
    .battery-bar-bg {
        width: 36px; height: 14px; border: 2px solid #fff;
        border-radius: 3px; position: relative;
    }
    .battery-bar-bg::after { /* borne + */
        content: ''; position: absolute;
        right: -5px; top: 3px;
        width: 3px; height: 6px;
        background: #fff; border-radius: 0 2px 2px 0;
    }
    .battery-bar-fill {
        height: 100%; border-radius: 1px;
        transition: width 0.5s, background 0.5s;
    }
    .battery-text { min-width: 34px; text-align: right; }


    .controls { height: 50vh; padding: 10px; box-sizing: border-box; display: flex; align-items: center; justify-content: center; }
    
    .layout { 
        display: grid; width: 100%; height: 100%;
        grid-template-areas: "drv up scr" "left stop right" ". down ."; 
        grid-template-columns: 1fr 1.2fr 1fr; grid-template-rows: 1fr 1fr 1fr; gap: 15px; 
    }
    
    .btn { 
        background: #2a2a2a; border-radius: 20px; 
        display: flex; align-items: center; justify-content: center;
        font-size: 40px; cursor: pointer;
        border: 1px solid #444; transition: all 0.1s;
    }
    
    /* État actif forcé par JavaScript pour garantir le fonctionnement sur mobile */
    .btn.is-active { 
        background: #00ffcc !important; color: #000 !important; 
        box-shadow: 0 0 25px #00ffcc; border-color: #fff; 
    }
    
    .btn-up { grid-area: up; } 
    .btn-left { grid-area: left; } 
    .btn-right { grid-area: right; } 
    .btn-down { grid-area: down; }
    
    .btn-stop { grid-area: stop; background: #ff3333; font-size: 24px; font-weight: bold; border-color: #800000; }
    .btn-stop.is-active { background: #ff0000 !important; box-shadow: 0 0 25px #ff0000; }

    .btn-screen { 
        grid-area: scr; background: #333; font-size: 14px; color: #00ffcc;
        border: 1px dashed #00ffcc; border-radius: 20px;
        text-align: center; line-height: 1.1; flex-direction: column;
    }

    .btn-driver { 
        grid-area: drv; background: #333; font-size: 14px; color: #00ff88;
        border: 1px dashed #00ff88; border-radius: 20px;
        text-align: center; line-height: 1.1; flex-direction: column;
    }

    .btn-driver.driver-off {
        color: #ff4444;
        border-color: #ff4444;
    }

</style></head>
<body>
    <div class="camera-container" style="position:relative;">
        <div class="camera-placeholder">--- LIVE FEED ---</div>
        <div class="battery-overlay">
            <div class="battery-bar-bg">
                <div class="battery-bar-fill" id="batFill"></div>
            </div>
            <span class="battery-text" id="batText">---%</span>
        </div>
    </div>
    <div class="controls">
        <div class="layout">
            <div class="btn btn-up" data-dir="forward">▲</div>
            <div class="btn btn-screen" id="scrBtn">SCREEN<br>ON</div>
            <div class="btn btn-driver" id="drvBtn">DRIVER<br>ON</div>
            <div class="btn btn-left" data-dir="left">◄</div>
            <div class="btn btn-stop" data-dir="stop">STOP</div>
            <div class="btn btn-right" data-dir="right">►</div>
            <div class="btn btn-down" data-dir="backward">▼</div>
        </div>
    </div>
    <script>
        let screenState = true;
        let controller;

        // Gestion de l'écran
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

        // Gestion des mouvements (Unifiée Mobile/PC)
        function sendMove(dir) {
            if (controller) controller.abort();
            controller = new AbortController();
            fetch('/' + dir, { signal: controller.signal });
        }


        // Mise à jour de l'indicateur de batterie
        function updateBatteryUI(pct) {
            const fill = document.getElementById('batFill');
            const text = document.getElementById('batText');
            if (!fill || !text) return;
            
            fill.style.width = pct + '%';
            fill.style.background = pct > 50 ? '#00ff88' 
                                  : pct > 20 ? '#ffaa00' 
                                  :            '#ff4444';
            text.textContent = pct + '%';
        }

        // Polling toutes les 30s (synchronisé avec BATTERY_READ_INTERVAL)
        async function fetchBattery() {
            try {
                const r = await fetch('/battery');
                if (r.ok) updateBatteryUI(parseInt(await r.text()));
            } catch(e) {}
        }
        fetchBattery(); // lecture immédiate au chargement
        setInterval(fetchBattery, 30000);

        document.querySelectorAll('.btn').forEach(button => {
            if (button.id === 'scrBtn' || button.id === 'drvBtn') return;

            const start = (e) => {
                e.preventDefault();
                button.classList.add('is-active');
                sendMove(button.dataset.dir);
            };

            const end = (e) => {
                e.preventDefault();
                button.classList.remove('is-active');
                if (button.dataset.dir !== 'stop') sendMove('stop');
            };

            // Événements Tactiles
            button.addEventListener('touchstart', start);
            button.addEventListener('touchend', end);
            // Événements Souris (pour garder la compatibilité PC)
            button.addEventListener('mousedown', start);
            button.addEventListener('mouseup', end);
        });

        // Empêche le menu contextuel au clic droit / appui long
        window.oncontextmenu = function(event) {
            event.preventDefault();
            event.stopPropagation();
            return false;
        };
    </script>
</body></html>)rawliteral";

#endif