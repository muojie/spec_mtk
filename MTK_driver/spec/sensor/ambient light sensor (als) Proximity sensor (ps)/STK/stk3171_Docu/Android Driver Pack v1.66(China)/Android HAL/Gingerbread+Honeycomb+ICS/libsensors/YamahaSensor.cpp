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

#include "YamahaSensor.h"
#include "sensors.h"

/*****************************************************************************/

#define CONVERT_YAMAHA_X     (1.0/1000.0)
#define CONVERT_YAMAHA_Y     (1.0/1000.0)
#define CONVERT_YAMAHA_Z     (1.0/1000.0)

#define YAHAMA_ATTR_PATH_MAX_SIZE   256

YamahaSensor::YamahaSensor(const char* class_path,const char* sensor_name,size_t nReaderSize,int nID,int nSensorType)
    : SensorBase("", sensor_name),
      mEnabled(0),
      mInputReader(nReaderSize),
      mHasPendingEvent(false)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = nID;
    mPendingEvent.type = nSensorType;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
    this->m_strEnablePath = new char[YAHAMA_ATTR_PATH_MAX_SIZE+1];
    this->m_strDelayPath = new char[YAHAMA_ATTR_PATH_MAX_SIZE+1];
    memset(m_strEnablePath,0,YAHAMA_ATTR_PATH_MAX_SIZE+1);
    memset(m_strDelayPath,0,YAHAMA_ATTR_PATH_MAX_SIZE+1);
    snprintf(m_strEnablePath,YAHAMA_ATTR_PATH_MAX_SIZE,"%s/%s",class_path,"enable");
    snprintf(m_strDelayPath,YAHAMA_ATTR_PATH_MAX_SIZE,"%s/%s",class_path,"delay");
}

YamahaSensor::~YamahaSensor()
{
    delete[] m_strEnablePath;
    delete[] m_strDelayPath;
}

int YamahaSensor::setInitialState() {
    struct input_absinfo absinfo_x;
    struct input_absinfo absinfo_y;
    struct input_absinfo absinfo_z;
    struct input_absinfo absinfo_status;
    int nReadStatus = ioctl(data_fd, EVIOCGABS(ABS_STATUS),&absinfo_status);
    if (!ioctl(data_fd, EVIOCGABS(ABS_X), &absinfo_x) &&
        !ioctl(data_fd, EVIOCGABS(ABS_Y), &absinfo_y) &&
        !ioctl(data_fd, EVIOCGABS(ABS_Z), &absinfo_z)) {
        mPendingEvent.data[0] = absinfo_x.value * CONVERT_YAMAHA_X;
        mPendingEvent.data[1] = absinfo_y.value * CONVERT_YAMAHA_Y;
        mPendingEvent.data[2] = absinfo_z.value * CONVERT_YAMAHA_Z;
        if (!nReadStatus)
            mPendingEvent.acceleration.status = absinfo_status.value;
        mHasPendingEvent = true;
    }
    return 0;
}
int YamahaSensor::enable(int32_t, int en)
{
    if (en)
    {
        this->mEnabled = 1;
        this->WriteAttribute(this->m_strEnablePath,1);
    }
    else
    {
        this->mEnabled = 0;
        this->WriteAttribute(this->m_strEnablePath,0);
    }
    return 0;
}

int YamahaSensor::setDelay(int32_t handle, int64_t ns)
{
    uint32_t ms;
    ms = uint32_t(ns/1000000);
    int err = 0;

    err = WriteAttribute(this->m_strDelayPath,ms);
    return err;
}


bool YamahaSensor::hasPendingEvents() const {
    return mHasPendingEvent;
}

int YamahaSensor::readEvents(sensors_event_t* data, int count)
{

    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }
    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event))
    {
        int type = event->type;
        if (type == EV_ABS)
        {
            processEvent(event->code, event->value);
            mHasPendingEvent = true;

        } else if (type == EV_SYN)
        {
            mPendingEvent.timestamp = timevalToNano(event->time);
            if (mEnabled) {
                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
            }
        } else {
            LOGE("YamahaSensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }
    return numEventReceived;
}

void YamahaSensor::processEvent(int code, int value)
{
    switch (code) {
        case ABS_X:
            mPendingEvent.data[0] = value * CONVERT_YAMAHA_X;
            break;
        case ABS_Y:
            mPendingEvent.data[1] = value * CONVERT_YAMAHA_Y;
            break;
        case ABS_Z:
            mPendingEvent.data[2] = value * CONVERT_YAMAHA_Z;
            break;
        case ABS_STATUS:
            mPendingEvent.acceleration.status = value;
        default:
            ;
    }
}

int YamahaSensor::WriteAttribute(const char* strAttrPath,const int32_t value)
{
    size_t buf_size = 16;
    char buffer[buf_size+1];
    int fd;
    int nWriteSize;
    memset(buffer,0,sizeof(buffer)+1);
    nWriteSize = snprintf(buffer,buf_size,"%d", value);

    fd = open(strAttrPath,O_WRONLY);
    if (fd>=0)
    {
        write(fd,buffer,nWriteSize+1);
        close(fd);
        return 0;
    }
    LOGE_IF(fd<0,"Error : open %s fail",strAttrPath);

    return -EINVAL;
}

#endif // YAMAHA_3IN1_SENSOR
