// scaler.v --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 四 7月 18 11:00:49 2019 (+0800)
// Last-Updated: 四 7月 18 11:11:28 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 1
// URL: http://wuhongyi.cn 

module scaler
  (
   din,
   dout ,
   endcount,
   clk
   );
   
   parameter DATA_W = 32;
   output[DATA_W-1:0]  dout;
   reg   [DATA_W-1:0]  dout;
   
   input din;
   input endcount;
   input clk;
   // input rst_n;
   // wire ;                 
   // reg ;

   reg [DATA_W-1:0] scaler_tmp;
   //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
   
   // assign  = ;//wire
   
   // --- --- ---

   reg 	sign_1b , sign_2b , sign_pos;
   always @(posedge clk) begin
      sign_1b <= din;
      sign_2b <= sign_1b; 
   end
   
   always @(posedge clk) begin
      if(sign_2b && !sign_1b) 
	sign_pos <= 1;
      else
	sign_pos <= 0;
   end

   always  @(posedge clk)begin
      if(sign_pos && endcount)begin
	 scaler_tmp <= 1;
      end
      else if(endcount)begin
	 scaler_tmp <= 0;
      end
      else if(sign_pos)begin
	 scaler_tmp <= scaler_tmp+1;
      end
   end // always  @ (posedge user_clk)
   
   always @(posedge clk) begin
      if(endcount) begin
	 dout <= scaler_tmp;
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
// scaler.v ends here
