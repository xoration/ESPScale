/*
    Copyright Stefan Seifert 2018-2019

    This file is part of ESPScale.

    ESPScale is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESPScale is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ESPScale.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SPIFFSEditor.h>
#include "CgScaleWebApi.h"
#include "Configuration.h"
#include "CgScale.h"
#include "CGModels.h"
#include "elapsedMillis.h"

AsyncWebServer server(80);

CgScaleWebApi webApi;
elapsedMillis wifiTimeoutTimer;
int wifiTimeout = 10000;

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

CgScale scale;
long t = 0;


void StationModeDisconnected(const WiFiEventStationModeDisconnected &event);
void StationModeGotIP(const WiFiEventStationModeGotIP &event);

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    SPIFFS.begin();
    Config.LoadConfiguration();

    wifi_station_set_hostname(const_cast<char *>(Config.hostname.c_str()));
    disconnectedEventHandler = WiFi.onStationModeDisconnected(&StationModeDisconnected);
    gotIpEventHandler = WiFi.onStationModeGotIP(&StationModeGotIP);

    WiFi.mode(WIFI_STA);
    WiFi.begin(Config.autoconnectSsid.c_str(), Config.autoconnectPassword.c_str());

    MDNS.begin(const_cast<char *>(Config.hostname.c_str()));
    MDNS.addService("http", "tcp", 80);

    Models.LoadModels();
    scale.Reload();
    webApi.Setup(&scale);

    wifiTimeoutTimer = 0;
}

void loop()
{
    if (wifiTimeoutTimer > wifiTimeout && WiFi.status() != WL_CONNECTED)
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(const_cast<char *>(Config.accesspointModeSsid.c_str()), Config.accesspointModePassword.c_str());
    }

    scale.Loop();
}

void StationModeDisconnected(const WiFiEventStationModeDisconnected &event)
{

    Serial.println(F("WiFi disconnection detected."));
}

void StationModeGotIP(const WiFiEventStationModeGotIP &event)
{

    Serial.print(F("WiFi connection detected: "));

    Serial.println(event.ip);
}