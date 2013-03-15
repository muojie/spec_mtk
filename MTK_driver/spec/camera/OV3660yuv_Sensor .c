/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   OV3660yuv_sensor.c
 *
 * Project:
 * --------
 *   APLS
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *
 * Author:
 * ------------
 *   HengJun Wang (mtk70677)
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/io.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "OV3660yuv_Sensor.h"
#include "OV3660yuv_Camera_Sensor_para.h"
#include "OV3660yuv_CameraCustomized.h"

//extern void stop_process(void);

#define __SENSOR_CONTROL__

#ifdef __SENSOR_CONTROL__
#define SENSOR_CONTROL_FLOW() 	printk("Teng [%s]\t\n",__FUNCTION__)
#define SENSOR_LOG(fmt, arg...) printk("Teng [Line=%4d]\t\n"fmt,__LINE__,##arg)
#define SENSOR_ERR(fmt, arg...) printk("Teng [Line=%4d]\t\n"fmt,__LINE__,##arg)
#else
#define SENSOR_CONTROL_FLOW()
#define SENSOR_LOG(fmt, arg...)
#define SENSOR_ERR(fmt, arg...)
#endif

static MSDK_SENSOR_CONFIG_STRUCT OV3660SensorConfigData;
static struct OV3660_Sensor_Struct OV3660_Sensor_Driver;

#if 1 // Teng
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);

#define OV3660_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, OV3660_WRITE_ID)


kal_uint16 OV3660_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
//	char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };    // Teng
	
//	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,OV8825MIPI_WRITE_ID);
    iReadReg((u16) addr ,(u8*)&get_byte,OV3660_WRITE_ID);
    return get_byte;
}

#else
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
/*************************************************************************
* FUNCTION
*    OV3660_write_cmos_sensor
*
* DESCRIPTION
*    This function wirte data to CMOS sensor through I2C
*
* PARAMETERS
*    addr: the 16bit address of register
*    para: the 8bit value of register
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV3660_write_cmos_sensor(kal_uint16 addr, kal_uint8 para)
{
	  kal_uint8 out_buff[3];

    out_buff[0] = addr>>8;
    out_buff[1] = addr&0xFF;
    out_buff[2] = para;

    iWriteRegI2C((u8*)out_buff , (u16)sizeof(out_buff), OV3660_WRITE_ID); 

    #if (defined(__OV3660_DEBUG_TRACE__))
         if (sizeof(out_buff) != rt) printk("I2C write %x, %x error\n", addr, para);
    #endif
}

/*************************************************************************
* FUNCTION
*    OV3660_read_cmos_sensor
*
* DESCRIPTION
*    This function read data from CMOS sensor through I2C.
*
* PARAMETERS
*    addr: the 16bit address of register
*
* RETURNS
*    8bit data read through I2C
*
* LOCAL AFFECTED
*
*************************************************************************/
static kal_uint8 OV3660_read_cmos_sensor(kal_uint16 addr)
{
	  kal_uint8 in_buff[1] = {0xFF};
	  kal_uint8 out_buff[2];

    out_buff[0] = addr>>8;
    out_buff[1] = addr&0xFF;

	  if (0 != iReadRegI2C((u8*)out_buff , (u16) sizeof(out_buff), (u8*)in_buff, (u16) sizeof(in_buff), OV3660_WRITE_ID))
	  {
	      printk("[Error:OV3660]I2C read %x error\n", addr);
	  }

    #if (defined(__OV3660_DEBUG_TRACE__))
         if (size != rt) printk("I2C read %x error\n", addr);
         #endif
	  return in_buff[0];
}
#endif

void OV3660_Set_Dummy(kal_uint16 pixels, kal_uint16 lines)
{
	  kal_uint32 Total_Pixels,Total_Lines;	
    
    if (OV3660_Sensor_Driver.bIsCapture==KAL_FALSE) //1024x768
    {
    	  //Total_Pixels = OV3660_DEFUALT_XGA_PIXEL_NUMS + pixels;
		    //Total_Lines = OV3660_DEFULAT_XGA_LINE_NUMS + lines;
		    
		    Total_Pixels = 0x08fc + pixels;
		    Total_Lines = 0x0310 + lines;
	  }
	  else //2048x1536
	  {
		    //Total_Pixels = OV3660_DEFUALT_QXGA_PIXEL_NUMS + pixels;
		    //Total_Lines = OV3660_DEFULAT_QXGA_LINE_NUMS + lines;
		    
		    Total_Pixels = 0x08fc + pixels;
		    Total_Lines = 0x0620 + lines;
	  }

    OV3660_write_cmos_sensor(0x380d,(Total_Pixels&0xFF));         //VTS[7:0]
    OV3660_write_cmos_sensor(0x380c,((Total_Pixels&0xFF00)>>8));  //VTS[15:8]

    OV3660_write_cmos_sensor(0x380f,(Total_Lines&0xFF));         //VTS[7:0]
    OV3660_write_cmos_sensor(0x380e,((Total_Lines&0xFF00)>>8));  //VTS[15:8]

	  SENSOR_LOG("Total_Pixels=%x,Total_Lines=%x\n",Total_Pixels,Total_Lines);
}    

kal_uint32 OV3660_read_shutter(void)
{
    kal_uint8 temp_reg1, temp_reg2,temp_reg3;
    kal_uint32 temp_reg, extra_exp_lines;

	  temp_reg1 = OV3660_read_cmos_sensor(0x3502);	// AEC[b7~b0]
	  temp_reg2 = OV3660_read_cmos_sensor(0x3501);	// AEC[b15~b8]
	  temp_reg3 = OV3660_read_cmos_sensor(0x3500);	// AEC[b15~b8]
	  temp_reg = ((temp_reg1 & 0xFF)>>4) | (temp_reg2 << 4) | (temp_reg3 <<12);

    temp_reg1 = OV3660_read_cmos_sensor(0x350D);    // EXVTS[b7~b0]
    temp_reg2 = OV3660_read_cmos_sensor(0x350C);    // EXVTS[b15~b8]
    extra_exp_lines = (temp_reg1 & 0xFF) | (temp_reg2 << 8);
        
	  SENSOR_LOG("Shutter=%x,Extra_shutter=%x\n",temp_reg,extra_exp_lines);
    return temp_reg;
    //return temp_reg + extra_exp_lines;
}   

static void OV3660_Write_Shutter(kal_uint16 shutter)
{
	  kal_uint32 OV3660_extra_exposure_lines;
	
    if (OV3660_Sensor_Driver.bIsCapture==KAL_FALSE) 
    {
        if (shutter <= OV3660_PV_EXPOSURE_LIMITATION) 
        {
            OV3660_extra_exposure_lines = 0;
        }
        else
        {
            OV3660_extra_exposure_lines=shutter - OV3660_PV_EXPOSURE_LIMITATION;
        }

        if (shutter > OV3660_PV_EXPOSURE_LIMITATION) 
        {
            shutter = OV3660_PV_EXPOSURE_LIMITATION;
        }
    }
    else 
    {
        if (shutter <= OV3660_FULL_EXPOSURE_LIMITATION) 
        {
            OV3660_extra_exposure_lines = 0;
    	  }
    	  else 
    	  {
            OV3660_extra_exposure_lines = shutter - OV3660_FULL_EXPOSURE_LIMITATION;
        }

        if (shutter > OV3660_FULL_EXPOSURE_LIMITATION) 
        {
            shutter = OV3660_FULL_EXPOSURE_LIMITATION;
        }
    }

        // set extra exposure line
        if(OV3660_extra_exposure_lines <= 0)
    	  {
		       OV3660_extra_exposure_lines = 0;
	      }
	
        OV3660_write_cmos_sensor(0x350D, OV3660_extra_exposure_lines & 0xFF);          // EXVTS[b7~b0]
        OV3660_write_cmos_sensor(0x350C, (OV3660_extra_exposure_lines & 0xFF00) >> 8); // EXVTS[b15~b8]
	
        /* Max exporsure time is 1 frmae period event if Tex is set longer than 1 frame period */
	      shutter *=16;
        OV3660_write_cmos_sensor(0x3502, shutter & 0xFF);           //AEC[7:0]
        OV3660_write_cmos_sensor(0x3501, (shutter & 0xFF00) >> 8);  //AEC[8:15]
	      OV3660_write_cmos_sensor(0x3500, (shutter & 0xFF0000) >> 16);

	      SENSOR_LOG("Shutter=%x,Extra_shutter=%x\n",shutter,OV3660_extra_exposure_lines);

}   

kal_uint16 OV3660_read_gain(void)
{
	  kal_uint8 temp_reg1 = OV3660_read_cmos_sensor(0x350B);    // EXVTS[b7~b0]
    kal_uint8 temp_reg2 = OV3660_read_cmos_sensor(0x350A);    // EXVTS[b15~b8]
    kal_uint16 Gain = (temp_reg1 & 0xFF) | ((0x03 & temp_reg2) << 8);
	
	  SENSOR_LOG("Gain=%x\n",Gain);
	  return Gain;
}
static void OV3660_Write_Gain(kal_uint16 gain)
{
	  SENSOR_LOG("Gain=%x\n",gain);

    OV3660_write_cmos_sensor(0x350A,((gain&0x0300)>>8));
	  OV3660_write_cmos_sensor(0x350B,(gain&0xFF));
}  
static void OV3660_AECtrl(kal_bool enable)
{
	  kal_uint16 RegValue;

	  /*AE enable Reg0x3503 bit[0] = 0 enable*/
	  RegValue = OV3660_read_cmos_sensor(0x3503);

	  if(enable)
	  {
		   OV3660_write_cmos_sensor(0x3503,RegValue&(~0x01));
	  }
	  else
	  {
		   OV3660_write_cmos_sensor(0x3503,RegValue|(0x01));
	  }
}
static void OV3660_AGCtrl(kal_bool enable)
{
	  kal_uint16 RegValue;

	  /*AG enable Reg0x3503 bit[1] = 0 enable*/
	  RegValue = OV3660_read_cmos_sensor(0x3503);

	  if(enable)
	  {
		   OV3660_write_cmos_sensor(0x3503,RegValue&(~0x02));
	  }
	  else
	  {
		   OV3660_write_cmos_sensor(0x3503,RegValue|(0x02));
	  }
}

