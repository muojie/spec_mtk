/*
 ============================================================================
 Name        : i2c-tool.c
 Author      : zlchen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "keyboard.h"
//#include <syswait.h>

#define  CALIBRATION_FLAG	1
#define  NORMAL_MODE		8
#define  DEBUG_MODE			3
#define  INTERNAL_MODE		4
#define  RASTER_MODE		5
#define  VERSION_FLAG		6
#define  BOOTLOADER_MODE    7
#define  BUTTON_RAW         2
#define  REEPROM            12
#define  WEEPROM            13
#define	 RESET_TP			9
#define  ENABLE_IRQ         10
#define  DISABLE_IRQ        11

#define  BUFSIZE 			6
#define  RBUF               300
#define  WBUF               143
#define  VLEN               5

#define  CALI_TIME          5

static int gldev;

union address
{
	short int id;
	unsigned char cd[2];
};

int calibration(int sel)
{
	int i, arg = 0, ret;
	char buf[2];
/*	buf[0] = sel == 1 ? 0x37 : 0x78;*/
/*	buf[1] = 0x03;*/
	buf[0] = 0x3A;
	buf[1] = 0x03;
	ioctl(gldev, CALIBRATION_FLAG, arg);
	printf("please wait..");
	ret = write(gldev, buf, 2);
	for (i = 0; i < 3; i++)
	{
		sleep(1);
		printf("..");
		fflush(stdout);
	}
	printf("\n");

	if (ret == 2)
	{
		printf("Calibration successfully\n");
	}
	else
	{
		printf("Calibration faild\n");
	}
	return ret == 2 ? 0 : -1;
}

int normal_mode(void)
{
	int ret, arg = 0, pos[6];

	init_keyboard();
	ioctl(gldev, NORMAL_MODE, arg);
	while (1)
	{
		char buf[10] = "\0";
		if (kbhit())
		{
			char key = getkey();
			if (key == '\n') break;
		}

		ret = read(gldev, buf, 10);
		if (buf[0] != 0)
		{
			pos[0] = buf[0];
			pos[1] = buf[1];
			pos[2] = ((buf[3] << 8) | (buf[2]));
			pos[3] = ((buf[5] << 8) | (buf[4]));
			pos[4] = ((buf[7] << 8) | (buf[6]));
			pos[5] = ((buf[9] << 8) | (buf[8]));			

			printf("touching=%d oldtouching=%d x1=%-3d y1=%-3d x2=%-3d y2=%-3d\n", pos[0], pos[1],
					pos[2], pos[3], pos[4], pos[5]);

		}
	}

	close_keyboard();
	return ret;
}

int raster_mode(void)
{
	return 0;
}

void show_raw(char axis, int len, int start, short int raw[], char flag, int fptr)
{
	int i;
	char buf[1024] = "\0";
	sprintf(buf, "%c: ", axis);
	for (i = 0; i < len; i++)
	{
		char str[8];
		sprintf(str, "%4d ", raw[i + start]);
		strcat(buf, str);
	}
	strcat(buf, "\n");
	printf("%s", buf);

	if (flag && len>0 )
	{
		write(fptr, buf, strlen(buf));
	}
}

