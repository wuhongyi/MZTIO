# timing contraints

create_clock -period 20.000 -name ptpclk -waveform {0.000 10.000} [get_ports I_PTPCLK]
create_clock -period 20.000 -name ethclk50 -waveform {0.000 10.000} [get_ports IO_GPIO0p]

# pin definitions
#9
set_property PACKAGE_PIN R19 [get_ports {TriggerAll[22]}]
#10
set_property PACKAGE_PIN T19 [get_ports {TriggerAll[23]}] 

#11
set_property PACKAGE_PIN T11 [get_ports {TriggerAll[21]}] 
#12
set_property PACKAGE_PIN T12 [get_ports {TriggerAll[24]}] 
#13
set_property PACKAGE_PIN T10 [get_ports {TriggerAll[20]}] 
#14
set_property PACKAGE_PIN U12 [get_ports {TriggerAll[25]}] 

#17
set_property PACKAGE_PIN U13 [get_ports {TriggerAll[19]}]
#18
set_property PACKAGE_PIN V12 [get_ports {TriggerAll[26]}]
#19
set_property PACKAGE_PIN V13 [get_ports {TriggerAll[18]}]
#20
set_property PACKAGE_PIN W13 [get_ports {TriggerAll[27]}]

#23
set_property PACKAGE_PIN T14 [get_ports {TriggerAll[17]}]
#24
set_property PACKAGE_PIN P14 [get_ports {TriggerAll[28]}]
#25
set_property PACKAGE_PIN T15 [get_ports {TriggerAll[16]}]
#26
set_property PACKAGE_PIN R14 [get_ports {TriggerAll[29]}]

#29
set_property PACKAGE_PIN Y16 [get_ports {TriggerAll[15]}]
#30
set_property PACKAGE_PIN W14 [get_ports {TriggerAll[30]}]
#31
set_property PACKAGE_PIN Y17 [get_ports {TriggerAll[14]}]
#32
set_property PACKAGE_PIN Y14 [get_ports {TriggerAll[31]}]

#35
set_property PACKAGE_PIN T16 [get_ports {TriggerAll[13]}]
#36
set_property PACKAGE_PIN V15 [get_ports {EB_Data[16]}]
#37
set_property PACKAGE_PIN U17 [get_ports {TriggerAll[12]}]
#38
set_property PACKAGE_PIN W15 [get_ports {EB_Data[17]}]

#41
set_property PACKAGE_PIN U14 [get_ports PTPClk]
#42
set_property PACKAGE_PIN U18 [get_ports {EB_Data[18]}]
#43
set_property PACKAGE_PIN U15 [get_ports {TriggerAll[11]}]
#44
set_property PACKAGE_PIN U19 [get_ports {EB_Data[19]}]

#47
set_property PACKAGE_PIN N18 [get_ports Rx_Clk]
#48
set_property PACKAGE_PIN N20 [get_ports {EB_Data[20]}]
#49
set_property PACKAGE_PIN P19 [get_ports {TriggerAll[10]}]
#50
set_property PACKAGE_PIN P20 [get_ports {EB_Data[21]}]

#53
set_property PACKAGE_PIN T20 [get_ports {TriggerAll[9]}]
#54
set_property PACKAGE_PIN V20 [get_ports {EB_Data[22]}]
#55
set_property PACKAGE_PIN U20 [get_ports {TriggerAll[8]}]
#56
set_property PACKAGE_PIN W20 [get_ports {EB_Data[23]}]

#61
set_property PACKAGE_PIN Y18 [get_ports {TriggerAll[7]}]
#62
set_property PACKAGE_PIN V16 [get_ports {EB_Data[24]}]
#63
set_property PACKAGE_PIN Y19 [get_ports {TriggerAll[6]}]
#64
set_property PACKAGE_PIN W16 [get_ports {EB_Data[25]}]

#67
set_property PACKAGE_PIN R16 [get_ports {TriggerAll[5]}]
#68
set_property PACKAGE_PIN T17 [get_ports {EB_Data[26]}]
#69
set_property PACKAGE_PIN R17 [get_ports {TriggerAll[4]}]
#70
set_property PACKAGE_PIN R18 [get_ports {EB_Data[27]}]

