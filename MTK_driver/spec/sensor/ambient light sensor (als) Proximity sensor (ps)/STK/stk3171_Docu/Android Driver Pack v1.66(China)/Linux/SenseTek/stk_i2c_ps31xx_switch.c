/*
 *  stk_i2c_ps31xx_switch.c - Linux kernel modules for proximity/ambient light sensor
 *  (Polling + Switch Mode)
 *
 *  Copyright (C) 2011 Patrick Chang / SenseTek <patrick_chang@sitronix.com.tw>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/errno.h>

#include "stk_i2c_ps31xx.h"
#include "stk_defines.h"
#include "stk_lk_defs.h"

#define STKALS_DRV_NAME	"stk_als"
#define STKPS_DRV_NAME "stk_ps"
#define DEVICE_NAME		"stk-oss"
#define DRIVER_VERSION  STK_DRIVER_VER
#define LightSensorDevName "stk_als"
#define ProximitySensorDevName "stk_ps"

#define ALS_ODR_DELAY (1000/CONFIG_STK_ALS_SAMPLING_RATE)
#define PS_ODR_DELAY (1000/CONFIG_STK_PS_SAMPLING_RATE)

#define stk_writeb i2c_smbus_write_byte_data
#define stk_readb i2c_smbus_read_byte_data

#define stk_writew i2c_smbus_write_word_data
#define stk_readw i2c_smbus_read_word_data

#define STK_LOCK0 mutex_unlock(&stkps_io_lock)
#define STK_LOCK1 mutex_lock(&stkps_io_lock)

static int32_t init_all_setting(void);
static int32_t enable_ps(uint8_t enable);
static int32_t enable_als(uint8_t enable);
static int32_t software_reset(void);

static int32_t set_als_it(uint8_t it);
static int32_t set_als_gain(uint8_t gain);
static int32_t set_ps_it(uint8_t it);
static int32_t set_ps_slp(uint8_t slp);
static int32_t set_ps_led_driving_current(uint8_t irdr);
static int32_t set_ps_gc(uint8_t gc);

static int32_t set_ps_thd_l(uint8_t thd_l);
static int32_t set_ps_thd_h(uint8_t thd_h);

static int ps_polling_function(void* arg);
static int als_polling_function(void* arg);
static struct task_struct *ps_polling_tsk=NULL;
static struct task_struct *als_polling_tsk=NULL;

static struct mutex stkps_io_lock;
static struct completion ps_thread_completion;
static struct completion als_thread_completion;
static struct stkps31xx_data* pStkPsData = NULL;
static struct wake_lock proximity_sensor_wakelock;

#ifdef CONFIG_STK_PS_ENGINEER_TUNING
static uint8_t ps_code_low_thd;
static uint8_t ps_code_high_thd;
#endif

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING

static int32_t als_transmittance = CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING

inline uint32_t alscode2lux(uint32_t alscode)
{
   alscode += ((alscode<<7)+(alscode<<3)+(alscode>>1));     // 137.5
      //x1       //   x128         x8            x0.5
    alscode<<=3; // x 8 (software extend to 19 bits)
    // Gain & IT setting ==> x8
    // ==> i.e. code x 8800
    // Org : 1 code = 0.88 Lux
    // 8800 code = 0.88 lux --> this means it must be * 1/10000


#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    alscode/=als_transmittance;
#else
    alscode/=CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    return alscode;
}

static int32_t init_all_setting(void)
{
    if (software_reset()<0)
    {
        ERR("STK PS : error --> device not found\n");
        return 0;
    }
    enable_ps(0);
    enable_als(0);
    set_ps_slp(STK_PS_SLEEP_TIME);
    set_ps_gc(CONFIG_STK_PS_GAIN_SETTING);
    set_ps_it(STK_PS_INTEGRAL_TIME);
    set_ps_led_driving_current(STK_PS_IRLED_DRIVING_CURRENT);
    set_als_gain(0x01); // x2
    set_als_it(0x02); // x4
    set_ps_thd_h(CONFIG_STK_PS_CODE_HIGH_THRESHOLD);
    set_ps_thd_l(CONFIG_STK_PS_CODE_LOW_THRESHOLD);

    return 1;
}

static int32_t software_reset(void)
{
    // software reset and check stk 83xx is valid
    int32_t r;
    uint8_t w_reg;
    uint8_t org_reg;

    r = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_STATUS_REG);
    if (r<0)
    {
        ERR("STK PS software reset: read i2c error\n");
        return r;
    }
    if ((r&STK_PS_STATUS_ID_MASK)!=STK_PS31xx_ID)
    {
        ERR("STK PS : invalid ID number");
        return -EINVAL;
    }
    r = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_GC_REG);
    if (r<0)
    {
        ERR("STK PS software reset: read i2c error\n");
        return r;
    }
    org_reg = (uint8_t)(r&0xf0);
    w_reg = ~((uint8_t)(r&0xff));
    r = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_GC_REG,w_reg);
    if (r<0)
    {
        ERR("STK PS software reset: write i2c error\n");
        return r;
    }
    r = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_GC_REG);
    if (w_reg!=(uint8_t)(r&0xff))
    {
        ERR("STK PS software reset: read-back value is not  the same\n");
        return -1;
    }
    r = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_SOFTWARE_RESET_REG,0);
    msleep(5);
    if (r<0)
    {
        ERR("STK PS software reset: read error after reset\n");
        return r;
    }
    return 0;
}



static int32_t set_als_it(uint8_t it)
{
    pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_IT_MASK);
    pStkPsData->als_cmd_reg |= (STK_ALS_CMD_IT_MASK & STK_ALS_CMD_IT(it));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_ALS_CMD_REG,pStkPsData->als_cmd_reg);

}
static int32_t set_als_gain(uint8_t gain)
{
	if(gain >= 2)
	{
		ERR("STK PS : Error! als_gain >= 2\n");
		return -EINVAL;
	}
    pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_GAIN_MASK);
    pStkPsData->als_cmd_reg |= (STK_ALS_CMD_GAIN_MASK & STK_ALS_CMD_GAIN(gain));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_ALS_CMD_REG,pStkPsData->als_cmd_reg);
}
static int32_t set_ps_it(uint8_t it)
{
    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_IT_MASK);
    pStkPsData->ps_cmd_reg |= (STK_PS_CMD_IT_MASK & STK_PS_CMD_IT(it));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);
}
static int32_t set_ps_slp(uint8_t slp)
{
    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SLP_MASK);
    pStkPsData->ps_cmd_reg |= (STK_PS_CMD_SLP_MASK & STK_PS_CMD_SLP(slp));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);

}
static int32_t set_ps_led_driving_current(uint8_t irdr)
{
    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_DR_MASK);
    pStkPsData->ps_cmd_reg |= (STK_PS_CMD_DR_MASK & STK_PS_CMD_DR(irdr));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);
}
static int32_t set_ps_gc(uint8_t gc)
{
    int32_t retval;

    retval = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_GC_REG);
    if (retval<0)
        return retval;
    pStkPsData->ps_gc_reg = (uint8_t)retval;
    pStkPsData->ps_gc_reg &= (~STK_PS_GC_GAIN_MASK);
    pStkPsData->ps_gc_reg |= (STK_PS_GC_GAIN(gc)&STK_PS_GC_GAIN_MASK);

    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_GC_REG,pStkPsData->ps_gc_reg);
}


static int32_t set_ps_thd_l(uint8_t thd_l)
{
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
    ps_code_low_thd = thd_l;
#endif
  return 0;
}
static int32_t set_ps_thd_h(uint8_t thd_h)
{
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
    ps_code_high_thd = thd_h;
#endif
    return 0;
}

inline int32_t get_als_reading(void)
{
    int32_t word_data;
    int32_t lsb,msb;
    msb = i2c_smbus_read_byte_data(pStkPsData->client,STK_ALS_DT1_REG);
    lsb = i2c_smbus_read_byte_data(pStkPsData->client,STK_ALS_DT2_REG);
    word_data = (msb<<8) | lsb;
    return word_data;
}

inline int32_t get_ps_reading(void)
{
    return i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_READING_REG);
}

inline void als_report_event(struct input_dev* dev,int32_t report_value)
{
	input_report_abs(dev, ABS_MISC, report_value);
	input_sync(dev);
	INFO("STK PS : als input event %d lux\n",report_value);
}

inline void update_and_check_report_als(int32_t als_code)
{
    int32_t lux_last,lux;
    lux_last = pStkPsData->als_lux_last;
    lux = alscode2lux(als_code);
    if (unlikely(abs(lux - lux_last)>=CONFIG_STK_ALS_CHANGE_THRESHOLD))
    {
        pStkPsData->als_lux_last = lux;
        als_report_event(pStkPsData->als_input_dev,lux);
    }
}

static int als_polling_function(void* arg)
{
    uint32_t delay;
    init_completion(&als_thread_completion);
    while (1)
    {

        STK_LOCK(1);
        delay = pStkPsData->als_delay;
        pStkPsData->als_reading = get_als_reading();
        update_and_check_report_als(pStkPsData->als_reading);
        if (pStkPsData->bALSThreadRunning == 0)
            break;
        STK_LOCK(0);
        msleep(delay);
    };

    STK_LOCK(0);
    complete(&als_thread_completion);

    return 0;
}


inline void ps_report_event(struct input_dev* dev,int32_t report_value)
{
    pStkPsData->ps_distance_last = report_value;
	input_report_abs(dev, ABS_DISTANCE, report_value);
	input_sync(dev);
	INFO("STK PS : ps input event %d\n",report_value);
	wake_lock_timeout(&proximity_sensor_wakelock, 2*HZ);
}


inline void update_and_check_report_ps(int32_t ps_code)
{
    int32_t ps_code_last;

    ps_code_last = pStkPsData->ps_code_last;
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
	//INFO("STK PS : ps_code=%d\n",ps_code);
	if (ps_code > ps_code_high_thd)
    {
        if (unlikely(ps_code_last < ps_code_high_thd))
        {
            pStkPsData->ps_code_last = ps_code;
            ps_report_event(pStkPsData->ps_input_dev,0);
        }
    }
#else
	if (ps_code > CONFIG_STK_PS_CODE_HIGH_THRESHOLD)
    {
        if (unlikely(ps_code_last < CONFIG_STK_PS_CODE_HIGH_THRESHOLD))
        {
            pStkPsData->ps_code_last = ps_code;
            ps_report_event(pStkPsData->ps_input_dev,0);
        }
    }
#endif

#ifdef CONFIG_STK_PS_ENGINEER_TUNING
	else if (ps_code < ps_code_low_thd)
    {
		if (ps_code_last > ps_code_low_thd)
		{
            pStkPsData->ps_code_last = ps_code;
            ps_report_event(pStkPsData->ps_input_dev,1);
		}
    }
#else
	else  if (ps_code < CONFIG_STK_PS_CODE_LOW_THRESHOLD)
    {
		if (ps_code_last > CONFIG_STK_PS_CODE_LOW_THRESHOLD)
		{
			pStkPsData->ps_code_last = ps_code;
			ps_report_event(pStkPsData->ps_input_dev,1);
		}
    }
#endif

	/*
    ps_distance_last = pStkPsData->ps_distance_last;
    distance_i = ps31xx_lookup_table[ps_code];
    if (distance_i>CONFIG_STK_PS_DISTANCE_HIGH_THRESHOLD)
    {
        // Long Range
        if (unlikely(ps_distance_last<CONFIG_STK_PS_DISTANCE_HIGH_THRESHOLD))
        {// From Mid/Short to Long
            pStkPsData->ps_distance_last = distance_i;
            ps_report_event(pStkPsData->ps_input_dev,1);
        }
    }
    else if (distance_i<CONFIG_STK_PS_DISTANCE_LOW_THRESHOLD)
    { // Short Range
      if (ps_distance_last>CONFIG_STK_PS_DISTANCE_LOW_THRESHOLD)
      {
          // Form Mid/Long to Short
            pStkPsData->ps_distance_last = distance_i;
            ps_report_event(pStkPsData->ps_input_dev,0);
      }
    }
	*/
}

