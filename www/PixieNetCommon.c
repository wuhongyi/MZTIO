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
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#include "PixieNetDefs.h"
#include "PixieNetCommon.h"


void I2Cstart(volatile unsigned int *mapped)  {
   unsigned int mval;
   // I2C start
   mval = 7;   // SDA = 1; SCL = 1; CTRL = 1 
   mapped[AI2CREG] = mval;
   usleep(I2CWAIT);
   mval = 6;   // SDA = 0; SCL = 1; CTRL = 1 
   mapped[AI2CREG] = mval;
   usleep(I2CWAIT);
}

 
void I2Cstop(volatile unsigned int *mapped)  {
   unsigned int mval;
   // I2C stop
   mval = 4;   // SDA = 0; SCL = 0; CTRL = 1 
   mapped[AI2CREG] = mval;
   usleep(I2CWAIT);

   mval = 6;   // SDA = 0; SCL = 1; CTRL = 1 
   mapped[AI2CREG] = mval;
   usleep(I2CWAIT);
   mval = 7;   // SDA = 1; SCL = 1; CTRL = 1 
   mapped[AI2CREG] = mval;
   usleep(I2CWAIT);
}

void I2Cslaveack(volatile unsigned int *mapped) {
    unsigned int mval;
    // I2C acknowledge
    mval = 0x0000;   // clear SCL and CTRL to give slave control of SDA     
    mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
    mval = 2;   // set SCL     
    mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
    mval = 0x0000;   // clear SCL and CTRL to give slave control of SDA   
         // for PLL
         mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
    // now can read SDA bit for ACK
}

void I2Cmasterack(volatile unsigned int *mapped) {
    unsigned int mval;
    // I2C acknowledge
    mval = 0x0004;   // clear SCL and SDA but not CTRL to keep control of SDA     
    mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
    mval = 6;   // set SCL     
    mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
}

void I2Cmasternoack(volatile unsigned int *mapped) {
    unsigned int mval;
    // I2C acknowledge
    mval = 0x0004;   // clear SCL and SDA but not CTRL to keep control of SDA     
    mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
    mval = 3;   // set SCL  and SDA   
    mapped[AI2CREG] = mval; 
    usleep(I2CWAIT);
}



void I2Cbytesend(volatile unsigned int *mapped, unsigned int *data) {
    unsigned int mval, k;
 // I2C byte send
   // SDA is captured during the low to high transition of SCL
   mval = 4;   // SDA = 0; SCL = 0; CTRL = 1 
   for( k=0; k<8; k++ )
   {
   //  printf("Sending a bit\n");
      mval = mval & 0x0005;   // clear SCL      
      mapped[AI2CREG] = mval; 
      usleep(I2CWAIT);
      if(data[7-k])
         mval = 5;            // SDA = 1; SCL = 0; CTRL = 1 
      else 
         mval = 4;            // SDA = 0; SCL = 0; CTRL = 1 
      mapped[AI2CREG] = mval; 
      usleep(I2CWAIT);
      mval = mval | 0x0002;   // set SCL      
      mapped[AI2CREG] = mval; 
      usleep(I2CWAIT);
   }
   // for PLL
               mval = mval & 0x0005;   // clear SCL    
               mapped[AI2CREG] = mval; 
}

void I2Cbytereceive(volatile unsigned int *mapped, unsigned int *data) {
 // I2C byte send
   unsigned int mval, k;
   // SDA is captured during the low to high transition of SCL
   mval = 0;   // SDA = 0; SCL = 0; CTRL = 0 
   for( k=0; k<8; k++ )
   {
      mval = 0;   // SDA = 0; SCL = 0; CTRL = 0       
      mapped[AI2CREG] = mval; 
      usleep(I2CWAIT);
      mval = 2;   // set SCL      
      mapped[AI2CREG] = mval; 
      usleep(I2CWAIT);
      mapped[AOUTBLOCK] = OB_EVREG;          
      mval = mapped[ACSROUT];
   //   printf("CSRout %x I2Cwait %d \n",mval,I2CWAIT);
      if(mval & 0x4)          // test for SDA out bit
         data[7-k] = 1;            
      else 
         data[7-k] = 0;            
   //   mapped[AI2CREG] = mval;   not for PLL
      usleep(I2CWAIT);
   }
}




 unsigned int setbit( unsigned int par, unsigned int bitc, unsigned int bitf)
 // returns 2^bitf if bit bitc of par is 1 
 { 
   unsigned int ret;
        ret = par & (1 << bitc);     // bitwise and or parameter with ccsra bit
        ret = ret >> bitc;                 // shift down to bit 0 
        ret = ret << bitf;                 // shift up to fippi bit
        return (ret);
  }

