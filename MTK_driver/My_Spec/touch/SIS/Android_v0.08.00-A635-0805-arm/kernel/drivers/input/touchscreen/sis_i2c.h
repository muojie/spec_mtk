/*
 * include/linux/sis_i2c.h - platform data structure for SiS 9200 family
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

#ifndef _LINUX_SIS_I2C_H
#define _LINUX_SIS_I2C_H

#define SIS_I2C_NAME "sis_i2c_ts"
#define SIS_SLAVE_ADDR					0x05
#define TIMER_NS    					12500000 //12.5ms
#define MAX_FINGERS						10

/* For SiS9200 i2c data format
 * Define if for SMBus Tx/Rx in X86.
 * Undef it for I2C Tx/Rx in Embedded System.
 */
//#define _SMBUS_INTERFACE  			//  ON/OFF

/* Interrupt setting and modes */
#define _I2C_INT_ENABLE					//  ON/OFF
#define GPIO_IRQ 						133

/*  Enable if use interrupt case 1 mode.
 *	Disable if use interrupt case 2 mode.
 */
#define _INT_MODE_1

/* POWER MODE */
#ifdef CONFIG_FW_SUPPORT_POWERMODE
#define MSK_POWERMODE					0x03
#define DEEPSLEEP_MODE					0X00
#define IDLE_MODE						0X01
#define ACTIVE_MODE 					0x02
#define WRITE_DEEPSLEEP_MODE 			0x80 //0x8C
#define WRITE_IDLE_MODE 				0x81 //0x8D
#define WRITE_ACTIVE_MODE 				0x82 //0x8E
#define READ_POWERMODE					0x00
#endif

/* Resolution mode */
#define SIS_MAX_X						4095
#define SIS_MAX_Y						4095

/* Fixed point mode */
//#define X_SENSE_LINE					38 //8
//#define Y_SENSE_LINE					22 //12
//#define SIS_MAX_X           			(X_SENSE_LINE * 128)
//#define SIS_MAX_Y	        			(Y_SENSE_LINE * 128)

#define ONE_BYTE						1
#define FIVE_BYTE						5
#define EIGHT_BYTE  					8
#define MAX_BYTE						16

#define SIS_CMD_NORMAL					0x0
#define SIS_CMD_SOFTRESET				0x82
#define SIS_CMD_RECALIBRATE				0x87
#define SIS_CMD_POWERMODE       		0x90
#define MAX_READ_BYTE_COUNT				16
#define MSK_TOUCHNUM					0x0f
#define MSK_HAS_CRC						0x10
#define MSK_DATAFMT						0xe0
#define MSK_PSTATE						0x0f
#define MSK_PID	                		0xf0
#define TOUCHDOWN						0x0
#define TOUCHUP							0x1
#define RES_FMT							0x00
#define FIX_FMT							0x40

#ifdef _SMBUS_INTERFACE
#define CMD_BASE						0
#else
#define CMD_BASE						1	//i2c
#endif

#define FORMAT_MODE						1
#define P1TSTATE						2
#define PKTINFO							(CMD_BASE + 1)
#define BUTTON_STATE					(CMD_BASE + 1)

#define NO_TOUCH						0x02
#define SINGLE_TOUCH					0x09
#define MULTI_TOUCH						0x0e
#define LAST_ONE						0x07
#define LAST_TWO						0x0c
#define BUTTON_TOUCH					0x05
#define BUTTON_TOUCH_ONE_POINT			0x0A
#define BUTTON_TOUCH_MULTI_TOUCH		0x0F
#define MSK_BUTTON_POINT				0xf0
#define BUTTON_TOUCH_SERIAL				0x70

#define CRCCNT(x) 						((x + 0x1) & (~0x1))

#define MSK_NOBTN						0
#define MSK_COMP						1
#define MSK_BACK						2
#define MSK_MENU						4
#define MSK_HOME						8

#endif /* _LINUX_SIS_I2C_H */

struct sis_i2c_rmi_platform_data {
	uint32_t version;	/* Use this entry for panels with */
				/* (major << 8 | minor) version or above. */
				/* If non-zero another array entry follows */
	int (*power)(int on);	/* Only valid in first array entry */
	uint32_t flags;
	unsigned long irqflags;
	uint32_t inactive_left; /* 0x10000 = screen width */
	uint32_t inactive_right; /* 0x10000 = screen width */
	uint32_t inactive_top; /* 0x10000 = screen height */
	uint32_t inactive_bottom; /* 0x10000 = screen height */
	uint32_t snap_left_on; /* 0x10000 = screen width */
	uint32_t snap_left_off; /* 0x10000 = screen width */
	uint32_t snap_right_on; /* 0x10000 = screen width */
	uint32_t snap_right_off; /* 0x10000 = screen width */
	uint32_t snap_top_on; /* 0x10000 = screen height */
	uint32_t snap_top_off; /* 0x10000 = screen height */
	uint32_t snap_bottom_on; /* 0x10000 = screen height */
	uint32_t snap_bottom_off; /* 0x10000 = screen height */
	uint32_t fuzz_x; /* 0x10000 = screen width */
	uint32_t fuzz_y; /* 0x10000 = screen height */
	int fuzz_p;
	int fuzz_w;
	int8_t sensitivity_adjust;
};

static const unsigned short crc16tab[256]= {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
        0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
        0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
        0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
        0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
        0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
        0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
        0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
        0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
        0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
        0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
        0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
        0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
        0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
        0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
        0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
        0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
        0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
        0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
        0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
        0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
        0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
        0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
        0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
        0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
        0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
        0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
        0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
        0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
        0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
        0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
        0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

struct Point {
	int id;
	unsigned short x, y;   // uint16_t ?
	bool bPressure;
	bool bWidth;
	bool touch, pre;
};

struct sisTP_driver_data {
	int id, fingers;
	bool valid;	//BTN_TOUCH is on/off
	struct Point pt[MAX_FINGERS];
	uint16_t CRC;
	uint8_t pre_keybit_state;
};

struct sis_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	struct hrtimer timer;
	struct work_struct  work;
	uint16_t max[2];
	int snap_state[2][2];
	int snap_down_on[2];
	int snap_down_off[2];
	int snap_up_on[2];
	int snap_up_off[2];
	int snap_down[2];
	int snap_up[2];
	uint32_t flags;
	int (*power)(int on);
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};
