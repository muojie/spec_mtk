/****************************************Copyright (c)****************************************************
**                                 http://www.PowerAVR.com
**								   http://www.PowerMCU.com
**--------------File Info---------------------------------------------------------------------------------
** File name:           main.c
** Last modified Date:  2010-05-12
** Last Version:        V1.00
** Descriptions:        The main() function example template
**
**--------------------------------------------------------------------------------------------------------
** Created by:          PowerAVR
** Created date:        2010-05-10
** Version:             V1.00
** Descriptions:        编写示例代码
**
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/
#include "LPC17xx.h"
#include "uart.h"

/*********************************************************************************************************
** Function name:       Delay
** Descriptions:        软件延时
** input parameters:    ulTime
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/


/*引脚初始化*/
void Rest(void)
{
	char i;
	LPC_GPIO2->FIODIR |= (1<<0);			//复位脚设置成输出
	LPC_GPIO2->FIODIR |= (1<<7);  			//数据脚设置成输出
	LPC_GPIO2->FIOPIN &= ~(1<<0); 			//复位脚拉低
 
	for(i=0;i<108;i++); 
	LPC_GPIO2->FIOPIN |= (1<<7);			 //数据较保持高电平 
	LPC_GPIO2->FIOPIN |= (1<<0);			 //复位脚拉高
}


char SendBuf_0[4]={0xff,0x10,0x96,0x79};		 //发送提速命令
char SendBuf_1[5]={0x00,0x84,0x00,0x00,0x08} ;	 //发送获取随机数命令
int main(void)
{
	char usFdiv;
	SystemInit();
	UART0_Init();								 //初始化串口
	Rest();										 //复位
	UART0_GetChar(17);							 //接收复位信息
		  
	UART0_SendByte(4,SendBuf_0); 				 //发送提速命令
	UART0_GetChar(4);

	LPC_UART0->LCR  = 0x83;                      /* 允许设置波特率               */
    usFdiv = (11059200*2 / 16) / 230400;         /* 设置波特率                   */
    LPC_UART0->DLM  = usFdiv / 256;
    LPC_UART0->DLL  = usFdiv % 256; 
	
    LPC_UART0->LCR  = 0x1F;                      /* 锁定波特率                   */
    LPC_UART0->FCR  = 0x87; 					  //清空接收发送寄存器

	UART0_SendByte(5,SendBuf_1);				  //发送获取随机数命令
	UART0_GetChar(11);							  //获取随机数
	while(1);
}
