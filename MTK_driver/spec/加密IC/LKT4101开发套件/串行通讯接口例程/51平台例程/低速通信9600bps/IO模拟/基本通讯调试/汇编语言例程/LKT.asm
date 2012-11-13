/*单片机实现跟LKT加密芯片的通讯，先正确实现下面复位应答以及获取随机数，这样通讯就初步成功*/ 
/*函数功能为：（1） 实现LKT加密芯片的复位 （2）实现获取八字节的随机数*/
 
  	RSET	EQU		P2.2
	IO		EQU		P2.0
	flag	EQU		20H.0		   //区分复位信息和随机数信息的长度	:0为17字节的复位信息，1为取八字节随机数的复位信息（11个字节）
	buf 	EQU		30H			   //数据缓存区
	ORG 	0000H

	
	LCALL	SamPowerOn				//上电发送复位请求
	LCALL   putdata1  				//将00 84 00 00 08 获取随机数命令写入数据缓存区
	LCALL   ICCcommand				//发送命令并将LKT加密芯片返回的数据存入数据缓存区		
	JMP		ED

putdata1: 							// 将00 84 00 00 08 获取随机数命令写入数据缓存区
	 MOV	R0,#buf
	 CLR	A
	 MOV	buf,A
	 MOV	buf+1,#84H
	 MOV	buf+2,A
	 MOV	buf+3,A
	 MOV	buf+4,#8
	 RET	  
	  
SamPowerOn:						    //上电发送复位请求
	LCALL	DelayEtu
	SETB	IO					    //必须为高等待接收LKT加密芯片发送的起始位
	CLR		RSET				 
	LCALL	Delay
	SETB	RSET
	CLR     flag					
	MOV		R0,#buf

Readchar:							//读取LTK加密芯片的返回信息
	LCALL	ReceChar
	MOV		@R0,A
	INC		R0	
	JB		flag,Icc
	CJNE	R0,#buf+17,Readchar	    //复位信息是否结束
	RET
Icc:			
	CJNE	R0,#buf+11,Readchar		//随机数信息是否结束 
	RET		 

ReceChar:							//读一个字节函数 
	JB		IO,$                 		
	MOV		R7,#9				  
	CLR 	A
	CLR		C
ReceChar1:			
	NOP
	LCALL	DelayEtu
	MOV		C,IO 
	RRC		A
	DJNZ	R7,ReceChar1
	LCALL	DelayEtu
	LCALL	Delay		
	NOP
	NOP
	MOV		C,IO 				    //接收偶校验位	 CY中的数据要与PSW.0相同
	JC		Rececharodd	
	JNB		PSW.0,Rececharok

Rececharerr:						//出错返回	   
	RET		
Rececharodd: 
	JNB		PSW.0,Rececharerr	 
	
Rececharok:							 //接受一个字节成功 等待两个停止位
	LCALL	DelayEtu
	LCALL	DelayEtu
	RET	 	 
 
SendChar:							 //发送一个字节
 	MOV		R7,#9
	MOV		C,PSW.0	   				 //将PSW.0保存到CY中 （偶校验位）
	CLR		IO						 //发送起始位
SendChar1:
	RRC		A
	LCALL	DelayEtu
	LCALL   Delay
   	MOV		IO,C					 
	DJNZ	R7,SendChar1

	NOP
	NOP
	LCALL	DelayEtu
	SETB	IO						 //发送停止位
	LCALL	DelayEtu
	LCALL	DelayEtu
	LCALL	Delay
	RET

ICCcommand:							 //发送命令信息
   	
	MOV		A,@R0 
	LCALL	SendChar
	INC		R0
	CJNE	R0,#buf+5,icccommand	
	MOV		R0,#buf
	SETB    flag					 //接收11个字节的随机数信息
	LCALL	Readchar		
	RET   
	           	                                                                                                                               
DelayEtu:							 //发送接收一位的时间 etu=372/3.579 = 104us
	MOV		R1,#42					 //因发送接收其它指令也要占用时间这里的	DelayEtu = 90us
	DJNZ	R1,$
	RET

Delay:								 //用于微调发送接收的时间
	MOV		R1,#4
	DJNZ	R1,$
	RET	  

ED:	   
	END	