static int ps_polling_function(void* arg)
{
    uint32_t delay;
    init_completion(&ps_thread_completion);
    while (1)
    {
        STK_LOCK(1);
        delay = pStkPsData->ps_delay;
        pStkPsData->ps_reading = get_ps_reading();
        update_and_check_report_ps(pStkPsData->ps_reading);
        if (pStkPsData->bPSThreadRunning == 0)
            break;
        STK_LOCK(0);
        msleep(delay);
    };
    STK_LOCK(0);
    complete(&ps_thread_completion);
    return 0;
}

static int32_t enable_ps(uint8_t enable)
{
    if (enable)
    {
        if (pStkPsData->bPSThreadRunning == 0)
        {
            // Dummy Report
            /*ps_report_event(pStkPsData->ps_input_dev,-1);*/

            //reset ps reading, and let it generate an input event while enabling
            //pStkPsData->ps_distance_last =
            //(CONFIG_STK_PS_DISTANCE_HIGH_THRESHOLD+CONFIG_STK_PS_DISTANCE_LOW_THRESHOLD)/2;
            pStkPsData->ps_code_last = 0;
            pStkPsData->ps_reading = 0;
            pStkPsData->bPSThreadRunning = 1;
            ps_polling_tsk = kthread_run(ps_polling_function,NULL,"ps_polling");
        }
        else
        {
            WARNING("STK PS : thread has running\n");
        }
    }
    else
    {

        if (pStkPsData->bPSThreadRunning)
        {

            pStkPsData->bPSThreadRunning = 0;
            STK_LOCK(0);
            wait_for_completion(&ps_thread_completion);
            STK_LOCK(1);
            ps_polling_tsk = NULL;
        }
    }

    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SD_MASK);
    pStkPsData->ps_cmd_reg |= STK_PS_CMD_SD(enable?0:1);
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);

}
static int32_t enable_als(uint8_t enable)
{
    if (enable)
    {
        if (pStkPsData->bALSThreadRunning == 0)
        {
            pStkPsData->als_reading = 0;
            pStkPsData->bALSThreadRunning = 1;
            als_polling_tsk = kthread_run(als_polling_function,NULL,"als_polling");
        }
        else
        {
            WARNING("STK ALS : thread has running\n");
        }
    }
    else
    {

        if (pStkPsData->bALSThreadRunning)
        {
            pStkPsData->bALSThreadRunning = 0;
            STK_LOCK(0);
            wait_for_completion(&als_thread_completion);
            STK_LOCK(1);
            als_polling_tsk = NULL;
        }
    }
    pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_SD_MASK);
    pStkPsData->als_cmd_reg |= STK_ALS_CMD_SD(enable?0:1);
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_ALS_CMD_REG,pStkPsData->als_cmd_reg);

}


