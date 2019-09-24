<!-- ubuntu.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:25:03 2019 (+0800)
;; Last-Updated: 二 9月 24 20:52:14 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 4
;; URL: http://wuhongyi.cn -->

# ubuntu

## basic configuration

### ubuntu 18

If the operating system is the latest version, no additional source configuration is required.

If you want to install CERN ROOT, add the following line to /etc/apt/sources.list

```
deb http://ports.ubuntu.com/ xenial main universe multiverse
```

### ubuntu 12

If the operating system version is the previous version, you need to modify the source configuration as follows.

Edit source list file

```
vim /etc/apt/sources.list
```

change into:
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

### software upgrade

```bash
apt-get update
```


```bash
#install firefox
apt-get install firefox
# install emacs
apt-get install emacs

# ROOT dependent library
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

Ubuntu color configuration, place the color configuration file .dircolors in the personal directory, the file name is .dir_colors in the readhat system.


----

## Restore SD card space

In order to speed up the installation speed of the image, only the SD card space of about 8/16G is actually formatted. The 16/32G SD card and the 8/16G space are not used. In order to be able to use, the following operations are performed.

```bash
fdisk /dev/mmcblk0
# Then enter: d [ENTER],2 [ENTER],n[ENTER] [ENTER],[ENTER],[ENTER],[ENTER],w[ENTER]. Then reboot the OS. If there is a problem, please refer to *Getting started with Xillinux for Zynq-7000 EPP* 
```

```bash
# Execute the following command
resize2fs /dev/mmcblk0p2

# Use the following command to view the result
df -h
```

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
- reboot or power cycle (reboot)


```bash
scp xillydemo.bit root@222.29.111.157:~
```


## /dev/mmcblk0p1

```
boot.bin  devicetree.dtb  uImage  xillydemo.bit
```


<!-- ubuntu.md ends here -->
