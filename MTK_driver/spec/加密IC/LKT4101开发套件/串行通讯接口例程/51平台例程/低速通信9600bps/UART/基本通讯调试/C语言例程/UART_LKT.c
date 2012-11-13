/*单片机实现跟LKT加密芯片的通讯，先正确实现下面复位应答以及获取随机数，这样通讯就初步成功*/
  /*函数功能为：（1） 实现LKT加密芯片的复位 （2）实现获取八字节的随机数*/
#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int

uchar buf[25];
uchar Sendbuf[5]={0x00,0x84,0x00,0x00,0x08};
sbit	REST = P2^0;
sbit	IO   = P3^1;

void Delay()					
{									
	uchar   i;
	for(i=0;i<27;i++);
}

void Rest()							
{
	IO   = 1;			    //必须为高等待接收LKT加密芯片发送的起始位
	REST = 0;
	Delay();
	REST = 1; 
}
void Uart_init()
{
	TMOD = 0x20;			// 定时器1工作于8位自动重载模式, 用于产生波特率
	TH1 = 0xFD;				// 波特率9600
	TL1 = 0xFD;
	SCON = 0xD0;			// 设定串行口工作方式
	PCON &= 0xef;			// 波特率不倍增	
	TR1 = 1;				// 启动定时器1
	IE = 0x0;				// 禁止任何中断
}

void Rece(uchar m)
{
	   uchar i;
	   for(i=0; i<m;i++)
	   {
			while(!RI);			// 是否有数据到来				
			buf[i] = SBUF;		// 暂存接收到的数据	  		
			RI = 0;	
		}

}
void Send()
{
	uchar i;
	for(i=0;i<5;i++)
	{
		ACC  = Sendbuf[i];
		TB8  = P;				//将奇偶校验位放在TB8
		SBUF = Sendbuf[i];
		while(!TI);
		TI = 0;
		Delay();
		
	}

}	
void main()
{	
	Uart_init();
	Rest();	   
	Rece(17);
	Delay();
	Send();	
	Rece(11);
	P0 = buf[9];
	while(1);	
}