/* drivers/input/touchscreen/sis_i2c.c - I2C Touch panel driver for SiS 9200 family
 *
 * Copyright (C) 2011 SiS, Inc.
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

#include <linux/module.h>
#include <linux/delay.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include "sis_i2c.h"
#include <linux/linkage.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

/* Addresses to scan */
static const unsigned short normal_i2c[] = { SIS_SLAVE_ADDR, I2C_CLIENT_END };
static struct workqueue_struct *sis_wq;
struct sis_ts_data *ts_bak = 0;
struct sisTP_driver_data *TPInfo = NULL;
static void sis_tpinfo_clear(struct sisTP_driver_data *TPInfo, int max);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sis_ts_early_suspend(struct early_suspend *h);
static void sis_ts_late_resume(struct early_suspend *h);
#endif
#ifdef CONFIG_X86
//static const struct i2c_client_address_data addr_data;
/* Insmod parameters */
static int sis_ts_detect(struct i2c_client *client, struct i2c_board_info *info);
#endif

int sis_sent_command_to_fw(struct i2c_client *client, int wlength, unsigned char *wdata, int rlength, unsigned char *rdata)
{
    int ret = -1;
    struct i2c_msg msg[2];

    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].len = wlength;
    msg[0].buf = (unsigned char *)wdata;

    if(wlength != 1)
	{
	    ret = i2c_transfer(client->adapter, msg, 1);
	}
    else
	{
	    msg[1].addr = client->addr;
	    msg[1].flags = I2C_M_RD;
	    msg[1].len = rlength;
	    msg[1].buf = rdata;
 	    ret = i2c_transfer(client->adapter, msg, 2);
	}
    return ret;
}

int sis_ReadPacket(struct i2c_client *client, uint8_t cmd, uint8_t* buf)
{
    uint8_t tmpbuf[MAX_READ_BYTE_COUNT] = {0};
    int ret = -1;
	uint8_t offset = 0;
	bool ReadNext = false;
	uint8_t ByteCount = 0;
	uint8_t fingers = 0;

#ifndef _SMBUS_INTERFACE
    struct i2c_msg msg[2];
    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = (char *)(&cmd);
    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = MAX_READ_BYTE_COUNT;
    msg[1].buf = tmpbuf;
#endif

	do
    {
#ifdef _SMBUS_INTERFACE
        ret = i2c_smbus_read_block_data(client, cmd, tmpbuf);
#else
        ret = i2c_transfer(client->adapter, msg, 2);
        ret = tmpbuf[0] & 0xff;
#endif
#if 0
		printk(KERN_INFO "chaoban test: Buf_Data = %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
			buf[0], buf[1], buf[2], buf[3], buf[4],
			buf[5], buf[6], buf[7], buf[8], buf[9],
			buf[10], buf[11], buf[12], buf[13], buf[14],
			buf[15]
			);
#endif

        if (ret > MAX_READ_BYTE_COUNT)
        {
            return -1;
        }

		switch (ret)
		{
			case NO_TOUCH: 		//ByteCount:2,NoTouch
			case SINGLE_TOUCH:  //ByteCount:9,Single Point
			case LAST_ONE:		//ByteCount:7,Last one point
			case BUTTON_TOUCH:  //ByteCount:5,ButtonTouch
			case BUTTON_TOUCH_ONE_POINT: //ByteCount:10,ButtonTouch + Single Point  //CY
				ReadNext = false;//only read once packet
				break;

			case BUTTON_TOUCH_MULTI_TOUCH: //ByteCount:15,ButtonTouch + Multi Touch  //CY
			case MULTI_TOUCH:	//ByteCount:14,Multi Touch
				fingers = (tmpbuf[PKTINFO] & MSK_TOUCHNUM); //get total fingers' number
				if ((fingers <= 0) || (fingers > MAX_FINGERS))
        		{
        		    return -1;
        		}

        		ByteCount = 2 + (fingers * 5 ) + CRCCNT(fingers);   // Total byte count
				if (ret == BUTTON_TOUCH_MULTI_TOUCH) // for button touch event
				{									 // add one bytecount,BS
				  ByteCount += 1;
				}
        		ByteCount = ByteCount - ret;    // Byte counts that remain to be received
        		ReadNext= ByteCount > 0 ? true : false;		//whether are remained packets needed to read ?
        		break;

        	case LAST_TWO:  //ByteCount:12,Last two point
				ByteCount = ByteCount - ret;
				ReadNext= ByteCount > 0 ? true : false;
				break;

			default:    // I2C,SMBus Read fail or Data incorrect
         	    printk(KERN_INFO "chaoban test: Unknow bytecount = %d\n", ret);
        		return -1;
        		break;
		}

        memcpy(&buf[offset], &tmpbuf[CMD_BASE], ret);
        offset += ret;
    }
    while (ReadNext);

    return ret;
}

