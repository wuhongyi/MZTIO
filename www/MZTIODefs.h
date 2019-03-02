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
#define PS_CODE_VERSION 0x1000
#define BOARD_VERSION_A 0xB100    
#define SYSTEM_CLOCK_MHZ 100
#define NCHANNELS 4
#define DACWAIT 20            // usleep cycles to wait for DAC programming
#define DACSETTLE 80000       // usleep cycles to wait for DAC stable output after filter
#define TWOTO32   4294967296

// Limits for settings
#define MAX_NBITS_MOST 16
#define MAX_NBITS_TA 32
#define MAX_FOR_BITPATTERN16 65535
#define MAX_FOR_BITPATTERN32 4294967296  
#define MAX_FOR_OUTSELECT 8


// system reg addr defines
// io block (0) local control
// write
#define ACSRIN        0x000
#define AI2CREG       0x002
#define AOUTBLOCK     0x003
#define ACOUNTER_CLR  0x009
#define ABVAL         0x006
// read
#define ABRDINFO      0x102


// io block (0) trigger control
// starting address of blocks with similar function
#define AOUTENA         0x100
#define ACOINCMASK      0x108
#define AMULTMASK       0x110
#define ACOINCPATTERN   0x118
#define AMULTTHRESHOLD  0x120
#define AOUTSELECT      0x128
// and the offsets within a block
#define AOFFFA  0
#define AOFFFB  1
#define AOFFFC  2
#define AOFFTA  3
#define AOFFEB  4

// output block (1) local results
#define ACSROUT       0x000
#define ARS_LOC       0x000
#define ARS_TIO       0x100

// outblocks
#define OB_IOREG     0x0			// I/O
#define OB_EVREG     0x1			// output data


// program control constants
#define LINESZ                1024  // max number of characters in ini file line
#define I2CWAIT               4     // us between I2C clock toggles
#define SDA                   1     // bit definitions for I2C I/O
#define SCL                   2     // bit definitions for I2C I/O
#define SDAENA                4     // bit definitions for I2C I/O
#define N_PL_IN_PAR           16    // number of input parameters for system and each channel
#define N_PL_RS_PAR           32    // number of runstats parameters for system and each channel
#define N_USED_RS_PAR         19    // not all RS parapmeters are used, can save some readout and printout cycles
#define MAX_PAR_NAME_LENGTH   65    // Maximum length of parameter names
#define BLREADPERIOD          20
#define MIN_POLL_TIME         100
#define FPGAOUT_IS_OFF        0
#define LVDS_IS_INPUT         0

