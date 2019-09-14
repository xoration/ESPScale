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

#ifndef _CGSCALEWEBAPI_h
#define _CGSCALEWEBAPI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Hash.h>
#include <ESPAsyncWebServer.h>
#include "CgScale.h"

#define WEBDEBUG

class CgScaleWebApi
{
  private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    AsyncEventSource events;
    CgScale *localScale;

    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void onNotFound(AsyncWebServerRequest *request);
    void OnScaleGet(AsyncWebServerRequest *request);
    void OnScaleTare(AsyncWebServerRequest *request);
    void OnSettingsGet(AsyncWebServerRequest *request);
    void OnSettingsPost(AsyncWebServerRequest *request);
    void OnModelsGet(AsyncWebServerRequest *request);
    void OnModelsPost(AsyncWebServerRequest *request);
    void OnModelsDeletePost(AsyncWebServerRequest *request);

  public:
    CgScaleWebApi();
    void Setup(CgScale *scale);
    void Loop();
};
#endif