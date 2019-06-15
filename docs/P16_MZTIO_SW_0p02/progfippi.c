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
  int k, lbit, fbit;
  long long int revsn;


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

  //unsigned int  mval, CW;
  unsigned int i2cdata[8];


  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0) {
    perror("Failed to open devfile");
    return -1;
  }

  //Lock the PL address space so multiple programs cant step on eachother.
  if( flock( fd, LOCK_EX | LOCK_NB ) )
  {
    printf( "Failed to get file lock on /dev/uio0\n" );
    return -2;
  }
  
  map_addr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (map_addr == MAP_FAILED) {
    perror("Failed to mmap");
    return -3;
  }

  mapped = (unsigned int *) map_addr;




  // ******************* Main code begins ********************
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
  
  // ********** LOCAL_CONTROL PARAMETERS ******************

    //LOCAL_CONTROL_00
    if(fippiconfig.LOCAL_CONTROL_00 > 65535) {
      printf("Invalid LOCAL_CONTROL_00 = 0x%x\n",fippiconfig.LOCAL_CONTROL_00);
      return -500;
    }


  // ********** TRIGGER CONTROL PARAMETERS ******************

  // set outputs of FPGA to LVDS buffers as tristate before programming the buffers' direction via I2C (below)
   mapped[AOUTENA+AOFFFA] =  FPGAOUT_IS_OFF;     // write to FPGA, 
   mapped[AOUTENA+AOFFFB] =  FPGAOUT_IS_OFF;     // write to FPGA, 
   mapped[AOUTENA+AOFFFC] =  FPGAOUT_IS_OFF;     // write to FPGA, 



  // ------ Output enables -------
    //FRONT_A_OUTENA
    if(fippiconfig.FRONT_A_OUTENA > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_A_OUTENA = 0x%x\n",fippiconfig.FRONT_A_OUTENA);
      return -600;
    }
    // delay write until after I2C programming
    // no conversion to int necessary
 
    //FRONT_B_OUTENA
    if(fippiconfig.FRONT_B_OUTENA > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_B_OUTENA = 0x%x\n",fippiconfig.FRONT_B_OUTENA);
      return -700;
    }
    // delay write until after I2C programming
    // no conversion to int necessary

    //FRONT_C_OUTENA
    if(fippiconfig.FRONT_C_OUTENA > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_C_OUTENA = 0x%x\n",fippiconfig.FRONT_C_OUTENA);
      return -800;                             
    }
    // delay write until after I2C programming
    // no conversion to int necessary


    //LVDS_A_OUTENA
    if(fippiconfig.LVDS_A_OUTENA > MAX_FOR_BITPATTERN16) {
      printf("Invalid LVDS_A_OUTENA = 0x%x\n",fippiconfig.LVDS_A_OUTENA);
      return -600;
    }
    // delay write until after I2C programming
    // no conversion to int necessary
 
    //LVDS_B_OUTENA
    if(fippiconfig.LVDS_B_OUTENA > MAX_FOR_BITPATTERN16) {
      printf("Invalid LVDS_B_OUTENA = 0x%x\n",fippiconfig.LVDS_B_OUTENA);
      return -700;
    }
    // delay write until after I2C programming
    // no conversion to int necessary

    //LVDS_C_OUTENA
    if(fippiconfig.LVDS_C_OUTENA > MAX_FOR_BITPATTERN16) {
      printf("Invalid LVDS_C_OUTENA = 0x%x\n",fippiconfig.LVDS_C_OUTENA);
      return -800;
    }

    // check if there is a conflict: LVDS out - FPGA out: ok, pass though FPGA -> front
    //     out = 1                   LVDS out - FPGA in: ok, signal from DB -> FPGA and front
    //     in =  0                   LVDS in  - FPGA out: not allowed, conflict
    //                               LVDS in  - FPGA in: ok, pass though front -> FPGA
    for( k = 0; k <16; k++ )     // check each bit
    {
       lbit = (fippiconfig.LVDS_A_OUTENA  >> k ) & 0x0001;
       fbit = (fippiconfig.FRONT_A_OUTENA >> k ) & 0x0001;
       if(lbit==1 && fbit==0)  printf("FRONT/LVDS_A_OUTENA settings (bit %d) expect input from daughtercard\n",k); 
       if(lbit==0 && fbit==1)  
       {
         printf("FRONT/LVDS_A output settings (bit %d) create a conflict\n",k); 
         return -600;
       }

       lbit = (fippiconfig.LVDS_B_OUTENA  >> k ) & 0x0001;
       fbit = (fippiconfig.FRONT_B_OUTENA >> k ) & 0x0001;
       if(lbit==1 && fbit==0)  printf("FRONT/LVDS_B_OUTENA settings (bit %d) expect input from daughtercard\n",k); 
       if(lbit==0 && fbit==1)  
       {
         printf("FRONT/LVDS_B output settings (bit %d) create a conflict\n",k); 
         return -700;
       }

       lbit = (fippiconfig.LVDS_C_OUTENA  >> k ) & 0x0001;
       fbit = (fippiconfig.FRONT_C_OUTENA >> k ) & 0x0001;
       if(lbit==1 && fbit==0)  printf("FRONT/LVDS_C_OUTENA settings (bit %d) expect input from daughtercard\n",k); 
       if(lbit==0 && fbit==1)  
       {
         printf("FRONT/LVDS_C output settings (bit %d) create a conflict\n",k); 
         return -800;
       }
       // other conditions ok
    }

    //TRIGGERALL_OUTENA
    if(fippiconfig.TRIGGERALL_OUTENA > 0 ) { //MAX_FOR_BITPATTERN32) {
      printf("Invalid TRIGGERALL_OUTENA = 0x%x, for now use backplane only as input\n",fippiconfig.TRIGGERALL_OUTENA);
      return -900;
    }
    // no conversion to int necessary
    mapped[AOUTENA+AOFFTA] =  fippiconfig.TRIGGERALL_OUTENA;     // write to FPGA, then read back to verify
    if(mapped[AOUTENA+AOFFTA] != fippiconfig.TRIGGERALL_OUTENA) printf("Error writing TRIGGERALL_OUTENA register\n");

    //EBDATA_OUTENA
    if(fippiconfig.EBDATA_OUTENA > 0 ) { //MAX_FOR_BITPATTERN16) {
      printf("Invalid EBDATA_OUTENA = 0x%x, for now use backplane only as input\n",fippiconfig.EBDATA_OUTENA);
      return -1000;
    }
    // no conversion to int necessary
    mapped[AOUTENA+AOFFEB] =  fippiconfig.EBDATA_OUTENA;     // write to FOGA, then read back to verify
    if(mapped[AOUTENA+AOFFEB] != fippiconfig.EBDATA_OUTENA) printf("Error writing EBDATA_OUTENA register\n");


    // ------ Coincidence Mask -------

        //FRONT_A_COINC_MASK
    if(fippiconfig.FRONT_A_COINC_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_A_COINC_MASK = 0x%x\n",fippiconfig.FRONT_A_COINC_MASK);
      return -1100;
    }
    // no conversion to int necessary
    mapped[ACOINCMASK+AOFFFA] =  fippiconfig.FRONT_A_COINC_MASK;     // write to FPGA, then read back to verify
    if(mapped[ACOINCMASK+AOFFFA] != fippiconfig.FRONT_A_COINC_MASK) printf("Error writing FRONT_A_COINC_MASK register\n");

 
    //FRONT_B_COINC_MASK
    if(fippiconfig.FRONT_B_COINC_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_B_COINC_MASK = 0x%x\n",fippiconfig.FRONT_B_COINC_MASK);
      return -1200;
    }
    // no conversion to int necessary
    mapped[ACOINCMASK+AOFFFB] =  fippiconfig.FRONT_B_COINC_MASK;     // write to FPGA, then read back to verify
    if(mapped[ACOINCMASK+AOFFFB] != fippiconfig.FRONT_B_COINC_MASK) printf("Error writing FRONT_B_COINC_MASK register\n");



    //FRONT_C_COINC_MASK
    if(fippiconfig.FRONT_C_COINC_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_C_COINC_MASK = 0x%x\n",fippiconfig.FRONT_C_COINC_MASK);
      return -1300;
    }
    // no conversion to int necessary
    mapped[ACOINCMASK+AOFFFC] =  fippiconfig.FRONT_C_COINC_MASK;     // write to FPGA, then read back to verify
    if(mapped[ACOINCMASK+AOFFFC] != fippiconfig.FRONT_C_COINC_MASK) printf("Error writing FRONT_C_COINC_MASK register\n");


    //TRIGGERALL_COINC_MASK
    if(fippiconfig.TRIGGERALL_COINC_MASK > MAX_FOR_BITPATTERN32) {
      printf("Invalid TRIGGERALL_COINC_MASK = 0x%x \n",fippiconfig.TRIGGERALL_COINC_MASK);
      return -1400;
    }
    // no conversion to int necessary
    mapped[ACOINCMASK+AOFFTA] =  fippiconfig.TRIGGERALL_COINC_MASK;     // write to FPGA, then read back to verify
    if(mapped[ACOINCMASK+AOFFTA] != fippiconfig.TRIGGERALL_COINC_MASK) printf("Error writing TRIGGERALL_COINC_MASK register\n");

    //EBDATA_COINC_MASK
    if(fippiconfig.EBDATA_COINC_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid EBDATA_COINC_MASK = 0x%x\n",fippiconfig.EBDATA_COINC_MASK);
      return -1500;
    }
    // no conversion to int necessary
    mapped[ACOINCMASK+AOFFEB] =  fippiconfig.EBDATA_COINC_MASK;     // write to FOGA, then read back to verify
    if(mapped[ACOINCMASK+AOFFEB] != fippiconfig.EBDATA_COINC_MASK) printf("Error writing EBDATA_COINC_MASK register\n");


       // ------ Multiplicity Mask -------

        //FRONT_A_MULT_MASK
    if(fippiconfig.FRONT_A_MULT_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_A_MULT_MASK = 0x%x\n",fippiconfig.FRONT_A_MULT_MASK);
      return -1600;
    }
    // no conversion to int necessary
    mapped[AMULTMASK+AOFFFA] =  fippiconfig.FRONT_A_MULT_MASK;     // write to FPGA, then read back to verify
    if(mapped[AMULTMASK+AOFFFA] != fippiconfig.FRONT_A_MULT_MASK) printf("Error writing FRONT_A_MULT_MASK register\n");

 
    //FRONT_B_MULT_MASK
    if(fippiconfig.FRONT_B_MULT_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_B_MULT_MASK = 0x%x\n",fippiconfig.FRONT_B_MULT_MASK);
      return -1700;
    }
    // no conversion to int necessary
    mapped[AMULTMASK+AOFFFB] =  fippiconfig.FRONT_B_MULT_MASK;     // write to FPGA, then read back to verify
    if(mapped[AMULTMASK+AOFFFB] != fippiconfig.FRONT_B_MULT_MASK) printf("Error writing FRONT_B_MULT_MASK register\n");



    //FRONT_C_MULT_MASK
    if(fippiconfig.FRONT_C_MULT_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_C_MULT_MASK = 0x%x\n",fippiconfig.FRONT_C_MULT_MASK);
      return -1800;
    }
    // no conversion to int necessary
    mapped[AMULTMASK+AOFFFC] =  fippiconfig.FRONT_C_MULT_MASK;     // write to FPGA, then read back to verify
    if(mapped[AMULTMASK+AOFFFC] != fippiconfig.FRONT_C_MULT_MASK) printf("Error writing FRONT_C_MULT_MASK register\n");


    //TRIGGERALL_MULT_MASK
    if(fippiconfig.TRIGGERALL_MULT_MASK > MAX_FOR_BITPATTERN32) {
      printf("Invalid TRIGGERALL_MULT_MASK = 0x%x \n",fippiconfig.TRIGGERALL_MULT_MASK);
      return -1900;
    }
    // no conversion to int necessary
    mapped[AMULTMASK+AOFFTA] =  fippiconfig.TRIGGERALL_MULT_MASK;     // write to FPGA, then read back to verify
    if(mapped[AMULTMASK+AOFFTA] != fippiconfig.TRIGGERALL_MULT_MASK) printf("Error writing TRIGGERALL_MULT_MASK register\n");

    //EBDATA_MULT_MASK
    if(fippiconfig.EBDATA_MULT_MASK > MAX_FOR_BITPATTERN16) {
      printf("Invalid EBDATA_MULT_MASK = 0x%x \n",fippiconfig.EBDATA_MULT_MASK);
      return -2000;
    }
    // no conversion to int necessary
    mapped[AMULTMASK+AOFFEB] =  fippiconfig.EBDATA_MULT_MASK;     // write to FOGA, then read back to verify
    if(mapped[AMULTMASK+AOFFEB] != fippiconfig.EBDATA_MULT_MASK) printf("Error writing EBDATA_MULT_MASK register\n");


        // ------ COINC_PATTERN -------

        //FRONT_A_COINC_PATTERN
    if(fippiconfig.FRONT_A_COINC_PATTERN > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_A_COINC_PATTERN = 0x%x\n",fippiconfig.FRONT_A_COINC_PATTERN);
      return -2100;
    }
    // no conversion to int necessary
    mapped[ACOINCPATTERN+AOFFFA] =  fippiconfig.FRONT_A_COINC_PATTERN;     // write to FPGA, then read back to verify
    if(mapped[ACOINCPATTERN+AOFFFA] != fippiconfig.FRONT_A_COINC_PATTERN) printf("Error writing FRONT_A_COINC_PATTERN register\n");

 
    //FRONT_B_COINC_PATTERN
    if(fippiconfig.FRONT_B_COINC_PATTERN > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_B_COINC_PATTERN = 0x%x\n",fippiconfig.FRONT_B_COINC_PATTERN);
      return -2200;
    }
    // no conversion to int necessary
    mapped[ACOINCPATTERN+AOFFFB] =  fippiconfig.FRONT_B_COINC_PATTERN;     // write to FPGA, then read back to verify
    if(mapped[ACOINCPATTERN+AOFFFB] != fippiconfig.FRONT_B_COINC_PATTERN) printf("Error writing FRONT_B_COINC_PATTERN register\n");



    //FRONT_C_COINC_PATTERN
    if(fippiconfig.FRONT_C_COINC_PATTERN > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_C_COINC_PATTERN = 0x%x\n",fippiconfig.FRONT_C_COINC_PATTERN);
      return -2300;
    }
    // no conversion to int necessary
    mapped[ACOINCPATTERN+AOFFFC] =  fippiconfig.FRONT_C_COINC_PATTERN;     // write to FPGA, then read back to verify
    if(mapped[ACOINCPATTERN+AOFFFC] != fippiconfig.FRONT_C_COINC_PATTERN) printf("Error writing FRONT_C_COINC_PATTERN register\n");


    //TRIGGERALL_COINC_PATTERN
    if(fippiconfig.TRIGGERALL_COINC_PATTERN > MAX_FOR_BITPATTERN32) {
      printf("Invalid TRIGGERALL_COINC_PATTERN = 0x%x \n",fippiconfig.TRIGGERALL_COINC_PATTERN);
      return -2400;
    }
    // no conversion to int necessary
    mapped[ACOINCPATTERN+AOFFTA] =  fippiconfig.TRIGGERALL_COINC_PATTERN;     // write to FPGA, then read back to verify
    if(mapped[ACOINCPATTERN+AOFFTA] != fippiconfig.TRIGGERALL_COINC_PATTERN) printf("Error writing TRIGGERALL_COINC_PATTERN register\n");

    //EBDATA_COINC_PATTERN
    if(fippiconfig.EBDATA_COINC_PATTERN > MAX_FOR_BITPATTERN16) {
      printf("Invalid EBDATA_COINC_PATTERN = 0x%x \n",fippiconfig.EBDATA_COINC_PATTERN);
      return -2500;
    }
    // no conversion to int necessary
    mapped[ACOINCPATTERN+AOFFEB] =  fippiconfig.EBDATA_COINC_PATTERN;     // write to FOGA, then read back to verify
    if(mapped[ACOINCPATTERN+AOFFEB] != fippiconfig.EBDATA_COINC_PATTERN) printf("Error writing EBDATA_COINC_PATTERN register\n");


          // ------ MULT_THRESHOLD -------

        //FRONT_A_MULT_THRESHOLD
    if(fippiconfig.FRONT_A_MULT_THRESHOLD > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_A_MULT_THRESHOLD = 0x%x\n",fippiconfig.FRONT_A_MULT_THRESHOLD);
      return -2600;
    }
    // no conversion to int necessary
    mapped[AMULTTHRESHOLD+AOFFFA] =  fippiconfig.FRONT_A_MULT_THRESHOLD;     // write to FPGA, then read back to verify
    if(mapped[AMULTTHRESHOLD+AOFFFA] != fippiconfig.FRONT_A_MULT_THRESHOLD) printf("Error writing FRONT_A_MULT_THRESHOLD register\n");

 
    //FRONT_B_MULT_THRESHOLD
    if(fippiconfig.FRONT_B_MULT_THRESHOLD > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_B_MULT_THRESHOLD = 0x%x\n",fippiconfig.FRONT_B_MULT_THRESHOLD);
      return -2700;
    }
    // no conversion to int necessary
    mapped[AMULTTHRESHOLD+AOFFFB] =  fippiconfig.FRONT_B_MULT_THRESHOLD;     // write to FPGA, then read back to verify
    if(mapped[AMULTTHRESHOLD+AOFFFB] != fippiconfig.FRONT_B_MULT_THRESHOLD) printf("Error writing FRONT_B_MULT_THRESHOLD register\n");



    //FRONT_C_MULT_THRESHOLD
    if(fippiconfig.FRONT_C_MULT_THRESHOLD > MAX_FOR_BITPATTERN16) {
      printf("Invalid FRONT_C_MULT_THRESHOLD = 0x%x\n",fippiconfig.FRONT_C_MULT_THRESHOLD);
      return -2800;
    }
    // no conversion to int necessary
    mapped[AMULTTHRESHOLD+AOFFFC] =  fippiconfig.FRONT_C_MULT_THRESHOLD;     // write to FPGA, then read back to verify
    if(mapped[AMULTTHRESHOLD+AOFFFC] != fippiconfig.FRONT_C_MULT_THRESHOLD) printf("Error writing FRONT_C_MULT_THRESHOLD register\n");


    //TRIGGERALL_MULT_THRESHOLD
    if(fippiconfig.TRIGGERALL_MULT_THRESHOLD > MAX_FOR_BITPATTERN32) {
      printf("Invalid TRIGGERALL_MULT_THRESHOLD = 0x%x \n",fippiconfig.TRIGGERALL_MULT_THRESHOLD);
      return -2900;
    }
    // no conversion to int necessary
    mapped[AMULTTHRESHOLD+AOFFTA] =  fippiconfig.TRIGGERALL_MULT_THRESHOLD;     // write to FPGA, then read back to verify
    if(mapped[AMULTTHRESHOLD+AOFFTA] != fippiconfig.TRIGGERALL_MULT_THRESHOLD) printf("Error writing TRIGGERALL_MULT_THRESHOLD register\n");

    //EBDATA_MULT_THRESHOLD
    if(fippiconfig.EBDATA_MULT_THRESHOLD > MAX_FOR_BITPATTERN16) {
      printf("Invalid EBDATA_MULT_THRESHOLD = 0x%x \n",fippiconfig.EBDATA_MULT_THRESHOLD);
      return -3000;
    }
    // no conversion to int necessary
    mapped[AMULTTHRESHOLD+AOFFEB] =  fippiconfig.EBDATA_MULT_THRESHOLD;     // write to FOGA, then read back to verify
    if(mapped[AMULTTHRESHOLD+AOFFEB] != fippiconfig.EBDATA_MULT_THRESHOLD) printf("Error writing EBDATA_MULT_THRESHOLD register\n");


              // ------ OUTPUT_SELECT -------

        //FRONT_A_OUTPUT_SELECT
    if(fippiconfig.FRONT_A_OUTPUT_SELECT > MAX_FOR_OUTSELECT) {
      printf("Invalid FRONT_A_OUTPUT_SELECT = 0x%x\n",fippiconfig.FRONT_A_OUTPUT_SELECT);
      return -3100;
    }
    // no conversion to int necessary
    mapped[AOUTSELECT+AOFFFA] =  fippiconfig.FRONT_A_OUTPUT_SELECT;     // write to FPGA, then read back to verify
    if(mapped[AOUTSELECT+AOFFFA] != fippiconfig.FRONT_A_OUTPUT_SELECT) printf("Error writing FRONT_A_OUTPUT_SELECT register\n");

 
    //FRONT_B_OUTPUT_SELECT
    if(fippiconfig.FRONT_B_OUTPUT_SELECT > MAX_FOR_OUTSELECT) {
      printf("Invalid FRONT_B_OUTPUT_SELECT = 0x%x\n",fippiconfig.FRONT_B_OUTPUT_SELECT);
      return -3200;
    }
    // no conversion to int necessary
    mapped[AOUTSELECT+AOFFFB] =  fippiconfig.FRONT_B_OUTPUT_SELECT;     // write to FPGA, then read back to verify
    if(mapped[AOUTSELECT+AOFFFB] != fippiconfig.FRONT_B_OUTPUT_SELECT) printf("Error writing FRONT_B_OUTPUT_SELECT register\n");



    //FRONT_C_OUTPUT_SELECT
    if(fippiconfig.FRONT_C_OUTPUT_SELECT > MAX_FOR_OUTSELECT) {
      printf("Invalid FRONT_C_OUTPUT_SELECT = 0x%x\n",fippiconfig.FRONT_C_OUTPUT_SELECT);
      return -3300;
    }
    // no conversion to int necessary
    mapped[AOUTSELECT+AOFFFC] =  fippiconfig.FRONT_C_OUTPUT_SELECT;     // write to FPGA, then read back to verify
    if(mapped[AOUTSELECT+AOFFFC] != fippiconfig.FRONT_C_OUTPUT_SELECT) printf("Error writing FRONT_C_OUTPUT_SELECT register\n");


    //TRIGGERALL_OUTPUT_SELECT
    if(fippiconfig.TRIGGERALL_OUTPUT_SELECT > MAX_FOR_OUTSELECT) {
      printf("Invalid TRIGGERALL_OUTPUT_SELECT = 0x%x \n",fippiconfig.TRIGGERALL_OUTPUT_SELECT);
      return -3400;
    }
    // no conversion to int necessary
    mapped[AOUTSELECT+AOFFTA] =  fippiconfig.TRIGGERALL_OUTPUT_SELECT;     // write to FPGA, then read back to verify
    if(mapped[AOUTSELECT+AOFFTA] != fippiconfig.TRIGGERALL_OUTPUT_SELECT) printf("Error writing TRIGGERALL_OUTPUT_SELECT register\n");

    //EBDATA_OUTPUT_SELECT
    if(fippiconfig.EBDATA_OUTPUT_SELECT > MAX_FOR_OUTSELECT) {
      printf("Invalid EBDATA_OUTPUT_SELECT = 0x%x \n",fippiconfig.EBDATA_OUTPUT_SELECT);
      return -3500;
    }
    // no conversion to int necessary
    mapped[AOUTSELECT+AOFFEB] =  fippiconfig.EBDATA_OUTPUT_SELECT;     // write to FOGA, then read back to verify
    if(mapped[AOUTSELECT+AOFFEB] != fippiconfig.EBDATA_OUTPUT_SELECT) printf("Error writing EBDATA_OUTPUT_SELECT register\n");



   // ************************ I2C programming *********************************

   // LVDS buffer direction for FRONT A-C is applied via FPGA's I2C
   // another device on the I2C bus is the thermometer / 4-byte memory

    // ---------------------- program LVDS input vs output  -----------------------

    // lower A
   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 1;   // A2       // Front IO A, addresses [A2:A0] =  000, 100
   i2cdata[2] = 0;   // A1
   i2cdata[1] = 0;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = ( fippiconfig.LVDS_A_OUTENA >> k ) & 0x0001 ;
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

    // upper A   
   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 0;   // A2       // Front IO A, addresses [A2:A0] =  000, 100
   i2cdata[2] = 0;   // A1
   i2cdata[1] = 0;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = ( fippiconfig.LVDS_A_OUTENA >> (k+8) ) & 0x0001 ;
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);


    // lower B
   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 1;   // A2       // Front IO B, addresses [A2:A0] =  001, 101
   i2cdata[2] = 0;   // A1
   i2cdata[1] = 1;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = ( fippiconfig.LVDS_B_OUTENA >> k ) & 0x0001 ;
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

    // upper B  
   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 0;   // A2       // Front IO B, addresses [A2:A0] =  001, 101
   i2cdata[2] = 0;   // A1
   i2cdata[1] = 1;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = ( fippiconfig.LVDS_B_OUTENA >> (k+8) ) & 0x0001 ;
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

   
    // lower C
   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 1;   // A2       // Front IO C, addresses [A2:A0] =  010, 110
   i2cdata[2] = 1;   // A1
   i2cdata[1] = 0;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = ( fippiconfig.LVDS_C_OUTENA >> k ) & 0x0001 ;
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

    // upper C   
   I2Cstart(mapped);

   // I2C addr byte
   i2cdata[7] = 0;
   i2cdata[6] = 1;
   i2cdata[5] = 0;
   i2cdata[4] = 0;
   i2cdata[3] = 0;   // A2       // Front IO C, addresses [A2:A0] =  010, 110
   i2cdata[2] = 1;   // A1
   i2cdata[1] = 0;   // A0
   i2cdata[0] = 0;   // R/W*
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
   {
      i2cdata[k] = ( fippiconfig.LVDS_C_OUTENA >> (k+8) ) & 0x0001 ;
   }
   I2Cbytesend(mapped, i2cdata);
   I2Cslaveack(mapped);

   // I2C data byte
   I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
   I2Cslaveack(mapped);

   I2Cstop(mapped);

  
   // ************************ end I2C *****************************************

   // now enable FPGA front panel outputs

    mapped[AOUTENA+AOFFFA] =  fippiconfig.FRONT_A_OUTENA;     // write to FPGA, then read back to verify
    if(mapped[AOUTENA+AOFFFA] != fippiconfig.FRONT_A_OUTENA) printf("Error writing FRONT_A_OUTENA register\n");
    mapped[AOUTENA+AOFFFB] =  fippiconfig.FRONT_B_OUTENA;     // write to FPGA, then read back to verify
    if(mapped[AOUTENA+AOFFFB] != fippiconfig.FRONT_B_OUTENA) printf("Error writing FRONT_B_OUTENA register\n");
    mapped[AOUTENA+AOFFFC] =  fippiconfig.FRONT_C_OUTENA;     // write to FPGA, then read back to verify
    if(mapped[AOUTENA+AOFFFC] != fippiconfig.FRONT_C_OUTENA) printf("Error writing FRONT_C_OUTENA register\n");

   // MZ TrigIO board temperature
    printf("Board temperature: %d C \n",(int)board_temperature(mapped) );

   // ***** ZYNQ temperature
     printf("Zynq temperature: %d C \n",(int)zynq_temperature() );

   // ***** check HW info *********
   revsn = hwinfo(mapped);
   printf("Board unique ID 0x%016llX, Serial Number %llu \n",revsn, (revsn>>32) & 0xFFFF);
  // if(k==0) printf("WARNING: HW may be incompatible with this SW/FW \n");

 
 // clean up  
 flock( fd, LOCK_UN );
 munmap(map_addr, size);
 close(fd);
 return 0;
}










