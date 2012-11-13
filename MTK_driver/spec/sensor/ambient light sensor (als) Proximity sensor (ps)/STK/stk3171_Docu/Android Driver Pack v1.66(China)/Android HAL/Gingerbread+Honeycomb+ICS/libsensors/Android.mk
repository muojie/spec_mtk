# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_SIMULATOR),true)

# HAL module implemenation, not prelinked, and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)
LOCAL_MODULE := sensors.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_MODULE_TAGS := eng
#LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"
LOCAL_SRC_FILES := 						\
				sensors.c  			\
				sensors_poll_impl.cpp 	\
				InputEventReader.cpp	\
				SensorBase.cpp			\
				LightSensor.cpp			\
				GyroSensor.cpp          \
				ProximitySensorBinaryMode.cpp	\
				ProximitySensorDistanceMode.cpp \
				AkmSensor.cpp	\
				Accelerometer.cpp \
				YamahaSensor.cpp \
				YamahaSensorA.cpp \
				YamahaSensorM.cpp \
				YamahaSensorO.cpp \
				Invensense3in1Sensor.cpp

# < --- invensense 3 in 1 ---
# for Invense3in1, you need to include extra header files
#
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/mlsdk/platform/include
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/mlsdk/platform/linux
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/mlsdk/mllite
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/mlsdk/mldmp

#LOCAL_PREBUILT_LIBS :=  libmpl.so libmllite.so libmlplatform.so
#LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libdl libmpl libmllite libmlplatform
#LOCAL_CPPFLAGS+=-DLINUX=1
#LOCAL_PRELINK_MODULE := false
#include $(BUILD_MULTI_PREBUILT)
#include $(BUILD_SHARED_LIBRARY)
# --- invensense 3 in 1 --- >

# < --- SenseTek ---
LOCAL_SHARED_LIBRARIES := liblog libcutils libc
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
# --- SenseTek --- >

endif # !TARGET_SIMULATOR
