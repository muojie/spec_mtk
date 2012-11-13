/*
 * Driver for Pixcir I2C touchscreen controllers.
 *
 * Copyright (C) 2010-2012 Pixcir, Inc.
 *
 * pixcir_i2c_ts.c V3.0	from v3.0 support TangoC solution and remove the previous soltutions
 *
 * pixcir_i2c_ts.c V3.1	Add bootloader function	7
 *			Add RESET_TP		9
 * 			Add ENABLE_IRQ		10
 *			Add DISABLE_IRQ		11
 * 			Add BOOTLOADER_STU	16
 *			Add ATTB_VALUE		17
 *			Add Write/Read Interface for APP software
 *
 * pixcir_i2c_ts.c V3.2.09	for INT_MODE 0x09
 *				change to workqueue for ISR
 *				adaptable report rate self
 *
 * pixcir_i2c_ts.c V3.3.09	Add Android early power management
 *				Add irq_flag for pixcir debug tool
 *				Add CRC attb check when bootloader
 *
 * This code is proprietary and is subject to license terms.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <linux/earlysuspend.h>

#include "pixcir_i2c_ts.h"

/*********************************Bee-0928-TOP****************************************/
#define PIXCIR_DEBUG		0
#define	PIXCIR_STATIC		0

#define SLAVE_ADDR		0x5c
#define	BOOTLOADER_ADDR		0x5d

#ifndef I2C_MAJOR
#define I2C_MAJOR 		125
#endif

#define I2C_MINORS 		256

#define	CALIBRATION_FLAG	1
#define	BOOTLOADER		7
#define RESET_TP		9

#define	ENABLE_IRQ		10
#define	DISABLE_IRQ		11
#define	BOOTLOADER_STU		16
#define ATTB_VALUE		17

#define	MAX_FINGER_NUM		5

static unsigned char status_reg = 0;
volatile int global_irq, irq_flag;
volatile int work_pending;
#if PIXCIR_STATIC
	#include <linux/time.h>
	volatile int drop_counter, work_counter, max_drop_counter, total_drop_counter;
	struct timeval tv1, tv2;
#endif
struct i2c_dev {
	struct list_head list;
	struct i2c_adapter *adap;
	struct device *dev;
};

static struct i2c_driver pixcir_i2c_ts_driver;
static struct class *i2c_dev_class;
static LIST_HEAD( i2c_dev_list);
static DEFINE_SPINLOCK( i2c_dev_list_lock);

static void return_i2c_dev(struct i2c_dev *i2c_dev)
{
	spin_lock(&i2c_dev_list_lock);
	list_del(&i2c_dev->list);
	spin_unlock(&i2c_dev_list_lock);
	kfree(i2c_dev);
}

static struct i2c_dev *i2c_dev_get_by_minor(unsigned index)
{
	struct i2c_dev *i2c_dev;
	i2c_dev = NULL;

	spin_lock(&i2c_dev_list_lock);
	list_for_each_entry(i2c_dev, &i2c_dev_list, list)
	{
		if (i2c_dev->adap->nr == index)
			goto found;
	}
	i2c_dev = NULL;
	found: spin_unlock(&i2c_dev_list_lock);
	return i2c_dev;
}

static struct i2c_dev *get_free_i2c_dev(struct i2c_adapter *adap)
{
	struct i2c_dev *i2c_dev;

	if (adap->nr >= I2C_MINORS) {
		printk(KERN_ERR "i2c-dev: Out of device minors (%d)\n",
				adap->nr);
		return ERR_PTR(-ENODEV);
	}

	i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev)
		return ERR_PTR(-ENOMEM);

	i2c_dev->adap = adap;

	spin_lock(&i2c_dev_list_lock);
	list_add_tail(&i2c_dev->list, &i2c_dev_list);
	spin_unlock(&i2c_dev_list_lock);
	return i2c_dev;
}
/*********************************Bee-0928-bottom**************************************/

static struct workqueue_struct *pixcir_wq;

struct pixcir_i2c_ts_data {
	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;
	struct early_suspend early_suspend;
	int irq;
};

struct point_node_t {
	unsigned char 	active ;
	unsigned char	finger_id;
	unsigned int	posx;
	unsigned int	posy;
};