#ifdef CONFIG_STK_SYSFS_DBG
// For Debug
static ssize_t help_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
     return sprintf(buf, "Usage : cat xxxx\nor echo val > xxxx\
     \nWhere xxxx = ps_code : RO (0~255)\nals_code : RO (0~65535)\nlux : RW (0~by your setting)\ndistance : RW(by your setting)\
     \nals_enable : RW (0~1)\nps_enable : RW(0~1)\nals_transmittance : RW (1~10000)\
     \nps_sleep_time : RW (0~3)\nps_led_driving_current : RW(0~1)\nps_integral_time(0~3)\nps_gain_setting : RW(0~3)\n");

}

static ssize_t driver_version_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf,"%s\n",STK_DRIVER_VER);
}

static ssize_t als_code_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t reading;
    STK_LOCK(1);
    reading = pStkPsData->als_reading;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", reading);
}


static ssize_t ps_code_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t reading;
    STK_LOCK(1);
    reading = pStkPsData->ps_reading;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", reading);
}

#endif //CONFIG_STK_SYSFS_DBG

static ssize_t lux_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", alscode2lux((1<<16) -1));//full code

}

static ssize_t dist_mode_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf,"%d\n",(int32_t)STK_PS_DISTANCE_MODE);
}
static ssize_t dist_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n"); // means 0.001 cm in Android
}
static ssize_t lux_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n"); // means 1 lux in Android
}
static ssize_t distance_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n",1);
}

