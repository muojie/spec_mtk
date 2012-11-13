/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/* drivers/hwmon/mt6516/amit/stk31xx.c - stk31xx ALS/PS driver
 * 
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <linux/hwmsen_helper.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "stk31xx.h"
#include <linux/wakelock.h> 

#ifdef MT6516
#include <mach/mt6516_devs.h>
#include <mach/mt6516_typedefs.h>
#include <mach/mt6516_gpio.h>
#include <mach/mt6516_pll.h>
#endif

#ifdef MT6573
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_pll.h>
#endif

#ifdef MT6575
#include <mach/mt6575_devs.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_gpio.h>
#include <mach/mt6575_pm_ldo.h>
#endif

#ifdef MT6573
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

#endif

#ifdef MT6575
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

#endif

/*-------------------------MT6516&MT6573 define-------------------------------*/
#ifdef MT6516
#define POWER_NONE_MACRO MT6516_POWER_NONE
#endif

#ifdef MT6573
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif

#ifdef MT6575
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif

#if 0
/* TEMPLATE */
#define GPIO_ALS_EINT_PIN         GPIO190
#define GPIO_ALS_EINT_PIN_M_GPIO  GPIO_MODE_00
#define GPIO_ALS_EINT_PIN_M_EINT  GPIO_MODE_01
#define GPIO_ALS_EINT_PIN_M_PWM  GPIO_MODE_04
#define CUST_EINT_ALS_NUM              3
#define CUST_EINT_ALS_DEBOUNCE_CN      0
#define CUST_EINT_ALS_POLARITY         CUST_EINT_POLARITY_LOW
#define CUST_EINT_ALS_SENSITIVE        CUST_EINT_LEVEL_SENSITIVE
#define CUST_EINT_ALS_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE
#endif



/******************************************************************************
 * configuration
*******************************************************************************/

/*----------------------------------------------------------------------------*/
#define stk31xx_DEV_NAME     "stk31xx"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO fmt, ##args)                 
/******************************************************************************
 * extern functions
*******************************************************************************/
#ifdef MT6516
extern void MT6516_EINTIRQUnmask(unsigned int line);
extern void MT6516_EINTIRQMask(unsigned int line);
extern void MT6516_EINT_Set_Polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void MT6516_EINT_Set_HW_Debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 MT6516_EINT_Set_Sensitivity(kal_uint8 eintno, kal_bool sens);
extern void MT6516_EINT_Registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);
#endif
/*----------------------------------------------------------------------------*/
#define mt6516_I2C_DATA_PORT        ((base) + 0x0000)
#define mt6516_I2C_SLAVE_ADDR       ((base) + 0x0004)
#define mt6516_I2C_INTR_MASK        ((base) + 0x0008)
#define mt6516_I2C_INTR_STAT        ((base) + 0x000c)
#define mt6516_I2C_CONTROL          ((base) + 0x0010)
#define mt6516_I2C_TRANSFER_LEN     ((base) + 0x0014)
#define mt6516_I2C_TRANSAC_LEN      ((base) + 0x0018)
#define mt6516_I2C_DELAY_LEN        ((base) + 0x001c)
#define mt6516_I2C_TIMING           ((base) + 0x0020)
#define mt6516_I2C_START            ((base) + 0x0024)
#define mt6516_I2C_FIFO_STAT        ((base) + 0x0030)
#define mt6516_I2C_FIFO_THRESH      ((base) + 0x0034)
#define mt6516_I2C_FIFO_ADDR_CLR    ((base) + 0x0038)
#define mt6516_I2C_IO_CONFIG        ((base) + 0x0040)
#define mt6516_I2C_DEBUG            ((base) + 0x0044)
#define mt6516_I2C_HS               ((base) + 0x0048)
#define mt6516_I2C_DEBUGSTAT        ((base) + 0x0064)
#define mt6516_I2C_DEBUGCTRL        ((base) + 0x0068)
/*----------------------------------------------------------------------------*/
static struct i2c_client *stk31xx_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id stk31xx_i2c_id[] = {{stk31xx_DEV_NAME,0},{}};
/*the adapter id & i2c address will be available in customization*/
static unsigned short stk31xx_force[] = {0x00, 0x00, I2C_CLIENT_END, I2C_CLIENT_END};
static const unsigned short *const stk31xx_forces[] = { stk31xx_force, NULL };
static struct i2c_client_address_data stk31xx_addr_data = { .forces = stk31xx_forces,};
/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int stk31xx_i2c_remove(struct i2c_client *client);
static int stk31xx_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int stk31xx_i2c_resume(struct i2c_client *client);
static int stk31xx_set_als_int_thd(struct i2c_client *client, u16 als_data_reg);



static struct stk31xx_priv *g_stk31xx_ptr = NULL;
static int is_near;
static unsigned int gain_setting;
/*----------------------------------------------------------------------------*/
typedef enum {
    STK_TRC_ALS_DATA= 0x0001,
    STK_TRC_PS_DATA = 0x0002,
    STK_TRC_EINT    = 0x0004,
    STK_TRC_IOCTL   = 0x0008,
    STK_TRC_I2C     = 0x0010,
    STK_TRC_CVT_ALS = 0x0020,
    STK_TRC_CVT_PS  = 0x0040,
    STK_TRC_DEBUG   = 0x8000,
} STK_TRC;
/*----------------------------------------------------------------------------*/
typedef enum {
    STK_BIT_ALS    = 1,
    STK_BIT_PS     = 2,
} STK_BIT;
/*----------------------------------------------------------------------------*/
struct stk31xx_i2c_addr {    /*define a series of i2c slave address*/
    u8  status;        /*Interrupt status*/
    u8  init;       /*device initialization */
    u8  als_cmd;    /*ALS command*/
    u8  als_dat1;   /*ALS MSB*/
    u8  als_dat0;   /*ALS LSB*/
    u8  ps_cmd;     /*PS command*/
    u8  ps_dat;     /*PS data*/
	u8	ps_gain;	/*PS gain*/
    u8  ps_high_thd;     /*PS INT threshold*/
	u8  ps_low_thd;     /*PS INT threshold*/
	u8  als_high_thd1;	/*ALS INT threshold high*/
	u8  als_high_thd2;	/*ALS INT threshold high*/
	u8  als_low_thd1;	/*ALS INT threshold low*/
	u8  als_low_thd2;	/*ALS INT threshold low*/
};
/*----------------------------------------------------------------------------*/
struct stk31xx_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct delayed_work  eint_work;

    /*i2c address group*/
    struct stk31xx_i2c_addr  addr;
    
    /*misc*/
    atomic_t    trace;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;


    /*data*/
    u16         als;
    u8          ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

    atomic_t    als_cmd_val;    /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_high_thd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_low_thd_val;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver stk31xx_i2c_driver = {	
	.probe      = stk31xx_i2c_probe,
	.remove     = stk31xx_i2c_remove,
	.detect     = stk31xx_i2c_detect,
	.suspend    = stk31xx_i2c_suspend,
	.resume     = stk31xx_i2c_resume,
	.id_table   = stk31xx_i2c_id,
	.address_data = &stk31xx_addr_data,
	.driver = {
		.owner          = THIS_MODULE,
		.name           = stk31xx_DEV_NAME,
	},
};

