<!-- RemoteControl.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:23:17 2019 (+0800)
;; Last-Updated: 一 5月 27 21:24:33 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 1
;; URL: http://wuhongyi.cn -->

# remote control

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

<!-- RemoteControl.md ends here -->