static ssize_t ps_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t enable;
    STK_LOCK(1);
    enable = pStkPsData->bPSThreadRunning;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", enable);
}

static ssize_t ps_enable_store(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               const char *buf, size_t len)
{
    uint32_t value = simple_strtoul(buf, NULL, 10);
    INFO("STK PS31xx Driver : Enable PS : %d\n",value);
    STK_LOCK(1);
    enable_ps(value);
    STK_LOCK(0);
    return len;
}

static ssize_t als_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t enable;
    STK_LOCK(1);
    enable = pStkPsData->bALSThreadRunning;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", enable);
}


static ssize_t als_enable_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    uint32_t value = simple_strtoul(buf, NULL, 10);
    INFO("STK PS31xx Driver : Enable ALS : %d\n",value);
    STK_LOCK(1);
    enable_als(value);
    STK_LOCK(0);
    return len;
}


static ssize_t lux_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t als_reading;
    STK_LOCK(1);
    als_reading = pStkPsData->als_reading;
    STK_LOCK(0);
    return sprintf(buf, "%d lux\n", alscode2lux(als_reading));
}

static ssize_t lux_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 16);
    STK_LOCK(1);
    als_report_event(pStkPsData->als_input_dev,value);
    STK_LOCK(0);
    return len;
}

static ssize_t distance_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t dist=1;
    STK_LOCK(1);
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
	if (get_ps_reading()>=ps_code_high_thd)
#else
	if (get_ps_reading()>=CONFIG_STK_PS_CODE_HIGH_THRESHOLD)
#endif
	{
		ps_report_event(pStkPsData->ps_input_dev,0);
		dist=0;
	}
	else
	{
		ps_report_event(pStkPsData->ps_input_dev,1);
		dist=1;
	}
    STK_LOCK(0);

    return sprintf(buf, "%d\n", dist);
}

static ssize_t distance_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 16);
    STK_LOCK(1);
    ps_report_event(pStkPsData->ps_input_dev,value);
    STK_LOCK(0);
    return len;
}
#define __DEBUG_SYSFS_BIN 1


ssize_t stk_bin_sysfs_read(als_lux_range)
{
    uint32_t* pDst = (uint32_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Lux Range(bin) size !=4\n");
        return 0;
    }
#endif

    *pDst = alscode2lux((1<<16)-1);
    return sizeof(uint32_t);
}
ssize_t stk_bin_sysfs_read(ps_distance_range)
{
    int32_t* pDst = (int32_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(int32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Distance Range(bin) size !=4\n");
        return 0;
    }
#endif

  *pDst = 1;
  return sizeof(int32_t);
}

ssize_t stk_bin_sysfs_read(ps_distance_mode)
{
    uint8_t* pDst = (uint8_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint8_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Distance Range(bin) size !=4\n");
        return 0;
    }
#endif

  *pDst = STK_PS_DISTANCE_MODE;
  return sizeof(uint8_t);
}

ssize_t stk_bin_sysfs_read(ps_distance_resolution)
{
    uint32_t* pDst = (uint32_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Distance Resolution(bin) size !=4\n");
        return 0;
    }
#endif
  // means 1 cm for Android
  *pDst = 1;
  return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(als_lux_resolution)
{
    uint32_t* pDst = (uint32_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Distance Range(bin) size !=4\n");
        return 0;
    }
#endif
   // means 1 lux for Android
  *pDst = 1;
  return sizeof(uint32_t);
}



ssize_t stk_bin_sysfs_read(distance)
{
    int32_t* pDst = (int32_t*) buffer;

#if __DEBUG_SYSFS_BIN
    if (count != sizeof(int32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Distance(bin) size !=4\n");
        return 0;
    }
#endif

    STK_LOCK(1);
    *pDst = pStkPsData->ps_distance_last;
    STK_LOCK(0);
    return sizeof(int32_t);
}
ssize_t stk_bin_sysfs_read(lux_bin)
{

    int32_t *pDst = (int32_t*)buffer;

#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read Lux(bin) size !=4\n");
        return 0;
    }
#endif
    STK_LOCK(1);
    *pDst = pStkPsData->als_lux_last;
    STK_LOCK(0);
    return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(ps_enable)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint8_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read ps_enable_bin size !=1\n");
        return 0;
    }
#endif
    STK_LOCK(1);
    buffer[0] = pStkPsData->bPSThreadRunning?1:0;
    STK_LOCK(0);
    return sizeof(uint8_t);
}


ssize_t stk_bin_sysfs_read(als_enable)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint8_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read als_enable_bin size !=1\n");
        return 0;
    }
