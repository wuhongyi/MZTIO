<!-- RemoteControl.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:23:17 2019 (+0800)
;; Last-Updated: 六 7月 13 14:36:28 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 3
;; URL: http://wuhongyi.cn -->

# remote control

## minicom

将 USB 线连接电脑，获取系统 IP

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

- 进入 Serial port setup，修改 Serial Device 为 /dev/ttyUSB0。Bps/Par/Bits 采用默认的 115200 8N1，底部两个选项均为 NO
- 进入 Modem and dialing ，将A、B、K项内容删除
- 然后选择 Save setup as dfl 保存设置
- 最后选择 Exit 退出配置模式，进入控制模式

user：root
password: xia17pxn

密码采用默认的，方便使用者都能登陆

```
ssh -Y root@222.29.111.80
```

## 静态IP设置

因为 Ubuntu18.04 采用的是 netplan 来管理 network。所以可以在 /etc/netplan/ 目录下创建一个以 yaml 结尾的文件。比如 01-netplan.yaml 文件。 

然后在此文件下写入以下配置：

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

**特别要注意的是这里的每一行的空格一定要有的，否则会报错误而设置失败！**


最后使用 `sudo netplan apply` 来重启网络服务就可以了。使用 `ip  a` 查看你的静态IP是否设置成功了！

<!-- RemoteControl.md ends here -->
