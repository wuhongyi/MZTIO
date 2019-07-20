<!-- ubuntu.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:25:03 2019 (+0800)
;; Last-Updated: 五 7月 19 18:17:55 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 2
;; URL: http://wuhongyi.cn -->

# ubuntu

## 基本配置

### ubuntu 18

如果操作系统是当前最新版本，则不需要进行额外的源配置。

如果要安装CERN ROOT，则在 /etc/apt/sources.list 中添加以下行

```
deb http://ports.ubuntu.com/ xenial main universe multiverse
```

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
```bash
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

```bash
apt-get install root-system-bin
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


## /dev/mmcblk0p1

```
boot.bin  devicetree.dtb  uImage  xillydemo.bit
```


<!-- ubuntu.md ends here -->
