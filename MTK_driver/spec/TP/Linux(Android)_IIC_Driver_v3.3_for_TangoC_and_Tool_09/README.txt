*****************************************************************************************
*											*
*			 Linux/android I2C driver & Tool for TangoC from V3.0		*
*				author:Bee/Jessay/Hangsang				*
*											*
*           ___    _____   ______							*
* |\   |   /   \     |     |								*
* | \  |  |     |    |     |_____ 							*
* |  \ |  |     |    |     |								*
* |   \|   \___/     |     |_____  							*
*											*
* ALL TOOLS IN THIS PACKAGE ONLY USE WITH PIXCIR RELEASE DRIVER				*
* OR YOUR SELF DRIVER COMPATIBLE WITH PIXCIR'S.						*
*											*
*****************************************************************************************

Folder:
Android_APK_Tool:apk for android
Android_Terminal_Tool: debug tool for Android under terminal
android_usb_debug: adb tool for windows
Linux(Android)_IIC_Driver_Vx.x.x: driver source code

---2012-01-12
Driver:
	Update to v3.3.09 & v3.3.0A
		Add Android early power management
		Add irq_flag for pixcir debug tool
		Add CRC attb check when bootloader

Tool:
	Terminal Tool:
		Release TangoC_Engineer_Tertool_v1.4
		Add FW download CRC
		Add power mode change when showing raw data

	APK Tool:
		Release TangoC_Engineer_UITool_v1.4.4.apk
		Add FW download CRC
		Add drawing app
		Add Raw data show(rawdata/internal/raster)
		Add read CRC and FW version


---2011-11-18
Driver:
	Change GPL License to Pixcir Proprietary 

Tool:
	Terminal Tool:
		Release TangoC_Engineer_Tertool_v1.1_stable


---2011-11-13
Driver:
	Update to v3.2.09
		1.for INT_MODE 0x09
		2.adaptable report rate by self
		3.add power management
		4.implove multi-finger drowing performance

Tool:
	Terminal Tool:
		release Tangoc_i2ctool_eng_v1.1_beta
		1.Add Raw data show
		2.Add Internal data show
		3.Add Raster data show
		4.Add CRC read
		5.Add EEPROM read


---2011-09-16
Driver:
	Update to v3.1
	1.Add bootloader function	7
 	2.Add RESET_TP			9
	3.Add ENABLE_IRQ		10
	4.Add DISABLE_IRQ		11
	5.Add BOOTLOADER_STU		16
	6.Add ATTB_VALUE		17
	7.Add Write/Read Interface for APP software

Tool:
	Terminal Tool:
		release V1.0
		1.Add update firmware function

	APK Tool
		Update to v1.2
		1.Add update firmware function


---2011-07-13
Driver:
	1.V3.0,Initial version for TangoC support 5 fingers report,and remove early solutins
	2.Add app interface for calibartion

Tool:
	1.Calibration tool