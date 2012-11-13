#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int
sbit	REST = P2^2;
sbit	IO   = P2^0;

uchar	buf[25];										//数据缓存区
uchar	SendBuf_1[20]={0xA3,0x12,0x01,0x01,0x02,0x03,0x04,0x05,
						0x06,0x07,0x08,0x08,0x11,0x22,0x33,0x44,
						0x55,0x66,0x77,0x88};


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

void SendChar(uchar m,uchar Send[])
{
    uchar i,n,flag,SendChar;
	
	for(n = 0 ;n<m ; n++)
	{	
		ACC  = Send[n] ;
		flag = P;
		SendChar = 	Send[n];	 		//将发送的字节放在SendChar
		IO  = 0;
		for(i=0;i<8;i++)			   //发送一个字节
		{
			Delay_100us();
			if(SendChar & 0x01)
				IO = 1;
			else
				IO = 0;
			SendChar >>=1;
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
	ReceArt(17); 						 //必须接收完或延时足够长（大于LKT加密芯片向MCU传送返回信息的时间）才能向LKT加密芯片发送数据
	Delay_100us();	
	SendChar(20,SendBuf_1)	;		 	//将SendBuf的长度和首地址传给SendChar(uchar m,uchar Send[])
	ReceArt(10);	
	while(1);
}
