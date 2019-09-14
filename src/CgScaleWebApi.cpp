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
#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <AsyncJson.h>
#include "CgScaleWebApi.h"
#include "CgScale.h"
#include "CGModels.h"
#include "Configuration.h"

CgScaleWebApi::CgScaleWebApi() : server(80),
                                 ws("/ws"),
                                 events("/events")
{
}

void CgScaleWebApi::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
#ifdef WEBDEBUG
    if (type == WS_EVT_CONNECT)
    {
        char str[64];
        sprintf(str, "Websocket: [%s][%u] connect", server->url(), client->id());

        Serial.println(str);
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        char str[64];
        sprintf(str, "Websocket: [%s][%u] disconnect", server->url(), client->id());
        Serial.println(str);
    }
#endif
}

void CgScaleWebApi::onNotFound(AsyncWebServerRequest *request)
{
#ifdef WEBDEBUG
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
        Serial.printf("GET");
    else if (request->method() == HTTP_POST)
        Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
        Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
        Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
        Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
        Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
        Serial.printf("OPTIONS");
    else
        Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength())
    {
        Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
        Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }
#endif

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++)
    {
        AsyncWebHeader *h = request->getHeader(i);

#ifdef WEBDEBUG
        Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
#endif
    }

#ifdef WEBDEBUG
    int params = request->params();
    for (i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);

        if (p->isFile())
        {
            Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        }
        else if (p->isPost())
        {
            Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
        else
        {
            Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }
#endif

    request->send(404);
}

void CgScaleWebApi::OnScaleGet(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &root = response->getRoot();

    root[F("CG")] = String(localScale->CG);
    root[F("Weight")] = String(localScale->TotalWeight, 2);
    root[F("FrontWeight")] = String(localScale->FrontWeight, 2);
    root[F("BackWeight")] = String(localScale->BackWeight, 2);

    if (Config.WingPegDistance <= 0 || Config.LengthWingstopperToFrontWingpeg <= 0 || Config.FrontCellCalibrationFactor == 1 || Config.BackCellCalibrationFactor == 1)
    {
        root[F("CalibrationNecessary")] = true;
    }
    else
    {
        root[F("CalibrationNecessary")] = false;
    }

    response->setLength();
    request->send(response);
}

void CgScaleWebApi::OnScaleTare(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &root = response->getRoot();

	localScale->Tare();

    response->setLength();
    request->send(response);
}

void CgScaleWebApi::OnSettingsGet(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &root = response->getRoot();

    root[F("Hostname")] = Config.hostname;
    root[F("AutoconnectSsid")] = Config.autoconnectSsid;
    root[F("AutoconnectPassword")] = Config.autoconnectPassword;
    root[F("AccesspointModeSsid")] = Config.accesspointModeSsid;
    root[F("AccesspointModePassword")] = Config.accesspointModePassword;

    root[F("FrontCalibrationFactor")] = Config.FrontCellCalibrationFactor;
    root[F("BackCalibrationFactor")] = Config.BackCellCalibrationFactor;
    root[F("WingPegDistance")] = Config.WingPegDistance;
    root[F("LengthWingstopperToFrontWingpeg")] = Config.LengthWingstopperToFrontWingpeg;

    response->setLength();
    request->send(response);
}

void CgScaleWebApi::OnSettingsPost(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &retMsg = response->getRoot();

    retMsg[F("type")] = F("warning");

    if (!request->hasParam("data", true))
    {
        retMsg[F("message")] = F("No values found!");
        response->setLength();
        request->send(response);
        return;
    }

    String json = request->getParam("data", true)->value();

    if (json.length() > 1024)
    {
        retMsg[F("message")] = F("Data too large!");
        response->setLength();
        request->send(response);
        return;
    }

    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);

    if (!root.success())
    {
        retMsg[F("message")] = F("Failed to parse data!");
        response->setLength();
        request->send(response);
        return;
    }

    if (!(root.containsKey("FrontCalibrationFactor") 
            && root.containsKey("BackCalibrationFactor") 
            && root.containsKey("WingPegDistance") 
            && root.containsKey("LengthWingstopperToFrontWingpeg") 
            && root.containsKey("Hostname") 
            && root.containsKey("AutoconnectSsid") 
            && root.containsKey("AutoconnectPassword") 
            && root.containsKey("AccesspointModeSsid") 
            && root.containsKey("AccesspointModePassword")))
    {
        retMsg[F("message")] = F("Values are missing!");
        response->setLength();
        request->send(response);
        return;
    }

    String wifiConfigValue;
    wifiConfigValue = root[F("Hostname")].as<String>();

    if (wifiConfigValue == "")
    {
        retMsg[F("message")] = F("Hostname can't be empty!");
        response->setLength();
        request->send(response);
        return;
    }
    Config.hostname = wifiConfigValue;

    wifiConfigValue = root[F("AutoconnectSsid")].as<String>();
    Config.autoconnectSsid = wifiConfigValue;

    wifiConfigValue = root[F("AutoconnectPassword")].as<String>();
    Config.autoconnectPassword = wifiConfigValue;

    wifiConfigValue = root[F("AccesspointModeSsid")].as<String>();
    Config.accesspointModeSsid = wifiConfigValue;

    wifiConfigValue = root[F("AccesspointModePassword")].as<String>();
    Config.accesspointModePassword = wifiConfigValue;

    float value = 0;
    value = root[F("FrontCalibrationFactor")].as<float>();

    if (value == 0)
    {
        retMsg[F("message")] = F("Front Calibration Factor can't be 0!");
        response->setLength();
        request->send(response);
        return;
    }
    Config.FrontCellCalibrationFactor = value;

    value = root[F("BackCalibrationFactor")].as<float>();

    if (value == 0)
    {
        retMsg[F("message")] = F("Back Calibration Factor can't be 0!");
        response->setLength();
        request->send(response);
        return;
    }

    Config.BackCellCalibrationFactor = value;

    value = root[F("WingPegDistance")].as<float>();

    if (value < 0)
    {
        retMsg[F("message")] = F("WinpPeg distance can't smaller than 0!");
        response->setLength();
        request->send(response);
        return;
    }

    Config.WingPegDistance = value;

    value = root[F("LengthWingstopperToFrontWingpeg")].as<float>();

    if (value < 0)
    {
        retMsg[F("message")] = F("Distance wingstopper to first wingpeg can't smaller than 0!");
        response->setLength();
        request->send(response);
        return;
    }

    Config.LengthWingstopperToFrontWingpeg = value;

    Config.SaveConfiguration();
    localScale->Reload();

    retMsg[F("type")] = F("success");
    retMsg[F("message")] = F("Configuration successfully saved!");
    response->setLength();
    request->send(response);
}