int check_gpio_interrupt(void)
{
    int ret = 0;
    //TODO
    //CHECK GPIO INTERRUPT STATUS BY YOUR PLATFORM SETTING.
    ret = gpio_get_value(GPIO_IRQ);
    return ret;
}

void ts_report_key(struct i2c_client *client, uint8_t keybit_state)
{
	int i = 0;
	uint8_t diff_keybit_state= 0x0; //check keybit_state is difference with pre_keybit_state
	uint8_t key_value = 0x0; //button location for binary
	uint8_t  key_pressed = 0x0; //button is up or down
	struct sis_ts_data *ts = i2c_get_clientdata(client);

	if (!ts)
	{
		printk("%s error: Missing Platform Data!\n", __func__);
		return;
	}

	diff_keybit_state = TPInfo->pre_keybit_state ^ keybit_state;

	if (diff_keybit_state)
	{
		for (i = 0; i < 8; i++)
		{
		    if ((diff_keybit_state >> i) & 0x01)
			{
				key_value = diff_keybit_state & (0x01 << i);
				key_pressed = (keybit_state >> i) & 0x01;
				switch (key_value)
				{
					case MSK_COMP:
						input_report_key(ts->input_dev, KEY_COMPOSE, key_pressed);
						break;
					case MSK_BACK:
						input_report_key(ts->input_dev, KEY_BACK, key_pressed);
						break;
					case MSK_MENU:
						input_report_key(ts->input_dev, KEY_MENU, key_pressed);
						break;
					case MSK_HOME:
						input_report_key(ts->input_dev, KEY_HOME, key_pressed);
						break;
					case MSK_NOBTN:
						//Release the button if it touched.
					default:
						break;
				}
			}
		}
		TPInfo->pre_keybit_state = keybit_state;
	}
}