void tango1(int type, int mode, char flag)
{
	int buflen, rawlen;
	int i, ret, fptr = -1;		
	unsigned char xline = 0;
	unsigned char yline = 0;
	unsigned char buf[98];
	short int xyraw[48];
	
	if(type==3)
	{
		buflen = 98;
		rawlen = 48;
	}	
	else
	{
		buflen = 66;
		rawlen = 32;
	}
	if (flag)
	{
		char *fdat = "./rawdata.txt";
		fptr = open(fdat, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (fptr == -1)
		{
			puts("Open raw.dat failed.");
			return;
		}
	}

	init_keyboard();
	ioctl(gldev, DISABLE_IRQ, 0);
	ioctl(gldev, mode, 1);
	while (1)
	{
		if (kbhit())
		{
			char key = getkey();
			if (key == '\n')
			{
				ioctl(gldev, ENABLE_IRQ, 0);
				printf("break debug.\n");
				break;
			}
		}

		ret = read(gldev, buf, buflen);
		xline = buf[buflen-2];
		yline = buf[buflen-1];
		
/*        {			*/
/*			for(i = 0; i < buflen; i++)*/
/*			{				*/
/*				printf("%x ", buf[i]);*/
/*				if(i%50==0 && i>0) printf("\n");*/
/*			}*/
/*			printf("\n");*/
/*		}		*/

		for (i = 0; i < rawlen; i++)
		{
			xyraw[i] = ((buf[i * 2 + 1] << 8) | buf[i * 2]);
		}
		show_raw('X', xline-1, yline - 1, xyraw, flag, fptr);
		show_raw('Y', yline-1, 0, xyraw, flag, fptr);		
	}
	close_keyboard();
	if (flag)
	{
		close(fptr);
	}
}

void tango2(int type, int mode, char flag)
{
	int i, ret, len, fptr = -1;
	unsigned char xline = 0;
	unsigned char yline = 0;
	unsigned char xline2 = 0;
	unsigned char buf[131];ioctl(gldev, RESET_TP, 0);
	short int xyraw1[34], xyraw2[32], xy_tmp[66];

	if (flag)
	{
		char *fdat = "./rawdata.txt";
		fptr = open(fdat, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (fptr == -1)
		{
			puts("Open raw.dat failed.");
			return;
		}
	}

	init_keyboard();
	ioctl(gldev, DISABLE_IRQ, 0);
	ioctl(gldev, mode, 2);
	while (1)
	{
		if (kbhit())
		{
			char key = getkey();
			if (key == '\n')
			{
				ioctl(gldev, ENABLE_IRQ, 0);
				break;
			}
		}

		ret = read(gldev, buf, 131);
		xline = buf[64];
		yline = buf[65];
		xline2 = buf[66];    // printf("xline=%d yline=%d xline2=%d\n", xline, yline, xline2);
		for (i = 0; i < 32; i++)
		{
			xyraw1[i] = ((buf[i * 2 + 1] << 8) | buf[i * 2]);
			xyraw2[i] = ((buf[67 + i * 2 + 1] << 8) | buf[67 + i * 2]);
		}

		for (i = 0; i < xline - 1; i++)
		{
			xy_tmp[i] = xyraw2[i];
		}
		if (xline2)
		{
			for (i = 0; i < xline2 - 1; i++)
			{
				xy_tmp[xline - 1 + i] = xyraw1[i];
			}
			len = xline - 1 + xline2 - 1;
		}
		else
		{
			len = xline - 1;
		}
		show_raw('X', len, 0, xy_tmp, flag, fptr);	
		show_raw('Y', yline - 1, xline2, xyraw1, flag, fptr);
	}
	close_keyboard();
	if (flag)
	{
		close(fptr);
	}
}

void debug_and_internal(int type, int mode, char flag)
{
	switch( type )
	{
	case 1:	
	case 3:
		tango1(type, mode, flag);
		break;
	case 2:		
	case 4:		
	case 5:
		tango2(type, mode, flag);
		break;
	}	
}

void debug_mode(int type, char flag)
{
	debug_and_internal(type, DEBUG_MODE, flag);
}

void internal_mode(int type, char flag)
{	
	debug_and_internal(type, INTERNAL_MODE, flag);
}

unsigned char hex2char(char c)
{
	return c < 'A'? c - '0': c - 'A' + 10;
}

void msleep(int ms)
{
	while(ms--)	usleep(1000);
}

int bootloader_mode(char *fname)
{
	int i, ret, len, arg = 0;
	FILE *fp;
	char wbuf[WBUF];	
	unsigned char flag = 1;
	unsigned char reset = 0;

	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		puts("Open file error");
		return -1;
	}
	ioctl(gldev, DISABLE_IRQ, 0);
	ioctl(gldev, BOOTLOADER_MODE, arg);	
	while (1)
	{
		char rbuf[RBUF];
		fgets(rbuf, RBUF, fp);
		if (feof(fp)) break;		
		if( !strncmp(rbuf, "!delay", 6)) 
		{	
			printf("%s", rbuf);		
			msleep(300);
			continue;
		}
		else if ( rbuf[0]!='$') continue;
		else if( !strncmp(rbuf, "$00", 3))
		{
			flag = 0;
		}
		if( !strncmp(rbuf, "$03", 3) && flag )
		{
			reset = 1;
			flag = 0;
			for (i = 0; i < WBUF; i++)
			{
				wbuf[i] = 0;
			}
			
			ret = write(gldev, wbuf, WBUF);
			msleep(150);
			if (ret != WBUF)
			{
				ioctl(gldev, ENABLE_IRQ, 0);
				fclose(fp);
				puts("Download failed.");
				return 1;
			}
		}
		
		for (i = 0; i < WBUF; i++)
		{
			wbuf[i] = (hex2char(rbuf[(i + 1) * 2 - 1]) << 4) | hex2char(rbuf[(i + 1) * 2]);
		}
		printf("%s", rbuf);		
		ret = write(gldev, wbuf, WBUF);
		msleep(150); 
		if (ret != WBUF)
		{
			ioctl(gldev, ENABLE_IRQ, 0);
			fclose(fp);
			puts("Download failed.");
			return 1;
		}
	}

	ioctl(gldev, ENABLE_IRQ, 0);
	fclose(fp);
	puts("Download successed.");

	if(ret==WBUF && reset )
	{
		ioctl(gldev, RESET_TP, 0);
		sleep(1);
		calibration(1);
	}

	return 1;
}

int get_version()
{
	int i, ret, arg = 0;
	unsigned char buf[VLEN];
	ioctl(gldev, VERSION_FLAG, arg);
	ret = read(gldev, buf, VLEN);
	printf("Version: ");
	for (i = 0; i < VLEN-1; i++)
	{
		printf("0x%x ", buf[i]);
	}
	printf("\n");
	printf("SubVersion: 0x%x\n", buf[VLEN-1]);
		
	return ret;
}

void input_fname(char *fname)
{
	printf("Input fname: ");
	scanf("%s", fname);
	getchar();
}

void input_debug_param(int *type, char *flag)
{
input_debug_lable:
	puts("Select solutions<1~5>:");
	puts("1> Atmel168_1TG");
	puts("2> Atmel168_2TG");
	puts("3> M48");
	puts("4> M48_F32");
	puts("5> R8C_3GA_2TG");
	
	printf("Input num: ");	
	scanf("%d", type);
	while(getchar()!='\n');
	if(*type<0 || *type>5 ) 
	{
		puts("Input invalid data.");
		goto input_debug_lable;
	}

	printf("Do you save the data?(Y/N):");
	while(1)
	{
		*flag = getchar();
		if(*flag=='y'||*flag=='Y'||*flag=='n'||*flag=='N') break;
	}
	while(getchar()!='\n');
}

void main_menu()
{
	puts("Press a key in brackes.");
	puts("[C] Calibration.");
//	puts("[N] Normal mode.");
	//puts("[R] Raster mode.");
//	puts("[D] Debug mode.");
	puts("[B] Bootloader mode.");
/*	puts("[I] Internal mode.");*/
	puts("[T] Touch Panel reset.");	
/*	puts("[R] Read EEPROM.");*/
/*	puts("[W] Write EEPROM.");*/
/*	puts("[V] Version.");*/
	puts("[M] Menu show.");
	puts("[X] Exit.");
}

int select_calibrate(char cmd)
{
	int ret = 1;
	if (cmd == 'A' || cmd == 'a')
	{
		ret = 2;
	}
	return ret;
}

int input_calibrate_menu()
{
	char cmd='p';
	puts("[P] PIXCIR Calibrate.");
	puts("[A] AUO Calibrate."); 
	printf("Input cmd: ");
	while( 1 )
	{
		cmd = getchar();
	    if( cmd=='a' || cmd=='A' || cmd=='p' || cmd=='P' )
	    {		       
	      	break;    
	   	}
	}
	while (getchar()!='\n');	
	return select_calibrate(cmd);
}

int button_raw()
{
	int i, ret;
	unsigned char buf[8];
	ioctl(gldev, BUTTON_RAW, 0);
	ret = read(gldev, buf, 8);
	printf("Button raw data: ");
	for (i = 0; i < 8; i++)
	{
		printf("%c", buf[i]);
	}
	printf("\n");

	return ret;
}

int read_eeprom()
{
	int i, ret, bytes = 0;
	char *p;
	int addr;
	union address add;

	ioctl(gldev, REEPROM, 0);
	printf("Input address: ");
	scanf("%d", &addr);
	while(getchar()!='\n');

	printf("read bytes number: ");
	scanf("%d", &bytes);
	while(getchar()!='\n');
	if (bytes <= 0)
	{
		puts("input data error.");
		return 0;
	}

	p = (char *) malloc(bytes * sizeof(char));
	if (!p)
	{
		puts("malloc failed.");
		return 0;
	}
	add.id = (short int) addr;

	ret = write(gldev, add.cd, sizeof(short int));
	if (ret == -1)
	{
		puts("write address failed.");
		return 0;
	}

	ret = read(gldev, p, bytes);
	if (ret == -1)
	{
		puts("read data failed.");
		return 0;
	}
	printf("data: ");
	for (i = 0; i < bytes; i++)
	{
		printf("%d ", p[i]);
		if (i % 10 && i > 0) printf("\n");
	}
	printf("\n");
	free(p);

	return ret;
}

int write_eeprom()
{
	int ret, data, addr;
	union address add;

	ioctl(gldev, WEEPROM, 0);
	printf("Input address: ");
	scanf("%d", &addr);
	while(getchar()!='\n');

	printf("write one byte: ");
	scanf("%d", &data);
	while(getchar()!='\n');

	add.id = (short int) addr;
	ret = write(gldev, add.cd, sizeof(short int));
	if (ret == -1)
	{
		puts("write address failed.");
		return 0;
	}

	unsigned char byte = (unsigned char) data;
	ret = write(gldev, &byte, 1);
	if (ret == -1)
	{
		puts("write data failed.");
		return 0;
	}
	else
	{
		puts("write data success.");
		return 1;
	}
}

void execute(void)
{
	int num, type = 1;
	char cmd, flag = 'n';
	char fname[64] = "/auto.pix";

	main_menu();
	while (1)
	{
 	    printf("Input cmd: ");
		cmd = getchar();
		while(getchar()!='\n');	
		switch (cmd)
		{
		case 't':
		case 'T':			
			ioctl(gldev, RESET_TP, 0);
			break;

		case 'c':
		case 'C':
			//num = input_calibrate_menu();					    
			//num = select_calibrate(cmd);	
			num = 1;		
			calibration(num);
			break;

/*		case 'n':*/
/*		case 'N':*/
/*			normal_mode();*/
/*			break;*/
			/*		case 'r':*/
			/*		case 'R':*/
			/*			raster_mode();*/
			/*			break;*/

/*		case 'd':*/
/*		case 'D':*/
/* 			input_debug_param(&type,&flag);*/
/*			debug_mode(type, flag);*/
/*			break;*/

		case 'b':
		case 'B':
			input_fname(fname);			
			bootloader_mode(fname);
			break;

/*		case 'v':*/
/*		case 'V':*/
/*			get_version();*/
/*			break;*/
/*		*/
/*		case 'i':*/
/*		case 'I':*/
/*		    input_debug_param(&type,&flag);*/
/*			internal_mode(type, flag);*/
/*		    break;*/

			/*		case 't':	*/
			/*		case 'T':*/
			/*		    button_raw();*/
			/*		    break;	*/

/*		case 'r':*/
/*		case 'R':*/
/*			read_eeprom();*/
/*			break;*/

/*		case 'w':*/
/*		case 'W':*/
/*			write_eeprom();*/
/*			break;*/

		case 'm':
		case 'M':
			main_menu();
			break;

		case 'x':
		case 'X':
			return;
		}			
	}
}

void perform(char *argv[])
{
	int num = 0;
	char cmd = argv[1][1];
	switch (cmd)
	{
	case 'c':
	case 'C':
		num = select_calibrate(argv[2][0]);		
		calibration(num);
		break;
	case 'n':
	case 'N':
		normal_mode();
		break;
		/*		case 'r':*/
		/*		case 'R':*/
		/*			raster_mode();*/
		/*			break;*/
/*	case 'd':*/
/*	case 'D':*/
/*		num = atoi(argv[2]);*/
/*		//debug_mode(num, DEBUG_MODE);*/
/*		break;*/

/*	case 'v':*/
/*	case 'V':*/
/*		get_version();*/
/*		break;*/
		/*	case 'i':*/
		/*	case 'I':*/
		/*	    num = atoi(argv[2]);*/
		/*		debug_mode(num, INTERNAL_MODE);*/
		/*		break;*/
/*	case 'b':*/
/*	case 'B':*/
/*		{*/
/*			char *fname = argv[2];*/
/*			bootloader_mode(fname);*/
/*		}*/
/*		break;*/
	}
}

int open_device()
{
	int i;
	char fname[32];
	for(i=0; i<256; i++)
	{
		sprintf(fname, "/dev/pixcir_i2c_ts%d", i);
		gldev = open(fname, O_RDWR, S_IRUSR | S_IWUSR);
		if(gldev!=-1) break;
	}
	if (gldev == -1)
	{
		puts("open failed.");
		return 0;
	}
	return 1;
}

int main(int argc, char* argv[])
{
	if( !open_device() ) return;
	
	argc == 1 ? execute() : perform(argv);

	close(gldev);

	puts("End.");
	return 0;
}