void CgScaleWebApi::OnModelsGet(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &root = response->getRoot();
    JsonArray &modelsJson = root.createNestedArray("Models");

    for (CGModel value : Models.models)
    {
        JsonObject &model = modelsJson.createNestedObject();
        model[F("ModelName")] = value.Name;
        model[F("ModelWeight")] = value.Weight;
        model[F("ModelCg")] = value.CG;
    }

    response->setLength();
    request->send(response);
}

void CgScaleWebApi::OnModelsPost(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &retMsg = response->getRoot();

    retMsg[F("type")] = F("warning");

    if (!request->hasParam("data", true))
    {
        retMsg[F("message")] = F("No values found!");
        response->setLength();
        request->send(response);
        return;
    }

    String json = request->getParam("data", true)->value();

    if (json.length() > 1024)
    {
        retMsg[F("message")] = F("Data too large!");
        response->setLength();
        request->send(response);
        return;
    }

    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);

    if (!root.success())
    {
        retMsg[F("message")] = F("Failed to parse data!");
        response->setLength();
        request->send(response);
        return;
    }

    if (!(root.containsKey("ModelName")))
    {
        retMsg[F("message")] = F("Values are missing!");
        response->setLength();
        request->send(response);
        return;
    }

    String modelName = root[F("ModelName")].as<String>();

    if (!(root.containsKey("ModelCg")))
    {
        retMsg[F("message")] = F("Values are missing!");
        response->setLength();
        request->send(response);
        return;
    }

    float modelCg = root[F("ModelCg")].as<float>();

    if (!(root.containsKey("ModelWeight")))
    {
        retMsg[F("message")] = F("Values are missing!");
        response->setLength();
        request->send(response);
        return;
    }

    float modelWeight = root[F("ModelWeight")].as<float>();

    Models.AddModel(modelName, modelCg, modelWeight);
    Models.SaveModels();
    retMsg[F("type")] = F("success");
    retMsg[F("message")] = F("Models saved!");
    response->setLength();
    request->send(response);
}

void CgScaleWebApi::OnModelsDeletePost(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject &retMsg = response->getRoot();

    retMsg[F("type")] = F("warning");

    if (!request->hasParam("data", true))
    {
        retMsg[F("message")] = F("No values found!");
        response->setLength();
        request->send(response);
        return;
    }

    String json = request->getParam("data", true)->value();

    if (json.length() > 1024)
    {
        retMsg[F("message")] = F("Data too large!");
        response->setLength();
        request->send(response);
        return;
    }

    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);

    if (!root.success())
    {
        retMsg[F("message")] = F("Failed to parse data!");
        response->setLength();
        request->send(response);
        return;
    }

    if (!(root.containsKey("ModelName")))
    {
        retMsg[F("message")] = F("Values are missing!");
        response->setLength();
        request->send(response);
        return;
    }

    String modelName = root[F("ModelName")].as<String>();

    Models.DelModel(modelName);
    Models.SaveModels();
    retMsg[F("type")] = F("success");
    retMsg[F("message")] = F("Models saved!");
    response->setLength();
    request->send(response);
}

void CgScaleWebApi::Setup(CgScale *scale)
{
    using namespace std::placeholders;
    localScale = scale;

    ws.onEvent(std::bind(&CgScaleWebApi::onWsEvent, this, _1, _2, _3, _4, _5, _6));
    server.addHandler(&ws);

    server.addHandler(new SPIFFSEditor("admin", "admin"));

    events.onConnect([](AsyncEventSourceClient *client) {
        client->send("hello!", NULL, millis(), 1000);
    });
    server.addHandler(&events);

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.on("/api/scale", HTTP_GET, std::bind(&CgScaleWebApi::OnScaleGet, this, _1));
    server.on("/api/scale/tare", HTTP_POST, std::bind(&CgScaleWebApi::OnScaleTare, this, _1));
    server.on("/api/settings", HTTP_GET, std::bind(&CgScaleWebApi::OnSettingsGet, this, _1));
    server.on("/api/settings", HTTP_POST, std::bind(&CgScaleWebApi::OnSettingsPost, this, _1));
    server.on("/api/models/delete", HTTP_POST, std::bind(&CgScaleWebApi::OnModelsDeletePost, this, _1));
    server.on("/api/models", HTTP_GET, std::bind(&CgScaleWebApi::OnModelsGet, this, _1));
    server.on("/api/models", HTTP_POST, std::bind(&CgScaleWebApi::OnModelsPost, this, _1));

    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

    server.onNotFound(std::bind(&CgScaleWebApi::onNotFound, this, _1));

    server.begin();
}

void CgScaleWebApi::Loop()
{
}