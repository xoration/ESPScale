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

#include <ESPAsyncWiFiManager.h>

char hostName[] = "CGScale";


AsyncWebServer server(80);
DNSServer dns;

CgScaleWebApi webApi;
CgScale scale;
long t = 0;

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    wifi_station_set_hostname(hostName);
    AsyncWiFiManager wifiManager(&server, &dns);

    wifiManager.autoConnect("CGScale");

    MDNS.addService("http", "tcp", 80);

    SPIFFS.begin();
    Config.LoadConfiguration();
    Models.LoadModels();
    scale.Reload();
    webApi.Setup(&scale);
}

void loop()
{
    scale.Loop();
}