/*********************单片机晶振为24MHZ    加密芯片使用3.6864MHZ *******************************/

#include<reg51.h>

#define	uchar  unsigned  char
#define	uint   unsigned  int
sbit	REST = P2^2;			   //定义复位脚
sbit	IO   = P2^0;			   //定义数据脚

uchar	buf[25];									//数据缓存区
uchar	SendBuf[4]={0xFF,0x10,0x94,0x7B} ;		 	//	发送高速通讯命令，该命令的通讯速度为（fosc/64） 
uchar	SendBuf_1[23]={0x80,0x08,0x00,0x00,0x12,0x01,0x01,0x02,
						0x03,0x04,0x05,0x06,0x07,0x08,0x08,0x11,
						0x22,0x33,0x44,0x55,0x66,0x77,0x88}	;//发送命令

uchar	SendBuf_2[5]={0x00,0xc0,0x00,0x00,0x08}	;	//发送获取数据命令

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

/****************接收加密芯片在9600kbps下的返回信息并将其存入数据缓存区****************/
void ReceArt(char m)	   		   
{
	uchar i,n,sec;
	
	for(n=0;n<m;n++)
	{	
		while(IO);
		Delay_33us();		 //该延时的目的是使IO口接收数据的时间在芯片返回每位数据的近似中间位置
		for(i=0;i<8;i++)     //读一个字节函数 该循环一次的时间大约100us  左右 
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
		for(sec=0;sec<5;sec++);		//该延时的目的是使IO口接收数据的时间在芯片返回每位数据的近似中间位置
		for(i=0;i<8;i++)   		    //读一个字节函数 该循环一次的时间大约17us 左右 
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
			Delay_33us();			//延时两个停止位
		}  	
	}
}
/****************发送加密芯片提速命令****************/

void SendChar(uchar m,uchar Send[])
{
    uchar i,n,flag,sec,SendChar;
	
	for(n = 0 ;n<m ; n++)
		{	 
			
			ACC  = Send[n] ;
			flag = P;
			SendChar = 	Send[n];	 		//将发送的字节放在SendChar		
			IO  = 0;	
			for(i=0;i<8;i++)			    //发送一个字节  该循环一次的时间大约100us 左右
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
			Delay_90us();			
			Delay_90us();				  //延时两个停止位
		}	
}	 

  /****************提速成57600之后发送获取随机数命令****************/
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
	uchar  temp;
	temp = SendBuf_1[1];				 //保存所发送第二字节
	Rest();
	ReceArt(17); 						//必须接收完或延时足够长（大于LKT加密芯片向MCU传送返回信息的时间）才能向LKT加密芯片发送数据
	Delay_90us();	

	SendChar(4,SendBuf)	;		 	     //将SendBuf的长度和首地址传给SendChar(uchar m,uchar Send[])
	ReceArt(4);							 //返回数据应为0xFF,0x10,0x94,0x7B 提速成功

	SendChar_1(5,SendBuf_1)	;			//根据T=0协议先向LKT加密芯片发送前5字节，LKT加密芯片应返回第二字节
	ReceArt_1(1);							
	if(buf[0] == temp)					//判断是否返回第二字节，返回正确发送剩下的字节
	{	
		
		SendChar_1(18,&SendBuf_1[5]);
		ReceArt_1(2);	
		  									
	}	
			  
	if(buf[0]==0x61 && buf[1] ==0x08 )	 //判断应答位是否正确
	{
		SendChar_1(5,SendBuf_2) ;		 //应答正确发送读取数据信息
		ReceArt_1(11);					 //返回数据应为84 6C5E94DCADD39F1D 9000
	}
	while(1);		
}
