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

#if (AS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR)))

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include "Accelerometer.h"

#ifdef IS_STK8311
	#include "stk8311.h"
#elif defined IS_STK8312
	#include "stk8312.h"
#endif	//#ifdef IS_STK8311

#define DEBUG_ACC 1
/*****************************************************************************/

Accelerometer::Accelerometer()
: SensorBase(ACC_DEVICE_PATH, ACCELEROMETER_NAME),
      mEnabled(0),
      mHasPendingEvent(false),
      mInputReader(4)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_A;
    mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    open_device(O_RDWR);

    if (dev_fd>=0)
	{
		char flags = 0;
		if (!ioctl(dev_fd, STK_IOCTL_GET_ENABLE, &flags)) 
		{
			if (flags) 
			{
				mEnabled = 1;
				setInitialState();
			}
		}	
    }
    if (!mEnabled) {
        close_device();
    }	
	
}

Accelerometer::~Accelerometer()
{
    enable(ID_A,0);
}

int Accelerometer::enable(int32_t handle, int enabled)
{
    char flags = enabled ? 1 : 0;
    int err = 0;
    if (flags != mEnabled) 
	{
        if (!mEnabled) 
            open_device(O_RDWR);
        err = ioctl(dev_fd, STK_IOCTL_SET_ENABLE, &flags);
        err = err<0 ? -errno : 0;
        LOGE_IF(err, "STK_IOCTL_SET_ENABLE failed (%s)", strerror(-err));
        if (!err) 
		{
            mEnabled = enabled ? 1 : 0;
            if (enabled) 
                setInitialState();
        }
        if (!mEnabled) 
            close_device();
    }
    return err;
}

bool Accelerometer::hasPendingEvents() const {
    return mHasPendingEvent;
}

int Accelerometer::setDelay(int32_t handle, int64_t ns)
{
	uint32_t delay_cmd = (uint32_t)ns;
    int err = 0;
	
    if (mEnabled) 
	{	
#if DEBUG_ACC
	LOGD("%s:delay=%d\n", __func__, delay_cmd);
#endif	//#if DEBUG_ACC		
	
	//	open_device(O_RDWR);
		err = ioctl(dev_fd, STK_IOCTL_SET_DELAY, &delay_cmd);
        err = err<0 ? -errno : 0;
        LOGE_IF(err, "STK_IOCTL_SET_ENABLE failed (%s)", strerror(-err));			
	//	close_device();
	}
    return err;
}

int Accelerometer::setInitialState() 
{
    struct input_absinfo absinfo_x;
    struct input_absinfo absinfo_y;
    struct input_absinfo absinfo_z;
    float value;
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_X), &absinfo_x) &&
        !ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Y), &absinfo_y) &&
        !ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Z), &absinfo_z)) 
	{
        value = absinfo_x.value;
        mPendingEvent.acceleration.x = value * CONVERT_A_X;
        value = absinfo_y.value;
        mPendingEvent.acceleration.y = value * CONVERT_A_Y;
        value = absinfo_z.value;
        mPendingEvent.acceleration.z = value * CONVERT_A_Z;
		mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
        mHasPendingEvent = true;
    }
    return 0;
}


int Accelerometer::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) 
	{
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }
    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

#if DEBUG_ACC
	LOGD("Read %d sets of acc dat\n", n);
#endif	//#if DEBUG_ACC		
		
    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event))
    {
        int type = event->type;
        if (type == EV_ABS)
        {
#if DEBUG_ACC
			LOGD("type = EV_ABS,code=%d,value=%d\n", event->code, event->value);
#endif	//#if DEBUG_ACC				
            processEvent(event->code, event->value);
            mHasPendingEvent = true;

        } 
		else if (type == EV_SYN)
        {	
#if DEBUG_ACC
			LOGD("type = EV_SYN,acc:%6.3f,%6.3f,%6.3f\n",mPendingEvent.acceleration.x, mPendingEvent.acceleration.y, mPendingEvent.acceleration.z);
#endif	//#if DEBUG_ACC			
            if (mEnabled) 
			{
				
				mPendingEvent.timestamp = timevalToNano(event->time);			
                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
            }
        } 
		else 
		{
            LOGE("Accelerometer: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }
    return numEventReceived;
}

void Accelerometer::processEvent(int code, int value)
{
    switch (code) 
	{
        case EVENT_TYPE_ACCEL_X:
            mPendingEvent.acceleration.x = value * CONVERT_A_X;
            break;
        case EVENT_TYPE_ACCEL_Y:
            mPendingEvent.acceleration.y = value * CONVERT_A_Y;
            break;
        case EVENT_TYPE_ACCEL_Z:
            mPendingEvent.acceleration.z = value * CONVERT_A_Z;
            break;
        default:
            ;
    }
}

#endif //(AS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR)))