long long int hwinfo( volatile unsigned int *mapped )
// returns 32bit hwrev_sn, or 0 on error
{
   unsigned int  mval, i2cdata[8], ctrl[8], id[4];
   long long int revsn;
   int k, m;

  // ---------------- read EEPROM ---------------------------
   mapped[AOUTBLOCK] = OB_IOREG;     //  i/o registers


  ctrl[7] = 1;      // PN XL PROM  (TMP116)
  ctrl[6] = 0;
  ctrl[5] = 0;
  ctrl[4] = 1;  
  ctrl[3] = 0;
  ctrl[2] = 0;
  ctrl[1] = 0;
  ctrl[0] = 0;    


    // ------------- read 4 16bit words from EEPROM  -------------------- 

    for( m = 5; m < 9; m ++ )
    {

        // 2 bytes: ctrl, addr  write
      I2Cstart(mapped);
      ctrl[0] = 0;   // R/W*         // write starting addr to read from
      I2Cbytesend(mapped, ctrl);
      I2Cslaveack(mapped);
       mval = m;   // addr 7 = serial number
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
   
      I2Cbytereceive(mapped, i2cdata);
      for( k = 0; k < 8; k ++ )
         if(i2cdata[k])
            mval = mval + (1<<(k+0));
      I2Cmasternoack(mapped);
      I2Cstop(mapped);

      id[m-5] = mval;
   //   printf("I2C read serial number %d\n",mval);

   } // end for registers

   mapped[ABVAL] = id[2];
   revsn =  (long long int)id[0] + 
            ((long long int)(id[1])<<16) + 
            ((long long int)(id[2])<<32) + 
            ((long long int)(id[3])<<48) ;



   return(revsn);

}


float board_temperature( volatile unsigned int *mapped )
{
   unsigned int  mval, i2cdata[8];
   unsigned int ctrl[8];
   int k;

  // ---------------- read EEPROM ---------------------------
   mapped[AOUTBLOCK] = OB_EVREG;	  // read/write from/to MZ IO block


  ctrl[7] = 1;      // TMP116
  ctrl[6] = 0;
  ctrl[5] = 0;
  ctrl[4] = 1;  
  ctrl[3] = 0;
  ctrl[2] = 0;
  ctrl[1] = 0;
  ctrl[0] = 0;    
 
   // 2 bytes: ctrl, addr  write
   I2Cstart(mapped);
   ctrl[0] = 0;   // R/W*         // write starting addr to read from
   I2Cbytesend(mapped, ctrl);
   I2Cslaveack(mapped);
    mval = 0x00;   // addr 7 = serial number
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

   I2Cbytereceive(mapped, i2cdata);
   for( k = 0; k < 8; k ++ )
      if(i2cdata[k])
         mval = mval + (1<<(k+0));
   I2Cmasternoack(mapped);
   I2Cstop(mapped);
  
//    printf("I2C read test 0x%04X\n",mval);
//   printf("I2C (main) temperature (addr=0), temp (C) %f\n",mval*0.0078125);


  return(mval*0.0078125);
}//float board_temperature( volatile unsigned int *mapped )