void OV3660_Computer_AECAGC(kal_uint16 preview_clk_in_M, kal_uint16 capture_clk_in_M)
{
#if 0
    kal_uint16 PV_Line_Width;
    kal_uint16 Capture_Line_Width;
    kal_uint16 Capture_Maximum_Shutter;
    kal_uint16 Capture_Exposure;
    kal_uint16 Capture_Gain16;
    kal_uint32 Capture_Banding_Filter;
    kal_uint32 Gain_Exposure=0;

    PV_Line_Width = OV3640_PV_PERIOD_PIXEL_NUMS + OV3640_PV_Dummy_Pixels;   

    Capture_Line_Width = OV3640_FULL_PERIOD_PIXEL_NUMS + OV3640_Capture_Dummy_Pixels;
    Capture_Maximum_Shutter = OV3640_FULL_EXPOSURE_LIMITATION + OV3640_Capture_Dummy_Lines;

    if (OV3640_Banding_setting == AE_FLICKER_MODE_50HZ)
#if WINMO_USE        
        Capture_Banding_Filter = (kal_uint32)(capture_clk_in_M*1000000/100/(2*Capture_Line_Width)+0.5);
#else 
        Capture_Banding_Filter = (kal_uint32)(capture_clk_in_M*100000/100/(2*Capture_Line_Width));
#endif 
    else
#if WINMO_USE
        Capture_Banding_Filter = (kal_uint16)(capture_clk_in_M*1000000/120/(2*Capture_Line_Width)+0.5);
#else 
        Capture_Banding_Filter = (kal_uint32)(capture_clk_in_M*100000/120/(2*Capture_Line_Width) );
#endif 

    /*   Gain_Exposure = OV3640_PV_Gain16*(OV3640_PV_Shutter+OV3640_PV_Extra_Lines)*PV_Line_Width/g_Preview_PCLK_Frequency/Capture_Line_Width*g_Capture_PCLK_Frequency
    ;*/
    OV3640_PV_Gain16 = OV3640_read_OV3640_gain();
    Gain_Exposure = 1 * OV3640_PV_Gain16;  //For OV3640
    ///////////////////////
    Gain_Exposure *=(OV3640_PV_Shutter+OV3640_PV_Extra_Lines);
    Gain_Exposure *=PV_Line_Width;  //970
    //   Gain_Exposure /=g_Preview_PCLK_Frequency;
    Gain_Exposure /=Capture_Line_Width;//1940
    Gain_Exposure = Gain_Exposure*capture_clk_in_M/preview_clk_in_M;// for clock   

    //redistribute gain and exposure
    if (Gain_Exposure < (kal_uint32)(Capture_Banding_Filter * 16))     // Exposure < 1/100/120
    {
       if(Gain_Exposure<16){//exposure line smaller than 2 lines and gain smaller than 0x08 
            Gain_Exposure = Gain_Exposure*4;     
            Capture_Exposure = 1;
            Capture_Gain16 = (Gain_Exposure*2 + 1)/Capture_Exposure/2/4;
        }
        else
        {
            Capture_Exposure = Gain_Exposure /16;
            Capture_Gain16 = (Gain_Exposure*2 + 1)/Capture_Exposure/2;
        }
    }
    else 
    {
        if (Gain_Exposure >(kal_uint32)( Capture_Maximum_Shutter * 16)) // Exposure > Capture_Maximum_Shutter
        {
           
            Capture_Exposure = Capture_Maximum_Shutter/Capture_Banding_Filter*Capture_Banding_Filter;
            Capture_Gain16 = (Gain_Exposure*2 + 1)/Capture_Exposure/2;
            if (Capture_Gain16 > OV3640_Capture_Max_Gain16) 
            {
                // gain reach maximum, insert extra line
                Capture_Exposure = (kal_uint16)(Gain_Exposure*11 /10 /OV3640_Capture_Max_Gain16);
                
                // Exposure = n/100/120
                Capture_Exposure = Capture_Exposure/Capture_Banding_Filter * Capture_Banding_Filter;
                Capture_Gain16 = ((Gain_Exposure *4)/ Capture_Exposure+3)/4;
            }
        }
        else  // 1/100 < Exposure < Capture_Maximum_Shutter, Exposure = n/100/120
        {
            Capture_Exposure = Gain_Exposure/16/Capture_Banding_Filter;
            Capture_Exposure = Capture_Exposure * Capture_Banding_Filter;
            Capture_Gain16 = (Gain_Exposure*2 +1) / Capture_Exposure/2;
        }
    }
    
    OV3640_Capture_Gain16 = Capture_Gain16;
    OV3640_Capture_Extra_Lines = (Capture_Exposure > Capture_Maximum_Shutter)?
            (Capture_Exposure - Capture_Maximum_Shutter/Capture_Banding_Filter*Capture_Banding_Filter):0;     
    
    OV3640_Capture_Shutter = Capture_Exposure - OV3640_Capture_Extra_Lines;
#endif
}