static void sis_ts_work_func(struct work_struct *work)
{
	struct sis_ts_data *ts = container_of(work, struct sis_ts_data, work);
    int ret = -1;
	uint8_t buf[64] = {0};
	uint8_t i = 0, fingers = 0;
	uint8_t px = 0, py = 0, pstatus = 0;

    /* I2C or SMBUS block data read */
    ret = sis_ReadPacket(ts->client, SIS_CMD_NORMAL, buf);
#if 0
	if (ret > 2)
	{
		printk(KERN_INFO "chaoban test: Buf_Data = %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
			buf[0], buf[1], buf[2], buf[3], buf[4],
			buf[5], buf[6], buf[7], buf[8], buf[9],
			buf[10], buf[11], buf[12], buf[13], buf[14],
			buf[15],
			buf[16], buf[17], buf[18], buf[19], buf[20],
			buf[21], buf[22], buf[23], buf[24], buf[25],
			buf[26], buf[27], buf[28], buf[29], buf[30],
			buf[31]
			);
	}
#endif
	if (ret < 0)
	{
	    printk(KERN_INFO "chaoban test: ret = -1\n");
		goto err_free_allocate;
	}
	else if ((ret > 2) && (!(buf[1] & MSK_HAS_CRC)))
	{
	    printk(KERN_INFO "chaoban test: command type error\n");
		goto err_free_allocate;
	}

	if ((ret == 2) && (TPInfo->id == buf[0]))
	{
		goto label_send_report;
	}

	sis_tpinfo_clear(TPInfo, MAX_FINGERS);

	/* Parser and Get the sis9200 data */
	fingers = (buf[FORMAT_MODE] & MSK_TOUCHNUM);
	TPInfo->fingers = fingers = (fingers > MAX_FINGERS ? 0 : fingers);
	TPInfo->id = buf[0];

	if ((buf[FORMAT_MODE] & MSK_BUTTON_POINT) == BUTTON_TOUCH_SERIAL)
	{
		int temp_fingers = 0;
		if (fingers > 1)
		{
			 temp_fingers = 2; // when fingers is >= 2 , BS is placed at the same position
		}
		else
		{
			 temp_fingers = fingers;
		}
		ts_report_key(ts->client, buf[BUTTON_STATE + temp_fingers * 5]);
										//buf[BUTTON_STATE + temp_fingers * 5]: BS location
	}
	else
	{
		if (TPInfo->pre_keybit_state)
	  	{
			ts_report_key(ts->client, 0x0);//clear for polling
	  	}
	}

	for (i = 0; i < fingers; i++)
	{
        pstatus = 2 + (i * 5) + 2 * (i >> 1);    // Calc point status
		if (((buf[FORMAT_MODE] & MSK_BUTTON_POINT) == BUTTON_TOUCH_SERIAL) && i > 1)
		{
			pstatus += 1; 					// for button event and above 3 points
		}
	    px = pstatus + 1;                   // Calc point x_coord
	    py = px + 2;                        // Calc point y_coord

		TPInfo->pt[i].bPressure = (buf[pstatus] & MSK_PSTATE) == TOUCHDOWN ? 1 : 0;
		TPInfo->pt[i].bWidth = (buf[pstatus] & MSK_PSTATE) == TOUCHDOWN ? 1 : 0;
		TPInfo->pt[i].id = (buf[pstatus] & MSK_PID) >> 4;
		TPInfo->pt[i].x = (((buf[px] & 0xff) << 8) | (buf[px + 1] & 0xff));
        TPInfo->pt[i].y = (((buf[py] & 0xff) << 8) | (buf[py + 1] & 0xff));
	}
#if 0
	for (i = 0; i < fingers; i++)
	{
		printk(KERN_INFO "chaoban test: x[%d] = %d, y[%d] = %d\n", i, TPInfo->pt[i].x, i, TPInfo->pt[i].y);
	}
#endif

label_send_report:
    /* Report co-ordinates to the multi-touch stack */
	i = 0;
	do
	{
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, TPInfo->pt[i].bPressure);
//		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, TPInfo->pt[i].bWidth);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, TPInfo->pt[i].id);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, TPInfo->pt[i].x);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, TPInfo->pt[i].y);
//		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, TPInfo->pt[i].id);	//Android 2.3
		input_mt_sync(ts->input_dev);
		i++;
	}
	while ((i < TPInfo->fingers) && (i < MAX_FINGERS));

	input_sync(ts->input_dev);

err_free_allocate:

    if (ts->use_irq)
    {
#ifdef _INT_MODE_1 //case 1 mode
	    //TODO: After interrupt status low, read i2c bus data by polling, until interrupt status is high
	    ret = check_gpio_interrupt();	//interrupt pin is still LOW, read data until interrupt pin is released.
	    if (!ret)
	    {
	        hrtimer_start(&ts->timer, ktime_set(0, TIMER_NS), HRTIMER_MODE_REL);
	    }
	    else
	    {
			if (TPInfo->pre_keybit_state)
			{
				ts_report_key(ts->client, 0x0);//clear for interrupt
			}
        	enable_irq(ts->client->irq);
	    }
#else // case 2 mode
		enable_irq(ts->client->irq);
#endif
	}
	else
	{	// For Polling mode
	    hrtimer_start(&ts->timer, ktime_set(0, TIMER_NS), HRTIMER_MODE_REL);
	}

    return;
}

static void sis_tpinfo_clear(struct sisTP_driver_data *TPInfo, int max)
{
	int i = 0;
	for(i = 0; i < max; i++)
	{
		TPInfo->pt[i].id = -1;
		TPInfo->pt[i].touch = -1;
		TPInfo->pt[i].x = 0;
		TPInfo->pt[i].y = 0;
		TPInfo->pt[i].bPressure = 0;
		TPInfo->pt[i].bWidth = 0;
	}
	TPInfo->CRC = 0x0;
	TPInfo->id = 0x0;
	TPInfo->fingers = 0;
}

