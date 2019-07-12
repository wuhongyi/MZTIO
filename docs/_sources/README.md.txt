<!-- README.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 四 12月 20 20:21:20 2018 (+0800)
;; Last-Updated: 六 6月 15 14:04:58 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 48
;; URL: http://wuhongyi.cn -->

# README

https://support.xia.com/default.asp?W372

The Pixie-16 MZ-TrigIO is designed to route signals from the backplane (rear connectors) to the front panel (front connectors) and make logical combinations between them in FPGA fabric. It has the following features and capabilities:
- Ethernet programmable trigger/coincidence control module for the Pixie-16
- 48+ Pixie-16 backplane trigger connections to local Zynq processor
- 48 front panel LVDS connections to local Zynq processor
- MicroZed Zynq processor with embedded Linux, acting as a standalone PC with built-in SD card drive, USB host, 10/100 Ethernet, webserver, etc
- 1588 PTP and SyncE clock synchronization
- Open source user access to software and firmware
- Use as standalone desktop unit or in 6U PXI chassis
- Custom I/O standards via daughtercards


----

## Safety

Please take a moment to review these safety precautions. They are provided both for your protection and to prevent damage to the Pixie module and connected equipment. This safety information applies to all operators and service personnel.

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


<!-- README.md ends here -->
