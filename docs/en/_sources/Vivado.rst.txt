.. Vivado.md --- 
.. 
.. Description: 
.. Author: Hongyi Wu(吴鸿毅)
.. Email: wuhongyi@qq.com 
.. Created: 一 5月 27 21:21:23 2019 (+0800)
.. Last-Updated: 六 1月 18 21:40:50 2020 (+0800)
..           By: Hongyi Wu(吴鸿毅)
..     Update #: 8
.. URL: http://wuhongyi.cn 

##################################################
Vivado
##################################################

============================================================
Install
============================================================

.. code-block:: bash

  tar   -zxvf   Xilinx_Vivado_SDK_2018.3_1207_2324.tar.gz
  cd    Xilinx_Vivado_SDK_2018.3_1207_2324
  ./xsetup


.. image:: img/Vivado_install0.png

Click "continue" to choose not to download the latest version, then click "Next" to go to the next step

.. image:: img/Vivado_install1.png

Click on the three optional boxes and then click "Next" to go to the next step

.. image:: img/Vivado_install2.png

Select "Vinado HL Design Edition" and click "Next" to go to the next step

.. image:: img/Vivado_install3.png

Click "Next" directly to enter the next step 

.. image:: img/Vivado_install4.png

Select the installation directory, here I choose to install to "/home/wuhongyi/Xilinx", and then click "Next" to enter the next step  

.. image:: img/Vivado_install5.png

Wait for the installation to complete

.. image:: img/Vivado_install6.png

**The following two steps are not necessary.**
	   
Copy the "vivadoLicence.lic" file to the installation directory, here is "/home/wuhongyi/Xilinx"

After the installation is complete, the following interface will pop up
	   
.. image:: img/Vivado_install7.png

Click on the "Load License" in the upper left and select our "vivadoLicence.lic" file

Then click "View License Status" in the upper left to view the authorized IP core	   

.. image:: img/Vivado_install8.png


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
