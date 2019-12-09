.. RemoteControl.md 
.. 
.. Description: 
.. Author: Hongyi Wu(吴鸿毅)
.. Email: wuhongyi@qq.com 
.. Created: 一 5月 27 21:23:17 2019 (+0800)
.. Last-Updated: 一 12月  9 11:14:16 2019 (+0800)
..           By: Hongyi Wu(吴鸿毅)
..     Update #: 9
.. URL: http://wuhongyi.cn 

##################################################
远程控制
##################################################

============================================================
minicom
============================================================

将 USB 线连接电脑，获取系统 IP

在 linux 中可以采用串口通讯软件 minicom

.. code:: bash
	  
   minicom -s


.. code:: bash
	  
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


- 选择 `Serial port setup` ，修改 Serial Device 为 `/dev/ttyUSB0` 。Bps/Par/Bits 修改为 `115200 8N1` ， 底端最后两个选项为 `NO`
- 选择 `Modem and dialing` ， 删除 A, B, K 条的内容
- 再然后，选择 `Save setup as dfl` 保存该修改设置
- 最后，选择 `Exit` 来退出配置模式，进入控制模式

.. code:: bash
	  
   user：root
   password: xia17pxn
    
   密码采用默认的，方便使用者都能登陆


假设该模块的 IP 地址为 222.29.111.80， 您可以通过以下命令远程登陆。

.. code:: bash
	  
   ssh -Y root@222.29.111.80


============================================================
静态 IP 设置
============================================================

因为 Ubuntu18.04 采用的是 netplan 来管理 network。所以可以在 /etc/netplan/ 目录下创建一个以 yaml 结尾的文件。比如 01-netplan.yaml 文件。 

然后在此文件下写入以下配置(你需要修改IP地址及网关)：

.. code:: yaml
	  
   network:
     version: 2
     renderer: networkd
     ethernets:
       enp3s0:
         dhcp4: no
         addresses: [192.168.1.110/24]
         gateway4:  192.168.1.1
         nameservers:
           addresses: [8.8.8.8, 114.114.114.114]


**特别要注意的是这里的每一行的空格一定要有的，否则会报错误而设置失败！**

.. code:: yaml
	  
   network:
     version: 2
     renderer: networkd
     ethernets:
       eth0:
         addresses: [10.10.6.33/24]
         gateway4: 10.10.6.10
         dhcp4: no 


以上参数为CIAE实验使用的配置。

最后使用 `sudo netplan apply` 来重启网络服务就可以了。使用 `ip  a` 查看你的静态IP是否设置成功了！

.. RemoteControl.md ends here 
