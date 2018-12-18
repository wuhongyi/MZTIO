#

# IODELAY group for GMII-to-RGMII block
#set_property IODELAY_GROUP tri_mode_ethernet_mac_iodelay_grp1 [get_cells *_i/gmii_to_rgmii_0/U0/i_*_gmii_to_rgmii_0_0_idelayctrl]

# Rename the gmii_to_rgmii_0_gmii_clk_125m_out clock to gmii_clk_125m_out so that the in-built constraints will find it
# Based on AR57197: http://www.xilinx.com/support/answers/57197.html
#create_clock -period 8.000 -name gmii_clk_125m_out [get_pins *_i/gmii_to_rgmii_0/U0/i_*_gmii_to_rgmii_0_0_clocking/mmcm_adv_inst/CLKOUT0]

