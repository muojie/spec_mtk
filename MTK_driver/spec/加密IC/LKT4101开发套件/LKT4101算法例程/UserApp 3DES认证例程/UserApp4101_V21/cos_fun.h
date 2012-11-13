#ifndef COS_FUN_H_FILE
#define COS_FUN_H_FILE


extern void  ReadNVM (unsigned int addr,unsigned char xdata * buf,unsigned char len) ;

extern void  WriteNVM(unsigned int addr,unsigned char xdata * buf,unsigned char len) ;

extern void  GetRandomData(unsigned char xdata *buf,unsigned char len);

extern void  CardConfig(unsigned char item, unsigned int val);

extern void DESEncrypt( unsigned char  xdata *in, unsigned char xdata *key,unsigned char xdata *out);

extern void DESDecrypt( unsigned char  xdata *in, unsigned char xdata *key,unsigned char xdata *out);



#endif
