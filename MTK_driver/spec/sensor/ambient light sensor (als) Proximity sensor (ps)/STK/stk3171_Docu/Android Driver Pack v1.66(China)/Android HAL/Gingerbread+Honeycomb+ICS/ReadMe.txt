libsensors/enabled_sensors.h			-->	Enable/Disable Sensor
libsensors/AkmSensor.h				-->	Header for AKM Sensor 
libsensors/AkmSensor.cpp			-->	AKM Sensor(eCompass+g-sensor+Orientation)
libsensors/Accelerometer.cpp			-->	SenseTek acelerometer HAL header
libsensors/Accelerometer.h			-->	SenseTek acelerometer HAL
libsensors/GyroSensor.h				-->	STM Gryosensor header(from Samsung crespo)
libsensors/GyroSensor.cpp			-->	STM Gryosensor(from Samsung crespo)
libsensors/InputEventReader.h			-->	Input Reader header (utilities)
libsensors/InputEventReader.cpp			-->	Input Reader implementation (utilities)
libsensors/Invensense3in1Sensor.h		-->	Invensense 3 in 1 sensor header (gs+gyro+eCompass)
libsensors/Invensense3in1Sensor.cpp		-->	Invensense 3 in 1 sensor  (need extra file from Invensense)
libsensors/LightSensor.h			-->	SenseTek Light Sensor HAL header
libsensors/LightSensor.cpp			-->	SenseTek Light Sensor HAL implementation
libsensors/ProximitySensorBinaryMode.h		-->	SenseTek Proximity Sensor HAL header (binary mode)
libsensors/ProximitySensorBinaryMode.cpp	-->	SenseTek Proximity Sensor HAL implementation (binary mode)
libsensors/ProximitySensorDistanceMode.h	-->	SenseTek Proximity Sensor HAL header (distance mode)
libsensors/ProximitySensorDistanceMode.cpp	-->	SenseTek Proximity Sensor HAL implementation (distance mode)
libsensors/SensorBase.h				-->	Provide abstract class/method for all sensors
libsensors/SensorBase.cpp			-->	Implements some common methods
libsensors/sensors.c				-->	Implementations of hardware module information
libsensors/YamahaSensor.h			-->	Header file for Yamaha's Sensor 
libsensors/YamahaSensorA.h			-->	Header file for Yamaha's Accelerometer
libsensors/YamahaSensorM.h			-->	Header file for Yamaha's M-Sensor
libsensors/YamahaSensorO.h			-->	Header file for Yamaha's Orientation Sensor
libsensors/YamahaSensor.cpp			-->	Yamaha's Sensor 
libsensors/YamahaSensorA.cpp			-->	Yamaha's Accelerometer
libsensors/YamahaSensorM.cpp			-->	Yamaha's M-Sensor
libsensors/YamahaSensorO.cpp			-->	Yamaha's Orientation Sensor
libsensors/sensors_poll_impl.h			-->	Implements for nuSensor
libsensors/sensors_poll_impl.cpp		-->	Implements for nuSensor
libsensors/stk_defines.h			-->	SenseTek basic definitions
sensors.stk_gh(only als).so			-->	Pre-compiled binary sensor HAL(ARMv7,for Gingerbread/Honeycomb)
sensors.stk_gh(ps+als).so			-->	Pre-compiled binary sensor HAL(ARMv7,for Gingerbread/Honeycomb)
ReadMe.txt					-->	This file
