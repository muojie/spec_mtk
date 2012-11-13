/*
 *  stk_i2c_als_220x_int.c - Linux kernel modules for stk220x ambient light sensor with interrupt
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
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include "stk_defines.h"
#include "stk_lk_defs.h"
#include "stk_i2c_als_220xgeneric.h"

#define ADDITIONAL_GPIO_CFG 1

/* // Additional GPIO CFG Header */
#if ADDITIONAL_GPIO_CFG
#include <mach/gpio.h>
#endif

#define STKALS_DRV_NAME	"stk_als"
#define DEVICE_NAME		"stk-oss"
#define DRIVER_VERSION  STK_DRIVER_VER


#define stk_writeb(x,y) i2c_smbus_write_byte_data(pStkAlsData->client,x,y)
#define stk_readb(x) i2c_smbus_read_byte_data(pStkAlsData->client,x)


#define ABS(x) ((x)>=0? (x):(-x))

#define STK_LOCK0 mutex_unlock(&stkals_io_lock)
#define STK_LOCK1 mutex_lock(&stkals_io_lock)

static int32_t init_all_setting(void);
static int32_t get_lux(void);
#if 0
static int32_t get_it(void);
static int32_t get_gain(void);
#endif
static int32_t set_it(uint32_t it);
static int32_t set_gain(uint32_t gain);
static int32_t enable_als(uint32_t enable);
static int32_t set_power_state(uint32_t nShutdown);

static struct mutex stkals_io_lock;


struct stkals_data* pStkAlsData = NULL;

#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
static int32_t als_transmittance = CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING

static struct workqueue_struct *stk_oss_work_queue = NULL;

inline void report_event(struct input_dev* dev,int32_t report_value)
{
	input_report_abs(dev, ABS_MISC, report_value);
	input_sync(dev);
	INFO("STK ALS : als input event %d lux\n",report_value);
}

inline int32_t get_reading(void)
{
	return STK_ALS_DATA(stk_readb(STK_ALS_DT1_REG),stk_readb(STK_ALS_DT2_REG));
}

inline int32_t alscode2lux(int32_t alscode)
{
    //R-set Def --> 500KOhm ==> x 5
    //IT --> x1
    //Gain --> x2
    // Total x 10

    alscode<<=10; // x 1024
    // Org : 1 code (100KOhm, IT = x1 , Gain = x1) ~ 1 lux
    // x10 x1024 ==> 10240 code = 1 lux
#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    return alscode/als_transmittance;
#else
    return alscode/CONFIG_STK_ALS_TRANSMITTANCE;
#endif //CONFIG_STK_ALS_TRANSMITTANCE_TUNING
}

static int32_t get_lux()
{
    return  alscode2lux(get_reading());
}
#if 0
static int32_t get_it()
{
	int32_t val;
	val = stk_readb(STK_ALS_CMD_REG);
	val = (val &STK_ALS_CMD_IT_MASK)>>STK_ALS_CMD_IT_SHIFT;
    return val;
}
static int32_t get_gain()
{
	int32_t val;
	val = stk_readb(STK_ALS_CMD_REG);
	val = (val &STK_ALS_CMD_GAIN_MASK)>>STK_ALS_CMD_GAIN_SHIFT;
    return val;
}
#endif

static int32_t set_thd(uint32_t thd)
{
	int32_t val;
	val = stk_readb(STK_ALS_INT_REG);
	val &= (~STK_ALS_INT_THD_MASK);
	val |= STK_ALS_INT_THD(thd);
	return stk_writeb(STK_ALS_INT_REG,val);

}

static int32_t set_prst(uint32_t prst)
{
	int32_t val;
	val = stk_readb(STK_ALS_INT_REG);
	val &= (~STK_ALS_INT_PRST_MASK);
	val |= STK_ALS_INT_PRST(prst);
	return stk_writeb(STK_ALS_INT_REG,val);

}

static int32_t set_it(uint32_t it)
{
	int32_t val;
	val = stk_readb(STK_ALS_CMD_REG);
	val &= (~STK_ALS_CMD_IT_MASK);
	val |= STK_ALS_CMD_IT(it);
	return stk_writeb(STK_ALS_CMD_REG,val);
}