static enum hrtimer_restart sis_ts_timer_func(struct hrtimer *timer)
{
	struct sis_ts_data *ts = container_of(timer, struct sis_ts_data, timer);
	queue_work(sis_wq, &ts->work);
	return HRTIMER_NORESTART;
}

static irqreturn_t sis_ts_irq_handler(int irq, void *dev_id)
{
	struct sis_ts_data *ts = dev_id;
	disable_irq_nosync(ts->client->irq);
	queue_work(sis_wq, &ts->work);
	return IRQ_HANDLED;
}

static int initial_irq(void)
{
	int ret = 0;
#ifdef _I2C_INT_ENABLE
	/* initialize gpio and interrupt pins */
	/* TODO */
	ret = gpio_request(GPIO_IRQ, "GPIO_133");	// ex. GPIO_133 for interrupt mode
	if (ret < 0)
	{
		// Set Active Low. Please reference the file include/linux/interrupt.h
		printk(KERN_ERR "sis_ts_probe: Failed to gpio_request\n");
		printk(KERN_ERR "sis_ts_probe: Fail : gpio_request was called before this driver call\n");
	}
#else
	ret = -1;
#endif
	return ret;
}


#ifdef CONFIG_FW_SUPPORT_POWERMODE
bool sis_check_fw_ready(struct i2c_client *client)
{
  	int ret = 0, retry = 0, check_num = 10;
	unsigned char read_cmd;
	unsigned char rdata[MAX_READ_BYTE_COUNT] = {0};
	unsigned char CheckI2C_Address[MAX_READ_BYTE_COUNT] = {0x88,0x0e,0x70,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6f,0x93};

	do
	{
		ret = sis_sent_command_to_fw(client, MAX_BYTE, CheckI2C_Address, MAX_BYTE,rdata);

	  if (ret < 0)
	  {
		  printk(KERN_ERR "sis_check_ready: i2c_transfer write error %d\n", ret);
		  if (check_num != 0 )
		  {
			  printk(KERN_ERR "sis_check_ready: retry_count- %d\n", check_num);
			  check_num--;
			  retry = 1 ;
		  }
		  else
		  {
			  printk(KERN_ERR "sis_check_ready: I2C not ready\n");
			  retry = 0 ;
		  }
	  }
	  else
	  {
		  read_cmd = SIS_CMD_NORMAL;
		  ret = sis_sent_command_to_fw(client, ONE_BYTE, &read_cmd, MAX_BYTE, rdata);
		  if (ret < 0)
		  {
			  printk(KERN_ERR "sis_check_ready: i2c_transfer read error %d\n", ret);
			  if (check_num != 0 )
			  {
				  printk(KERN_ERR "sis_check_ready: retry_count- %d\n", check_num);
				  check_num--;
				  retry = 1 ;
			  }
			  else
			  {
				  printk(KERN_ERR "sis_check_ready: I2C not ready\n");
				  retry = 0 ;
			  }
		  }
		  else
		  {
			  ret = rdata[0];
			  #if 0
			  for ( i = 0; i <= 15; i++ )
			  {
				  printk(KERN_INFO "%02x ",rdata[i]);
			  }
			  printk(KERN_INFO "\n");
			  #endif

			  if (ret == 6 && rdata[4]== 0x01)
			  {
				  return true;
			  }
			  else
			  {
				  if (check_num != 0 )
				  {
					  printk(KERN_ERR "sis_check_ready: retry_count- %d\n",check_num);
					  check_num--;
					  retry = 1 ;
				  }
				  else
				  {
					  printk(KERN_ERR "sis_check_ready: I2C not ready\n");
					  retry = 0 ;
				  }
			  }
		  }
		  if (retry == 1) msleep(1000);
	  }
	}while(retry);

	return false;
}