static struct stk31xx_priv *stk31xx_obj = NULL;
static struct platform_driver stk31xx_alsps_driver;
static int stk31xx_get_ps_value(struct stk31xx_priv *obj, u16 ps);
static int stk31xx_get_als_value(struct stk31xx_priv *obj, u16 als);
static int stk31xx_read_als(struct i2c_client *client, u16 *data);
static int stk31xx_read_ps(struct i2c_client *client, u8 *data);

struct wake_lock ps_lock;

/*----------------------------------------------------------------------------*/
int stk31xx_get_addr(struct alsps_hw *hw, struct stk31xx_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->status    = ALSPS_STATUS; 
	addr->als_cmd   = ALS_CMD;         
	addr->als_dat1  = ALS_DT1;
	addr->als_dat0  = ALS_DT2;
	addr->ps_cmd    = PS_CMD;
	addr->ps_high_thd    = PS_THDH;
	addr->ps_low_thd    = PS_THDL;
	addr->ps_dat    = PS_DT;
	addr->ps_gain = PS_GAIN;
	addr->als_high_thd1 = ALS_THDH1;
	addr->als_high_thd2 = ALS_THDH2;
	addr->als_low_thd1 = ALS_THDL1;
	addr->als_low_thd2 = ALS_THDL2;	
	
	return 0;
}
/*----------------------------------------------------------------------------*/
int stk31xx_get_timing(void)
{
return 200;
/*
	u32 base = I2C2_BASE; 
	return (__raw_readw(mt6516_I2C_HS) << 16) | (__raw_readw(mt6516_I2C_TIMING));
*/
}
/*----------------------------------------------------------------------------*/
/*
int stk31xx_config_timing(int sample_div, int step_div)
{
	u32 base = I2C2_BASE; 
	unsigned long tmp;

	tmp  = __raw_readw(mt6516_I2C_TIMING) & ~((0x7 << 8) | (0x1f << 0));
	tmp  = (sample_div & 0x7) << 8 | (step_div & 0x1f) << 0 | tmp;

	return (__raw_readw(mt6516_I2C_HS) << 16) | (tmp);
}
*/
/*----------------------------------------------------------------------------*/
int stk31xx_master_recv(struct i2c_client *client, u16 addr, u8 *buf ,int count)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	int ret = 0, retry = 0;
	int trc = atomic_read(&obj->trace);
	int max_try = atomic_read(&obj->i2c_retry);

	while(retry++ < max_try)
	{
		ret = hwmsen_read_block(client, addr, buf, count);
		if(ret == 0)
            break;
		udelay(100);
	}

	if(unlikely(trc))
	{
		if(trc & STK_TRC_I2C)
		{
			APS_LOG("(recv) %x %d %d %p [%02X]\n", msg.addr, msg.flags, msg.len, msg.buf, msg.buf[0]);    
		}

		if((retry != 1) && (trc & STK_TRC_DEBUG))
		{
			APS_LOG("(recv) %d/%d\n", retry-1, max_try); 

		}
	}

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	transmitted, else error code. */
	return (ret == 0) ? count : ret;
}
/*----------------------------------------------------------------------------*/
int stk31xx_master_send(struct i2c_client *client, u16 addr, u8 *buf ,int count)
{
	int ret = 0, retry = 0;
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;
	int trc = atomic_read(&obj->trace);
	int max_try = atomic_read(&obj->i2c_retry);


	while(retry++ < max_try)
	{
		ret = hwmsen_write_block(client, addr, buf, count);
		if (ret == 0)
		    break;
		udelay(100);
	}

	if(unlikely(trc))
	{
		if(trc & STK_TRC_I2C)
		{
			APS_LOG("(send) %x %d %d %p [%02X]\n", msg.addr, msg.flags, msg.len, msg.buf, msg.buf[0]);    
		}

		if((retry != 1) && (trc & STK_TRC_DEBUG))
		{
			APS_LOG("(send) %d/%d\n", retry-1, max_try);
		}
	}
	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	transmitted, else error code. */
	return (ret == 0) ? count : ret;
}
/*----------------------------------------------------------------------------*/
int stk31xx_read_als(struct i2c_client *client, u16 *data)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf[2];

	if(1 != (ret = stk31xx_master_recv(client, obj->addr.als_dat1, (char*)&buf[1], 1)))
	{
		APS_DBG("reads als data1 = %d\n", ret);
		return -EFAULT;
	}
	else if(1 != (ret = stk31xx_master_recv(client, obj->addr.als_dat0, (char*)&buf[0], 1)))
	{
		APS_DBG("reads als data2 = %d\n", ret);
		return -EFAULT;
	}
	
	*data = (buf[1] << 8) | (buf[0]);
	if(atomic_read(&obj->trace) & STK_TRC_ALS_DATA)
	{
		APS_DBG("ALS: 0x%04X\n", (u32)(*data));
	}
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_write_als(struct i2c_client *client, u8 data)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int ret = 0;
    
    ret = stk31xx_master_send(client, obj->addr.als_cmd, &data, 1);
	if(ret < 0)
	{
		APS_ERR("write als = %d\n", ret);
		return -EFAULT;
	}
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_read_ps(struct i2c_client *client, u8 *data)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;

	if(sizeof(*data) != (ret = stk31xx_master_recv(client, obj->addr.ps_dat, (char*)data, sizeof(*data))))
	{
		APS_ERR("reads ps data = %d\n", ret);
		return -EFAULT;
	} 

	if(atomic_read(&obj->trace) & STK_TRC_PS_DATA)
	{
		APS_DBG("PS:  0x%04X\n", (u32)(*data));
	}
	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_write_ps(struct i2c_client *client, u8 data)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = stk31xx_master_send(client, obj->addr.ps_cmd, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write ps = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_read_ps_gain(struct i2c_client *client, u8 *data)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;

	if(sizeof(*data) != (ret = stk31xx_master_recv(client, obj->addr.ps_gain, (char*)data, sizeof(*data))))
	{
		APS_ERR("reads ps gain = %d\n", ret);
		return -EFAULT;
	} 

	APS_DBG("PS gain:  0x%04X\n", (u32)(*data));

	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_write_ps_gain(struct i2c_client *client, u8 data)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = stk31xx_master_send(client, obj->addr.ps_gain, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write ps gain = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_write_ps_high_thd(struct i2c_client *client, u8 thd)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	u8 buf = thd;
	int ret = 0;

	if(sizeof(buf) != (ret = stk31xx_master_send(client, obj->addr.ps_high_thd, (char*)&buf, sizeof(buf))))
	{
		APS_ERR("write thd = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
int stk31xx_write_ps_low_thd(struct i2c_client *client, u8 thd)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	u8 buf = thd;
	int ret = 0;

	if(sizeof(buf) != (ret = stk31xx_master_send(client, obj->addr.ps_low_thd, (char*)&buf, sizeof(buf))))
	{
		APS_ERR("write thd = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void stk31xx_power(struct alsps_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	//APS_LOG("power %s\n", on ? "on" : "off");

	if(hw->power_id != POWER_NONE_MACRO)
	{
		if(power_on == on)
		{
			APS_LOG("ignore power control: %d\n", on);
		}
		else if(on)
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "stk31xx")) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "stk31xx")) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_enable_als(struct i2c_client *client, int enable)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int err, cur = 0, old = atomic_read(&obj->als_cmd_val);
	int trc = atomic_read(&obj->trace);
	hwm_sensor_data sensor_data;
	
	if(enable)
	{
		cur = old & (~SD_ALS);   
	}
	else
	{
		cur = old | (SD_ALS); 
	}
	
	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("%s: %08X, %08X, %d\n", __func__, cur, old, enable);
	}
	
	if(0 == (cur ^ old))
	{
		return 0;
	}
	
	if(0 == (err = stk31xx_write_als(client, cur))) 
	{
		atomic_set(&obj->als_cmd_val, cur);
	}
	
	if(enable)
	{
		atomic_set(&obj->als_deb_on, 1);
		atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)/(1000/HZ));
		//set_bit(STK_BIT_ALS,  &obj->pending_intr);
		//schedule_delayed_work(&obj->eint_work,900); //after enable the value is not accurate
		
		msleep(900);
		
		APS_LOG("stk31xx_enable_als, force read als\n");
		if((err = stk31xx_read_als(obj->client, &obj->als)))
		{
			APS_ERR("%s:stk31xx read als data: %d\n",__func__, err);
		}
		stk31xx_set_als_int_thd(obj->client, obj->als);
		//map and store data to hwm_sensor_data
		sensor_data.values[0] = stk31xx_get_als_value(obj, obj->als);
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
		APS_LOG("%s:als raw 0x%x -> value 0x%x \n", __func__, obj->als,sensor_data.values[0]);
		//let up layer to know
		if((err = hwmsen_get_interrupt_data(ID_LIGHT, &sensor_data)))
		{
		APS_ERR("%s:call hwmsen_get_interrupt_data fail = %d\n", __func__, err);
		}		
		
		set_bit(STK_BIT_ALS,  &obj->pending_intr);
		schedule_delayed_work(&obj->eint_work,200); //after enable the value is not accurate		
	}

	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("enable als (%d)\n", enable);
	}

	return err;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_enable_ps(struct i2c_client *client, int enable)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int err, cur = 0, old = atomic_read(&obj->ps_cmd_val);
	int trc = atomic_read(&obj->trace);

	if(enable)
	{
		cur = old & (~SD_PS); 
        wake_lock(&ps_lock);
	}
	else
	{
		cur = old | (SD_PS); 
        wake_unlock(&ps_lock);
	}
	
	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("%s: %08X, %08X, %d\n", __func__, cur, old, enable);
	}
	
	if(0 == (cur ^ old))
	{
		return 0;
	}
	
	if(0 == (err = stk31xx_write_ps(client, cur))) 
	{
		atomic_set(&obj->ps_cmd_val, cur);
	}
	
	if(enable)
	{
		atomic_set(&obj->ps_deb_on, 1);
		atomic_set(&obj->ps_deb_end, jiffies+atomic_read(&obj->ps_debounce)/(1000/HZ));
		set_bit(STK_BIT_PS,  &obj->pending_intr);
		schedule_delayed_work(&obj->eint_work,120);
	}

	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("enable ps  (%d)\n", enable);
	}

	return err;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_check_intr(struct i2c_client *client, u8 *status) 
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int err;
	//u8 status;

	//if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	//    return 0;

    err = stk31xx_master_recv(client, obj->addr.status, status, 1);
	if (err < 0)
	{
		APS_ERR("WARNING: read status error: %d\n", err);
		return -EFAULT;
	}
	APS_LOG("stk31xx_check_intr: read status reg: 0x%x\n", *status);
    
	if(*status & 0x10)
	{
		set_bit(STK_BIT_ALS, &obj->pending_intr);
	}
	else
	{
	   clear_bit(STK_BIT_ALS, &obj->pending_intr);
	}
	
	if(*status & 0x20)
	{
		set_bit(STK_BIT_PS,  &obj->pending_intr);
	}
	else
	{
	    clear_bit(STK_BIT_PS, &obj->pending_intr);
	}
	
	if(atomic_read(&obj->trace) & STK_TRC_DEBUG)
	{
		APS_LOG("check intr: 0x%02X => 0x%08lX\n", *status, obj->pending_intr);
	}


	return 0;
}

