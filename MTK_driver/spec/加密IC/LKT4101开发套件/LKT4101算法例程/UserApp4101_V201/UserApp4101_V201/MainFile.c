typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;


extern void test_fun1(BYTE xdata *inbuf,BYTE xdata *outbuf); // DES Encrypt
extern void test_fun2(BYTE xdata *inbuf,BYTE xdata *outbuf); // XOR
extern void test_fun3(BYTE xdata *outbuf); // get random
extern void test_fun4(BYTE xdata *TxComm); 

//####################################################################
//编译连接后  data 区的大小不能大于 54  (0x0036)
//编译连接后 xdata 区的大小不能大于 640 (0x0280) 
//编译连接后  code 区的大小不能大于 48K (0xC000)
//####################################################################


//全局变量的定义示例：
BYTE xdata g_test_var1,g_test_var2;
WORD xdata g_test_Wvar;

//如果需要为全局变量初始化，则必须在调用算法的过程中来完成初始化
void GlobalVar_Init() 
{//将此函数放在算法调用的时候完成。
 g_test_var1 = 0x12;
 g_test_var1 = 0x34;
 g_test_Wvar = 0x7788;
}


// 注意：不要使用类似下列注释中的方法来完成全局变量的初始化
/*
BYTE xdata g_test_var1 = 0x12;
WORD xdata g_test_Wvar = 0x7788;
*/





//==========================================================
//Fun_UserAppAlg 用户算法入口
//用户的算法在此实现
//==========================================================
BYTE UserAppAlg(BYTE xdata *RxComm,BYTE xdata *TxComm)
{
 BYTE cmd;

 cmd = RxComm[1]; 

 if (cmd == 0x00) 
 {
  GlobalVar_Init(); //全局变量的初始化 
 }

 if (cmd == 0x01) 
 {
  test_fun1(RxComm  , TxComm);
 }
 
 if (cmd == 0x02) 
  {  
   test_fun2(RxComm,TxComm);
  }

 if (cmd == 0x03) 
 {
  test_fun3(TxComm);
 }

 if (cmd == 0x04) 
 {
  test_fun4(TxComm);
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