uint8_t sis_check_fw_mode(struct i2c_client *client, uint8_t mode, bool can_read00)
{
  	int ret;
	uint8_t tmpbuf[MAX_READ_BYTE_COUNT] = {0};
    uint8_t cmd[5] = {0};
    uint16_t crc = 0;
	char rcmd;
	//struct i2c_msg msg[2];
	//struct sis_ts_data *ts = i2c_get_clientdata(client);

  	cmd[0] = SIS_CMD_POWERMODE; //command
    cmd[1] = 0x03; //bytecount
    cmd[2] = mode; //read power mode
    crc = (crc<<8) ^ crc16tab[((crc>>8) ^ cmd[2] )&0x00FF];
    cmd[3] = (crc >> 8)& 0xff;
    cmd[4] = crc & 0xff;

#if 0
	msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].len = 5;
    msg[0].buf = (char *)(cmd);

	ret = i2c_transfer(client->adapter, msg, 1);
#endif

#if 0
	printk(KERN_INFO "command:%02x\n", mode);
	int i;
	for ( i = 0; i <= 15; i++ )
	{
		printk(KERN_INFO "%02x ", cmd[i]);
	}
	printk(KERN_INFO "\n");
#endif

	ret = sis_sent_command_to_fw(client, FIVE_BYTE, cmd, MAX_BYTE, tmpbuf);

	if (ret < 0)
	{
		printk(KERN_ERR "sis_check_fw_mode: i2c_transfer write error - %02x\n", mode);
	}

	if (can_read00)
	{
		msleep(100);
		rcmd = SIS_CMD_NORMAL;
#if 0
		msg[0].addr = client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = (char *)(&rcmd);
		msg[1].addr = client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = 4;
		msg[1].buf = tmpbuf;

		ret = i2c_transfer(client->adapter, msg, 2);
#endif
		ret = sis_sent_command_to_fw(client, ONE_BYTE, &rcmd, MAX_BYTE, tmpbuf);

		if(ret < 0)
		{
			printk(KERN_ERR "sis_check_fw_mode: SIS_CMD_NORMAL -i2c_transfer write error\n");
		}
		else
		{
#if 0
			printk(KERN_INFO "get:\n");
			int j;
			for ( j = 0; j <= 15; j++ )
			{
				printk(KERN_INFO "%02x ", tmpbuf[j]);
			}
			printk(KERN_INFO "\n");
#endif
			if (tmpbuf[0] == 0x03)
			{
				return tmpbuf[1];
			}
		}
	}
	return -1;
}

void sis_fw_softreset(struct i2c_client *client)
{
	//re-calibration
	uint8_t tmpbuf[MAX_READ_BYTE_COUNT] = {0};
	uint8_t wcmd = SIS_CMD_SOFTRESET;
	int ret = 0;

	ret = sis_sent_command_to_fw(client, ONE_BYTE, &wcmd, MAX_BYTE,tmpbuf);

	if (ret < 0)
	{
		printk(KERN_ERR "sis_fw_softreset: i2c write error %d\n", ret);
	}
	else
	{
#if 0
		printk(KERN_INFO "SOFTRESET: ");
		int j;
		for ( j = 0; j <= 15; j++ )
		{
			printk(KERN_INFO "%02x ", tmpbuf[j]);
		}
		printk(KERN_INFO "\n");
#endif
		if(tmpbuf[0] == 0x04 && tmpbuf[1] == 0x0 && tmpbuf[2] == 0x80)
		{
			sis_check_fw_ready(client);
		}
		else
		{
			printk(KERN_ERR "sis_fw_softreset: SOFTRESET NACK %d\n", ret);
		}
	}
}
#endif //CONFIG_FW_SUPPORT_POWERMODE