static int32_t set_gain(uint32_t gain)
{
	int32_t val;
	val = stk_readb(STK_ALS_CMD_REG);
	val &= (~STK_ALS_CMD_GAIN_MASK);
	val |= STK_ALS_CMD_GAIN(gain);
	return stk_writeb(STK_ALS_CMD_REG,val);
}

static int32_t set_power_state(uint32_t nShutdown)
{
	int32_t val;
	val = stk_readb(STK_ALS_CMD_REG);
	val &= (~STK_ALS_CMD_SD_MASK);
	val |= STK_ALS_CMD_SD(nShutdown);
	return stk_writeb(STK_ALS_CMD_REG,val);
}


static int32_t enable_als(uint32_t enable)
{
	int32_t ret;

	pStkAlsData->enable_als_irq = 0;

	ret = set_power_state(enable?0:1);

	if (enable)
	{
	/* wait IT*/
	msleep(1000);
		pStkAlsData->enable_als_irq = 1;
		enable_irq(pStkAlsData->irq);
	}
	else
	{
		disable_irq(pStkAlsData->irq);
	}

	return ret;
}


static int32_t init_all_setting()
{
	uint32_t val;
	enable_als(0);

	set_gain(1); //x2
	set_it(1); //x1, if R-set = 100k, integration time = 100 ms
	val = stk_readb(STK_ALS_CMD_REG);
	INFO("Init ALS Setting --> CMDREG = 0x%x\n",val);
	pStkAlsData->enable_als_irq = 1;

	val = stk_readb(STK_ALS_INT_REG);
	INFO("Init ALS Setting --> INTREG = 0x%x\n",val);

	if(val & STK_ALS_INT_FLAG_MASK)
	{
		val &= (~STK_ALS_INT_FLAG_MASK);
		stk_writeb(STK_ALS_INT_REG,val);
	}
    return 0; // OK
}


static ssize_t help_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
     return sprintf(buf, "Usage : cat xxxx\nor echo val > xxxx\
     \nWhere xxxx = lux : RO (0~by your setting)\
     \nals_enable : RW (0~1)\nals_transmittance : RW (1~10000)\n");

}

static ssize_t lux_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", alscode2lux((1<<12) -1));//full code

}


static ssize_t als_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t enable;
    STK_LOCK(1);
	enable = pStkAlsData->enable_als_irq;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", enable);
}


static ssize_t als_enable_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    uint32_t value = simple_strtoul(buf, NULL, 10);
    INFO("STK ALS Driver : Enable ALS : %d\n",value);
    STK_LOCK(1);
    enable_als(value);
    STK_LOCK(0);
    return len;
}


static ssize_t lux_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t lux;
    STK_LOCK(1);
    lux = pStkAlsData->als_lux_last;
    STK_LOCK(0);
    return sprintf(buf, "%d lux\n", lux);
}
static ssize_t lux_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    report_event(pStkAlsData->input_dev,value);
    STK_LOCK(0);
    return len;
}
static ssize_t lux_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n");
}
static ssize_t driver_version_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf,"%s\n",STK_DRIVER_VER);
}


#define __DEBUG_SYSFS_BIN 1

ssize_t stk_bin_sysfs_read(als_lux_range)
{
    uint32_t* pDst = (uint32_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK ALS Driver : Error --> Read Lux Range(bin) size !=4\n");
        return 0;
    }
#endif

    *pDst = alscode2lux((1<<12)-1);
    return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(als_lux_resolution)
{
    uint32_t* pDst = (uint32_t*)buffer;
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK ALS Driver : Error --> Read Distance Range(bin) size !=4\n");
        return 0;
    }
#endif
   // means 1 lux for Android
  *pDst = 1;
  return sizeof(uint32_t);
}

ssize_t stk_bin_sysfs_read(lux_bin)
{

    int32_t *pDst = (int32_t*)buffer;

#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK ALS Driver : Error --> Read Lux(bin) size !=4\n");
        return 0;
    }
#endif
    STK_LOCK(1);
    *pDst = pStkAlsData->als_lux_last;
    STK_LOCK(0);
    return sizeof(uint32_t);
}


