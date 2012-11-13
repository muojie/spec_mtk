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


  key[0]  = 8;
  for(i=0;i<8;i++)
  {
   key[i+1] = RxComm[i+2];
  }

  in[0] = RxComm[10]; // 待加密数据的长度

  for(i=0;i<in[0] ;i++)
  {
   in[i+1] = RxComm[i+11];
  }

  DESEncrypt(in,key,out);// in,key 两个输入参数都是LV格式,输出参数out也是LV格式.

  for(i=0;i<out[0];i++)
  {
   TxComm[i+1] = out[i+1]; //输出8字节DES加密的结果
  }

   TxComm[0]=out[0];  //输出长度为 out[0]
 }


//========================================
//test_fun3 功能是取16字节随机数
//
//========================================
void test_fun3(unsigned char xdata *TxComm) // get random
{
 GetRandomData( TxComm + 1, 16 );

 TxComm[0] = 16 ;

}
//========================================
//test_fun4 功能是获取8字节芯片ID号
//
//========================================
void test_fun4(unsigned char xdata *TxComm) // get 8 bytes chip ID
{
 GetChipID( TxComm + 1);
 
 TxComm[0] = 8 ;

}

