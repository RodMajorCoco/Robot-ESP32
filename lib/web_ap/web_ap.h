/************************************************************
 *  Fichier  : web_ap.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Interface web embarquée (HTML/CSS/JS) du portail de
 *    configuration en mode Access Point.
 *    Extraite de web_interface.h (v1.x).
 ************************************************************/

#ifndef WEB_AP_H
#define WEB_AP_H

#include <Arduino.h>

const char index_html_AP[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <meta charset="utf-8">
    <style>
        body { font-family: sans-serif; background: #121212; color: #fff; margin: 0; padding: 20px; box-sizing: border-box; }
        h2 { color: #00ffcc; margin-bottom: 20px; }
        input, select { width: 100%; padding: 10px; margin: 8px 0 16px; background: #2a2a2a; border: 1px solid #444; color: #fff; border-radius: 8px; font-size: 16px; box-sizing: border-box; }
        button { width: 100%; padding: 14px; background: #00ffcc; color: #000; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; margin-bottom: 10px; }
        button.secondary { background: #333; color: #00ffcc; border: 1px solid #00ffcc; }
        .net { padding: 10px; background: #2a2a2a; border-radius: 8px; margin: 6px 0; cursor: pointer; display: flex; justify-content: space-between; align-items: center; border: 1px solid #444; }
        .net:active { border-color: #00ffcc; }
        .rssi { font-size: 12px; color: #888; }
        #status { padding: 12px; border-radius: 8px; margin-top: 12px; display: none; }
        .ok  { background: #1a3a1a; color: #00ff88; border: 1px solid #00ff88; }
        .err { background: #3a1a1a; color: #ff4444; border: 1px solid #ff4444; }
    </style>
</head>
<body>
    <h2>&#x1F4F6; Configuration WiFi</h2>
    <div id="netList"><p style="color:#888">Scan en cours...</p></div>
    <br>
    <label>Réseau (SSID)</label>
    <input type="text"     id="ssid"    placeholder="Nom du réseau">
    <label>Mot de passe</label>
    <input type="password" id="pass"    placeholder="Mot de passe WiFi">
    <label>Utilisateur web</label>
    <input type="text"     id="webuser" placeholder="admin" value="admin">
    <label>Mot de passe web</label>
    <input type="password" id="webpass" placeholder="Mot de passe interface web">
    <button onclick="save()">Enregistrer et redémarrer</button>
    <button class="secondary" onclick="scanNets()">&#x21BB; Rescanner</button>
    <div id="status"></div>
    <script>
        function scanNets() {
            document.getElementById('netList').innerHTML = '<p style="color:#888">Scan en cours...</p>';
            fetch('/scan').then(r => r.json()).then(nets => {
                let h = '<p style="color:#aaa;font-size:13px">Touchez un réseau pour le sélectionner</p>';
                nets.forEach(n => {
                    const bars = n.rssi > -60 ? '▰▰▰' : n.rssi > -75 ? '▰▰▱' : '▰▱▱';
                    h += `<div class="net" onclick="document.getElementById('ssid').value='${n.ssid}'">
                        <span>${n.ssid}</span>
                        <span class="rssi">${bars} ${n.rssi}dBm${n.secure ? ' 🔒' : ''}</span>
                    </div>`;
                });
                document.getElementById('netList').innerHTML = h;
            }).catch(() => {
                document.getElementById('netList').innerHTML = '<p style="color:#ff4444">Scan échoué</p>';
            });
        }
        function save() {
            const s  = document.getElementById('ssid').value.trim();
            const p  = document.getElementById('pass').value;
            const u  = document.getElementById('webuser').value.trim();
            const wp = document.getElementById('webpass').value;
            const st = document.getElementById('status');
            if (!s)      { st.className='err'; st.style.display='block'; st.textContent='SSID requis'; return; }
            if (!u || !wp) { st.className='err'; st.style.display='block'; st.textContent='Identifiants web requis'; return; }
            fetch('/save', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `ssid=${encodeURIComponent(s)}&pass=${encodeURIComponent(p)}&webuser=${encodeURIComponent(u)}&webpass=${encodeURIComponent(wp)}`
            }).then(r => {
                st.style.display = 'block';
                if (r.ok) { st.className='ok';  st.textContent='Sauvegardé — redémarrage...'; }
                else      { st.className='err'; st.textContent='Erreur'; }
            });
        }
        scanNets();
    </script>
</body>
</html>)rawliteral";

#endif // WEB_AP_H