ssize_t stk_bin_sysfs_read(als_enable)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint8_t))
    {
        WARNING("STK ALS Driver : Error --> Read als_enable_bin size !=1\n");
        return 0;
    }
#endif
    STK_LOCK(1);

    STK_LOCK(0);
    return sizeof(uint8_t);
}

ssize_t  stk_bin_sysfs_write(als_enable)
{
#if __DEBUG_SYSFS_BIN
    INFO("STK ALS Driver : Enable ALS : %d\n",(int32_t)(buffer[0]));
#endif
    STK_LOCK(1);
    enable_als(buffer[0]);
    STK_LOCK(0);
    return count;
}


ssize_t stk_bin_sysfs_read(als_min_delay)
{
#if __DEBUG_SYSFS_BIN
    if (count != sizeof(uint32_t))
    {
        WARNING("STK ALS Driver : Error --> Read als_min_delay size !=4\n");
        return 0;
    }
#endif
    *((uint32_t*)buffer) = ALS_MIN_DELAY;
    return sizeof(uint32_t);
}

#ifdef CONFIG_STK_SYSFS_DBG
static struct kobj_attribute help_attribute = (struct kobj_attribute)__ATTR_RO(help);
static struct kobj_attribute driver_version_attribute = (struct kobj_attribute)__ATTR_RO(driver_version);
#endif //CONFIG_STK_SYSFS_DBG
static struct kobj_attribute lux_range_attribute = (struct kobj_attribute)__ATTR_RO(lux_range);
static struct kobj_attribute lux_attribute = (struct kobj_attribute)__ATTR_RW(lux);
static struct kobj_attribute als_enable_attribute = (struct kobj_attribute)__ATTR_RW(als_enable);
static struct kobj_attribute als_lux_res_attribute = (struct kobj_attribute)__ATTR_RO(lux_res);

static struct bin_attribute als_lux_range_bin_attribute = __ATTR_BIN_RO(lux_range_bin,als_lux_range_read,sizeof(uint32_t));
static struct bin_attribute als_lux_bin_attribute = __ATTR_BIN_RO(lux_bin,lux_bin_read,sizeof(uint32_t));
static struct bin_attribute als_lux_res_bin_attribute = __ATTR_BIN_RO(lux_resolution_bin,als_lux_resolution_read,sizeof(uint32_t));
static struct bin_attribute als_enable_bin_attribute = __ATTR_BIN_RW(als_enable_bin,als_enable_read,als_enable_write,sizeof(uint8_t));
/* <---DEPRECATED */
static struct bin_attribute als_min_delay_bin_attribute = __ATTR_BIN_RO(als_min_delay_bin,als_min_delay_read,sizeof(uint32_t));
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


#endif //CONFIG_STK_SYSFS_DBG

static struct attribute* sensetek_optical_sensors_attrs [] =
{
    &lux_range_attribute.attr,
    &lux_attribute.attr,
    &als_enable_attribute.attr,
    &als_lux_res_attribute.attr,
    NULL,
};