static struct point_node_t point_slot[MAX_FINGER_NUM*2];

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pixcir_i2c_ts_early_suspend(struct early_suspend *h);
static void pixcir_i2c_ts_late_resume(struct early_suspend *h);
#endif

static void pixcir_ts_poscheck(struct work_struct *work)
{
	struct pixcir_i2c_ts_data *tsdata = container_of(work,
			struct pixcir_i2c_ts_data,
			work.work);
	
	unsigned char *p;
	unsigned char touch, button, pix_id,slot_id;
	unsigned char rdbuf[27], wrbuf[1] = { 0 };
	int ret, i;

	ret = i2c_master_send(tsdata->client, wrbuf, sizeof(wrbuf));
	if (ret != sizeof(wrbuf)) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_send failed(), ret=%d\n",
			__func__, ret);
	}

	ret = i2c_master_recv(tsdata->client, rdbuf, sizeof(rdbuf));
	if (ret != sizeof(rdbuf)) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}

	touch = rdbuf[0]&0x07;
	button = rdbuf[1];
#if PIXCIR_DEBUG
	printk("touch=%d,button=%d\n",touch,button);
#endif
#ifdef BUTTON
	if(button) {
		switch(button) {
			case 1:
				input_report_key(tsdata->input, KEY_HOME, 1);
			case 2:
				//add other key down report
			case 4:
			case 8:
			//add case for other more key 
			default:
				break;
		}
	} else {
		input_report_key(tsdata->input, KEY_HOME, 0);
		//add other key up report
	}
#endif

	p=&rdbuf[2];
	for (i=0; i<touch; i++)	{
		pix_id = (*(p+4));
		slot_id = ((pix_id & 7)<<1) | ((pix_id & 8)>>3);
		point_slot[slot_id].active = 1;
		point_slot[slot_id].finger_id = pix_id;	
		point_slot[slot_id].posx = (*(p+1)<<8)+(*(p));
		point_slot[slot_id].posy = (*(p+3)<<8)+(*(p+2));
		p+=5;
	}

	if(touch) {
		input_report_key(tsdata->input, BTN_TOUCH, 1);
		input_report_abs(tsdata->input, ABS_MT_TOUCH_MAJOR, 15);
		for (i=0; i<MAX_FINGER_NUM*2; i++) {
			if (point_slot[i].active == 1) {
				point_slot[i].active = 0;
				input_report_key(tsdata->input, ABS_MT_TRACKING_ID, i);
				input_report_abs(tsdata->input, ABS_MT_POSITION_X,  point_slot[i].posx);
				input_report_abs(tsdata->input, ABS_MT_POSITION_Y,  point_slot[i].posy);
				input_report_abs(tsdata->input, ABS_MT_TOUCH_MAJOR, 15);
				input_mt_sync(tsdata->input);
				printk("slot=%d,x%d=%d,y%d=%d  ",i, i/2,point_slot[i].posx, i/2, point_slot[i].posy);
			}
		}	printk("\n");
	} else {
		input_report_key(tsdata->input, BTN_TOUCH, 0);
		input_report_abs(tsdata->input, ABS_MT_TOUCH_MAJOR, 0);
	}
	input_sync(tsdata->input);

	work_pending = 0;
}

static irqreturn_t pixcir_ts_isr(int irq, void *dev_id)
{
	struct pixcir_i2c_ts_data *tsdata = dev_id;

	if (!work_pending) {
		work_pending = 1;
	#if PIXCIR_STATIC
		total_drop_counter += drop_counter;
		if(drop_counter > max_drop_counter)
			max_drop_counter = drop_counter;
		if(work_counter == 0) {
			do_gettimeofday (&tv1);
			printk("\n\n");
			printk("work_counter = %d, tv_sec=%10i\n",work_counter, (int)tv1.tv_sec);
		}
		work_counter++;
		if(work_counter==20000) {
			do_gettimeofday (&tv2);
			printk("work_counter = %d, tv_sec=%10i\n",work_counter, (int)tv2.tv_sec);
			printk("work_counter=%d,total_drop_counter=%d,max_drop_counter =%d,cost_time=%d\n",\
				work_counter,total_drop_counter,max_drop_counter,(int)(tv2.tv_sec - tv1.tv_sec));
			printk("TP report rate=%d, system report_rate=%d\n",\
				(work_counter + total_drop_counter)/(int)(tv2.tv_sec - tv1.tv_sec),work_counter/(int)(tv2.tv_sec - tv1.tv_sec));
			total_drop_counter = 0;
			work_counter = 0;
			max_drop_counter = 0;
		}
		drop_counter = 0;
#endif
		queue_work(pixcir_wq, &tsdata->work.work);
	}
#if PIXCIR_STATIC
	else {
		drop_counter++;
	}
#endif
	return IRQ_HANDLED;
}

