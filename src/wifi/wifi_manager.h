#pragma once
#include "WiFi.h"

void connectWiFi(const char *ssid, const char *pass)
{
    if (cfg::WIFI_SSID[0] == '\0')
    {
        Serial.println("No WiFi SSID set");
    }

    Serial.printf("Connecting to %s", ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

bool isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String getLocalIP()
{
    return WiFi.localIP().toString();
}