#ifdef CONFIG_STK_SYSFS_DBG
static struct attribute* sensetek_optical_sensors_dbg_attrs [] =
{
    &help_attribute.attr,
    &lux_range_attribute.attr,
    &lux_attribute.attr,
    &als_enable_attribute.attr,
    &als_lux_res_attribute.attr,
    &driver_version_attribute.attr,
#if CONFIG_STK_ALS_TRANSMITTANCE_TUNING
    &als_transmittance_attribute.attr,
#endif
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
    &als_lux_range_bin_attribute,
    &als_lux_bin_attribute,
    &als_lux_res_bin_attribute,
    &als_enable_bin_attribute,
    &als_min_delay_bin_attribute,
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

static int stk_sysfs_remove_files(struct kobject *kobj,struct attribute** attrs)
{
    while(*attrs!=NULL)
    {
        sysfs_remove_file(kobj,*attrs);
        attrs++;
    }
    return 0;
}

/*
//DEPRECATED
static void stk_sysfs_remove_bin_files(struct kobject *kobj,struct bin_attribute** bin_attrs)
{
    while(*bin_attrs!=NULL)
    {
        sysfs_remove_bin_file(kobj,*bin_attrs);
        bin_attrs++;
    }
}
//DEPRECATED
*/


static void stk_oss_wq_function(struct work_struct *work)
{
	int32_t lux;
	uint32_t val;

	STK_LOCK(1);
	lux = get_lux();
	pStkAlsData->als_lux_last = lux;
	report_event(pStkAlsData->input_dev,lux);
	STK_LOCK(0);

	if(pStkAlsData->enable_als_irq)
		enable_irq(pStkAlsData->irq);
	else
		disable_irq(pStkAlsData->irq);

	val = stk_readb(STK_ALS_INT_REG);
	val &= (~STK_ALS_INT_FLAG_MASK);
	stk_writeb(STK_ALS_INT_REG,val);

}

static irqreturn_t stk_oss_irq_handler(int irq, void *data)
{

	struct stkals_data *pData = data;
	disable_irq(pData->irq);
	//schedule_work(&pData->work);

    queue_work(stk_oss_work_queue,&pData->work);
	//reset_int_flag(0);
	//enable_irq(pData->irq);
	return IRQ_HANDLED;
}


/*
 * Called when a stk als device is matched with this driver
 */
static int stk_als_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
    int32_t err= -EINVAL;
	struct stkals_data*  als_data;
	int irq;

	if (i2c_smbus_read_byte_data(client,STK_ALS_CMD_REG)<0)
	{
		ERR("STKALS : no device found\n");
		return -ENODEV;
	}
	INFO("STK ALS : als i2c slave address = 0x%x\n",client->addr);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	{
		ERR("STKALS -- No Support for I2C_FUNC_SMBUS_BYTE_DATA\n");
		return -ENODEV;
	}
	als_data = kzalloc(sizeof(struct stkals_data),GFP_KERNEL);
    if (unlikely(als_data == 0))
    {
		ERR("STKALS -- No enough memory\n");
		return -ENOMEM;
	}
	als_data->client = client;
	i2c_set_clientdata(client,als_data);
	mutex_init(&stkals_io_lock);

	pStkAlsData = als_data;

	/*	irq setting	*/
	/*
	INFO("STK APS : irq # = %d\n",client->irq);
	if (client->irq<=0)
	{
		ERR("STK APS : fail --> you don't(or forget to) specify a irq number\nirq # = %d\n",client->irq);
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return -EINVAL;
	}
	*/

	stk_oss_work_queue = create_workqueue("stk_oss_wq");
	if(stk_oss_work_queue)
		INIT_WORK(&als_data->work, stk_oss_wq_function);
	else
	{
		ERR("%s:create_workqueue error\n", __func__);
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}


#if ADDITIONAL_GPIO_CFG // Additional GPIO CFG
	// interrput settings for devkit8000
	// --------devkit8000 specific---------
	if (gpio_request(158, "EINT"))
	{
		ERR("stk als gpio_request() failed\n");
		err = -1;
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}
	gpio_direction_input(158);

	irq = gpio_to_irq(158);
	if ( irq < 0 )
	{
		ERR("stk als gpio_to_irq() failed\n");
		err = -1;
		gpio_free( 158 );
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}
	client->irq = irq;
	INFO("STK ALS : irq # = %d\n",irq);
#endif // Additional GPIO CFG

	err = set_power_state(0);
	if (err < 0)
	{
		ERR("%s: set_power_state error\n", __func__);
		gpio_free( 158 );
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}


	err = request_irq(client->irq, stk_oss_irq_handler, STK_IRQF_MODE, DEVICE_NAME, als_data);
	if (err < 0)
	{
		ERR("%s: request_irq(%d) failed for (%d)\n",
			__func__, client->irq, err);
		gpio_free( 158 );
		mutex_destroy(&stkals_io_lock);
		kfree(als_data);
		pStkAlsData = NULL;
		return err;
	}


#ifdef CONFIG_STK_ALS_TRANSMITTANCE_TUNING
	als_transmittance = CONFIG_STK_ALS_TRANSMITTANCE;
#endif
	init_all_setting();



