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

#include "Configuration.h"
#include <FS.h>
#include <SPIFFSEditor.h>
#include <AsyncJson.h>

#define CONFIGURATIONFILE "/cfg/configuration.jsn"

Configuration::Configuration()
{
    WingPegDistance = 0;
    LengthWingstopperToFrontWingpeg = 0;
    FrontCellCalibrationFactor = 1;
    BackCellCalibrationFactor = 1;
}

void Configuration::LoadConfiguration()
{
    if (SPIFFS.exists(CONFIGURATIONFILE))
    {
        File file = SPIFFS.open(CONFIGURATIONFILE, "r");
        String content = file.readStringUntil('\n');

        DynamicJsonBuffer jsonBuffer;
        JsonObject &configFile = jsonBuffer.parseObject(content);
        FrontCellCalibrationFactor = configFile["FrontcellCalFactor"];
        BackCellCalibrationFactor = configFile["BackcellCalFactor"];
        WingPegDistance = configFile["WingPegDistance"];
        LengthWingstopperToFrontWingpeg = configFile["LengthWingstopperToFrontWingpeg"];
        file.close();
    }
}

void Configuration::SaveConfiguration()
{
    File file = SPIFFS.open(CONFIGURATIONFILE, "w");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &configFile = jsonBuffer.createObject();
    configFile["FrontcellCalFactor"] = FrontCellCalibrationFactor;
    configFile["BackcellCalFactor"] = BackCellCalibrationFactor;
    configFile["WingPegDistance"] = WingPegDistance;
    configFile["LengthWingstopperToFrontWingpeg"] = LengthWingstopperToFrontWingpeg;

    configFile.printTo(file);
    file.close();
}

Configuration Config;