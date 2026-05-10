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
    .camera-placeholder { color: #00ffcc; font-size: 1.2rem; text-transform: uppercase; letter-spacing: 2px; }

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
    <div class="camera-container">
        <div class="camera-placeholder">--- LIVE FEED ---</div>
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