static int sis_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct sis_ts_data *ts = NULL;
	struct sis_i2c_rmi_platform_data *pdata = NULL;
    printk(KERN_INFO "sis_ts_probe\n");

    TPInfo = kzalloc(sizeof(struct sisTP_driver_data), GFP_KERNEL);
    if (TPInfo == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts_bak = ts;

	//1. Init Work queue and necessary buffers
	INIT_WORK(&ts->work, sis_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);
    pdata = client->dev.platform_data;

	if (pdata)
		ts->power = pdata->power;
	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0) {
			printk(KERN_ERR "sis_ts_probe power on failed\n");
			goto err_power_failed;
		}
	}

	//2. Allocate input device
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "sis_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = "SiS 9200 family I2C touchscreen";

	set_bit(EV_ABS, ts->input_dev->evbit);
    set_bit(ABS_MT_POSITION_X, ts->input_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, ts->input_dev->absbit);
    set_bit(ABS_MT_TOUCH_MAJOR, ts->input_dev->absbit);
    set_bit(ABS_MT_WIDTH_MAJOR, ts->input_dev->absbit);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, SIS_MAX_X, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, SIS_MAX_Y, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 1, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 1, 0, 0);

    /* add for touch keys */
	set_bit(KEY_COMPOSE, ts->input_dev->keybit);
	set_bit(KEY_BACK, ts->input_dev->keybit);
	set_bit(KEY_MENU, ts->input_dev->keybit);
	set_bit(KEY_HOME, ts->input_dev->keybit);

	//3. Register input device to core
	ret = input_register_device(ts->input_dev);

	if (ret) {
		printk(KERN_ERR "sis_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	//4. irp or timer setup
	ret = initial_irq();

	if (ret < 0) {

	}
	else
	{
		client->irq = gpio_to_irq(GPIO_IRQ);
		ret = request_irq(client->irq, sis_ts_irq_handler, IRQF_TRIGGER_FALLING, client->name, ts);
		if (ret == 0) {
		   ts->use_irq = 1;
		}
		else {
			dev_err(&client->dev, "request_irq failed\n");
		}
	}

	hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ts->timer.function = sis_ts_timer_func;

	if (!ts->use_irq) {
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = sis_ts_early_suspend;
	ts->early_suspend.resume = sis_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
	printk(KERN_INFO "sis_ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");

	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
err_power_failed:
	kfree(ts);
err_alloc_data_failed:
	return ret;
}

static int sis_ts_remove(struct i2c_client *client)
{
	struct sis_ts_data *ts = i2c_get_clientdata(client);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int sis_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = 0;
	struct sis_ts_data *ts = i2c_get_clientdata(client);
#ifdef _SMBUS_INTERFACE
	uint8_t tmpbuf[MAX_READ_BYTE_COUNT] = {0};
	uint8_t cmd[5] = {0};
	uint16_t crc = 0;
#endif

#ifdef CONFIG_FW_SUPPORT_POWERMODE
	int retry = 5;
#endif

	TPInfo->pre_keybit_state = 0x0;

	if (ts->use_irq)
		disable_irq(client->irq);
	else
		hrtimer_cancel(&ts->timer);
//	ret = cancel_work_sync(&ts->work);
	flush_scheduled_work();
	if (ret && ts->use_irq) /* if work was pending disable-count is now 2 */
		enable_irq(client->irq);

#ifdef CONFIG_FW_SUPPORT_POWERMODE
#ifdef _SMBUS_INTERFACE
		cmd[0] = 0x90; //command
		cmd[1] = 0x03; //bytecount
		cmd[2] = 0x80; //10000000,Deep-Sleep Mode
//		cmd[2] = 0x81; //10000001,Idle Mode
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ cmd[2] )&0x00FF];
		cmd[3] = (crc >> 8)& 0xff;
		cmd[4] = crc & 0xff;
		ret = i2c_smbus_read_block_data(client, cmd, tmpbuf);
#else
		sis_check_fw_mode(client, WRITE_DEEPSLEEP_MODE, false);//Change to Deepsleep Mode
		do
		{
			msleep(1000);
		  	if (retry == 0)
		  		printk(KERN_INFO "sis_ts_suspend: change mode failed\n");
		  	else
		  		retry --;
		}
		while (retry > 0 && (sis_check_fw_mode(client, READ_POWERMODE, true) & MSK_POWERMODE) != DEEPSLEEP_MODE);
#endif
#endif
	if (ts->power) {
		ret = ts->power(0);
		if (ret < 0)
			printk(KERN_ERR "sis_ts_suspend power off failed\n");
	}
	return 0;
}

