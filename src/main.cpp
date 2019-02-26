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
//#define DISPLAY

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SPIFFSEditor.h>
#include "CgScaleWebApi.h"
#include "Configuration.h"
#include "CgScale.h"
#include "CGModels.h"
#include "elapsedMillis.h"

#ifdef DISPLAY
#include "U8g2lib.h"
#endif

AsyncWebServer server(80);

CgScaleWebApi webApi;
elapsedMillis wifiTimeoutTimer;
int wifiTimeout = 10000;

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

CgScale scale;
long t = 0;

#ifdef DISPLAY
U8G2_SH1106_128X64_NONAME_1_HW_I2C oledDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/D3, /* data=*/D4);
#endif

void StationModeDisconnected(const WiFiEventStationModeDisconnected &event);
void StationModeGotIP(const WiFiEventStationModeGotIP &event);

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    SPIFFS.begin();
    Config.LoadConfiguration();

#ifdef DISPLAY
    oledDisplay.begin();
    oledDisplay.setPowerSave(0);

    oledDisplay.setFont(u8g2_font_ncenB14_tr);
#endif

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

#ifdef DISPLAY
    oledDisplay.firstPage();
    do
    {
        oledDisplay.setCursor(90, 20);
        oledDisplay.print("CG: ");
        oledDisplay.setCursor(0, 20);
        oledDisplay.print(scale.CG);
        oledDisplay.setCursor(90, 50);
        oledDisplay.print("Weight: ");
        oledDisplay.setCursor(0, 50);
        oledDisplay.print(scale.TotalWeight);
    } while (oledDisplay.nextPage());
#endif
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