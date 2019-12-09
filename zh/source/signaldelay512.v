// signaldelay512.v --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 四 7月 18 11:42:52 2019 (+0800)
// Last-Updated: 六 12月  7 21:04:21 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 3
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

   reg 	      delay1;
   reg 	      delay2;
   reg 	      delay3;
   reg 	      delay4;
   reg 	      delay5;
   always @(posedge clk) begin
      delay1 <= din;
      delay2 <= delay1;
      delay3 <= delay2;
      delay4 <= delay3;
      delay5 <= delay4;
   end
   
   
   always @(posedge clk) begin
      if(delay == 0)begin
	 dout <= din;
      end
      else if(delay == 1) begin
	 dout <= delay1;
      end
      else if(delay == 2) begin
	 dout <= delay2;
      end
      else if(delay == 3) begin
	 dout <= delay3;
      end
      else if(delay == 4) begin
	 dout <= delay4;
      end
      else if(delay == 5) begin
	 dout <= delay5;
      end
      else begin
	 if(delay_rd_en) begin
	    dout <= delayoutput_tmp;
	 end	 
      end
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
