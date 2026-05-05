#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

// HTML de l'interface (simplifié pour le mobile)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" charset="utf-8">
<style>
    body, html { margin: 0; padding: 0; height: 100%; background: #121212; font-family: sans-serif; color: white; overflow: hidden; }
    
    /* Zone Caméra (Haut) - 50% de la hauteur */
    .camera-container { height: 50vh; background: #000; display: flex; align-items: center; justify-content: center; border-bottom: 2px solid #444; }
    .camera-placeholder { color: #00ffcc; font-size: 1.2rem; text-transform: uppercase; letter-spacing: 2px; }

    /* Zone Contrôles (Bas) - 50% de la hauteur */
    .controls { 
        height: 50vh; 
        padding: 10px; 
        box-sizing: border-box;
        display: flex;
        align-items: center;
        justify-content: center;
    }
    
    /* Grille qui prend tout l'espace */
    .layout { 
        display: grid; 
        width: 100%;
        height: 100%;
        grid-template-areas: 
            ". up ." 
            "left stop right" 
            ". down ."; 
        grid-template-columns: 1fr 1.2fr 1fr; /* Colonne centrale légèrement plus large */
        grid-template-rows: 1fr 1fr 1fr;
        gap: 15px; 
    }
    
    .btn { 
        background: #2a2a2a; border-radius: 20px; 
        display: flex; align-items: center; justify-content: center;
        font-size: 40px; cursor: pointer; user-select: none; 
        -webkit-tap-highlight-color: transparent;
        border: 1px solid #444;
        transition: background 0.1s;
    }
    
    /* Couleurs et intensité */
    .btn:active { background: #00ffcc; color: #000; box-shadow: 0 0 25px #00ffcc; border-color: #fff; }
    
    .btn-up { grid-area: up; } 
    .btn-left { grid-area: left; } 
    .btn-right { grid-area: right; } 
    .btn-down { grid-area: down; }
    
    .btn-stop { 
        grid-area: stop; 
        background: #ff3333; 
        font-size: 24px; 
        font-weight: bold; 
        border-color: #800000;
    }
    .btn-stop:active { background: #ff0000; box-shadow: 0 0 25px #ff0000; }

</style></head>
<body>
    <div class="camera-container">
        <div class="camera-placeholder">--- LIVE FEED ---</div>
    </div>
    <div class="controls">
        <div class="layout">
            <div class="btn btn-up" ontouchstart="move('forward')" ontouchend="move('stop')" onmousedown="move('forward')" onmouseup="move('stop')">▲</div>
            <div class="btn btn-left" ontouchstart="move('left')" ontouchend="move('stop')" onmousedown="move('left')" onmouseup="move('stop')">◄</div>
            <div class="btn btn-stop" onmousedown="move('stop')" ontouchstart="move('stop')">STOP</div>
            <div class="btn btn-right" ontouchstart="move('right')" ontouchend="move('stop')" onmousedown="move('right')" onmouseup="move('stop')">►</div>
            <div class="btn btn-down" ontouchstart="move('backward')" ontouchend="move('stop')" onmousedown="move('backward')" onmouseup="move('stop')">▼</div>
        </div>
    </div>
    <script>
        let controller;
            function move(dir) {
                if (controller) controller.abort(); // Annule la requête précédente si elle est encore en cours
                controller = new AbortController();
                fetch('/' + dir, { signal: controller.signal });
            }
            document.body.addEventListener('touchstart', function(e) { 
                if (e.target.classList.contains('btn')) e.preventDefault(); 
            }, { passive: false });
    </script>
</body></html>)rawliteral";

#endif