#include "WebServer.h"
#include "config.h"
#include "web_server.h"
#include "../wifi/wifi_manager.h"

namespace
{
    WebServer server(cfg::WEB_SERVER_PORT);
}

void handleRoot()
{
    String html = R"rawliteral(
    <html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Self-Balancing Robot - Dashboard</title>
    <style>
   
    </style>
</head>
<body>
    <header>
        <h1>Self-Balancing Robot - Dashboard</h1>
    </header>

    <main>
        <!-- IMU -->
         <section>
            <h2>IMU</h2>
            <div id="imu-data">
                <p>Pitch: <span id="pitch">0</span>°</p>
                <p>Roll: <span id="roll">0</span>°</p>
                <p>Yaw: <span id="yaw">0</span>°</p>
            </div>
         </section>
    </main>

    <script>
        
    </script>
</body>
</html>)rawliteral";
    server.send(200, "text/html", html);
}

void handleNotFound()
{
    server.send(404, "text/plain", "Not found");
}

void webServerInit()
{
    connectWiFi(cfg::WIFI_SSID, cfg::WIFI_PASS);
    Serial.printf("[WEB] Server starting at http://%s\n", getLocalIP().c_str());

    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[WEB] Server started");
}

void webServerUpdate()
{
    server.handleClient();
}
