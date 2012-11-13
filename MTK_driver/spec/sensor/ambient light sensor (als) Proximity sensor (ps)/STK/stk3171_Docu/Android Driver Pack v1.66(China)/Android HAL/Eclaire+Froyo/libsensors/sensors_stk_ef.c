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

/* SenseTek Sensors HAL Template
 * Copyright (C) 2011 Sitronix
 * Author : Patrick Chang
 * Version : 1.1
 * Support Sensor : STK ALS
 *
 */


#define LOG_TAG "Sensors"
#define LOG_NDEBUG 1

#define DBG_INFO 0


/* Define Utilities Marco*/
#if DBG_INFO
#define  D(...)  LOGD(__VA_ARGS__)
#else
#define  D(...)  ((void)0)
#endif

#define  E(...)  LOGE(__VA_ARGS__)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#if DBG_INFO
#define INFO(...) LOGI(__VA_ARGS__)
#else
#define INFO(...) ((void)0)
#endif


#define __MAX(a,b) ((a>b)?a:b)
#define __MAX3(a,b,c) __MAX(__MAX(a,b),c)
#define __MAX4(a,b,c,d) __MAX(__MAX(a,b),__MAX(c,d))

/* Enable / Disable Sensor
 *  You Must Modify Here
 */
#define AS 0	/*Accelerometer*/
#define MS 0	/*Magnetic Sensor*/
#define OS 0	/*Orientation Sensor*/
#define TS 0	/*Temperature Sensor*/
#define PS 1	/*Proximity Sensor*/
#define LS 1	/*Light Sensor*/
#define GS 0	/*Gyroscope*/
#define SS 0	/*Pressure Sensor, Stress Sensor*/


#define MAX_NUM_SENSORS (AS+MS+OS+TS+PS+LS)

#define ID_BASE 0
#define ID_A  (ID_BASE)
#define ID_M  (ID_BASE+AS) 
#define ID_O  (ID_BASE+AS+MS)
#define ID_T  (ID_BASE+AS+MS+OS)
#define ID_P  (ID_BASE+AS+MS+OS+TS)
#define ID_L  (ID_BASE+AS+MS+OS+TS+PS)
#define ID_G  (ID_BASE+AS+MS+OS+TS+PS+LS)
#define ID_S  (ID_BASE+AS+MS+OS+TS+PS+LS+SS)


/* Definition flags of 
 * SENSORS_ACCELERATION 
 * SENSORS_MAGNETIC_FIELD
 * SENSORS_ORIENTATION
 * SENSORS_TEMPERATURE
 * SENSORS_PROXIMITY
 * SENSORS_LIGHT
 */
#if AS
#define	SENSORS_ACCELERATION	(1 << ID_A)
#else
#define SENSORS_ACCELERATION	0
#endif


#if MS
#define SENSORS_MAGNETIC_FIELD	(1 << ID_M)
#else
#define SENSORS_MAGNETIC_FIELD	0
#endif

#if OS
#define SENSORS_ORIENTATION		(1 << ID_O)
#else
#define SENSORS_ORIENTATION		0
#endif

#if TS
#define SENSORS_TEMPERATURE		(1 << ID_T)
#else
#define SENSORS_TEMPERATURE		0
#endif

#if PS
#define SENSORS_PROXIMITY		(1 << ID_P)
#else
#define SENSORS_PROXIMITY		0
#endif

#if LS
#define SENSORS_LIGHT			(1 << ID_L)
#else
#define SENSORS_LIGHT			0
#endif

#if GS
#define SENSORS_GYRO				(1 << ID_G)
#else
#define SENSORS_GYRO				0
#endif

#if SS
#define SENSORS_PRESSURE			(1 << ID_S)
#else
#define SENSORS_PRESSURE			0
#endif



/* Modify here if you want to add another FDs*/
#define AS_FD 0
#define MS_FD 0
#define OS_FD 0
#define TS_FD 0
/* if TS/OS/MS/AS is a group --> only set TS_FD = 1*/
#define PS_FD 1
#define LS_FD 1
/* if LS/PS is a group --> only set LS_FD = 1*/
#define GS_FD 0
#define SS_FD 0