#endif
    STK_LOCK(1);
    buffer[0] = pStkPsData->bALSThreadRunning?1:0;
    STK_LOCK(0);
    return sizeof(uint8_t);
}
ssize_t  stk_bin_sysfs_write(ps_enable)
{
#if __DEBUG_SYSFS_BIN
    INFO("STK PS31xx Driver : Enable PS : %d\n",(int32_t)(buffer[0]));
#endif
    STK_LOCK(1);
    enable_ps(buffer[0]);
    STK_LOCK(0);
    return count;
}
ssize_t  stk_bin_sysfs_write(als_enable)
{
#if __DEBUG_SYSFS_BIN
    INFO("STK PS31xx Driver : Enable ALS : %d\n",(int32_t)(buffer[0]));
#endif
    STK_LOCK(1);
    enable_als(buffer[0]);
    STK_LOCK(0);
    return count;
}

ssize_t stk_bin_sysfs_read(als_delay)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read als_delay size !=4\n");
        return 0;
    }
#endif
    STK_LOCK(1);
    *((uint32_t*)buffer) = pStkPsData->als_delay;
    STK_LOCK(0);
    return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_write(als_delay)
{
    uint32_t delay;
#if __DEBUG_SYSFS_BIN
    INFO("STK PS31xx Driver : Set ALS Delay: %d\n",*((int32_t*)buffer));
#endif
    delay = *((uint32_t*)buffer);
    if (delay<ALS_MIN_DELAY)
        delay = ALS_MIN_DELAY;
    STK_LOCK(1);
    pStkPsData->als_delay = delay;
    STK_LOCK(0);
    return count;
}

ssize_t stk_bin_sysfs_read(ps_delay)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read ps_delay size !=4\n");
        return 0;
    }
#endif
    STK_LOCK(1);
    *((uint32_t*)buffer) = pStkPsData->ps_delay;
    STK_LOCK(0);
    return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_write(ps_delay)
{
    uint32_t delay;
#if __DEBUG_SYSFS_BIN
    INFO("STK PS31xx Driver : Set PS Delay: %d\n",*((int32_t*)buffer));
#endif
    delay = *((uint32_t*)buffer);
    if (delay<PS_MIN_DELAY)
        delay = PS_MIN_DELAY;
    STK_LOCK(1);
    pStkPsData->ps_delay = delay;
    STK_LOCK(0);
    return count;
}

ssize_t stk_bin_sysfs_read(als_min_delay)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read als_min_delay size !=4\n");
        return 0;
    }
#endif
    *((uint32_t*)buffer) = ALS_MIN_DELAY;
    return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(ps_min_delay)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK PS31xx Driver : Error --> Read ps_min_delay size !=4\n");
        return 0;
    }
#endif
    *((uint32_t*)buffer) = PS_MIN_DELAY;
    return sizeof(uint32_t);
}

#ifdef CONFIG_STK_SYSFS_DBG
/* Only for debug */
static struct kobj_attribute help_attribute = (struct kobj_attribute)__ATTR_RO(help);
static struct kobj_attribute driver_version_attribute = (struct kobj_attribute)__ATTR_RO(driver_version);
static struct kobj_attribute als_code_attribute = (struct kobj_attribute)__ATTR_RO(als_code);
static struct kobj_attribute ps_code_attribute = (struct kobj_attribute)__ATTR_RO(ps_code);
#endif //CONFIG_STK_SYSFS_DBG

static struct kobj_attribute lux_range_attribute = (struct kobj_attribute)__ATTR_RO(lux_range);
static struct kobj_attribute lux_attribute = (struct kobj_attribute)__ATTR_RW(lux);
static struct kobj_attribute distance_attribute = (struct kobj_attribute)__ATTR_RW(distance);
static struct kobj_attribute ps_enable_attribute = (struct kobj_attribute)__ATTR_RW(ps_enable);
static struct kobj_attribute als_enable_attribute = (struct kobj_attribute)__ATTR_RW(als_enable);
static struct kobj_attribute ps_dist_mode_attribute = (struct kobj_attribute)__ATTR_RO(dist_mode);
static struct kobj_attribute ps_dist_res_attribute = (struct kobj_attribute)__ATTR_RO(dist_res);
static struct kobj_attribute lux_res_attribute = (struct kobj_attribute)__ATTR_RO(lux_res);
static struct kobj_attribute ps_distance_range_attribute = (struct kobj_attribute)__ATTR_RO(distance_range);

