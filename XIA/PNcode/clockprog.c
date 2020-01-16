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

// gcc -Wall clockprog.c PixieNetCommon.o -o clockprog


#include "PixieNetDefs.h"
#include "PixieNetCommon.h"

int main( int argc, char *argv[] ) {

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  int k;

  unsigned int pllregs[32] ={0};
  unsigned int mode;
  unsigned int i2cdata[8] = {0};
  unsigned int mval = 0;
  unsigned int reg;
  
  unsigned int icaddr[8];     // I2C address of CDCE813-Q1
  icaddr[7] = 1;    // A6
  icaddr[6] = 1;
  icaddr[5] = 0;
  icaddr[4] = 0;  
  icaddr[3] = 1;
  icaddr[2] = 0;
  icaddr[1] = 1;     // A0
  icaddr[0] = 0;     // R/W*


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
    return(-1);
  }

  mapped = (unsigned int *) map_addr;

   // ************************ parse arguments *********************************

   if( argc!=2)  {
     printf( "please give argument mode   \n" );
     printf( "   0 = read registers  \n" );
     printf( "   1 = write to EEPROM \n" );
     printf( "   2 = 25MHZ in and 1x out (FPGA)  \n" );
     printf( "   3 = 25MHZ in and 2x out (FPGA, ext) \n" );
     return(-2);
   }

   mode = strtol(argv[1], NULL, 10);

   if(mode>3 || mode<0)  {
     printf( "invalid mode selected, exiting  \n" );
     return(-3);
  }




   // ************************ I2C programming PLL *********************************
   
   /*      see CDCE813-Q1 data sheet and TI ClockPro utility

    */


    if(mode==0) {
     for(reg=0x00;reg<0x20;reg++)
     {
         I2Cstart(mapped);
         usleep(DACWAIT);	
      
         // I2C addr byte - write
         icaddr[0] = 0;   // R/W*
         I2Cbytesend(mapped, icaddr);
         I2Cslaveack(mapped);
      
         // I2C command code
         i2cdata[7] = 1;     // 0 for block read/write, 1 for byte read/write
         i2cdata[6] = 0;     // 0..6: addr offset
         i2cdata[5] = 0;
         i2cdata[4] = (reg & 0x10) >> 4; //0;   
         i2cdata[3] = (reg & 0x08) >> 3; //0;   
         i2cdata[2] = (reg & 0x04) >> 2; //0;    
         i2cdata[1] = (reg & 0x02) >> 1; //0;       
         i2cdata[0] = (reg & 0x01) >> 0; //0;   
         printf("pll reg %d 0x%02X (%d%d%d%d%d):",reg,reg,i2cdata[4], i2cdata[3], i2cdata[2], i2cdata[1], i2cdata[0] );
         I2Cbytesend(mapped, i2cdata);
         I2Cslaveack(mapped); 
      
         I2Cstart(mapped);
         usleep(DACWAIT);	

         // I2C addr byte - read
         icaddr[0] = 1;   // R/W*
         I2Cbytesend(mapped, icaddr);
         I2Cslaveack(mapped);     
      
         I2Cbytereceive(mapped, i2cdata);   
         mval=0;
         for( k = 0; k < 8; k ++ )
            if(i2cdata[k])
               mval = mval + (1<<(k+0));
         printf(" 0x%02X \n",mval );
         I2Cmasterack(mapped);
         usleep(DACWAIT);	

         I2Cstop(mapped);
   
      }   // end for loop over regs
      return(0);

    }   // end mode 0

    if(mode==1) {
         pllregs[06] = 0x01;  // set reg 6 bit 0 for EEPrOM write command

         reg = 06;            // I2C write to reg. 6 only
         I2Cstart(mapped);
         usleep(DACWAIT);	
      
         // I2C addr byte - write
         icaddr[0] = 0;   // R/W*
         I2Cbytesend(mapped, icaddr);
         I2Cslaveack(mapped);
      
         // I2C command code
         i2cdata[7] = 1;     // 0 for block read/write, 1 for byte read/write
         i2cdata[6] = 0;     // 0..6: addr offset
         i2cdata[5] = 0;
         i2cdata[4] = (reg & 0x10) >> 4; //0;   
         i2cdata[3] = (reg & 0x08) >> 3; //0;   
         i2cdata[2] = (reg & 0x04) >> 2; //0;    
         i2cdata[1] = (reg & 0x02) >> 1; //0;       
         i2cdata[0] = (reg & 0x01) >> 0; //0;   
         I2Cbytesend(mapped, i2cdata);
         I2Cslaveack(mapped);        

          // I2C data byte - write
         i2cdata[7] = (pllregs[reg] & 0x80) >> 7; //0;       // 
         i2cdata[6] = (pllregs[reg] & 0x40) >> 6; //0;  
         i2cdata[5] = (pllregs[reg] & 0x20) >> 5; //0;  
         i2cdata[4] = (pllregs[reg] & 0x10) >> 4; //0;   
         i2cdata[3] = (pllregs[reg] & 0x08) >> 3; //0;   
         i2cdata[2] = (pllregs[reg] & 0x04) >> 2; //0;    
         i2cdata[1] = (pllregs[reg] & 0x02) >> 1; //0;       
         i2cdata[0] = (pllregs[reg] & 0x01) >> 0; //0;   
         I2Cbytesend(mapped, i2cdata);

         I2Cslaveack(mapped);

         usleep(DACWAIT);	
         I2Cstop(mapped);   

         usleep(1000);

         printf("clockprog: EEPROM write completed\n" );
         return(0);
    }     // end mode 1


    if(mode==2) {
       // defaults from chip plus enable for Y1

       printf("clockprog: programming clock PLL for 25 MHz output on Y1\n" );

       // do not write to pllregs[0]
       pllregs[01] = 0x09;
       pllregs[02] = 0x9C;
       pllregs[03] = 0x01;
       pllregs[04] = 0x00;
       pllregs[05] = 0x00;
       pllregs[06] = 0x0E;
       // pllregs 7-15 unused
       pllregs[16] = 0x00;
       pllregs[17] = 0x00;
       pllregs[18] = 0x00;
       pllregs[19] = 0x00;

       pllregs[20] = 0x61;
       pllregs[21] = 0x00;
       pllregs[22] = 0x00;
       pllregs[23] = 0x00;
       pllregs[24] = 0x1F;
       pllregs[25] = 0xF0;
       pllregs[26] = 0x02;
       pllregs[27] = 0x10;
       pllregs[28] = 0x1F;
       pllregs[29] = 0xF0;
       pllregs[30] = 0x02;
       pllregs[31] = 0x10;
    }

     if(mode==3) {
       printf("clockprog: programming clock PLL for 25 MHz outputs on Y1 and Y3\n" );

       // generated by ClockPro
       // inputs:  - 25 in, 25 MHz out on Y1 (FPGA) and Y3 (MMCX "clk")
       //          - Signal Source LVCMOS 
       //          - Device CDCE913 (CDCE813 _almost_ same)

       // do not write to pllregs[0]
       pllregs[01] = 0x09;    // Clock Pro says 0x08, but that's for CDCE913
       pllregs[02] = 0x34;
       pllregs[03] = 0x01;
       pllregs[04] = 0x02;
       pllregs[05] = 0x50;
       pllregs[06] = 0x40;
       // pllregs 7-15 unused
       pllregs[16] = 0x00;
       pllregs[17] = 0x00;
       pllregs[18] = 0x00;
       pllregs[19] = 0x00;

       pllregs[20] = 0xCD;
       pllregs[21] = 0x02;
       pllregs[22] = 0x00;
       pllregs[23] = 0x00;
       pllregs[24] = 0x66;
       pllregs[25] = 0x09;
       pllregs[26] = 0x93;
       pllregs[27] = 0x2C;
       pllregs[28] = 0x66;
       pllregs[29] = 0x09;
       pllregs[30] = 0x93;
       pllregs[31] = 0x2C;
    }


   if(mode<4) {
     printf("clockprog: now programming\n" );
     for(reg=0x00;reg<0x20;reg++)
     {
         if(reg<7 || reg >15) 
         {
            I2Cstart(mapped);
            usleep(DACWAIT);	
         
            // I2C addr byte - write
            icaddr[0] = 0;   // R/W*
            I2Cbytesend(mapped, icaddr);
            I2Cslaveack(mapped);
         
            // I2C command code
            i2cdata[7] = 1;     // 0 for block read/write, 1 for byte read/write
            i2cdata[6] = 0;     // 0..6: addr offset
            i2cdata[5] = 0;
            i2cdata[4] = (reg & 0x10) >> 4; //0;   
            i2cdata[3] = (reg & 0x08) >> 3; //0;   
            i2cdata[2] = (reg & 0x04) >> 2; //0;    
            i2cdata[1] = (reg & 0x02) >> 1; //0;       
            i2cdata[0] = (reg & 0x01) >> 0; //0;   
            I2Cbytesend(mapped, i2cdata);
            I2Cslaveack(mapped);        

             // I2C data byte - write
            i2cdata[7] = (pllregs[reg] & 0x80) >> 7; //0;       // 
            i2cdata[6] = (pllregs[reg] & 0x40) >> 6; //0;  
            i2cdata[5] = (pllregs[reg] & 0x20) >> 5; //0;  
            i2cdata[4] = (pllregs[reg] & 0x10) >> 4; //0;   
            i2cdata[3] = (pllregs[reg] & 0x08) >> 3; //0;   
            i2cdata[2] = (pllregs[reg] & 0x04) >> 2; //0;    
            i2cdata[1] = (pllregs[reg] & 0x02) >> 1; //0;       
            i2cdata[0] = (pllregs[reg] & 0x01) >> 0; //0;   
            I2Cbytesend(mapped, i2cdata);

            I2Cslaveack(mapped);
   
            usleep(DACWAIT);	
            I2Cstop(mapped);    
         }  // end if reg in range
   
      }   // end for loop over regs
      return(0);

    }   // end mode 2-3




 
 // clean up  
 flock( fd, LOCK_UN );
 munmap(map_addr, size);
 close(fd);
 return 0;
}










