.. RemoteControl.md 
.. 
.. Description: 
.. Author: Hongyi Wu(吴鸿毅)
.. Email: wuhongyi@qq.com 
.. Created: 一 5月 27 21:23:17 2019 (+0800)
.. Last-Updated: 一 12月  9 11:11:04 2019 (+0800)
..           By: Hongyi Wu(吴鸿毅)
..     Update #: 9
.. URL: http://wuhongyi.cn 

##################################################
remote control
##################################################

============================================================
minicom
============================================================

Connect the USB cable to your computer to get the IP

Serial communication software(minicom) can be used in Linux OS


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


- Enter `Serial port setup` ，modify Serial Device to `/dev/ttyUSB0` 。Bps/Par/Bits change to `115200 8N1`, the bottom two options are `NO`
- Enter `Modem and dialing` , delete A, B, and K items
- Then select `Save setup as dfl` to save the settings 
- Finally, select `Exit` to exit the configuration mode and enter the control mode


.. code:: bash
	  
   user：root
   password: xia17pxn
    
   The password is the default, so users can log in.


Assuming the IP address is 222.29.111.80, you can log in with the following command.

.. code:: bash
	  
   ssh -Y root@222.29.111.80


============================================================
static IP setting
============================================================


Because Ubuntu 18.04 uses netplan to manage the network. So you can create a file ending in yaml in the /etc/netplan/ directory. For example, the 01-netplan.yaml file.

Then write the following configuration under this file(**You need to modify the IP address and gateway**):

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


**It is important to note that the spaces in each line must be there, otherwise the error will be reported and the setting will fail!**

.. code:: yaml
	  
   network:
     version: 2
     renderer: networkd
     ethernets:
       eth0:
         addresses: [10.10.6.33/24]
         gateway4: 10.10.6.10
         dhcp4: no 


The above parameters are the configurations used by the CIAE experiment.

Finally, use `sudo netplan apply` to restart the network service. Use `ip a` to see if your static IP is set up successfully!

.. RemoteControl.md ends here 