static int __devinit pixcir_i2c_ts_probe(struct i2c_client *client,
					 const struct i2c_device_id *id)
{
	//const struct pixcir_ts_platform_data *pdata = client->dev.platform_data;
	struct pixcir_i2c_ts_data *tsdata;
	struct input_dev *input;
	struct device *dev;
	struct i2c_dev *i2c_dev;
	int i, error;

	//if (!pdata) {
	//	dev_err(&client->dev, "platform data not defined\n");
	//	return -EINVAL;
	//}

	pixcir_init(); //reset pin set to 0 or 1 and platform init

	for(i=0; i<MAX_FINGER_NUM*2; i++) {
		point_slot[i].active = 0;
	}

	work_pending = 0;
#if PIXCIR_STATIC
	work_counter = 0;
	drop_counter = 0;
	max_drop_counter = 0;
#endif

	tsdata = kzalloc(sizeof(*tsdata), GFP_KERNEL);
	input = input_allocate_device();
	if (!tsdata || !input) {
		dev_err(&client->dev, "Failed to allocate driver data!\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	tsdata->client = client;
	tsdata->input = input;
	//tsdata->chip = pdata;
	tsdata->irq = client->irq;
	global_irq = client->irq;

	INIT_WORK(&tsdata->work.work, pixcir_ts_poscheck);

	input->name = client->name;
	input->id.bustype = BUS_I2C;
	input->dev.parent = &client->dev;

	__set_bit(EV_KEY, input->evbit);
	__set_bit(EV_ABS, input->evbit);
	__set_bit(EV_SYN, input->evbit);
	__set_bit(BTN_TOUCH, input->keybit);
	__set_bit(ABS_MT_TOUCH_MAJOR, input->absbit);
	__set_bit(ABS_MT_TRACKING_ID, input->absbit);
	__set_bit(ABS_MT_POSITION_X, input->absbit);
	__set_bit(ABS_MT_POSITION_Y, input->absbit);

	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, X_MAX, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, Y_MAX, 0, 0);

	input_set_drvdata(input, tsdata);

	error = request_irq(client->irq, pixcir_ts_isr,	
			    IRQF_TRIGGER_FALLING,
			    client->name, tsdata);
	if (error) {
		dev_err(&client->dev, "Unable to request touchscreen IRQ.\n");
		goto err_free_mem;
	}
	disable_irq_nosync(client->irq);

	error = input_register_device(input);
	if (error)
		goto err_free_irq;

	i2c_set_clientdata(client, tsdata);
	device_init_wakeup(&client->dev, 1);

	/*********************************Bee-0928-TOP****************************************/
	i2c_dev = get_free_i2c_dev(client->adapter);
	if (IS_ERR(i2c_dev)) {
		error = PTR_ERR(i2c_dev);
		return error;
	}

	dev = device_create(i2c_dev_class, &client->adapter->dev, MKDEV(I2C_MAJOR,
			client->adapter->nr), NULL, "pixcir_i2c_ts%d", 0);
	if (IS_ERR(dev)) {
		error = PTR_ERR(dev);
		return error;
	}
	/*********************************Bee-0928-BOTTOM****************************************/

#ifdef CONFIG_HAS_EARLYSUSPEND
	tsdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	tsdata->early_suspend.suspend = pixcir_i2c_ts_early_suspend;
	tsdata->early_suspend.resume = pixcir_i2c_ts_late_resume;
	register_early_suspend(&tsdata->early_suspend);
#endif

	dev_err(&tsdata->client->dev, "insmod successfully!\n");
	
	irq_flag = 1;
	enable_irq(client->irq);

	return 0;

err_free_irq:
	free_irq(client->irq, tsdata);
err_free_mem:
	input_free_device(input);
	kfree(tsdata);
	return error;
}

static int __devexit pixcir_i2c_ts_remove(struct i2c_client *client)
{
	int error;
	struct i2c_dev *i2c_dev;
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&tsdata->early_suspend);
#endif

	device_init_wakeup(&client->dev, 0);

	mb();
	free_irq(client->irq, tsdata);

	/*********************************Bee-0928-TOP****************************************/
	i2c_dev = get_free_i2c_dev(client->adapter);
	if (IS_ERR(i2c_dev)) {
		error = PTR_ERR(i2c_dev);
		return error;
	}

	return_i2c_dev(i2c_dev);
	device_destroy(i2c_dev_class, MKDEV(I2C_MAJOR, client->adapter->nr));
	/*********************************Bee-0928-BOTTOM****************************************/

	input_unregister_device(tsdata->input);
	kfree(tsdata);

	return 0;
}