static void OV3660_Sensor_Driver_Init(void)
{
	 OV3660_write_cmos_sensor(0x3103,0x13);
     OV3660_write_cmos_sensor(0x3008,0x42);
	 mDELAY(4);
     OV3660_write_cmos_sensor(0x3017,0xff);
     OV3660_write_cmos_sensor(0x3018,0xff);
     OV3660_write_cmos_sensor(0x302c,0xC3); //  set drv current to max
     
     OV3660_write_cmos_sensor(0x3032,0x00);
     OV3660_write_cmos_sensor(0x3614,0x80);
     OV3660_write_cmos_sensor(0x3618,0x00);
     OV3660_write_cmos_sensor(0x3619,0x75);
     OV3660_write_cmos_sensor(0x3622,0x80);
     OV3660_write_cmos_sensor(0x3623,0x00);
     OV3660_write_cmos_sensor(0x3624,0x03);
     OV3660_write_cmos_sensor(0x3630,0x52);
     OV3660_write_cmos_sensor(0x3632,0x07);
     OV3660_write_cmos_sensor(0x3633,0xd2);
     OV3660_write_cmos_sensor(0x3704,0x80);
     OV3660_write_cmos_sensor(0x3708,0x66);
     OV3660_write_cmos_sensor(0x3709,0x12);
     OV3660_write_cmos_sensor(0x370b,0x12);
     OV3660_write_cmos_sensor(0x3717,0x00);
     OV3660_write_cmos_sensor(0x371b,0x60);
     OV3660_write_cmos_sensor(0x371c,0x00);
     OV3660_write_cmos_sensor(0x3901,0x13);

     OV3660_write_cmos_sensor(0x3600,0x08);
     OV3660_write_cmos_sensor(0x3620,0x43);
     OV3660_write_cmos_sensor(0x3702,0x20);
     OV3660_write_cmos_sensor(0x3739,0x48);
     OV3660_write_cmos_sensor(0x3730,0x20);
     OV3660_write_cmos_sensor(0x370c,0x0c);
     
     OV3660_write_cmos_sensor(0x3a18,0x00);
     OV3660_write_cmos_sensor(0x3a19,0xf8);
     
     OV3660_write_cmos_sensor(0x3000,0x10);
     OV3660_write_cmos_sensor(0x3002,0x1c);
     OV3660_write_cmos_sensor(0x3004,0xef);
     OV3660_write_cmos_sensor(0x3006,0xc3);
     
     OV3660_write_cmos_sensor(0x6700,0x05);
     OV3660_write_cmos_sensor(0x6701,0x19);
     OV3660_write_cmos_sensor(0x6702,0xfd);
     OV3660_write_cmos_sensor(0x6703,0xd1);
     OV3660_write_cmos_sensor(0x6704,0xff);
     OV3660_write_cmos_sensor(0x6705,0xff);

     OV3660_write_cmos_sensor(0x3800,0x00);
     OV3660_write_cmos_sensor(0x3801,0x00);
     OV3660_write_cmos_sensor(0x3802,0x00);
     OV3660_write_cmos_sensor(0x3803,0x00);
     OV3660_write_cmos_sensor(0x3804,0x08);
     OV3660_write_cmos_sensor(0x3805,0x1f);
     OV3660_write_cmos_sensor(0x3806,0x06);
     OV3660_write_cmos_sensor(0x3807,0x09);
     OV3660_write_cmos_sensor(0x3808,0x04);
     OV3660_write_cmos_sensor(0x3809,0x00);
     OV3660_write_cmos_sensor(0x380a,0x03);
     OV3660_write_cmos_sensor(0x380b,0x00);
     OV3660_write_cmos_sensor(0x3810,0x00);
     OV3660_write_cmos_sensor(0x3811,0x08);
     OV3660_write_cmos_sensor(0x3812,0x00);
     OV3660_write_cmos_sensor(0x3813,0x02);
     OV3660_write_cmos_sensor(0x3814,0x31);
     OV3660_write_cmos_sensor(0x3815,0x31);
     
     OV3660_write_cmos_sensor(0x3826,0x23);
     OV3660_write_cmos_sensor(0x303a,0x00);
     OV3660_write_cmos_sensor(0x303b,0x1b);
     OV3660_write_cmos_sensor(0x303c,0x12); // 0x12: 15fps, 0x16: 5fps
     OV3660_write_cmos_sensor(0x303d,0x30);
     OV3660_write_cmos_sensor(0x3824,0x01);
     OV3660_write_cmos_sensor(0x460c,0x20);
     
     OV3660_write_cmos_sensor(0x380c,0x08);
     OV3660_write_cmos_sensor(0x380d,0xfc);
     OV3660_write_cmos_sensor(0x380e,0x03);
     OV3660_write_cmos_sensor(0x380f,0x51);
     
     OV3660_write_cmos_sensor(0x3a08,0x00); // 50hz
     OV3660_write_cmos_sensor(0x3a09,0x7f);
     OV3660_write_cmos_sensor(0x3a0e,0x06);
     OV3660_write_cmos_sensor(0x3a0a,0x00); // 60hz
     OV3660_write_cmos_sensor(0x3a0b,0x6a);
     OV3660_write_cmos_sensor(0x3a0d,0x08);	   
     
     OV3660_write_cmos_sensor(0x3a00,0x38);
     OV3660_write_cmos_sensor(0x3a14,0x03);
     OV3660_write_cmos_sensor(0x3a15,0x10);
     OV3660_write_cmos_sensor(0x3a02,0x03);
     OV3660_write_cmos_sensor(0x3a03,0x10);

     OV3660_write_cmos_sensor(0x3c00,0x04);		
     OV3660_write_cmos_sensor(0x3c01,0x80);	
     
     OV3660_write_cmos_sensor(0x4300,0x30);
     OV3660_write_cmos_sensor(0x440e,0x08);
     OV3660_write_cmos_sensor(0x4520,0x0b);
     OV3660_write_cmos_sensor(0x460b,0x37);
     OV3660_write_cmos_sensor(0x4713,0x02);
     OV3660_write_cmos_sensor(0x471c,0xd0);
     OV3660_write_cmos_sensor(0x5086,0x00);
     
     OV3660_write_cmos_sensor(0x5001,0x03);
     OV3660_write_cmos_sensor(0x5002,0x00);
     OV3660_write_cmos_sensor(0x501f,0x00);
     
     OV3660_write_cmos_sensor(0x3820,0x01);
     OV3660_write_cmos_sensor(0x3821,0x07);
     OV3660_write_cmos_sensor(0x4514,0xbb);
     OV3660_write_cmos_sensor(0x3008,0x02);
	 mDELAY(4);
	//awb                                  
     OV3660_write_cmos_sensor(0x5180,0x0ff); 
     OV3660_write_cmos_sensor(0x5181,0xf0);  
     OV3660_write_cmos_sensor(0x5182,0x0 );  
     OV3660_write_cmos_sensor(0x5183 ,0x14); 
     OV3660_write_cmos_sensor(0x5184 ,0x25); 
     OV3660_write_cmos_sensor(0x5185 ,0x24); 
     OV3660_write_cmos_sensor(0x5186 ,0x4 ); 
     OV3660_write_cmos_sensor(0x5187 ,0xd ); 
     OV3660_write_cmos_sensor(0x5188 ,0x20); 
     OV3660_write_cmos_sensor(0x5189 ,0x88); 
     OV3660_write_cmos_sensor(0x518a ,0x60); 
     OV3660_write_cmos_sensor(0x518b ,0x89); 
     OV3660_write_cmos_sensor(0x518c ,0x6 ); 
     OV3660_write_cmos_sensor(0x518d ,0x84); 
     OV3660_write_cmos_sensor(0x518e ,0x34); 
     OV3660_write_cmos_sensor(0x518f ,0x3c); 
     OV3660_write_cmos_sensor(0x5190 ,0x46); 
     OV3660_write_cmos_sensor(0x5191 ,0xf0); 
     OV3660_write_cmos_sensor(0x5192 ,0x4 ); 
     OV3660_write_cmos_sensor(0x5193 ,0x70); 
     OV3660_write_cmos_sensor(0x5194 ,0xf0); 
     OV3660_write_cmos_sensor(0x5195 ,0xf0); 
     OV3660_write_cmos_sensor(0x5196 ,0x3 ); 
     OV3660_write_cmos_sensor(0x5197 ,0x1 ); 
     OV3660_write_cmos_sensor(0x5198 ,0x4 ); 
     OV3660_write_cmos_sensor(0x5199 ,0xd4); 
     OV3660_write_cmos_sensor(0x519a ,0x4 ); 
     OV3660_write_cmos_sensor(0x519b ,0x0 ); 
     OV3660_write_cmos_sensor(0x519c ,0x7 ); 
     OV3660_write_cmos_sensor(0x519d ,0x1a); 
     OV3660_write_cmos_sensor(0x519e ,0x38); 
//CMX     
     OV3660_write_cmos_sensor(0x5381,0x1c);  
     OV3660_write_cmos_sensor(0x5382,0x5a);
     OV3660_write_cmos_sensor(0x5383,0x12); 
     OV3660_write_cmos_sensor(0x5384,0x06); 
     OV3660_write_cmos_sensor(0x5385,0x76);
     OV3660_write_cmos_sensor(0x5386,0x7c);
     OV3660_write_cmos_sensor(0x5387,0x7c);
     OV3660_write_cmos_sensor(0x5388,0x60);
     OV3660_write_cmos_sensor(0x5389,0x1c);
     OV3660_write_cmos_sensor(0x538a,0x01); 
     OV3660_write_cmos_sensor(0x538b,0x98);
     
     OV3660_write_cmos_sensor(0x5000,0xa7);
 //gamma                                
      OV3660_write_cmos_sensor(0x5490,0x1c); 
      OV3660_write_cmos_sensor(0x5481,0x3 ); 
      OV3660_write_cmos_sensor(0x5482,0x6  );
      OV3660_write_cmos_sensor(0x5483,0x15); 
      OV3660_write_cmos_sensor(0x5484,0x48); 
      OV3660_write_cmos_sensor(0x5485,0x5b); 
      OV3660_write_cmos_sensor(0x5486,0x6f); 
      OV3660_write_cmos_sensor(0x5487,0x7d); 
      OV3660_write_cmos_sensor(0x5488,0x8d); 
      OV3660_write_cmos_sensor(0x5489,0x98); 
      OV3660_write_cmos_sensor(0x548a,0xa2); 
      OV3660_write_cmos_sensor(0x548b,0xaf); 
      OV3660_write_cmos_sensor(0x548c,0xbb); 
      OV3660_write_cmos_sensor(0x548d,0xcd); 
      OV3660_write_cmos_sensor(0x548e,0xdc); 
      OV3660_write_cmos_sensor(0x548f,0xeb); 
     
     OV3660_write_cmos_sensor(0x5000,0xa7);
//LENC
     OV3660_write_cmos_sensor(0x5800,0x1F);
     OV3660_write_cmos_sensor(0x5801,0x12);
     OV3660_write_cmos_sensor(0x5802,0x10);
     OV3660_write_cmos_sensor(0x5803,0x12);
     OV3660_write_cmos_sensor(0x5804,0x14);
     OV3660_write_cmos_sensor(0x5805,0x23);
     OV3660_write_cmos_sensor(0x5806,0x0D);
     OV3660_write_cmos_sensor(0x5807,0x09);
     OV3660_write_cmos_sensor(0x5808,0x07);
     OV3660_write_cmos_sensor(0x5809,0x08);
     OV3660_write_cmos_sensor(0x580A,0x0B);
     OV3660_write_cmos_sensor(0x580B,0x0F);
     OV3660_write_cmos_sensor(0x580C,0x09);
     OV3660_write_cmos_sensor(0x580D,0x04);
     OV3660_write_cmos_sensor(0x580E,0x01);
     OV3660_write_cmos_sensor(0x580F,0x01);
     OV3660_write_cmos_sensor(0x5810,0x05);
     OV3660_write_cmos_sensor(0x5811,0x0B);
     OV3660_write_cmos_sensor(0x5812,0x06);
     OV3660_write_cmos_sensor(0x5813,0x02);
     OV3660_write_cmos_sensor(0x5814,0x00);
     OV3660_write_cmos_sensor(0x5815,0x00);
     OV3660_write_cmos_sensor(0x5816,0x03);
     OV3660_write_cmos_sensor(0x5817,0x09);
     OV3660_write_cmos_sensor(0x5818,0x06);
     OV3660_write_cmos_sensor(0x5819,0x0b);
     OV3660_write_cmos_sensor(0x581A,0x09);
     OV3660_write_cmos_sensor(0x581B,0x0a);
     OV3660_write_cmos_sensor(0x581C,0x0d);
     OV3660_write_cmos_sensor(0x581D,0x10);
     OV3660_write_cmos_sensor(0x581E,0x28);
     OV3660_write_cmos_sensor(0x581F,0x1d);
     OV3660_write_cmos_sensor(0x5820,0x0b);
     OV3660_write_cmos_sensor(0x5821,0x0d);
     OV3660_write_cmos_sensor(0x5822,0x11);
     OV3660_write_cmos_sensor(0x5823,0x21);
     OV3660_write_cmos_sensor(0x5824,0x43);
     OV3660_write_cmos_sensor(0x5825,0x18);
     OV3660_write_cmos_sensor(0x5826,0x09);
     OV3660_write_cmos_sensor(0x5827,0x28);
     OV3660_write_cmos_sensor(0x5828,0x45);
     OV3660_write_cmos_sensor(0x5829,0x29);
     OV3660_write_cmos_sensor(0x582A,0x36);
     OV3660_write_cmos_sensor(0x582B,0x34);
     OV3660_write_cmos_sensor(0x582C,0x26);
     OV3660_write_cmos_sensor(0x582D,0x18);
     OV3660_write_cmos_sensor(0x582E,0x27);
     OV3660_write_cmos_sensor(0x582F,0x42);
     OV3660_write_cmos_sensor(0x5830,0x50);
     OV3660_write_cmos_sensor(0x5831,0x42);
     OV3660_write_cmos_sensor(0x5832,0x17);
     OV3660_write_cmos_sensor(0x5833,0x28);
     OV3660_write_cmos_sensor(0x5834,0x36);
     OV3660_write_cmos_sensor(0x5835,0x34);
     OV3660_write_cmos_sensor(0x5836,0x26);
     OV3660_write_cmos_sensor(0x5837,0x18);
     OV3660_write_cmos_sensor(0x5838,0x44);
     OV3660_write_cmos_sensor(0x5839,0x28);
     OV3660_write_cmos_sensor(0x583A,0x19);
     OV3660_write_cmos_sensor(0x583B,0x27);
     OV3660_write_cmos_sensor(0x583C,0x44);
     OV3660_write_cmos_sensor(0x583D,0xCE);
//AE target     
     OV3660_write_cmos_sensor(0x3a0f,0x30);//0x38); // 40
     OV3660_write_cmos_sensor(0x3a10,0x28);//0x30); // 38
     OV3660_write_cmos_sensor(0x3a1b,0x30);//0x38); // 40
     OV3660_write_cmos_sensor(0x3a1e,0x28);//0x30); // 38
     OV3660_write_cmos_sensor(0x3a11,0x70);
     OV3660_write_cmos_sensor(0x3a1f,0x14);
     
//     OV3660_write_cmos_sensor(0x5302,0x28);
//     OV3660_write_cmos_sensor(0x5303,0x18);
//     OV3660_write_cmos_sensor(0x5306,0x18);
//     OV3660_write_cmos_sensor(0x5307,0x28);
     
     OV3660_write_cmos_sensor(0x4002,0xc5);
     OV3660_write_cmos_sensor(0x4003,0x81);
     OV3660_write_cmos_sensor(0x4005,0x12);
//AE window     
     OV3660_write_cmos_sensor(0x5688,0x22);
     OV3660_write_cmos_sensor(0x5689,0x22);
     OV3660_write_cmos_sensor(0x568a,0x42);//0x82);
     OV3660_write_cmos_sensor(0x568b,0x24);//0x28);
     OV3660_write_cmos_sensor(0x568c,0x42);//0x82);
     OV3660_write_cmos_sensor(0x568d,0x24);//0x28);
     OV3660_write_cmos_sensor(0x568e,0x22);
     OV3660_write_cmos_sensor(0x568f,0x22);
//UV adjust     
//     OV3660_write_cmos_sensor(0x5001,0x83);
//     OV3660_write_cmos_sensor(0x5580,0x06);
//     OV3660_write_cmos_sensor(0x5588,0x00);
//     OV3660_write_cmos_sensor(0x5583,0x40);
//     OV3660_write_cmos_sensor(0x5584,0x2c);	
     
	//CIP                            
	OV3660_write_cmos_sensor(0x5300,0x0c);// 2x = real gain
	OV3660_write_cmos_sensor(0x5301,0x20);
	OV3660_write_cmos_sensor(0x5302,0x10);
	OV3660_write_cmos_sensor(0x5303,0x0 );
	OV3660_write_cmos_sensor(0x5304,0xc );
	OV3660_write_cmos_sensor(0x5305,0x20);
	OV3660_write_cmos_sensor(0x5306,0xc );
	OV3660_write_cmos_sensor(0x5307,0x20);
	OV3660_write_cmos_sensor(0x5309,0x0c);
	OV3660_write_cmos_sensor(0x530a,0x20);
	OV3660_write_cmos_sensor(0x530b,0x0 );
	OV3660_write_cmos_sensor(0x530c,0x10);
	                           
	                           
	//UV adjust                    
	OV3660_write_cmos_sensor(0x5001,0x87);
	OV3660_write_cmos_sensor(0x5580,0x2 );
	OV3660_write_cmos_sensor(0x5583,0x40);
	OV3660_write_cmos_sensor(0x5584,0x40);
	OV3660_write_cmos_sensor(0x5589,0x30);
	OV3660_write_cmos_sensor(0x558a,0x0);
	OV3660_write_cmos_sensor(0x558b,0x78);
	                           
	                           
	//dns                           
	OV3660_write_cmos_sensor(0x5308,0x25);
	OV3660_write_cmos_sensor(0x5304,0x04);
	OV3660_write_cmos_sensor(0x5305,0x20);
	OV3660_write_cmos_sensor(0x5306,0x0c);//08);
	OV3660_write_cmos_sensor(0x5307,0x1c);//50);
	                           
	//Sharpness Auto               
	OV3660_write_cmos_sensor(0x5308,0x25);//00);
	
	OV3660_write_cmos_sensor(0x5300,0x05);
	OV3660_write_cmos_sensor(0x5301,0x0d);//18);
	OV3660_write_cmos_sensor(0x5302,0x15);//18);
	OV3660_write_cmos_sensor(0x5303,0x06);//00);
	
	OV3660_write_cmos_sensor(0x5309,0x08);//10);
	OV3660_write_cmos_sensor(0x530a,0x18);
	OV3660_write_cmos_sensor(0x530b,0x04);
	OV3660_write_cmos_sensor(0x530c,0x18);     
     

}

