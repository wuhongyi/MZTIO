<!-- README.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 四 12月 20 20:21:20 2018 (+0800)
;; Last-Updated: 二 2月 26 22:59:30 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 39
;; URL: http://wuhongyi.cn -->

# README

https://support.xia.com/default.asp?W372

- Open Vivado. Use Tools > Run Tcl Script to run project generating script …/verilog/xillydemo-vivado.tcl. The resulting project file is in ...\verilog\vivado
There have been cases where the script crashes Vivado, and then the compile has ~100 pin property critical warnings. In such cases, start over.  
- Compile demo project (generate bitstream). Ignore warnings and critical warnings.
- Check build/xillydemo.runs/impl_1/xillydemo.bit 

将USB线连接电脑，获取系统 IP

在 linux 中可以采用串口通讯软件 minicom

```bash
minicom -s
```

```
+-----[configuration]------+
| Filenames and paths      |
| File transfer protocols  |
| Serial port setup        |
| Modem and dialing        |
| Screen and keyboard      |
| Save setup as dfl        |
| Save setup as..          |
| Exit                     |
| Exit from Minicom        |
+--------------------------+
```

- 进入 Serial port setup，修改 Serial Device 为 /dev/ttyUSB0。Bps/Par/Bits 采用默认的 115200 8N1
- 进入 Modem and dialing ，将A、B、K项内容删除
- 然后选择 Save setup as dfl 保存设置
- 最后选择 Exit 退出配置模式，进入控制模式

user：root
password: xia17pxn

密码采用默认的，方便使用者都能登陆

```
ssh -Y root@222.29.111.232
```

## 基本配置

### ubuntu 18

如果操作系统是当前最新版本，则不需要进行额外的源配置。


### ubuntu 12

如果操作系统版本是之前的老版本，则需要按照以下进行源的修改配置。

编辑源列表文件
```
vim /etc/apt/sources.list
```

修改为
```
deb http://old-releases.ubuntu.com/ubuntu vivid main restricted universe multiverse   
deb http://old-releases.ubuntu.com/ubuntu vivid-security main restricted universe multiverse   
deb http://old-releases.ubuntu.com/ubuntu vivid-updates main restricted universe multiverse   
deb http://old-releases.ubuntu.com/ubuntu vivid-proposed main restricted universe multiverse   
deb http://old-releases.ubuntu.com/ubuntu vivid-backports main restricted universe multiverse   
deb-src http://old-releases.ubuntu.com/ubuntu vivid main restricted universe multiverse   
deb-src http://old-releases.ubuntu.com/ubuntu vivid-security main restricted universe multiverse   
deb-src http://old-releases.ubuntu.com/ubuntu vivid-updates main restricted universe multiverse   
deb-src http://old-releases.ubuntu.com/ubuntu vivid-proposed main restricted universe multiverse   
deb-src http://old-releases.ubuntu.com/ubuntu vivid-backports main restricted universe multiverse 

deb http://mirrors.ustc.edu.cn/ubuntu/ vivid main universe
deb-src http://mirrors.ustc.edu.cn/ubuntu/ vivid main universe
```

### 软件升级

运行
```
apt-get update
```


```bash
#安装 firefox
apt-get install firefox
# 安装emacs
apt-get install emacs

# ROOT 依赖库文件
apt-get install cmake
apt-get install libx11-dev
apt-get install libxpm-dev
apt-get install libxft-dev 
apt-get install libxext-dev
apt-get install gfortran 
apt-get install libssl-dev 
apt-get install xlibmesa-glu-dev 
apt-get install libglew1.5-dev 
apt-get install libftgl-dev 
apt-get install libmysqlclient-dev 
apt-get install libfftw3-dev 
apt-get install libcfitsio-dev 
apt-get install graphviz-dev
apt-get install libavahi-compat-libdnssd-dev 
apt-get install libxml2-dev 
apt-get install libkrb5-dev 
apt-get install libgsl0-dev 
apt-get install libqt4-dev
```

ubuntu 颜色配置，个人目录下放置颜色配置文件 .dircolors，该文件在 readhat 系统中文件名为 .dir_colors


----

## 恢复SD卡原始空间

为了加快镜像装载速度，实际上只格式化了8/16G左右的SD卡空间，我16/32G的SD卡还有8/16G多的空间都没用到，为了能够进行使用进行如下操作

fdisk /dev/mmcblk0
然后分别输入: d [ENTER],2 [ENTER],n[ENTER] [ENTER],[ENTER],[ENTER],[ENTER],w[ENTER]， 若中间出现问题详细参考Getting started with Xillinux for Zynq-7000 EPP ， 然后重启linux 开机后

resize2fs /dev/mmcblk0p2
并使用

df -h
查看最后追加的结果

----

## update the boot files

To mount the SD card boot partition to a folder /mnt/sd, execute
```bash
mount /dev/mmcblk0p1 /mnt/sd
```
this is useful to update the boot files without removing the SD card. The Pixie-16 MZ-TrigIO has to be rebooted before the new boot files become effective.

So the precedure would be 
- generate FW files on a desktop PC
- copy to shared Linux folder on the SD card (/var/www)
- mount boot partition mount /dev/mmcblk0p1 /mnt/sd (create /mnt/sd if not already there) 
- copy files e.g. cp /var/www/xillydemo.bit /mnt/sd
- reboot or power cycle ( reboot)


```bash
scp xillydemo.bit root@222.29.111.157:~
```

----

## 程序说明

PixieNetCommon.c          
PixieNetCommon.h          
PixieNetConfig.cpp        
PixieNetConfig.h          
PixieNetDefs.h            

makefile            
cgistats.c                
clockprog.c         
monitordaq.c        
progfippi.c    
runstats.c     
writeI2C.c     

d3.v3.min.js        
defaults.ini        
settings.ini   

Xia_LLC_web_header.jpg    
dygraph-combined.js 
index.html          
plotly-latest.min.js
rspage.html    
webopspasswords 存放网页密码




<!-- README.md ends here -->