/*************************************Bee-0928****************************************/
/*                        	     pixcir_open                                     */
/*************************************Bee-0928****************************************/
static int pixcir_open(struct inode *inode, struct file *file)
{
	int subminor;
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	struct i2c_dev *i2c_dev;
	int ret = 0;
#if PIXCIR_DEBUG
	printk("enter pixcir_open function\n");
#endif
	subminor = iminor(inode);

	lock_kernel();
	i2c_dev = i2c_dev_get_by_minor(subminor);
	if (!i2c_dev) {
		printk("error i2c_dev\n");
		return -ENODEV;
	}

	adapter = i2c_get_adapter(i2c_dev->adap->nr);
	if (!adapter) {
		return -ENODEV;
	}
	
	client = kzalloc(sizeof(*client), GFP_KERNEL);

	if (!client) {
		i2c_put_adapter(adapter);
		ret = -ENOMEM;
	}

	snprintf(client->name, I2C_NAME_SIZE, "pixcir_i2c_ts%d", adapter->nr);
	client->driver = &pixcir_i2c_ts_driver;
	client->adapter = adapter;
	
	file->private_data = client;

	return 0;
}

/*************************************Bee-0928****************************************/
/*                        	     pixcir_ioctl                                    */
/*************************************Bee-0928****************************************/
static long pixcir_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client *) file->private_data;
#if PIXCIR_DEBUG
	printk("pixcir_ioctl(),cmd = %d, arg = %ld\n", cmd, arg);
#endif

	switch (cmd)
	{
	case CALIBRATION_FLAG:	//CALIBRATION_FLAG = 1
		client->addr = SLAVE_ADDR;
		status_reg = CALIBRATION_FLAG;
		break;

	case BOOTLOADER:	//BOOTLOADER = 7
		client->addr = BOOTLOADER_ADDR;
		status_reg = BOOTLOADER;

		pixcir_reset();
		mdelay(5);
		break;

	case RESET_TP:		//RESET_TP = 9
		pixcir_reset();
		break;
		
	case ENABLE_IRQ:	//ENABLE_IRQ = 10
		status_reg = 0;
		if(irq_flag == 0) {
			irq_flag = 1;
			enable_irq(global_irq);
		}
		break;
		
	case DISABLE_IRQ:	//DISABLE_IRQ = 11
		if(irq_flag == 1) {
			irq_flag = 0;
			disable_irq_nosync(global_irq);
		}
		break;

	case BOOTLOADER_STU:	//BOOTLOADER_STU = 16
		client->addr = BOOTLOADER_ADDR;
		status_reg = BOOTLOADER_STU;

		pixcir_reset();
		mdelay(5);
		break;

	case ATTB_VALUE:	//ATTB_VALUE = 17
		client->addr = SLAVE_ADDR;
		status_reg = ATTB_VALUE;
		break;

	default:
		client->addr = SLAVE_ADDR;
		status_reg = 0;
		break;
	}
	return 0;
}