#define ID_FD_BASE 	0
#define ID_AS_FD  	(ID_FD_BASE)
#define ID_MS_FD	(ID_FD_BASE+AS_FD) 
#define ID_OS_FD  	(ID_FD_BASE+AS_FD+MS_FD)
#define ID_TS_FD  	(ID_FD_BASE+AS_FD+MS_FD+OS_FD)
#define ID_PS_FD  	(ID_FD_BASE+AS_FD+MS_FD+OS_FD+TS_FD)
#define ID_LS_FD  	(ID_FD_BASE+AS_FD+MS_FD+OS_FD+TS_FD+PS_FD)
#define ID_GS_FD  	(ID_FD_BASE+AS_FD+MS_FD+OS_FD+TS_FD+PS_FD+LS_FD)
#define ID_SS_FD  	(ID_FD_BASE+AS_FD+MS_FD+OS_FD+TS_FD+PS_FD+LS_FD+SS_FD)

#define MAX_LINUX_NUM_FDS			(AS_FD+MS_FD+OS_FD+TS_FD+PS_FD+LS_FD+GS_FD+SS_FD)


#define SUPPORTED_SENSORS  (SENSORS_ACCELERATION|SENSORS_MAGNETIC_FIELD|SENSORS_ORIENTATION|SENSORS_TEMPERATURE|SENSORS_PROXIMITY|SENSORS_LIGHT|SENSORS_GYRO|SENSORS_PRESSURE)



/**************************
 * Add Your Definition Here
 **************************/ 





/**************************
 * Definition End
 **************************/

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <cutils/native_handle.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <linux/input.h>
#include <hardware/sensors.h>
#include "stk_defines.h"

/* include your header file here */



/** SENSOR IDS AND NAMES
 **/


#define  ID_CHECK(x)  ((unsigned)((x)-ID_BASE) < MAX_NUM_SENSORS)
static uint32_t nDelayMicroSecond = 20000;

static int id_to_sensor[MAX_NUM_SENSORS] = {
#if AS	
    [ID_A] = SENSOR_TYPE_ACCELEROMETER,
#endif
#if MS    
    [ID_M] = SENSOR_TYPE_MAGNETIC_FIELD,
#endif
#if OS
    [ID_O] = SENSOR_TYPE_ORIENTATION,
#endif
#if TS    
    [ID_T] = SENSOR_TYPE_TEMPERATURE,
#endif
#if PS
    [ID_P] = SENSOR_TYPE_PROXIMITY,
#endif
#if LS
    [ID_L] = SENSOR_TYPE_LIGHT,
#endif
#if GS
	[ID_G] = SENSOR_TYPE_GYROSCOPE,
#endif

#if SS
	[ID_S] = SENSOR_TYPE_PRESSURE,
#endif
};


/* Data Type Declaration*/
/* sensors_control_context_t
 *
 * This one is used to send commands to the sensors drivers.
 */

typedef struct sensors_control_context_t {
    struct sensors_control_device_t  device;
    uint32_t                         active_sensors;
} sensors_control_context_t;


/* sensors_data_context_t
 *
 * This one is used to read sensor data from the hardware.
 */

typedef struct sensors_data_context_t {
    struct sensors_data_device_t  device;
    sensors_data_t                sensors[MAX_NUM_SENSORS];
    int                           events_fd[MAX_LINUX_NUM_FDS];
    uint32_t                      pendingSensors;

} sensors_data_context_t;


/* Add your private data type here */





/* end of private data type */



/** MODULE REGISTRATION SUPPORT
 **
 ** This is required so that hardware/libhardware/hardware.c
 ** will dlopen() this library appropriately.
 **/

static uint8_t ps_distance_mode = 0;

/*
 * the following is the list of all supported sensors.
 * this table is used to build sSensorList declared below
 * according to which hardware sensors are reported as
 * available from the emulator (see get_sensors_list below)
 *
 *
 */