    pStkAlsData->input_dev = input_allocate_device();
	if (pStkAlsData->input_dev==NULL)
	{
		ERR("STK ALS : can not allocate als input device\n");
		free_irq(client->irq, pStkAlsData);
#if ADDITIONAL_GPIO_CFG // Additional GPIO CFG
		gpio_free( 158 );
#endif	//#if ADDITIONAL_GPIO_CFG
	    mutex_destroy(&stkals_io_lock);
	    kfree(pStkAlsData);
	    pStkAlsData = NULL;
	    return -ENOMEM;
    }
    pStkAlsData->input_dev->name = ALS_NAME;
	set_bit(EV_ABS, pStkAlsData->input_dev->evbit);
    input_set_abs_params(pStkAlsData->input_dev, ABS_MISC, 0, alscode2lux((1<<12)-1), 0, 0);
    err = input_register_device(pStkAlsData->input_dev);
    if (err<0)
    {
        ERR("STK ALS : can not register als input device\n");
		free_irq(client->irq, pStkAlsData);
#if ADDITIONAL_GPIO_CFG // Additional GPIO CFG
		gpio_free( 158 );
#endif	//#if ADDITIONAL_GPIO_CFG
        mutex_destroy(&stkals_io_lock);
        input_free_device(pStkAlsData->input_dev);
        kfree(pStkAlsData);
        pStkAlsData = NULL;
        return err;
    }
    INFO("STK ALS : register als input device OK\n");

	return 0;


}

static int stk_als_remove(struct i2c_client *client)
{
    platform_device_put(stk_oss_dev);
#if ADDITIONAL_GPIO_CFG // Additional GPIO CFG
		gpio_free( 158 );
#endif	//#if ADDITIONAL_GPIO_CFG
	mutex_destroy(&stkals_io_lock);

	if (pStkAlsData)
	{
		if (stk_oss_work_queue)
		{
			flush_workqueue(stk_oss_work_queue);
            destroy_workqueue(stk_oss_work_queue);
		}
        input_unregister_device(pStkAlsData->input_dev);
        input_free_device(pStkAlsData->input_dev);
		free_irq(client->irq, pStkAlsData);
	//	kfree(pStkAlsData);
		pStkAlsData = 0;
	}
	return 0;
}

static const struct i2c_device_id stk_als_id[] = {
	{ "stk_als", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, stk_als_id);

static struct i2c_driver stk_als_driver = {
	.driver = {
		.name = STKALS_DRV_NAME,
	},
	.probe = stk_als_probe,
	.remove = stk_als_remove,
	.id_table = stk_als_id,
};

static int __init stk_i2c_als_init(void)
{
    int ret;
    ret = i2c_add_driver(&stk_als_driver);
    if (ret)
        return ret;
    if (pStkAlsData == NULL)
        return -EINVAL;
    stk_oss_dev = platform_device_alloc(DEVICE_NAME,-1);
    if (!stk_oss_dev)
    {
       i2c_del_driver(&stk_als_driver);
       return -ENOMEM;
    }
    if (platform_device_add(stk_oss_dev))
    {
       i2c_del_driver(&stk_als_driver);
       return -ENOMEM;
    }
    ret = stk_sysfs_create_bin_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_bin_attrs);
    if (ret)
    {
        i2c_del_driver(&stk_als_driver);
        return -ENOMEM;
    }
    ret = stk_sysfs_create_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_attrs);
    if (ret)
    {
        i2c_del_driver(&stk_als_driver);
        return -ENOMEM;
    }
#ifdef CONFIG_STK_SYSFS_DBG
    ret = sysfs_create_group(&(stk_oss_dev->dev.kobj), &sensetek_optics_sensors_attrs_group);
    if (ret)
    {
        i2c_del_driver(&stk_als_driver);
        return -ENOMEM;
    }
#endif
	INFO("STK ALS Module initialized.\n");
	return 0;
}

static void __exit stk_i2c_als_exit(void)
{
	//stk_sysfs_remove_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_attrs);
	i2c_del_driver(&stk_als_driver);
}

MODULE_AUTHOR("Patrick Chang <patrick_chang@sitronix.com>");
MODULE_DESCRIPTION("SenseTek Ambient Light Sensor driver");
MODULE_LICENSE("GPL");

module_init(stk_i2c_als_init);
module_exit(stk_i2c_als_exit);
