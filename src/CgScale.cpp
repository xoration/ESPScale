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

#include "CgScale.h"
#include <HX711_ADC.h>
#include "Configuration.h"

CgScale::CgScale() : CgScale(0, 0)
{
}

CgScale::CgScale(float wingPegDistance, float lengthWingstopperToFrontWingpeg) : loadCell_front(D6, D5), //HX711 pins front sensor (DOUT, PD_SCK)
                                                                                 loadCell_back(D2, D1)   //HX711 pins front sensor (DOUT, PD_SCK)
{
    this->wingPegDistance = wingPegDistance;
    this->lengthWingstopperToFrontWingpeg = lengthWingstopperToFrontWingpeg;
    this->CG = 0;
    this->FrontWeight = 0;
    this->BackWeight = 0;
    this->TotalWeight = 0;
    this->needsSetup = true;
    this->CGOffset = ((wingPegDistance / 2) + lengthWingstopperToFrontWingpeg);
}

void CgScale::SetFrontCalibrationFactor(float factor)
{
    loadCell_front.setCalFactor(factor);
}
void CgScale::SetBackCalibrationFactor(float factor)
{
    loadCell_back.setCalFactor(factor);
}

void CgScale::Setup()
{
    ESP.wdtDisable();

    loadCell_front.begin();
    loadCell_back.begin();

    byte loadcell_1_rdy = 0;
    byte loadcell_2_rdy = 0;
    while ((loadcell_1_rdy + loadcell_2_rdy) < 2)
    {
        delay(1);
        if (!loadcell_1_rdy)
        {
            loadcell_1_rdy = loadCell_front.startMultiple(stabilistationTime);
            if (loadcell_1_rdy)
            {
                Serial.println("Loadcell front stabilised.");
            }
        }

        if (!loadcell_2_rdy)
        {
            loadcell_2_rdy = loadCell_back.startMultiple(stabilistationTime);
            if (loadcell_2_rdy)
            {
                Serial.println("Loadcell back stabilised.");
            }
        }
    }

    loadCell_front.setCalFactor(Config.FrontCellCalibrationFactor);
    loadCell_back.setCalFactor(Config.BackCellCalibrationFactor);
    this->needsSetup = false;

    ESP.wdtEnable(WDTO_4S);
}

void CgScale::Reload()
{
    loadCell_front.setCalFactor(Config.FrontCellCalibrationFactor);
    loadCell_back.setCalFactor(Config.BackCellCalibrationFactor);
    this->wingPegDistance = Config.WingPegDistance;
    this->lengthWingstopperToFrontWingpeg = Config.LengthWingstopperToFrontWingpeg;
    this->CGOffset = ((this->wingPegDistance / 2) + this->lengthWingstopperToFrontWingpeg);
}

void CgScale::Loop()
{
    if (needsSetup)
    {
        Setup();
    }

    loadCell_front.update();
    loadCell_back.update();

    FrontWeight = loadCell_front.getData();
    BackWeight = loadCell_back.getData();

    TotalWeight = FrontWeight + BackWeight;

    double CGRatio = 0;

    if (BackWeight > 10 && FrontWeight > 10)
    {
        CGRatio = BackWeight / (FrontWeight + BackWeight);
        CG = (wingPegDistance * CGRatio) - (wingPegDistance / 2) + CGOffset;
    }
    else
    {
        CG = 0;
    }
}