<!-- README.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 四 12月 20 20:21:20 2018 (+0800)
;; Last-Updated: 二 7月 23 10:21:51 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 54
;; URL: http://wuhongyi.cn -->

# README

**If you need firmware, please contact Hongyi Wu(wuhongyi@qq.com)**

**如果你需要固件，请联系吴鸿毅(wuhongyi@qq.com)**


**We will gradually improve the Chinese part of the this manual.**

If you want to know how PKU uses MZTIO, please click on the link below: [PKUMZTIO](http://wuhongyi.cn/MZTIO/)

XIA SUPPORT: [XIA](https://support.xia.com/default.asp?W372) [Blog](http://support.xia.com/default.asp?W801)

----

The Pixie-16 MZ-TrigIO is designed to route signals from the backplane (rear connectors) to the front panel (front connectors) and make logical combinations between them in FPGA fabric. It has the following features and capabilities:
- Ethernet programmable trigger/coincidence control module for the Pixie-16
- 48+ Pixie-16 backplane trigger connections to local Zynq processor
- 48 front panel LVDS connections to local Zynq processor
- MicroZed Zynq processor with embedded Linux, acting as a standalone PC with built-in SD card drive, USB host, 10/100 Ethernet, webserver, etc
- 1588 PTP and SyncE clock synchronization
- Open source user access to software and firmware
- Use as standalone desktop unit or in 6U PXI chassis
- Custom I/O standards via daughtercards

Pixie-16 MZ-TrigIO 设计用于将信号从背板（后连接器）连接到前面板（前连接器），并在 FPGA 架构中实现逻辑组合。它具有以下功能和特性：
- 用于 Pixie-16 的以太网可编程触发/符合控制模块
- 48+ Pixie-16 背板触发连接到本地 Zynq 处理器
- 48 个前面板 LVDS 连接到本地 Zynq 处理器
- 带嵌入式 Linux 的 MicroZed Zynq 处理器，作为独立 PC，内置 SD 卡驱动器，USB 主机，10/100 以太网，网络服务器等
- 1588 PTP 和 SyncE 时钟同步
- 开源用户访问软件和固件
- 用作独立桌面设备或 6U PXI 机箱
- 通过子卡自定义 I/O 标准

----

## Safety

**Please take a moment to review these safety precautions. They are provided both for your protection and to prevent damage to the Pixie module and connected equipment. This safety information applies to all operators and service personnel.**

- Power Source
	- The Pixie-16 MZ-TrigIO module is powered through an AC/DC wall adapter or a PXI backplane. The default adapter has a variety of AC plug attachments for different localities.
	- Please remember to shut down the Linux OS before removing the power plug from the Pixie-16 MZ-TrigIO or powering down the PXI chassis.
- User Adjustments/Disassembly
	- To avoid personal injury, and/or damage, always disconnect power before accessing the module’s interior. There are a few jumpers related to clocking on the board that experienced users may want to use.
- Voltage Ratings
	- Signals on the inputs and outputs must not exceed ± 3.3V. Please review the pinout in the appendix before making any connections.
- Daughtercards
	- Daughtercards can be used as alternatives to front panel and rear inputs, which requires caution to avoid conflicts from FPGA outputs and standard connector inputs.
- Servicing and Cleaning
	- To avoid personal injury, and/or damage to the Pixie module or connected equipment, do not attempt to repair or clean the inside of these units.
- Linux Passwords
	- The Pixie-16 MZ-TrigIO Linux OS comes with default user IDs and passwords for 1) SSH login, 2) SMB file sharing, and 3) Web Operations as described below. Users should immediately change these passwords, especially when the Pixie-16 MZ-TrigIO is connected to external networks. Don’t let hackers take over your Pixie-16 MZ-TrigIO!
- Linux Backup
	- The Pixie-16 MZ-TrigIO Linux OS is stored on a removable SD card. SD cards’ file systems can become corrupted, which would crash the Linux system and make the Pixie-16 MZ-TrigIO unable to operate. Therefore periodic backup of the SD card is recommended, for example using Win32DiskImager. (Byte for byte copy is required).
	- Note that all Linux passwords are stored on the SD card.


**请花点时间查看这些安全预防措施。它们既可以保护您，也可以防止损坏 Pixie 模块和连接的设备。此安全信息适用于所有操作员和维修人员。**

- 电源
	- Pixie-16 MZ-TrigIO 模块通过 AC/DC 适配器或 PXI 背板供电。默认适配器具有适用于不同地区的各种 AC 插头附件。
	- 在从 Pixie-16 MZ-TrigIO 拔下电源插头或关闭 PXI 机箱电源之前，请记得关闭 Linux 操作系统。
- 用户调整/反汇编
	- 为避免人身伤害和/或损坏，在进入模块内部之前，请务必断开电源​​。有一些与有经验的用户可能想要使用的电路板上的时钟相关的跳线。
- 电压额定值
	- 输入和输出信号不得超过 ±3.3V。在进行任何连接之前，请查看附录中的引脚分配。
- 子卡
	- 子卡可用作前面板和背面输入的替代品，这需要小心避免 FPGA 输出和标准连接器输入的冲突。
- 维修和清洁
	- 为避免人身伤害和/或损坏 Pixie 模块或连接的设备，请勿尝试修理或清洁这些设备的内部。
- Linux密码
	- Pixie-16 MZ-TrigIO Linux 操作系统附带默认用户 ID 和密码，用于1）SSH登录，2）SMB文件共享，以及3）Web操作，如下所述。用户应立即更改这些密码，尤其是当 Pixie-16 MZ-TrigIO 连接到外部网络时。不要让黑客接管你的 Pixie-16 MZ-TrigIO！
- Linux备份
	- Pixie-16 MZ-TrigIO Linux OS 存储在可移动 SD 卡上。 SD 卡的文件系统可能会损坏，这会使 Linux 系统崩溃并使 Pixie-16 MZ-TrigIO 无法运行。因此，建议定期备份 SD 卡，例如使用 Win32DiskImager。（需要一个字节一个字节的复制）。
	- 请注意，所有 Linux 密码都存储在 SD 卡上。


----

## Logic programming


In order to meet the needs of medium and low energy experimental nuclear physics, we have developed the following basic functions.

- signal delay
- signal extend
- coincidence
- multiplicity
- scaler/counter
- down scale
- remote parameter adjustment
- ......

为了适应中低能实验核物理的需求，我们发展了以下基本功能：

- 信号延迟
- 信号展宽
- 符合
- 多重性选择
- scaler计数器
- down scale 分除
- 远程参数调节
- 。。。。。。



<!-- README.md ends here -->
