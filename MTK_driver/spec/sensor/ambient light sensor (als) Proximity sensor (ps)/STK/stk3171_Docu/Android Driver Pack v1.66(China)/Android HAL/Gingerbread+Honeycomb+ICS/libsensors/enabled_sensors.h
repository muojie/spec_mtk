
/*
 * SenseTek/Sitronix Corp.
 * v1.0
 * by Patrick Chang 2011/03/17
 */


#ifndef ENABLED_SENSORS_H
#define ENABLED_SENSORS_H

/* if you are using AKM 3 in 1 senosr, set AKM_3IN1_SENSOR = 1 */
#define AKM_3IN1_SENSOR 0
#define INVENSENSE_3IN1_SENSOR 0
#define YAMAHA_3IN1_SENSOR 0


#if ((INVENSENSE_3IN1_SENSOR&&AKM_3IN1_SENSOR) || (AKM_3IN1_SENSOR&&YAMAHA_3IN1_SENSOR) || (INVENSENSE_3IN1_SENSOR&&YAMAHA_3IN1_SENSOR))
#error "This HAL does NOT support AKM_3IN1_SENSOR/INVENSENSE_3IN1_SENSOR/YAMAHA_3IN1_SENSOR at the same time"
#endif


#define AS 0	/*Accelerometer*/
#define MS 0	/*Magnetic Sensor*/
#define OS 0	/*Orientation Sensor*/
#define TS 0	/*Temperature Sensor*/
#define PS 1	/*Proximity Sensor*/
#define LS 1	/*Light Sensor*/
#define GS 0	/*Gyroscope*/
#define PR 0	/*Pressure Sensor*/
#define GR 0    /*Gravity*/
#define LA 0    /*Linear Acceleration*/
#define RV 0    /*Rotation Vector*/

#if AS
#define IS_STK8311	
//#define IS_STK8312	
#endif

#if AKM_3IN1_SENSOR
// Hardware : G-Sensor + e-Compass + Temperature Sensor
// Software --> external Orientation (G-Sensor + e-Compass)
#undef AS
#undef MS
#undef OS
#define AS 1
#define MS 1
#define OS 1
#endif

#if INVENSENSE_3IN1_SENSOR
// Hardware : G-Sensor + Gyroscope + e-Compass
// Software --> external Gravity (G-sensor w/ LPF)+ Linear Acceleration (G-Sensor w/ HPF)
//              + Rotation Vector (Gyroscope + G-Sensor) + Orientation (G-Sensor + e-Compass)
#undef AS
#undef MS
#undef OS
#undef GS
#undef GR
#undef LA
#undef RV
#undef PR
#define AS 1
#define MS 1
#define OS 1
#define GS 1
#define GR 1
#define LA 1
#define RV 1
#define PR 1
#endif

#if YAMAHA_3IN1_SENSOR
// Hardware : G-Sensor + e-Compass + Temperature Sensor
// Software --> external Orientation (G-Sensor + e-Compass)
#undef AS
#undef MS
#undef OS
#define AS 1
#define AS 1
#define MS 1
#define OS 1
#endif
/*****************************************************************************/
#define ID_BASE 0
#define ID_A  (ID_BASE)
#define ID_M  (ID_BASE+AS)
#define ID_O  (ID_BASE+AS+MS)
#define ID_T  (ID_BASE+AS+MS+OS)
#define ID_P  (ID_BASE+AS+MS+OS+TS)
#define ID_L  (ID_BASE+AS+MS+OS+TS+PS)
#define ID_G  (ID_BASE+AS+MS+OS+TS+PS+LS)
#define ID_PR  (ID_BASE+AS+MS+OS+TS+PS+LS+GS)
#define ID_GR  (ID_BASE+AS+MS+OS+TS+PS+LS+GS+PR)
#define ID_LA  (ID_BASE+AS+MS+OS+TS+PS+LS+GS+PR+GR)
#define ID_RV  (ID_BASE+AS+MS+OS+TS+PS+LS+GS+PR+GR+LA)
/*****************************************************************************/
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
#define SENSORS_GYRO			(1 << ID_G)
#else
#define SENSORS_GYRO			0
#endif

#if PR
#define SENSORS_PRESSURE		(1 << ID_PR)
#else
#define SENSORS_PRESSURE		0
#endif

#if GR
#define SENSORS_GRAVITY         (1 << ID_GR)
#else
#define SENSORS_GRAVITY         0
#endif

#if LA
#define SENSORS_LINEAR_ACCEL    (1 << ID_LA)
#else
#define SENSORS_LINEAR_ACCEL    0
#endif

#if RV
#define SENSORS_ROTATION_VECTOR (1 << ID_RV)
#else
#define SENSORS_ROTATION_VECTOR 0
#endif


#define SUPPORTED_SENSORS  (SENSORS_ACCELERATION\
                            |SENSORS_MAGNETIC_FIELD|SENSORS_ORIENTATION\
                            |SENSORS_TEMPERATURE|SENSORS_PROXIMITY\
                            |SENSORS_LIGHT|SENSORS_GYRO\
                            |SENSORS_PRESSURE|SENSORS_GRAVITY\
                            |SENSORS_LINEAR_ACCEL|SENSORS_ROTATION_VECTOR)
/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#endif  // ENABLED_SENSORS_H