//modified by chiva for switching camera problem.
static kal_uint32 OV3660_GetSensorID(kal_uint32 *sensorID);
BOOL OV3660_Init(void)
{
	kal_uint32  sensor_id = 0; 

	OV3660_GetSensorID(&sensor_id);
   	SENSOR_LOG("[OV3660_YUV]OV3660Open Read Sensor ID= 0x%04x\n", sensor_id); 
	
	 if (sensor_id != OV3660_SENSOR_ID)
	 {
	     return FALSE;
	 }

	OV3660_Sensor_Driver_Init();

	OV3660_write_cmos_sensor(0x302c,0x03);//set drv current to min.
	OV3660_write_cmos_sensor(0x3008,0x42);//Enter Sleep Mode
	mDELAY(10);

	return TRUE;
}

static OV3660_Preview_Setting(void)
{
     OV3660_write_cmos_sensor(0x3008,0x42);
     
  #ifdef __PREVIEW_SIZE_VGA__
	   /*640X480*/	   
	   OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x20); // bit[2]='1'
	    
	   OV3660_write_cmos_sensor(0x3800,0x01);
     OV3660_write_cmos_sensor(0x3801,0x80);
     OV3660_write_cmos_sensor(0x3802,0x01);
     OV3660_write_cmos_sensor(0x3803,0x1e);
     OV3660_write_cmos_sensor(0x3804,0x06);
     OV3660_write_cmos_sensor(0x3805,0x9f);
     OV3660_write_cmos_sensor(0x3806,0x04);
     OV3660_write_cmos_sensor(0x3807,0xed);
     OV3660_write_cmos_sensor(0x3808,0x02); // 640
     OV3660_write_cmos_sensor(0x3809,0x80);
     OV3660_write_cmos_sensor(0x380a,0x01); // 480
     OV3660_write_cmos_sensor(0x380b,0xe0);
     OV3660_write_cmos_sensor(0x3810,0x00);
     OV3660_write_cmos_sensor(0x3811,0x08);
     OV3660_write_cmos_sensor(0x3812,0x00);
     OV3660_write_cmos_sensor(0x3813,0x04);
     OV3660_write_cmos_sensor(0x3814,0x31);
     OV3660_write_cmos_sensor(0x3815,0x31);
     
     OV3660_write_cmos_sensor(0x3826,0x23);
     OV3660_write_cmos_sensor(0x303a,0x00);
     OV3660_write_cmos_sensor(0x303b,0x1b);
     OV3660_write_cmos_sensor(0x303c,0x12); // 0x12: 15fps, 0x16: 5fps
     OV3660_write_cmos_sensor(0x303d,0x30);
     OV3660_write_cmos_sensor(0x3824,0x01);
     OV3660_write_cmos_sensor(0x460c,0x20);
     
     OV3660_write_cmos_sensor(0x380c,0x08);
     OV3660_write_cmos_sensor(0x380d,0xfc);
     OV3660_write_cmos_sensor(0x380e,0x03);
     OV3660_write_cmos_sensor(0x380f,0x51);
;//banding    
     OV3660_write_cmos_sensor(0x3a08,0x00); // 50hz
     OV3660_write_cmos_sensor(0x3a09,0x7f);
     OV3660_write_cmos_sensor(0x3a0e,0x06);
     OV3660_write_cmos_sensor(0x3a0a,0x00); // 60hz
     OV3660_write_cmos_sensor(0x3a0b,0x6a);
     OV3660_write_cmos_sensor(0x3a0d,0x08);		   	   
  #else
	   /*1024X768*/	   
	   OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)&0xdf); // bit[2]='0'
	   
	   OV3660_write_cmos_sensor(0x3800,0x00);
     OV3660_write_cmos_sensor(0x3801,0x00);
     OV3660_write_cmos_sensor(0x3802,0x00);
     OV3660_write_cmos_sensor(0x3803,0x00);
     OV3660_write_cmos_sensor(0x3804,0x08);
     OV3660_write_cmos_sensor(0x3805,0x1f);
     OV3660_write_cmos_sensor(0x3806,0x06);
     OV3660_write_cmos_sensor(0x3807,0x09);
     OV3660_write_cmos_sensor(0x3808,0x04); // 1024
     OV3660_write_cmos_sensor(0x3809,0x00);
     OV3660_write_cmos_sensor(0x380a,0x03); // 768
     OV3660_write_cmos_sensor(0x380b,0x00);
     OV3660_write_cmos_sensor(0x3810,0x00);
     OV3660_write_cmos_sensor(0x3811,0x08);
     OV3660_write_cmos_sensor(0x3812,0x00);
     OV3660_write_cmos_sensor(0x3813,0x02);
     OV3660_write_cmos_sensor(0x3814,0x31);
     OV3660_write_cmos_sensor(0x3815,0x31);
     
     OV3660_write_cmos_sensor(0x3826,0x23);
     OV3660_write_cmos_sensor(0x303a,0x00);
     OV3660_write_cmos_sensor(0x303b,0x1b);
     OV3660_write_cmos_sensor(0x303c,0x12); // 0x12: 15fps, 0x16: 5fps
     OV3660_write_cmos_sensor(0x303d,0x30);
     OV3660_write_cmos_sensor(0x3824,0x01);
     OV3660_write_cmos_sensor(0x460c,0x20);
     
     OV3660_write_cmos_sensor(0x380c,0x08);
     OV3660_write_cmos_sensor(0x380d,0xfc);
     OV3660_write_cmos_sensor(0x380e,0x03);
     OV3660_write_cmos_sensor(0x380f,0x51);
     
     OV3660_write_cmos_sensor(0x3a08,0x00); // 50hz
     OV3660_write_cmos_sensor(0x3a09,0x7f);
     OV3660_write_cmos_sensor(0x3a0e,0x06);
     OV3660_write_cmos_sensor(0x3a0a,0x00); // 60hz
     OV3660_write_cmos_sensor(0x3a0b,0x6a);
     OV3660_write_cmos_sensor(0x3a0d,0x08);	   
  #endif 
//     OV3660_write_cmos_sensor(0x5302,0x28);  disable sharpness dns setting kimi0621
//     OV3660_write_cmos_sensor(0x5303,0x18);
//     OV3660_write_cmos_sensor(0x5306,0x18);
//     OV3660_write_cmos_sensor(0x5307,0x28);     
     //OV3660_write_cmos_sensor(0x3820,0x01);
     //OV3660_write_cmos_sensor(0x3821,0x07);
     //OV3660_write_cmos_sensor(0x4514,0xbb);
     OV3660_write_cmos_sensor(0x3618,0x00);
     OV3660_write_cmos_sensor(0x3708,0x66);
     OV3660_write_cmos_sensor(0x3709,0x12);
     OV3660_write_cmos_sensor(0x4520,0x0b);   
     
     OV3660_write_cmos_sensor(0x3008,0x02);
}

