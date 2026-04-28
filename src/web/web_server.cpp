#include "WebServer.h"
#include "config.h"
#include "web_server.h"
#include "../wifi/wifi_manager.h"
#include "motors.h"
#include "imu.h"
#include "balance.h"
#include "page.h"

// Provided by main.cpp — sliding-window estimate of the effective control-loop rate.
extern float mainGetImuHz();

namespace
{
    WebServer server(cfg::WEB_SERVER_PORT);
    const char *modeToStr(ControlMode m)
    {
        return (m == ControlMode::BALANCE) ? "BALANCE" : "MANUAL";
    }
    const char *dirToStr(MotorState::Direction d)
    {
        switch (d)
        {
        case MotorState::Direction::FORWARD:
            return "FORWARD";
        case MotorState::Direction::BACKWARD:
            return "BACKWARD";
        default:
            return "STOP";
        }
    };

    bool extractJsonFloat(const String &body, const char *key, float &out)
    {
        const String needle = String("\"") + key + "\":";
        int idx = body.indexOf(needle);
        if (idx < 0)
            return false;
        idx += needle.length();

        // Skip whitespace
        while (idx < (int)body.length() && isspace((unsigned char)body[idx]))
            idx++;

        const char *start = body.c_str() + idx;
        char *end = nullptr;
        const float v = strtof(start, &end);
        if (end == start)
            return false;
        out = v;
        return true;
    }
}

void handleRoot()
{
    // Prevent stale UI from browser cache (important while iterating on embedded HTML/JS).
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", ROOT_PAGE);
}

void handleNotFound()
{
    server.send(404, "text/plain", "Not found");
}

void handleData()
{
    const MotorState &m = motorsGetState();
    const Angles &a = imuGetAngles();

    const bool leftRunning  = fabsf(m.speedLeft)  > cfg::MOTOR_DEADBAND_STEPS_PER_SEC;
    const bool rightRunning = fabsf(m.speedRight) > cfg::MOTOR_DEADBAND_STEPS_PER_SEC;

    float kp = 0, ki = 0, kd = 0;
    balanceGetPidGains(kp, ki, kd);

    float pidP = 0, pidI = 0, pidD = 0;
    balanceGetPidComponents(pidP, pidI, pidD);

    const float measuredAngle = balanceGetMeasuredAngle();
    const float maxSps = cfg::MOTOR_MAX_STEPS_PER_SEC;

    String json = "{";
    json += "\"mode\":\"" + String(modeToStr(balanceGetMode())) + "\",";
    json += "\"pitch\":" + String(a.pitch, 2) + ",";
    json += "\"roll\":" + String(measuredAngle, 2) + ",";
    json += "\"yaw\":" + String(a.yaw, 2) + ",";
    json += "\"leftSpeed\":" + String(m.speedLeft, 1) + ",";
    json += "\"rightSpeed\":" + String(m.speedRight, 1) + ",";
    json += "\"leftDir\":\"" + String(dirToStr(m.dirLeft)) + "\",";
    json += "\"rightDir\":\"" + String(dirToStr(m.dirRight)) + "\",";
    json += "\"leftStatus\":\"" + String(leftRunning ? "RUNNING" : "IDLE") + "\",";
    json += "\"rightStatus\":\"" + String(rightRunning ? "RUNNING" : "IDLE") + "\",";
    json += "\"kp\":" + String(kp, 4) + ",";
    json += "\"ki\":" + String(ki, 6) + ",";
    json += "\"kd\":" + String(kd, 4) + ",";
    json += "\"target\":" + String(balanceGetTargetAngle(), 2) + ",";
    // PID component breakdown (steps/s) so the tuning UI can chart contributions.
    json += "\"pidP\":" + String(pidP, 1) + ",";
    json += "\"pidI\":" + String(pidI, 1) + ",";
    json += "\"pidD\":" + String(pidD, 1) + ",";
    json += "\"maxSps\":" + String(maxSps, 0) + ",";
    // System telemetry (cheap to compute, useful when accessing remotely).
    json += "\"imuHz\":" + String(mainGetImuHz(), 1) + ",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"uptime\":" + String(millis() / 1000UL);
    json += "}";

    server.send(200, "application/json", json);
}

void handleModeGet()
{
    String json = "{";
    json += "\"mode\":\"" + String(modeToStr(balanceGetMode())) + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleModePost()
{
    // Accept either JSON body: {"mode":"BALANCE"} / {"mode":"MANUAL"}
    // or query: ?mode=balance|manual
    String modeArg;
    if (server.hasArg("mode"))
        modeArg = server.arg("mode");
    else
        modeArg = server.arg("plain");

    modeArg.toLowerCase();

    if (modeArg.indexOf("balance") >= 0)
    {
        balanceSetMode(ControlMode::BALANCE);
        handleModeGet();
        return;
    }
    if (modeArg.indexOf("manual") >= 0)
    {
        balanceSetMode(ControlMode::MANUAL);
        handleModeGet();
        return;
    }

    server.send(400, "text/plain", "Invalid mode. Use BALANCE or MANUAL.");
}

void handlePidGet()
{
    float kp = 0, ki = 0, kd = 0;
    balanceGetPidGains(kp, ki, kd);

    String json = "{";
    json += "\"kp\":" + String(kp, 6) + ",";
    json += "\"ki\":" + String(ki, 6) + ",";
    json += "\"kd\":" + String(kd, 6) + ",";
    json += "\"target\":" + String(balanceGetTargetAngle(), 2);
    json += "}";

    server.send(200, "application/json", json);
}

void handlePidPost()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");
    const String body = server.arg("plain");

    if (body.length() == 0)
    {
        server.send(400, "text/plain", "Empty body");
        return;
    }

    float kp = 0, ki = 0, kd = 0;

    // Read current gains as defaults so a partial update doesn't zero missing fields.
    balanceGetPidGains(kp, ki, kd);
    float target = balanceGetTargetAngle();

    extractJsonFloat(body, "kp", kp);
    extractJsonFloat(body, "ki", ki);
    extractJsonFloat(body, "kd", kd);
    extractJsonFloat(body, "target", target);

    balanceSetPidGains(kp, ki, kd);
    balanceSetTargetAngle(target);
    handlePidGet();
}

void handlePidReset()
{
    balanceResetPid();
    server.send(200, "text/plain", "ok");
}

void handleControl()
{
    if (!server.hasArg("action"))
    {
        server.send(400, "text/plain", "Missing action");
        return;
    }

    String action = server.arg("action");

    if (action == "stop")
    {
        balanceHardStop();
    }
    else if (action == "run")
    {
        float sl = server.hasArg("sl") ? server.arg("sl").toFloat() : 0.0f;
        float sr = server.hasArg("sr") ? server.arg("sr").toFloat() : 0.0f;
        balanceSetMode(ControlMode::MANUAL);
        balanceSetManualSpeeds(sl, sr);
    }

    server.send(200, "text/plain", "ok");
}

void webServerInit()
{
    connectWiFi(cfg::WIFI_SSID, cfg::WIFI_PASS);
    Serial.printf("[WEB] Server starting at http://%s:%d\n",
                  getLocalIP().c_str(), cfg::WEB_SERVER_PORT);

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/control", handleControl);
    server.on("/mode", HTTP_GET, handleModeGet);
    server.on("/mode", HTTP_POST, handleModePost);
    server.on("/pid", HTTP_GET, handlePidGet);
    server.on("/pid", HTTP_POST, handlePidPost);
    server.on("/pid/reset", HTTP_POST, handlePidReset);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("[WEB] Server started");
}

void webServerUpdate()
{
    server.handleClient();
}