static struct bin_attribute ps_distance_range_bin_attribute = __ATTR_BIN_RO(distance_range_bin,ps_distance_range_read,sizeof(int32_t));
static struct bin_attribute ps_distance_bin_attribute = __ATTR_BIN_RO(distance_bin,distance_read,sizeof(int32_t));
static struct bin_attribute ps_dist_res_bin_attribute = __ATTR_BIN_RO(distance_resolution_bin,ps_distance_resolution_read,sizeof(uint32_t));
static struct bin_attribute ps_dist_mode_bin_attribute = __ATTR_BIN_RO(ps_dist_mode_bin,ps_distance_mode_read,sizeof(uint8_t));
static struct bin_attribute ps_enable_bin_attribute = __ATTR_BIN_RW(ps_enable_bin,ps_enable_read,ps_enable_write,sizeof(uint8_t));
static struct bin_attribute als_lux_range_bin_attribute = __ATTR_BIN_RO(lux_range_bin,als_lux_range_read,sizeof(uint32_t));
static struct bin_attribute als_lux_bin_attribute = __ATTR_BIN_RO(lux_bin,lux_bin_read,sizeof(uint32_t));
static struct bin_attribute als_lux_res_bin_attribute = __ATTR_BIN_RO(lux_resolution_bin,als_lux_resolution_read,sizeof(uint32_t));
static struct bin_attribute als_enable_bin_attribute = __ATTR_BIN_RW(als_enable_bin,als_enable_read,als_enable_write,sizeof(uint8_t));
/* <---DEPRECATED */
static struct bin_attribute als_delay_bin_attribute = __ATTR_BIN_RW(als_delay_bin,als_delay_read,als_delay_write,sizeof(uint32_t));
static struct bin_attribute ps_delay_bin_attribute = __ATTR_BIN_RW(ps_delay_bin,ps_delay_read,ps_delay_write,sizeof(uint32_t));
static struct bin_attribute als_min_delay_bin_attribute = __ATTR_BIN_RO(als_min_delay_bin,als_min_delay_read,sizeof(uint32_t));
static struct bin_attribute ps_min_delay_bin_attribute = __ATTR_BIN_RO(ps_min_delay_bin,ps_min_delay_read,sizeof(uint32_t));
/* DEPRECATED---> */
#ifdef CONFIG_STK_SYSFS_DBG

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
#pragma message("Enable STK ALS Transmittance Tuning w/ SYSFS")
static ssize_t als_transmittance_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t transmittance;
    STK_LOCK(1);
    transmittance = als_transmittance;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", transmittance);
}


static ssize_t als_transmittance_store(struct kobject *kobj,
                                       struct kobj_attribute *attr,
                                       const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    als_transmittance = value;
    STK_LOCK(0);
    return len;
}



static struct kobj_attribute als_transmittance_attribute = (struct kobj_attribute)__ATTR_RW(als_transmittance);
#endif // CONFIG_STK_ALS_TRANSMITTANCE_TUNING



#ifdef CONFIG_STK_PS_ENGINEER_TUNING

#pragma message("Enable STK PS Engineering Tuning w/ SYSFS")
static ssize_t ps_sleep_time_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t value;
    STK_LOCK(1);
    value = pStkPsData->ps_cmd_reg;
    STK_LOCK(0);
    value&=STK_PS_CMD_SLP_MASK;
    value>>=STK_PS_CMD_SLP_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_sleep_time_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_slp(value);
    STK_LOCK(0);
    return len;
}


static ssize_t ps_led_driving_current_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t value;
    STK_LOCK(1);
    value = pStkPsData->ps_cmd_reg;
    STK_LOCK(0);
    value&=STK_PS_CMD_DR_MASK;
    value>>=STK_PS_CMD_DR_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_led_driving_current_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_led_driving_current(value);
    STK_LOCK(0);
    return len;
}
static ssize_t ps_integral_time_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t value;
    STK_LOCK(1);
    value = pStkPsData->ps_cmd_reg;
    STK_LOCK(0);
    value&=STK_PS_CMD_IT_MASK;
    value>>=STK_PS_CMD_IT_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_integral_time_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_it((uint8_t)value);
    STK_LOCK(0);
    return len;
}
static ssize_t ps_gain_setting_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t gc_reg;
    STK_LOCK(1);
    gc_reg = pStkPsData->ps_gc_reg;
    STK_LOCK(0);
    return sprintf(buf, "0x%x\n", gc_reg);
}


static ssize_t ps_gain_setting_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_gc((uint8_t)value);
    STK_LOCK(0);
    return len;
}


static ssize_t ps_code_thd_l_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", ps_code_low_thd);
}


static ssize_t ps_code_thd_l_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_thd_l(value);
    STK_LOCK(0);
    return len;
}

static ssize_t ps_code_thd_h_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", ps_code_high_thd);
}


static ssize_t ps_code_thd_h_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_thd_h(value);
    STK_LOCK(0);
    return len;
}

