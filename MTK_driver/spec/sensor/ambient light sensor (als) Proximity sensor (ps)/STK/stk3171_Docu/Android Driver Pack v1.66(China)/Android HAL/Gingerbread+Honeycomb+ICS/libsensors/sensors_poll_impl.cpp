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

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>

#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>

#include "enabled_sensors.h"
#include "sensors_poll_impl.h"
#include "stk_defines.h"
#include "sensors.h"
#if AKM_3IN1_SENSOR
#include "AkmSensor.h"
#endif

#if INVENSENSE_3IN1_SENSOR
#include "Invensense3in1Sensor.h"
#endif

#if YAMAHA_3IN1_SENSOR
#include "YamahaSensor.h"
#include "YamahaSensorA.h"
#include "YamahaSensorM.h"
#include "YamahaSensorO.h"
#endif

#if PS // SenseTek/Sitronix
#include "ProximitySensorBinaryMode.h"
#include "ProximitySensorDistanceMode.h"
#endif

#if LS // SenseTek/Sitronix
#include "LightSensor.h"
#endif

#if (AS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
#include "Accelerometer.h"
#endif

#if (MS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
#include "ECompass.h"
#endif

#if (OS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
#include "Orientation.h"
#endif

#if (GS && (!INVENSENSE_3IN1_SENSOR))
#include "GyroSensor.h"
#endif

/* Add here if you need PR/LA/GR/RV...*/


#define DriveBase 0

#if AKM_3IN1_SENSOR

#define akm_driver (DriveBase)
#define ts_driver (akm_driver+1)
#define ps_driver (ts_driver+TS)
#define ls_driver (ps_driver+PS)
#define gs_driver (ls_driver+LS)
#define pr_driver (gs_driver+GS)
#define gr_driver (pr_driver+PR)
#define la_driver (gr_driver+GR)
#define rv_driver (la_driver+LA)

static int HandleToDriver(int handle)
{
    switch (handle)
    {
        case ID_A:
        case ID_O:
        case ID_M:
            return akm_driver;
#if PS
        case ID_P:
            return ps_driver;
#endif

#if LS
        case ID_L:
            return ls_driver;
#endif

#if TS
        case ID_T:
            return ts_driver;
#endif
#if GS
        case ID_G:
            return gs_driver;
#endif
#if PR
        case ID_PR:
            return pr_driver;
#endif
#if GR
        case ID_GR:
            return gr_driver;
#endif
#if LA
        case ID_LA:
            return la_driver;
#endif
#if RV
        case ID_RV:
            return rv_driver;
#endif
        default:
        ;
    }
    return -EINVAL;
}

#define numSensorDrivers (1+TS+PS+LS+GS+PR+GR+LA+RV)

#elif INVENSENSE_3IN1_SENSOR

#define ins_driver (DriveBase)
#define ts_driver (ins_driver+1)
#define ps_driver (ts_driver+TS)
#define ls_driver (ps_driver+PS)
static int HandleToDriver(int handle)
{
    switch (handle)
    {
        case ID_A:
        case ID_O:
        case ID_M:
        case ID_RV:
        case ID_LA:
        case ID_GR:
        case ID_PR:
        case ID_G:
            return ins_driver;
#if PS
        case ID_P:
            return ps_driver;
#endif

#if LS
        case ID_L:
            return ls_driver;
#endif
        default:
        ;
    }
    return -EINVAL;
}
#define numSensorDrivers (1+TS+PS+LS)

#elif YAMAHA_3IN1_SENSOR
#define SEARCH_PATH_MAX_LENGTH 512
#define BUF_SIZE 4096
namespace
{

    int remove_newline(char *buf, int len)
    {
        if (buf == NULL || len < 0)
        {
            return -1;
        }
        while (len > 0 && (buf[len - 1] == '\n' || buf[len -1] == '\r'))
        {
            buf[len - 1] = '\0';
            len--;
        }
        return 0;
    }

