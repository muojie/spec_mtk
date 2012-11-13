/*单片机实现跟LKT加密芯片的通讯，先正确实现下面复位应答以及获取随机数，这样通讯就初步成功*/

/*函数功能为：（1） 实现LKT加密芯片的复位 （2）实现获取八字节的随机数*/

#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int
sbit	REST = P2^2;
sbit	IO   = P2^0;

uchar	buf[25];										//数据缓存区
uchar	SendBuf[5]={0x00,0x84,0x00,0x00,0x10} ;			//获取八字节随机数命令

/*****************用于微调发送接收的时间**************/
void Delay()				 		
{
	uchar	i;
	for(i=0;i<3;i++);
}	
	
/*********************发送接收一位的时间 etu=372/3.579 = 104us*************
***************因发送接收其它指令也要占用时间这里的	DelayEtu = 90us********/
void Delay_100us()					
{									
	uchar   i;
	for(i=0;i<28;i++);
}

/*****************上电发送复位请求******************/
void Rest()							
{
	Delay() ;
	IO   = 1;			    		//必须为高等待接收LKT加密芯片发送的起始位
	REST = 0;
	Delay_100us();
	REST = 1; 
} 

/****************读取LTK4101的返回信息并将其存入数据缓存区****************/
void ReceArt(char m)	   		   
{
	uchar i,n;
	
	for(n=0;n<m;n++)
	{	
		while(IO);
		for(i=0;i<8;i++)   		   //读一个字节函数 
		{
			Delay_100us();
			buf[n] >>= 1;
			if(1 == IO)
				buf[n] |= 0x80;
		}
		Delay_100us();
		ACC = buf[n];							   
		if(IO == P)				    //判断偶校验位
		{
		   Delay_100us();
		   Delay_100us();
		}  	
	}

}

/****************发送随机数命令信息***************/

void SendChar()
{
    uchar i,n,flag;
	
	for(n = 0 ;n<5 ; n++)
	{	
		ACC  = SendBuf[n] ;
		flag = P; 
		IO  = 0;
		for(i=0;i<8;i++)			   //发送一个字节
		{
			Delay_100us();
			if(SendBuf[n] & 0x01)
				IO = 1;
			else
				IO = 0;
			SendBuf[n] >>=1;
		}
		Delay_100us();
		IO = flag;					   //发送保存的奇偶位
		Delay_100us();	
		IO = 1;						   //发送停止位
		Delay_100us();	
		Delay_100us();   		
	}
}	 

void main()
{
	Rest();	
	ReceArt(17); 	 	   //接收复位信息
	Delay_100us();
	SendChar()	;
	ReceArt(19);		  //接收随机数信息
	P0 = buf[17];
	while(1);
}
