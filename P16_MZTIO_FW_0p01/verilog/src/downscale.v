// downscale.v --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 四 7月 18 13:14:06 2019 (+0800)
// Last-Updated: 四 7月 18 13:31:55 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 1
// URL: http://wuhongyi.cn 

module downscale
  (
   din,
   dout,
   down,
   clk
   );
   
   parameter DATA_W = 16;
   input [DATA_W-1:0]  down;
   input din;
   output dout;
   reg 	  dout;
   
   input clk;
   // input rst_n;
   // wire ;                 
   // reg ;
   //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
   
   // assign  = ;//wire
   
   // --- --- ---

   reg tmp1;
   reg tmp2;   
   always @(posedge clk) begin
      tmp1 <= din;
      tmp2 <= tmp1;
   end

   reg [DATA_W-1:0]   cnt    ;
   wire 	      add_cnt;
   wire 	      end_cnt;
   always @(posedge clk) begin
      if(add_cnt) begin
	 if(end_cnt)
	   cnt <= 0;
	 else
	   cnt <= cnt + 1;
      end
   end
   assign add_cnt = tmp1 && !tmp2;//condition: add 1 
   assign end_cnt = add_cnt && cnt >= down-1; //End condition, last value

   always  @(posedge clk)begin
      if(end_cnt)begin
	 dout <= 1;
      end
      else begin
	 dout <= 0;
      end
   end  

   
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
// downscale.v ends here
