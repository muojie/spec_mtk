
/*********************单片机晶振为24MHZ    加密芯片使用3.6864MHZ *******************************/

#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int

sbit	REST = P2^2;			  //定义复位脚
sbit	IO   = P2^0;			  //定义数据脚


uchar	buf[25];								//数据缓存区
uchar	SendBuf[4]={0xFF,0x10,0x94,0x7B} ;		//	发送高速通讯命令，该命令的通讯速度为（fosc/64）
uchar	SendBuf_1[19]={0xA3,0x11,0x02,0xAA,0xAA,0xAA,0xAA,
						0xAA,0xAA,0xAA,0xAA,0xFF,0xFF,0xFF,0xFF,
						0xFF,0xFF,0xFF,0xFF};  

/*发送命令实现DES加密演示命令应返回 A3 08 5555555555555555  */


/*****************用于微调发送接收的时间**************/
void Delay_33us()				 		
{
	uchar	i;
	for(i=0;i<19;i++);
}	
	
/*********************发送接收一位的时间 etu=372/3.6864 = 100us*************
***************因发送接收其它指令也要占用时间这里的	DelayEtu = 90us********/
void Delay_90us()					
{									
	uchar   i;
	for(i=0;i<58;i++);
}

/*****************上电发送复位请求******************/
void Rest()							
{
	IO   = 1;			    		//必须为高等待接收LKT加密芯片发送的起始位
	REST = 0;
	Delay_90us();
	REST = 1; 
} 

/****************读取LTK4101的返回信息并将其存入数据缓存区****************/
void ReceArt(char m)	   		   
{
	uchar i,n,sec;
	
	for(n=0;n<m;n++)
	{	
		while(IO);
		Delay_33us();				 //该延时的目的是使IO口接收数据的时间在芯片返回每位数据的近似中间位置
		for(i=0;i<8;i++)   		    //读一个字节函数 该循环一次的时间大约100us  左右 
		{
			Delay_90us();
			for(sec=0;sec<3;sec++);	  //微调时间
			buf[n] >>= 1;
			if(1 == IO)
				buf[n] |= 0x80;
		}
		Delay_90us(); 
		ACC = buf[n];								   
		if(IO == P)				    //判断偶校验位
		{
		    Delay_90us();			//延时两个停止位
		  	Delay_90us();
		}  	
	}
}

/****************接收加密芯片在57600kbps下的返回信息并将其存入数据缓存区****************/
void ReceArt_1(char m)	   		   
{
	uchar i,n,sec;
	for(n=0;n<m;n++)
	{	
		while(IO);
		for(sec=0;sec<5;sec++);		 //该延时的目的是使IO口接收数据的时间在芯片返回每位数据的近似中间位置
		for(i=0;i<8;i++)   		     //读一个字节函数 该循环一次的时间大约17us 左右 
		{
			for(sec=0;sec<5;sec++);
			buf[n] >>= 1;
			if(1 == IO)
				buf[n] |= 0x80;
		}
		for(sec=0;sec<10;sec++); 
		ACC = buf[n];								   
		if(IO == P)				    //判断偶校验位
		{
			Delay_33us();		    //延时两个停止位
		}  	
	}
}

/****************发送随机数命令信息***************/

void SendChar(uchar m,uchar Send[])
{
    uchar i,n,flag,sec,SendChar;
	
	for(n = 0 ;n<m ; n++)
		{	 
			
			ACC  = Send[n] ;
			flag = P;
			SendChar = 	Send[n];	 		//将发送的字节放在SendChar		
			IO  = 0;	
			for(i=0;i<8;i++)			   //发送一个字节
			{
				Delay_90us();	
				for(sec=0;sec<5;sec++);	
				if(SendChar & 0x01)
					IO = 1;
				else
					IO = 0;
				SendChar >>=1;
			}
			Delay_90us();
			IO = flag;					   //发送保存的奇偶位
			Delay_90us();	
			IO = 1;						   //发送停止位
			Delay_90us();				   //延时两个停止位
			Delay_90us();	
		}	
}	 

void SendChar_1(uchar m,uchar Send[])
{
    uchar i,n,flag,sec,SendChar;
	
		for(n = 0 ;n<m ; n++)
		{	
			
			ACC  = Send[n] ;
			flag = P;  
			SendChar = 	Send[n];	 		//将发送的字节放在SendChar	 
		   	IO  = 0;
			for(i=0;i<8;i++)			   //发送一个字节
			{
				for(sec=0;sec<7;sec++);
				if(SendChar & 0x01)
					IO = 1;
				else
					IO = 0;
				SendChar >>=1;
			}
			for(sec=0;sec<9;sec++);
			IO = flag;					   //发送保存的奇偶位
			for(sec=0;sec<9;sec++);	
			IO = 1;						   //发送停止位
			for(sec=0;sec<11;sec++);		
		}
}
void main()
{
	Rest();							   //必须接收完或延时足够长（大于LKT加密芯片向MCU传送返回信息的时间）才能向LKT加密芯片发送数据
	ReceArt(17); 					  
	Delay_90us();					  

	SendChar(4,SendBuf)	;		 	  //将SendBuf的长度和首地址传给SendChar(uchar m,uchar Send[])
	ReceArt(4);						  //返回数据应为0xFF,0x10,0x94,0x7B

	SendChar_1(19,SendBuf_1)	;	  //将SendBuf_1的长度和首地址传给SendChar_1(uchar m,uchar Send[])
	ReceArt_1(10);					  //返回数据应为A3  08 5555555555555555
	while(1);
}