static OV3660_Capture_Setting(Void)
{
	   OV3660_write_cmos_sensor(0x3008,0x42);
	
  #ifdef __PREVIEW_SIZE_VGA__
	   OV3660_write_cmos_sensor(0x3826,0x23);
     OV3660_write_cmos_sensor(0x303a,0x00);
     OV3660_write_cmos_sensor(0x303b,0x1b);
     OV3660_write_cmos_sensor(0x303c,0x12); // 7.5fps
     OV3660_write_cmos_sensor(0x303d,0x30); 
     OV3660_write_cmos_sensor(0x3c00,0x04);		
     OV3660_write_cmos_sensor(0x3c01,0x80);	
     OV3660_write_cmos_sensor(0x3824,0x01);
     OV3660_write_cmos_sensor(0x460c,0x20);
     
     OV3660_write_cmos_sensor(0x380c,0x08);
     OV3660_write_cmos_sensor(0x380d,0xfc);
     OV3660_write_cmos_sensor(0x380e,0x06);
     OV3660_write_cmos_sensor(0x380f,0xa3);
     
     OV3660_write_cmos_sensor(0x3a08,0x00); // 50hz
     OV3660_write_cmos_sensor(0x3a09,0x7f);
     OV3660_write_cmos_sensor(0x3a0e,0x0c);
     OV3660_write_cmos_sensor(0x3a0a,0x00); // 60hz
     OV3660_write_cmos_sensor(0x3a0b,0x6a);
     OV3660_write_cmos_sensor(0x3a0d,0x0f);	 
  #else
	   OV3660_write_cmos_sensor(0x3826,0x23);
     OV3660_write_cmos_sensor(0x303a,0x00);
     OV3660_write_cmos_sensor(0x303b,0x1b);
     OV3660_write_cmos_sensor(0x303c,0x12); // 7.5fps
     OV3660_write_cmos_sensor(0x303d,0x30); 
     OV3660_write_cmos_sensor(0x3c00,0x04);		
     OV3660_write_cmos_sensor(0x3c01,0x80);	
     OV3660_write_cmos_sensor(0x3824,0x01);
     OV3660_write_cmos_sensor(0x460c,0x20);
     
     OV3660_write_cmos_sensor(0x380c,0x08);
     OV3660_write_cmos_sensor(0x380d,0xfc);
     OV3660_write_cmos_sensor(0x380e,0x06);
     OV3660_write_cmos_sensor(0x380f,0xa3);
     
     OV3660_write_cmos_sensor(0x3a08,0x00); // 50hz
     OV3660_write_cmos_sensor(0x3a09,0x7f);
     OV3660_write_cmos_sensor(0x3a0e,0x0c);
     OV3660_write_cmos_sensor(0x3a0a,0x00); // 60hz
     OV3660_write_cmos_sensor(0x3a0b,0x6a);
     OV3660_write_cmos_sensor(0x3a0d,0x0f);	
  #endif	
;//	   OV3660_write_cmos_sensor(0x5302,0x40);             disable sharpness setting kimi0621
;//    OV3660_write_cmos_sensor(0x5303,0x30);
;//    OV3660_write_cmos_sensor(0x5306,0x18); // 20
;//    OV3660_write_cmos_sensor(0x5307,0x28); // 30
     
     OV3660_write_cmos_sensor(0x3800,0x00);
     OV3660_write_cmos_sensor(0x3801,0x00);
     OV3660_write_cmos_sensor(0x3802,0x00);
     OV3660_write_cmos_sensor(0x3803,0x00);
     OV3660_write_cmos_sensor(0x3804,0x08);
     OV3660_write_cmos_sensor(0x3805,0x1f);
     OV3660_write_cmos_sensor(0x3806,0x06);
     OV3660_write_cmos_sensor(0x3807,0x0b);
     OV3660_write_cmos_sensor(0x3808,0x08); // 2048
     OV3660_write_cmos_sensor(0x3809,0x00);
     OV3660_write_cmos_sensor(0x380a,0x06); // 1536
     OV3660_write_cmos_sensor(0x380b,0x00);
     OV3660_write_cmos_sensor(0x3810,0x00);
     OV3660_write_cmos_sensor(0x3811,0x10);
     OV3660_write_cmos_sensor(0x3812,0x00);
     OV3660_write_cmos_sensor(0x3813,0x06);
     OV3660_write_cmos_sensor(0x3814,0x11);
     OV3660_write_cmos_sensor(0x3815,0x11);       
     //OV3660_write_cmos_sensor(0x3820,0x40);
     //OV3660_write_cmos_sensor(0x3821,0x06);
     //OV3660_write_cmos_sensor(0x4514,0x00);
     OV3660_write_cmos_sensor(0x3618,0x78);
     OV3660_write_cmos_sensor(0x3708,0x63);
     OV3660_write_cmos_sensor(0x3709,0x12);
     OV3660_write_cmos_sensor(0x4520,0xb0);
     
     OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)&0xdf); // bit[2]='0'
     
     OV3660_write_cmos_sensor(0x3008,0x02);
}
/*************************************************************************
* FUNCTION
*	OV3660_GetSensorID
*
* DESCRIPTION
*	This function get the sensor ID
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 OV3660_GetSensorID(kal_uint32 *sensorID)

{
    int retry = 3; 
	
      printk(KERN_INFO "Teng %s\n",__FUNCTION__);
	  OV3660_write_cmos_sensor(0x3008, 0x82);
	  mDELAY(10); // At least 2ms
//	  OV3660_write_cmos_sensor(0x3008, 0x02);
//	  mDELAY(10);

    do 
    {  
        *sensorID=((OV3660_read_cmos_sensor(0x300A)<< 8)|OV3660_read_cmos_sensor(0x300B));
        SENSOR_LOG("[OV3660_YUV]Read Sensor ID=0x%x\n", *sensorID); 
        if (*sensorID == OV3660_SENSOR_ID)
            break; 
        retry--; 
    } while (retry > 0);

    if (*sensorID != OV3660_SENSOR_ID){
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;    
}   /* OV3660Open  */

/*************************************************************************
* FUNCTION
*	OV3660Open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 OV3660Open(void)
{
	 kal_uint16 sensor_id=0; 
	 int retry = 10; 

	 SENSOR_CONTROL_FLOW();

	 //  1.ReSet All Variable For OV3660_Sensor_Struct
   memset(&OV3660_Sensor_Driver, 0, sizeof(struct OV3660_Sensor_Struct)); 

	 //  2./3.To Do sw Reset &  Check Sensor ID
	 OV3660_GetSensorID(&OV3660_Sensor_Driver.Sensor_ID);
     SENSOR_LOG("[OV3660_YUV]OV3660Open Read Sensor ID= 0x%04x\n",OV3660_Sensor_Driver.Sensor_ID); 
	
	 if (OV3660_Sensor_Driver.Sensor_ID != OV3660_SENSOR_ID)
	 {
	     return ERROR_SENSOR_CONNECT_FAIL;
	 }

	 // 4.Init Sensor Module
   OV3660_Sensor_Driver_Init();	

	 // 5. Init OV3660_Sensor_Driver Variable
	 // OV3660_Sensor_Driver.MPEG4_encode_mode=KAL_FALSE;
	 // OV3660_Sensor_Driver.extra_exposure_lines=0;
	 // OV3660_Sensor_Driver.exposure_lines=0;
	
	 OV3660_Sensor_Driver.bIsCapture=KAL_FALSE;		
	 OV3660_Sensor_Driver.bNightMode =KAL_FALSE; // to distinguish night mode or auto mode, default: auto mode setting
	 OV3660_Sensor_Driver.u8BandValue = AE_FLICKER_MODE_50HZ; // to distinguish between 50HZ and 60HZ.		
	 OV3660_Sensor_Driver.AE_Enable = KAL_TRUE; 
	 OV3660_Sensor_Driver.AG_Enable = KAL_TRUE;
	 OV3660_Sensor_Driver.u8ZoomFactor = 0;

	 return ERROR_NONE;
}  /* OV3660Open  */


/*************************************************************************
* FUNCTION
*	OV3660Close
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 OV3660Close(void)
{
	SENSOR_CONTROL_FLOW();
	return ERROR_NONE;
}   /* OV3660Close */

static kal_uint32 OV3660GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	SENSOR_CONTROL_FLOW();
	pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;	
	pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;
	return ERROR_NONE;
}	/* OV3660GetResolution() */

static kal_uint32 OV3660GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
	MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	SENSOR_CONTROL_FLOW();
	
	pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_PARALLEL;
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_HIGH;
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorDriver3D = 0;  
	pSensorInfo->SensorMasterClockSwitch = 0;
	pSensorInfo->PreviewDelayFrame = 3;//3	 
	pSensorInfo->VideoDelayFrame = 3;//3 
	pSensorInfo->CaptureDelayFrame = 2; 	
	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   	

    printk(KERN_INFO "Teng %s, ScenarioId=%d\n",__FUNCTION__, ScenarioId);
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:		
			   pSensorInfo->SensorClockFreq=26;// 24
			   pSensorInfo->SensorClockRisingCount= 0;
			   pSensorInfo->SensorClockFallingCount= 2;			
			   pSensorInfo->SensorGrabStartX = 1; 
			   pSensorInfo->SensorGrabStartY = 2;			   
		     break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:		
			   pSensorInfo->SensorClockFreq=26;// 24
			   pSensorInfo->SensorClockRisingCount= 0;
			   pSensorInfo->SensorClockFallingCount= 2;
			   pSensorInfo->SensorGrabStartX = 1; 
			   pSensorInfo->SensorGrabStartY = 1;			   
		     break;
		default:
			   pSensorInfo->SensorClockFreq=26;// 24
		   	 pSensorInfo->SensorClockRisingCount= 0;
			   pSensorInfo->SensorClockFallingCount= 2;			
			   pSensorInfo->SensorGrabStartX = 1; 
			   pSensorInfo->SensorGrabStartY = 1;			   
		     break;
	}
	       memcpy(pSensorConfigData, &OV3660SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	
	       return ERROR_NONE;
}	/* OV3660GetInfo() */

