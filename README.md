<!-- README.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 四 12月 20 20:21:20 2018 (+0800)
;; Last-Updated: 一 1月 21 13:41:36 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 8
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
deb http://mirrors.aliyun.com/ubuntu/ trusty main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-security main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-proposed main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-backports main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-security main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-updates main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-proposed main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-backports main restricted universe multiverse

deb http://mirrors.ustc.edu.cn/ubuntu/ vivid main universe
deb-src http://mirrors.ustc.edu.cn/ubuntu/ vivid main universe
```

运行
```
apt-get update
```


```bash

apt-get install gcc-4.9-base:i386

```


```bash
# 安装emacs
apt install emacs
```







<!-- README.md ends here -->
