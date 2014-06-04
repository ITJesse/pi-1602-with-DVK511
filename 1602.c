/***********************************
本程序用于树莓派通过GPIO连接1602的效
果演示，使用wiringPi库和其附带的LCD
库进行GPIO和1602的操作。

                 created by @qtsharp
************************************
增加IP显示，DS18B20温度，AP客户端数
量，客户端IP显示。
					added by ITJesse
************************************
示例1：
    $sudo ./l602
    屏幕显示：
        ------------------
        |  Raspberry Pi! |
        |51.9C 215/477MB |
        ------------------
        
示例2：
    $sudo ./1602 QtSharp
    屏幕显示：
        ------------------
        |QtSharp         |
        |51.9C 215/477MB |
        ------------------

示例3：
    $sudo ./1602 \ \ Hello\ World
    屏幕显示：
        ------------------
        |  hello world   |
        |51.9C 215/477MB |
        ------------------

************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <wiringPi.h>
#include <lcd.h>

const int RS = 3;	// 
const int EN = 14;	// 
const int D0 = 4;	// 
const int D1 = 12;	// 
const int D2 = 13;	// 
const int D3 = 6;	// 

int show_sys_info (int fd)
{
	FILE *fp;
    char temp_char[15]; //树莓派温度
    char Total[20]; //总内存量
    char Free[20]; //空闲内存量
	
	lcdPosition (fd, 0, 0); lcdPuts (fd, "System Info:");
	fp=fopen("/sys/class/thermal/thermal_zone0/temp","r"); //读取树莓派温度
	fgets(temp_char,9,fp);
	float Temp;
	Temp=atof(temp_char)/1000;
	lcdPosition (fd, 0, 1);lcdPrintf (fd, "%3.1fC", Temp) ; 
	fclose(fp);

	fp=fopen("/proc/meminfo","r"); //读取内存信息
	fgets(Total,19,fp);
	fgets(Total,4,fp);
	fgets(Free,9,fp);
	fgets(Free,19,fp);
	fgets(Free,4,fp);
	lcdPosition (fd, 7, 1);
	lcdPrintf (fd, "%3d/%3dMB", atoi(Free),atoi(Total)) ;
	fclose(fp);
}

int show_net_info (int fd)
{
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;
	char * s1="eth0";
	char buf[60];
	
	lcdPosition (fd, 0, 0); lcdPuts (fd, "Network Info:");
	
    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
			sprintf(buf, "%s", ifAddrStruct->ifa_name);
			if(strcmp(buf, s1) == 0){
				// is a valid IP4 Address
				tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
				char addressBuffer[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
				lcdPosition (fd, 0, 1);lcdPrintf (fd, "%s", addressBuffer) ; 
			}
        }
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    return 0;
}

int show_temp(fd){
	lcdPosition (fd, 0, 0); lcdPuts (fd, "Temperature:");
	int fdata = -1, ret;
	char *tmp1, tmp2[10], ch='t';
	char dev_name[100] = "/sys/devices/w1_bus_master1/28-000005e38995/w1_slave";
	long value;
	int integer, decimal;
	char buffer[100];
	int i,j;
	float temp;
	
	if ((fdata = open(dev_name, O_RDONLY)) < 0)
	{
		//perror("open error");
		lcdPosition (fd, 0, 1); lcdPrintf (fd, "open error");
		return 0;
	}

	ret = read(fdata, buffer, sizeof(buffer));
	if (ret < 0)
	{
		//perror("read error");
		lcdPosition (fd, 0, 1); lcdPrintf (fd, "read error");
		exit(1);
	}	


	tmp1 = strchr(buffer, ch);		
	sscanf(tmp1, "t=%s", tmp2);
	
	value = atoi(tmp2);	
	integer = value / 1000;
	decimal = value % 1000;
	
	//printf("temperature is %d.%d\n", integer, decimal);
	sprintf(buffer, "%d.%d", integer, decimal);
	temp = atof(buffer);
	lcdPosition (fd, 0, 1); lcdPrintf (fd, "%.1fC", temp);
	close(fdata);
	return 0;
}

int show_client_count(int fd)
{
	FILE *fp;
	fp = popen("hostapd_cli all_sta", "r");

	char mid;
	int i = 0;
	
	lcdPosition (fd, 0, 0); lcdPuts (fd, "Client Info:");

	while(!feof(fp))
	{
		mid = fgetc(fp);//从txt文本中读取一个字符赋值给mid
		if(mid == '\n')  //如果这个字符为换行符
			i++;                        
	}
	//printf("\n行數：\n",i);  
	lcdPosition (fd, 0, 1);lcdPrintf (fd, "Count: %d", (i-1)/8) ;	
	fclose(fp);

	return 0;
}

int show_client_info(int fd)
{
	FILE *fp;
	fp = fopen("/var/lib/misc/dnsmasq.leases","r");

	char buf[200];
	int i = 0;
	char * buf2;

	while(fgets(buf,sizeof(buf),fp) != NULL){
		//printf("%s\n", buf);
		cls(fd);

		buf2 = strtok(buf, " ");
		while(buf2 != NULL) 
		{
			//printf("%s\n", buf2);
			i++;
			if(i == 4)
			{
				lcdPosition (fd, 0, 0);lcdPrintf (fd, "%.16s", buf2);
				//printf("%s\n", s);
			}
			if(i == 3)
			{
				lcdPosition (fd, 0, 1);lcdPrintf (fd, "%.16s", buf2);
				//printf("%s\n", s);
			}
			buf2 = strtok(NULL, " ");
			//sleep(1);
		}
		i = 0;
		sleep(3);
	}
	fclose(fp);

	return 0;
}

int cls(int fd)
{
	lcdPosition (fd, 0, 0) ;
	lcdPuts (fd, "                ") ; //清空第一行
	lcdPosition (fd, 0, 1) ;
	lcdPuts (fd, "                ") ; //清空第二行
	return 0;
}

int main (int args, char *argv[])
{
    if (wiringPiSetup () == -1)
        exit (1) ;
    int fd = lcdInit (2, 16, 4, RS, EN, D0,D1,D2,D3,D0,D1,D2,D3) ;
    if (fd == -1)
    {
        printf ("lcdInit 1 failed\n") ;
        return 1 ;
    }
    sleep (1) ; //显示屏初始化
    
    lcdPosition (fd, 0, 0); lcdPuts (fd, "  Raspberry Pi!"); //启动信息
    sleep(1);
    
    if(argv[1])
    {
        lcdPosition (fd, 0, 0) ;
        lcdPuts (fd, "                ") ; //清空第一行
        lcdPosition (fd, 0, 0) ; lcdPuts (fd, argv[1]) ; //命令行参数显示至第一行
    } 
    
    while(1)
    {
		show_sys_info(fd);
		sleep(5);
		cls(fd);

		show_net_info(fd);
		sleep(5);
		cls(fd);
		
		show_client_count(fd);
		sleep(3);
		cls(fd);
		
		show_client_info(fd);
		cls(fd);
		
		show_temp(fd);
        sleep(5);
		cls(fd);
    }

    return 0;
    
}
