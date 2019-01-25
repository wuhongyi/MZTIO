<!-- README.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 四 12月 20 20:21:20 2018 (+0800)
;; Last-Updated: 五 1月 25 21:28:08 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 13
;; URL: http://wuhongyi.cn -->

# README

https://support.xia.com/default.asp?W372

- Open Vivado. Use Tools > Run Tcl Script to run project generating script …/verilog/xillydemo-vivado.tcl. The resulting project file is in ...\verilog\vivado
There have been cases where the script crashes Vivado, and then the compile has ~100 pin property critical warnings. In such cases, start over.  
- Compile demo project (generate bitstream). Ignore warnings and critical warnings.
- Check ...\verilog\vivado\xillydemo.runs\impl_1\xillydemo.bit 


将USB线连接电脑，获取系统 IP

user：root
password: xia17pxn

密码采用默认的，方便使用者都能登陆

```
ssh -Y root@222.29.111.157
```

## 基本配置

原文件备份

```
cp /etc/apt/sources.list /etc/apt/sources.list.bak
```

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



运行
```
apt-get update
```




```bash
#安装 firefox
apt-get install firefox
# 安装emacs
apt install emacs
```


## 恢复SD卡原始空间

为了加快镜像装载速度，实际上只格式化了16G左右的SD卡空间，我32G的SD卡还有16G多的空间都没用到，为了能够进行使用进行如下操作

fdisk /dev/mmcblk0
然后分别输入: d [ENTER],2 [ENTER],n[ENTER] [ENTER],[ENTER],[ENTER],[ENTER],w[ENTER]， 若中间出现问题详细参考Getting started with Xillinux for Zynq-7000 EPP ， 然后重启linux 开机后

resize2fs /dev/mmcblk0p2
并使用

df -h
查看最后追加的结果




<!-- README.md ends here -->