#73
set_property PACKAGE_PIN V17 [get_ports {TriggerAll[3]}]
#74
set_property PACKAGE_PIN W18 [get_ports {EB_Data[28]}]
#75
set_property PACKAGE_PIN V18 [get_ports {TriggerAll[2]}]
#76
set_property PACKAGE_PIN W19 [get_ports {EB_Data[29]}]

#81
set_property PACKAGE_PIN N17 [get_ports {TriggerAll[1]}]
#82
set_property PACKAGE_PIN P15 [get_ports {EB_Data[30]}]
#83
set_property PACKAGE_PIN P18 [get_ports {TriggerAll[0]}]
#84
set_property PACKAGE_PIN P16 [get_ports {EB_Data[31]}]

#87
set_property PACKAGE_PIN U7 [get_ports {Control[4]}]
#88
set_property PACKAGE_PIN T9 [get_ports {Rx_D[0]}]
#89
set_property PACKAGE_PIN V7 [get_ports {Control[5]}]

#90
set_property PACKAGE_PIN U10 [get_ports {Rx_D[1]}]

#91
set_property PACKAGE_PIN V8 [get_ports {EB_Ctrl[1]}]
#92
set_property PACKAGE_PIN T5 [get_ports {Rx_Er}]
#93
set_property PACKAGE_PIN W8 [get_ports {EB_Ctrl[0]}]
#94
set_property PACKAGE_PIN U5 [get_ports CRS]

##100
set_property PACKAGE_PIN V5 [get_ports {PTPTrig[3]}]


##13
set_property PACKAGE_PIN G14 [get_ports {FrontIO_A[7]}]
##14
set_property PACKAGE_PIN J15 [get_ports {FrontIO_A[8]}]

##17
set_property PACKAGE_PIN C20 [get_ports {FrontIO_A[6]}]
##18
set_property PACKAGE_PIN B19 [get_ports {FrontIO_A[9]}]
##19
set_property PACKAGE_PIN B20 [get_ports {FrontIO_A[5]}]
##20
set_property PACKAGE_PIN A20 [get_ports {FrontIO_A[10]}]

##23
set_property PACKAGE_PIN E17 [get_ports {FrontIO_A[4]}]
##24
set_property PACKAGE_PIN D19 [get_ports {FrontIO_A[11]}]
##25
set_property PACKAGE_PIN D18 [get_ports {FrontIO_A[3]}]
##26
set_property PACKAGE_PIN D20 [get_ports {FrontIO_A[12]}]

##29
set_property PACKAGE_PIN E18 [get_ports {FrontIO_A[2]}]
##30
set_property PACKAGE_PIN F16 [get_ports {FrontIO_A[13]}]
##31
set_property PACKAGE_PIN E19 [get_ports {FrontIO_A[1]}]
##32
set_property PACKAGE_PIN F17 [get_ports {FrontIO_A[14]}]

##35
set_property PACKAGE_PIN L19 [get_ports {FrontIO_A[0]}]
##36
set_property PACKAGE_PIN M19 [get_ports {FrontIO_A[15]}]
##37
set_property PACKAGE_PIN L20 [get_ports {FrontIO_C[15]}]
##38
set_property PACKAGE_PIN M20 [get_ports {FrontIO_B[0]}]

##41
set_property PACKAGE_PIN M17 [get_ports {FrontIO_C[14]}]
##42
set_property PACKAGE_PIN K19 [get_ports {FrontIO_B[1]}]
##43
set_property PACKAGE_PIN M18 [get_ports {FrontIO_C[13]}]
##44
set_property PACKAGE_PIN J19 [get_ports {FrontIO_B[2]}]

##47
set_property PACKAGE_PIN L16 [get_ports {FrontIO_C[12]}]
##48
set_property PACKAGE_PIN K17 [get_ports {FrontIO_B[3]}]
##49
set_property PACKAGE_PIN L17 [get_ports SCL]
##50
set_property PACKAGE_PIN K18 [get_ports SDA]

