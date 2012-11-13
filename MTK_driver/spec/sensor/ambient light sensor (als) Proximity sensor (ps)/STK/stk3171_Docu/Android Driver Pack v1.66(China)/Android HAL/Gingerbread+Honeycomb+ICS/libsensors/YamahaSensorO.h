/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef YAMAHA_SENSOR_O_H
#define YAMAHA_SENSOR_O_H

#include "YamahaSensor.h"

/*****************************************************************************/

class YamahaSensorA;
class YamahaSensorM;

class YamahaSensorO : public YamahaSensor {
private:
    YamahaSensorO() {}
    YamahaSensorA* m_pGSensor;
    YamahaSensorM* m_pMSensor;

public:
    YamahaSensorO(const char* class_path,const char* sensor_name,size_t nReaderSize,
                  int nID,int nSensorType,YamahaSensorA* pGS,YamahaSensorM* pMS);
    virtual ~YamahaSensorO();
    virtual int enable(int32_t handle, int enabled);
};

/*****************************************************************************/

#endif  // YAMAHA_SENSOR_O_H
