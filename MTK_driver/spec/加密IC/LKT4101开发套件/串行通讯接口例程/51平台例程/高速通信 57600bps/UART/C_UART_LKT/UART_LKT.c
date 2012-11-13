  /*函数功能为：（1） 实现LKT加密芯片的复位 （2）实现获取八字节的随机数
  *********************单片机晶振为11.0592MHZ，   加密芯片使用3.6864MHZ *******************************/
#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int

uchar buf[25]=0;
uchar Sendbuf[4]={0xFF,0x10,0x94,0x7B};			 //	发送高速通讯命令，该命令的通讯速度为（fosc/64）  
uchar Sendbuf_1[5]={0x00,0x84,0x00,0x00,0x08};	 //获取八字节随机数命令

sbit	REST = P2^0;							 //复位脚

void Delay()					
{									
	uchar   i;
	for(i=0;i<26;i++);
}						

/*UART初始化函数：1起始位、8数据位、偶校验、2停止位 */
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

/*9600Kbps下的接收函数*/
void Rest()							
{
	REST = 0;
	Delay();
	REST = 1; 
}
	
/*57600Kbps下的接收函数*/
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
/*9600Kbps下的发送函数*/

void Send()
{
	uchar i;
	for(i=0;i<4;i++)
	{
		ACC  = Sendbuf[i];
		TB8  = P;				//将奇偶校验位放在TB8
		SBUF = Sendbuf[i];
		while(!TI);				// 是否数据传送完成
		TI = 0;
		Delay(); 				//延时1个etu 因为51单片机只能发送1个停止位，
								//所以加一个延时延长停止为时间
	}

}

/*57600Kbps下的发送函数*/

void Send_1()
{
	uchar i,sec;
	for(i=0;i<5;i++)
	{
		ACC  = Sendbuf_1[i];
		TB8  = P;				 //将奇偶校验位放在TB8
		SBUF = Sendbuf_1[i];
		while(!TI);
		TI = 0;					  //延时1个etu 因为51单片机只能发送1个停止位，
		for(sec=0;sec<4;sec++);   //所以加一个延时延长停止为时间		
	}

}	

void main()
{	
	Uart_init();			 //UART初始化
	Rest();	   				 //复位加密芯片
	Rece(17);				 //接收复位信息
	Delay();

	Send();					 //发送高速命令
	Rece(4);   				 //提速成功的话芯片返回的数据和发送命令相同

	Delay();
	TL1 = 0xFF;				 //设置57600kbps的初值
	TH1 = 0xFF;			
	PCON |= (1<<7);	 		// 波特率倍增来达到57600kbps
	
			   	
	Send_1();  				 //以57600kbps发送获取随机数命令
	Rece(11);				 //接收返回数据（84 + 8字节随机数 + 90 00）
	while(1);	 
}