    int find_string(const char* strSouce,const char* strFind)
    {
        int nLengthSrc=0,nLengthFind=0;
        int nPosition;
        nLengthSrc = strlen(strSouce);
        nLengthFind = strlen(strFind);
        nPosition = 0;
        while(nLengthSrc>0)
        {
            if (nLengthSrc < nLengthFind)
                return -1;
            if (*strSouce==*strFind)
            {
                if (strncmp(strSouce,strFind,nLengthFind)==0)
                    return nPosition;
            }
            nPosition++;
            strSouce++;
            nLengthSrc--;
        }
        return -1;
    }

    char* search_input_path(const char* strInputName)
    {
        const char *dirname = "/sys/class/input";
        char buf[BUF_SIZE+1];
        char classpath[SEARCH_PATH_MAX_LENGTH+1];
        DIR *dir;
        struct dirent *dir_entry;
        int fd, nread;
        bool found = false;
        const int nLengthOfInputParse = strlen("input");

        if (strInputName == NULL ) {
            return NULL;
        }

        dir = opendir(dirname);
        if(dir == NULL) {
            return NULL;
        }

        while((dir_entry = readdir(dir)))
        {
            if (strncmp(dir_entry->d_name, "input", nLengthOfInputParse) != 0)
            {
                continue;
            }

            snprintf(classpath, SEARCH_PATH_MAX_LENGTH, "%s/%s", dirname, dir_entry->d_name);
            snprintf(buf, sizeof(buf), "%s/name", classpath);

            fd = open(buf, O_RDONLY);
            if (fd < 0)
            {
                continue;
            }
            memset(buf,0,BUF_SIZE+1);
            if ((nread = read(fd, buf, BUF_SIZE)) < 0)
            {
                close(fd);
                continue;
            }
            remove_newline(buf,nread);
            if (strcmp(buf,strInputName)==0)
            {
                found = 1;
                close(fd);
                break;
            }
            close(fd);
            fd = -1;
        }

        closedir(dir);

        if (found)
        {
            char* pBuffer = new char[strlen(classpath)+1];
            strcpy(pBuffer,classpath);
            INFO("Sensor HAL : Found input %s @ %s",strInputName,pBuffer);
            return pBuffer;
        }
        return NULL;
    }
    inline void free_searched_path(char* strPath)
    {
        if (strPath!=NULL)
            delete[] strPath;
    }

};
#define as_driver (DriveBase)
#define ms_driver (as_driver+1)
#define os_driver (ms_driver+1)
#define ts_driver (os_driver+1)
#define ps_driver (ts_driver+TS)
#define ls_driver (ps_driver+PS)
#define gs_driver (ls_driver+LS)
#define pr_driver (gs_driver+GS)
#define gr_driver (pr_driver+PR)
#define la_driver (gr_driver+GR)
#define rv_driver (la_driver+LA)

static int HandleToDriver(int handle)
{
    switch (handle)
    {
        case ID_A:
            return as_driver;
        case ID_M:
            return ms_driver;
        case ID_O:
            return os_driver;
#if PS
        case ID_P:
            return ps_driver;
#endif
#if LS
        case ID_L:
            return ls_driver;
#endif
#if TS
        case ID_T:
            return ts_driver;
#endif
#if GS
        case ID_G:
            return gs_driver;
#endif
#if PR
        case ID_PR:
            return pr_driver;
#endif
#if GR
        case ID_GR:
            return gr_driver;
#endif
#if LA
        case ID_LA:
            return la_driver;
#endif
#if RV
        case ID_RV:
            return rv_driver;
#endif
        default:
        ;
    }
    return -EINVAL;
}
#define numSensorDrivers (AS+MS+OS+TS+PS+LS+GS+PR+GR+LA+RV)

#else
// Discrete

#define as_driver (DriveBase)
#define ms_driver (as_driver+AS)
#define os_driver (ms_driver+MS)
#define ts_driver (os_driver+OS)
#define ps_driver (ts_driver+TS)
#define ls_driver (ps_driver+PS)
#define gs_driver (ls_driver+LS)
#define pr_driver (gs_driver+GS)
#define gr_driver (pr_driver+PR)
#define la_driver (gr_driver+GR)
#define rv_driver (la_driver+LA)

