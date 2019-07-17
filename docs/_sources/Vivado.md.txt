<!-- Vivado.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 一 5月 27 21:21:23 2019 (+0800)
;; Last-Updated: 六 7月 13 17:01:30 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 2
;; URL: http://wuhongyi.cn -->

# Vivado

首次打开时，需要清空 ```P16_MZTIO_FW_0p01/build``` 文件夹

- Open Vivado. Use Tools > Run Tcl Script to run project generating script …/verilog/xillydemo-vivado.tcl. The resulting project file is in ...\verilog\vivado
There have been cases where the script crashes Vivado, and then the compile has ~100 pin property critical warnings. In such cases, start over.  
- Compile demo project (generate bitstream). Ignore warnings and critical warnings.
- Check build/xillydemo.runs/impl_1/xillydemo.bit 




<!-- Vivado.md ends here -->
