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

// gcc -Wall writeI2C.c PixieNetCommon.o -o writeI2C


#include "PixieNetDefs.h"
#include "PixieNetCommon.h"

int main( int argc, char *argv[] )
{

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  int k;

  unsigned int sn = 0xDEAD;
  unsigned int rev = 0xBEEF;
  unsigned int zero[8] = {0};
  unsigned int i2cdata[8] = {0};
  unsigned int mval = 0;
  unsigned int ctrl[8];
  /* ctrl[7] = 1;     // PN PROM
     ctrl[6] = 0;
     ctrl[5] = 1;
     ctrl[4] = 0;  
     ctrl[3] = 0;
     ctrl[2] = 0;
     ctrl[1] = 0;
     ctrl[0] = 0;     */

  ctrl[7] = 1;      // PN XL PROM  (TMP116)
  ctrl[6] = 0;
  ctrl[5] = 0;
  ctrl[4] = 1;  
  ctrl[3] = 0;
  ctrl[2] = 0;
  ctrl[1] = 0;
  ctrl[0] = 0;    


  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0)
    {
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

  if (map_addr == MAP_FAILED)
    {
      perror("Failed to mmap");
      return 1;
    }

  mapped = (unsigned int *) map_addr;

  // ************************ parse arguments *********************************

  if( argc!=3)
    {
      printf( "please give arguments addr (0x##) for test read and serial number (decimal)  \n" );
      printf( "if addr >15, skip write \n" );
      return 2;
    }

  rev = strtol(argv[1], NULL, 16);
  sn = strtol(argv[2], NULL, 10);

  // ************************ prepare to write *********************************

      
  mapped[AOUTBLOCK] = OB_IOREG;	  // read/write from/to MZ IO block


  // ************************ I2C programming EEPROM *********************************
   
  /*       bugs: sequential reads seem to need a restart each time, data sheet says only the first
     
	   byte  content
	   0,1    temperature
	   2,3    configuration
	   4,5    temp high   revision low   ?
	   6,7    temp low     s/n           ?
	   8,9     unlock
	   10-17   unique ID
	   30,31   device ID 


  */

  if (rev<16)
    {

      // ----------- write unlock reg --------------
  
      // 4 bytes: ctrl, addr, data, data  : 
      I2Cstart(mapped);
      ctrl[0] = 0;   // R/W*
      I2Cbytesend(mapped, ctrl);     // I2C control byte: write
      I2Cslaveack(mapped);  

      //  printf("I2C write 1\n");

      mval = 0x4;   // register address  : 4 = unlock reg
      i2cdata[7] = (mval & 0x0080) >> 7 ;    
      i2cdata[6] = (mval & 0x0040) >> 6 ;    
      i2cdata[5] = (mval & 0x0020) >> 5 ;    
      i2cdata[4] = (mval & 0x0010) >> 4 ;
      i2cdata[3] = (mval & 0x0008) >> 3 ;    
      i2cdata[2] = (mval & 0x0004) >> 2 ;   
      i2cdata[1] = (mval & 0x0002) >> 1 ;    
      i2cdata[0] = (mval & 0x0001)      ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);

      mval = 0x8000;  //  register data  : set unlock bit
      i2cdata[7] = (mval & 0x8000) >> 15 ;    
      i2cdata[6] = (mval & 0x4000) >> 14 ;    
      i2cdata[5] = (mval & 0x2000) >> 13 ;    
      i2cdata[4] = (mval & 0x1000) >> 12 ; 
      i2cdata[3] = (mval & 0x0800) >> 11 ;    
      i2cdata[2] = (mval & 0x0400) >> 10 ;   
      i2cdata[1] = (mval & 0x0200) >> 9 ;    
      i2cdata[0] = (mval & 0x0100) >> 8 ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);

      i2cdata[7] = (mval & 0x0080) >> 7 ;    
      i2cdata[6] = (mval & 0x0040) >> 6 ;    
      i2cdata[5] = (mval & 0x0020) >> 5 ;    
      i2cdata[4] = (mval & 0x0010) >> 4 ;
      i2cdata[3] = (mval & 0x0008) >> 3 ;    
      i2cdata[2] = (mval & 0x0004) >> 2 ;   
      i2cdata[1] = (mval & 0x0002) >> 1 ;    
      i2cdata[0] = (mval & 0x0001)      ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);

      I2Cstop(mapped);
      usleep(1000);

      // ----------- write serial number  --------------

      // 4 bytes: ctrl, addr, data, data  : 
      I2Cstart(mapped);
      ctrl[0] = 0;   // R/W*
      I2Cbytesend(mapped, ctrl);     // I2C control byte: write
      I2Cslaveack(mapped);  

      mval = 0x07;   // register address  : 7 = unique id all zeros
      i2cdata[7] = (mval & 0x0080) >> 7 ;    
      i2cdata[6] = (mval & 0x0040) >> 6 ;    
      i2cdata[5] = (mval & 0x0020) >> 5 ;    
      i2cdata[4] = (mval & 0x0010) >> 4 ;
      i2cdata[3] = (mval & 0x0008) >> 3 ;    
      i2cdata[2] = (mval & 0x0004) >> 2 ;   
      i2cdata[1] = (mval & 0x0002) >> 1 ;    
      i2cdata[0] = (mval & 0x0001)      ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);

      //  register data  : serial number
      i2cdata[7] = (sn & 0x8000) >> 15 ;    
      i2cdata[6] = (sn & 0x4000) >> 14 ;    
      i2cdata[5] = (sn & 0x2000) >> 13 ;    
      i2cdata[4] = (sn & 0x1000) >> 12 ; 
      i2cdata[3] = (sn & 0x0800) >> 11 ;    
      i2cdata[2] = (sn & 0x0400) >> 10 ;   
      i2cdata[1] = (sn & 0x0200) >> 9 ;    
      i2cdata[0] = (sn & 0x0100) >> 8 ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);

      i2cdata[7] = (sn & 0x0080) >> 7 ;    
      i2cdata[6] = (sn & 0x0040) >> 6 ;    
      i2cdata[5] = (sn & 0x0020) >> 5 ;    
      i2cdata[4] = (sn & 0x0010) >> 4 ;
      i2cdata[3] = (sn & 0x0008) >> 3 ;    
      i2cdata[2] = (sn & 0x0004) >> 2 ;   
      i2cdata[1] = (sn & 0x0002) >> 1 ;    
      i2cdata[0] = (sn & 0x0001)      ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);
      I2Cstop(mapped);

      usleep(10000);    // wait 7ms or more. should check and wait more if necessary, really, 
      usleep(10000);  
    
      // -----------general call reset --------------

      // 2 bytes: ctrl, reset  : 
      I2Cstart(mapped);
      I2Cbytesend(mapped, zero);     // I2C control byte (general address): write
      I2Cslaveack(mapped);  

      mval = 0x06;   // reset code word
      i2cdata[7] = (mval & 0x0080) >> 7 ;    
      i2cdata[6] = (mval & 0x0040) >> 6 ;    
      i2cdata[5] = (mval & 0x0020) >> 5 ;    
      i2cdata[4] = (mval & 0x0010) >> 4 ;
      i2cdata[3] = (mval & 0x0008) >> 3 ;    
      i2cdata[2] = (mval & 0x0004) >> 2 ;   
      i2cdata[1] = (mval & 0x0002) >> 1 ;    
      i2cdata[0] = (mval & 0x0001)      ;   
      I2Cbytesend(mapped, i2cdata);
      I2Cslaveack(mapped);

      I2Cslaveack(mapped);
      I2Cstop(mapped);

      usleep(10000);
      usleep(10000);   

    }   // end <16

  // ----------- read back for verification --------------
   


     
  // read serial number

  // 2 bytes: ctrl, addr  write
  I2Cstart(mapped);
  ctrl[0] = 0;   // R/W*         // write starting addr to read from
  I2Cbytesend(mapped, ctrl);
  I2Cslaveack(mapped);
  mval = 0x07;   // addr 7 = serial number
  i2cdata[7] = (mval & 0x0080) >> 7 ;    
  i2cdata[6] = (mval & 0x0040) >> 6 ;    
  i2cdata[5] = (mval & 0x0020) >> 5 ;    
  i2cdata[4] = (mval & 0x0010) >> 4 ;
  i2cdata[3] = (mval & 0x0008) >> 3 ;    
  i2cdata[2] = (mval & 0x0004) >> 2 ;   
  i2cdata[1] = (mval & 0x0002) >> 1 ;    
  i2cdata[0] = (mval & 0x0001)      ;   
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);
  usleep(300);



  // read data bytes 
  mval = 0;
  ctrl[0] = 1;   // R/W*         // now read 
  usleep(100);
  I2Cstart(mapped);               //restart
  I2Cbytesend(mapped, ctrl);      // device address
  I2Cslaveack(mapped);
  I2Cbytereceive(mapped, i2cdata);
  for( k = 0; k < 8; k ++ )
    if(i2cdata[k])
      mval = mval + (1<<(k+8));
  I2Cmasterack(mapped);

  //   usleep(100);
  //   I2Cslaveack(mapped);
  I2Cbytereceive(mapped, i2cdata);
  for( k = 0; k < 8; k ++ )
    if(i2cdata[k])
      mval = mval + (1<<(k+0));
  //I2Cmasterack(mapped);
  I2Cmasternoack(mapped);
  I2Cstop(mapped);

  printf("I2C read serial number %d\n",mval);

  usleep(10000); 
   
  mval = rev & 0xF;   // register address  : 7 = unique id all zeros
  i2cdata[7] = (mval & 0x0080) >> 7 ;    
  i2cdata[6] = (mval & 0x0040) >> 6 ;    
  i2cdata[5] = (mval & 0x0020) >> 5 ;    
  i2cdata[4] = (mval & 0x0010) >> 4 ;
  i2cdata[3] = (mval & 0x0008) >> 3 ;    
  i2cdata[2] = (mval & 0x0004) >> 2 ;   
  i2cdata[1] = (mval & 0x0002) >> 1 ;    
  i2cdata[0] = (mval & 0x0001)      ;   

  // 2 bytes: ctrl, addr  write
  I2Cstart(mapped);
  ctrl[0] = 0;   // R/W*         // write starting addr to read from
  I2Cbytesend(mapped, ctrl);
  I2Cslaveack(mapped);
  I2Cbytesend(mapped, i2cdata);     // address 0  = temp value
  I2Cslaveack(mapped);

  usleep(300);


  // read data bytes
  mval = 0;
  ctrl[0] = 1;   // R/W*         // now read 
  usleep(100);
  I2Cstart(mapped);               //restart
  I2Cbytesend(mapped, ctrl);      // device address
  I2Cslaveack(mapped);
  I2Cbytereceive(mapped, i2cdata);
  for( k = 0; k < 8; k ++ )
    if(i2cdata[k])
      mval = mval + (1<<(k+8));
  I2Cmasterack(mapped);

  //  usleep(100);
  //  I2Cslaveack(mapped);
  I2Cbytereceive(mapped, i2cdata);
  for( k = 0; k < 8; k ++ )
    if(i2cdata[k])
      mval = mval + (1<<(k+0));
  //I2Cmasterack(mapped);
  I2Cmasternoack(mapped);
  I2Cstop(mapped);

  printf("I2C read test 0x%04X\n",mval);
  printf("if temperature (addr=0), temp (C) %f\n",mval*0.0078125);



   
 
  // clean up  
  flock( fd, LOCK_UN );
  munmap(map_addr, size);
  close(fd);
  return 0;
}