static int HandleToDriver(int handle)
{
    switch (handle)
    {
#if AS
        case ID_A:
            return as_driver;
#endif
#if MS
        case ID_M:
            return akm_driver;
#endif
#if OS
        case ID_O:
            return os_driver;
#endif
#if PS
        case ID_P:
            return ps_driver;
#endif

#if LS
        case ID_L:
            return ls_driver;
#endif

#if TS
        case ID_T:
            return ts_driver;
#endif
#if GS
        case ID_G:
            return gs_driver;
#endif
#if PR
        case ID_PR:
            return pr_driver;
#endif
#if GR
        case ID_GR:
            return gr_driver;
#endif
#if LA
        case ID_LA:
            return la_driver;
#endif
#if RV
        case ID_RV:
            return rv_driver;
#endif
        default:
        ;
    }
    return -EINVAL;
}

#define numSensorDrivers (AS+MS+OS+TS+PS+LS+GS+PR+GR+LA+RV)

#endif



#define numFds (numSensorDrivers+1)

/*****************************************************************************/

struct sensors_poll_context_t {
    struct sensors_poll_device_t device; // must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);

private:


    static const size_t wake = numFds - 1;
    static const char WAKE_MESSAGE = 'W';
    struct pollfd mPollFds[numFds];
    int mWritePipeFd;
    SensorBase* mSensors[numSensorDrivers];

};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{

#if AKM_3IN1_SENSOR
    mSensors[akm_driver] = new AkmSensor();
    mPollFds[akm_driver].fd = mSensors[akm_driver]->getFd();
    mPollFds[akm_driver].events = POLLIN;
    mPollFds[akm_driver].revents = 0;
#elif INVENSENSE_3IN1_SENSOR
    mSensors[ins_driver] = new MPLSensor();
    mPollFds[ins_driver].fd = mSensors[ins_driver]->getFd();
    mPollFds[ins_driver].events = POLLIN;
    mPollFds[ins_driver].revents = 0;

#elif YAMAHA_3IN1_SENSOR
    YamahaSensorA *pA;
    YamahaSensorM *pM;
    char* strPath;
    strPath = search_input_path(as_name);
    INFO("SENSOR HAL : Found input %s @ %s",as_name,strPath);
    pA = new YamahaSensorA(strPath,as_name,4,ID_A,SENSOR_TYPE_ACCELEROMETER);
    free_searched_path(strPath);
    strPath = search_input_path(ms_name);
    INFO("SENSOR HAL : Found input %s @ %s",ms_name,strPath);
    pM = new YamahaSensorM(strPath,ms_name,4,ID_M,SENSOR_TYPE_MAGNETIC_FIELD);
    free_searched_path(strPath);
    mSensors[as_driver] = pA;
    mPollFds[as_driver].fd = mSensors[as_driver]->getFd();
    mPollFds[as_driver].events = POLLIN;
    mPollFds[as_driver].revents = 0;

    mSensors[ms_driver] = pM;
    mPollFds[ms_driver].fd = mSensors[ms_driver]->getFd();
    mPollFds[ms_driver].events = POLLIN;
    mPollFds[ms_driver].revents = 0;
    strPath = search_input_path(os_name);
    INFO("SENSOR HAL : Found input %s @ %s",os_name,strPath);
    mSensors[os_driver] = new YamahaSensorO(strPath,os_name,4,ID_O,SENSOR_TYPE_ORIENTATION,pA,pM);
    free_searched_path(strPath);
    mPollFds[os_driver].fd = mSensors[os_driver]->getFd();
    mPollFds[os_driver].events = POLLIN;
    mPollFds[os_driver].revents = 0;
#endif

#if LS
    mSensors[ls_driver] = new LightSensor();
    mPollFds[ls_driver].fd = mSensors[ls_driver]->getFd();
    mPollFds[ls_driver].events = POLLIN;
    mPollFds[ls_driver].revents = 0;
#endif

#if PS
    int fd;
    uint8_t ps_distance_mode = 0;
#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS
    fd = open(ps_distance_mode_bin_path,O_RDONLY);
	read(fd,&ps_distance_mode,1);
#else
    const int buf_size = 20;
    char buffer[buf_size+1];
    fd = open(ps_distance_mode_path,O_RDONLY);
    read(fd,buffer,buf_size);
    ps_distance_mode = atoi(buffer)?1:0;
#endif
    close(fd);
	if (ps_distance_mode)
	{
        mSensors[ps_driver] = new ProximitySensorDistanceMode();
	}
    else
    {
        mSensors[ps_driver] = new ProximitySensorBinaryMode();

    }
    mPollFds[ps_driver].fd = mSensors[ps_driver]->getFd();
    mPollFds[ps_driver].events = POLLIN;
    mPollFds[ps_driver].revents = 0;
#endif

 #if (GS && (!INVENSENSE_3IN1_SENSOR))
    mSensors[gs_driver] = new GyroSensor();
    mPollFds[gs_driver].fd = mSensors[gs_driver]->getFd();
    mPollFds[gs_driver].events = POLLIN;
    mPollFds[gs_driver].revents = 0;
#endif

#if (MS && (!(INVENSENSE_3IN1_SENSOR||AKM_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
    mSensors[ms_driver] = new ECompass();
    mPollFds[ms_driver].fd = mSensors[ms_driver]->getFd();
    mPollFds[ms_driver].events = POLLIN;
    mPollFds[ms_driver].revents = 0;
#endif
#if (AS && (!(INVENSENSE_3IN1_SENSOR||AKM_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
    mSensors[as_driver] = new Accelerometer();
    mPollFds[as_driver].fd = mSensors[as_driver]->getFd();
    mPollFds[as_driver].events = POLLIN;
    mPollFds[as_driver].revents = 0;
#endif

#if (OS && (!(INVENSENSE_3IN1_SENSOR||AKM_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
    mSensors[os_driver] = new OrientationSensor();
    mPollFds[os_driver].fd = mSensors[os_driver]->getFd();
    mPollFds[os_driver].events = POLLIN;
    mPollFds[os_driver].revents = 0;
#endif


/* Add here if you need extra driver, e.g. PR/RV/GR/LA/TS */


    int wakeFds[2];
    int result = pipe(wakeFds);
    LOGE_IF(result<0, "error creating wake pipe (%s)", strerror(errno));
    fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
    fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
    mWritePipeFd = wakeFds[1];

    mPollFds[wake].fd = wakeFds[0];
    mPollFds[wake].events = POLLIN;
    mPollFds[wake].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i=0 ; i<numSensorDrivers ; i++) {
        delete mSensors[i];
    }
    close(mPollFds[wake].fd);
    close(mWritePipeFd);
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int index = HandleToDriver(handle);
    if (index < 0) return index;
    int err =  mSensors[index]->enable(handle, enabled);
    if (enabled && !err) {
        const char wakeMessage(WAKE_MESSAGE);
        int result = write(mWritePipeFd, &wakeMessage, 1);
        LOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
    }
    return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {

    int index = HandleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->setDelay(handle, ns);
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;

    do {
        // see if we have some leftover from the last poll()
        for (int i=0 ; count && i<numSensorDrivers ; i++)
        {
            SensorBase* const sensor(mSensors[i]);
            if ((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents()))
            {
                int nb = sensor->readEvents(data, count);
                if (nb < count)
                {
                    // no more data for this sensor
                    mPollFds[i].revents = 0;
                }
                count -= nb;
                nbEvents += nb;
                data += nb;
            }
        }

        if (count)
        {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait if we don't have
            // anything to return
            n = poll(mPollFds, numFds, nbEvents ? 0 : -1);
            if (n<0)
            {
                LOGE("poll() failed (%s)", strerror(errno));
                return -errno;
            }
            if (mPollFds[wake].revents & POLLIN)
            {
                char msg;
                int result = read(mPollFds[wake].fd, &msg, 1);
                LOGE_IF(result<0, "error reading from wake pipe (%s)", strerror(errno));
                LOGE_IF(msg != WAKE_MESSAGE, "unknown message on wake queue (0x%02x)", int(msg));
                mPollFds[wake].revents = 0;
            }
        }
        // if we have events and space, go read them
    } while (n && count);

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}


int init_sensors_poll_impl(hw_module_t const* module, hw_device_t** device)
{
    int status = -EINVAL;
    sensors_poll_context_t *dev = new sensors_poll_context_t();
    memset(&dev->device, 0, sizeof(sensors_poll_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = 0;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;

    *device = &dev->device.common;
    status = 0;
    return status;
}


/*****************************************************************************/


