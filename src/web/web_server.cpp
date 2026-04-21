#include "WebServer.h"
#include "config.h"
#include "web_server.h"
#include "../wifi/wifi_manager.h"
#include "motors.h"
#include "imu.h"
#include "pages.h"

namespace
{
    WebServer server(cfg::WEB_SERVER_PORT);
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
}

void handleRoot()
{
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

    String json = "{";
    json += "\"pitch\":" + String(a.pitch, 2) + ",";
    json += "\"roll\":" + String(a.roll, 2) + ",";
    json += "\"yaw\":" + String(a.yaw, 2) + ",";
    json += "\"leftSpeed\":" + String(m.speedLeft, 1) + ",";
    json += "\"rightSpeed\":" + String(m.speedRight, 1) + ",";
    json += "\"leftDir\":\"" + String(dirToStr(m.dirLeft)) + "\",";
    json += "\"rightDir\":\"" + String(dirToStr(m.dirRight)) + "\",";
    json += "\"status\":\"" + String(m.status == MotorState::Status::RUNNING ? "RUNNING" : "IDLE") + "\"";
    json += "}";

    server.send(200, "application/json", json);
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
        motorsStop();
    }
    else if (action == "run")
    {
        float sl = server.hasArg("sl") ? server.arg("sl").toFloat() : 0.0f;
        float sr = server.hasArg("sr") ? server.arg("sr").toFloat() : 0.0f;
        motorsSetSpeed(sl, sr);
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
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("[WEB] Server started");
}

void webServerUpdate()
{
    server.handleClient();
}
