pi-1602-with-DVK511
===================

本程序用于树莓派通过DVK511扩展板连接1602的效果演示，使用wiringPi库和其附带的LCD库进行GPIO和1602的操作。
默认显示系统温度，空闲内存，当前IP（方便SSH连接），DS18B20温度

自行编译需要安装wiringPi库：

    git clone git://git.drogon.net/wiringPi
    
    cd wiringPi 
    
    git pull origin 
    
    cd wiringPi 
    
    ./build 
	
需加载w1_gpio，w1_therm模块才能正常读取DS18B20的数据

	modprobe w1_gpio
	
	modprobe w1_therm
	
如需开机自动加载

	sudo nano /etc/modules
	
在末尾加上
	
	w1_gpio
	
	w1_therm
	
保存并重启

代码中需自行修改DS18B20序列号

	ls /sys/bus/w1/devices
	
其中28-00000xxxxxxx便是DS18B20的序列号

替换掉源代码13行中的序列号，否则无法正常读取温度

编译：

    gcc 1602.c -o 1602 -lwiringPi -lwiringPiDev

运行：

    sudo ./1602 hello