static void OV3660_HVMirror(ACDK_SENSOR_IMAGE_MIRROR_ENUM SensorImageMirror)
{
	  kal_uint8 flip,mirror;
	  SENSOR_CONTROL_FLOW();
	  	
	  switch (SensorImageMirror)
	  {
		    case IMAGE_NORMAL:
			       if(OV3660_Sensor_Driver.bIsCapture==KAL_FALSE)
			       {
				        OV3660_write_cmos_sensor(0x3820,0x01);
				        OV3660_write_cmos_sensor(0x3821,0x07);
				        OV3660_write_cmos_sensor(0x4514,0xbb);
			       }
			       else
			       {
				        OV3660_write_cmos_sensor(0x3820,0x40);
				        OV3660_write_cmos_sensor(0x3821,0x06);
				        OV3660_write_cmos_sensor(0x4514,0x00);
			       }
			       break;
		    case IMAGE_H_MIRROR:			 
			       if(OV3660_Sensor_Driver.bIsCapture==KAL_FALSE)
			       {
				        OV3660_write_cmos_sensor(0x3820,0x01);
				        OV3660_write_cmos_sensor(0x3821,0x01);
				        OV3660_write_cmos_sensor(0x4514,0xaa);
			       }
			       else
			       {
				        OV3660_write_cmos_sensor(0x3820,0x40);
				        OV3660_write_cmos_sensor(0x3821,0x00);
				        OV3660_write_cmos_sensor(0x4514,0x00);
			       }
			       break;
		    case IMAGE_V_MIRROR:			 
			       if(OV3660_Sensor_Driver.bIsCapture==KAL_FALSE)
			       {
				        OV3660_write_cmos_sensor(0x3820,0x07);
				        OV3660_write_cmos_sensor(0x3821,0x07);
				        OV3660_write_cmos_sensor(0x4514,0xaa);
			       }
			       else
			       {
				        OV3660_write_cmos_sensor(0x3820,0x46);
				        OV3660_write_cmos_sensor(0x3821,0x06);
				        OV3660_write_cmos_sensor(0x4514,0xbb);
			       }
			       break;
		    case IMAGE_HV_MIRROR:						 
			       if(OV3660_Sensor_Driver.bIsCapture==KAL_FALSE)
			       {
				        OV3660_write_cmos_sensor(0x3820,0x07);
				        OV3660_write_cmos_sensor(0x3821,0x01);
				        OV3660_write_cmos_sensor(0x4514,0xbb);
			       }
			       else
			       {
				        OV3660_write_cmos_sensor(0x3820,0x46);
			          OV3660_write_cmos_sensor(0x3821,0x00);
				        OV3660_write_cmos_sensor(0x4514,0x88);//aa
			       }
			       break;
	}

}
/*************************************************************************
* FUNCTION
* OV3660_Preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 OV3660_Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	SENSOR_CONTROL_FLOW();
  //stop_process();

  //OV3660_Write_Shutter(OV3660_Sensor_Driver.PV_Shutter);
  //OV3660_Write_Gain(OV3660_Sensor_Driver.PV_Gain);
  
	if(OV3660_Sensor_Driver.bIsCapture==KAL_TRUE)
		{ 
           OV3660_Preview_Setting();  
	       //mDELAY(50);
		}
    
	OV3660_Sensor_Driver.bIsCapture=KAL_FALSE;//For Mirror/flip Function, will be use this variable
	OV3660_HVMirror(sensor_config_data->SensorImageMirror);

	SENSOR_LOG("sensor_config_data->SensorImageMirror=%x\n",sensor_config_data->SensorImageMirror);
	/*Enable AE/AG*/
	/*If panorama is enable, AE/AG will be disable or it will be enable*/
	OV3660_AECtrl(OV3660_Sensor_Driver.AE_Enable);
	OV3660_AGCtrl(OV3660_Sensor_Driver.AG_Enable);
        OV3660_write_cmos_sensor(0x3503,OV3660_read_cmos_sensor(0x3503)&0xfc);
        //OV3660_write_cmos_sensor(0x3a00,OV3660_read_cmos_sensor(0x3a00)|0x04);       

	OV3660_Sensor_Driver.PV_PCLK =  96;
	OV3660_Sensor_Driver.PV_Dummy_Pixels = 0;
	OV3660_Sensor_Driver.PV_Dummy_Lines = 0;
	OV3660_Sensor_Driver.PV_Pixels_Per_Line = OV3660_DEFUALT_XGA_PIXEL_NUMS + OV3660_Sensor_Driver.PV_Dummy_Pixels;
	OV3660_Sensor_Driver.PV_Lines_Per_Frame = OV3660_DEFULAT_XGA_LINE_NUMS + OV3660_Sensor_Driver.PV_Dummy_Lines;

	OV3660_Set_Dummy(OV3660_Sensor_Driver.PV_Dummy_Pixels, OV3660_Sensor_Driver.PV_Dummy_Lines);
	
  //image_window->GrabStartX= IMAGE_SENSOR_VGA_INSERTED_PIXELS;
  //image_window->GrabStartY= IMAGE_SENSOR_VGA_INSERTED_LINES;
  //image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
  //image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;
	// copy sensor_config_data
	memcpy(&OV3660SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	
	//Step 2. Read Sensor info from Preview Mode
	//OV3660_Sensor_Driver.PV_Gain = (kal_uint32)OV3660_read_gain();
	//OV3660_Sensor_Driver.PV_Shutter = OV3660_read_shutter();
	//SENSOR_LOG("PV_Gain=%x,PV_Shutter=%x\n",OV3660_Sensor_Driver.PV_Gain,OV3660_Sensor_Driver.PV_Shutter);
	return ERROR_NONE;

}   /*  OV3660_Preview   */

/*************************************************************************
* FUNCTION
*	OV3660_Capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 OV3660_Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint32 shutter;//=OV3660_Sensor_Driver.exposure_lines;
	kal_uint32 Capture_Banding_Filter;
	kal_uint32 cap_max_shutter;
	kal_uint32 exposure;
	kal_uint32 cap_exposure;
//added by mandrave
SENSOR_LOG("\nCapture start--mandrave\n");
if(OV3660_Sensor_Driver.bIsCapture==KAL_FALSE)
{
//added end

	SENSOR_CONTROL_FLOW();
  //stop_process();	
	//Step 1.disable AE,AWB
	//Alway it need to disable,even if the paramo is enable or not
	OV3660_AECtrl(KAL_FALSE);
	OV3660_AGCtrl(KAL_FALSE);
	OV3660_write_cmos_sensor(0x3503,OV3660_read_cmos_sensor(0x3503)|0x03);
	//OV3660_write_cmos_sensor(0x3a00,OV3660_read_cmos_sensor(0x3a00)&0xfb);
	
	//Step 2. Read Sensor info from Preview Mode
	//OV3660_Sensor_Driver.PV_Gain = (kal_uint32)OV3660_read_gain();
	OV3660_Sensor_Driver.PV_Shutter = OV3660_read_shutter();
	SENSOR_LOG("PV_Gain=%x,PV_Shutter=%x\n",OV3660_Sensor_Driver.PV_Gain,OV3660_Sensor_Driver.PV_Shutter);
	//Step 3. Apply Capture Setting
	mDELAY(30);
	OV3660_Capture_Setting();

	OV3660_Sensor_Driver.bIsCapture=KAL_TRUE;
	OV3660_HVMirror(sensor_config_data->SensorImageMirror);
	SENSOR_LOG("sensor_config_data->SensorImageMirror=%x\n",sensor_config_data->SensorImageMirror);
	
	//Step 4. Set some flag
	   SENSOR_LOG("u8ZoomFactor%x\n",OV3660_Sensor_Driver.u8ZoomFactor);
	if(OV3660_Sensor_Driver.u8ZoomFactor <=2){
		//OV3660_write_cmos_sensor(0x303c, 0x11);// Pclk=96Mhz
		//OV3660_write_cmos_sensor(0x303b, 0x18);//0x18->0x1b(Default value)
		
		OV3660_Sensor_Driver.CP_PCLK =	96;
		OV3660_Sensor_Driver.CP_Dummy_Pixels = 0;//0
		OV3660_Sensor_Driver.CP_Dummy_Lines = 0;
		
		//OV3660_Sensor_Driver.PV_Shutter=OV3660_Sensor_Driver.PV_Shutter/3;//   /3
	}
	else
	{
		//OV3660_write_cmos_sensor(0x303c, 0x12);// Pclk=48Mhz
		//OV3660_write_cmos_sensor(0x303b, 0x18);//0x18->0x1b(Default value)
		
		OV3660_Sensor_Driver.CP_PCLK =	48;
		OV3660_Sensor_Driver.CP_Dummy_Pixels = 2300;//0
		OV3660_Sensor_Driver.CP_Dummy_Lines = 0;
		
		OV3660_Sensor_Driver.PV_Shutter=OV3660_Sensor_Driver.PV_Shutter/2;
	}
	
	//OV3660_Sensor_Driver.CP_Pixels_Per_Line = OV3660_DEFUALT_QXGA_PIXEL_NUMS + OV3660_Sensor_Driver.CP_Dummy_Pixels;
	//OV3660_Sensor_Driver.CP_Lines_Per_Frame = OV3660_DEFULAT_QXGA_LINE_NUMS + OV3660_Sensor_Driver.CP_Dummy_Lines;
	
	//Step 5. Calculate Shutter
	//shutter = (OV3660_Sensor_Driver.PV_Shutter*OV3660_Sensor_Driver.PV_Pixels_Per_Line*OV3660_Sensor_Driver.PV_PCLK)/(OV3660_Sensor_Driver.CP_Pixels_Per_Line*OV3660_Sensor_Driver.CP_PCLK);	
	//exposure = shutter * OV3660_Sensor_Driver.PV_Gain; 	
	//if (shutter == 0)
	//{
	//	shutter = 1; 
	//}

	//OV3660_Sensor_Driver.CP_Shutter = shutter;
	//OV3660_Sensor_Driver.CP_Gain = exposure / shutter; 
	//OV3660_Sensor_Driver.bIsCapture=KAL_TRUE;

	SENSOR_LOG("CP_Shutter=%x,CP_Gain =%x\n",OV3660_Sensor_Driver.CP_Shutter,OV3660_Sensor_Driver.CP_Gain);

	// Step 6: Set Dummy,Shuter, Gain if necessary
	OV3660_Set_Dummy(OV3660_Sensor_Driver.CP_Dummy_Pixels, OV3660_Sensor_Driver.CP_Dummy_Lines);
	OV3660_Write_Shutter(OV3660_Sensor_Driver.PV_Shutter);
	//OV3660_Write_Gain(OV3660_Sensor_Driver.CP_Gain);
	
	mDELAY(10);//(50);
	
	memcpy(&OV3660SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

  //image_window->GrabStartX = IMAGE_SENSOR_VGA_INSERTED_PIXELS;
  //image_window->GrabStartY = IMAGE_SENSOR_VGA_INSERTED_LINES;
  //image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
  //image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;  

//added by mandrave
}
SENSOR_LOG("\nCapture end--mandrave\n");
//added end

	return ERROR_NONE;
}   

static kal_uint32 OV3660Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	  SENSOR_CONTROL_FLOW();

    printk(KERN_INFO "Teng %s, ScenarioId=%d\n",__FUNCTION__, ScenarioId);
	  switch (ScenarioId)
  {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			                    OV3660_Preview(pImageWindow, pSensorConfigData);
		                      break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			                    OV3660_Capture(pImageWindow, pSensorConfigData);
		                      break;
		default:
			                    return ERROR_INVALID_SCENARIO_ID;
	}
	return TRUE;
}

static BOOL OV3660_set_param_wb(UINT16 para)
{
	kal_uint8 reg3406=0x00;	
	SENSOR_CONTROL_FLOW();

	if(OV3660_Sensor_Driver.u8WbValue==para)
		 return FALSE;	
	OV3660_Sensor_Driver.u8WbValue = para;
  reg3406 = OV3660_read_cmos_sensor(0x3406);
  
	switch (para)
	{
		 case AWB_MODE_AUTO:
			                  OV3660_write_cmos_sensor(0x3406,reg3406&0xfe);
			                  break;
	 	 case AWB_MODE_DAYLIGHT: //sunny
			                  OV3660_write_cmos_sensor(0x3406,reg3406|0x01);
			                  OV3660_write_cmos_sensor(0x3400,0x07);
			                  OV3660_write_cmos_sensor(0x3401,0x1d);
			                  OV3660_write_cmos_sensor(0x3402,0x04);
			                  OV3660_write_cmos_sensor(0x3403,0x00);
			                  OV3660_write_cmos_sensor(0x3404,0x04);
			                  OV3660_write_cmos_sensor(0x3405,0xb4);
			                  break;
		 case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			                  OV3660_write_cmos_sensor(0x3406,reg3406|0x01);
			                  OV3660_write_cmos_sensor(0x3400,0x04);
			                  OV3660_write_cmos_sensor(0x3401,0xa0);
			                  OV3660_write_cmos_sensor(0x3402,0x04);
			                  OV3660_write_cmos_sensor(0x3403,0x00);
			                  OV3660_write_cmos_sensor(0x3404,0x06);
			                  OV3660_write_cmos_sensor(0x3405,0xac);
			                  break;
		 case AWB_MODE_INCANDESCENT: //office 
			                  OV3660_write_cmos_sensor(0x3406,reg3406|0x01);
			                  OV3660_write_cmos_sensor(0x3400,0x05);
			                  OV3660_write_cmos_sensor(0x3401,0x4f);
			                  OV3660_write_cmos_sensor(0x3402,0x04);
			                  OV3660_write_cmos_sensor(0x3403,0x00);
			                  OV3660_write_cmos_sensor(0x3404,0x07);
			                  OV3660_write_cmos_sensor(0x3405,0x35);
			                  break;
		 case AWB_MODE_TUNGSTEN: //home 
			                  OV3660_write_cmos_sensor(0x3406,reg3406|0x01);
			                  OV3660_write_cmos_sensor(0x3400,0x04);
			                  OV3660_write_cmos_sensor(0x3401,0xc7);
		                  	OV3660_write_cmos_sensor(0x3402,0x04);
			                  OV3660_write_cmos_sensor(0x3403,0x00);
			                  OV3660_write_cmos_sensor(0x3404,0x07);
			                  OV3660_write_cmos_sensor(0x3405,0xed);
			                  break;
		 case AWB_MODE_FLUORESCENT: 
			                  OV3660_write_cmos_sensor(0x3406,reg3406|0x01);
			                  OV3660_write_cmos_sensor(0x3400,0x06);
			                  OV3660_write_cmos_sensor(0x3401,0x75);
			                  OV3660_write_cmos_sensor(0x3402,0x04);
			                  OV3660_write_cmos_sensor(0x3403,0x00);
			                  OV3660_write_cmos_sensor(0x3404,0x07);
			                  OV3660_write_cmos_sensor(0x3405,0xad);
			                  break; 
		 default:
			                  return FALSE;
	}
	return TRUE;
} /* OV3660_set_param_wb */