static int sis_ts_resume(struct i2c_client *client)
{
	int ret = 0;
	struct sis_ts_data *ts = i2c_get_clientdata(client);

#ifdef _SMBUS_INTERFACE
	uint8_t tmpbuf[MAX_READ_BYTE_COUNT] = {0};
	uint8_t cmd[5] = {0};
	uint16_t crc = 0;
#endif
#ifdef CONFIG_FW_SUPPORT_POWERMODE
	int retry = 5;
#endif
	if (ts->power)
	{
		ret = ts->power(1);
		if (ret < 0)
			printk(KERN_ERR "sis_ts_resume power on failed\n");
	}

#ifdef CONFIG_FW_SUPPORT_POWERMODE
	if( (sis_check_fw_mode(client, READ_POWERMODE, true) & MSK_POWERMODE) != ACTIVE_MODE )
	{
#ifdef _SMBUS_INTERFACE
//		ret = i2c_smbus_write_byte_data(client, 0xf0, 0x86); /* deep sleep */
		cmd[0] = 0x90; //command
		cmd[1] = 0x03; //bytecount
		cmd[2] = 0x82; //10000010,Active Mode
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ cmd[2] )&0x00FF];
		cmd[3] = (crc >> 8)& 0xff;
		cmd[4] = crc & 0xff;
		ret = i2c_smbus_read_block_data(client, cmd, tmpbuf);
#else
		sis_check_fw_mode(client, WRITE_ACTIVE_MODE, false); //Change to Active Mode
		do
		{
			msleep(1000);
			if (retry == 0)
				printk(KERN_INFO "sis_ts_resume: change mode failed\n");
		  	else
		  		retry --;
		}
		while (retry > 0 && (sis_check_fw_mode(client, READ_POWERMODE, true) & MSK_POWERMODE) != ACTIVE_MODE);
#endif
		sis_fw_softreset(client);
	}
	else
	{
		printk(KERN_ERR "sis_ts_resume Active mode\n");
	}
