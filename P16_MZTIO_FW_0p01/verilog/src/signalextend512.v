// signalextend512.v --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 四 7月 18 12:00:19 2019 (+0800)
// Last-Updated: 四 7月 18 12:53:32 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 2
// URL: http://wuhongyi.cn 

module signalextend512
  (
   din,
   dout,
   extend,
   clk
   );
   
   // parameter DATA_W = 8;
   
   input din;
   output dout;
   reg 	  dout;
   input [9:0] extend;
   
   input clk;
   // input rst_n;
   // wire ;                 
   // reg ;
   //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
   
   // assign  = ;//wire

   reg widthoutput_tmp1;
   reg widthoutput_tmp2;   
   always @(posedge clk) begin
      widthoutput_tmp1 <= din;
      widthoutput_tmp2 <= widthoutput_tmp1;
   end

   reg [9 :0]   cntwidth    ;
   wire 	add_cntwidth;
   wire 	end_cntwidth;
   reg 		add_flag;
   
   always @(posedge clk) begin
      if(add_cntwidth) begin
	 if(end_cntwidth)
	   cntwidth <= 0;
	 else
	   cntwidth <= cntwidth + 1;
      end
   end
   assign add_cntwidth = add_flag==1;//condition: add 1 
   assign end_cntwidth = add_cntwidth && cntwidth == extend-1; //End condition, last value
   
   always  @(posedge clk)begin
      if(widthoutput_tmp1 && !widthoutput_tmp2)begin
	 add_flag <= 1;
	 dout <= 1;
      end
      else if(end_cntwidth)begin
	 add_flag <= 0;
	 dout <= 0;
      end
   end   

   
   // --- --- ---
   
   // always 
   // always@(*)begin
      
   // end
   
   // --- --- ---
   
   // always@(posedge clk or negedge rst_n)begin
   //    if(rst_n==1'b0)begin
	 
   //    end
   //    else begin
	 
   //    end
   // end
   
endmodule


// 
// signalextend512.v ends here
