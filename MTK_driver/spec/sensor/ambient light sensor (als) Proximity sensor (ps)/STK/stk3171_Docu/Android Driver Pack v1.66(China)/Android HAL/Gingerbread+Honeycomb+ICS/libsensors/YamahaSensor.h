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

#ifndef YAMAHA_SENSOR_H
#define YAMAHA_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors_poll_impl.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

#define ABS_STATUS                          (ABS_BRAKE)

class YamahaSensor : public SensorBase
{
protected:
    int mEnabled;
    InputEventCircularReader mInputReader;
    sensors_event_t mPendingEvent;
    bool mHasPendingEvent;
    int setInitialState();
    int WriteAttribute(const char* strAttrPath,const int32_t value);
    char* m_strEnablePath;
    char* m_strDelayPath;
    YamahaSensor() : SensorBase(NULL,NULL),mInputReader(4) {}
    virtual void processEvent(int code, int value);
public:

            YamahaSensor(const char* dev_path,const char* sensor_name,size_t nReaderSize,int nID,int nSensorType);
    virtual ~YamahaSensor();
    virtual int readEvents(sensors_event_t* data, int count);
    virtual bool hasPendingEvents() const;
    virtual int enable(int32_t handle, int enabled);
    virtual int setDelay(int32_t handle, int64_t ns);
private:



};

#define as_name "accelerometer"
#define ms_name "geomagnetic"
#define os_name "orientation"

/*****************************************************************************/

#endif  // YAMAHA_SENSOR_H
