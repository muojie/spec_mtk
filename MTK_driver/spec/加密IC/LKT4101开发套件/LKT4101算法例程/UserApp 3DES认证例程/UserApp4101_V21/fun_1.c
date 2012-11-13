#include "cos_fun.h"


//========================================
// test_fun1 功能是计算DES加密
// RxComm[1]:
// RxComm[2..9]: 8字节密钥
// RxComm[10] : 待加密数据的长度
// RxComm[11....N]: 待加密数据
//========================================
void test_fun1(unsigned char xdata *RxComm,unsigned char xdata *TxComm)
 {
  unsigned char i;
  unsigned char xdata in[129];  //待加密的数据长度 不超过128字节
  unsigned char xdata out[128];
  unsigned char xdata key[9];
  unsigned char xdata *BUFF;

  key[0]  = 16;
  
  
  ReadNVM(0x0000,&BUFF[0],0x10);
  for(i=0;i<16;i++)
  {
   key[i+1] = BUFF[i];          
  }

  in[0] = RxComm[2]; // 待加密数据的长度

  for(i=0;i<in[0] ;i++)
  {
   in[i+1] = RxComm[i+3];
  }

  DESEncrypt(in,key,out);// in,key 两个输入参数都是LV格式,输出参数out也是LV格式.

  for(i=0;i<out[0];i++)
  {
   TxComm[i+1] = out[i+1]; //输出8字节DES加密的结果
  }

   TxComm[0]=out[0];  //输出长度为 out[0]
 }





