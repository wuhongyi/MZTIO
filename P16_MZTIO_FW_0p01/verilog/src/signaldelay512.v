// signaldelay512.v --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 四 7月 18 11:42:52 2019 (+0800)
// Last-Updated: 四 7月 18 11:49:15 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 1
// URL: http://wuhongyi.cn 

module signaldelay512
  (
   din,
   dout,
   delay,
   clk
   );
   
   // parameter DATA_W = 8;
   output dout;
   reg    dout;

   input [9:0] delay;
   input       din;
   
   
   input clk;
   // input rst_n;
   // wire ;                 
   // reg ;
   //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
   
   // assign  = ;//wire
   wire [9:0] count;
   wire       delay_we_en;
   wire       delay_rd_en;
   wire       delayoutput_tmp;
   wire       empty;
   wire       full;

   assign delay_rd_en = (count>0 && count >= delay);//(~empty && delay >= delayandwidth1[9:0]);  // empty threshold 4
   assign delay_we_en = (~full && count <= delay);
   
   always @(posedge clk) begin
	 if(delay_rd_en)
	   dout <= delayoutput_tmp;
   end

   fifo_delay512 signaldealy
     (
      .clk(clk), 
      .srst(0),
      .din(din),
      .wr_en(delay_we_en),
      .rd_en(delay_rd_en),
      .dout(delayoutput_tmp),
      .full(full),
      .empty(empty), 
      .data_count(count)
      ); 

  
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
// signaldelay512.v ends here
