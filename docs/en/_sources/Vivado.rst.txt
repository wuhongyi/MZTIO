.. Vivado.md --- 
.. 
.. Description: 
.. Author: Hongyi Wu(吴鸿毅)
.. Email: wuhongyi@qq.com 
.. Created: 一 5月 27 21:21:23 2019 (+0800)
.. Last-Updated: 一 12月  9 10:28:24 2019 (+0800)
..           By: Hongyi Wu(吴鸿毅)
..     Update #: 6
.. URL: http://wuhongyi.cn 

##################################################
Vivado
##################################################

============================================================
Install
============================================================

aaa

============================================================
Compile
============================================================

When you open it for the first time, you need to clear the ``P16_MZTIO_FW_0p01/build`` folder.

- Open Vivado. Use Tools > Run Tcl Script to run project generating script …/verilog/xillydemo-vivado.tcl. The resulting project file is in ...\verilog\vivado
- There have been cases where the script crashes Vivado, and then the compile has ~100 pin property critical warnings. In such cases, start over.  
- Compile demo project (generate bitstream). Ignore warnings and critical warnings.
- Check build/xillydemo.runs/impl_1/xillydemo.bit 

============================================================
In system debug
============================================================

Is possible???


.. https://www.cnblogs.com/bayunaner/articles/9522618.html 




.. Vivado.md ends here 