##53
set_property PACKAGE_PIN H16 [get_ports {FrontIO_C[11]}]
##54
set_property PACKAGE_PIN J18 [get_ports {FrontIO_B[4]}]
##55
set_property PACKAGE_PIN H17 [get_ports {FrontIO_C[10]}]
##56
set_property PACKAGE_PIN H18 [get_ports {FrontIO_B[5]}]

##61
set_property PACKAGE_PIN G17 [get_ports {FrontIO_C[9]}]
##62
set_property PACKAGE_PIN F19 [get_ports {FrontIO_B[6]}]
##63
set_property PACKAGE_PIN G18 [get_ports {FrontIO_C[8]}]
##64
set_property PACKAGE_PIN F20 [get_ports {FrontIO_B[7]}]

##67
set_property PACKAGE_PIN G19 [get_ports {FrontIO_C[7]}]
##68
set_property PACKAGE_PIN J20 [get_ports {FrontIO_B[8]}]
##69
set_property PACKAGE_PIN G20 [get_ports {FrontIO_C[6]}]
##70
set_property PACKAGE_PIN H20 [get_ports {FrontIO_B[9]}]

##73
set_property PACKAGE_PIN K14 [get_ports {FrontIO_C[5]}]
##74
set_property PACKAGE_PIN H15 [get_ports {FrontIO_B[10]}]
##75
set_property PACKAGE_PIN J14 [get_ports {FrontIO_C[4]}]
##76
set_property PACKAGE_PIN G15 [get_ports {FrontIO_B[11]}]

##81
set_property PACKAGE_PIN N15 [get_ports {FrontIO_C[3]}]
##82
set_property PACKAGE_PIN L14 [get_ports {FrontIO_B[12]}]
##83
set_property PACKAGE_PIN N16 [get_ports {FrontIO_C[2]}]
##84
set_property PACKAGE_PIN L15 [get_ports {FrontIO_B[13]}]

##87
set_property PACKAGE_PIN M14 [get_ports {FrontIO_C[1]}]
##88
set_property PACKAGE_PIN K16 [get_ports {FrontIO_B[14]}]
##89
set_property PACKAGE_PIN M15 [get_ports {FrontIO_C[0]}]
##90
set_property PACKAGE_PIN J16 [get_ports {FrontIO_B[15]}]

##93
set_property PACKAGE_PIN Y12 [get_ports Tx_En]
##94
set_property PACKAGE_PIN V11 [get_ports {Tx_D[0]}]
##95
set_property PACKAGE_PIN Y13 [get_ports {Tx_D[1]}]
##96
set_property PACKAGE_PIN V10 [get_ports MDC]

##97
set_property PACKAGE_PIN V6 [get_ports MDIO]
##99
set_property PACKAGE_PIN W6 [get_ports {PTPTrig[4]}]

set_property IOSTANDARD LVCMOS33 [get_ports CRS]
set_property IOSTANDARD LVCMOS33 [get_ports MD*]
set_property IOSTANDARD LVCMOS33 [get_ports PTP*]
set_property IOSTANDARD LVCMOS33 [get_ports Rx_Clk*]
set_property IOSTANDARD LVCMOS33 [get_ports Rx_Er*]
set_property IOSTANDARD LVCMOS33 [get_ports Rx_D*]
set_property IOSTANDARD LVCMOS33 [get_ports SCL]
set_property IOSTANDARD LVCMOS33 [get_ports SDA]
set_property IOSTANDARD LVCMOS33 [get_ports Tx_En*]
set_property IOSTANDARD LVCMOS33 [get_ports Tx_D*]
set_property IOSTANDARD LVCMOS33 [get_ports Control*]
set_property IOSTANDARD LVCMOS33 [get_ports EB_Ctrl*]
set_property IOSTANDARD LVCMOS33 [get_ports EB_Data*]
set_property IOSTANDARD LVCMOS33 [get_ports FrontIO*]
set_property IOSTANDARD LVCMOS33 [get_ports TriggerAll*]