float zynq_temperature()
{
  // try kernel <4 device file location
  float temperature = -999;
  char line[LINESZ];

  FILE *devfile = fopen( "/sys/devices/amba.0/f8007100.ps7-xadc/temp","r" );
  if( devfile )
  {
    fgets( line, LINESZ, devfile );    
    fclose(devfile);
    if( sscanf( line, "%f", &temperature ) != 1 )
    {
      //  printf( "got line '%s' trying to read ZYNQ temperature\n", line );
    }
  } else {

  // try kernel 4 device location
    // printf( "trying K4 location\n");
    // assume local shortcut exists to 
    // /sys/devices/soc0/amba/f8007100.adc/iio:device0/in_temp0_raw
    // which has trouble with fopen due to the :
    FILE *devfile1 = fopen( "/var/www/temp0_raw","r");
    if(!devfile1)
    { 
       // printf( "Could not open device file\n");
    } else {
       fgets( line, LINESZ, devfile1);
    //   printf( "%s\n", line);
       fclose(devfile1);
       if( sscanf( line, "%f", &temperature ) !=1 )
       {
         //  printf( "got line '%s' trying to read ZYNQ temperature\n", line );
       } else {
         temperature = (temperature - 2219)*123.04/1000;
         // constants 2219 and 123.04 are from .../in_temp0_offset and _scale
         // don't seem to change
       }
        
    }


  }

  return temperature;
}

