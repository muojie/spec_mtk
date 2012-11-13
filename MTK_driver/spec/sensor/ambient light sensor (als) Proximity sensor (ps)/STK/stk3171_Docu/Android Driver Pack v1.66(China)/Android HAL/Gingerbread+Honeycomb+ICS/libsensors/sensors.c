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
#include "enabled_sensors.h"
#include "sensors_poll_impl.h"
#include "stk_defines.h"
#include <stdlib.h>
#include <cutils/log.h>
#include <memory.h>
/*****************************************************************************/

/*
 * The SENSORS Module
 */


static struct sensor_t sSensorList[] = {

#if AKM_3IN1_SENSOR
        {
            .name       = "BMA150 3-axis Accelerometer",
            .vendor     = "Bosh",
            .version    = 1,
            .handle     = ID_A,
            .type       = SENSOR_TYPE_ACCELEROMETER,
            .maxRange   = 4.0f*9.81f,
            .resolution = (4.0f*9.81f)/256.0f,
            .power      = 0.2f,
            .minDelay   = 0,
            .reserved   = {}
        },
        {
            .name       = "AK8973 3-axis Magnetic field sensor",
            .vendor     = "Asahi Kasei",
            .version    = 1,
            .handle     = ID_M,
            .type       = SENSOR_TYPE_MAGNETIC_FIELD,
            .maxRange   = 2000.0f,
            .resolution = 1.0f/16.0f,
            .power      = 6.8f,
            .minDelay   = 0,
            .reserved   = {}
        },
        {
            .name       = "AK8973 Orientation sensor",
            .vendor     = "Asahi Kasei",
            .version    = 1,
            .handle     = ID_O,
            .type       = SENSOR_TYPE_ORIENTATION,
            .maxRange   = 360.0f,
            .resolution = 1.0f,
            .power      = 7.0f,
            .minDelay   = 0,
            .reserved   = {}
        },
#elif INVENSENSE_3IN1_SENSOR
        {
            .name       = "MPL accel",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_A,
            .type       = SENSOR_TYPE_ACCELEROMETER,
            .maxRange   = 10240.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
        {
            .name       = "MPL magnetic field",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_M,
            .type       = SENSOR_TYPE_MAGNETIC_FIELD,
            .maxRange   = 10240.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
        {
            .name       = "MPL Orientation",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_O,
            .type       = SENSOR_TYPE_ORIENTATION,
            .maxRange   = 360.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
        {
            .name       = "MPL Gryoscope",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_G,
            .type       = SENSOR_TYPE_GYROSCOPE,
            .maxRange   = 10240.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
        {
            .name       = "MPL rotation vector",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_RV,
            .type       = SENSOR_TYPE_ROTATION_VECTOR,
            .maxRange   = 10240.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
        {
            .name       = "MPL linear accel",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_LA,
            .type       = SENSOR_TYPE_LINEAR_ACCELERATION,
            .maxRange   = 10240.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
        {
            .name       = "MPL gravity",
            .vendor     = "Invensense",
            .version    = 1,
            .handle     = ID_GR,
            .type       = SENSOR_TYPE_GRAVITY,
            .maxRange   = 10240.0f,
            .resolution = 1.0f,
            .power      = 0.5f,
            .minDelay   = 20000,
            .reserved   = {}
        },
#elif YAMAHA_3IN1_SENSOR
        {
            .name       = "Vendor's Accelerometer",
            .vendor     = "Yamaha Corporation",
            .version    = 1,
            .handle     = ID_A,
            .type       = SENSOR_TYPE_ACCELEROMETER,
            .maxRange   = 4.0f*9.81f,
            .resolution = (4.0f*9.81f)/256.0f,
            .power      = 0.2f,
            .minDelay   = 10000,
            .reserved   = {}
        },
#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS529
        {
            .name       = "MS-3C (YAS529) Magnetic Sensor",
            .vendor     = "Yamaha Corporation",
            .version    = 1,
            .handle     = ID_M,
            .type       = SENSOR_TYPE_MAGNETIC_FIELD,
            .maxRange   = 800.0f,
            .resolution = 1.0f,
            .power      = 4.0f,
            .minDelay   = 20000,
            .reserved   = {}
        },
#elif YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS530
        {
            .name       = "MS-3E (YAS529) Magnetic Sensor",
            .vendor     = "Yamaha Corporation",
            .version    = 1,
            .handle     = ID_M,
            .type       = SENSOR_TYPE_MAGNETIC_FIELD,
            .maxRange   = 300.0f,
            .resolution = 1.0f,
            .power      = 4.0f,
            .minDelay   = 10000,
            .reserved   = {}
        },

#endif
        {
            .name       = "MS-x Orientation Sensor",
            .vendor     = "Yamaha Corporation",
            .version    = 1,
            .handle     = ID_O,
            .type       = SENSOR_TYPE_ORIENTATION,
            .maxRange   = 360.0f,
            .resolution = 1.0f,
            .power      = 7.0f,
            .minDelay   = 10000,
            .reserved   = {}
        },
#endif //AKM_3IN1_SENSOR  or INVENSENSE_3IN1_SENSOR or YAMAHA_3IN1_SENSOR

#if (AS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
#ifdef IS_STK8311
    {
        .name       = "stk8311",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_A,
        .type       = SENSOR_TYPE_ACCELEROMETER,
        .maxRange   = 8.0f*9.81f,
        .resolution = 9.81f/256.0f,
        .power      = 0.155f,
        .minDelay   = 2500,
        .reserved   = {}
    },	 
#endif	 
#ifdef IS_STK8312
    {
        .name       = "stk8312",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_A,
        .type       = SENSOR_TYPE_ACCELEROMETER,
        .maxRange   = 6.0f*9.81f,
        .resolution = 9.81f/21.34f,
        .power      = 0.155f,
        .minDelay   = 2500,
        .reserved   = {}
    },
#endif	 
#endif //AS
#if (MS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
    {
        .name       = "SenseTek/Yamaha M-Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_M,
        .type       = SENSOR_TYPE_MAGNETIC_FIELD,
        .maxRange   = 2000.0f,
        .resolution = 1.0f/16.0f,
        .power      = 0.1f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //MS
#if (OS && (!(AKM_3IN1_SENSOR||INVENSENSE_3IN1_SENSOR||YAMAHA_3IN1_SENSOR)))
    {
        .name       = "SenseTek Orientation Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_O,
        .type       = SENSOR_TYPE_ORIENTATION,
        .maxRange   = 360.0f,
        .resolution = 1.0f,
        .power      = 0.0f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif // OS

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
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //TS
#if PS
	{
        .name       = "SenseTek Proximity Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_P,
        .type       = SENSOR_TYPE_PROXIMITY,
        .maxRange   = 10.0f,
        .resolution = 0.1f,
        .power      = 0.2f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //PS
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
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //LS
#if (GS && (!INVENSENSE_3IN1_SENSOR))
	{
        .name       = "SenseTek/STM K3G Gryoscope",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_G,
        .type       = SENSOR_TYPE_GYROSCOPE,
        .maxRange   = (2000.0f*(float)M_PI/180.0f),
        .resolution = ((70.0f / 1000.0f) * ((float)M_PI / 180.0f)),
        .power      = 6.1f,
        .minDelay   = 1200,
        .reserved   = {}
    },
#endif //GS
#if PR
	{
        .name       = "SenseTek Pressure Sensor",
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_PR,
        .type       = SENSOR_TYPE_PRESSURE,
        .maxRange   = 4096.0f,
        .resolution = 1.0f,
        .power      = 0.083f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //PR

#if (GR && (!INVENSENSE_3IN1_SENSOR))
	{
        .name       = "SenseTek Gravity", // software implement
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_GR,
        .type       = SENSOR_TYPE_GRAVITY,
        .maxRange   = 8.0f/2048.0f*9.81f,
        .resolution = 1.0f,
        .power      = 0.1f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //GR
#if (LV && (!INVENSENSE_3IN1_SENSOR))
	{
        .name       = "SenseTek Linear Acceleration", // software implement
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_LA,
        .type       = SENSOR_TYPE_LINEAR_ACCELERATION,
        .maxRange   = 8.0f/2048.0f*9.81f,
        .resolution = 1.0f,
        .power      = 0.1f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //LV

#if (RV && (!INVENSENSE_3IN1_SENSOR))
	{
        .name       = "SenseTek Rotation Vector", // software implement
        .vendor     = "SenseTek/Sitronix",
        .version    = 1,
        .handle     = ID_RV,
        .type       = SENSOR_TYPE_ROTATION_VECTOR,
        .maxRange   = 10240.0f,
        .resolution = 1.0f,
        .power      = 0.1f,
        .minDelay   = 0,
        .reserved   = {}
    },
#endif //RV

};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);


static int get_sensor_index_in_list(int ID)
{
    unsigned int i = 0;
    for (i= 0;i<ARRAY_SIZE(sSensorList);i++)
    {
        if (sSensorList[i].handle == ID)
            return i;
    }
    return -EINVAL;
}
#if STK_ENABLE_SENSOR_USE_BINARY_SYSFS
static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
#if (PS||LS)
    int fd;
    int min_delay;
    int index;
    int64_t delay_ns;
#endif
#if LS
    uint32_t als_lux_range;
#endif
#if PS
    uint8_t ps_distance_mode = 0;
    uint32_t ps_distance_range;
#endif
    *list = sSensorList;
#if PS

	fd = open(ps_distance_mode_bin_path,O_RDONLY);
	read(fd,&ps_distance_mode,1);
	close(fd);

	fd = open(ps_distance_range_bin_path,O_RDONLY);
	read(fd,&ps_distance_range,4);
	close(fd);
	index = get_sensor_index_in_list(ID_P);
	if (ps_distance_mode)
	{
		sSensorList[index].maxRange = (float)ps_distance_range/1000.0f;
		sSensorList[index].resolution = 0.001f;
	}
	else
	{
		sSensorList[index].maxRange = (float)ps_distance_range;
		sSensorList[index].resolution =1.0f;
	}
#endif

#if LS

    fd = open(als_lux_range_bin_path,O_RDONLY);
    read(fd,&als_lux_range,4);
    close(fd);
	index = get_sensor_index_in_list(ID_L);
    sSensorList[index].maxRange = (float)als_lux_range;
#endif

    return ARRAY_SIZE(sSensorList);
}
#else
static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
#if (PS||LS)
    int fd;
    int min_delay;
    int index;
    int64_t delay_ns;
    const int buf_size = 20;
    char buffer[buf_size+1];
#endif
#if LS
    uint32_t als_lux_range;
#endif
#if PS
    uint8_t ps_distance_mode = 0;
    uint32_t ps_distance_range;
#endif

    *list = sSensorList;
#if PS

	fd = open(ps_distance_mode_path,O_RDONLY);
	LOGE_IF(fd<0,"failed to open %s",ps_distance_mode_path);
	if (fd>=0)
	{
	    memset(buffer,0,buf_size+1);
        read(fd,buffer,buf_size);
        close(fd);
        ps_distance_mode = atoi(buffer)?1:0;
    }
	fd = open(ps_distance_range_path,O_RDONLY);
	LOGE_IF(fd<0,"failed to open %s",ps_distance_range_path);
	if (fd>=0)
	{
	    memset(buffer,0,buf_size+1);
        read(fd,buffer,buf_size);
        close(fd);
        ps_distance_range = (uint32_t)atoi(buffer);
        index = get_sensor_index_in_list(ID_P);
        if (ps_distance_mode)
        {
            sSensorList[index].maxRange = (float)ps_distance_range/1000.0f;
            sSensorList[index].resolution = 0.001f;
        }
        else
        {
            sSensorList[index].maxRange = (float)ps_distance_range;
            sSensorList[index].resolution =1.0f;
        }
	}

#endif

#if LS
    fd = open(als_lux_range_path,O_RDONLY);
    LOGE_IF(fd<0,"failed to open %s",als_lux_range_path);
    if (fd>=0)
    {
        memset(buffer,0,buf_size+1);
        read(fd,buffer,buf_size);
        close(fd);
        als_lux_range = (uint32_t)atoi(buffer);
        index = get_sensor_index_in_list(ID_L);
        sSensorList[index].maxRange = (float)als_lux_range;
    }
#endif

    return ARRAY_SIZE(sSensorList);
}
#endif //STK_ENABLE_SENSOR_USE_BINARY_SYSFS

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

const struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "SenseTek Sensors Templeate Module",
        .author = "Patrick Chang / SenseTek / Sitronix Corp.",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};

/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    return init_sensors_poll_impl(module,device);
}