static struct kobj_attribute ps_sleep_time_attribute = (struct kobj_attribute)__ATTR_RW(ps_sleep_time);
static struct kobj_attribute ps_led_driving_current_attribute = (struct kobj_attribute)__ATTR_RW(ps_led_driving_current);
static struct kobj_attribute ps_integral_time_attribute = (struct kobj_attribute)__ATTR_RW(ps_integral_time);
static struct kobj_attribute ps_gain_setting_attribute = (struct kobj_attribute)__ATTR_RW(ps_gain_setting);
static struct kobj_attribute ps_code_thd_l_attribute = (struct kobj_attribute)__ATTR_RW(ps_code_thd_l);
static struct kobj_attribute ps_code_thd_h_attribute = (struct kobj_attribute)__ATTR_RW(ps_code_thd_h);
#endif //CONFIG_STK_PS_ENGINEER_TUNING

#endif //CONFIG_STK_SYSFS_DBG

static struct attribute* sensetek_optical_sensors_attrs [] =
{
    &lux_range_attribute.attr,
    &lux_attribute.attr,
    &distance_attribute.attr,
    &ps_enable_attribute.attr,
    &als_enable_attribute.attr,
    &ps_dist_mode_attribute.attr,
    &ps_dist_res_attribute.attr,
    &lux_res_attribute.attr,
    &ps_distance_range_attribute.attr,
    NULL,
};

#ifdef CONFIG_STK_SYSFS_DBG

static struct attribute* sensetek_optical_sensors_dbg_attrs [] =
{
    &help_attribute.attr,
    &driver_version_attribute.attr,
    &lux_range_attribute.attr,
    &ps_code_attribute.attr,
    &als_code_attribute.attr,
    &lux_attribute.attr,
    &distance_attribute.attr,
    &ps_enable_attribute.attr,
    &als_enable_attribute.attr,
    &ps_dist_mode_attribute.attr,
    &ps_dist_res_attribute.attr,
    &lux_res_attribute.attr,
    &ps_distance_range_attribute.attr,
#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    &als_transmittance_attribute.attr,
#endif
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
    &ps_sleep_time_attribute.attr,
    &ps_led_driving_current_attribute.attr,
    &ps_integral_time_attribute.attr,
    &ps_gain_setting_attribute.attr,
    &ps_code_thd_l_attribute.attr,
    &ps_code_thd_h_attribute.attr,
#endif //CONFIG_STK_PS_ENGINEER_TUNING
    NULL,
};
// those attributes are only for engineer test/debug
static struct attribute_group sensetek_optics_sensors_attrs_group =
{
    .name = "DBG",
    .attrs = sensetek_optical_sensors_dbg_attrs,
};
#endif //CONFIG_STK_SYSFS_DBG

static struct bin_attribute* sensetek_optical_sensors_bin_attrs[] =
{
    &ps_distance_range_bin_attribute,
    &als_lux_range_bin_attribute,
    &ps_distance_bin_attribute,
    &als_lux_bin_attribute,
    &ps_enable_bin_attribute,
    &als_lux_res_bin_attribute,
    &als_enable_bin_attribute,
    &ps_dist_mode_bin_attribute,
    &ps_dist_res_bin_attribute,
    &als_delay_bin_attribute,
    &ps_delay_bin_attribute,
    &als_min_delay_bin_attribute,
    &ps_min_delay_bin_attribute,
    NULL,
};

static struct platform_device *stk_oss_dev = NULL; /* Device structure */

static int stk_sysfs_create_files(struct kobject *kobj,struct attribute** attrs)
{
    int err;
    while(*attrs!=NULL)
    {
        err = sysfs_create_file(kobj,*attrs);
        if (err)
            return err;
        attrs++;
    }
    return 0;
}

static int stk_sysfs_create_bin_files(struct kobject *kobj,struct bin_attribute** bin_attrs)
{
    int err;
    while(*bin_attrs!=NULL)
    {
        err = sysfs_create_bin_file(kobj,*bin_attrs);
        if (err)
            return err;
        bin_attrs++;
    }
    return 0;
}

static void stk_sysfs_remove_bin_files(struct kobject *kobj,struct bin_attribute** bin_attrs)
{
    while(*bin_attrs!=NULL)
    {
        sysfs_remove_bin_file(kobj,*bin_attrs);
        bin_attrs++;
    }
}