static int stk31xx_clear_intr(struct i2c_client *client, u8 status, u8 disable_flag)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int err;
	
	status &= 0x30;
    status = status & (~disable_flag);
    err = stk31xx_master_send(client, obj->addr.status, &status, 1);
    APS_LOG("stk31xx_clear_intr: set status reg: 0x%x\n", status);
	if (err < 0)
	{
		APS_ERR("ERROR: stk31xx_clear_intr clear intrrupt fail: %d\n", err);
		return -EFAULT;
	}



	return 0;
}

/*----------------------------------------------------------------------------*/
/*
static u16 stk31xx_lux2alscode(u16 input_lux)
{
	u32 ratio;
	u32 output_code;

	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	ratio = (stk31xx_obj->hw->als_value[10] << 10 ) / stk31xx_obj->hw->als_level[10];
	output_code = (input_lux<<10) / ratio;
	
	if (unlikely(output_code>=(1<<16)))
        output_code = (1<<16) -1;
	return (u16)output_code;
}
*/
/*----------------------------------------------------------------------------*/
static int stk31xx_set_als_int_thd(struct i2c_client *client, u16 als_data_reg) 
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int err;
	u32 als_thd_h, als_thd_l;
	u8 buf[2] = {0,0};		
		
    als_thd_h = als_data_reg + CONFIG_STK_ALS_CHANGE_THRESHOLD;
    als_thd_l = als_data_reg - CONFIG_STK_ALS_CHANGE_THRESHOLD;
    if (als_thd_h >= (1<<16))
        als_thd_h = (1<<16) -1;
    if (als_thd_l <0)
        als_thd_l = 0;
	APS_LOG("stk31xx_set_als_int_thd:als_thd_h:%d,als_thd_l:%d\n", als_thd_h, als_thd_l);	
		
    buf[0] = (u8) ((0xFF00 & als_thd_h) >> 8);
    buf[1] = (u8) (0x00FF & als_thd_h);
	//APS_LOG("%s:als_thd_h, buf[0]=0x%x, buf[1]=0x%x\n", __func__, buf[0], buf[1]);
    err = stk31xx_master_send(client, obj->addr.als_high_thd1, &buf, 1);
	if (err < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, err);
		return 0;
	}

    err = stk31xx_master_send(client, obj->addr.als_high_thd2, &(buf[1]), 1);
	if (err < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, err);
		return 0;
	}
	
    buf[0] = (u8) ((0xFF00 & als_thd_l) >> 8);
    buf[1] = (u8) (0x00FF & als_thd_l);
	//APS_LOG("%s:als_thd_l, buf[0]=0x%x, buf[1]=0x%x\n", __func__, buf[0], buf[1]);	
    err = stk31xx_master_send(client, obj->addr.als_low_thd1, &buf, 1);
	if (err < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, err);
		return 0;
	}	

    err = stk31xx_master_send(client, obj->addr.als_low_thd2, &(buf[1]), 1);
	if (err < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, err);
		return 0;
	}	

	return 0;

}

