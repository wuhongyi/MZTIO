// testbench.v --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 六 7月 13 17:15:49 2019 (+0800)
// Last-Updated: 六 7月 13 17:19:04 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 1
// URL: http://wuhongyi.cn 

`timescale 1 ns/1 ns

module testbench();
   
   reg clk  ;
   reg rst_n;
   
   // 模块输入变量类型定义为reg，输出变量类型定义为wire
   reg [3:0] din0  ;
   reg       din1  ;
   
   wire      dout0;
   wire [4:0] dout1;
   
   // module_name#(.PAR(10)) 指定参数
   module_name uut(
		   .clk          (clk     ), 
		   .rst_n        (rst_n   ),
		   .din0         (din0    ),
		   .din1         (din1    ),
		   .dout0        (dout0   ),
		   .dout1        (dout1   )
		   );
   
   
   parameter CYCLE    = 20;
   parameter RST_TIME = 3 ;
   // integer i;//for loop	      
   
   
   initial begin
      clk = 1;
      forever
	#(CYCLE/2)
	clk=~clk;
   end
   
   initial begin
      rst_n = 1;
      #2;
      rst_n = 0;
      #(CYCLE*RST_TIME);
      rst_n = 1;
   end
   
   initial begin
      #1;
      din0 = 0;
      #(10*CYCLE);
      
   end
   
   
   // for(i=0;i<200;i=i+1)
   //   begin
   //      din = $random();
   //      #(1*CYCLE);
   //   end
   
   
endmodule



// 
// testbench.v ends here
