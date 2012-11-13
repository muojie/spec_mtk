#include "cos_fun.h"

//========================================
//test_fun2 功能是写数据到NVM

//========================================
 void test_fun2(unsigned char xdata *RxComm,unsigned char xdata *TxComm)
 {
 
    WriteNVM(RxComm[2]*0x100 + RxComm[3],&RxComm[5],RxComm[4]); // 存储区写:RxComm[2]RxComm[3]表示偏移地址，RxComm[4]写入数据的长度，RxComm[5].。。。之后是写入的数据
    TxComm[0] = 0;
	
 }


//========================================
//test_fun3 功能是读取NVM数据
（注意：实际应用中，可以内部程序使用，不要使用这个读取函数设置外部读取NVM区的命令，否则NVM区存储的数据就会被外界读出。在例子中删除它，这样写入的密钥就无法读出，保护密钥的安全。
//
//========================================
void test_fun3(unsigned char xdata *RxComm,unsigned char xdata *TxComm) 
{

   ReadNVM(RxComm[2]*0x100 + RxComm[3],&TxComm[1],RxComm[4]); // 存储区读 其中RxComm[2]RxComm[3]是读数据的起始地址; TxComm[1]是读出的数据；RxComm[4]存放需要读取的数据长度
   TxComm[0] = RxComm[4];                                       //
   
 }


