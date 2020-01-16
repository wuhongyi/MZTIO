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
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <math.h>
// need to compile with -lm option

#include "PixieNetDefs.h"
#include "PixieNetCommon.h"


int main(void) {

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  int k, addr, ch, adc, dac;
  unsigned int mval;

  unsigned int mins[NCHANNELS] = {4192,4192,4192,4192};
  unsigned int mint[NCHANNELS] = {4192,4192,4192,4192};
  unsigned int readdr[NCHANNELS] = {AADC0,AADC1,AADC2,AADC3};
  unsigned int targetdac[NCHANNELS] = {0,0,0,0};
  unsigned int targetBL[NCHANNELS] = {400,400,400,400};     // TODO: BL% read from ini file, compute 
  double dacadj;
  unsigned int oldadc, adcchanged, saveaux;



  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0) {
    perror("Failed to open devfile");
    return 1;
  }

  map_addr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (map_addr == MAP_FAILED) {
    perror("Failed to mmap");
    return 1;
  }

  mapped = (unsigned int *) map_addr;


  // ******************* XIA code begins ********************

  mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
  saveaux = mapped[AAUXCTRL];
  mapped[AAUXCTRL] = 0;    // turn off pulser, other stuff
  mapped[ACSRIN] = 0x0000; // all off

  // ----------- swap channels 0<>1 and 2<>3 if necessary  -------------
  printf("Checking for swapped channels ...\n");
  
  mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
  for( ch = 0; ch < NCHANNELS; ch ++ )
  {
      // find if change of DAC changes ADC
     adc = 0;
     oldadc = 0;
     adcchanged = 0;
     addr = N_PL_IN_PAR+ch*N_PL_IN_PAR+CA_DAC;   // channel registers begin after NPLPAR system registers, NPLPAR each
     dac = 0; 
     k=0;

     // scan through DAC settings
     do {
         mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
         mapped[addr] = dac;
         usleep(DACWAIT);
         mapped[addr] = dac; //;     //TODO: double write required?
         if(mapped[addr] != dac) printf("Error writing parameters to DAC register\n");
         usleep(DACSETTLE);		// wait for DAC's RC filter

         mapped[AOUTBLOCK] = OB_EVREG;		// switch reads to event data block of addresses
         adc = (mapped[readdr[ch]] & 0xFFFF);  // dummy read to refresh read register
         adc = (mapped[readdr[ch]] & 0xFFFF);
         if (k==0)   {
            oldadc = adc;
         } else {
            if ( abs(oldadc-adc)>200)  adcchanged = 1;  // look for a change > 200 steps. Not foolproof with pulses!
         }
         k=k+1;
         dac = dac+4096;
      //   printf("Channel %u: DAC value %u, adc %u, adcdiff %d\n",ch,dac,adc,abs(oldadc-adc));
     } while ( (adcchanged==0) & (k<16) );//  dac loop

     // check if there was a change, if not, swap channels
     if (k==16)  {
         if (ch<2)  {
            mval =  mapped[AADCCTRL];
            mval = mval ^ 0x0001;
            mapped[AADCCTRL] = mval;	  // swap 0/1
         } else {
            mval =  mapped[AADCCTRL];
            mval = mval ^ 0x0002;
            mapped[AADCCTRL] = mval;	  // swap 2/3
         }
         printf("Channel %u: ADC values does not change with DAC. Swapped channel inputs\n",ch);
     }

  } // endfor  channels
  
  
  // ----------- need to have correct polarity  -------------

  // TODO!



  // ----------- adjust offset: search for two DAC settings with valid ADC response, then extrapolate  -------
  printf("Adjusting DC offsets (correct polarity required) ...\n");

  for( ch = 0; ch < NCHANNELS; ch ++ )
  {
     dac = 0;
     adc = 0;
     addr = N_PL_IN_PAR+ch*N_PL_IN_PAR+CA_DAC;   // channel registers begin after NPLPAR system registers, NPLPAR each
     k=0;

     // 1. find first DAC value with valid response
     do  {
         mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
         mapped[addr] = dac; //dac;
         usleep(DACWAIT);
         mapped[addr] = dac; //;     //TODO: double write required?
         if(mapped[addr] != dac) printf("Error writing parameters to DAC register\n");
         usleep(DACSETTLE);		// wait for DAC's RC filter

         mapped[AOUTBLOCK] = OB_EVREG;		// switch reads to event data block of addresses
         adc = (mapped[readdr[ch]] & 0xFFFF);  // dummy read to refresh read register
         adc = (mapped[readdr[ch]] & 0xFFFF);
         //for( i = 0; i < NTRACE_SAMPLES; i ++ )
         //      adc0[i] = (mapped[readdr[ch]] & 0xFFFF);

         //printf("Channel %u: addr 0x%x, DAC value %u, adc %u\n",ch,addr,dac,adc);
         dac = dac + 2048;
         k=k+1;
     } while ( ((adc>4000) | (adc<100)) & (dac < 65536)  );    //& (k<33)
     //printf("Channel %u: DAC value %u, adc %u\n",ch,dac,adc);
     dac = dac - 2048;               // dac is now the lowest valid DAC value
  


     // 2. get min/max of many samples
     for( k = 0; k < NTRACE_SAMPLES; k ++ )   {
         adc = (mapped[readdr[ch]] & 0xFFFF);
         //avg[ch] = avg[ch]+ adc/NTRACE_SAMPLES;   // find average
         if (adc < mins[ch])  mins[ch] = adc;    // find min
         //if (adc > maxs[ch])  maxs[ch] = adc;    // find max
     }
    // printf("Channel %u: DAC value %u, min adc read %u\n",ch,dac,mins[ch]);

     // 3. change DAC settings
     mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
     dac = dac + 1024;               // new, second dac
     mapped[addr] = dac;
     usleep(DACWAIT);		// wait for programming
     mapped[addr] = dac;     //TODO: double write required?
     usleep(DACSETTLE);		// wait for programming

     // 4. get min/max of many samples
     mapped[AOUTBLOCK] = OB_EVREG;		// switch reads to event data block of addresses
     adc = (mapped[readdr[ch]] & 0xFFFF);  // dummy read to refresh read register
      for( k = 0; k < NTRACE_SAMPLES; k ++ )   {
         adc = (mapped[readdr[ch]] & 0xFFFF);
        // avg[ch] = avg[ch]+ adc/NTRACE_SAMPLES;   // find average
         if (adc < mint[ch])  mint[ch] = adc;    // find min
        // if (adc > maxs[ch])  maxs[ch] = adc;    // find max
     }
     //printf("Channel %u: DAC value %u, min adc read %u\n",ch,dac,mint[ch]);

     // 5. compute target dac from 2 points
     dacadj =  1024.0 * ((double)targetBL[ch] - (double)mint[ch]) / ((double)mint[ch] - (double)mins[ch]);
     dac = dac + (int)floor(dacadj);
     if( (dac>0) & (dac<65536) )
     {
        //printf("Channel %u: DAC adjustment %f\n",ch,dacadj  );
        targetdac[ch]  = dac;
     } else {
          printf("Channel %u: could not find target DAC value\n",ch);
     }
      
   }    // endfor

   // 6. set all channels to target and report voltages
   for( ch = 0; ch < NCHANNELS; ch ++ )
   {
        addr = N_PL_IN_PAR+ch*N_PL_IN_PAR+CA_DAC;   // channel registers begin after NPLPAR system registers, NPLPAR each
        
        mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
        dac = targetdac[ch];	
        mapped[addr] = dac;
        usleep(DACWAIT);		// wait for programming
        mapped[addr] = dac;     //TODO: double write required?
        usleep(DACSETTLE);		// wait for programming

        mapped[AOUTBLOCK] = OB_EVREG;		// switch reads to event data block of addresses
        adc = (mapped[readdr[ch]] & 0xFFFF);  // dummy read to refresh read register
        adc = (mapped[readdr[ch]] & 0xFFFF);  
        printf("Channel %u: DAC value %u, offset %fV, ADC %u\n",ch,dac,V_OFFSET_MAX*(1.0-(double)dac/32678.0), adc);
   }

   mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block
   mapped[AAUXCTRL] = saveaux;    // turn on pulser TODO: read from file!
    
 
 // clean up  
 munmap(map_addr, size);
 close(fd);
 return 0;
}







