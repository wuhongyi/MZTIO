/*----------------------------------------------------------------------
 * Copyright (c) 2017 XIA LLC
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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/file.h>


// need to compile with -lm option

#include "PixieNetDefs.h"
#include "PixieNetCommon.h"
#include "PixieNetConfig.h"


int main(void) {

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  int k, addr;


  // ******************* read ini file and fill struct with values ********************


  
  PixieNetFippiConfig fippiconfig;		// struct holding the input parameters
  const char *defaults_file = "defaults.ini";
  int rval = init_PixieNetFippiConfig_from_file( defaults_file, 0, &fippiconfig );   // first load defaults, do not allow missing parameters
  if( rval != 0 )
  {
    printf( "Failed to parse FPGA settings from %s, rval=%d\n", defaults_file, rval );
    return rval;
  }
  const char *settings_file = "settings.ini";
  rval = init_PixieNetFippiConfig_from_file( settings_file, 1, &fippiconfig );   // second override with user settings, do allow missing
  if( rval != 0 )
  {
    printf( "Failed to parse FPGA settings from %s, rval=%d\n", settings_file, rval );
    return rval;
  }

  unsigned int  mval, dac;
  unsigned int CW, FR, SL[NCHANNELS], SG[NCHANNELS], FL[NCHANNELS], FG[NCHANNELS], TH[NCHANNELS];
  unsigned int PSAM, PSEP, TL[NCHANNELS], TD[NCHANNELS], GW[NCHANNELS], GD[NCHANNELS];
  unsigned int QDCL0[NCHANNELS], QDCL1[NCHANNELS], QDCD0[NCHANNELS], QDCD1[NCHANNELS];
  unsigned int gain[NCHANNELS*2], saveR2[NCHANNELS], i2cdata[8];


  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0) {
    perror("Failed to open devfile");
    return 1;
  }

  //Lock the PL address space so multiple programs cant step on eachother.
  if( flock( fd, LOCK_EX | LOCK_NB ) )
  {
    printf( "Failed to get file lock on /dev/uio0\n" );
    return 1;
  }
  
  map_addr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (map_addr == MAP_FAILED) {
    perror("Failed to mmap");
    return 1;
  }

  mapped = (unsigned int *) map_addr;




  // ******************* XIA code begins ********************
  // first, set CSR run control options   
  mapped[ACSRIN] = 0x0000; // all off
  mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block


  // take struct values, convert to FPGA units, write to register, one by one
  // error/dependency check: returns -xxyy, 
  // with xx = parameter number (line) in file and yy = channel number (0 for module)

  // ********** SYSTEM PARAMETERS ******************
    if(fippiconfig.NUMBER_CHANNELS != NCHANNELS) {
      printf("Invalid NUMBER_CHANNELS = %d, should be %d\n",fippiconfig.NUMBER_CHANNELS,NCHANNELS);
      return -100;
    }

  if(fippiconfig.C_CONTROL > 65535) {
      printf("Invalid C_CONTROL = %d, and actually currently unused\n",fippiconfig.C_CONTROL);
      return -200;
    }
  if(fippiconfig.REQ_RUNTIME < 5.0) {
      printf("Invalid REQ_RUNTIME = %f, please increase\n",fippiconfig.REQ_RUNTIME);
      return -300;
    }
  
  if(fippiconfig.POLL_TIME < MIN_POLL_TIME) {
      printf("Invalid POLL_TIME = %d, please increase to more than %d\n",fippiconfig.POLL_TIME,MIN_POLL_TIME);
      return -400;
    }
  
  // ********** MODULE PARAMETERS ******************

    //MODULE_CSRA
    if(fippiconfig.MODULE_CSRA > 65535) {
      printf("Invalid MODULE_CSRA = 0x%x\n",fippiconfig.MODULE_CSRA);
      return -1700;
    }

    //MODULE_CSRB
    if(fippiconfig.MODULE_CSRB > 65535) {
      printf("Invalid MODULE_CSRB = 0x%x\n",fippiconfig.MODULE_CSRB);
      return -1800;
    }

    // COINCIDENCE_PATTERN
    if(fippiconfig.COINCIDENCE_PATTERN > 65535) {
      printf("Invalid COINCIDENCE_PATTERN = 0x%x\n",fippiconfig.COINCIDENCE_PATTERN);
      return -1900;
    }


    // COINCIDENCE_WINDOW
    CW = (int)floorf(fippiconfig.COINCIDENCE_WINDOW*SYSTEM_CLOCK_MHZ);       // multiply time in us *  # ticks per us = time in ticks
    if( (CW > MAX_CW) | (CW < MIN_CW) ) {
      printf("Invalid COINCIDENCE_WINDOW = %f, must be between %f and %f us\n",fippiconfig.COINCIDENCE_WINDOW, (double)MIN_CW/SYSTEM_CLOCK_MHZ, (double)MAX_CW/SYSTEM_CLOCK_MHZ);
      return -2000;
    }

    // RUN_TYPE
    if( !( (fippiconfig.RUN_TYPE == 0x301)  ||
           (fippiconfig.RUN_TYPE == 0x400)  ||
           (fippiconfig.RUN_TYPE == 0x402)  ||
           (fippiconfig.RUN_TYPE == 0x500)  ||
           (fippiconfig.RUN_TYPE == 0x501)  ||
           (fippiconfig.RUN_TYPE == 0x502)  ||
           (fippiconfig.RUN_TYPE == 0x503)   ) ) {
      printf("Invalid RUN_TYPE = 0x%x, please check manual for a list of supported run types\n",fippiconfig.RUN_TYPE);
      return -2100;
    }

    mval = fippiconfig.COINCIDENCE_PATTERN;
     
     if( (fippiconfig.RUN_TYPE == 0x503)  || 
         (fippiconfig.RUN_TYPE == 0x402)   )     // set LM402 bit
         mval = mval | 0x10000;

     if( (fippiconfig.RUN_TYPE == 0x501)  || 
         (fippiconfig.RUN_TYPE == 0x502)  ||
         (fippiconfig.RUN_TYPE == 0x503)  || 
         (fippiconfig.RUN_TYPE == 0x301)   )     // set MCA/notrace bit
         mval = mval | 0x20000;

    if(fippiconfig.MODULE_CSRA & 0x0001)   // test bit 0 (CWGROUP)
        mval = mval | 0x40000;             // set CWGROUP bit
         

     mapped[ACOINCPATTERN] = mval;
     if(mapped[ACOINCPATTERN] != mval) printf("Error writing value COINCIDENCE_PATTERN register\n");

    //  printf("CoincPattern = 0x%x, \n",mval);

    // FILTER_RANGE
    FR = fippiconfig.FILTER_RANGE;
    if( (FR > MAX_FR) | (FR < MIN_FR) ) {
      printf("Invalid FILTER_RANGE = %d, must be between %d and %d\n",FR,MIN_FR, MAX_FR);
      return -2200;
    }

    // ACCEPT_PATTERN
    if(fippiconfig.ACCEPT_PATTERN != 0x0020 ) {
      printf("Invalid ACCEPT_PATTERN = 0x%x, currently fixed to 0x20\n",fippiconfig.ACCEPT_PATTERN);
      return -2300;
    }

    // SYNC_AT_START
    if(fippiconfig.SYNC_AT_START >1) {
      printf("Invalid SYNC_AT_START = %d, can only be 0 and 1\n",fippiconfig.SYNC_AT_START);
      return -2400;
    }

    // HV DAC
    mval = (int)floor( (fippiconfig.HV_DAC/5.0) * 65535);		// map 0..5V range to 0..64K	
    if(mval > 65535) {
         printf("Invalid HV_DAC = %f, can only be between 0 and 5V\n",fippiconfig.HV_DAC);
         return -2500;
    }
    mapped[AHVDAC] = mval;
    if(mapped[AHVDAC] != mval) printf("Error writing to HV_DAC register\n");
    usleep(DACWAIT);		// wait for programming
    mapped[AHVDAC] = mval;     // repeat, sometimes doesn't take?
    if(mapped[AHVDAC] != mval) printf("Error writing to HV_DAC register\n");
    usleep(DACWAIT);
  
  
  // Offboard serial:
    if(fippiconfig.SERIAL_IO > 65535) {
      printf("Invalid SERIAL_IO = 0x%x\n",fippiconfig.SERIAL_IO);
      return -2600;
    }
    mapped[ASERIALIO] = fippiconfig.SERIAL_IO;
    if(mapped[ASERIALIO] != fippiconfig.SERIAL_IO) printf("Error writing to SERIAL_IO register\n");
    usleep(DACWAIT);		// wait for programming

  // AUX CTRL:
    if(fippiconfig.AUX_CTRL > 65535) {
      printf("Invalid AUX_CTRL = 0x%x\n",fippiconfig.AUX_CTRL);
      return -2700;
    }
    mapped[AAUXCTRL] = fippiconfig.AUX_CTRL;
    if(mapped[AAUXCTRL] != fippiconfig.AUX_CTRL) printf("Error writing AUX_CTRL register\n");


  // ********** CHANNEL PARAMETERS ******************
   
  // CCSRA-C :  R0
  for( k = 0; k < NCHANNELS; k ++ )
  {
      if(fippiconfig.CHANNEL_CSRA[k] > 65535) {
         printf("Invalid CHANNEL_CSRA = 0x%x\n",fippiconfig.CHANNEL_CSRA[k]);
         return -3300-k;
      } 
      if(fippiconfig.CHANNEL_CSRC[k] > 65535) {
         printf("Invalid CHANNEL_CSRC = 0x%x\n",fippiconfig.CHANNEL_CSRC[k]);
         return -3500-k;
      }  
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each
      mval = (fippiconfig.CHANNEL_CSRA[k] + (fippiconfig.CHANNEL_CSRC[k] << 16));
      mapped[addr+0] = mval;
      if(mapped[addr+0] != mval) printf("Error writing to CHANNEL_CSR register\n");

   //   printf("CHANNEL_CSRA[%d] = 0x%x, addr %d\n",k,mval & 0xFFFF, addr);
  }

  // energy filter : R1
    for( k = 0; k < NCHANNELS; k ++ )
    {
      SL[k] = (int)floorf(fippiconfig.ENERGY_RISETIME[k] * FILTER_CLOCK_MHZ);
      SL[k] = SL[k] >> FR;
      if(SL[k] <MIN_SL) {
         printf("Invalid ENERGY_RISETIME = %f, minimum %f us at this filter range\n",fippiconfig.ENERGY_RISETIME[k],(double)((MIN_SL<<FR)/FILTER_CLOCK_MHZ));
         return -3600-k;
      } 
      SG[k] = (int)floorf(fippiconfig.ENERGY_FLATTOP[k] * FILTER_CLOCK_MHZ);
      SG[k] = SG[k] >> FR;
      if(SG[k] <MIN_SG) {
         printf("Invalid ENERGY_FLATTOP = %f, minimum %f us at this filter range\n",fippiconfig.ENERGY_FLATTOP[k],(double)((MIN_SG<<FR)/FILTER_CLOCK_MHZ));
         return -3700-k;
      } 
      if( (SL[k]+SG[k]) >MAX_SLSG) {
         printf("Invalid combined energy filter, maximum %f us at this filter range\n",(double)((MAX_SLSG<<FR)/FILTER_CLOCK_MHZ));
         return -3700-k;
      } 
      mval = SL[k]-1;
      mval = mval + ((SL[k]+SG[k]-1)     <<  8);
      mval = mval + ((SG[k]-1)           << 16);
      mval = mval + (((2*SL[k]+SG[k])/4) << 24);
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each
      mapped[addr+1] = mval;
      if(mapped[addr+1] != mval) printf("Error writing parameters to ENERGY_FILTER register\n");
    }

  // trigger filter : R2
    for( k = 0; k < NCHANNELS; k ++ )
    {
      FL[k] = (int)floorf(fippiconfig.TRIGGER_RISETIME[k] * FILTER_CLOCK_MHZ);
      if(FL[k] <MIN_FL) {
         printf("Invalid TRIGGER_RISETIME = %f, minimum %f us\n",fippiconfig.TRIGGER_RISETIME[k],(double)(MIN_FL/FILTER_CLOCK_MHZ));
         return -3800-k;
      } 
      FG[k] = (int)floorf(fippiconfig.TRIGGER_FLATTOP[k] * FILTER_CLOCK_MHZ);
      if(FG[k] <MIN_FL) {
         printf("Invalid TRIGGER_FLATTOP = %f, minimum %f us\n",fippiconfig.TRIGGER_FLATTOP[k],(double)(MIN_FG/FILTER_CLOCK_MHZ));
         return -3900-k;
      } 
      if( (FL[k]+FG[k]) >MAX_FLFG) {
         printf("Invalid combined trigger filter, maximum %f us\n",(double)(MAX_FLFG/FILTER_CLOCK_MHZ));
         return -3900-k;
      } 
      TH[k] = (int)floor(fippiconfig.TRIGGER_THRESHOLD[k]*FL[k]/2.0);
      if(TH[k] > MAX_TH)     {
         printf("Invalid TRIGGER_THRESHOLD = %f, maximum %f at this trigger filter rise time\n",fippiconfig.TRIGGER_THRESHOLD[k],MAX_TH*8.0/(double)FL[k]);
         return -4000-k;
      } 
      mval = FL[k]-1;
      mval = mval + ((FL[k]+FG[k]-1) <<  8);
      mval = mval + ((TH[k])         << 14);
      mval = mval + ((FR)            << 26);
      saveR2[k] = mval;
      mval = mval + ( 1              << 31); 
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each
      mapped[addr+2] = mval;
      if(mapped[addr+2] != mval) printf("Error writing parameters to trigger filter register\n");
    }

   // gain, Efilter2: R3
   // current version only has 2 gains: 2 and 5. applied via I2C below, only save bit pattern here
   for( k = 0; k < NCHANNELS; k ++ )
   {
        if( !( (fippiconfig.ANALOG_GAIN[k] == GAIN_HIGH)  ||
               (fippiconfig.ANALOG_GAIN[k] == GAIN_LOW)   ) ) {
        printf("ANALOG_GAIN = %f not matching available gains exactly, rounding to nearest\n",fippiconfig.ANALOG_GAIN[k]);
    }

      if(fippiconfig.ANALOG_GAIN[k] > (GAIN_HIGH+GAIN_LOW)/2 ) {
         gain[2*k+1] = 1;      // 2'b10 = gain 5
         gain[2*k]   = 0;   
      }
      else  {
        gain[2*k+1] = 0;      
        gain[2*k]   = 1;      // 2'b01 = gain 2
      }
      PSAM = SL[k]+SG[k]-5;       
      PSEP = (PSAM+6) * (1 << FR);
      PSEP = 8192 - PSEP;
      mval = PSAM;
      mval = mval + (PSEP        << 13);
      mval = mval + (gain[2*k]   << 26);
      mval = mval + (gain[2*k+1] << 27);
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each
      mapped[addr+3] = mval;
      if(mapped[addr+3] != mval) printf("Error writing parameters to gain register\n");
      // no limits for DIG_GAIN
   }

   // DAC : R4 
   for( k = 0; k < NCHANNELS; k ++ )
   {
      dac = (int)floor( (1 - fippiconfig.VOFFSET[k]/ V_OFFSET_MAX) * 32768);	
      if(dac > 65535)  {
         printf("Invalid VOFFSET = %f, must be between %f and -%f\n",fippiconfig.VOFFSET[k], V_OFFSET_MAX-0.05, V_OFFSET_MAX-0.05);
         return -4300-k;
      }
      mval=dac;
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each
      mapped[addr+4] = mval;
      if(mapped[addr+4] != mval) printf("Error writing parameters to DAC register\n");
      usleep(DACWAIT);		// wait for programming
      mapped[addr+4] = mval;     // repeat, sometimes doesn't take?
      if(mapped[addr+4] != mval) printf("Error writing parameters to DAC register\n");
      usleep(DACWAIT);     
 //     printf("DAC %d, value 0x%x (%d), [%f V] \n",k, dac, dac,fippiconfig.VOFFSET[k]);
   }


   // waveforms: R5, R6
   for( k = 0; k < NCHANNELS; k ++ )
   {
      TL[k] = BLOCKSIZE_400*(int)floor(fippiconfig.TRACE_LENGTH[k]*ADC_CLK_MHZ/BLOCKSIZE_400);       // multiply time in us *  # ticks per us = time in ticks; must be multiple of BLOCKSIZE_400
      if(TL[k] > MAX_TL)  {
         printf("Invalid TRACE_LENGTH = %f, maximum %f us\n",fippiconfig.TRACE_LENGTH[k],(double)MAX_TL/ADC_CLK_MHZ);
         return -4400-k;
      }
      if(TL[k] <fippiconfig.TRACE_LENGTH[k]*ADC_CLK_MHZ)  {
         printf("TRACE_LENGTH[%d] will be rounded off to = %f us, %d samples\n",k,(double)TL[k]/ADC_CLK_MHZ,TL[k]);
      }
      TD[k] = (int)floor(fippiconfig.TRACE_DELAY[k]*ADC_CLK_MHZ);       // multiply time in us *  # ticks per us = time in ticks
      if(TD[k] > MAX_TL-TWEAK_UD)  {
         printf("Invalid TRACE_DELAY = %f, maximum %f us\n",fippiconfig.TRACE_DELAY[k],(double)(MAX_TL-TWEAK_UD)/ADC_CLK_MHZ);
         return -4500-k;
      }
    }

    if( (fippiconfig.RUN_TYPE == 0x500)  || 
        (fippiconfig.RUN_TYPE == 0x400)  ||
        (fippiconfig.RUN_TYPE == 0x402)   )     // issue warning if TL not same 
    {
        mval = 1;
        for( k = 1; k < NCHANNELS; k ++ )
        { 
            if(TL[k] != TL[0]) mval =0;
        }
        if(mval==0) printf("WARNING: TRACE_LENGTHs must be the same for all channels for current LM file parser to work");      
    }

   // check limits on a few parameters that are not part of R5,6 but traditionally in this order in the settings file
   for( k = 0; k < NCHANNELS; k ++ )
   {
      if(fippiconfig.BINFACTOR[k] > MAX_BFACT)     {
         printf("Invalid BINFACTOR = %d, maximum %d\n",fippiconfig.BINFACTOR[k],MAX_BFACT);
         return -4800-k;
      } 
      if(fippiconfig.TAU[k] <0)  {
         printf("Invalid TAU = %f, must be positive\n",fippiconfig.TAU[k]);
         return -4900-k;
      }
      // no limit on BLCUT
      if(fippiconfig.XDT[k] <0)  {
         printf("Invalid XDT = %f, must be positive\n",fippiconfig.XDT[k]);
         return -5000-k;
      }
      if(fippiconfig.BASELINE_PERCENT[k] <0)  {
         printf("Invalid BASELINE_PERCENT = %f, must be positive\n",fippiconfig.BASELINE_PERCENT[k]);
         return -5100-k;
      }
   }

   for( k = 0; k < NCHANNELS; k ++ )
   {
      if(fippiconfig.PSA_THRESHOLD[k] > MAX_PSATH) {
         printf("Invalid PSA_THRESHOLD = %d, maximum %d\n",fippiconfig.PSA_THRESHOLD[k],MAX_PSATH);
         return -5300-k;                                                       
      }
      mval = (TD[k]+TWEAK_UD)/4;           // add tweak to accomodate trigger pipelining delay
      mval = mval + (TL[k]>0)*(1<<29);     // set bit 29 if TL is not zero
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each
      mapped[addr+5] = mval;
      if(mapped[addr+5] != mval) printf("Error writing parameters to TRACE1 register");
   
      mval = TL[k]/4;
      mval = mval + (CW       <<  16);
      mval = mval + (fippiconfig.PSA_THRESHOLD[k]  <<  24);  // 
      mapped[addr+6] = mval;
      if(mapped[addr+6] != mval) printf("Error writing parameters to TRACE2 register");
   }

   // INTEGRATOR: check only
   for( k = 0; k < NCHANNELS; k ++ )
   {
      if(fippiconfig.INTEGRATOR[k] > 2)     {
         printf("Invalid INTEGRATOR = %d, maximum %d\n",fippiconfig.INTEGRATOR[k],2);
         return -5400-k;
      } 
   }

    // gate: R7
   for( k = 0; k < NCHANNELS; k ++ )
   {
      GW[k] = (int)floor( fippiconfig.GATE_WINDOW[k] * FILTER_CLOCK_MHZ);
      if(GW[k] > MAX_GW) {
         printf("Invalid GATE_WINDOW = %f, maximum %d us\n",fippiconfig.GATE_WINDOW[k],MAX_GW/FILTER_CLOCK_MHZ);
         return -5500-k;
      }
      GD[k] = (int)floor( fippiconfig.GATE_DELAY[k] * FILTER_CLOCK_MHZ);
      if(GD[k] > MAX_GD) {
         printf("Invalid GATE_DELAY = %f, maximum %d us\n",fippiconfig.GATE_DELAY[k],MAX_GD/FILTER_CLOCK_MHZ);
         return -5600-k;
      }
      mval = GW[k];
      mval = mval + (GD[k]    <<  8);
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each     
      mapped[addr+7] = mval;
      if(mapped[addr+7] != mval) printf("Error writing parameters to GATE register");
   }

   // ADC serial I/O: R8
   // currently not used

   // coincdelay: R9
   for( k = 0; k < NCHANNELS; k ++ )
   {
      mval = (int)floor( fippiconfig.COINC_DELAY[k] * ADC_CLK_MHZ);    
      if(mval > MAX_CD) {
         printf("Invalid COINC_DELAY = %f, maximum %d us\n",fippiconfig.COINC_DELAY[k],MAX_CD/ADC_CLK_MHZ);
         return -5700-k;
      }

      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each     
      mapped[addr+9] = mval;
      if(mapped[addr+9] != mval) printf("Error writing parameters to COINC_DELAY register");
   }

    // BLAVG: check only
   for( k = 0; k < NCHANNELS; k ++ )
   {
      if(fippiconfig.BLAVG[k] > 65535)     {
         printf("Invalid BLAVG = %d, maximum %d\n",fippiconfig.BLAVG[k],65535);
         return -5800-k;
      } 
      if( (fippiconfig.BLAVG[k] >0) && (fippiconfig.BLAVG[k] < 65535-MAX_BLAVG ) )     {
         printf("Invalid BLAVG = %d, minimum %d (or zero to turn off)\n",fippiconfig.BLAVG[k],65535-MAX_BLAVG);
         return -5800-k;
      } 
   }

   // PSA: R10
   for( k = 0; k < NCHANNELS; k ++ )
   { 
      if(fippiconfig.QDC0_LENGTH[k] > MAX_QDCL)    {
         printf("Invalid QDC0_LENGTH = %d, maximum %d samples \n",fippiconfig.QDC0_LENGTH[k],MAX_QDCL);
         return -5900-k;
      } 
      if(fippiconfig.QDC0_LENGTH[k]+fippiconfig.QDC0_DELAY[k] > MAX_QDCLD)    {
         printf("Invalid QDC0_DELAY = %d, maximum length plus delay %d samples \n",fippiconfig.QDC0_DELAY[k],MAX_QDCLD);
         return -6000-k;
      } 
      if(fippiconfig.QDC1_LENGTH[k] > MAX_QDCL)    {
         printf("Invalid QDC1_LENGTH = %d, maximum %d samples \n",fippiconfig.QDC1_LENGTH[k],MAX_QDCL);
         return -6100-k;
      } 
      if(fippiconfig.QDC1_LENGTH[k]+fippiconfig.QDC1_DELAY[k] > MAX_QDCLD)    {
         printf("Invalid QDC1_DELAY = %d, maximum length plus delay %d samples \n",fippiconfig.QDC1_DELAY[k],MAX_QDCLD);
         return -6200-k;
      } 
      if(fippiconfig.QDC0_LENGTH[k]+fippiconfig.QDC0_DELAY[k] > fippiconfig.QDC1_LENGTH[k]+fippiconfig.QDC1_DELAY[k])    {
         printf("Invalid QDC1_DELAY/_LENGTH; must finish later than QDC0 \n");
         return -6300-k;
      } 
      if( (fippiconfig.QDC0_LENGTH[k] & 0x0001) == 1)    {
         printf("NOTE: QDC0_LENGTH is an odd number, rounding down to nearest even number \n");
      } 
      if( (fippiconfig.QDC1_LENGTH[k] & 0x0001) == 1)    {
         printf("NOTE: QDC1_LENGTH is an odd number, rounding down to nearest even number \n");
      } 
      if( ( (fippiconfig.QDC0_DELAY[k] - fippiconfig.QDC1_DELAY[k]) & 0x0001) == 1)    {
         printf("NOTE: QDC_DELAYs are NOT both even or both odd, using last bit of QDC0_DELAY \n");
      }                                            

      // 250 MHz implementation works on 2 samples at a time, so divide by 2
      QDCL0[k] = (int)floorf( fippiconfig.QDC0_LENGTH[k]/2.0)+1;  
      QDCD0[k] = fippiconfig.QDC0_DELAY[k] + QDCL0[k]*2 +2;
      QDCL1[k] = (int)floorf( fippiconfig.QDC1_LENGTH[k]/2.0)+1;
      QDCD1[k] = fippiconfig.QDC1_DELAY[k] + QDCL1[k]*2 +2;
      mval = QDCL0[k];
      mval = mval + (QDCD0[k] <<  7);
      mval = mval + (QDCL1[k] <<  16);
      mval = mval + (QDCD1[k] <<  23);
      mval = mval + (1<<15);     // set bit 15 for 2x correction for QDC0 (always)
      mval = mval + (1<<31);     // set bit 31 for 2x correction for QDC1 (always)
      if( fippiconfig.QDC_DIV8[k])  {
         mval = mval | (1<<5);      // set bits to divide result by 8
         mval = mval | (1<<21);
      }

      // optional division by 8 of output sums not implemented, controlled by MCSRB bits
      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each     
      mapped[addr+10] = mval;
      if(mapped[addr+10] != mval) printf("Error writing parameters to QDC register");
   }

       // MCA2D_SCALEX/Y, PSA_NG_THRESHOLD: check only
   for( k = 0; k < NCHANNELS; k ++ )
   {
      if( (fippiconfig.MCA2D_SCALEX[k] > (65535/MCA2D_BINS)) || (fippiconfig.MCA2D_SCALEX[k] <0) )    {
         // must be positive, and at most its a 64K number spread over MCA2D_BINS
         printf("Invalid MCA2D_SCALEX = %f, maximum %d\n",fippiconfig.MCA2D_SCALEX[k],(65535/MCA2D_BINS));
         return -6400-k;
      } 

      if( (fippiconfig.MCA2D_SCALEY[k] > (65535/MCA2D_BINS)) || (fippiconfig.MCA2D_SCALEY[k] <0) )    {
         // must be positive, and at most its a 64K number spread over MCA2D_BINS
         printf("Invalid MCA2D_SCALEY = %f, maximum %d\n",fippiconfig.MCA2D_SCALEY[k],(65535/MCA2D_BINS));
         return -6500-k;
      } 
       
      if( fippiconfig.PSA_NG_THRESHOLD[k] <0) {
         // must be positive
         printf("Invalid PSA_NG_THRESHOLD = %f, must be positive\n",fippiconfig.PSA_NG_THRESHOLD[k]);
         return -6600-k;
      } 
   }

    // ADC AVG: R11
   for( k = 0; k < NCHANNELS; k ++ )
   { 
      if(fippiconfig.ADC_AVG[k] > MAX_AVG_ADC)    {
         printf("Invalid ADC_AVG = %d, maximum %d samples \n",fippiconfig.ADC_AVG[k],MAX_AVG_ADC);
         return -6700-k;
      }  
      if(fippiconfig.THRESH_ADC_AVG[k] > MAX_TH_AVG_ADC)    {
         printf("Invalid THRESH_ADC_AVG = %d, maximum %d samples \n",fippiconfig.THRESH_ADC_AVG[k],MAX_TH_AVG_ADC);
         return -6800-k;
      } 
      if(fippiconfig.ADC_AVG[k] < MIN_AVG_ADC)    {
         printf("Invalid ADC_AVG = %d, minimum %d samples \n",fippiconfig.ADC_AVG[k],MIN_AVG_ADC);
         return -6900-k;
      }  
      if(fippiconfig.THRESH_ADC_AVG[k] < MIN_TH_AVG_ADC)    {
         printf("Invalid THRESH_ADC_AVG = %d, minimum %d samples \n",fippiconfig.THRESH_ADC_AVG[k],MIN_TH_AVG_ADC);
         return -7000-k;
      } 
      if(fippiconfig.ADC_AVG[k] > 2*(int)floor(fippiconfig.ADC_AVG[k]/2) )    {     
         printf("Invalid ADC_AVG = %d, must be multiple of 2 \n",fippiconfig.THRESH_ADC_AVG[k]);
         return -7100-k;
      } 

      mval = (int)floor(fippiconfig.ADC_AVG[k]/2) - 1;          // number of samples to average
      mval = mval + (fippiconfig.THRESH_ADC_AVG[k] <<  16);     // trigger threshold
      if(fippiconfig.ADC_AVG[k] > 2048)
         mval = mval + (1<< 31); 
      else  if(fippiconfig.ADC_AVG[k] > 64)
         mval = mval + (1<< 30); 
      else
          mval = mval + (1<< 29); 

      addr = N_PL_IN_PAR+k*N_PL_IN_PAR;   // channel registers begin after NPLPAR system registers, NPLPAR each     
      mapped[addr+11] = mval;
      if(mapped[addr+11] != mval) printf("Error writing parameters to ADC AVG register");
   }
   

   // restart/initialize filters 
   usleep(100);      // wait for filter FIFOs to clear, really should be longest SL+SG
   for( k = 0; k < NCHANNELS; k ++ )
   {
        addr = 16+k*16;
        mapped[addr+2] = saveR2[k];       // restart filters with the halt bit in R2 set to zero
   }
   usleep(100);      // really should be longest SL+SG
   mapped[ADSP_CLR] = 1;
   mapped[ARTC_CLR] = 1;


   // ************************ I2C programming *********************************
   // gain and termination applied across all channels via FPGA's I2C
   // TODO
   // I2C connects to gain enables, termination relays, thermometer, PROM (with s/n etc), optional external

    // ---------------------- program gains -----------------------

   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 0;   // A2
   i2cdata[2] = 1;   // A1
   i2cdata[1] = 0;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = gain[k];
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

      // ---------------------- program termination -----------------------

   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 0;   // A2
   i2cdata[2] = 0;   // A1
   i2cdata[1] = 1;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   // settings taken from MCSRB
   i2cdata[7] = (fippiconfig.MODULE_CSRB & 0x0080) >> 7 ;    // power down ADC driver D, NYI
   i2cdata[6] = (fippiconfig.MODULE_CSRB & 0x0040) >> 6 ;    // power down ADC driver C, NYI
   i2cdata[5] = (fippiconfig.MODULE_CSRB & 0x0020) >> 5 ;    // power down ADC driver B, NYI
   i2cdata[4] = (fippiconfig.MODULE_CSRB & 0x0010) >> 4 ;    // power down ADC driver A, NYI
   i2cdata[3] = (fippiconfig.MODULE_CSRB & 0x0008) >> 3 ;    //unused
   i2cdata[2] = (fippiconfig.MODULE_CSRB & 0x0004) >> 2 ;    // term. CD
   i2cdata[1] = (fippiconfig.MODULE_CSRB & 0x0002) >> 1 ;    // term. AB
   i2cdata[0] = (fippiconfig.MODULE_CSRB & 0x0001)      ;    //unused
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

  
   // ************************ end I2C *****************************************

   // ADC board temperature
    printf("ADC board temperature: %d C \n",(int)board_temperature(mapped) );

   // ***** ZYNQ temperature
     printf("Zynq temperature: %d C \n",(int)zynq_temperature() );

   // ***** check HW info *********
   k = hwinfo(mapped);
   printf("Revision %04X, Serial Number %d \n",(k>>16) & 0xFFFF, k & 0xFFFF);
   if(k==0) printf("WARNING: HW may be incompatible with this SW/FW \n");

 
 // clean up  
 flock( fd, LOCK_UN );
 munmap(map_addr, size);
 close(fd);
 return 0;
}










