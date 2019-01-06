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

#include <CGModels.h>
#include "ArduinoJson.h"
#include <FS.h>
#include <SPIFFSEditor.h>
#include <AsyncJson.h>

#define MODELSFILE "/models.jsn"

CGModels::CGModels()
{
}

void CGModels::LoadModels()
{
    if (SPIFFS.exists(MODELSFILE))
    {
        File file = SPIFFS.open(MODELSFILE, "r");
        String content = file.readStringUntil('\n');

        DynamicJsonBuffer jsonBuffer;
        JsonArray &modelFile = jsonBuffer.parseArray(content);
        
        for (auto value : modelFile)
        {
            JsonObject &model = value.as<JsonObject>();
            Models.AddModel(model["Name"].asString(), model["CG"].as<float>(), model["Weight"].as<float>());
        }

        file.close();
    }
}

void CGModels::SaveModels()
{
    File file = SPIFFS.open(MODELSFILE, "w");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &modelsJson = jsonBuffer.createObject();
    JsonArray &modelsFile = modelsJson.createNestedArray("Models");

    for (CGModel value : models) 
    {
        JsonObject& model = modelsFile.createNestedObject();
        model["Name"] = value.Name;
        model["CG"] = value.CG;
        model["Weight"] = value.Weight; 
    }

    modelsFile.printTo(file);
    file.close();
}

bool CGModels::AddModel(String name, float cg, float weight)
{

    CGModel model;
    model.Name = name;
    model.CG  = cg;
    model.Weight = weight;

    for (CGModel cgModel : models) 
    {
        if (cgModel.Name == name) 
        {
            return false;
        } 
    }

    models.push_back(model);

    return true;
}

bool CGModels::DelModel(String name)
{
    models.remove_if([&](CGModel model) { return model.Name == name; });
}

CGModels Models;