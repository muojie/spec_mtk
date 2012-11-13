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

#if PS

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>
#include "stk_defines.h"
#include "ProximitySensorDistanceMode.h"

/*****************************************************************************/
#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS
ProximitySensorDistanceMode::ProximitySensorDistanceMode()
    : SensorBase(ps_enable_bin_path, PS_NAME),
      mEnabled(0),
      mInputReader(4),
      mHasPendingEvent(false)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_P;
    mPendingEvent.type = SENSOR_TYPE_PROXIMITY;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    open_device(O_RDWR);

    if (dev_fd>=0)
    {
        char flags = 0;
        if (read(dev_fd, &flags, 1) == 1)
        {
            if (flags)
            {
                mEnabled = 1;
                setInitialState();
            }
        }
		else
			LOGE("%s:read failed!", __func__);		

    }
	else
		LOGE("%s:open_device failed!", __func__);	
}
#else
ProximitySensorDistanceMode::ProximitySensorDistanceMode()
    : SensorBase(ps_enable_path, PS_NAME),
      mEnabled(0),
      mInputReader(4),
      mHasPendingEvent(false)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_P;
    mPendingEvent.type = SENSOR_TYPE_PROXIMITY;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    int fd = open(ps_enable_path,O_RDONLY);

    if (fd>=0)
    {
        char flags = '0';
        if (read(fd, &flags, 1)==1)
        {
            if (flags=='1')
            {
                mEnabled = 1;
                setInitialState();
            }
        }
		else
			LOGE("%s:read fd failed!", __func__);			
        close(fd);
    }
	else
		LOGE("%s:open fd failed!", __func__);		
}
#endif //STK_ENABLE_SENSOR_USE_BINARY_SYSFS


ProximitySensorDistanceMode::~ProximitySensorDistanceMode()
{
    enable(ID_P,0);
}

int ProximitySensorDistanceMode::setInitialState()
{
    struct input_absinfo absinfo;
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PROXIMITY), &absinfo))
    {
        // make sure to report an event immediately
        mHasPendingEvent = true;
        // In distance mode, SenseTek driver will report distance data directly, and its uint is 10um
        // 1000 * 10um ==> 1 cm
        mPendingEvent.distance = ((float)absinfo.value)/1000.0f;
    }
    return 0;
}
#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS
int ProximitySensorDistanceMode::enable(int32_t, int en) {
    int err = 0;
    char bEnable = (en?1:0);
    if (bEnable != mEnabled) {
        if (dev_fd>=0)
        {
            if(lseek(dev_fd,0,SEEK_SET) == -1)
			{
				LOGE("%s:lseek failed!", __func__);		
				return -2;
			}			
            if(write(dev_fd,&bEnable,sizeof(bEnable)) == -1)
			{
				LOGE("%s:write failed!", __func__);		
				return -3;
			}				
            mEnabled = bEnable;
            err = 0;
            if (mEnabled)
                setInitialState();
        }
        else
            err = -1;

    }
    return err;
}
#else
int ProximitySensorDistanceMode::enable(int32_t, int en) {
    int err = 0;
    int fd;
    char bEnable = (en?1:0);
    const char* strEnable[] = {"0","1"};
    if (bEnable != mEnabled) {
        fd = open(ps_enable_path,O_WRONLY);
        if (fd>=0)
        {
            if(write(fd,en?strEnable[1]:strEnable[0],2) == -1)
			{
				LOGE("%s:write failed!", __func__);		
				return -3;
			}			
            mEnabled = bEnable;
            err = 0;
            if (mEnabled)
                setInitialState();
            close(fd);
        }
        else
		{
			LOGE("%s:open fd failed!", __func__);			
            err = -1;
	    }
    }
    return err;
}
#endif //STK_ENABLE_SENSOR_USE_BINARY_SYSFS

bool ProximitySensorDistanceMode::hasPendingEvents() const {
    return mHasPendingEvent;
}

int ProximitySensorDistanceMode::readEvents(sensors_event_t* data, int count)
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

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if (event->code == EVENT_TYPE_PROXIMITY) {
                // In distance mode, SenseTek driver will report distance data directly, and its uint is 10um
                // 1000 * 10um ==> 1 cm
                mPendingEvent.distance = ((float)event->value)/1000.0f;
            }
        } else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            if (mEnabled) {
                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
            }
        } else {
            LOGE("ProximitySensorDistanceMode: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

#endif //PS