/***********************************Bee-0928****************************************/
/*                        	  pixcir_read                                      */
/***********************************Bee-0928****************************************/
static ssize_t pixcir_read (struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	struct i2c_client *client = (struct i2c_client *)file->private_data;
	unsigned char *tmp, bootloader_stu[4], attb_value[1];
	int ret = 0;

	switch(status_reg)
	{
	case BOOTLOADER_STU:
		i2c_master_recv(client, bootloader_stu, sizeof(bootloader_stu));
		if (ret!=sizeof(bootloader_stu)) {
			dev_err(&client->dev,
				"%s: BOOTLOADER_STU: i2c_master_recv() failed, ret=%d\n",
				__func__, ret);
			return -EFAULT;
		}

		if (copy_to_user(buf, bootloader_stu, sizeof(bootloader_stu))) {
			dev_err(&client->dev,
				"%s: BOOTLOADER_STU: copy_to_user() failed.\n",	__func__);
			return -EFAULT;
		} else {
			ret = 4;
		}
		break;

	case ATTB_VALUE:
		attb_value[0] = attb_read_val();
		if(copy_to_user(buf, attb_value, sizeof(attb_value))) {
			dev_err(&client->dev,
				"%s: ATTB_VALUE: copy_to_user() failed.\n", __func__);
			return -EFAULT;
		} else {
			ret = 1;
		}
		break;

	default:
		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp==NULL)
			return -ENOMEM;

		ret = i2c_master_recv(client, tmp, count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: default: i2c_master_recv() failed, ret=%d\n",
				__func__, ret);
			return -EFAULT;
		}

		if(copy_to_user(buf, tmp, count)) {
			dev_err(&client->dev,
				"%s: default: copy_to_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}
		kfree(tmp);
		break;
	}
	return ret;
}

/***********************************Bee-0928****************************************/
/*                        	  pixcir_write                                     */
/***********************************Bee-0928****************************************/
static ssize_t pixcir_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct i2c_client *client;
	unsigned char *tmp, bootload_data[143];
	int ret = 0, time_out = 0;

	client = file->private_data;

	switch(status_reg)
	{
	case CALIBRATION_FLAG:	//CALIBRATION_FLAG=1
		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp == NULL)
			return -ENOMEM;

		if (copy_from_user(tmp, buf, count)) { 	
			dev_err(&client->dev,
				"%s: CALIBRATION_FLAG: copy_from_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}

		ret = i2c_master_send(client, tmp, count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: CALIBRATION: i2c_master_send() failed, ret=%d\n",
				__func__, ret);
			kfree(tmp);
			return -EFAULT;
		}

		kfree(tmp);
		break;

	case BOOTLOADER:
		memset(bootload_data, 0, sizeof(bootload_data));

		if (copy_from_user(bootload_data, buf, count)) {
			dev_err(&client->dev,
				"%s: BOOTLOADER: copy_from_user() failed.\n", __func__);
			return -EFAULT;
		}
		time_out = 0;
		while (attb_read_val()) {
			if(time_out > 100)			
				break;
			else {
				time_out++;
				mdelay(1);
			}
		}
		ret = i2c_master_send(client, bootload_data, count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: BOOTLOADER: i2c_master_send() failed, ret = %d\n",
				__func__, ret);
			return -EFAULT;
		}
		time_out = 0;
		while (!attb_read_val()) {
			if(time_out > 100)			
				break;
			else {
				time_out++;
				mdelay(1);
			}
		}

		time_out = 0;
		while(attb_read_val()) {
			if(time_out > 100)
				break;
			else {
				time_out++;
				mdelay(1);
			}
		}

		break;
	default:
		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp == NULL)
			return -ENOMEM;

		if (copy_from_user(tmp, buf, count)) { 	
			dev_err(&client->dev,
				"%s: default: copy_from_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}
		
		ret = i2c_master_send(client,tmp,count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: default: i2c_master_send() failed, ret=%d\n",
				__func__, ret);
			kfree(tmp);
			return -EFAULT;
		}
		kfree(tmp);
		break;
	}
	return ret;
}

/***********************************Bee-0928****************************************/
/*                        	  pixcir_release                                   */
/***********************************Bee-0928****************************************/
static int pixcir_release(struct inode *inode, struct file *file)
{
	struct i2c_client *client = file->private_data;
#if PIXCIR_DEBUG
	printk("enter pixcir_release funtion\n");
#endif
	i2c_put_adapter(client->adapter);
	kfree(client);
	file->private_data = NULL;

	return 0;
}

/*********************************Bee-0928-TOP****************************************/
static const struct file_operations pixcir_i2c_ts_fops =
{	.owner		= THIS_MODULE,
	.open		= pixcir_open,
	.unlocked_ioctl = pixcir_ioctl,
	.read		= pixcir_read,
	.write		= pixcir_write,
	.release	= pixcir_release,
};
/*********************************Bee-0928-BOTTOM****************************************/

