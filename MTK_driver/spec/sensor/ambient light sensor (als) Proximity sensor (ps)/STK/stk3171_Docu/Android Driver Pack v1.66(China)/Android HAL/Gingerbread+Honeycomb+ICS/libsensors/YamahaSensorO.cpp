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

#include "enabled_sensors.h"

#if YAMAHA_3IN1_SENSOR

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "stk_defines.h"
#include "YamahaSensorO.h"
#include "YamahaSensorA.h"
#include "YamahaSensorM.h"
#include "sensors.h"
/*****************************************************************************/




YamahaSensorO::YamahaSensorO(const char* class_path,const char* sensor_name,size_t nReaderSize,
                  int nID,int nSensorType,YamahaSensorA* pGS,YamahaSensorM* pMS)
    : YamahaSensor(class_path, sensor_name,nReaderSize,nID,nSensorType),
      m_pGSensor(pGS),
      m_pMSensor(pMS)
{
}


YamahaSensorO::~YamahaSensorO()
{
    this->m_pGSensor->EnableOrientation(0);
    this->m_pMSensor->EnableOrientation(0);
    enable(ID_O,0);
}



int YamahaSensorO::enable(int32_t handle, int en) {
    if (en!=mEnabled)
    {
        m_pGSensor->EnableOrientation(en);
        m_pMSensor->EnableOrientation(en);
        if (en)
            this->setInitialState();
        return YamahaSensor::enable(handle,en);
    }
    LOGI("No need to enable/disable orientation sensor");
    return 0;
}


#endif // YAMAHA_3IN1_SENSOR
