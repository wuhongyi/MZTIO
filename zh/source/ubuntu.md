<!-- ubuntu.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:25:03 2019 (+0800)
;; Last-Updated: 二 9月 24 20:52:07 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 4
;; URL: http://wuhongyi.cn -->

# ubuntu

## basic configuration

### ubuntu 18

如果操作系统是当前最新版本，则不需要进行额外的源配置。

如果要安装 CERN ROOT，则在 /etc/apt/sources.list 中添加以下行

```
deb http://ports.ubuntu.com/ xenial main universe multiverse
```

### ubuntu 12

如果操作系统版本是之前的老版本，则需要按照以下进行源的修改配置。

编辑源列表文件

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

ubuntu 颜色配置，个人目录下放置颜色配置文件 .dircolors，该文件在 readhat 系统中文件名为 .dir_colors

----

## 恢复SD卡原始空间

为了加快镜像装载速度，实际上只格式化了8/16G左右的SD卡空间，我16/32G的SD卡还有8/16G多的空间都没用到，为了能够进行使用进行如下操作

```bash
fdisk /dev/mmcblk0
# 然后分别输入: d [ENTER],2 [ENTER],n[ENTER] [ENTER],[ENTER],[ENTER],[ENTER],w[ENTER]， 若中间出现问题详细参考Getting started with Xillinux for Zynq-7000 EPP ， 然后重启linux 开机后
```

```bash
# 执行以下命令
resize2fs /dev/mmcblk0p2

# 使用以下命令查看追加的结果
df -h
```


----

## 升级启动文件

要将 SD 卡启动分区挂载到 /mnt/sd 文件夹，请执行

```bash
mount /dev/mmcblk0p1 /mnt/sd
```

这在不删除 SD 卡的情况下更新启动文件很有用。在新的启动文件生效之前，必须重新启动 Pixie-16 MZ-TrigIO。

操作流程如下
- 在台式机上生成固件文件
- 复制文件到 SD 卡上的文件夹（/var/www）
- 挂载启动分区 /dev/mmcblk0p1 到 /mnt/sd（如果尚未创建 /mnt/sd，则创建该目录）
- 复制文件，例如 cp /var/www/xillydemo.bit /mnt/sd
- 重新启动或关机后再开机（重新启动）


```bash
scp xillydemo.bit root@222.29.111.157:~
```


## /dev/mmcblk0p1

```
boot.bin  devicetree.dtb  uImage  xillydemo.bit
```


<!-- ubuntu.md ends here -->