static BOOL OV3660_set_param_effect(UINT16 para)
{
	kal_uint32 ret = KAL_TRUE;
	
	SENSOR_CONTROL_FLOW();

	if(para==OV3660_Sensor_Driver.u8EffectValue)
	 	 return FALSE;
	OV3660_Sensor_Driver.u8EffectValue = para;
	switch (para)
	{
	  	case MEFFECT_OFF:
	  		               OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,0x06);	
			                 OV3660_write_cmos_sensor(0x5583,0x40);	
			                 OV3660_write_cmos_sensor(0x5584,0x2c);	
			                 break;
		  case MEFFECT_MONO:
		  	               OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,0x26);			                 
			                 break;
		  case MEFFECT_SEPIA: 
		  	               OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,0x1e);
			                 OV3660_write_cmos_sensor(0x5583,0x40);
			                 OV3660_write_cmos_sensor(0x5584,0xa0);
			                 break;
		  case MEFFECT_NEGATIVE:		
		  	               OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);	                 	
			                 OV3660_write_cmos_sensor(0x5580,0x46);			                
			                 break;
		  case MEFFECT_SOLARIZE:
		  case MEFFECT_POSTERIZE: 
		  case MEFFECT_AQUA: 
		  case MEFFECT_BLACKBOARD: 
		  case MEFFECT_WHITEBOARD: 			
			                 break;
		  case MEFFECT_SEPIAGREEN:
		  	               OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,0x1e);
			                 OV3660_write_cmos_sensor(0x5583,0x60);
			                 OV3660_write_cmos_sensor(0x5584,0x60);	
		                   break;
		  case MEFFECT_SEPIABLUE:
		  	               OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,0x1e);
			                 OV3660_write_cmos_sensor(0x5583,0xa0);
			                 OV3660_write_cmos_sensor(0x5584,0x40);
			                 break;
		  default:
			                 ret = FALSE;
	}
	    return ret;
} /* OV3660_set_param_effect */

static BOOL OV3660_set_param_banding(UINT16 para)
{
	kal_uint32 pclok,frame_length,line_length;
	kal_uint16 base_shutter,banding;
	kal_uint8 reg3C00;

	SENSOR_CONTROL_FLOW();

	if(OV3660_Sensor_Driver.u8BandValue == para)
		 return TRUE;
	
	OV3660_Sensor_Driver.u8BandValue = para;

	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			                        OV3660_write_cmos_sensor(0x3c00,0x04);		
			                        OV3660_write_cmos_sensor(0x3c01,0x80);	
			                        break;
		case AE_FLICKER_MODE_60HZ:
			                        OV3660_write_cmos_sensor(0x3c00,0x00);			
			                        OV3660_write_cmos_sensor(0x3c01,0x80);		
			                        break;
		default:
			                        return FALSE;
	}
	return TRUE;
}

static BOOL OV3660_set_param_exposure(UINT16 para)
{
	SENSOR_CONTROL_FLOW();
	SENSOR_LOG("OV3660_set_param_exposure=%x",para);

	if(para == OV3660_Sensor_Driver.u8EvValue)
		 return FALSE;
	OV3660_Sensor_Driver.u8EvValue = para;
	/*For YUV, Only support EV=-2,-1,0,1,2*/
	switch (para)
	{
		case AE_EV_COMP_00:	
			                 OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,OV3660_read_cmos_sensor(0x5580)|0x04);
			                 OV3660_write_cmos_sensor(0x5588,0x00);	
			                 OV3660_write_cmos_sensor(0x5587,0x00);			               
			                 break;
		case AE_EV_COMP_03:
		case AE_EV_COMP_05:
		case AE_EV_COMP_07:
			                 break;
		case AE_EV_COMP_10:// EV compensate 1.0
			                 OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,OV3660_read_cmos_sensor(0x5580)|0x04);
			                 OV3660_write_cmos_sensor(0x5588,0x00);	
			                 OV3660_write_cmos_sensor(0x5587,0x10);		
			                 break;
		case AE_EV_COMP_13:// EV compensate 1.3
		case AE_EV_COMP_15:// EV compensate 1.5
		case AE_EV_COMP_17:// EV compensate 1.7
			                 break;
		case AE_EV_COMP_20:// EV compensate 2.0
			                 OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,OV3660_read_cmos_sensor(0x5580)|0x04);
			                 OV3660_write_cmos_sensor(0x5588,0x00);	
			                 OV3660_write_cmos_sensor(0x5587,0x20);		
		                	 break;
		case AE_EV_COMP_n03:// EV compensate -0.3	
			                 break;
		case AE_EV_COMP_n05:// EV compensate -0.5
		case AE_EV_COMP_n07:// EV compensate -0.7
			                 break;
		case AE_EV_COMP_n10:// EV compensate -1.0
			                 OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,OV3660_read_cmos_sensor(0x5580)|0x04);
			                 OV3660_write_cmos_sensor(0x5588,0x08);	
			                 OV3660_write_cmos_sensor(0x5587,0x10);		
			                 break;
		case AE_EV_COMP_n13: // EV compensate -1.3	
		case AE_EV_COMP_n15:// EV compensate -1.5
		case AE_EV_COMP_n17:// EV compensate -1.7
			                 break;
		case AE_EV_COMP_n20:// EV compensate -2.0
			                 OV3660_write_cmos_sensor(0x5001,OV3660_read_cmos_sensor(0x5001)|0x80);
			                 OV3660_write_cmos_sensor(0x5580,OV3660_read_cmos_sensor(0x5580)|0x04);
			                 OV3660_write_cmos_sensor(0x5588,0x08);	
			                 OV3660_write_cmos_sensor(0x5587,0x20);	
			                 break;
		 default:
			                 return FALSE;
		}
	   return TRUE;
} /* OV3660_set_param_exposure */

static void OV3660_night_mode(kal_bool bEnable);

static kal_uint32 OV3660_YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
	SENSOR_CONTROL_FLOW();

	if(OV3660_Sensor_Driver.bIsCapture)
		 return;

    printk(KERN_INFO "Teng %s, iCmd=0x%x\n",__FUNCTION__, iCmd);
	switch (iCmd) {
		case FID_SCENE_MODE: //  TODO:
		     if (iPara == SCENE_MODE_OFF)
		     {
		         OV3660_night_mode(FALSE); 
		     }
		     else if
		    (iPara == SCENE_MODE_NIGHTSCENE){
		      	 OV3660_night_mode(TRUE); 
		     }	    
		     break; 
		case FID_COLOR_EFFECT:
			   OV3660_set_param_effect(iPara);
			   break;
		case FID_AWB_MODE:
			   OV3660_set_param_wb(iPara);
			   break;
		case FID_AE_FLICKER:
			   OV3660_set_param_banding(iPara);
			   break;		
		case FID_AE_EV:
			   OV3660_set_param_exposure(iPara);
			   break;
		case FID_AE_SCENE_MODE:/*For Parama*/
#if 0			
			if (iPara == AE_MODE_OFF) {
				  OV3660_Sensor_Driver.AE_Enable = KAL_FALSE; 
				  OV3660_Sensor_Driver.AG_Enable = KAL_FALSE;
			}else {
				  OV3660_Sensor_Driver.AE_Enable = KAL_TRUE; 
				  OV3660_Sensor_Driver.AG_Enable = KAL_TRUE;
			}
			    OV3660_AECtrl(OV3660_Sensor_Driver.AE_Enable);
			    OV3660_AGCtrl(OV3660_Sensor_Driver.AG_Enable);
#endif
			    break;
		case FID_AE_METERING:
		case FID_AE_ISO:
		case FID_AE_STROBE:
			    break;			
		case FID_AF_MODE:
		case FID_AF_METERING:
			    break;			
		case FID_ISP_EDGE:
		case FID_ISP_HUE:
		case FID_ISP_SAT:
		case FID_ISP_BRIGHT:
		case FID_ISP_CONTRAST:
			    break;			
		case FID_CAPTURE_MODE:
		case FID_CAP_SIZE:
		case FID_PREVIEW_SIZE:
		case FID_FRAME_RATE:
		case FID_FRAME_RATE_RANGE:
			    break;			
		case FID_FOCUS_DIST_NORMAL:
		case FID_FOCUS_DIST_MACRO:
			    break;
		case FID_FD_ON_OFF:
			    break;
		case FID_EIS:
			    break;
		case FID_ZOOM_FACTOR: //  TODO: The macor define?
	    	  OV3660_Sensor_Driver.u8ZoomFactor= iPara; 	
			    break;
		default:
			    break;
	}	
	  return TRUE;
}   /* OV3660_YUVSensorSetting */