/*----------------------------------------------------------------------------*/
void stk31xx_eint_func(void)
{
	struct stk31xx_priv *obj = g_stk31xx_ptr;
	APS_LOG(" interrupt fuc\n");
	if(!obj)
	{
		return;
	}
	
	//schedule_work(&obj->eint_work);
	schedule_delayed_work(&obj->eint_work,0);
	if(atomic_read(&obj->trace) & STK_TRC_EINT)
	{
		APS_LOG("eint: als/ps intrs\n");
	}
}
/*----------------------------------------------------------------------------*/
static void stk31xx_eint_work(struct work_struct *work)
{
	struct stk31xx_priv *obj = g_stk31xx_ptr;
	int err;
	hwm_sensor_data sensor_data;
	u8 int_status,disable_flag = 0;
	
	memset(&sensor_data, 0, sizeof(sensor_data));

	APS_LOG(" eint work\n");
	
	if((err = stk31xx_check_intr(obj->client, &int_status)))
	{
		APS_ERR("check intrs fail: %d\n", err);
		msleep(30);
		#ifdef MT6516
		MT6516_EINTIRQUnmask(CUST_EINT_ALS_NUM);      
		#endif     
		#ifdef MT6573
		mt65xx_eint_unmask(CUST_EINT_ALS_NUM);      
		#endif
		#ifdef MT6575
		mt65xx_eint_unmask(CUST_EINT_ALS_NUM);      
		#endif			
		return;
	}

    APS_LOG(" &obj->pending_intr =%lx\n",obj->pending_intr);
	
	if((1<<STK_BIT_ALS) & obj->pending_intr)
	{
	  //get raw data
	  APS_LOG("stk als change\n");
	  disable_flag = 0x10;
	  if((err = stk31xx_read_als(obj->client, &obj->als)))
	  {
		 APS_ERR("stk31xx read als data: %d\n", err);;
	  }
	  stk31xx_set_als_int_thd(obj->client, obj->als);
	  //map and store data to hwm_sensor_data
 	  sensor_data.values[0] = stk31xx_get_als_value(obj, obj->als);
	  sensor_data.value_divide = 1;
	  sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
	  APS_LOG("%s:als raw 0x%x -> value 0x%x \n", __func__, obj->als,sensor_data.values[0]);
	  //let up layer to know
	  if((err = hwmsen_get_interrupt_data(ID_LIGHT, &sensor_data)))
	  {
		APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
	  }
	  
	}
	if((1<<STK_BIT_PS) &  obj->pending_intr)
	{
	  //get raw data
	  APS_LOG("stk ps change\n");
	  disable_flag |= 0x20;
      if((err = stk31xx_read_ps(obj->client, &obj->ps)))
      {
         APS_ERR("stk31xx read ps data: %d\n", err);
      }
    
	  //map and store data to hwm_sensor_data
	  sensor_data.values[0] = stk31xx_get_ps_value(obj, obj->ps);
	  sensor_data.value_divide = 1;
	  sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
	  APS_LOG("%s:ps raw 0x%x -> value 0x%x \n",__func__, obj->ps,sensor_data.values[0]);
	  //let up layer to know
	  if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
	  {
		APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
	  }
	}
	
	if(err = stk31xx_clear_intr(obj->client, int_status, disable_flag))
	{
		APS_ERR("stk31xx_clear_intr fail: %d\n", err);
		msleep(30);
	}	
		
	msleep(1);
	#ifdef MT6516
	MT6516_EINTIRQUnmask(CUST_EINT_ALS_NUM);      
	#endif     
	#ifdef MT6573
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);      
	#endif
	#ifdef MT6575
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);      
	#endif		
}
/*----------------------------------------------------------------------------*/
int stk31xx_setup_eint(struct i2c_client *client)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        

	g_stk31xx_ptr = obj;
	/*configure to GPIO function, external interrupt*/

  printk("ALS/PS interrupt pin = %d\n", GPIO_ALS_EINT_PIN);		
	
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
  mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);
	
	//mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	//mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	//mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, GPIO_PULL_ENABLE);
	//mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

#ifdef MT6516

	MT6516_EINT_Set_Sensitivity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
	MT6516_EINT_Set_Polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
	MT6516_EINT_Set_HW_Debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	MT6516_EINT_Registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_POLARITY, stk31xx_eint_func, 0);
	MT6516_EINTIRQUnmask(CUST_EINT_ALS_NUM);  
#endif
    //
#ifdef MT6573
	
    mt65xx_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
	mt65xx_eint_set_polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
	mt65xx_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_POLARITY, stk31xx_eint_func, 0);
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);  
#endif  

#ifdef MT6575
		mt65xx_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
		mt65xx_eint_set_polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
		mt65xx_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
		mt65xx_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_POLARITY, stk31xx_eint_func, 0);
		mt65xx_eint_unmask(CUST_EINT_ALS_NUM);	
