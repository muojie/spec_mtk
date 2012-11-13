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

#include <linux/lightsensor.h>

#include <cutils/log.h>

#include "stk_defines.h"
#include "YamahaSensorM.h"
#include "sensors.h"
/*
#define CONVERT_YAMAHA_MAG_X     (1.0/1000.0)
#define CONVERT_YAMAHA_MAG_Y     (1.0/1000.0)
#define CONVERT_YAMAHA_MAG_Z     (1.0/1000.0)
*/
/*****************************************************************************/


YamahaSensorM::YamahaSensorM(const char* class_path,const char* sensor_name,size_t nReaderSize,int nID,int nSensorType)
: YamahaSensor(class_path,sensor_name,nReaderSize,nID,nSensorType)
{

}

YamahaSensorM::~YamahaSensorM()
{
    this->EnableOrientation(0);
    enable(ID_M,0);

}
int YamahaSensorM::enable(int32_t, int en)
{
    if (en)
    {
        if (en)
            this->setInitialState();
        this->mEnabled = 1;
        this->WriteAttribute(this->m_strEnablePath,1);
    }
    else
    {
        this->mEnabled = 0;
        if (this->m_nOrientationEnable==0)
            this->WriteAttribute(this->m_strEnablePath,0);
    }
    return 0;
}



void YamahaSensorM::EnableOrientation(int nEnable)
{
    this->m_nOrientationEnable = nEnable;
    if (nEnable)
    {
        this->WriteAttribute(this->m_strEnablePath,1);
    }
    else
    {
        if (mEnabled == 0)
        {
            this->WriteAttribute(this->m_strEnablePath,0);
        }

    }

}
/*
void YamahaSensorM::processEvent(int code, int value)
{
    switch (code) {
        case ABS_X:
            mPendingEvent.data[0] = value * CONVERT_YAMAHA_MAG_X;
            break;
        case ABS_Y:
            mPendingEvent.data[1] = value * CONVERT_YAMAHA_MAG_Y;
            break;
        case ABS_Z:
            mPendingEvent.data[2] = value * CONVERT_YAMAHA_MAG_Z;
            break;
        case ABS_STATUS:
            mPendingEvent.acceleration.status = value;
        default:
            ;
    }
}*/

#endif // YAMAHA_3IN1_SENSOR