static struct sensor_t sSensorList[MAX_NUM_SENSORS] = {
 
#if AS	
	{
        .name       = "SenseTek Accelerometer",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_A,
        .type       = SENSOR_TYPE_ACCELEROMETER,
        .maxRange   = 8.0f,
        .resolution = 8.0f/2048.0f,
        .power      = 0.1f,
        .reserved   = {}
    },
#endif

#if MS	
	{
        .name       = "SenseTek M-Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_M,
        .type       = SENSOR_TYPE_MAGNETIC_FIELD,
        .maxRange   = 2000.0f,
        .resolution = 1.0f,
        .power      = 0.1f,
        .reserved   = {}
    },
#endif

#if OS	
	{
        .name       = "SenseTek Orientation Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_O,
        .type       = SENSOR_TYPE_ORIENTATION,
        .maxRange   = 360.0f,
        .resolution = 1.0f,
        .power      = 0.0f,
        .reserved   = {}
    },
#endif

#if TS	
	{
        .name       = "SenseTek Temperature Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_T,
        .type       = SENSOR_TYPE_TEMPERATURE,
        .maxRange   = 200.0f,
        .resolution = 0.1f,
        .power      = 0.1f,
        .reserved   = {}
    },
#endif

#if PS	
	{
        .name       = "SenseTek Proximity Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_P,
        .type       = SENSOR_TYPE_PROXIMITY,
        .maxRange   = 10.0f,
        .resolution = 0.1f,
        .power      = 0.1f,
        .reserved   = {}
    },
#endif
	
#if LS	
	{
        .name       = "SenseTek Ambient Light Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_L,
        .type       = SENSOR_TYPE_LIGHT,
        .maxRange   = 4096.0f,
        .resolution = 1.0f,
        .power      = 0.083f,
        .reserved   = {}
    },
#endif

#if GS	
	{
        .name       = "SenseTek Gyro Scope",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_G,
        .type       = SENSOR_TYPE_GYROSCOPE,
        .maxRange   = 4096.0f,
        .resolution = 1.0f,
        .power      = 0.083f,
        .reserved   = {}
    },
#endif

#if SS	
	{
        .name       = "SenseTek Pressure Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_S,
        .type       = SENSOR_TYPE_PRESSURE,
        .maxRange   = 4096.0f,
        .resolution = 1.0f,
        .power      = 0.083f,
        .reserved   = {}
    },
#endif


};

static int open_inputs(int mode, int *pFD,char* strSearchName)
{
    /* scan all input drivers and look for strSearchName */
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    *pFD = -1;
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, mode);
        if (fd>=0) 
        {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) 
            {
                name[0] = '\0';
            }
            if (!strcmp(name, strSearchName))
            {
                *pFD = fd;
                closedir(dir);
                return 0;
            }
            else
                close(fd);
        }
    }
    closedir(dir);

    fd = 0;
    if (*pFD < 0) {
        E("Couldn't find or open '%s' driver (%s)",strSearchName, strerror(errno));
        fd = -1;
    }
    return fd;	
}




static int64_t data__now_ns(void);
static int pick_sensor(sensors_data_context_t* data, sensors_data_t*  values);
static void data__poll_process_syn(struct sensors_data_context_t *dev,
                                   struct input_event *event,
                                   uint32_t new_sensors);
static int data__close(struct hw_device_t *dev);

static int open_sensors(const struct hw_module_t* module,const char* name,struct hw_device_t* *device);
static int sensors__get_sensors_list(struct sensors_module_t* module,struct sensor_t const** list);
/* Module Declaration*/

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

const struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "Senstek SENSORS Module",
        .author = "Patrick Chang/SenseTek/Sitronix",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list,
};


/* this must return a file descriptor that will be used to read
 * the sensors data (it is passed to data__data_open() below
 */
static native_handle_t*
control__open_data_source(struct sensors_control_device_t *dev)
{
    sensors_control_context_t*  ctl = (void*)dev;
    native_handle_t* handle;
	int als_fd;
#if PS_FD		
	int ps_fd;
#endif
    // TODO
	/* open sensor device */
	if (open_inputs(O_RDONLY,&als_fd,ALS_NAME)<0)
	{
		D("Open ALS INPUT Fail");
		return NULL;
	}
#if PS_FD	
	if (open_inputs(O_RDONLY,&ps_fd,PS_NAME)<0)
	{
		D("Open PS INPUT Fail");
		return NULL;		
	}
#endif	
	handle = native_handle_create(MAX_LINUX_NUM_FDS, 0);
    handle->data[ID_LS_FD] = als_fd;
#if PS_FD	    
    handle->data[ID_PS_FD] = ps_fd;
#endif    

    return handle;
}