#endif 

	
    return 0;
	
}
/*----------------------------------------------------------------------------*/
static int stk31xx_init_client(struct i2c_client *client)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);
	int err;
	u8 gain;
	u8 int_status;
	
	if((err = stk31xx_setup_eint(client)))
	{
		APS_ERR("setup eint: %d\n", err);
		return err;
	}
	
	if((err = stk31xx_check_intr(client, &int_status)))
	{
		APS_ERR("check intr: %d\n", err);
		//    return err;
	}	
	
	if(err = stk31xx_clear_intr(client, int_status, 0x30))
	{
		APS_ERR("clear intr: %d\n", err);	
	}
	
	if((err = stk31xx_write_als(client, atomic_read(&obj->als_cmd_val))))
	{
		APS_ERR("write als: %d\n", err);
		return err;
	}
	
	if((err = stk31xx_write_ps(client, atomic_read(&obj->ps_cmd_val))))
	{
		APS_ERR("write ps: %d\n", err);
		return err;        
	}
	
	if((err = stk31xx_write_ps_high_thd(client, atomic_read(&obj->ps_high_thd_val))))
	{
		APS_ERR("write thd: %d\n", err);
		return err;        
	}
	
	if((err = stk31xx_write_ps_low_thd(client, atomic_read(&obj->ps_low_thd_val))))
	{
		APS_ERR("write thd: %d\n", err);
		return err;        
	}
	
	/*Read PS Gain Register*/
	if((err = stk31xx_read_ps_gain(client, &gain)))
	{
		APS_ERR("read ps gain: %d\n", err);
		return err;
	}
	
	gain = (gain&0xF0)|gain_setting; //Setting PS Gain
	printk("PS GAIN SETTING = 0x%x\n",gain_setting);
	
	/*Write PS Gain*/
	if((err = stk31xx_write_ps_gain(client, gain)))
	{
		APS_ERR("write ps gain: %d\n", err);
		return err;
	}
	/*Set PS Gain END*/
	
	return 0;
}
/******************************************************************************
 * Sysfs attributes
*******************************************************************************/
static ssize_t stk31xx_show_config(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "(%d %d %d %d %d %d)\n", 
		atomic_read(&stk31xx_obj->i2c_retry), atomic_read(&stk31xx_obj->als_debounce), 
		atomic_read(&stk31xx_obj->ps_mask), atomic_read(&stk31xx_obj->ps_high_thd_val),atomic_read(&stk31xx_obj->ps_low_thd_val), atomic_read(&stk31xx_obj->ps_debounce));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_store_config(struct device_driver *ddri, const char *buf, size_t count)
{
	int retry, als_deb, ps_deb, mask, hthres, lthres;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	if(6 == sscanf(buf, "%d %d %d %d %d %d", &retry, &als_deb, &mask, &hthres, &lthres, &ps_deb))
	{ 
		atomic_set(&stk31xx_obj->i2c_retry, retry);
		atomic_set(&stk31xx_obj->als_debounce, als_deb);
		atomic_set(&stk31xx_obj->ps_mask, mask);
		atomic_set(&stk31xx_obj->ps_high_thd_val, hthres);    
		atomic_set(&stk31xx_obj->ps_low_thd_val, lthres);        
		atomic_set(&stk31xx_obj->ps_debounce, ps_deb);
	}
	else
	{
		APS_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_trace(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&stk31xx_obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_store_trace(struct device_driver *ddri, const char *buf, size_t count)
{
    int trace;
    if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&stk31xx_obj->trace, trace);
	}
	else 
	{
		APS_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_als(struct device_driver *ddri, char *buf)
{
	int res;
	
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	if((res = stk31xx_read_als(stk31xx_obj->client, &stk31xx_obj->als)))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
	else
	{
		return snprintf(buf, PAGE_SIZE, "0x%04X\n", stk31xx_obj->als);     
	}
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_ps(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	if((res = stk31xx_read_ps(stk31xx_obj->client, &stk31xx_obj->ps)))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
	else
	{
		return snprintf(buf, PAGE_SIZE, "0x%04X\n", stk31xx_obj->ps);     
	}
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_reg(struct device_driver *ddri, char *buf)
{
	u8 int_status;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	/*read*/
	stk31xx_check_intr(stk31xx_obj->client, &int_status);
	stk31xx_clear_intr(stk31xx_obj->client, int_status, 0x0);
	stk31xx_read_ps(stk31xx_obj->client, &stk31xx_obj->ps);
	stk31xx_read_als(stk31xx_obj->client, &stk31xx_obj->als);
	/*write*/
	stk31xx_write_als(stk31xx_obj->client, atomic_read(&stk31xx_obj->als_cmd_val));
	stk31xx_write_ps(stk31xx_obj->client, atomic_read(&stk31xx_obj->ps_cmd_val)); 
	stk31xx_write_ps_high_thd(stk31xx_obj->client, atomic_read(&stk31xx_obj->ps_high_thd_val));
	stk31xx_write_ps_low_thd(stk31xx_obj->client, atomic_read(&stk31xx_obj->ps_low_thd_val));
	return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_send(struct device_driver *ddri, char *buf)
{
    return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_store_send(struct device_driver *ddri, const char *buf, size_t count)
{
	int addr, cmd;
	u8 dat;

	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	else if(2 != sscanf(buf, "%x %x", &addr, &cmd))
	{
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	dat = (u8)cmd;
	APS_LOG("send(%02X, %02X) = %d\n", addr, cmd, 
	stk31xx_master_send(stk31xx_obj->client, (u16)addr, &dat, sizeof(dat)));
	
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_recv(struct device_driver *ddri, char *buf)
{
    return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_store_recv(struct device_driver *ddri, const char *buf, size_t count)
{
	int addr;
	u8 dat;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	else if(1 != sscanf(buf, "%x", &addr))
	{
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	APS_LOG("recv(%02X) = %d, 0x%02X\n", addr, 
	stk31xx_master_recv(stk31xx_obj->client, (u16)addr, (char*)&dat, sizeof(dat)), dat);
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_status(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	if(stk31xx_obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d, (%d %d) (%02X %02X) (%02X %02X %02X) (%02X %02X %02X)\n", 
			stk31xx_obj->hw->i2c_num, stk31xx_obj->hw->power_id, stk31xx_obj->hw->power_vol, stk31xx_obj->addr.init, 
			stk31xx_obj->addr.status,stk31xx_obj->addr.als_cmd, stk31xx_obj->addr.als_dat0, stk31xx_obj->addr.als_dat1,
			stk31xx_obj->addr.ps_cmd, stk31xx_obj->addr.ps_dat, stk31xx_obj->addr.ps_high_thd);
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	
	len += snprintf(buf+len, PAGE_SIZE-len, "REGS: %02X %02X %02X %02X %02lX %02lX\n", 
				atomic_read(&stk31xx_obj->als_cmd_val), atomic_read(&stk31xx_obj->ps_cmd_val), 
				atomic_read(&stk31xx_obj->ps_high_thd_val), atomic_read(&stk31xx_obj->ps_low_thd_val),stk31xx_obj->enable, stk31xx_obj->pending_intr);
	#ifdef MT6516
	len += snprintf(buf+len, PAGE_SIZE-len, "EINT: %d (%d %d %d %d)\n", mt_get_gpio_in(GPIO_ALS_EINT_PIN),
				CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_DEBOUNCE_CN);

	len += snprintf(buf+len, PAGE_SIZE-len, "GPIO: %d (%d %d %d %d)\n",	GPIO_ALS_EINT_PIN, 
				mt_get_gpio_dir(GPIO_ALS_EINT_PIN), mt_get_gpio_mode(GPIO_ALS_EINT_PIN), 
				mt_get_gpio_pull_enable(GPIO_ALS_EINT_PIN), mt_get_gpio_pull_select(GPIO_ALS_EINT_PIN));
	#endif

	len += snprintf(buf+len, PAGE_SIZE-len, "MISC: %d %d\n", atomic_read(&stk31xx_obj->als_suspend), atomic_read(&stk31xx_obj->ps_suspend));

	return len;
}
/*----------------------------------------------------------------------------*/
/*
static ssize_t stk31xx_show_i2c(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	u32 base = I2C2_BASE;

	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	len += snprintf(buf+len, PAGE_SIZE-len, "DATA_PORT      = 0x%08X\n", __raw_readl(mt6516_I2C_DATA_PORT    ));
	len += snprintf(buf+len, PAGE_SIZE-len, "SLAVE_ADDR     = 0x%08X\n", __raw_readl(mt6516_I2C_SLAVE_ADDR));
	len += snprintf(buf+len, PAGE_SIZE-len, "INTR_MASK      = 0x%08X\n", __raw_readl(mt6516_I2C_INTR_MASK));
	len += snprintf(buf+len, PAGE_SIZE-len, "INTR_STAT      = 0x%08X\n", __raw_readl(mt6516_I2C_INTR_STAT));
	len += snprintf(buf+len, PAGE_SIZE-len, "CONTROL        = 0x%08X\n", __raw_readl(mt6516_I2C_CONTROL));
	len += snprintf(buf+len, PAGE_SIZE-len, "TRANSFER_LEN   = 0x%08X\n", __raw_readl(mt6516_I2C_TRANSFER_LEN));
	len += snprintf(buf+len, PAGE_SIZE-len, "TRANSAC_LEN    = 0x%08X\n", __raw_readl(mt6516_I2C_TRANSAC_LEN));
	len += snprintf(buf+len, PAGE_SIZE-len, "DELAY_LEN      = 0x%08X\n", __raw_readl(mt6516_I2C_DELAY_LEN));
	len += snprintf(buf+len, PAGE_SIZE-len, "TIMING         = 0x%08X\n", __raw_readl(mt6516_I2C_TIMING));
	len += snprintf(buf+len, PAGE_SIZE-len, "START          = 0x%08X\n", __raw_readl(mt6516_I2C_START));
	len += snprintf(buf+len, PAGE_SIZE-len, "FIFO_STAT      = 0x%08X\n", __raw_readl(mt6516_I2C_FIFO_STAT));
	len += snprintf(buf+len, PAGE_SIZE-len, "FIFO_THRESH    = 0x%08X\n", __raw_readl(mt6516_I2C_FIFO_THRESH));
	len += snprintf(buf+len, PAGE_SIZE-len, "FIFO_ADDR_CLR  = 0x%08X\n", __raw_readl(mt6516_I2C_FIFO_ADDR_CLR));
	len += snprintf(buf+len, PAGE_SIZE-len, "IO_CONFIG      = 0x%08X\n", __raw_readl(mt6516_I2C_IO_CONFIG));
	len += snprintf(buf+len, PAGE_SIZE-len, "DEBUG          = 0x%08X\n", __raw_readl(mt6516_I2C_DEBUG));
	len += snprintf(buf+len, PAGE_SIZE-len, "HS             = 0x%08X\n", __raw_readl(mt6516_I2C_HS));
	len += snprintf(buf+len, PAGE_SIZE-len, "DEBUGSTAT      = 0x%08X\n", __raw_readl(mt6516_I2C_DEBUGSTAT));
	len += snprintf(buf+len, PAGE_SIZE-len, "DEBUGCTRL      = 0x%08X\n", __raw_readl(mt6516_I2C_DEBUGCTRL));    

	return len;
}
*/
/*----------------------------------------------------------------------------*/
/*
static ssize_t stk31xx_store_i2c(struct device_driver *ddri, const char *buf, size_t count)
{
	int sample_div, step_div;
	unsigned long tmp;
	u32 base = I2C2_BASE;    

	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	else if(2 != sscanf(buf, "%d %d", &sample_div, &step_div))
	{
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}
	tmp  = __raw_readw(mt6516_I2C_TIMING) & ~((0x7 << 8) | (0x1f << 0));
	tmp  = (sample_div & 0x7) << 8 | (step_div & 0x1f) << 0 | tmp;
	__raw_writew(tmp, mt6516_I2C_TIMING);        

	return count;
}
*/
/*----------------------------------------------------------------------------*/
#define IS_SPACE(CH) (((CH) == ' ') || ((CH) == '\n'))
/*----------------------------------------------------------------------------*/
static int read_int_from_buf(struct stk31xx_priv *obj, const char* buf, size_t count,
                             u32 data[], int len)
{
	int idx = 0;
	char *cur = (char*)buf, *end = (char*)(buf+count);

	while(idx < len)
	{
		while((cur < end) && IS_SPACE(*cur))
		{
			cur++;        
		}

		if(1 != sscanf(cur, "%d", &data[idx]))
		{
			break;
		}

		idx++; 
		while((cur < end) && !IS_SPACE(*cur))
		{
			cur++;
		}
	}
	return idx;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_alslv(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	for(idx = 0; idx < stk31xx_obj->als_level_num; idx++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "%d ", stk31xx_obj->hw->als_level[idx]);
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_store_alslv(struct device_driver *ddri, const char *buf, size_t count)
{
//	struct stk31xx_priv *obj;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	else if(!strcmp(buf, "def"))
	{
		memcpy(stk31xx_obj->als_level, stk31xx_obj->hw->als_level, sizeof(stk31xx_obj->als_level));
	}
	else if(stk31xx_obj->als_level_num != read_int_from_buf(stk31xx_obj, buf, count, 
			stk31xx_obj->hw->als_level, stk31xx_obj->als_level_num))
	{
		APS_ERR("invalid format: '%s'\n", buf);
	}    
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_show_alsval(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	
	for(idx = 0; idx < stk31xx_obj->als_value_num; idx++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "%d ", stk31xx_obj->hw->als_value[idx]);
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk31xx_store_alsval(struct device_driver *ddri, const char *buf, size_t count)
{
	if(!stk31xx_obj)
	{
		APS_ERR("stk31xx_obj is null!!\n");
		return 0;
	}
	else if(!strcmp(buf, "def"))
	{
		memcpy(stk31xx_obj->als_value, stk31xx_obj->hw->als_value, sizeof(stk31xx_obj->als_value));
	}
	else if(stk31xx_obj->als_value_num != read_int_from_buf(stk31xx_obj, buf, count, 
			stk31xx_obj->hw->als_value, stk31xx_obj->als_value_num))
	{
		APS_ERR("invalid format: '%s'\n", buf);
	}    
	return count;
}
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(als,     S_IWUSR | S_IRUGO, stk31xx_show_als,   NULL);
static DRIVER_ATTR(ps,      S_IWUSR | S_IRUGO, stk31xx_show_ps,    NULL);
static DRIVER_ATTR(config,  S_IWUSR | S_IRUGO, stk31xx_show_config,stk31xx_store_config);
static DRIVER_ATTR(alslv,   S_IWUSR | S_IRUGO, stk31xx_show_alslv, stk31xx_store_alslv);
static DRIVER_ATTR(alsval,  S_IWUSR | S_IRUGO, stk31xx_show_alsval,stk31xx_store_alsval);
static DRIVER_ATTR(trace,   S_IWUSR | S_IRUGO, stk31xx_show_trace, stk31xx_store_trace);
static DRIVER_ATTR(status,  S_IWUSR | S_IRUGO, stk31xx_show_status,  NULL);
static DRIVER_ATTR(send,    S_IWUSR | S_IRUGO, stk31xx_show_send,  stk31xx_store_send);
static DRIVER_ATTR(recv,    S_IWUSR | S_IRUGO, stk31xx_show_recv,  stk31xx_store_recv);
static DRIVER_ATTR(reg,     S_IWUSR | S_IRUGO, stk31xx_show_reg,   NULL);
//static DRIVER_ATTR(i2c,     S_IWUSR | S_IRUGO, stk31xx_show_i2c,   stk31xx_store_i2c);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *stk31xx_attr_list[] = {
    &driver_attr_als,
    &driver_attr_ps,    
    &driver_attr_trace,        /*trace log*/
    &driver_attr_config,
    &driver_attr_alslv,
    &driver_attr_alsval,
    &driver_attr_status,
    &driver_attr_send,
    &driver_attr_recv,
//    &driver_attr_i2c,
    &driver_attr_reg,
};

/*----------------------------------------------------------------------------*/
static int stk31xx_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(stk31xx_attr_list)/sizeof(stk31xx_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, stk31xx_attr_list[idx])))
		{            
			APS_ERR("driver_create_file (%s) = %d\n", stk31xx_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
	static int stk31xx_delete_attr(struct device_driver *driver)
	{
	int idx ,err = 0;
	int num = (int)(sizeof(stk31xx_attr_list)/sizeof(stk31xx_attr_list[0]));

	if (!driver)
	return -EINVAL;

	for (idx = 0; idx < num; idx++) 
	{
		driver_remove_file(driver, stk31xx_attr_list[idx]);
	}
	
	return err;
}
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int stk31xx_get_als_value(struct stk31xx_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als < obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
	
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		if (atomic_read(&obj->trace) & STK_TRC_CVT_ALS)
		{
			APS_DBG("ALS: %05d => %05d\n", als, obj->hw->als_value[idx]);
		}
		
		return obj->hw->als_value[idx];
	}
	else
	{
		if(atomic_read(&obj->trace) & STK_TRC_CVT_ALS)
		{
			APS_DBG("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		}
		return -1;
	}
}
/*----------------------------------------------------------------------------*/
static int stk31xx_get_ps_value(struct stk31xx_priv *obj, u16 ps)
{
	int val, mask = atomic_read(&obj->ps_mask);
	int invalid = 0;

	if( is_near != 1 && ps >= atomic_read(&obj->ps_high_thd_val))
	{
		val = 0;  /*close*/
		is_near = 1;		
	}
	else if( is_near != 0 && ps <= atomic_read(&obj->ps_low_thd_val))
	{
		val = 1;  /*far away*/
		is_near = 0;
	}else{
		if(is_near==1)
			val = 0;//near
		else
			val = 1;//far
	}
	
	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		if(unlikely(atomic_read(&obj->trace) & STK_TRC_CVT_PS))
		{
			if(mask)
			{
				APS_DBG("PS:  %05d => %05d [M] \n", ps, val);
			}
			else
			{
				APS_DBG("PS:  %05d => %05d\n", ps, val);
			}
		}
		return val;
		
	}	
	else
	{
		if(unlikely(atomic_read(&obj->trace) & STK_TRC_CVT_PS))
		{
			APS_DBG("PS:  %05d => %05d (-1)\n", ps, val);    
		}
		return -1;
	}	
}
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int stk31xx_open(struct inode *inode, struct file *file)
{
	file->private_data = stk31xx_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int stk31xx_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct stk31xx_priv *obj = i2c_get_clientdata(client);  
	int err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;

	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = stk31xx_enable_ps(obj->client, 1)))
				{
					APS_ERR("enable ps fail: %d\n", err); 
					goto err_out;
				}
				
				set_bit(STK_BIT_PS, &obj->enable);
			}
			else
			{
				if((err = stk31xx_enable_ps(obj->client, 0)))
				{
					APS_ERR("disable ps fail: %d\n", err); 
					goto err_out;
				}
				
				clear_bit(STK_BIT_PS, &obj->enable);
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(STK_BIT_PS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:    
			if((err = stk31xx_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = stk31xx_get_ps_value(obj, obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    
			if((err = stk31xx_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = obj->ps;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;            

		case ALSPS_SET_ALS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = stk31xx_enable_als(obj->client, 1)))
				{
					APS_ERR("enable als fail: %d\n", err); 
					goto err_out;
				}
				set_bit(STK_BIT_ALS, &obj->enable);
			}
			else
			{
				if((err = stk31xx_enable_als(obj->client, 0)))
				{
					APS_ERR("disable als fail: %d\n", err); 
					goto err_out;
				}
				clear_bit(STK_BIT_ALS, &obj->enable);
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(STK_BIT_ALS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA: 
			if((err = stk31xx_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = stk31xx_get_als_value(obj, obj->als);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			if((err = stk31xx_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = obj->als;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;    
}
/*----------------------------------------------------------------------------*/
static struct file_operations stk31xx_fops = {
	.owner = THIS_MODULE,
	.open = stk31xx_open,
	.release = stk31xx_release,
	.ioctl = stk31xx_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice stk31xx_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &stk31xx_fops,
};
/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);    
	int err;
	APS_FUN();    
/*
	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}
		
		atomic_set(&obj->als_suspend, 1);
		if((err = stk31xx_enable_als(client, 0)))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}

		atomic_set(&obj->ps_suspend, 1);
		if((err = stk31xx_enable_ps(client, 0)))
		{
			APS_ERR("disable ps:  %d\n", err);
			return err;
		}
		
		stk31xx_power(obj->hw, 0);
	}
*/
	return 0;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_resume(struct i2c_client *client)
{
	struct stk31xx_priv *obj = i2c_get_clientdata(client);        
	int err;
	APS_FUN();
/*
	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	stk31xx_power(obj->hw, 1);
	if((err = stk31xx_init_client(client)))
	{
		APS_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(STK_BIT_ALS, &obj->enable))
	{
		if((err = stk31xx_enable_als(client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err);        
		}
	}
	atomic_set(&obj->ps_suspend, 0);
	if(test_bit(STK_BIT_PS,  &obj->enable))
	{
		if((err = stk31xx_enable_ps(client, 1)))
		{
			APS_ERR("enable ps fail: %d\n", err);                
		}
	}
*/
	return 0;
}
/*----------------------------------------------------------------------------*/
static void stk31xx_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	struct stk31xx_priv *obj = container_of(h, struct stk31xx_priv, early_drv);   
	int err;
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}
	
	atomic_set(&obj->als_suspend, 1);    
	if((err = stk31xx_enable_als(obj->client, 0)))
	{
		APS_ERR("disable als fail: %d\n", err); 
	}
}
/*----------------------------------------------------------------------------*/
static void stk31xx_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	struct stk31xx_priv *obj = container_of(h, struct stk31xx_priv, early_drv);         
	int err;
	hwm_sensor_data sensor_data;
	
	memset(&sensor_data, 0, sizeof(sensor_data));
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	atomic_set(&obj->als_suspend, 0);
	if(test_bit(STK_BIT_ALS, &obj->enable))
	{
		if((err = stk31xx_enable_als(obj->client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err);        

		}
	}
}

int stk31xx_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct stk31xx_priv *obj = (struct stk31xx_priv *)self;
	
	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{				
				value = *(int *)buff_in;
				if(value)
				{
					if((err = stk31xx_enable_ps(obj->client, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(STK_BIT_PS, &obj->enable);
				}
				else
				{
					if((err = stk31xx_enable_ps(obj->client, 0)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(STK_BIT_PS, &obj->enable);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;				
				
				if((err = stk31xx_read_ps(obj->client, &obj->ps)))
				{
					err = -1;;
				}
				else
				{
					sensor_data->values[0] = stk31xx_get_ps_value(obj, obj->ps);
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				}				
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}

int stk31xx_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct stk31xx_priv *obj = (struct stk31xx_priv *)self;
	
	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;				
				if(value)
				{
					if((err = stk31xx_enable_als(obj->client, 1)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(STK_BIT_ALS, &obj->enable);
				}
				else
				{
					if((err = stk31xx_enable_als(obj->client, 0)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(STK_BIT_ALS, &obj->enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;
								
				if((err = stk31xx_read_als(obj->client, &obj->als)))
				{
					err = -1;;
				}
				else
				{
					sensor_data->values[0] = stk31xx_get_als_value(obj, obj->als);
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				}				
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, stk31xx_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct stk31xx_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;

    wake_lock_init(&ps_lock,WAKE_LOCK_SUSPEND,"ps wakelock");

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	stk31xx_obj = obj;
	obj->hw = get_cust_alsps_hw();
	stk31xx_get_addr(obj->hw, &obj->addr);

	INIT_DELAYED_WORK(&obj->eint_work, stk31xx_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 2000);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 1000);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->trace, 0x00);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->als_cmd_val, obj->hw->als_cmd_val);//range:32768 refresh 100ms
	atomic_set(&obj->ps_cmd_val, obj->hw->ps_cmd_val);
	gain_setting = obj->hw->ps_gain_setting;
	
  atomic_set(&obj->ps_high_thd_val, obj->hw->ps_high_thd_val );  //qijunp obj->hw->ps_high_threshold
	atomic_set(&obj->ps_low_thd_val, obj->hw->ps_low_thd_val );  //qijunp obj->hw->ps_low_threshold
	APS_LOG("stk31xx_i2c_probe, stage 3\n");
	is_near = 2;
	
	if(obj->hw->polling_mode_ps == 0)
	{
	  //atomic_set(&obj->ps_cmd_val,  0x02);
	  atomic_set(&obj->ps_cmd_val,  (obj->hw->ps_cmd_val) | 0x02);
	  APS_LOG("enable PS interrupt\n");
	}
	if(obj->hw->polling_mode_als == 0)
	{
	  //atomic_set(&obj->als_cmd_val,  0x02);//enable interrupt
	  atomic_set(&obj->als_cmd_val,  (obj->hw->als_cmd_val) | 0x02);
	  //atomic_add(&obj->als_cmd_val,  0x02);//enable interrupt
	  APS_LOG("enable ALS interrupt\n");
	}	
	APS_LOG("ps_cmd_val=0x%x\n", obj->ps_cmd_val);
	APS_LOG("als_cmd_val=0x%x\n", obj->als_cmd_val);
	
	APS_LOG("stk31xx_i2c_probe() OK!\n");
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);   
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	if(!(atomic_read(&obj->als_cmd_val) & SD_ALS))
	{
		set_bit(STK_BIT_ALS, &obj->enable);
	}
	
	if(!(atomic_read(&obj->ps_cmd_val) & SD_PS))
	{
		set_bit(STK_BIT_PS, &obj->enable);
	}
	
	stk31xx_i2c_client = client;

	
	if((err = stk31xx_init_client(client)))
	{
		goto exit_init_failed;
	}
	
	if((err = misc_register(&stk31xx_device)))
	{
		APS_ERR("stk31xx_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if((err = stk31xx_create_attr(&stk31xx_alsps_driver.driver)))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
	obj_ps.self = stk31xx_obj;
	if(1 == obj->hw->polling_mode_ps)
	{
	  obj_ps.polling = 1;
	}
	else
	{
	  obj_ps.polling = 0;//interrupt mode
	}
	obj_ps.sensor_operate = stk31xx_ps_operate;
	if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	obj_als.self = stk31xx_obj;
	if(1 == obj->hw->polling_mode_als)
	{
	  obj_als.polling = 1;
	}
	else
	{
	  obj_als.polling = 0;//interrupt mode
	}
	obj_als.sensor_operate = stk31xx_als_operate;
	if((err = hwmsen_attach(ID_LIGHT, &obj_als)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}


#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = stk31xx_early_suspend,
	obj->early_drv.resume   = stk31xx_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif

	APS_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&stk31xx_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
//	exit_kfree:
	kfree(obj);
	exit:
	stk31xx_i2c_client = NULL;           
	#ifdef MT6516        
	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	#endif
	APS_ERR("%s: err = %d\n", __func__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_i2c_remove(struct i2c_client *client)
{
	int err;	
	
	if((err = stk31xx_delete_attr(&stk31xx_i2c_driver.driver)))
	{
		APS_ERR("stk31xx_delete_attr fail: %d\n", err);
	} 

	if((err = misc_deregister(&stk31xx_device)))
	{
		APS_ERR("misc_deregister fail: %d\n", err);    
	}
	
	stk31xx_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	struct stk31xx_i2c_addr addr;

	printk("stk31xx_probe is called!\n");
	stk31xx_power(hw, 1);    
	stk31xx_get_addr(hw, &addr);
	stk31xx_force[0] = hw->i2c_num;
	stk31xx_force[1] = hw->i2c_addr[0];
	if(i2c_add_driver(&stk31xx_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int stk31xx_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	stk31xx_power(hw, 0);    
	i2c_del_driver(&stk31xx_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver stk31xx_alsps_driver = {
	.probe      = stk31xx_probe,
	.remove     = stk31xx_remove,    
	.driver     = {
		.name  = "als_ps",
		.owner = THIS_MODULE,
	}
};
/*----------------------------------------------------------------------------*/
static int __init stk31xx_init(void)
{
	APS_FUN();
	printk("stk31xx_init is called!\n");
	if(platform_driver_register(&stk31xx_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit stk31xx_exit(void)
{
	APS_FUN();
	platform_driver_unregister(&stk31xx_alsps_driver);
}
/*----------------------------------------------------------------------------*/
module_init(stk31xx_init);
module_exit(stk31xx_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("MingHsien Hsieh");
MODULE_DESCRIPTION("stk31xx proximity and light sensor driver");
MODULE_LICENSE("GPL");
