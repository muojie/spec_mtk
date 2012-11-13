#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int
sbit	REST = P2^2;
sbit	IO   = P2^0;

uchar	buf[25];										//数据缓存区
uchar	SendBuf_1[22]={0x80,0x08,0x00,0x00,0x11,0x02,0xAA,
						0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,
						0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff	};
				

uchar	SendBuf_2[5]={0x00,0xc0,0x00,0x00,0x08}	;

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
		SendChar = 	Send[n];	 
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
	uchar  temp;
	temp = SendBuf_1[1];			  //保存所发送第二字节
	Rest();
	ReceArt(17); 					  //必须接收完或延时足够长（大于LKT加密芯片向MCU传送返回信息的时间）才能向LKT加密芯片发送数据
	Delay_100us();	
	SendChar(5,SendBuf_1)	;		 //根据T=0协议先向LKT加密芯片发送前5字节，LKT加密芯片应返回第二字节
	ReceArt(1);	
	if(buf[0] == temp)				 //判断是否返回第二字节，返回正确发送剩下的字节
	{	
		
		SendChar(17,&SendBuf_1[5]);
		ReceArt(2);	
		  									
	}			  
	if(buf[0]==0x61 && buf[1] ==0x08 );	  //判断应答位是否正确
	SendChar(5,SendBuf_2) ;				  //应答正确发送读取数据信息
	ReceArt(10);	
	while(1);
}