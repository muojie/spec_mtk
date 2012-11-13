#include "cos_fun.h"

//========================================
//test_fun2 功能是计算8字节的异或
//
//TxComm[1..8] = RxComm[2..9] XOR RxComm[10..17] 
//========================================
 void test_fun2(unsigned char xdata *RxComm,unsigned char xdata *TxComm)
 {
  unsigned char i;
  
  for(i=0;i<8;i++)
  {
   TxComm[i+1] = RxComm[i+2] ^  RxComm[i+10];
  }

   TxComm[0] = 8; //输出长度为8

 }


