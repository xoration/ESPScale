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

#ifndef _CGSCALECONFIGURATION_h
#define _CGSCALECONFIGURATION_h

#include "Arduino.h"
#include <list>

class Configuration
{
  private:
  public:
    Configuration();
    void LoadConfiguration();
    void SaveConfiguration();

    float WingPegDistance;
    float LengthWingstopperToFrontWingpeg;
    float FrontCellCalibrationFactor;
    float BackCellCalibrationFactor;
    String hostname;
    String autoconnectSsid;
    String autoconnectPassword;
    String accesspointModeSsid;
    String accesspointModePassword;
};

extern Configuration Config;

#endif