int read_print_runstats(int mode, int dest, volatile unsigned int *mapped ) {
// mode 0: full print of all runstats, including raw values
// mode 1: only print register info, not temperatures etc thatrequire (slow) I2C reads
// dest 0: print to file
// dest 1: print to stdout      -- useful for cgi
// dest 2: print to both        -- currently fails if called by web client due to file write permissions

  int k, lastrs;
  FILE * fil;
  unsigned int m[N_PL_RS_PAR], c[N_PL_RS_PAR]; 
  long long int revsn;
  //unsigned int csr, csrbit;
  double rtime,  trate;
  char N[4][32] = {      // names for the cgi array
    "ParameterL",
    "LocalLogic",
    "ParameterT",
    "TriggerIOStatus"};


   // Run stats PL Parameter names applicable to a Pixie module 
char Module_PLRS_Names[N_PL_RS_PAR][MAX_PAR_NAME_LENGTH] = {
  // "reserved",
   "CSROUT",		//0       // hex
   "FW_VERSION", 
   "SW_VERSION", 
   "UNIQUE_ID", 
   "UNIQUE_ID", 
   "COINCTEST", 
   "reserved", 
   "reserved", 
   "reserved", 
   "reserved", 
   "NUMTRIGGERS", 	   //10    // dec
   "NUMTRIGGERS",
   "RUNTICKS", 
   "RUNTICKS", 
   "RUNTIME", 
   "TRIGGERRATE", 
   "T_ZYNQ", 
   "T_BOARD",
   "SNUM",
   "reserved",		    //20
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",	    //30
   "reserved"
};

 // Run stats PL Parameter names for Trigger I/O registers
char Channel_PLRS_Names[N_PL_RS_PAR][MAX_PAR_NAME_LENGTH] = {
 //  "reserved",
   "IN_FRONTA",		//0       // hex
   "IN_FRONTB", 
   "IN_FRONTC", 
   "IN_TRIGGERALL", 
   "IN_EBDATA", 
   "CMASK_FRONTA", 
   "CMASK_FRONTB", 
   "CMASK_FRONTC", 
   "CMASK_TRIGGERALL", 
   "CMASK_EBDATA", 
   "MMSUM_FRONTA", 	   //10       // dec
   "MMSUM_FRONTB", 
   "MMSUM_FRONTC", 
   "MMSUM_TRIGGERALL", 
   "MMSUM_EBDATA", 
   "reserved", 
   "reserved", 
   "reserved", 
   "reserved",
   "reserved",		    //20
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",
   "reserved",	    //30
   "reserved"
};      

// return(0);

//}

  // ************** XIA code begins **************************
  // open the output file
  if(dest != 1)  {
          fil = fopen("RS.csv","w");
          fprintf(fil,"ParameterL,LocalLogic,ParameterT,TriggerIOStatus\n");
   }
      

  // read _used_ RS values (32bit) from FPGA 
  // at this point, raw binary values; later conversion into count rates etc

 //   mapped[AOUTBLOCK] = OB_RSREG;		// switch reads to run statistics block of addresses
 // must be done by calling function
   for( k = 0; k < N_PL_RS_PAR; k ++ )
   {
      m[k]  = mapped[ARS_LOC+k];
      c[k]  = mapped[ARS_TIO+k];
   }
 //  csr = m[1];    // more memorable name for CSR
   m[2] = PS_CODE_VERSION;   // overwrite with SW version

   // condense the outputs a bit, many unused fields in "c"
   c[5]  = c[8];
   c[6]  = c[9]; 
   c[7]  = c[10];
   c[8]  = c[11]; 
   c[9]  = c[12];
   c[10] = c[16]; 
   c[11] = c[17];
   c[12] = c[18];
   c[13] = c[19];
   c[14] = c[20]; 
   c[15] = 0;
   c[16] = 0;
   c[17] = 0;
   c[18] = 0;


   // compute and print useful output values
   // run time in s
   rtime = ((double)m[12]+(double)m[13]*TWOTO32)*1.0e-6/SYSTEM_CLOCK_MHZ;

   // trigger rate in 1/s
   if(rtime==0)
      trate = 0; 
   else 
      trate = ((double)m[10]+(double)m[11]*TWOTO32) / rtime;

   // determine how much to "print"
   if(mode == 1)  {
     lastrs = 15;
     m[3] = 0 ;
     m[4] = 0 ;
   } else {
     lastrs = N_USED_RS_PAR;
     // temperatures and other I2C data
     m[16] = (int)zynq_temperature();
     m[17] = (int)board_temperature(mapped);
     revsn = hwinfo(mapped);                      // this is a pretty slow I2C I/O
     m[3] = (int)(0xFFFFFFFF &  revsn );          
     m[4] = (int)(0xFFFFFFFF & (revsn>>32) );  
   }

   // print values 
   for( k = 0; k < lastrs; k ++ )
   {
      if(k<10) {   // print bit patterns for some parameters
         if(dest != 1) fprintf(fil,"%s,0x%X,%s,0x%X\n ",Module_PLRS_Names[k],m[k],Channel_PLRS_Names[k],c[k]);
         if(dest != 0) printf("{%s:\"%s\",%s:\"0x%X\",%s:\"%s\",%s:\"0x%X\"},  \n",N[0],Module_PLRS_Names[k],N[1],m[k],N[2],Channel_PLRS_Names[k],N[3],c[k]);
   //  if(dest != 0) printf("{%s:\"%s\",%s:\"0x%X\",%s:\"%s\",%s:%u,%s:%u,%s:%u,%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],m[k],N[2],Channel_PLRS_Names[k],N[3],c[0][k],N[4],c[1][k],N[5],c[2][k],N[6],c[3][k]);
      } else if(k==14) {   // some results are floats
         if(dest != 1) fprintf(fil,"%s,%f,%s,%u\n ",Module_PLRS_Names[k],rtime,Channel_PLRS_Names[k],c[k]);
         if(dest != 0) printf("{%s:\"%s\",%s:\"%f\",%s:\"%s\",%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],rtime,N[2],Channel_PLRS_Names[k],N[3],c[k]);
      } else if(k==15) {   // some results are floats
         if(dest != 1) fprintf(fil,"%s,%f,%s,%u\n ",Module_PLRS_Names[k],trate,Channel_PLRS_Names[k],c[k]);
         if(dest != 0) printf("{%s:\"%s\",%s:\"%f\",%s:\"%s\",%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],trate,N[2],Channel_PLRS_Names[k],N[3],c[k]);
      } else  {
         if(dest != 1) fprintf(fil,"%s,%u,%s,%u\n ",Module_PLRS_Names[k],m[k],Channel_PLRS_Names[k],c[k]);
        if(dest != 0) printf("{%s:\"%s\",%s:%u,%s:\"%s\",%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],m[k],N[2],Channel_PLRS_Names[k],N[3],c[k]);
      }
   }
   

 // clean up  
 if(dest != 1) fclose(fil);
 return 0;
}