#endif

	if (ts->use_irq)
		enable_irq(client->irq);
	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sis_ts_early_suspend(struct early_suspend *h)
{
	struct sis_ts_data *ts;
	TPInfo->pre_keybit_state = 0x0;
	ts = container_of(h, struct sis_ts_data, early_suspend);
	sis_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void sis_ts_late_resume(struct early_suspend *h)
{
	struct sis_ts_data *ts;
	ts = container_of(h, struct sis_ts_data, early_suspend);
	sis_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id sis_ts_id[] = {
	{ SIS_I2C_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, sis_ts_id);

static struct i2c_driver sis_ts_driver = {
	.probe		= sis_ts_probe,
	.remove		= sis_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= sis_ts_suspend,
	.resume		= sis_ts_resume,
#endif
#ifdef CONFIG_X86
    .class      = I2C_CLASS_HWMON,
    .detect		= sis_ts_detect,
	.address_list	= normal_i2c,
#endif
	.id_table	= sis_ts_id,
	.driver = {
		.name	= SIS_I2C_NAME,
	},
};

static int __devinit sis_ts_init(void)
{
	printk( KERN_INFO "sis_ts_init\n" );
	sis_wq = create_singlethread_workqueue("sis_wq");

	if (!sis_wq)
		return -ENOMEM;
	return i2c_add_driver(&sis_ts_driver);
}

#ifdef CONFIG_X86
/* Return 0 if detection is successful, -ENODEV otherwise */
static int sis_ts_detect(struct i2c_client *client,
		       struct i2c_board_info *info)
{
	const char *type_name;
    printk(KERN_INFO "sis_ts_detect\n");
	type_name = "sis_i2c_ts";
	strlcpy(info->type, type_name, I2C_NAME_SIZE);
	return 0;
}
#endif

static void __exit sis_ts_exit(void)
{
	printk(KERN_INFO "sis_ts_exit\n");
	i2c_del_driver(&sis_ts_driver);
	if (sis_wq)
		destroy_workqueue(sis_wq);
}

asmlinkage long sys_sis_I2C_stop( void )
{
    printk(KERN_INFO "sys_sis_I2C_stop\n");

    flush_workqueue(sis_wq);

    msleep(200);

    if ( ts_bak == 0 )
    	return -13;

	if (ts_bak->use_irq)
	{
		//disable_irq_nosync(ts_bak->client->irq);
		disable_irq(ts_bak->client->irq);
	}
	hrtimer_cancel(&ts_bak->timer);

    return 0;
}

asmlinkage long sys_sis_I2C_start( void )
{
    printk(KERN_INFO "sys_sis_I2C_start\n");

    msleep(200);

    if (ts_bak == 0)
    	return -13;

	if (ts_bak->use_irq)
		enable_irq(ts_bak->client->irq);
	else
		hrtimer_start(&ts_bak->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

    return 0;
}

#define BUFFER_SIZE 16
asmlinkage long sys_sis_I2C_IO( unsigned char* data, int size )
{

    unsigned char cmd;
    int ret = 0;
    int i;
    unsigned char *kdata;

#ifndef _SMBUS_INTERFACE
	struct i2c_msg msg[2];
#endif

    printk(KERN_INFO "sys_sis_I2C_IO\n");

    if (ts_bak == 0)
    	return -13;

    ret = access_ok(VERIFY_WRITE, data, BUFFER_SIZE);
    if (!ret) {
        printk(KERN_INFO "cannot access user space memory\n");
        return -11;
    }

    kdata = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (kdata == 0)
    	return -12;

    ret = copy_from_user(kdata, data, BUFFER_SIZE);
    if (ret) {
        printk(KERN_INFO "copy_from_user fail\n");
        kfree(kdata);
        return -14;
    }

    cmd = kdata[0];

    printk(KERN_INFO "io cmd=%02x\n", cmd);

    if (size > 1)
    {
        printk(KERN_INFO "write\n");
#ifdef _SMBUS_INTERFACE
        ret = i2c_smbus_write_block_data(ts_bak->client, cmd, size-1, kdata+1);
        ret = i2c_smbus_read_block_data(ts_bak->client, 0x02, kdata);
#else
//Write
        msg[0].addr = ts_bak->client->addr;
        msg[0].flags = 0;
        msg[0].len = size;
        msg[0].buf = (unsigned char *)kdata;

        ret = i2c_transfer(ts_bak->client->adapter, msg, 1);
        if (ret < 0) {
            printk(KERN_INFO "i2c_transfer write error %d\n", ret);
			kfree(kdata);
			return -21;
        }
//Read
        cmd = 0x00;
        msg[0].addr = ts_bak->client->addr;
        msg[0].flags = 0;//Write data
        msg[0].len = 1;
        msg[0].buf = (unsigned char *)(&cmd);

        msg[1].addr = ts_bak->client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 16;
        msg[1].buf = (unsigned char *)kdata;

        ret = i2c_transfer(ts_bak->client->adapter, msg, 2);
        if (ret < 0) {
            printk(KERN_INFO "i2c_transfer read error %d\n", ret);
			kfree(kdata);
			return -21;
        }
#endif
    }
    else
    {
//		printk(KERN_INFO "read\n" );
#ifdef _SMBUS_INTERFACE
        ret = i2c_smbus_read_block_data( ts_bak->client, cmd, kdata );
#else
		msg[0].addr = ts_bak->client->addr;
        msg[0].flags = 0;//Write data
        msg[0].len = 1;
        msg[0].buf = (unsigned char *)(&cmd);

        msg[1].addr = ts_bak->client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 16;
        msg[1].buf = (unsigned char *)kdata;

        ret = i2c_transfer(ts_bak->client->adapter, msg, 2);
        if (ret < 0) {
            printk(KERN_INFO "i2c_transfer read error %d\n", ret);
			kfree( kdata );
			return -21;
        }
#endif
    }

#ifndef _SMBUS_INTERFACE
    ret = kdata[0];

    for ( i = 0; i < BUFFER_SIZE - 1; i++ ) {
	    kdata[i] = kdata[i+1];
    }
#endif

    printk(KERN_INFO "%d\n", ret);

    for ( i = 0; i < ret; i++ )
    {
        printk(KERN_INFO "%x ", kdata[i]);
    }

    printk( "\n" );

    if ( copy_to_user( data, kdata, BUFFER_SIZE ) )
    {
        printk(KERN_INFO "copy_to_user fail\n" );
        ret = -19;
    }

    kfree( kdata );

    return ret;
}
#undef BUFFER_SIZE

module_init(sis_ts_init);
module_exit(sis_ts_exit);
MODULE_DESCRIPTION("SiS 9200 Family Touchscreen Driver");
MODULE_LICENSE("GPL");
