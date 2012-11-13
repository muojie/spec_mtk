//if your TP has button please define this
#define BUTTON

static int attb_read_val(void);
static void pixcir_init(void);
static void pixcir_reset(void);

#define X_MAX 800//13800//1024
#define Y_MAX 480//8000//768

//#define BUTTON   //if have button on TP

/*********************Platform gpio define************************/
//#define	S5PC1XX
#define 	MINI6410
//#define 	MINI210
//#define		AMLOGIC


#ifdef S5PC1XX

	#include <plat/gpio-bank-e1.h> //reset pin GPE1_5
	#include <plat/gpio-bank-h1.h> //attb pin GPH1_3
	#include <mach/gpio.h>
	#include <plat/gpio-cfg.h>

	#define ATTB		S5PC1XX_GPH1(3)
	#define get_attb_value	gpio_get_value
	#define	RESETPIN_CFG	s3c_gpio_cfgpin(S5PC1XX_GPE1(5),S3C_GPIO_OUTPUT)
	#define	RESETPIN_SET0 	gpio_direction_output(S5PC1XX_GPE1(5),0)
	#define	RESETPIN_SET1	gpio_direction_output(S5PC1XX_GPE1(5),1)
#endif

#ifdef MINI6410	//mini6410

	#include <plat/gpio-cfg.h>
	#include <mach/gpio-bank-e.h>
	#include <mach/gpio-bank-n.h>
	#include <mach/gpio.h>

	#define ATTB		S3C64XX_GPN(11)
	#define get_attb_value	gpio_get_value
	#define	RESETPIN_CFG	s3c_gpio_cfgpin(S3C64XX_GPE(1),S3C_GPIO_OUTPUT)
	#define	RESETPIN_SET0 	gpio_direction_output(S3C64XX_GPE(1),0)
	#define	RESETPIN_SET1	gpio_direction_output(S3C64XX_GPE(1),1)
#endif

#ifdef MINI210
	#include <plat/gpio-cfg.h>
	#include <mach/gpio-bank.h>
	#include <mach/gpio.h>

	#define ATTB		S5PV210_GPH0(0)
	#define get_attb_value	gpio_get_value
	#define	RESETPIN_CFG	s3c_gpio_cfgpin(S5PV210_GPJ3(3),S3C_GPIO_OUTPUT)
	#define	RESETPIN_SET0 	gpio_direction_output(S5PV210_GPJ3(3),0)
	#define	RESETPIN_SET1	gpio_direction_output(S5PV210_GPJ3(3),1)
#endif

#ifdef AMLOGIC

	#include <linux/workqueue.h>
	#include <linux/smp_lock.h>
	#include <linux/gpio.h>

	#define	GPIO_PIXCIR_PENIRQ	((GPIOD_bank_bit2_24(24)<<16) |GPIOD_bit_bit2_24(24)) 
	#define ATTB			GPIO_PIXCIR_PENIRQ
	#define get_attb_value		gpio_get_value
	#define	RESETPIN_CFG		set_gpio_mode(GPIOD_bank_bit2_24(23), GPIOD_bit_bit2_24(23), GPIO_OUTPUT_MODE);
	#define	RESETPIN_SET0		set_gpio_val(GPIOD_bank_bit2_24(23), GPIOD_bit_bit2_24(23), 0);
	#define	RESETPIN_SET1		set_gpio_val(GPIOD_bank_bit2_24(23), GPIOD_bit_bit2_24(23), 1);
#endif


static int attb_read_val(void)
{
	return get_attb_value(ATTB);
}

static void pixcir_reset(void)
{
	RESETPIN_CFG;
	RESETPIN_SET1;
	mdelay(10);
	RESETPIN_SET0;
}

static void pixcir_init(void)
{
#ifdef AMLOGIC
	pixcir_reset();
	mdelay(60);
	/* set input mode */
    	gpio_direction_input(GPIO_PIXCIR_PENIRQ);
    	/* set gpio interrupt #0 source=GPIOD_24, and triggered by falling edge(=1) */
    	gpio_enable_edge_int(50+24, 1, 0);
#endif
	RESETPIN_SET0;
}


