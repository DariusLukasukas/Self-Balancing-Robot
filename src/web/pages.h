#pragma once
#include "Arduino.h"

const char ROOT_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Self-Balancing Robot - Dashboard</title>
  <style>
    :root { font-family: monospace; }
    header { text-transform: uppercase; }
    section { margin-bottom: 1.5rem; }
    .motors { display: flex; gap: 2rem; }
  </style>
</head>
<body>
  <header><h1>Self-Balancing Robot - Dashboard</h1></header>
  <main>

    <section>
      <header><h2>IMU</h2></header>
      <p>Pitch: <span id="pitch">0</span>°</p>
      <p>Roll:  <span id="roll">0</span>°</p>
      <p>Yaw:   <span id="yaw">0</span>°</p>
    </section>

    <section>
      <header><h2>Stepper Motors</h2></header>
      <div class="motors">
        <div>
          <h3>Left</h3>
          <p>Status:    <span id="left-status">IDLE</span></p>
          <p>Speed:     <span id="left-speed">0</span> steps/s</p>
          <p>Direction: <span id="left-dir">STOP</span></p>
        </div>
        <div>
          <h3>Right</h3>
          <p>Status:    <span id="right-status">IDLE</span></p>
          <p>Speed:     <span id="right-speed">0</span> steps/s</p>
          <p>Direction: <span id="right-dir">STOP</span></p>
        </div>
      </div>
    </section>

    <section>
      <header><h2>Controls</h2></header>

      <label>Left speed (steps/s)
        <input type="range" id="sl" min="-5000" max="5000" step="50" value="0"
               oninput="document.getElementById('sl-val').textContent = this.value">
        <span id="sl-val">0</span>
      </label>
      <br>
      <label>Right speed (steps/s)
        <input type="range" id="sr" min="-5000" max="5000" step="50" value="0"
               oninput="document.getElementById('sr-val').textContent = this.value">
        <span id="sr-val">0</span>
      </label>
      <br><br>
      <button onclick="sendRun()">Run</button>
      <button onclick="sendStop()">Stop</button>
    </section>

  </main>
  <script>
    // Poll /data every 200ms and update all spans
    async function poll() {
      try {
        const data = await fetch('/data').then(r => r.json());
        document.getElementById('pitch').textContent        = data.pitch;
        document.getElementById('roll').textContent         = data.roll;
        document.getElementById('yaw').textContent          = data.yaw;
        document.getElementById('left-speed').textContent   = data.leftSpeed;
        document.getElementById('right-speed').textContent  = data.rightSpeed;
        document.getElementById('left-dir').textContent     = data.leftDir;
        document.getElementById('right-dir').textContent    = data.rightDir;
        document.getElementById('left-status').textContent  = data.status;
        document.getElementById('right-status').textContent = data.status;
      } catch(e) {
        console.error('Poll failed:', e);
      }
    }

    async function sendRun() {
      const sl = document.getElementById('sl').value;
      const sr = document.getElementById('sr').value;
      await fetch(`/control?action=run&sl=${sl}&sr=${sr}`);
    }

    async function sendStop() {
      // Reset sliders visually
      document.getElementById('sl').value = 0;
      document.getElementById('sr').value = 0;
      document.getElementById('sl-val').textContent = 0;
      document.getElementById('sr-val').textContent = 0;
      await fetch('/control?action=stop');
    }

    setInterval(poll, 200);
  </script>
</body>
</html>
)rawliteral";