static int
control__activate(struct sensors_control_device_t *dev,
                  int handle,
                  int enabled)
{
    struct sensors_control_context_t*  ctl = (struct sensors_control_context_t*)dev;
    uint32_t        mask, sensors, active, new_sensors, changed;
    int             ret;
    int 			fd;
    uint8_t 		bEnable;
#if (!STK_ENABLE_SENSOR_USE_BINARY_SYSFS)
	const char* strEnable[]={"0","1"};
#endif    

    INFO("%s: handle=%s (%d) enabled=%d", __FUNCTION__,
        sSensorList[handle].name, handle, enabled);

    if (!ID_CHECK(handle)) {
        E("%s: bad handle ID", __FUNCTION__);
        return -1;
    }

    mask    = (1<<handle);
    sensors = enabled ? mask : 0;

    active      = ctl->active_sensors;
    new_sensors = (active & ~mask) | (sensors & mask);
    changed     = active ^ new_sensors;

	D("%s: chagned:%d, active:%x, mask:%x, sensors:%x", __FUNCTION__, changed, active, mask, sensors);

	/* To Do : Modify Here*/
    ctl->active_sensors = new_sensors;
	if (changed == 0)
		return 0;
							  
	switch(handle)
	{
#if AS		
		case ID_A:
		break;
#endif

#if MS		
		case ID_M:
		break;
#endif
#if OS		
		case ID_O:
		break;
#endif
#if TS		
		case ID_T:
		break;
#endif
#if PS		
		case ID_P:
#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS		
		fd = open(ps_enable_bin_path,O_WRONLY);
		if (fd>=0)
		{
			bEnable = enabled?1:0;
			ret = write(fd,&bEnable,1);
			close(fd);
			
		}
#else
		fd = open(ps_enable_path,O_WRONLY);
		if (fd>=0)
		{
			bEnable = enabled?1:0;
			ret = write(fd,strEnable[bEnable],2);
			close(fd);
		}

#endif		
		break;
#endif
#if LS		
		case ID_L:
#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS		
		fd = open(als_enable_bin_path,O_WRONLY);
		if (fd>=0)
		{
			bEnable = enabled?1:0;
			ret = write(fd,&bEnable,1);
			close(fd);
			
		}
#else
		fd = open(als_enable_path,O_WRONLY);
		if (fd>=0)
		{
			bEnable = enabled?1:0;
			ret = write(fd,strEnable[bEnable],2);
			close(fd);
		}
#endif			
		break;
#endif
#if GS
		case ID_G:
		break;
#endif

#if SS
		case ID_S:
		break;
#endif
		
	}

    return 0;
}

static int
control__set_delay(struct sensors_control_device_t *dev, int32_t ms)
{
    sensors_control_context_t*  ctl = (sensors_control_context_t*)dev;
	int fd;
		
	if (ms!=0)
		nDelayMicroSecond = ms*1000;
	else
		nDelayMicroSecond = 2000;
    return 0;

}

/* this function is used to force-stop the blocking read() in
 * data__poll. In order to keep the implementation as simple
 * as possible here, we send a command to the emulator which
 * shall send back an appropriate data block to the system.
 */
static int control__wake(struct sensors_control_device_t *dev)
{
    sensors_control_context_t*  ctl = (void*)dev;
    D("%s: dev=%p", __FUNCTION__, dev);
    /* TODO */
    int err = 0;
    int als_fd;
#if PS    
    int ps_fd;
#endif    
    if (open_inputs(O_RDWR, &als_fd,ALS_NAME) < 0)
    {
        return -1;
    }
#if PS    
	if (open_inputs(O_RDWR, &ps_fd,PS_NAME) < 0)
    {
        return -1;
    }
#endif    
    struct input_event event[1];
    event[0].type = EV_SYN;
    event[0].code = SYN_CONFIG;
    event[0].value = 0;
#if PS
    err = write(ps_fd, event, sizeof(event));
    LOGV_IF(err<0, "control__wake(proximity), fd=%d (%s)",
            p_fd, strerror(errno));
    close(ps_fd);
#endif
    err = write(als_fd, event, sizeof(event));
    LOGV_IF(err<0, "control__wake(light), fd=%d (%s)",
            l_fd, strerror(errno));
    close(als_fd);
    return err;
}


static int control__close(struct hw_device_t *dev) 
{
    sensors_control_context_t*  ctl = (void*)dev;

    free(ctl);
    return 0;
}




