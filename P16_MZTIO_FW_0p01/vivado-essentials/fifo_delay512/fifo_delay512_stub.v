// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.3 (lin64) Build 2405991 Thu Dec  6 23:36:41 MST 2018
// Date        : Tue Jul 16 21:39:38 2019
// Host        : localhost.localdomain running 64-bit CentOS Linux release 7.6.1810 (Core)
// Command     : write_verilog -force -mode synth_stub -rename_top fifo_delay512 -prefix
//               fifo_delay512_ fifo_generator_512_stub.v
// Design      : fifo_generator_512
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "fifo_generator_v13_2_3,Vivado 2018.3" *)
module fifo_delay512(clk, srst, din, wr_en, rd_en, dout, full, empty, 
  data_count)
/* synthesis syn_black_box black_box_pad_pin="clk,srst,din[0:0],wr_en,rd_en,dout[0:0],full,empty,data_count[9:0]" */;
  input clk;
  input srst;
  input [0:0]din;
  input wr_en;
  input rd_en;
  output [0:0]dout;
  output full;
  output empty;
  output [9:0]data_count;
endmodule
