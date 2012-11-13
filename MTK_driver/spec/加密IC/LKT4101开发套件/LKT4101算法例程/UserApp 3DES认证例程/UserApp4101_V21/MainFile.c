typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;


extern void test_fun1(BYTE xdata *inbuf,BYTE xdata *outbuf); // DES Encrypt
extern void test_fun2(BYTE xdata *inbuf,BYTE xdata *outbuf); //写密钥
extern void test_fun3(BYTE xdata *inbuf,BYTE xdata *outbuf); //读密钥


//####################################################################
//编译连接后  data 区的大小不能大于 54  (0x0036)
//编译连接后 xdata 区的大小不能大于 640 (0x0280) 
//编译连接后  code 区的大小不能大于 48K (0xC000)
//####################################################################


//==========================================================
//Fun_UserAppAlg 用户算法入口
//用户的算法在此实现
//==========================================================
BYTE UserAppAlg(BYTE xdata *RxComm,BYTE xdata *TxComm)
{
 BYTE cmd;

 cmd = RxComm[1]; 

 if (cmd == 0x01) 
 {
  test_fun1(RxComm  , TxComm);
 }
 
 if (cmd == 0x02) 
  {  
   test_fun2(RxComm, TxComm);
  }

 if (cmd == 0x03) 
 {
  test_fun3(RxComm,TxComm);
 }


 return 1;

}

//=====================以下main 函数不要修改===========================
void main()
{
 BYTE xdata inbuf[1],outbuf[1];
 UserAppAlg(inbuf,outbuf);
 for(;;);
}
//=====================================================================