/* return the current time in nanoseconds */
static int64_t data__now_ns(void)
{
    struct timespec  ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static int data__data_open(struct sensors_data_device_t *dev, native_handle_t* handle)
{
    sensors_data_context_t*  data = (void*)dev;
    int i;
    memset(&data->sensors, 0, sizeof(data->sensors));

    for (i=0 ; i<MAX_NUM_SENSORS ; i++) {
        data->sensors[i].vector.status = SENSOR_STATUS_ACCURACY_HIGH;
    }
    data->pendingSensors = 0;



	for (i=0;i<MAX_LINUX_NUM_FDS;i++)
	{
		data->events_fd[i] =  dup(handle->data[i]);
		D("%s: dev=%p handle:%d, fd[%d]=%d, ctl_fd[%d]:%d", __FUNCTION__, dev, handle,i, data->events_fd[i],i, handle->data[i]);
	}
	native_handle_close(handle);
    native_handle_delete(handle);
    return 0;
}

static int data__data_close(struct sensors_data_device_t *dev)
{
    sensors_data_context_t*  data = (void*)dev;
    D("%s: dev=%p", __FUNCTION__, dev);
	int i;

	for (i=0;i<MAX_LINUX_NUM_FDS;i++)
	{
		if (data->events_fd[i] >= 0) 
		{
			close(data->events_fd[i]);
			data->events_fd[i] = -1;
		}
	}
	return 0;
}

static int pick_sensor(sensors_data_context_t*      data,
            sensors_data_t*  values)
{
    uint32_t mask = SUPPORTED_SENSORS;
    INFO("pending sensors 0x%08x", data->pendingSensors);
    while (mask) 
    {
        uint32_t i = 31 - __builtin_clz(mask);
        mask &= ~(1<<i);
        if (data->pendingSensors & (1<<i)) 
        {
            data->pendingSensors &= ~(1<<i);
            *values = data->sensors[i];
            values->sensor = id_to_sensor[i];
            return i;
        }
    }
    E("no sensor to return: pendingSensors = %08x", data->pendingSensors);
    return -1;
}

static void data__poll_process_syn(struct sensors_data_context_t *dev,
                                   struct input_event *event,
                                   uint32_t new_sensors)
{
    if (new_sensors) 
    {
        dev->pendingSensors |= new_sensors;
        int64_t t = event->time.tv_sec*1000000000LL +
            event->time.tv_usec*1000;
        while (new_sensors) 
        {
            uint32_t i = 31 - __builtin_clz(new_sensors);
            new_sensors &= ~(1<<i);
            dev->sensors[i].time = t;
        }
    }
}

#if PS
static uint32_t data__poll_process_proximity(struct sensors_data_context_t *dev,
                                           int fd __attribute__((unused)),
                                          struct input_event *event)
{
    uint32_t new_sensors = 0;
    if (event->type == EV_ABS) 
    {
        if (event->code == ABS_DISTANCE) 
        {
            new_sensors = SENSORS_PROXIMITY;
            INFO("Proximity Sensor Event : %d code",event->value);
            if (ps_distance_mode)
            {
				dev->sensors[ID_P].distance = ((float)event->value)/1000.0f;
			}
			else
			{
				dev->sensors[ID_P].distance = (float)event->value;
			}
        }
    }
    return new_sensors;
}
#endif

static uint32_t data__poll_process_light(struct sensors_data_context_t *dev,
                                           int fd __attribute__((unused)),
                                          struct input_event *event)
{
    uint32_t new_sensors = 0;
    if (event->type == EV_ABS) 
    {
        if (event->code == ABS_MISC) 
        {
			INFO("Light Sensor Event : %d lux",event->value);
            new_sensors = SENSORS_LIGHT;
            dev->sensors[ID_L].light = (float)event->value;
        }
    }
    return new_sensors;
}

static int data__poll(struct sensors_data_device_t *device, sensors_data_t* values)
{
	struct sensors_data_context_t *dev = (struct sensors_data_context_t *)device;
    struct timeval timeout;
    uint32_t new_sensors = 0;
#if PS
    int ps_fd = dev->events_fd[ID_PS_FD];
#endif    
    int als_fd = dev->events_fd[ID_LS_FD];
    timeout.tv_sec = 0; 

    // there are pending sensors, returns them now...
    if (dev->pendingSensors) 
    {
        return pick_sensor(dev, values);
    }
	INFO("ps_fd = %d, als_fd = %d",ps_fd,als_fd);
    
    
    while (1) 
    {
        /* read the next event; first, read the ambient light event, then the
           proximity event */
        struct input_event event;
        int got_syn = 0;
        int exit = 0;
        int nread;
        fd_set rfds_set;
        int n;
        FD_ZERO(&rfds_set);
        FD_SET(als_fd, &rfds_set);
#if PS        
        FD_SET(ps_fd, &rfds_set);
        // timeout.tv_usec = nDelayMicroSecond;
        n = select( __MAX(als_fd,ps_fd)+ 1, &rfds_set, NULL, NULL, NULL);
        //n = select( __MAX(als_fd,ps_fd)+ 1, &rfds, NULL, NULL, &timeout);
#else
		n = select(als_fd+ 1, &rfds_set, NULL, NULL, NULL);
#endif                           		
        if (FD_ISSET(als_fd, &rfds_set)) 
        {
            nread = read(als_fd, &event, sizeof(event));
            INFO("read(als_fd, &event, sizeof(event)=%d) = %d",nread,sizeof(event));
            if (nread == sizeof(event)) 
            {
                new_sensors |= data__poll_process_light(dev, als_fd, &event);
                got_syn = event.type == EV_SYN;
                exit = got_syn && event.code == SYN_CONFIG;
                if (got_syn) 
                {
                    INFO("ps syn %08x", new_sensors);
                    data__poll_process_syn(dev, &event, new_sensors);
                    new_sensors = 0;
				}
            }
            else E("als read too small %d", nread);
        }
#if PS		
        if (FD_ISSET(ps_fd, &rfds_set)) 
        {
            nread = read(ps_fd, &event, sizeof(event));
            INFO("read(ps_fd, &event, sizeof(event)=%d) = %d",nread,sizeof(event));
            if (nread == sizeof(event)) 
            {
                new_sensors |= data__poll_process_proximity(dev, ps_fd, &event);
                         
                got_syn = event.type == EV_SYN;
                exit = got_syn && event.code == SYN_CONFIG;
                if (got_syn) 
                {
                    INFO("ps syn %08x", new_sensors);
                    data__poll_process_syn(dev, &event, new_sensors);
                    new_sensors = 0;
				}
            }
            else E("ps read too small %d", nread);
        }
#endif        		
        if (exit) 
        {
            // we use SYN_CONFIG to signal that we need to exit the
            // main loop.
            INFO("exit");
            return 0x7FFFFFFF;
        }
        if (got_syn && dev->pendingSensors) 
        {
            INFO("got syn, picking sensor");
            return pick_sensor(dev, values);
        }
    }
}

static int data__close(struct hw_device_t *dev) 
{
	
    sensors_data_context_t* data = (sensors_data_context_t*)dev;
	int i;
	if (data) 
	{
		for (i=0;i<MAX_LINUX_NUM_FDS;i++)
		{
			if (data->events_fd[i] > 0) {
				D("%s: dev=%p, fd[%d]:%d", __FUNCTION__, dev,i , data->events_fd[i]);
				close(data->events_fd[i]);
				data->events_fd[i] = -1;
			}
		}
		free(data);
	}
    return 0;
}

#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS
static int sensors__get_sensors_list(struct sensors_module_t* module,struct sensor_t const** list) 
{
    int n,fd;
	uint32_t ps_distance_range,als_lux_range;
	int rt = 0;
	
	n = ARRAY_SIZE(sSensorList);
	D("%s: returned %d sensors (mask=%d)", __FUNCTION__, n);
#if PS	
	fd = open(ps_distance_mode_bin_path,O_RDONLY);
	rt = read(fd,&ps_distance_mode,1);
	if(rt < 0)
		E("%s: open fd error, path:%s\n", __FUNCTION__, ps_distance_mode_bin_path);
	close(fd);

	fd = open(ps_distance_range_bin_path,O_RDONLY);
	rt = read(fd,&ps_distance_range,4);
	if(rt < 0)
		E("%s: open fd error, path:%s\n", __FUNCTION__, ps_distance_range_bin_path);
	close(fd);
	if (ps_distance_mode)
	{
		sSensorList[ID_P].maxRange = (float)ps_distance_range/1000.0f;
		sSensorList[ID_P].resolution = 0.001f;
	}
	else
	{
		sSensorList[ID_P].maxRange = (float)ps_distance_range;
		sSensorList[ID_P].resolution =1.0f;
	}
#endif	
	fd = open(als_lux_range_bin_path,O_RDONLY);
	rt = read(fd,&als_lux_range,4);
	if(rt < 0)
		E("%s: open fd error, path:%s\n", __FUNCTION__, als_lux_range_bin_path);	
	close(fd);
	
	sSensorList[ID_L].maxRange = (float)als_lux_range;
	sSensorList[ID_L].resolution = 1.0f;
	
    *list = sSensorList;
    return n;
}
#else
static int sensors__get_sensors_list(struct sensors_module_t* module,struct sensor_t const** list) 
{
    int n,fd;
	uint32_t ps_distance_range,als_lux_range;
	const int buf_size=20;
	char buffer[buf_size+1];
	int rt;
	n = ARRAY_SIZE(sSensorList);
	D("%s: returned %d sensors (mask=%d)", __FUNCTION__, n);
#if PS	
	fd = open(ps_distance_mode_path,O_RDONLY);
	memset(buffer,0,buf_size+1);
	rt = read(fd,buffer,buf_size);
	if(rt < 0)
		E("%s: open fd error, path:%s\n", __FUNCTION__, ps_distance_mode_path);		
	close(fd);
	ps_distance_mode = atoi(buffer)?1:0;
	fd = open(ps_distance_range_path,O_RDONLY);
	memset(buffer,0,buf_size+1);
	rt = read(fd,buffer,buf_size);
	if(rt < 0)
		E("%s: open fd error, path:%s\n", __FUNCTION__, ps_distance_range_path);		
	ps_distance_range = atoi(buffer);
	close(fd);
	if (ps_distance_mode)
	{
		sSensorList[ID_P].maxRange = (float)ps_distance_range/1000.0f;
		sSensorList[ID_P].resolution = 0.001f;
	}
	else
	{
		sSensorList[ID_P].maxRange = (float)ps_distance_range;
		sSensorList[ID_P].resolution =1.0f;
	}
#endif	
	fd = open(als_lux_range_path,O_RDONLY);
	memset(buffer,0,buf_size+1);
	rt = read(fd,buffer,buf_size);	
	if(rt < 0)
		E("%s: open fd error, path:%s\n", __FUNCTION__, als_lux_range_path);		
	als_lux_range = atoi(buffer);
	close(fd);
	
	sSensorList[ID_L].maxRange = (float)als_lux_range;
	sSensorList[ID_L].resolution = 1.0f;
	
    *list = sSensorList;
    return n;
}


#endif //STK_ENABLE_SENSOR_USE_BINARY_SYSFS

static int open_sensors(const struct hw_module_t* module,const char* name,struct hw_device_t* *device)
{
    int	status = -EINVAL;
	int	i;
    D("%s: name=%s", __FUNCTION__, name);

    if (!strcmp(name, SENSORS_HARDWARE_CONTROL))
    {
        sensors_control_context_t *dev = malloc(sizeof(*dev));

        memset(dev, 0, sizeof(*dev));

        dev->device.common.tag       = HARDWARE_DEVICE_TAG;
        dev->device.common.version   = 0;
        dev->device.common.module    = (struct hw_module_t*) module;
        dev->device.common.close     = control__close;
        dev->device.open_data_source = control__open_data_source;
        dev->device.activate         = control__activate;
        dev->device.set_delay        = control__set_delay;
        dev->device.wake             = control__wake;

        *device = &dev->device.common;
        status  = 0;
    }
    else if (!strcmp(name, SENSORS_HARDWARE_DATA)) {
        sensors_data_context_t *dev = malloc(sizeof(*dev));

        memset(dev, 0, sizeof(*dev));

        dev->device.common.tag     = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module  = (struct hw_module_t*) module;
        dev->device.common.close   = data__close;
        dev->device.data_open      = data__data_open;
        dev->device.data_close     = data__data_close;
        dev->device.poll           = data__poll;
        for (i=0;i<MAX_LINUX_NUM_FDS;i++)
			dev->events_fd[i]             = -1;

        *device = &dev->device.common;
        status  = 0;
    }
    return status;
}
