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

#ifndef _CGSCALE_h
#define _CGSCALE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <HX711_ADC.h>

//#define WINGPEGDISTANCE 120              // The distance between the two wingpegs where the wing is laying on (Mid wingpeg).
//#define LENGTHSTOPPERTOFRONTWINGPEG 30.0 //The distance between the wingstopper to the first wingpeg (Mid wingpeg).

class CgScale
{
  private:
    HX711_ADC loadCell_front; //HX711 pins front sensor (DOUT, PD_SCK)
    HX711_ADC loadCell_back;  //HX711 pins rear sensor (DOUT, PD_SCK)

    bool needsSetup;
    float wingPegDistance;
    float lengthWingstopperToFrontWingpeg;
    float CGOffset;
    float stabilistationTime = 3000;
    long t;

  public:
    CgScale();
    CgScale(float, float);
    void Setup();
    void SetFrontCalibrationFactor(float factor);
    void SetBackCalibrationFactor(float factor);
    void Reload();
    void Tare();
    
    void Loop();

    float CG;
    float FrontWeight;
    float BackWeight;
    float TotalWeight;
};

#endif