/*************************************************************************
* FUNCTION
*	OV3660_NightMode
*
* DESCRIPTION
*	This function night mode of OV3660.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void OV3660_night_mode(kal_bool bEnable)
{
    printk(KERN_INFO "Teng %s, bEnable=%d\n",__FUNCTION__, bEnable);
	if(OV3660_Sensor_Driver.bNightMode == bEnable)
		 return;
	OV3660_Sensor_Driver.bNightMode = bEnable;	
	 
	if(bEnable)
	{ 
		//night mode
	  OV3660_write_cmos_sensor(0x3a00, 0x3c);
		OV3660_write_cmos_sensor(0x3a02, 0x09);//decrease 1/3
		OV3660_write_cmos_sensor(0x3a03, 0xf3);
		OV3660_write_cmos_sensor(0x3a14, 0x09);
		OV3660_write_cmos_sensor(0x3a15, 0xf3);
	}
	else
	{
	  //normal mode
	  OV3660_write_cmos_sensor(0x3a00, 0x38);
		OV3660_write_cmos_sensor(0x3a02, 0x03);
		OV3660_write_cmos_sensor(0x3a03, 0x51);
		OV3660_write_cmos_sensor(0x3a14, 0x03);
		OV3660_write_cmos_sensor(0x3a15, 0x51);
	}
}

static kal_uint32 OV3660_YUVSetVideoMode(UINT16 u2FrameRate)
{
	SENSOR_CONTROL_FLOW();
	SENSOR_LOG("Video Set Frame Rate = %d",u2FrameRate);	
    if (u2FrameRate == 30)
	  {
        OV3660_write_cmos_sensor(0x3826,0x23);
        OV3660_write_cmos_sensor(0x303a,0x00);
        OV3660_write_cmos_sensor(0x303b,0x1b);
        OV3660_write_cmos_sensor(0x303c,0x12); // 0x12: 15fps, 0x16: 5fps
        OV3660_write_cmos_sensor(0x303d,0x30);
        OV3660_write_cmos_sensor(0x3824,0x01);
        OV3660_write_cmos_sensor(0x460c,0x20);
        
        OV3660_write_cmos_sensor(0x380c,0x08);
        OV3660_write_cmos_sensor(0x380d,0xfc);
        OV3660_write_cmos_sensor(0x380e,0x03);
        OV3660_write_cmos_sensor(0x380f,0x51);
        
        OV3660_write_cmos_sensor(0x3a08,0x00); // 50hz
        OV3660_write_cmos_sensor(0x3a09,0x7f);
        OV3660_write_cmos_sensor(0x3a0e,0x06);
        OV3660_write_cmos_sensor(0x3a0a,0x00); // 60hz
        OV3660_write_cmos_sensor(0x3a0b,0x6a);
        OV3660_write_cmos_sensor(0x3a0d,0x08);	
	
	      OV3660_write_cmos_sensor(0x3a00,0x38);
	      OV3660_write_cmos_sensor(0x3a02,0x03);
	      OV3660_write_cmos_sensor(0x3a03,0x10);
	      OV3660_write_cmos_sensor(0x3a14,0x03);
	      OV3660_write_cmos_sensor(0x3a15,0x10);
	  }
    else if (u2FrameRate == 15)   
    {
    	/*  OV3660_write_cmos_sensor(0x3826,0x23);
        OV3660_write_cmos_sensor(0x303a,0x00);
        OV3660_write_cmos_sensor(0x303b,0x1b);
        OV3660_write_cmos_sensor(0x303c,0x14);//7.5fps
        OV3660_write_cmos_sensor(0x303d,0x30);
        OV3660_write_cmos_sensor(0x3824,0x01);
        OV3660_write_cmos_sensor(0x460c,0x20);
        
        OV3660_write_cmos_sensor(0x380c,0x08);
        OV3660_write_cmos_sensor(0x380d,0xfc);
        OV3660_write_cmos_sensor(0x380e,0x03);
        OV3660_write_cmos_sensor(0x380f,0x51);
        
        OV3660_write_cmos_sensor(0x3a08,0x00);//50hz
        OV3660_write_cmos_sensor(0x3a09,0x3f);
        OV3660_write_cmos_sensor(0x3a0e,0x0c);
        OV3660_write_cmos_sensor(0x3a0a,0x00);//60hz
        OV3660_write_cmos_sensor(0x3a0b,0x35);
        OV3660_write_cmos_sensor(0x3a0d,0x0f);	*/
	
	      OV3660_write_cmos_sensor(0x3a00,0x3c);
	      OV3660_write_cmos_sensor(0x3a02,0x06);
	      OV3660_write_cmos_sensor(0x3a03,0xa2);
	      OV3660_write_cmos_sensor(0x3a14,0x06);
	      OV3660_write_cmos_sensor(0x3a15,0xa2); 	    	
    }   
    else 
    {
        printk("Wrong frame rate setting \n");
    }   
    return TRUE;
}

UINT32 OV3660SetSoftwarePWDNMode(kal_bool bEnable)
{
	  SENSOR_CONTROL_FLOW();
	  // enable software sleep mode   
    printk(KERN_INFO "Teng %s, bEnable=%d\n",__FUNCTION__, bEnable);
    if(bEnable) 
    { 
	     OV3660_write_cmos_sensor(0x3008,0x42);//Enter Sleep Mode
    } 
    else
    {
       OV3660_write_cmos_sensor(0x3008,0x02); //Exit Sleep Mode
    }
    return TRUE;
} 

/*************************************************************************
* FUNCTION
*    OV3660_get_size
*
* DESCRIPTION
*    This function return the image width and height of image sensor.
*
* PARAMETERS
*    *sensor_width: address pointer of horizontal effect pixels of image sensor
*    *sensor_height: address pointer of vertical effect pixels of image sensor
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV3660_get_size(kal_uint16 *sensor_width, kal_uint16 *sensor_height)
{
	SENSOR_CONTROL_FLOW();
	*sensor_width = IMAGE_SENSOR_FULL_WIDTH; /* must be 4:3 */
	*sensor_height = IMAGE_SENSOR_FULL_HEIGHT;
}

/*************************************************************************
* FUNCTION
*    OV3660_get_period
*
* DESCRIPTION
*    This function return the image width and height of image sensor.
*
* PARAMETERS
*    *pixel_number: address pointer of pixel numbers in one period of HSYNC
*    *line_number: address pointer of line numbers in one period of VSYNC
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV3660_get_period(kal_uint16 *pixel_number, kal_uint16 *line_number)
{
  *pixel_number = (kal_uint16)VGA_PERIOD_PIXEL_NUMS+OV3660_Sensor_Driver.PV_Dummy_Pixels;
  *line_number = (kal_uint16)VGA_PERIOD_LINE_NUMS+OV3660_Sensor_Driver.PV_Dummy_Lines;
}

/*************************************************************************
* FUNCTION
*    OV3660_feature_control
*
* DESCRIPTION
*    This function control sensor mode
*
* PARAMETERS
*    id: scenario id
*    image_window: image grab window
*    cfg_data: config data
*
* RETURNS
*    error code
*
* LOCAL AFFECTED
*
*************************************************************************/
static kal_uint32 OV3660FeatureControl(MSDK_SENSOR_FEATURE_ENUM id, kal_uint8 *para, kal_uint32 *len)
{
	UINT16 *pFeatureData16=(UINT16 *) para;
	UINT32 *pFeatureData32=(UINT32 *) para;

	SENSOR_CONTROL_FLOW();

    printk(KERN_INFO "Teng %s, id=%d\n",__FUNCTION__, id);
	switch (id)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			OV3660_get_size((kal_uint16 *)para, (kal_uint16 *)(para + sizeof(kal_uint16)));
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_GET_PERIOD:
			OV3660_get_period((kal_uint16 *)para, (kal_uint16 *)(para + sizeof(kal_uint16)));
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*(kal_uint32 *)para = OV3660_Sensor_Driver.PV_PCLK * 1000000;
			*len = sizeof(kal_uint32);
			printk(KERN_INFO "Teng %s, PV_PCLK=%d\n",__FUNCTION__, OV3660_Sensor_Driver.PV_PCLK);
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			OV3660_night_mode((kal_bool)*(kal_uint16 *)para);
			break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			OV3660_write_cmos_sensor(((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegAddr, ((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegData = OV3660_read_cmos_sensor(((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegAddr);
			break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
			//memcpy(&OV3660_Sensor_Driver.eng.CCT, para, sizeof(OV3660_Sensor_Driver.eng.CCT));
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
		case SENSOR_FEATURE_GET_CONFIG_PARA:
		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		case SENSOR_FEATURE_GET_GROUP_COUNT:
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
			break;
		case SENSOR_FEATURE_GET_ENG_INFO:
		/*
		* get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
		* if EEPROM does not exist in camera module.
		*/
			*(kal_uint32 *)para = LENS_DRIVER_ID_DO_NOT_CARE;
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			break;
		case SENSOR_FEATURE_SET_YUV_CMD:
			OV3660_YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:	
			OV3660_YUVSetVideoMode(*pFeatureData16);
		case SENSOR_FEATURE_SET_CALIBRATION_DATA:
		case SENSOR_FEATURE_SET_SENSOR_SYNC:
		case SENSOR_FEATURE_INITIALIZE_AF:
		case SENSOR_FEATURE_CONSTANT_AF:
		case SENSOR_FEATURE_MOVE_FOCUS_LENS:
		case SENSOR_FEATURE_GET_AF_STATUS:
		case SENSOR_FEATURE_GET_AF_INF:
		case SENSOR_FEATURE_GET_AF_MACRO:
			break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			OV3660_GetSensorID(pFeatureData32); 
			break; 	
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		case SENSOR_FEATURE_SET_TEST_PATTERN:
			break;
		case SENSOR_FEATURE_SET_SOFTWARE_PWDN:
			OV3660SetSoftwarePWDNMode((BOOL)*pFeatureData32);        	        	
			break;
		case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
		case SENSOR_FEATURE_CANCEL_AF:
		case SENSOR_FEATURE_SET_AF_WINDOW:
			break;
		default:
			break;
	}
	return ERROR_NONE;
}


UINT32 OV3660_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	static SENSOR_FUNCTION_STRUCT SensorFuncOV3660=
	{
		.SensorOpen = OV3660Open,
		.SensorGetInfo = OV3660GetInfo,
		.SensorGetResolution = OV3660GetResolution,
		.SensorFeatureControl = OV3660FeatureControl,
		.SensorControl = OV3660Control,
		.SensorClose = OV3660Close
	};

    printk(KERN_INFO "Teng %s\n",__FUNCTION__);
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV3660;
	return ERROR_NONE;
}	/* OV3660_YUV_SensorInit() */