static int pixcir_i2c_ts_suspend(struct i2c_client *client)
{
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(client);
	unsigned char wrbuf[2] = { 0 };
	int ret;
	//printk("------------------pixcir TP early suspend--------------------\n");
	wrbuf[0] = 0x33;
	wrbuf[1] = 0x03;	//enter into Sleep mode;
	/**************************************************************
	wrbuf[1]:	0x00: Active mode

			0x01: Sleep mode
			0xA4: Sleep mode automatically switch
			0x03: Freeze mode
	More details see application note 710 power manangement section
	****************************************************************/
	ret = i2c_master_send(tsdata->client, wrbuf, 2);
	if (ret != 2) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_send failed(), ret=%d\n",
			__func__, ret);
	}

	if (device_may_wakeup(&tsdata->client->dev))
		enable_irq_wake(tsdata->irq);

	return 0;
}

static int pixcir_i2c_ts_resume(struct i2c_client *client)
{
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(client);
	//printk("-----------------pixcir TP late resume---------------------\n");
///if suspend enter into freeze mode please reset TP
#if 0
	pixcir_reset();
#else
	unsigned char wrbuf[2] = { 0 };
	int ret;

	wrbuf[0] = 0x33;
	wrbuf[1] = 0;
	ret = i2c_master_send(tsdata->client, wrbuf, 2);
	if(ret != 2) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_send failed(), ret=%d\n",
			__func__, ret);
	}
#endif
	if (device_may_wakeup(&tsdata->client->dev))
		disable_irq_wake(tsdata->irq);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pixcir_i2c_ts_early_suspend(struct early_suspend *h)
{
	struct pixcir_i2c_ts_data *tsdata;
	tsdata = container_of(h, struct pixcir_i2c_ts_data, early_suspend);
	pixcir_i2c_ts_suspend(tsdata->client);
}

static void pixcir_i2c_ts_late_resume(struct early_suspend *h)
{
	struct pixcir_i2c_ts_data *tsdata;
	tsdata = container_of(h, struct pixcir_i2c_ts_data, early_suspend);
	pixcir_i2c_ts_resume(tsdata->client);
}
#endif

static const struct i2c_device_id pixcir_i2c_ts_id[] = {
	{ "pixcir_ts", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pixcir_i2c_ts_id);

static struct i2c_driver pixcir_i2c_ts_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "pixcir_i2c_ts_v3.3.09",
	},
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= pixcir_i2c_ts_suspend,
	.resume		= pixcir_i2c_ts_resume,
#endif
	.probe		= pixcir_i2c_ts_probe,
	.remove		= __devexit_p(pixcir_i2c_ts_remove),
	.id_table	= pixcir_i2c_ts_id,
};

static int __init pixcir_i2c_ts_init(void)
{
	int ret;
	pixcir_wq = create_singlethread_workqueue("pixcir_wq");
	if (!pixcir_wq)
		return -ENOMEM;
	/*********************************Bee-0928-TOP****************************************/
	ret = register_chrdev(I2C_MAJOR, "pixcir_i2c_ts", &pixcir_i2c_ts_fops);
	if (ret) {
		printk(KERN_ERR "%s:register chrdev failed\n", __FILE__);
		return ret;
	}

	i2c_dev_class = class_create(THIS_MODULE, "pixcir_i2c_dev");
	if (IS_ERR(i2c_dev_class)) {
		ret = PTR_ERR(i2c_dev_class);
		class_destroy(i2c_dev_class);
	}
	/********************************Bee-0928-BOTTOM******************************************/

	return i2c_add_driver(&pixcir_i2c_ts_driver);
}
module_init(pixcir_i2c_ts_init);

static void __exit pixcir_i2c_ts_exit(void)
{
	i2c_del_driver(&pixcir_i2c_ts_driver);
	/********************************Bee-0928-TOP******************************************/
	class_destroy(i2c_dev_class);
	unregister_chrdev(I2C_MAJOR,"pixcir_i2c_ts");
	/********************************Bee-0928-BOTTOM******************************************/
	if(pixcir_wq)
		destroy_workqueue(pixcir_wq);
}
module_exit(pixcir_i2c_ts_exit);

MODULE_AUTHOR("Jianchun Bian <jcbian@pixcir.com.cn>");
MODULE_DESCRIPTION("Pixcir I2C Touchscreen Driver");
MODULE_LICENSE("Pixcir Proprietary");
