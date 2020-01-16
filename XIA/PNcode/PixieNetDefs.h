/*----------------------------------------------------------------------
 * Copyright (c) 2018 XIA LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided 
 * that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above 
 *     copyright notice, this list of conditions and the 
 *     following disclaimer.
 *   * Redistributions in binary form must reproduce the 
 *     above copyright notice, this list of conditions and the 
 *     following disclaimer in the documentation and/or other 
 *     materials provided with the distribution.
 *   * Neither the name of XIA LLC
 *     nor the names of its contributors may be used to endorse 
 *     or promote products derived from this software without 
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *----------------------------------------------------------------------*/

// system constants
#define PS_CODE_VERSION 0x0220
#define PN_BOARD_VERSION_12_250_A 0xA990    
#define PN_BOARD_VERSION_12_250_B 0xA991  
#define PN_BOARD_VERSION_12_250_B_PTP 0xA981  
#define ADC_CLK_MHZ 250
#define SYSTEM_CLOCK_MHZ 125
#define FILTER_CLOCK_MHZ 125
#define NCHANNELS 4
#define V_OFFSET_MAX			1.25			// Pixie voltage offset maximum
#define MAX_MCA_BINS       32768
#define WEB_MCA_BINS       4096
#define MCA2D_BINS       100 // in each dimension
#define WEB_LOGEBIN        3
#define DACWAIT 20  // usleep cycles to wait for DAC programming
#define DACSETTLE 80000  // usleep cycles to wait for DAC stable output after filter
#define NTRACE_SAMPLES 8192
#define NAVG_TRACE_SAMPLES 4096
#define TWOTO32   4294967296
#define ICRSCALE 15        // factor between current iCR read and ICR in cps

// Limits for settings
#define MIN_CW 5             // Coinc Window limits
#define MAX_CW 511
#define MIN_FR 1             // FR limits
#define MAX_FR 6
#define MIN_SL 2             // energy filter limits
#define MIN_SG 3
#define MAX_SLSG 126
#define MIN_FL 2             // trigger filter limits
#define MIN_FG 3
#define MAX_FLFG 63
#define MAX_TH 1023
#define GAIN_HIGH 5          // gain limits
#define GAIN_LOW 2
#define MAX_TL 4092           // max length of captured waveform and pre-trigger delay
#define TWEAK_UD 28           // adjustment to pre-trigger delay for internal pipelining
#define MAX_BFACT 16
#define MAX_PSATH 2044
#define MAX_GW 255
#define MAX_GD 255
#define MAX_CD 255
#define MAX_QDCL  60          // length of QDC sum samples
#define MAX_QDCLD 250         // length plus delay of QDC sum, in samples
#define MAX_BLAVG 10
#define MAX_BADBL 20
#define MIN_AVG_ADC 2
#define MAX_AVG_ADC 65535
#define MIN_TH_AVG_ADC 1
#define MAX_TH_AVG_ADC 4095



// system reg addr defines
// block 0
#define ACSRIN        0x000
#define ACOINCPATTERN 0x001
#define AI2CREG       0x002
#define AOUTBLOCK     0x003
#define AHVDAC        0x004
#define ASERIALIO     0x005
#define AAUXCTRL      0x006
#define AADCCTRL      0x007
#define ADSP_CLR      0x008
#define ACOUNTER_CLR  0x009
#define ARTC_CLR      0x00A
#define ABVAL         0x00B
#define CA_DAC        0x004

// block 1
#define ACSROUT       0x100
#define AEVSTATS      0x101
#define ABRDINFO      0x102
#define APPSTIME      0x103
#define AEVHIT        0x104
#define AEVTSL        0x105
#define AEVTSH        0x106
#define AEVPPS        0x107
#define AEVPPS        0x107
#define AADCTRIG      0x108



// block 2
#define ARS0_MOD      0x200
#define AREALTIME     0x201

// channel reg addr defines
// block 1
// channel independent lower bits of event registers
#define CA_HIT			0x100
#define CA_TSL			0x101
#define CA_TSH		   0x102
#define CA_PSAA		0x103
#define CA_PSAB		0x104
#define CA_CFDA		0x105
#define CA_CFDB		0x106
#define CA_LSUM		0x107
#define CA_TSUM		0x108
#define CA_GSUM		0x109
#define CA_REJECT		0x10A
#define CA_LSUMB		0x10B
#define CA_TSUMB		0x10C
#define CA_GSUMB		0x10D
// ADC registers
#define AADC0        0x11F
#define AADC1        0x12F
#define AADC2        0x13F
#define AADC3        0x14F
// averaged ADC registers
#define AAVGADC0        0x11E
#define AAVGADC1        0x12E
#define AAVGADC2        0x13E
#define AAVGADC3        0x14E
//block 2
#define ARS0_CH0     0x220
#define ARS0_CH1     0x240
#define ARS0_CH2     0x260
#define ARS0_CH3     0x280
// block 3
#define AWF0         0x300
#define AWF1         0x301
#define AWF2         0x302
#define AWF3         0x303

// outblocks
#define OB_IOREG     0x0			// I/O
#define OB_EVREG     0x1			// Event data
#define OB_RSREG     0x2			// run statistics
#define OB_WFREG     0x3			// channel waveforms



// program control constants
#define LINESZ                1024  // max number of characters in ini file line
#define I2CWAIT               4     // us between I2C clock toggles
#define SDA                   1     // bit definitions for I2C I/O
#define SCL                   2     // bit definitions for I2C I/O
#define SDAENA                4     // bit definitions for I2C I/O
#define N_PL_IN_PAR           16    // number of input parameters for system and each channel
#define N_PL_RS_PAR           32    // number of runstats parameters for system and each channel
#define N_USED_RS_PAR         20    // not all RS parapmeters are used, can save some readout and printout cycles
#define MAX_PAR_NAME_LENGTH   65    // Maximum length of parameter names
#define BLREADPERIOD          20
#define MIN_POLL_TIME         100
#define BLOCKSIZE_400         32    // waveform block size (# 16bit words) in run type 0x400
#define FILE_HEAD_LENGTH_400  32    // file header size (# 16bit words) in run type 0x400
#define CHAN_HEAD_LENGTH_400  32    // event/channel header size (# 16bit words) in run type 0x400
#define WATERMARK     0x12345678    // for LM QC routine
#define EORMARK       0x01000002    // End Of Run

// channel hit pattern & info in LM data
#define HIT_ACCEPT            5     //  result of local coincidence test & pileup & veto & rangebad
#define HIT_COINCTEST         16    //  result of local coincidence test
#define HIT_PILEUP            18    //  result of local pileup test
#define HIT_LOCALHIT          20    //  set if this channel has a hit
#define HIT_OOR               22    //  set if this channel had the out of range flag set


