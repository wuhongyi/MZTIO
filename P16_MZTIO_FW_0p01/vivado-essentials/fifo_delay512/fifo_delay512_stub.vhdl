-- Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2018.3 (lin64) Build 2405991 Thu Dec  6 23:36:41 MST 2018
-- Date        : Tue Jul 16 21:39:38 2019
-- Host        : localhost.localdomain running 64-bit CentOS Linux release 7.6.1810 (Core)
-- Command     : write_vhdl -force -mode synth_stub -rename_top fifo_delay512 -prefix
--               fifo_delay512_ fifo_generator_512_stub.vhdl
-- Design      : fifo_generator_512
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7z020clg400-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity fifo_delay512 is
  Port ( 
    clk : in STD_LOGIC;
    srst : in STD_LOGIC;
    din : in STD_LOGIC_VECTOR ( 0 to 0 );
    wr_en : in STD_LOGIC;
    rd_en : in STD_LOGIC;
    dout : out STD_LOGIC_VECTOR ( 0 to 0 );
    full : out STD_LOGIC;
    empty : out STD_LOGIC;
    data_count : out STD_LOGIC_VECTOR ( 9 downto 0 )
  );

end fifo_delay512;

architecture stub of fifo_delay512 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "clk,srst,din[0:0],wr_en,rd_en,dout[0:0],full,empty,data_count[9:0]";
attribute x_core_info : string;
attribute x_core_info of stub : architecture is "fifo_generator_v13_2_3,Vivado 2018.3";
begin
end;
