#pragma once
#include "Arduino.h"

// NOTE: This is the page actually served by the ESP32 at "/".
// Keep it in sync with src/web/index.html.
const char ROOT_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Self-Balancing Robot - Dashboard</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/uplot@1.6.30/dist/uPlot.min.css">
    <style>
        :root {
            font-family: monospace;
            text-transform: uppercase;
            font-size: 14px;
            line-height: 1.45;
            color: #111;
        }

        *,
        *::before,
        *::after {
            box-sizing: border-box;
        }

        header {
            text-transform: uppercase;
            margin-bottom: 1rem;
        }

        body {
            margin: 0;
            padding: 1.25rem;
            background: #fff;
        }

        main {
            max-width: 1100px;
        }

        .subtle {
            opacity: 0.75;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 2rem;
        }

        .table-fixed {
            table-layout: fixed;
        }

        th,
        td {
            border: 1px solid #d0d0d0;
            padding: 0.6rem 0.75rem;
            text-align: center;
        }

        th {
            background-color: #f4f4f4;
            letter-spacing: 0.06em;
        }

        thead .table-title {
            background: #111;
            color: #fff;
            text-align: left;
            letter-spacing: 0.08em;
        }

        tbody tr:hover td {
            background: #fafafa;
        }

        .left {
            text-align: left;
        }

        .right {
            text-align: right;
        }

        .value-pill {
            display: inline-block;
            padding: 0.1rem 0.4rem;
            border: 1px solid #d0d0d0;
            background: #fff;
            border-radius: 2px;
            min-width: 10ch;
        }

        .value-pill.num {
            width: 18ch;
            text-align: right;
            font-variant-numeric: tabular-nums;
            font-feature-settings: "tnum" 1;
        }

        button {
            padding: 0.5rem 1rem;
            font-weight: 650;
            text-transform: uppercase;
            border: 1px solid #bdbdbd;
            background: #f8f8f8;
            cursor: pointer;
        }

        button:hover {
            background: #efefef;
        }

        button:active {
            background: #e7e7e7;
        }

        .btn-primary {
            background: #111;
            color: #fff;
            border-color: #111;
        }

        .btn-primary:hover {
            background: #000;
        }

        .btn-row {
            display: flex;
            gap: 0.5rem;
            justify-content: center;
            flex-wrap: wrap;
        }

        input[type="range"] {
            width: 100%;
        }

        .field {
            display: grid;
            grid-template-columns: 1fr auto;
            align-items: center;
            gap: 0.5rem;
        }

        input.num {
            width: 100%;
            height: 2.1rem;
            padding: 0 0.55rem;
            border: 1px solid #cfcfcf;
            background: linear-gradient(#fff, #fbfbfb);
            font: inherit;
            text-transform: none;
            text-align: right;
            border-radius: 2px;
            outline: none;
        }

        input.num:focus {
            border-color: #111;
            box-shadow: 0 0 0 2px rgba(0, 0, 0, 0.08);
        }

        .unit {
            opacity: 0.65;
            white-space: nowrap;
        }

        .status {
            text-align: left;
        }

        /* Telemetry charts */
        .telemetry-card {
            padding: 0.55rem !important;
        }

        .telemetry-chart {
            width: 100%;
            height: 200px;
            background: #fafafa;
            border: 1px solid #e5e5e5;
        }

        .telemetry-meta {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 0.4rem;
            font-size: 0.92em;
            flex-wrap: wrap;
            gap: 0.35rem;
        }

        .legend-swatch {
            display: inline-block;
            width: 18px;
            border-top: 2px solid currentColor;
            vertical-align: middle;
        }

        .legend-swatch.dashed {
            border-top-style: dashed;
        }

        .telemetry-actions {
            display: flex;
            gap: 0.5rem;
            flex-wrap: wrap;
            align-items: center;
        }

        /* Tuning metric cards (RMS, peak, saturation %, mean error). */
        .metric-row {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 0.5rem;
        }

        .metric-card {
            border: 1px solid #d0d0d0;
            background: #fafafa;
            padding: 0.55rem 0.7rem;
            display: flex;
            flex-direction: column;
            gap: 0.2rem;
            min-width: 0;
        }

        .metric-card .metric-label {
            opacity: 0.65;
            font-size: 0.85em;
            letter-spacing: 0.06em;
        }

        .metric-card .metric-value {
            font-size: 1.15em;
            font-weight: 650;
            font-variant-numeric: tabular-nums;
        }

        .metric-card.warn {
            background: #fff5e6;
            border-color: #e0a800;
        }

        .metric-card.error {
            background: #ffeaea;
            border-color: #c0392b;
        }

        /* uPlot tweaks: match the rest of the UI font and remove its native legend
           since we render our own legend in .telemetry-meta. */
        .uplot,
        .uplot * {
            font-family: monospace;
        }

        .uplot .u-legend {
            display: none;
        }

        /* Sticky emergency stop button */
        .estop-fab {
            position: fixed;
            right: 1rem;
            bottom: 1rem;
            z-index: 1000;
            background: #c0392b;
            color: #fff;
            border: 2px solid #962d22;
            padding: 0.85rem 1.2rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.08em;
            box-shadow: 0 4px 14px rgba(0, 0, 0, 0.25);
            border-radius: 4px;
            cursor: pointer;
            font-family: monospace;
            font-size: 0.95em;
        }

        .estop-fab:hover {
            background: #962d22;
        }

        .estop-fab:active {
            background: #7a2418;
        }

        .estop-fab.flash {
            animation: estop-flash 0.4s ease-out;
        }

        @keyframes estop-flash {
            0% { background: #fff; color: #c0392b; }
            100% { background: #c0392b; color: #fff; }
        }

        /* System status strip */
        .sysbar {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 0.5rem;
            margin-bottom: 1.25rem;
            font-size: 0.92em;
        }

        .sysbar-cell {
            border: 1px solid #d0d0d0;
            background: #fafafa;
            padding: 0.45rem 0.65rem;
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 0.5rem;
        }

        .sysbar-cell .sysbar-label {
            opacity: 0.65;
            letter-spacing: 0.06em;
        }

        .sysbar-cell .sysbar-value {
            font-variant-numeric: tabular-nums;
        }

        .sysbar-cell.warn {
            background: #fff5e6;
            border-color: #e0a800;
        }

        .sysbar-cell.error {
            background: #ffeaea;
            border-color: #c0392b;
        }

        /* Mobile: stack table cells into labeled rows (more rows, not wider tables) */
        .mobile-only {
            display: none;
        }

        @media (max-width: 720px) {

            html,
            body {
                overflow-x: hidden;
            }

            body {
                padding: 0.75rem;
            }

            :root {
                font-size: 13px;
            }

            th,
            td {
                padding: 0.5rem 0.55rem;
            }

            .btn-row {
                justify-content: stretch;
            }

            button {
                width: 100%;
            }

            .mobile-only {
                display: block;
            }

            /* Only stack tables that opt-in */
            table.stack-mobile {
                width: 100%;
                table-layout: auto;
            }

            table.stack-mobile colgroup {
                display: none;
            }

            table.stack-mobile thead tr:not(:first-child) {
                display: none;
            }

            table.stack-mobile tbody {
                display: block;
                width: 100%;
            }

            table.stack-mobile tbody tr,
            table.stack-mobile tbody td {
                display: block;
                width: 100%;
            }

            table.stack-mobile tbody tr {
                border: 1px solid #d0d0d0;
                margin-bottom: 0.75rem;
                background: #fff;
                overflow: visible;
            }

            table.stack-mobile tbody td {
                border: none;
                border-top: 1px solid #ededed;
                text-align: left;
                overflow: visible;
                min-width: 0;
            }

            table.stack-mobile tbody td:first-child {
                border-top: none;
            }

            table.stack-mobile tbody td[data-label]::before {
                content: attr(data-label);
                display: block;
                opacity: 0.75;
                letter-spacing: 0.06em;
                margin-bottom: 0.25rem;
            }

            /* make inputs never overflow the cell */
            .field,
            input.num,
            input[type="range"] {
                max-width: 100%;
                min-width: 0;
            }

            table.stack-mobile input[type="range"] {
                display: block;
                width: 100%;
                margin: 0;
                padding: 0;
            }

            /* Mobile: avoid shadows expanding outside card */
            input.num:focus {
                box-shadow: none;
                outline: 2px solid rgba(0, 0, 0, 0.18);
                outline-offset: 1px;
            }

            button:focus {
                outline: 2px solid rgba(0, 0, 0, 0.18);
                outline-offset: 1px;
            }

            /* hide desktop-only action/status cells that rely on rowspan */
            table.stack-mobile tbody td.desktop-only {
                display: none;
            }

            /* Optional: 2-column stacked layout (opt-in per table) */
            table.stack-mobile.stack-2col tbody tr {
                display: grid;
                grid-template-columns: 1fr 1fr;
                gap: 0.5rem;
                padding: 0.5rem;
                border-radius: 4px;
            }

            table.stack-mobile.stack-2col tbody td {
                width: auto;
                border: 1px solid #ededed;
                border-radius: 2px;
                padding: 0.5rem 0.55rem;
            }

            table.stack-mobile.stack-2col tbody td:first-child {
                border-top: 1px solid #ededed;
            }

            /* Make 2-col cards read cleaner (labels + values) */
            table.stack-mobile.stack-2col tbody td[data-label]::before {
                margin-bottom: 0.15rem;
                font-size: 0.82em;
                letter-spacing: 0.08em;
            }

            /* Status table: pills should fit the card width */
            #motors-status-table.stack-mobile.stack-2col .value-pill {
                width: 100%;
                min-width: 0;
                text-align: center;
            }

            .telemetry-chart {
                height: 160px;
            }

            .telemetry-actions button {
                width: auto;
                flex: 1 1 auto;
            }

            .sysbar {
                grid-template-columns: 1fr 1fr;
            }

            .metric-row {
                grid-template-columns: 1fr 1fr;
            }

            .estop-fab {
                left: 0.75rem;
                right: 0.75rem;
                bottom: 0.75rem;
                width: auto;
                padding: 0.95rem 1rem;
                text-align: center;
            }
        }
    </style>
</head>

<body>
    <button class="estop-fab" type="button" id="estop-fab" onclick="emergencyStop()" title="Cut motor output and switch to MANUAL">
        ■ EMERGENCY STOP
    </button>

    <header>
        <h1>Self-Balancing Robot</h1>
    </header>
    <main>

        <div class="sysbar" id="sysbar">
            <div class="sysbar-cell" id="sys-cell-imu">
                <span class="sysbar-label">IMU</span>
                <span class="sysbar-value"><span id="sys-imuhz">—</span> Hz</span>
            </div>
            <div class="sysbar-cell" id="sys-cell-rssi">
                <span class="sysbar-label">WiFi</span>
                <span class="sysbar-value"><span id="sys-rssi">—</span> dBm</span>
            </div>
            <div class="sysbar-cell" id="sys-cell-uptime">
                <span class="sysbar-label">Uptime</span>
                <span class="sysbar-value" id="sys-uptime">—</span>
            </div>
        </div>

        <table id="telemetry-table">
            <thead>
                <tr>
                    <th class="table-title">TELEMETRY (LIVE)</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td class="telemetry-card">
                        <div class="telemetry-meta">
                            <span class="subtle">Angle (°)</span>
                            <span class="subtle">
                                <span class="legend-swatch" style="color:#c0392b"></span> measured
                                &nbsp;
                                <span class="legend-swatch dashed" style="color:#2c3e50"></span> target
                            </span>
                        </div>
                        <div id="chart-angle" class="telemetry-chart"></div>
                    </td>
                </tr>
                <tr>
                    <td class="telemetry-card">
                        <div class="telemetry-meta">
                            <span class="subtle">Motor speed (steps/s)</span>
                            <span class="subtle">
                                <span class="legend-swatch" style="color:#2980b9"></span> left
                                &nbsp;
                                <span class="legend-swatch" style="color:#27ae60"></span> right
                            </span>
                        </div>
                        <div id="chart-motor" class="telemetry-chart"></div>
                    </td>
                </tr>
                <tr>
                    <td class="telemetry-card">
                        <div class="telemetry-actions">
                            <button type="button" onclick="telemetryReset()">Clear</button>
                            <button type="button" id="tel-toggle" onclick="telemetryToggle()">Pause</button>
                            <button type="button" onclick="telemetryExport()">Export CSV</button>
                            <span class="subtle" style="margin-left:auto">Window 30s &middot; ~5Hz</span>
                        </div>
                    </td>
                </tr>
            </tbody>
        </table>

       <table>
            <thead>
                <tr>
                    <th class="table-title" colspan="3">IMU (ANGLES)</th>
                </tr>
                <tr>
                    <th>Pitch</th>
                    <th>Roll</th>
                    <th>Yaw</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td><span class="value-pill"><span id="pitch">0</span>°</span></td>
                    <td><span class="value-pill"><span id="roll">0</span>°</span></td>
                    <td><span class="value-pill"><span id="yaw">0</span>°</span></td>
                </tr>
            </tbody>
        </table>

        <table class="table-fixed stack-mobile stack-2col" id="motors-status-table">
            <thead>
                <tr>
                    <th class="table-title" colspan="4">STEPPER MOTORS (STATUS)</th>
                </tr>
                <tr>
                    <th>Side</th>
                    <th>Status</th>
                    <th>Speed</th>
                    <th>Dir</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td data-label="Side">Left</td>
                    <td data-label="Status"><span class="value-pill" id="left-status">IDLE</span></td>
                    <td data-label="Speed"><span class="value-pill"><span id="left-speed">0</span> steps/s</span></td>
                    <td data-label="Dir"><span class="value-pill" id="left-dir">STOP</span></td>
                </tr>
                <tr>
                    <td data-label="Side">Right</td>
                    <td data-label="Status"><span class="value-pill" id="right-status">IDLE</span></td>
                    <td data-label="Speed"><span class="value-pill"><span id="right-speed">0</span> steps/s</span></td>
                    <td data-label="Dir"><span class="value-pill" id="right-dir">STOP</span></td>
                </tr>
            </tbody>
        </table>

        <table class="table-fixed stack-mobile" id="controls-table">
            <thead>
                <tr>
                    <th class="table-title" colspan="5">MOTOR CONTROLS (MANUAL)</th>
                </tr>
                <tr>
                    <th class="left">Channel</th>
                    <th>Setpoint</th>
                    <th>Value</th>
                    <th colspan="2">Actions</th>
                </tr>
            </thead>
            <tbody>
                <colgroup>
                    <col style="width: 18ch;">
                    <col>
                    <col style="width: 20ch;">
                    <col style="width: 26ch;">
                    <col style="width: 0;">
                </colgroup>
                <tr>
                    <td data-label="Channel" class="left">Left speed</td>
                    <td data-label="Setpoint"><input type="range" id="sl" min="-5000" max="5000" step="100" value="0"
                            oninput="document.getElementById('sl-val').textContent = this.value"></td>
                    <td data-label="Value" class="right"><span class="value-pill num"><span id="sl-val">0</span> steps/s</span></td>
                    <td class="desktop-only" colspan="2" rowspan="2" style="vertical-align: middle;">
                        <div class="btn-row">
                            <button class="btn-primary" onclick="sendRun()">Run</button>
                            <button onclick="sendStop()">Stop</button>
                        </div>
                        <div class="status-line" style="margin-top: 1rem;">Status: <span id="motor-status">idle</span></div>
                    </td>
                </tr>
                <tr>
                    <td data-label="Channel" class="left">Right speed</td>
                    <td data-label="Setpoint"><input type="range" id="sr" min="-5000" max="5000" step="100" value="0"
                            oninput="document.getElementById('sr-val').textContent = this.value"></td>
                    <td data-label="Value" class="right"><span class="value-pill num"><span id="sr-val">0</span> steps/s</span></td>
                </tr>
                <tr class="mobile-only">
                    <td data-label="Actions" colspan="5">
                        <div class="btn-row">
                            <button class="btn-primary" onclick="sendRun()">Run</button>
                            <button onclick="sendStop()">Stop</button>
                        </div>
                    </td>
                </tr>
                <tr class="mobile-only">
                    <td data-label="Status" colspan="5" class="left subtle">
                        Status: <span id="motor-status">idle</span>
                    </td>
                </tr>
            </tbody>
        </table>

        <table class="stack-mobile" id="mode-table">
            <thead>
                <tr>
                    <th class="table-title" colspan="3">CONTROL MODE</th>
                </tr>
                <tr>
                    <th>CURRENT</th>
                    <th>ACTIONS</th>
                    <th>NOTE</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td data-label="Current"><span class="value-pill" id="mode">—</span></td>
                    <td data-label="Actions">
                        <div class="btn-row">
                            <button type="button" class="btn-primary" onclick="setModeBalance()">Balance</button>
                            <button type="button" onclick="setModeManual()">Manual</button>
                        </div>
                        <div class="status-line" style="margin-top: 0.75rem;">Status: <span id="mode-status">idle</span></div>
                    </td>
                    <td data-label="Note" class="left subtle">
                        Manual uses sliders. Balance runs the PID loop.
                    </td>
                </tr>
            </tbody>
        </table>

        <table id="pid-diag-table">
            <thead>
                <tr>
                    <th class="table-title">PID DIAGNOSTICS</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td class="telemetry-card">
                        <div class="telemetry-meta">
                            <span class="subtle">PID contributions (steps/s)</span>
                            <span class="subtle">
                                <span class="legend-swatch" style="color:#16a085"></span> P
                                &nbsp;
                                <span class="legend-swatch" style="color:#d35400"></span> I
                                &nbsp;
                                <span class="legend-swatch" style="color:#8e44ad"></span> D
                            </span>
                        </div>
                        <div id="chart-pid" class="telemetry-chart"></div>
                    </td>
                </tr>
                <tr>
                    <td class="telemetry-card">
                        <div class="metric-row">
                            <div class="metric-card" id="metric-card-rms">
                                <span class="metric-label">RMS error</span>
                                <span class="metric-value"><span id="metric-rms">—</span>°</span>
                            </div>
                            <div class="metric-card" id="metric-card-peak">
                                <span class="metric-label">Peak |error|</span>
                                <span class="metric-value"><span id="metric-peak">—</span>°</span>
                            </div>
                            <div class="metric-card" id="metric-card-sat">
                                <span class="metric-label">Saturation</span>
                                <span class="metric-value"><span id="metric-sat">—</span>%</span>
                            </div>
                            <div class="metric-card" id="metric-card-mean">
                                <span class="metric-label">Mean error</span>
                                <span class="metric-value"><span id="metric-mean">—</span>°</span>
                            </div>
                        </div>
                    </td>
                </tr>
                <tr>
                    <td class="telemetry-card">
                        <div class="telemetry-actions">
                            <button type="button" onclick="stepResponseTest(2)">Step +2°</button>
                            <button type="button" onclick="stepResponseTest(-2)">Step −2°</button>
                            <span class="subtle" id="step-status" style="margin-left:auto">idle</span>
                        </div>
                    </td>
                </tr>
            </tbody>
        </table>

        <table class="stack-mobile" id="pid-table">
            <thead>
                <tr>
                    <th class="table-title" colspan="4">PID / BALANCE TUNING</th>
                </tr>
                <tr>
                    <th>PARAM</th>
                    <th>CURRENT</th>
                    <th>SET</th>
                    <th>ACTIONS</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td data-label="Param">KP</td>
                    <td data-label="Current"><span class="value-pill" id="kp">—</span></td>
                    <td data-label="Set" class="left">
                        <div class="field">
                            <input id="pid-kp-in" class="num" type="number" step="0.1" min="0" max="500" inputmode="decimal">
                        </div>
                    </td>
                    <td class="desktop-only" rowspan="4" style="vertical-align: middle;">
                        <div class="btn-row">
                            <button type="button" onclick="pidLoad()">Load</button>
                            <button type="button" class="btn-primary" onclick="pidApply()">Apply</button>
                            <button type="button" onclick="pidReset()">Reset I</button>
                        </div>
                        <div class="status-line" style="margin-top: 1rem;">Status: <span id="pid-status">idle</span></div>
                    </td>
                </tr>
                <tr>
                    <td data-label="Param">KI</td>
                    <td data-label="Current"><span class="value-pill" id="ki">—</span></td>
                    <td data-label="Set" class="left">
                        <div class="field">
                            <input id="pid-ki-in" class="num" type="number" step="0.001" min="0" max="100" inputmode="decimal">
                        </div>
                    </td>
                </tr>
                <tr>
                    <td data-label="Param">KD</td>
                    <td data-label="Current"><span class="value-pill" id="kd">—</span></td>
                    <td data-label="Set" class="left">
                        <div class="field">
                            <input id="pid-kd-in" class="num" type="number" step="0.01" min="0" max="200" inputmode="decimal">
                        </div>
                    </td>
                </tr>
                <tr>
                    <td data-label="Param">Target °</td>
                    <td data-label="Current"><span class="value-pill" id="pid-target">—</span></td>
                    <td data-label="Set" class="left">
                        <div class="field">
                            <input id="pid-target-in" class="num" type="number" step="0.1" min="-30" max="30" inputmode="decimal">
                        </div>
                    </td>
                </tr>
                <tr class="mobile-only">
                    <td data-label="Actions" colspan="4">
                        <div class="btn-row">
                            <button type="button" onclick="pidLoad()">Load</button>
                            <button type="button" class="btn-primary" onclick="pidApply()">Apply</button>
                            <button type="button" onclick="pidReset()">Reset I</button>
                        </div>
                    </td>
                </tr>
                <tr class="mobile-only">
                    <td data-label="Status" colspan="4" class="left subtle">
                        Status: <span id="pid-status">idle</span>
                    </td>
                </tr>
            </tbody>
        </table>

    </main>
    <script src="https://cdn.jsdelivr.net/npm/uplot@1.6.30/dist/uPlot.iife.min.js"></script>
    <script>
        const $ = (id) => document.getElementById(id);

        function setText(id, v) {
            const el = $(id);
            if (!el) return;
            el.textContent = v;
        }

        function setMotorStatus(msg) {
            setText('motor-status', msg);
        }

        function setPidStatus(msg) {
            setText('pid-status', msg);
        }

        function setModeStatus(msg) {
            setText('mode-status', msg);
        }

        // Rolling 30-second telemetry powered by uPlot (loaded from CDN).
        const telemetry = (function () {
            const WINDOW_SEC = 30;
            // uPlot column-oriented data: [t, ...series].
            let dataAngle = [[], [], []]; // t, measured, target
            let dataMotor = [[], [], []]; // t, left, right
            let dataPid   = [[], [], [], []]; // t, P, I, D
            let chartAngle = null;
            let chartMotor = null;
            let chartPid = null;
            let paused = false;
            let startT = performance.now() / 1000;
            let resizeObserver = null;
            // Latest robot-reported saturation cap (steps/s); updated from /data.
            let maxSps = 5000;

            function commonOpts(seriesDef) {
                return {
                    width: 600,
                    height: 200,
                    cursor: { drag: { setScale: false } },
                    legend: { show: false },
                    scales: {
                        x: { time: false, auto: true },
                        y: { auto: true },
                    },
                    axes: [
                        {
                            stroke: '#666',
                            grid: { stroke: '#ececec', width: 1 },
                            ticks: { stroke: '#ddd' },
                            font: '10px monospace',
                            size: 28,
                            values: (u, vals) => vals.map(v => v.toFixed(0) + 's'),
                        },
                        {
                            stroke: '#666',
                            grid: { stroke: '#ececec', width: 1 },
                            ticks: { stroke: '#ddd' },
                            font: '10px monospace',
                            size: 50,
                        },
                    ],
                    series: [{}, ...seriesDef],
                };
            }

            function fitWidth(chart, container) {
                if (!chart || !container) return;
                const w = Math.max(60, Math.floor(container.clientWidth));
                const h = chart.bbox && chart.bbox.height
                    ? chart.bbox.height / (window.devicePixelRatio || 1)
                    : container.clientHeight || 200;
                chart.setSize({ width: w, height: Math.round(h) });
            }

            function init() {
                if (typeof uPlot === 'undefined') {
                    console.warn('uPlot not loaded; charts disabled');
                    return false;
                }
                const elA = $('chart-angle');
                const elM = $('chart-motor');
                const elP = $('chart-pid');
                if (!elA || !elM) return false;

                const optsA = commonOpts([
                    { label: 'measured', stroke: '#c0392b', width: 1.6 },
                    { label: 'target',   stroke: '#2c3e50', width: 1.4, dash: [5, 4] },
                ]);
                optsA.width = elA.clientWidth || 600;
                optsA.height = elA.clientHeight || 200;
                chartAngle = new uPlot(optsA, dataAngle, elA);

                const optsM = commonOpts([
                    { label: 'left',  stroke: '#2980b9', width: 1.4 },
                    { label: 'right', stroke: '#27ae60', width: 1.4 },
                ]);
                optsM.width = elM.clientWidth || 600;
                optsM.height = elM.clientHeight || 200;
                chartMotor = new uPlot(optsM, dataMotor, elM);

                if (elP) {
                    const optsP = commonOpts([
                        { label: 'P', stroke: '#16a085', width: 1.5 },
                        { label: 'I', stroke: '#d35400', width: 1.5 },
                        { label: 'D', stroke: '#8e44ad', width: 1.5 },
                    ]);
                    optsP.width = elP.clientWidth || 600;
                    optsP.height = elP.clientHeight || 200;
                    chartPid = new uPlot(optsP, dataPid, elP);
                }

                resizeObserver = new ResizeObserver(() => {
                    fitWidth(chartAngle, elA);
                    fitWidth(chartMotor, elM);
                    if (chartPid && elP) fitWidth(chartPid, elP);
                });
                resizeObserver.observe(elA);
                resizeObserver.observe(elM);
                if (elP) resizeObserver.observe(elP);
                return true;
            }

            function trimSeries(arr, cutoff) {
                let i = 0;
                while (i < arr[0].length && arr[0][i] < cutoff) i++;
                if (i > 0) for (let s = 0; s < arr.length; s++) arr[s] = arr[s].slice(i);
                return arr;
            }

            function trim(t) {
                const cutoff = t - WINDOW_SEC;
                dataAngle = trimSeries(dataAngle, cutoff);
                dataMotor = trimSeries(dataMotor, cutoff);
                dataPid   = trimSeries(dataPid,   cutoff);
            }

            function pushNum(arr, idx, v) {
                arr[idx].push(Number.isFinite(v) ? v : null);
            }

            function push(d) {
                if (paused) return;
                if (!chartAngle || !chartMotor) return;
                if (Number.isFinite(parseFloat(d.maxSps))) maxSps = parseFloat(d.maxSps);
                const t = (performance.now() / 1000) - startT;

                dataAngle[0].push(t);
                pushNum(dataAngle, 1, parseFloat(d.roll));
                pushNum(dataAngle, 2, parseFloat(d.target));

                dataMotor[0].push(t);
                pushNum(dataMotor, 1, parseFloat(d.leftSpeed));
                pushNum(dataMotor, 2, parseFloat(d.rightSpeed));

                dataPid[0].push(t);
                pushNum(dataPid, 1, parseFloat(d.pidP));
                pushNum(dataPid, 2, parseFloat(d.pidI));
                pushNum(dataPid, 3, parseFloat(d.pidD));

                trim(t);

                chartAngle.setData(dataAngle);
                chartMotor.setData(dataMotor);
                if (chartPid) chartPid.setData(dataPid);

                updateMetrics();
            }

            function reset() {
                startT = performance.now() / 1000;
                dataAngle = [[], [], []];
                dataMotor = [[], [], []];
                dataPid   = [[], [], [], []];
                if (chartAngle) chartAngle.setData(dataAngle);
                if (chartMotor) chartMotor.setData(dataMotor);
                if (chartPid)   chartPid.setData(dataPid);
                clearMetrics();
            }

            function getMaxSps() { return maxSps; }
            function getAngleBuffer() { return dataAngle; }
            function getMotorBuffer() { return dataMotor; }

            function toggle() {
                paused = !paused;
                const btn = $('tel-toggle');
                if (btn) btn.textContent = paused ? 'Resume' : 'Pause';
            }

            function exportCsv() {
                if (dataAngle[0].length === 0) {
                    alert('No telemetry data to export yet.');
                    return;
                }
                const header = ['t_sec', 'angle_meas_deg', 'angle_target_deg', 'motor_left_sps', 'motor_right_sps'];
                const lines = [header.join(',')];
                for (let i = 0; i < dataAngle[0].length; i++) {
                    const row = [
                        dataAngle[0][i].toFixed(3),
                        dataAngle[1][i] != null ? dataAngle[1][i].toFixed(3) : '',
                        dataAngle[2][i] != null ? dataAngle[2][i].toFixed(3) : '',
                        dataMotor[1][i] != null ? dataMotor[1][i].toFixed(2) : '',
                        dataMotor[2][i] != null ? dataMotor[2][i].toFixed(2) : '',
                    ];
                    lines.push(row.join(','));
                }
                const csv = lines.join('\n');
                const blob = new Blob([csv], { type: 'text/csv;charset=utf-8' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                const stamp = new Date().toISOString().replace(/[:.]/g, '-');
                a.href = url;
                a.download = `telemetry-${stamp}.csv`;
                document.body.appendChild(a);
                a.click();
                a.remove();
                setTimeout(() => URL.revokeObjectURL(url), 0);
            }

            return { init, push, reset, toggle, exportCsv, getMaxSps, getAngleBuffer, getMotorBuffer };
        })();

        function telemetryReset() { telemetry.reset(); }
        function telemetryToggle() { telemetry.toggle(); }
        function telemetryExport() { telemetry.exportCsv(); }

        // ─── Tuning metrics computed from the rolling buffer ───
        function clearMetrics() {
            setText('metric-rms',  '—');
            setText('metric-peak', '—');
            setText('metric-sat',  '—');
            setText('metric-mean', '—');
            ['metric-card-rms', 'metric-card-peak', 'metric-card-sat', 'metric-card-mean']
                .forEach(id => { const el = $(id); if (el) el.classList.remove('warn', 'error'); });
        }

        function setMetricSeverity(id, v, warnAt, errorAt) {
            const el = $(id);
            if (!el) return;
            el.classList.remove('warn', 'error');
            if (!Number.isFinite(v)) return;
            if (v >= errorAt) el.classList.add('error');
            else if (v >= warnAt) el.classList.add('warn');
        }

        function updateMetrics() {
            const angle = telemetry.getAngleBuffer();
            const motor = telemetry.getMotorBuffer();
            const cap = telemetry.getMaxSps();
            // Threshold for "saturated": within 2% of the firmware-reported cap.
            const satThreshold = cap * 0.98;

            // Error stats from angle buffer (measured - target).
            let n = 0, sumSq = 0, sum = 0, peak = 0;
            for (let i = 0; i < angle[0].length; i++) {
                const m = angle[1][i];
                const t = angle[2][i];
                if (m == null || t == null) continue;
                const e = m - t;
                sum += e;
                sumSq += e * e;
                const ae = Math.abs(e);
                if (ae > peak) peak = ae;
                n++;
            }

            // Saturation rate from motor buffer (post-clamp speeds).
            let sN = 0, sat = 0;
            for (let i = 0; i < motor[0].length; i++) {
                const l = motor[1][i];
                const r = motor[2][i];
                if (l != null) { sN++; if (Math.abs(l) >= satThreshold) sat++; }
                if (r != null) { sN++; if (Math.abs(r) >= satThreshold) sat++; }
            }

            if (n < 2) { clearMetrics(); return; }
            const rms = Math.sqrt(sumSq / n);
            const mean = sum / n;
            const satPct = sN > 0 ? (100 * sat / sN) : 0;

            setText('metric-rms',  rms.toFixed(2));
            setText('metric-peak', peak.toFixed(2));
            setText('metric-sat',  satPct.toFixed(1));
            setText('metric-mean', mean.toFixed(2));

            // Color hints. Thresholds chosen for a typical balance robot;
            // tweak if your tuning targets are different.
            setMetricSeverity('metric-card-rms',  rms,    1.0, 2.5);
            setMetricSeverity('metric-card-peak', peak,   3.0, 7.0);
            setMetricSeverity('metric-card-sat',  satPct, 5.0, 25.0);
            setMetricSeverity('metric-card-mean', Math.abs(mean), 0.5, 1.5);
        }

        // ─── Step-response test: temporarily nudge target by `delta` for ~1s ───
        let _stepInFlight = false;
        async function stepResponseTest(delta) {
            if (_stepInFlight) return;
            _stepInFlight = true;
            const set = (msg) => setText('step-status', msg);
            try {
                set('reading target...');
                const cur = await fetch('/pid', { cache: 'no-store' }).then(r => {
                    if (!r.ok) throw new Error('GET /pid failed');
                    return r.json();
                });
                if (cur.target == null) throw new Error('no target field');
                const orig = parseFloat(cur.target);

                const post = (target) => fetch('/pid', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ kp: cur.kp, ki: cur.ki, kd: cur.kd, target }),
                });

                set(`step ${delta > 0 ? '+' : ''}${delta}°`);
                const r1 = await post(orig + delta);
                if (!r1.ok) throw new Error('step apply failed');
                await new Promise((res) => setTimeout(res, 1000));

                set('restoring target');
                const r2 = await post(orig);
                if (!r2.ok) throw new Error('restore failed');
                set('done');
                setTimeout(() => set('idle'), 1500);
            } catch (e) {
                console.error(e);
                set('error');
            } finally {
                _stepInFlight = false;
            }
        }

        async function emergencyStop() {
            const btn = $('estop-fab');
            if (btn) {
                btn.classList.remove('flash');
                void btn.offsetWidth; // restart animation
                btn.classList.add('flash');
            }
            try {
                await fetch('/control?action=stop');
                setMotorStatus('stopped');
                setModeStatus('stopped');
            } catch (e) {
                console.error(e);
                setMotorStatus('error');
            }
        }

        function formatUptime(s) {
            const sec = Math.floor(s % 60);
            const min = Math.floor((s / 60) % 60);
            const hr = Math.floor(s / 3600);
            const pad = (n) => n < 10 ? '0' + n : '' + n;
            return hr > 0 ? `${hr}:${pad(min)}:${pad(sec)}` : `${min}:${pad(sec)}`;
        }

        function updateSysbar(d) {
            if (d.imuHz !== undefined) {
                setText('sys-imuhz', Number(d.imuHz).toFixed(0));
                const cell = $('sys-cell-imu');
                if (cell) {
                    cell.classList.remove('warn', 'error');
                    if (d.imuHz < 30) cell.classList.add('error');
                    else if (d.imuHz < 45) cell.classList.add('warn');
                }
            }
            if (d.rssi !== undefined) {
                setText('sys-rssi', d.rssi);
                const cell = $('sys-cell-rssi');
                if (cell) {
                    cell.classList.remove('warn', 'error');
                    if (d.rssi <= -85) cell.classList.add('error');
                    else if (d.rssi <= -75) cell.classList.add('warn');
                }
            }
            if (d.uptime !== undefined) setText('sys-uptime', formatUptime(d.uptime));
        }

        async function poll() {
            try {
                const data = await fetch('/data', { cache: 'no-store' }).then((r) => r.json());
                if (data.mode !== undefined) setText('mode', data.mode);
                setText('pitch', data.pitch);
                setText('roll', data.roll);
                setText('yaw', data.yaw);
                setText('left-speed', data.leftSpeed);
                setText('right-speed', data.rightSpeed);
                setText('left-dir', data.leftDir);
                setText('right-dir', data.rightDir);
                setText('left-status', data.leftStatus);
                setText('right-status', data.rightStatus);

                if (data.kp !== undefined) setText('kp', data.kp);
                if (data.ki !== undefined) setText('ki', data.ki);
                if (data.kd !== undefined) setText('kd', data.kd);
                if (data.target !== undefined) setText('pid-target', data.target);

                updateSysbar(data);
                telemetry.push(data);
            } catch (e) {
                console.error('Poll failed:', e);
            }
        }

        async function modeLoad() {
            try {
                const res = await fetch('/mode', { cache: 'no-store' });
                if (!res.ok) return;
                const m = await res.json();
                if (m.mode !== undefined) setText('mode', m.mode);
            } catch (e) {
                // ignore
            }
        }

        async function setModeBalance() {
            try {
                setModeStatus('sending...');
                const res = await fetch('/mode?mode=balance', { method: 'POST' });
                if (res.ok) {
                    const m = await res.json();
                    if (m.mode !== undefined) setText('mode', m.mode);
                    setModeStatus('ok');
                } else {
                    setModeStatus('error');
                }
            } catch (e) {
                console.error(e);
                setModeStatus('error');
            }
        }

        async function setModeManual() {
            try {
                setModeStatus('sending...');
                const res = await fetch('/mode?mode=manual', { method: 'POST' });
                if (res.ok) {
                    const m = await res.json();
                    if (m.mode !== undefined) setText('mode', m.mode);
                    setModeStatus('ok');
                } else {
                    setModeStatus('error');
                }
            } catch (e) {
                console.error(e);
                setModeStatus('error');
            }
        }

        async function sendRun() {
            try {
                setMotorStatus('sending...');
                const sl = $('sl').value;
                const sr = $('sr').value;
                const res = await fetch(`/control?action=run&sl=${encodeURIComponent(sl)}&sr=${encodeURIComponent(sr)}`);
                setMotorStatus(res.ok ? 'ok' : 'error');
            } catch (e) {
                console.error(e);
                setMotorStatus('error');
            }
        }

        async function sendStop() {
            try {
                setMotorStatus('sending...');
                $('sl').value = 0;
                $('sr').value = 0;
                setText('sl-val', 0);
                setText('sr-val', 0);
                const res = await fetch('/control?action=stop');
                setMotorStatus(res.ok ? 'ok' : 'error');
            } catch (e) {
                console.error(e);
                setMotorStatus('error');
            }
        }

        function renderPid(pid) {
            if (pid.kp !== undefined) {
                setText('kp', pid.kp);
                // Always sync inputs so they show the live value after Load or Apply.
                $('pid-kp-in').value = pid.kp;
            }
            if (pid.ki !== undefined) {
                setText('ki', pid.ki);
                $('pid-ki-in').value = pid.ki;
            }
            if (pid.kd !== undefined) {
                setText('kd', pid.kd);
                $('pid-kd-in').value = pid.kd;
            }
            if (pid.target !== undefined) {
                setText('pid-target', pid.target);
                $('pid-target-in').value = pid.target;
            }
        }

        function readPidInputs() {
            const kp = parseFloat($('pid-kp-in').value);
            const ki = parseFloat($('pid-ki-in').value);
            const kd = parseFloat($('pid-kd-in').value);
            const target = parseFloat($('pid-target-in').value);
            if (isNaN(kp) || isNaN(ki) || isNaN(kd) || isNaN(target)) {
                setPidStatus('fill all four fields first');
                throw new Error('PID inputs incomplete');
            }
            if (kp < 0 || ki < 0 || kd < 0) {
                setPidStatus('gains must be >= 0');
                throw new Error('negative gains');
            }
            return { kp, ki, kd, target };
        }

        async function pidLoad() {
            try {
                setPidStatus('loading...');
                const res = await fetch('/pid', { cache: 'no-store' });
                if (!res.ok) throw new Error('GET /pid failed');
                const pid = await res.json();
                renderPid(pid);
                setPidStatus('loaded');
            } catch (e) {
                console.error(e);
                setPidStatus('not available');
            }
        }

        async function pidApply() {
            try {
                const pid = readPidInputs();
                setPidStatus('applying...');
                const res = await fetch('/pid', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(pid),
                });
                if (!res.ok) throw new Error('POST /pid failed');
                const updated = await res.json();
                renderPid(updated);
                setPidStatus('applied');
            } catch (e) {
                console.error(e);
                setPidStatus('not available');
            }
        }

        async function pidReset() {
            try {
                setPidStatus('resetting...');
                const res = await fetch('/pid/reset', { method: 'POST' });
                if (res.ok) {
                    setPidStatus('integrator reset');
                } else {
                    setPidStatus('error');
                }
            } catch (e) {
                console.error(e);
                setPidStatus('error');
            }
        }

        // Load current PID gains from firmware on page open.
        async function pidLoadOnOpen() {
            try {
                const res = await fetch('/pid', { cache: 'no-store' });
                if (!res.ok) return;
                const pid = await res.json();
                renderPid(pid);
            } catch (e) { /* ignore on open */ }
        }

        telemetry.init();
        setInterval(poll, 200);
        modeLoad();
        pidLoadOnOpen();
    </script>
</body>

</html>
)rawliteral";