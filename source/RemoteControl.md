<!-- RemoteControl.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:23:17 2019 (+0800)
;; Last-Updated: 日 7月 21 21:30:50 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 6
;; URL: http://wuhongyi.cn -->

# remote control

## minicom

Connect the USB cable to your computer to get the IP

将 USB 线连接电脑，获取系统 IP

Serial communication software(minicom) can be used in Linux OS

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

- Enter `Serial port setup`，modify Serial Device to `/dev/ttyUSB0`。Bps/Par/Bits change to `115200 8N1`, the bottom two options are `NO`
- Enter `Modem and dialing` , delete A, B, and K items
- Then select `Save setup as dfl` to save the settings 
- Finally, select `Exit` to exit the configuration mode and enter the control mode


```
user：root
password: xia17pxn

The password is the default, so users can log in.
密码采用默认的，方便使用者都能登陆
```

Assuming the IP address is 222.29.111.80, you can log in with the following command.

```bash
ssh -Y root@222.29.111.80
```

## static IP setting

Because Ubuntu 18.04 uses netplan to manage the network. So you can create a file ending in yaml in the /etc/netplan/ directory. For example, the 01-netplan.yaml file.

因为 Ubuntu18.04 采用的是 netplan 来管理 network。所以可以在 /etc/netplan/ 目录下创建一个以 yaml 结尾的文件。比如 01-netplan.yaml 文件。 

Then write the following configuration under this file(**You need to modify the IP address and gateway**):

然后在此文件下写入以下配置(你需要修改IP地址及网关)：

```yaml
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
```

**It is important to note that the spaces in each line must be there, otherwise the error will be reported and the setting will fail!**

**特别要注意的是这里的每一行的空格一定要有的，否则会报错误而设置失败！**

```yaml
network:
  version: 2
  renderer: networkd
  ethernets:
    eth0:
      addresses: [10.10.6.33/24]
      gateway4: 10.10.6.10
      dhcp4: no 
```


The above parameters are the configurations used by the CIAE experiment.

以上参数为CIAE实验使用的配置。

Finally, use `sudo netplan apply` to restart the network service. Use `ip a` to see if your static IP is set up successfully!

最后使用 `sudo netplan apply` 来重启网络服务就可以了。使用 `ip  a` 查看你的静态IP是否设置成功了！

<!-- RemoteControl.md ends here -->