static int stk_ps_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    /*
    printk("STKPS -- %s: I2C is probing (%s)%d\n nDetect = %d\n", __func__,id->name,id->driver_data);
    */
    int err;
    struct stkps31xx_data* ps_data;
    INFO("STK PS : I2C Probing");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        printk("STKPS -- No Support for I2C_FUNC_SMBUS_BYTE_DATA\n");
        return -ENODEV;
    }
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA))
    {
        printk("STKPS -- No Support for I2C_FUNC_SMBUS_WORD_DATA\n");
        return -ENODEV;
    }


    if (id->driver_data == 0)
    {

        ps_data = kzalloc(sizeof(struct stkps31xx_data),GFP_KERNEL);
        ps_data->client = client;
        i2c_set_clientdata(client,ps_data);
        mutex_init(&stkps_io_lock);

        ps_data->als_input_dev = input_allocate_device();
        ps_data->ps_input_dev = input_allocate_device();
        if ((ps_data->als_input_dev==NULL)||(ps_data->ps_input_dev==NULL))
        {
            if (ps_data->als_input_dev==NULL)
                input_free_device(ps_data->als_input_dev);
            if (ps_data->ps_input_dev==NULL)
                input_free_device(ps_data->ps_input_dev);
            ERR("%s: could not allocate input device\n", __func__);
            mutex_destroy(&stkps_io_lock);
            kfree(ps_data);
            return -ENOMEM;
        }
        ps_data->als_input_dev->name = ALS_NAME;
        ps_data->ps_input_dev->name = PS_NAME;
        set_bit(EV_ABS, ps_data->als_input_dev->evbit);
        set_bit(EV_ABS, ps_data->ps_input_dev->evbit);
        input_set_abs_params(ps_data->als_input_dev, ABS_MISC, 0, alscode2lux((1<<16)-1), 0, 0);
        input_set_abs_params(ps_data->ps_input_dev, ABS_DISTANCE, 0,1, 0, 0);
        err = input_register_device(ps_data->als_input_dev);
        if (err<0)
        {
            ERR("STK PS : can not register als input device\n");
            mutex_destroy(&stkps_io_lock);
            input_free_device(ps_data->als_input_dev);
            input_free_device(ps_data->ps_input_dev);
            kfree(ps_data);

            return err;
        }
        INFO("STK PS : register als input device OK\n");
        err = input_register_device(ps_data->ps_input_dev);
        if (err<0)
        {
            ERR("STK PS : can not register ps input device\n");
            input_unregister_device(ps_data->als_input_dev);
            mutex_destroy(&stkps_io_lock);
            input_free_device(ps_data->als_input_dev);
            input_free_device(ps_data->ps_input_dev);
            kfree(ps_data);
            return err;

        }
        pStkPsData = ps_data;
        ps_data->ps_delay = PS_ODR_DELAY;
        ps_data->als_delay = ALS_ODR_DELAY;
        wake_lock_init(&proximity_sensor_wakelock,WAKE_LOCK_IDLE,"stk_ps_wakelock");
        if (!init_all_setting())
        {
            input_unregister_device(pStkPsData->als_input_dev);
            input_unregister_device(pStkPsData->ps_input_dev);
            input_free_device(pStkPsData->als_input_dev);
            input_free_device(pStkPsData->ps_input_dev);
            kfree(pStkPsData);
            pStkPsData = NULL;
            return -EINVAL;
        }

        return 0;
    }

    return -EINVAL;
}


static int stk_ps_remove(struct i2c_client *client)
{

    mutex_destroy(&stkps_io_lock);
    wake_lock_destroy(&proximity_sensor_wakelock);
    if (pStkPsData)
    {
        input_unregister_device(pStkPsData->als_input_dev);
        input_unregister_device(pStkPsData->ps_input_dev);
        input_free_device(pStkPsData->als_input_dev);
        input_free_device(pStkPsData->ps_input_dev);
        kfree(pStkPsData);
        pStkPsData = 0;
    }
    return 0;
}

static const struct i2c_device_id stk_ps_id[] =
{
    { "stk_ps", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, stk_ps_id);

static struct i2c_driver stk_ps_driver =
{
    .driver = {
        .name = STKPS_DRV_NAME,

    },
    .probe = stk_ps_probe,
    .remove = stk_ps_remove,
    .id_table = stk_ps_id,
};


static int __init stk_i2c_ps31xx_init(void)
{

	int ret;
    ret = i2c_add_driver(&stk_ps_driver);
    if (ret)
        return ret;
    if (pStkPsData == NULL)
        return -EINVAL;

    stk_oss_dev = platform_device_alloc(DEVICE_NAME,-1);
    if (!stk_oss_dev)
    {
       i2c_del_driver(&stk_ps_driver);
       return -ENOMEM;
    }
    if (platform_device_add(stk_oss_dev))
    {
       i2c_del_driver(&stk_ps_driver);
       return -ENOMEM;
    }
    ret = stk_sysfs_create_bin_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_bin_attrs);
    if (ret)
    {
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }
    ret = stk_sysfs_create_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_attrs);
    if (ret)
    {
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }
#ifdef CONFIG_STK_SYSFS_DBG
    ret = sysfs_create_group(&(stk_oss_dev->dev.kobj), &sensetek_optics_sensors_attrs_group);
    if (ret)
    {
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }
#endif //CONFIG_STK_SYSFS_DBG

	INFO("STK PS Module initialized.\n");
    return 0;
}

static void __exit stk_i2c_ps31xx_exit(void)
{

    if (stk_oss_dev);
    {
#ifdef CONFIG_STK_SYSFS_DBG
        sysfs_remove_group(&(stk_oss_dev->dev.kobj), &sensetek_optics_sensors_attrs_group);
#endif
        stk_sysfs_remove_bin_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_bin_attrs);
    }
    platform_device_put(stk_oss_dev);

    i2c_del_driver(&stk_ps_driver);
    platform_device_unregister(stk_oss_dev);
}

MODULE_AUTHOR("Patrick Chang <patrick_chang@sitronix.com>");
MODULE_DESCRIPTION("SenseTek Proximity Sensor driver");
MODULE_LICENSE("GPL");
module_init(stk_i2c_ps31xx_init);
module_exit(stk_i2c_ps31xx_exit);
