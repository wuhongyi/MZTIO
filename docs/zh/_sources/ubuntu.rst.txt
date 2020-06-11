.. ubuntu.md --- 
.. 
.. Description: 
.. Author: Hongyi Wu(吴鸿毅)
.. Email: wuhongyi@qq.com 
.. Created: 一 5月 27 21:25:03 2019 (+0800)
.. Last-Updated: 二 6月  9 10:10:34 2020 (+0800)
..           By: Hongyi Wu(吴鸿毅)
..     Update #: 11
.. URL: http://wuhongyi.cn 

##################################################
ubuntu
##################################################

============================================================
基础配置
============================================================

----------------------------------------------------------------------
ubuntu 18
----------------------------------------------------------------------

如果操作系统是当前最新版本，则不需要进行额外的源配置。

使用国内镜像

.. code::
   
  deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic main universe multiverse
  deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-updates main universe multiverse


如果要安装 CERN ROOT，则在 /etc/apt/sources.list 中添加以下行

.. code:: 
	  
  deb http://ports.ubuntu.com/ xenial main universe multiverse


----------------------------------------------------------------------
ubuntu 12
----------------------------------------------------------------------

如果操作系统版本是之前的老版本，则需要按照以下进行源的修改配置。

编辑源列表文件

.. code:: bash

   vim /etc/apt/sources.list


修改为：

.. code:: bash
	  
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


----------------------------------------------------------------------
软件升级
----------------------------------------------------------------------

.. code:: bash
	  
   apt-get update



.. code:: bash
	  
   #install firefox
   apt-get install firefox
   # install emacs
   apt-get install emacs

   # ROOT dependent library
   apt-get install cmake libx11-dev libxpm-dev libxft-dev libxext-dev gfortran libssl-dev xlibmesa-glu-dev libglew1.5-dev libftgl-dev libmysqlclient-dev libfftw3-dev libcfitsio-dev graphviz-dev libavahi-compat-libdnssd-dev libxml2-dev libkrb5-dev libgsl0-dev libqt4-dev

   #install django
   apt install python3-pip
   pip3 install django==2.2
   
.. code:: bash
	  
   apt-get install root-system-bin


ubuntu 颜色配置，个人目录下放置颜色配置文件 .dircolors，该文件在 readhat 系统中文件名为 .dir_colors


----------------------------------------------------------------------
时区选择
----------------------------------------------------------------------

.. code:: bash
	  
   #先查看当前系统时间
   date -R   
   #查看结果显示的时区，如果与当地时区不一致，则可以通过以下方式进行修改

   tzselect
   # 下图中展示了中国用户如何修改成当地的时区，其它地区用户进行对应的选择即可
   cp /usr/share/zoneinfo/Asia/Shanghai  /etc/localtime

   #查看是否修改成功
   date -R  

.. image:: /_static/img/ubuntu_tzselect.png
   

----

============================================================
恢复SD卡原始空间
============================================================

为了加快镜像装载速度，实际上只格式化了8/16G左右的SD卡空间，我16/32G的SD卡还有8/16G多的空间都没用到，为了能够进行使用进行如下操作

.. code:: bash
	  
   fdisk /dev/mmcblk0
   # 然后分别输入: d [ENTER],2 [ENTER],n[ENTER] [ENTER],[ENTER],[ENTER],[ENTER],w[ENTER]， 若中间出现问题详细参考Getting started with Xillinux for Zynq-7000 EPP ， 然后重启linux 开机后


.. code:: bash
	  
   # 执行以下命令
   resize2fs /dev/mmcblk0p2

   # 使用以下命令查看追加的结果
   df -h


----

============================================================
升级启动文件
============================================================

要将 SD 卡启动分区挂载到 /mnt/sd 文件夹，请执行

.. code:: bash
	  
   mount /dev/mmcblk0p1 /mnt/sd


这在不删除 SD 卡的情况下更新启动文件很有用。在新的启动文件生效之前，必须重新启动 Pixie-16 MZ-TrigIO。

操作流程如下：

- 在台式机上生成固件文件
- 复制文件到 SD 卡上的文件夹（/var/www）
- 挂载启动分区 /dev/mmcblk0p1 到 /mnt/sd（如果尚未创建 /mnt/sd，则创建该目录）
- 复制文件，例如 cp /var/www/xillydemo.bit /mnt/sd
- 重新启动或关机后再开机（重新启动）


.. code:: bash
	  
   scp xillydemo.bit root@222.29.111.157:~


============================================================
/dev/mmcblk0p1
============================================================

.. code:: bash
	  
   boot.bin  devicetree.dtb  uImage  xillydemo.bit



.. ubuntu.md ends here 
