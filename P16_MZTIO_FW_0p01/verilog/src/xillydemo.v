`define NTS  

module xillydemo
  (
   input 	 CRS, // done
   output 	 MDC, // done
   inout 	 MDIO, // done
   input 	 PTPClk, // done
   input [4:3] 	 PTPTrig, // done
   input 	 Rx_Clk, // done
   input 	 Rx_Er, // done
   input [1:0] 	 Rx_D, // done
   output 	 SCL, // done
   inout 	 SDA, // done
   output 	 Tx_En, // done
   output [1:0]  Tx_D, // done
   
   input [5:4] 	 Control, // ? done   all slots
   input [1:0] 	 EB_Ctrl, // ? done   PXI segment only
   
   inout [31:16] EB_Data, // done     PXI segment only
   inout [15:0]  FrontIO_A, // done
   inout [15:0]  FrontIO_B, // done  
   inout [15:0]  FrontIO_C, // done
   inout [31:0]  TriggerAll // done   all slots
   ); 
   
`define SYSREVISION 32'hB100_0001    
   //    product ID (same as in EEPROM). (upper 16 bits)                                                  
   //   FW variant | major build number  | minor build number  | minor build number  (lower 16 bits)        
   
   // *************************************************************
   // *************** IO Buffers **********************************
   // ************************************************************* 
   // T   I   IO   O
   // 1   x   Z    IO
   // 0   1   1    1
   // 0   0   0    0
   
   // IOBUF ,when T==0, I => IO   You should assign an value to I
   //        when T==1, IO => O   You can use O directly

   // ----------- IO Buffers Trigger I/O----------------------------  
   wire [15:0] 	 FrontIO_Ain, FrontIO_Aout, FrontIO_Aena;    // front panel LVDS
   wire [15:0] 	 FrontIO_Bin, FrontIO_Bout, FrontIO_Bena;    // front panel LVDS  
   wire [15:0] 	 FrontIO_Cin, FrontIO_Cout, FrontIO_Cena;    // front panel LVDS
   
   wire [31:0] 	 TriggerAllin, TriggerAllout, TriggerAllena; // on J4 backplane
   wire [31:16]  EB_Datain, EB_Dataout, EB_Dataena;         // on J3 backplane
   
   // IOBUF "in" is the signal from the fabric to the IO buffer, for output to PCB on IO pin if T==0
   // IOBUF "out" is the signal to the fabric from the IO buffer and PCB
   IOBUF fa [15:0] (.IO(FrontIO_A),  .I(FrontIO_Ain), .O(FrontIO_Aout), .T(~FrontIO_Aena));
   IOBUF fb [15:0] (.IO(FrontIO_B),  .I(FrontIO_Bin), .O(FrontIO_Bout), .T(~FrontIO_Bena));
   IOBUF fc [15:0] (.IO(FrontIO_C),  .I(FrontIO_Cin), .O(FrontIO_Cout), .T(~FrontIO_Cena));

   // The configuration of the FrontIO_A/B/C is completely flexible. For example, if you connect the RJ-45 of a Pixie-16 to FrontI/O A 0-3 (the upper RJ-45 on the trigger board), signals will connect
   // FO5 - Front I/O A 3      FrontIO_Aena==0
   // FO1 - Front I/O A 0      FrontIO_Aena==0
   // FI5 - Front I/O A 1      FrontIO_Aena==1
   // FI1 - Front I/O A 2      FrontIO_Aena==1

   // F0  5p/5n  synchronization status / multiplicity result channel 0(pku firmware)
   // FO  1p/1n  not used / multiplicity result channel 1(pku firmware) 
   // FI  5p/5n  external fast trigger
   // FI  1p/1n  external validation trigger

   // FrontIO_Aout [3] [0]  [7] [4]  [11] [8]  [15] [12]
   // FrontIO_Ain  [1] [2]  [5] [6]  [9] [10]  [13] [14]
   
   IOBUF ta [31:0] (.IO(TriggerAll), .I(TriggerAllin), .O(TriggerAllout), .T(~TriggerAllena));
   IOBUF ebd [31:16] (.IO(EB_Data),  .I(EB_Datain), .O(EB_Dataout), .T(~EB_Dataena));

   
   wire [5:4] 	 ctrl;   // spare J4
   wire [1:0] 	 eb_c;   // spare J3
   IBUF en11 (.O(ctrl[4]), .I(Control[4]));
   IBUF en12 (.O(ctrl[5]), .I(Control[5]));
   IBUF en13 (.O(eb_c[0]), .I(EB_Ctrl[0]));
   IBUF en14 (.O(eb_c[1]), .I(EB_Ctrl[1]));
   
   /* Pixie-16 pin usage (see manual for full details)
    TriggerAll[0] = FTBACK 
    = input to P16
    global trigger from packplane (trigger module) 
    TriggerAll[1] = DPMFULLIN 
    = input to P16, from packplane (trigger module or director), 
    indicates system memory full to P16 module 
    TriggerAll[2] = DPMFULLOUT 
    = IO for P16; output enabled if PullUp_CTRL=1
    output from P16 = DPMfull_BP indicates P16's module's memory is full (wired OR on backplane)   
    input to P16 = not used             
    TriggerAll[3] = SYNCIN 
    = input to P16, from packplane (trigger module or director), 
    indicates ALL chassis in system are ready
    TriggerAll[4] = ETBACK 
    = input to P16
    global trigger from packplane (trigger module) 
    TriggerAll[5] = SYNCOUT 
    = IO for P16; output enabled if PullUp_CTRL=1; 
    output from P16 = synco, the wire OR of another backplane line  (wired OR on backplane)   
    input to P16 = not used
    TriggerAll[6] = ETLOCAL 
    = IO for P16; output enabled if PullUp_CTRL=1; 
    output from P16 = Ext_ValidTrig_In 
    input to P16 = ft_localcrate_bp, one option for module fast trigger
    TriggerAll[7] = FTLOCAL 
    = IO for P16; output enabled if PullUp_CTRL=1; 
    output from P16 = Ext_FastTrig_In 
    input to P16 = et_localcrate_bp, one option for module validation trigger
    
    see User Manual Table 3-9 for definition of Ext_FastTrig_In, Ext_ValidTrig_In   
    for example, to enable fast triggers on TriggerAll[7] to count below, 
    - set MODCSRB_CPLDPULLUP [bit 0] (Pullup_CTRL =1)
    - set MODCSRB_CHASSISMASTER [bit 6]
    - specify TrigControl 0 = 0x######2# (Ext_FastTrig_In = OR of channels' fast trigegrs)
    
    All others unused
    
    */
   
   
   // ----------- IO Buffers Ethernet & PTP ----------------------------  
   
   // ptpclk comes from the PLL on the MZ_TrigIO board. The PLL is programmed with the "clockprog" utility on the 
   // ARM/Linux side to specify the frequency. Input to the PLL is one of these signals (jumper settings on board)
   //  - P16 chassis backplane
   //  - MMCX front panel connector
   //  - PTP PHY (synchronized over network)
   // By default, the frequency is 50 MHz.  
   // The PTP triggers are signals created by the PHY at user defined time/date. See ptp-mii-tool on ARM/Linux side
   wire 	 ptpclk_o;
   IBUFG ptpc (.O(ptpclk_o), .I(PTPClk)); // user_clk???
   wire [4:3] 	 ptptrig;
   IBUF en0 (.O(ptptrig[3]), .I(PTPTrig[3]));
   IBUF en1 (.O(ptptrig[4]), .I(PTPTrig[4]));
   
   // Ethernet I/O
   wire [1:0] 	 rmii2phy_txd;
   wire 	 rmii2phy_tx_en;
   wire [1:0] 	 phy2rmii_rxd;
   wire 	 phy2rmii_rx_er;
   wire 	 phy2rmii_crs_dv;
   wire 	 ethclk50;
   wire 	 reset_rtl;
   wire 	 mdio_phy_mdc; 
   wire 	 mdio_phy_io; 
   OBUF en2 (.O(Tx_D[0]), .I(rmii2phy_txd[0]));
   OBUF en3 (.O(Tx_D[1]), .I(rmii2phy_txd[1]));
   OBUF en4 (.O(Tx_En), .I(rmii2phy_tx_en));
   IBUF en5 (.O(phy2rmii_rxd[0]), .I(Rx_D[0]));
   IBUF en6 (.O(phy2rmii_rxd[1]), .I(Rx_D[1]));
   IBUF en7 (.O(phy2rmii_rx_er), .I(Rx_Er));
   IBUF en8 (.O(phy2rmii_crs_dv), .I(CRS));
   IBUF en9 (.O(ethclk50), .I(Rx_Clk));
   OBUF en10 (.O(MDC), .I(mdio_phy_mdc));
   assign  MDIO = mdio_phy_io; //  no IOBUF spelled out, else causes no-routing error. (i/o/t to io in system.v)
   assign reset_rtl = 1;
   

   // ----------- IO Buffers local control ----------------------------  
   // I2C pins 
   // The I2C bus is toggled by the FPGA via register r/w from the ARM/Linux side. 
   // Devices on the I2C bus are
   //  - TMP116 PROM/thermometer
   //  - "bus externders" to control the direction of the front panel LVDS buffers
   //  - the PTPclk PLL
   wire 	 d18_sda, d18_scl, d18_ena, d18_sdaout;
   IOBUF i2c1 (.IO(SDA), .I(d18_sda), .O(d18_sdaout), .T(!d18_ena));
   OBUF i2c2 (.O(SCL), .I(d18_scl));



   

   // *************************************************************
   // *****************   I/O with the PS side ******************** //
   // *************************************************************
   
   // ----------------- xillybus logic ----------------------------------
   // Wires related to Xillybus Lite
   // xillybus lite gives up to 4K 32bit words of FPGA registers memory mapped into Linux uio space. 
   // here we define 512 in and out (but use only partially) 
   // reads is divided into 2 blocks, selected by "outblock" 
   //     outblock == 0 : read back what is wrtten into "litearray"
   //     outblock == 1 : read results from the logic
   //
   
   // i/o data array     Input register
   reg [7:0] 	 litearray0[0:511];
   reg [7:0] 	 litearray1[0:511];
   reg [7:0] 	 litearray2[0:511];
   reg [7:0] 	 litearray3[0:511];
   // 000   CSR
   // 001   
   // 002   D18
   // 003   outblock 
   // 006   snum
   // 100   FrontIO_Aena
   // 101   FrontIO_Bena
   // 102   FrontIO_Cena
   // 103   TriggerAllena
   // 104   EB_Dataena
   // 108   frontA_coincidence_mask
   // 109   frontB_coincidence_mask
   // 10A   frontC_coincidence_mask
   // 10B   TriggerAll_coincidence_mask
   // 10C   EB_Data_coincidence_mask
   // 110   frontA_multiplicity_mask
   // 111   frontB_multiplicity_mask
   // 112   frontC_multiplicity_mask
   // 113   TriggerAll_multiplicity_mask
   // 114   EB_Data_multiplicity_mask
   // 118   frontA_coincidence_pattern
   // 119   frontB_coincidence_pattern
   // 11A   frontC_coincidence_pattern
   // 11B   TriggerAll_coincidence_pattern
   // 11C   EB_Data_coincidence_pattern
   // 120   frontA_multiplicity_threshold
   // 121   frontB_multiplicity_threshold
   // 122   frontC_multiplicity_threshold
   // 123   TriggerAll_multiplicity_threshold
   // 124   EB_Data_multiplicity_threshold
   // 128   frontA_output_select
   // 129   frontB_output_select
   // 12A   frontC_output_select
   // 12B   TriggerAll_output_select
   // 12C   EB_Data_output_select
   
   
   // output data array     Output register
   wire [31:0] 	 evdata[0:511];  // 512 words, 32bit wide
   // 000   CSR
   // 001   VERSION
   // 002
   // 003
   // 004
   // 005   coincresult
   // 006
   // 007
   // 008
   // 009   
   // 00A   numtrig
   // 00B   numtrig
   // 00C   runticks
   // 00D   runticks
   // 00E
   // 00F
   // 010
   // 011
   // 012   snum
   // 100   FrontIO_Aena   // change
   // 101   FrontIO_Bena   // change
   // 102   FrontIO_Cena   // change
   // 103   TriggerAllena  // change
   // 104   EB_Dataena     // change
   // 108   FrontIO_Aout & frontA_coincidence_mask
   // 109   FrontIO_Bout & frontB_coincidence_mask
   // 10A   FrontIO_Cout & frontC_coincidence_mask
   // 10B   TriggerAllout & TriggerAll_coincidence_mask
   // 10C   16'h0000, EB_Dataout & EB_Data_coincidence_mask
   // 110   sumA
   // 111   sumB
   // 112   sumC
   // 113   sumT
   // 114   sumE

   // 1E0(480) - 1FF(511) output scaler

   
   // PS-PL wires
   wire 	 user_clk;
   wire 	 user_wren;
   wire [3:0] 	 user_wstrb;
   wire 	 user_rden;
   reg [31:0] 	 user_rd_data;
   wire [31:0] 	 user_wr_data;
   wire [31:0] 	 user_addr;       // lowest 2 user_addr bits are always zero, 4096 words max in lite; 
   wire 	 user_irq;
   assign      user_irq = 0;   // No interrupts for now
   wire [1:0] 	 outblock;// Used to control the read register position. 0 means read input register, 1 means read output register
   

   always @(posedge user_clk)
     begin    
        // write block
        if (user_wstrb[0])
          litearray0[user_addr[10:2]] <= user_wr_data[7:0];
	
        if (user_wstrb[1])
          litearray1[user_addr[10:2]] <= user_wr_data[15:8];
	
        if (user_wstrb[2])
          litearray2[user_addr[10:2]] <= user_wr_data[23:16];
	
        if (user_wstrb[3])
          litearray3[user_addr[10:2]] <= user_wr_data[31:24];                 
	
        // read block
        if (user_rden) begin
           if(outblock==0)
             user_rd_data <=  { litearray3[user_addr[10:2]], litearray2[user_addr[10:2]], litearray1[user_addr[10:2]], litearray0[user_addr[10:2]] };
           else if (outblock==1 )    
             user_rd_data <=  evdata[user_addr[10:2]];
           else 
             user_rd_data <= 32'h00000123;
        end  //read  
	
        // though they seem to come at the same time on the scope, addr and rden don't seem to be
        // active in the same clk cycle. More like rden enables the assignment in the next clk cycle
        // and addr only are valid in that next cycle. So addr can not be used in the if or casex 
        // condition. Instead use outblock, set in a previous write. 
     end
   
   // ***********  generate strobes for post-read/write actions *****************
   /* strobe/select pulses   (in user_clk domain) 
    prefix u_ indicates in user_clk domain
    */
   
   reg u_counter_clr;
   reg wrpulse, rdpulse;
   
   always @(posedge user_clk)
     begin
        wrpulse <= |user_wstrb;      // delay the strobes 1 cycle to match addr
        rdpulse <= user_rden;   
        u_counter_clr      <= wrpulse &  (user_addr[10:2] == 9'h009);
     end

   
   
   // ----------------- connect the input registers to local nets ---------------------------
   
   wire [15:0] CSRin, snum;
   wire        runenable;
   assign CSRin[15:0]  = {litearray1[9'h000][7:0],litearray0[9'h000][7:0]};    
   assign runenable = CSRin[0]; 
   
   assign d18_sda  = litearray0[2][0];      // I2C control
   assign d18_scl  = litearray0[2][1];
   assign d18_ena  = litearray0[2][2];
   assign outblock = litearray0[3][1:0];   // read back block 
   assign snum[15:0]  = {litearray1[9'h006][7:0],litearray0[9'h006][7:0]}; 

   
   assign FrontIO_Aena[15:0]  = {litearray1[9'h100][7:0],litearray0[9'h100][7:0]};
   assign FrontIO_Bena[15:0]  = {litearray1[9'h101][7:0],litearray0[9'h101][7:0]};
   assign FrontIO_Cena[15:0]  = {litearray1[9'h102][7:0],litearray0[9'h102][7:0]};
   assign TriggerAllena[31:0] = {litearray3[9'h103][7:0],litearray2[9'h103][7:0],litearray1[9'h103][7:0],litearray0[9'h103][7:0]};
   assign EB_Dataena[31:16]   = {litearray1[9'h104][7:0],litearray0[9'h104][7:0]};

   
   wire [15:0] frontA_coincidence_mask, frontB_coincidence_mask, frontC_coincidence_mask;
   wire [31:0] TriggerAll_coincidence_mask; 
   wire [31:16] EB_Data_coincidence_mask;
   assign frontA_coincidence_mask[15:0]     = {litearray1[9'h108][7:0],litearray0[9'h108][7:0]};
   assign frontB_coincidence_mask[15:0]     = {litearray1[9'h109][7:0],litearray0[9'h109][7:0]};
   assign frontC_coincidence_mask[15:0]     = {litearray1[9'h10A][7:0],litearray0[9'h10A][7:0]};
   assign TriggerAll_coincidence_mask[31:0] = {litearray3[9'h10B][7:0],litearray2[9'h10B][7:0],litearray1[9'h10B][7:0],litearray0[9'h10B][7:0]};
   assign EB_Data_coincidence_mask[31:16]   = {litearray1[9'h10C][7:0],litearray0[9'h10C][7:0]};
   
   wire [15:0] 	frontA_multiplicity_mask, frontB_multiplicity_mask, frontC_multiplicity_mask;
   wire [31:0] 	TriggerAll_multiplicity_mask; 
   wire [31:16] EB_Data_multiplicity_mask;
   assign frontA_multiplicity_mask[15:0]     = {litearray1[9'h110][7:0],litearray0[9'h110][7:0]};
   assign frontB_multiplicity_mask[15:0]     = {litearray1[9'h111][7:0],litearray0[9'h111][7:0]};
   assign frontC_multiplicity_mask[15:0]     = {litearray1[9'h112][7:0],litearray0[9'h112][7:0]};
   assign TriggerAll_multiplicity_mask[31:0] = {litearray3[9'h113][7:0],litearray2[9'h113][7:0],litearray1[9'h113][7:0],litearray0[9'h113][7:0]};
   assign EB_Data_multiplicity_mask[31:16]   = {litearray1[9'h114][7:0],litearray0[9'h114][7:0]};
   
   wire [15:0] 	frontA_coincidence_pattern, frontB_coincidence_pattern, frontC_coincidence_pattern;
   wire [31:0] 	TriggerAll_coincidence_pattern; 
   wire [31:16] EB_Data_coincidence_pattern;
   assign frontA_coincidence_pattern[15:0]     = {litearray1[9'h118][7:0],litearray0[9'h118][7:0]};
   assign frontB_coincidence_pattern[15:0]     = {litearray1[9'h119][7:0],litearray0[9'h119][7:0]};
   assign frontC_coincidence_pattern[15:0]     = {litearray1[9'h11A][7:0],litearray0[9'h11A][7:0]};
   assign TriggerAll_coincidence_pattern[31:0] = {litearray3[9'h11B][7:0],litearray2[9'h11B][7:0],litearray1[9'h11B][7:0],litearray0[9'h11B][7:0]};
   assign EB_Data_coincidence_pattern[31:16]   = {litearray1[9'h11C][7:0],litearray0[9'h11C][7:0]};

   wire [7:0] 	frontA_multiplicity_threshold, frontB_multiplicity_threshold, frontC_multiplicity_threshold;
   wire [7:0] 	TriggerAll_multiplicity_threshold; 
   wire [7:0] 	EB_Data_multiplicity_threshold;
   assign frontA_multiplicity_threshold[7:0]     = {litearray0[9'h120][7:0]};
   assign frontB_multiplicity_threshold[7:0]     = {litearray0[9'h121][7:0]};
   assign frontC_multiplicity_threshold[7:0]     = {litearray0[9'h122][7:0]};
   assign TriggerAll_multiplicity_threshold[7:0] = {litearray0[9'h123][7:0]};
   assign EB_Data_multiplicity_threshold[7:0]    = {litearray0[9'h124][7:0]};
   
   wire [7:0] 	frontA_output_select, frontB_output_select, frontC_output_select;
   wire [7:0] 	TriggerAll_output_select; 
   wire [7:0] 	EB_Data_output_select;
   assign frontA_output_select[7:0]     = {litearray0[9'h128][7:0]};
   assign frontB_output_select[7:0]     = {litearray0[9'h129][7:0]};
   assign frontC_output_select[7:0]     = {litearray0[9'h12A][7:0]};
   assign TriggerAll_output_select[7:0] = {litearray0[9'h12B][7:0]};
   assign EB_Data_output_select[7:0]    = {litearray0[9'h12C][7:0]};


   // -------------------- connect the output registers to local nets ---------------------
   wire [15:0] 	CSRout, coincresult;  
   reg [63:00] 	numtrig, runticks, ptpticks;
   reg [7:0] 	sumA, sumB, sumC, sumT, sumL, sumH, sumE;
   assign evdata[9'h000] = {16'h0000, CSRout};        // CSR output
   assign evdata[9'h001] = `SYSREVISION;  
   // 2-4 reserved, do not use (SW adds values)          
   assign evdata[9'h005] = {16'h0000, coincresult};  
   //6-9 unused (printed as hex)     
   assign evdata[9'h00A] = numtrig[31:00];  
   assign evdata[9'h00B] = numtrig[63:32];
   assign evdata[9'h00C] = runticks[31:00];  
   assign evdata[9'h00D] = runticks[63:32];   
   // 14-17 reserved, do not use (SW adds values)            
   assign evdata[9'h012] = {16'h0000, snum};       
   
   // inputs direct          wuhongyi
   assign evdata[9'h100] = {16'h0000, FrontIO_Aena};        
   assign evdata[9'h101] = {16'h0000, FrontIO_Bena};
   assign evdata[9'h102] = {16'h0000, FrontIO_Cena};
   assign evdata[9'h103] = {TriggerAllena};
   assign evdata[9'h104] = {16'h0000, EB_Dataena};
   
   // inputs with coincidence mask 
   assign evdata[9'h108] = {16'h0000, FrontIO_Aout & frontA_coincidence_mask};
   assign evdata[9'h109] = {16'h0000, FrontIO_Bout & frontB_coincidence_mask};
   assign evdata[9'h10A] = {16'h0000, FrontIO_Cout & frontC_coincidence_mask};
   assign evdata[9'h10B] = {TriggerAllout & TriggerAll_coincidence_mask};
   assign evdata[9'h10C] = {16'h0000, EB_Dataout & EB_Data_coincidence_mask};   
   
   // multiplicity sums 
   assign evdata[9'h110] = {24'h000000, sumA};
   assign evdata[9'h111] = {24'h000000, sumB};
   assign evdata[9'h112] = {24'h000000, sumC};
   assign evdata[9'h113] = {24'h000000, sumT};
   assign evdata[9'h114] = {24'h000000, sumE};  

   reg [31:0] 	scaler0[0:32];
   reg [31:0] 	scaler0_tmp[0:32];
   reg [31:0] 	cnt1s    ;
   wire 	add_cnt1s;
   wire 	end_cnt1s;
   always @(posedge user_clk) begin
      if(add_cnt1s) begin
	 if(end_cnt1s)
	   cnt1s <= 0;
	 else
	   cnt1s <= cnt1s + 1;
      end
   end
   assign add_cnt1s = 1;//condition: add 1 
   assign end_cnt1s = add_cnt1s && cnt1s >= 100000000 - 1; //End condition, last value

   reg sign_1b , sign_2b , sign_pos;
   always @(posedge user_clk) begin
      sign_1b <= FrontIO_Aout[3];
      sign_2b <= sign_1b; 
   end
   always @(posedge user_clk) begin
      if(sign_2b && !sign_1b) 
	sign_pos <= 1;
      else
	sign_pos <= 0;
   end

   always  @(posedge user_clk)begin
      if(sign_pos && end_cnt1s)begin
	 scaler0_tmp[5'h0] <= 1;
      end
      else if(end_cnt1s)begin
	 scaler0_tmp[5'h0] <= 0;
      end
      else if(sign_pos)begin
	 scaler0_tmp[5'h0] <= scaler0_tmp[5'h0]+1;
      end
   end
   
   always @(posedge user_clk) begin
      if(end_cnt1s) begin
	 scaler0[5'h0] <= scaler0_tmp[5'h0];
      end
   end

   assign evdata[9'h1E0] = scaler0[5'h0];
   
   
   // *************************************************************
   // ****************  Processing Logic **************************
   // *************************************************************
   
   wire 	proc_clk;
   assign proc_clk = user_clk;   // for now. possibly better to use ptpclk, but then have to handle the clock crossing
   
   // -----------------  define local nets for read -----------------
   
   // CSR Status data
   assign CSRout[0] = runenable;         
   assign CSRout[1] = 1'b0;              
   assign CSRout[2] = d18_sdaout;        
   assign CSRout[3] = ptptrig[3];        
   assign CSRout[4] = ptptrig[4];          
   assign CSRout[5] = 1'b0;   // PTPsyncGPIO;       
   assign CSRout[6] = 1'b0;       
   assign CSRout[7] = 1'b0;   // PTPena;         
   assign CSRout[8] = 1'b0;        
   assign CSRout[9] = 1'b0;        
   assign CSRout[10] = 1'b0;                 
   assign CSRout[11] = 1'b0; 
   assign CSRout[12] = 1'b0;        
   assign CSRout[13] = 1'b0;    
   assign CSRout[14] = 1'b0;      
   assign CSRout[15] = 1'b0;   
   
   // ----------------- coincidence test  ----------------- 
   assign coincresult[0] = (FrontIO_Aout & frontA_coincidence_mask)        == frontA_coincidence_pattern ;      
   assign coincresult[1] = (FrontIO_Bout & frontB_coincidence_mask)        == frontB_coincidence_pattern ;      
   assign coincresult[2] = (FrontIO_Cout & frontC_coincidence_mask)        == frontC_coincidence_pattern ;      
   assign coincresult[3] = (TriggerAllout & TriggerAll_coincidence_mask)   == TriggerAll_coincidence_pattern ;      
   assign coincresult[4] = (EB_Dataout & EB_Data_coincidence_mask)         == EB_Data_coincidence_pattern ;      

   //  -----------------  multiplicity sum and comparison ----------------- 
   wire [15:0] 	mmA, mmB, mmC;
   wire [31:0] 	mmT;
   wire [31:16] mmE; 
   reg [5:0] 	sumAa, sumBa, sumCa, sumLa, sumHa, sumEa;
   reg [5:0] 	sumAb, sumBb, sumCb, sumLb, sumHb, sumEb;
   reg [5:0] 	sumAc, sumBc, sumCc, sumLc, sumHc, sumEc;
   reg [5:0] 	sumAd, sumBd, sumCd, sumLd, sumHd, sumEd;
   assign mmA = (FrontIO_Aout & frontA_multiplicity_mask) ;    
   assign mmB = (FrontIO_Bout & frontB_multiplicity_mask);     
   assign mmC = (FrontIO_Cout & frontC_multiplicity_mask) ;    
   assign mmT = (TriggerAllout & TriggerAll_multiplicity_mask);
   assign mmE = (EB_Dataout & EB_Data_multiplicity_mask)  ;   
   
   always @(posedge proc_clk)
     begin  
        sumAa <= mmA[00] + mmA[01] + mmA[02] + mmA[03]; 
        sumAb <= mmA[04] + mmA[05] + mmA[06] + mmA[07]; 
        sumAc <= mmA[08] + mmA[09] + mmA[10] + mmA[11]; 
        sumAd <= mmA[12] + mmA[13] + mmA[14] + mmA[15]; 
        sumA <= sumAa + sumAb + sumAc + sumAd;
	
        sumBa <= mmB[00] + mmB[01] + mmB[02] + mmB[03]; 
        sumBb <= mmB[04] + mmB[05] + mmB[06] + mmB[07]; 
        sumBc <= mmB[08] + mmB[09] + mmB[10] + mmB[11]; 
        sumBd <= mmB[12] + mmB[13] + mmB[14] + mmB[15]; 
        sumB <= sumBa + sumBb + sumBc + sumBd;
        
        sumCa <= mmC[00] + mmC[01] + mmC[02] + mmC[03]; 
        sumCb <= mmC[04] + mmC[05] + mmC[06] + mmC[07]; 
        sumCc <= mmC[08] + mmC[09] + mmC[10] + mmC[11]; 
        sumCd <= mmC[12] + mmC[13] + mmC[14] + mmC[15]; 
        sumC <= sumCa + sumCb + sumCc + sumCd;
        
        sumLa <= mmT[00] + mmT[01] + mmT[02] + mmT[03]; 
        sumLb <= mmT[04] + mmT[05] + mmT[06] + mmT[07]; 
        sumLc <= mmT[08] + mmT[09] + mmT[10] + mmT[11]; 
        sumLd <= mmT[12] + mmT[13] + mmT[14] + mmT[15]; 
        sumL <= sumLa + sumLb + sumLc + sumLd;
        
        sumHa <= mmT[16] + mmT[17] + mmT[18] + mmT[19]; 
        sumHb <= mmT[20] + mmT[21] + mmT[22] + mmT[23]; 
        sumHc <= mmT[24] + mmT[25] + mmT[26] + mmT[27]; 
        sumHd <= mmT[28] + mmT[29] + mmT[30] + mmT[31]; 
        sumH <= sumHa + sumHb + sumHc + sumHd;
        sumT <= sumL  + sumH;
        
        sumEa <= mmE[16] + mmE[17] + mmE[18] + mmE[19]; 
        sumEb <= mmE[20] + mmE[21] + mmE[22] + mmE[23]; 
        sumEc <= mmE[24] + mmE[25] + mmE[26] + mmE[27]; 
        sumEd <= mmE[28] + mmE[29] + mmE[30] + mmE[31]; 
        sumE <= sumEa + sumEb + sumEc + sumEd;
	
     end
   assign coincresult[8]  = sumA  >= frontA_multiplicity_threshold;      
   assign coincresult[9]  = sumB  >= frontB_multiplicity_threshold ;      
   assign coincresult[10] = sumC  >= frontC_multiplicity_threshold ;      
   assign coincresult[11] = sumT  >= TriggerAll_multiplicity_threshold ;      
   assign coincresult[12] = sumE  >= EB_Data_multiplicity_threshold ;      
   
   
   // -----------------  counters -----------------
   
   // Note: Can add logic to use backplane line or PTP trigger to clear counters
   // and count only while run in progress. 
   
   // count time in clock ticks
   always @(posedge proc_clk)
     if(u_counter_clr)
       runticks <= 0;
     else
       runticks <= runticks +1;
   
   // count triggers in TriggerAll[7]  
   // first make a pulse
   reg ta7a, ta7b; 
   wire ta7p;    
   always @(posedge proc_clk)
     begin
        ta7a <= TriggerAllout[7] ;
        ta7b <= ta7a;
     end
   assign ta7p = ta7a & !ta7b;
   
   // then count the pulses
   always @(posedge proc_clk)
     if(u_counter_clr)
       numtrig <= 0;
     else if(ta7p)
       numtrig <= numtrig +1;
   
   
   // another counter on the other clock. 
   // probably violates some clock crossing, but not important right now.     
   always @(posedge ptpclk_o)
     if(u_counter_clr)
       ptpticks <= 0;
     else
       ptpticks <= ptpticks +1;

   
   // -----------------  multiplexer for trigger outputs -----------------
   
   
   // specify the outputs to the pins
   // the outputs are multiplexed with output_select
   //   0 - backplane to front panel 
   //   1 - backplane & coinc mask to front panel
   //   2 - backplane & multiplicity mask to front panel
   //   3 - backplane coincidence result to front panel
   //   4 - backplane multiplicity result to front panel
   //   5 - test signal to front panel

   assign TriggerAllin[31:0] = 0;        // for now, output nothing to backplane
   assign EB_Datain[31:16]   = 0;        // for now, output nothing to backplane
   
   // assign FrontIO_Ain[15:0] = (frontA_output_select == 0 )?  TriggerAllout[15:0] : 16'bzzzz ; 
   // assign FrontIO_Ain[15:0] = (frontA_output_select == 1 )? (TriggerAllout[15:0] & TriggerAll_coincidence_mask[15:0]) : 16'bzzzz ; 
   // assign FrontIO_Ain[15:0] = (frontA_output_select == 2 )? (TriggerAllout[15:0] & TriggerAll_multiplicity_mask[15:0]): 16'bzzzz ; 
   // assign FrontIO_Ain[15:0] = (frontA_output_select == 3 )? {16{coincresult[3]}} : 16'bzzzz ; 
   // assign FrontIO_Ain[15:0] = (frontA_output_select == 4 )? {16{coincresult[11]}} : 16'bzzzz ;
   // assign FrontIO_Ain[15:0] = (frontA_output_select == 5 )? {16{runticks[8]}} : 16'bzzzz ;

   // assign FrontIO_Bin[15:0] = (frontB_output_select == 0 )?  TriggerAllout[31:16] : 16'bzzzz ; 
   // assign FrontIO_Bin[15:0] = (frontB_output_select == 1 )? (TriggerAllout[31:16] & TriggerAll_coincidence_mask[31:16]) : 16'bzzzz ; 
   // assign FrontIO_Bin[15:0] = (frontB_output_select == 2 )? (TriggerAllout[31:16] & TriggerAll_multiplicity_mask[31:16]): 16'bzzzz ; 
   // assign FrontIO_Bin[15:0] = (frontB_output_select == 3 )? {16{coincresult[3]}} : 16'bzzzz ; 
   // assign FrontIO_Bin[15:0] = (frontB_output_select == 4 )? {16{coincresult[11]}} : 16'bzzzz ; 
   // assign FrontIO_Bin[15:0] = (frontB_output_select == 5 )? {16{runticks[8]}} : 16'bzzzz ;
   
   // assign FrontIO_Cin[15:0] = (frontC_output_select == 0 )?  EB_Dataout[31:16] : 16'bzzzz ; 
   // assign FrontIO_Cin[15:0] = (frontC_output_select == 1 )? (EB_Dataout[31:16] & EB_Data_coincidence_mask[31:16]) : 16'bzzzz ; 
   // assign FrontIO_Cin[15:0] = (frontC_output_select == 2 )? (EB_Dataout[31:16] & EB_Data_multiplicity_mask[31:16]): 16'bzzzz ; 
   // assign FrontIO_Cin[15:0] = (frontC_output_select == 3 )? {16{coincresult[4]}} : 16'bzzzz ; 
   // assign FrontIO_Cin[15:0] = (frontC_output_select == 4 )? {16{coincresult[12]}} : 16'bzzzz ; 
   // assign FrontIO_Cin[15:0] = (frontC_output_select == 5 )? {16{runticks[8]}} : 16'bzzzz ; 


   assign FrontIO_Ain[1] = FrontIO_Aout[3];
   assign FrontIO_Ain[2] = FrontIO_Aout[3];//FRONT_A_OUTENA    0x0006

   assign FrontIO_Ain[5] = FrontIO_Aout[7];
   assign FrontIO_Ain[6] = FrontIO_Aout[7];//FRONT_A_OUTENA    0x0006
   
   
   // output to CAEN DT2495, change to NIM signal   // FRONT_C_OUTENA  0xaa
   assign FrontIO_Cin[1] = FrontIO_Aout[0];
   assign FrontIO_Cin[3] = FrontIO_Aout[3];
   assign FrontIO_Cin[5] = FrontIO_Aout[4];
   assign FrontIO_Cin[7] = FrontIO_Aout[7];   


   /* ***************** xillybus instantiation ****************** */
   //  can ignore net definitions below, only using xillybus lite for now


   // Wires related to /dev/xillybus_mem_8
   wire      user_r_mem_8_rden;
   wire      user_r_mem_8_empty;
   reg [7:0] user_r_mem_8_data;
   wire      user_r_mem_8_eof;
   wire      user_r_mem_8_open;
   wire      user_w_mem_8_wren;
   wire      user_w_mem_8_full;
   wire [7:0] user_w_mem_8_data;
   wire       user_w_mem_8_open;
   wire [4:0] user_mem_8_addr;
   wire       user_mem_8_addr_update;

   // Wires related to /dev/xillybus_read_32
   wire       user_r_read_32_rden;
   wire       user_r_read_32_empty;
   wire [31:0] user_r_read_32_data;
   wire        user_r_read_32_eof;
   wire        user_r_read_32_open;

   // Wires related to /dev/xillybus_read_8
   wire        user_r_read_8_rden;
   wire        user_r_read_8_empty;
   wire [7:0]  user_r_read_8_data;
   wire        user_r_read_8_eof;
   wire        user_r_read_8_open;

   // Wires related to /dev/xillybus_write_32
   wire        user_w_write_32_wren;
   wire        user_w_write_32_full;
   wire [31:0] user_w_write_32_data;
   wire        user_w_write_32_open;

   // Wires related to /dev/xillybus_write_8
   wire        user_w_write_8_wren;
   wire        user_w_write_8_full;
   wire [7:0]  user_w_write_8_data;
   wire        user_w_write_8_open;

   // Clock and quiesce
   wire        bus_clk;
   wire        quiesce;


   // Note that none of the ARM processor's direct connections to pads is
   // attached in the instantion below. Normally, they should be connected as
   // toplevel ports here, but that confuses Vivado 2013.4 to think that
   // some of these ports are real I/Os, causing an implementation failure.
   // This detachment results in a lot of warnings during synthesis and
   // implementation, but has no practical significance, as these pads are
   // completely unrelated to the FPGA bitstream.

   xillybus xillybus_ins (
      
			  // Ports related to /dev/xillybus_mem_8
			  // FPGA to CPU signals:
			  .user_r_mem_8_rden(user_r_mem_8_rden),
			  .user_r_mem_8_empty(user_r_mem_8_empty),
			  .user_r_mem_8_data(user_r_mem_8_data),
			  .user_r_mem_8_eof(user_r_mem_8_eof),
			  .user_r_mem_8_open(user_r_mem_8_open),

			  // CPU to FPGA signals:
			  .user_w_mem_8_wren(user_w_mem_8_wren),
			  .user_w_mem_8_full(user_w_mem_8_full),
			  .user_w_mem_8_data(user_w_mem_8_data),
			  .user_w_mem_8_open(user_w_mem_8_open),

			  // Address signals:
			  .user_mem_8_addr(user_mem_8_addr),
			  .user_mem_8_addr_update(user_mem_8_addr_update),

			  // Ports related to /dev/xillybus_read_32
			  // FPGA to CPU signals:
			  .user_r_read_32_rden(user_r_read_32_rden),
			  .user_r_read_32_empty(user_r_read_32_empty),
			  .user_r_read_32_data(user_r_read_32_data),
			  .user_r_read_32_eof(user_r_read_32_eof),
			  .user_r_read_32_open(user_r_read_32_open),

			  // Ports related to /dev/xillybus_read_8
			  // FPGA to CPU signals:
			  .user_r_read_8_rden(user_r_read_8_rden),
			  .user_r_read_8_empty(user_r_read_8_empty),
			  .user_r_read_8_data(user_r_read_8_data),
			  .user_r_read_8_eof(user_r_read_8_eof),
			  .user_r_read_8_open(user_r_read_8_open),

			  // Ports related to /dev/xillybus_write_32
			  // CPU to FPGA signals:
			  .user_w_write_32_wren(user_w_write_32_wren),
			  .user_w_write_32_full(user_w_write_32_full),
			  .user_w_write_32_data(user_w_write_32_data),
			  .user_w_write_32_open(user_w_write_32_open),

			  // Ports related to /dev/xillybus_write_8
			  // CPU to FPGA signals:
			  .user_w_write_8_wren(user_w_write_8_wren),
			  .user_w_write_8_full(user_w_write_8_full),
			  .user_w_write_8_data(user_w_write_8_data),
			  .user_w_write_8_open(user_w_write_8_open),    

			  // Xillybus Lite signals:
			  .user_clk ( user_clk ),
			  .user_wren ( user_wren ),
			  .user_wstrb ( user_wstrb ),
			  .user_rden ( user_rden ),
			  .user_rd_data ( user_rd_data ),
			  .user_wr_data ( user_wr_data ),
			  .user_addr ( user_addr ),
			  .user_irq ( user_irq ),
      
			  // General signals
			  .bus_clk(bus_clk),//http://xillybus.com/doc/microblaze-fpga-core-api
			  .quiesce(quiesce), 
      
			  // Ethernet signals
			  .mdio_phy_mdc(mdio_phy_mdc),
			  .mdio_phy_io(mdio_phy_io),
			  .rmii2phy_txd(rmii2phy_txd),
			  .rmii2phy_tx_en(rmii2phy_tx_en),
			  .phy2rmii_rx_er(phy2rmii_rx_er),
			  .phy2rmii_crs_dv(phy2rmii_crs_dv),
			  .phy2rmii_rxd(phy2rmii_rxd),
			  .ethclk50(ethclk50),
			  .reset_rtl(reset_rtl),
			  .testmii()
      
			  );




   /*
    // signals below refer to the (non-free) IP core
    
    // A simple inferred RAM
    always @(posedge bus_clk)
    begin
    if (user_w_mem_8_wren)
    demoarray[user_mem_8_addr] <= user_w_mem_8_data;
    
    if (user_r_mem_8_rden)
    user_r_mem_8_data <= demoarray[user_mem_8_addr];	  
     end

    assign  user_r_mem_8_empty = 0;
    assign  user_r_mem_8_eof = 0;
    assign  user_w_mem_8_full = 0;

    // 32-bit loopback
    fifo_32x512 fifo_32
    (
    .clk(bus_clk),
    .srst(!user_w_write_32_open && !user_r_read_32_open),
    .din(user_w_write_32_data),
    .wr_en(user_w_write_32_wren),
    .rd_en(user_r_read_32_rden),
    .dout(user_r_read_32_data),
    .full(user_w_write_32_full),
    .empty(user_r_read_32_empty)
    );

    assign  user_r_read_32_eof = 0;
    
    // 8-bit loopback
    fifo_8x2048 fifo_8
    (
    .clk(bus_clk),
    .srst(!user_w_write_8_open && !user_r_read_8_open),
    .din(user_w_write_8_data),
    .wr_en(user_w_write_8_wren),
    .rd_en(user_r_read_8_rden),
    .dout(user_r_read_8_data),
    .full(user_w_write_8_full),
    .empty(user_r_read_8_empty)
    );

    assign  user_r_read_8_eof = 0;
    */

   /* original xillybus lite single byte access
    always @(posedge user_clk)
    begin
    if (user_wstrb[0])
    litearray0[user_addr[6:2]] <= user_wr_data[7:0];

    if (user_wstrb[1])                       
    litearray1[user_addr[6:2]] <= user_wr_data[15:8];

    if (user_wstrb[2])
    litearray2[user_addr[6:2]] <= user_wr_data[23:16];

    if (user_wstrb[3])
    litearray3[user_addr[6:2]] <= user_wr_data[31:24];
    
    if (user_rden)
    user_rd_data <= { litearray3[user_addr[6:2]],
    litearray2[user_addr[6:2]],
    litearray1[user_addr[6:2]],
    litearray0[user_addr[6:2]] };
     end
    */

   /* ***************** test points ****************** */


   
endmodule
