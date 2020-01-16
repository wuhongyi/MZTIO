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
    mval = 2;   // set SCL     
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





int hwinfo( volatile unsigned int *mapped )
// returns 32bit hwrev_sn, or 0 on error
{
   unsigned int  mval, i2cdata[8];
   unsigned int revsn;
   int k;

  // ---------------- read EEPROM ---------------------------
   mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers
   mval = mapped[ABRDINFO];
   //printf("ABRDINGFO 0x%04X\n",mval);
   mapped[AOUTBLOCK] = OB_IOREG;     // read from i/o registers

   unsigned int ctrl[8];
   ctrl[7] = (mval & 0x800000) >> 23 ;    
   ctrl[6] = (mval & 0x400000) >> 22 ;    
   ctrl[5] = (mval & 0x200000) >> 21 ;    
   ctrl[4] = (mval & 0x100000) >> 20 ; 
   ctrl[3] = (mval & 0x080000) >> 19 ;    
   ctrl[2] = (mval & 0x040000) >> 18 ;   
   ctrl[1] = (mval & 0x020000) >> 17 ;    
   ctrl[0] = (mval & 0x010000) >> 16 ;  

   I2Cstart(mapped);
   ctrl[0] = 0;   // R/W*
   I2Cbytesend(mapped, ctrl);     // I2C control byte: write
   I2Cslaveack(mapped);  

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

    // read data byte 0..1
   mval = 0;
   ctrl[0] = 1;   // R/W*         // now read
  
   usleep(100);
   I2Cstart(mapped);               //restart
   I2Cbytesend(mapped, ctrl);
   I2Cslaveack(mapped);
   I2Cbytereceive(mapped, i2cdata);
   for( k = 0; k < 8; k ++ )
      if(i2cdata[k])
         mval = mval + (1<<(k+0));
   I2Cmasterack(mapped);

   usleep(100);
   I2Cstart(mapped);               //restart
   I2Cbytesend(mapped, ctrl);
   I2Cslaveack(mapped);
   I2Cbytereceive(mapped, i2cdata);
   for( k = 0; k < 8; k ++ )
      if(i2cdata[k])
         mval = mval + (1<<(k+8));
   I2Cmasterack(mapped);

   //printf("I2C read Revision 0x%04X\n",mval);
   if ( (mval == PN_BOARD_VERSION_12_250_A)     ||
        (mval == PN_BOARD_VERSION_12_250_B)     ||
        (mval == PN_BOARD_VERSION_12_250_B_PTP) ||
        (mval == 0                        )   )
        //printf("HW Revision 0x%04X\n",mval);
        revsn = mval << 16;
   else
   {
       printf("Unsupported HW Revision 0x%04X\n",mval);
       return(0);
   }

   // read data byte 1..2
   mval = 0;
   ctrl[0] = 1;   // R/W*         // now read
  
   usleep(100);
   I2Cstart(mapped);               //restart
   I2Cbytesend(mapped, ctrl);
   I2Cslaveack(mapped);
   I2Cbytereceive(mapped, i2cdata);
   for( k = 0; k < 8; k ++ )
      if(i2cdata[k])
         mval = mval + (1<<(k+0));
   I2Cmasterack(mapped);

   usleep(100);
   I2Cstart(mapped);               //restart
   I2Cbytesend(mapped, ctrl);
   I2Cslaveack(mapped);
   I2Cbytereceive(mapped, i2cdata);
   for( k = 0; k < 8; k ++ )
      if(i2cdata[k])
         mval = mval + (1<<(k+8));
   I2Cmasterack(mapped);

   //printf("I2C read Serial number %d \n",mval);

   mapped[ABVAL] = mval;
   revsn =revsn + (mval & 0xFFFF);
   //printf("Revision %04X, Serial Number %d \n",(revsn>>16), revsn&0xFFFF);

 //  I2Cmasternoack(mapped);
   I2Cstop(mapped);
   return(revsn);

}


float board_temperature( volatile unsigned int *mapped )
{
  unsigned int i2cdata[8];
  
  I2Cstart(mapped);
  
  // I2C addr byte
  i2cdata[7] = 1;
  i2cdata[6] = 0;
  i2cdata[5] = 0;
  i2cdata[4] = 1;
  i2cdata[3] = 1;   // A2
  i2cdata[2] = 0;   // A1
  i2cdata[1] = 0;   // A0
  i2cdata[0] = 1;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  
  I2Cslaveack(mapped);
  
  I2Cbytereceive(mapped, i2cdata);
  unsigned int temperature_val = 0;
  for( int k = 0; k < 7; k ++ )
    if(i2cdata[k])
      temperature_val = temperature_val + (1<<(k+8));
  
  unsigned int Tsign = i2cdata[7];
  //printf("Temperature: bits 0x%x \n",mval >> 8);
  
  I2Cmasterack(mapped);
  I2Cbytereceive(mapped, i2cdata);     // second byte has fractional portion, no use
  I2Cmasternoack(mapped);
  I2Cstop(mapped);
  
  return (Tsign ? -1.0 : 1.0) * temperature_val / 256.0f;
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
// mode 1: only print times and rates
// dest 0: print to file
// dest 1: print to stdout      -- useful for cgi
// dest 2: print to both        -- currently fails if called by web client due to file write permissions

  int k, lastrs;
  FILE * fil;
  unsigned int m[N_PL_RS_PAR], c[NCHANNELS][N_PL_RS_PAR], csr, csrbit;
  double ma, ca[NCHANNELS], mb, cb[NCHANNELS], CT[NCHANNELS], val;
  char N[7][32] = {      // names for the cgi array
    "ParameterM",
    "Module",
    "ParameterC",
    "Channel0",
    "Channel1",
    "Channel2",
    "Channel3" };


   // Run stats PL Parameter names applicable to a Pixie module 
char Module_PLRS_Names[N_PL_RS_PAR][MAX_PAR_NAME_LENGTH] = {
   "reserved",
   "CSROUT",		//0 
   "SYSTIME", 
   "RUNTIME", 
   "RUNTIME", 
   "TOTALTIME", 
   "TOTALTIME", 
   "NUMEVENTS", 
   "NUMEVENTS", 
   "BHL_EHL", 
   "CHL_FIFILENGTH", 
   "FW_VERSION", 	   //10
   "SNUM",
   "PPSTIME", 
   "T_ADC", 
   "T_ZYNQ", 
   //	"reserved", 
   "HW_VERSION", 
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

 // Run stats PL Parameter names applicable to a Pixie channel 
char Channel_PLRS_Names[N_PL_RS_PAR][MAX_PAR_NAME_LENGTH] = {
   "reserved",
   "OOR*",		//0 
   "ICR", 
   "COUNTTIME", 
   "COUNTTIME", 
   "NTRIG", 
   "NTRIG", 
   "FTDT", 
   "FTDT", 
   "SFDT*", 
   "SFDT*", 
   "GCOUNT*", 	   //10
   "GCOUNT*", 
   "NOUT", 
   "NOUT", 
   "GDT*", 
   "GDT*", 
   "NPPI*", 
   "NPPI*", 
   //	"reserved",
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
          fprintf(fil,"ParameterM,Module,ParameterC,Channel0,Channel1,Channel2,Channel3\n");
   }
      

  // read _used_ RS values (32bit) from FPGA 
  // at this point, raw binary values; later conversion into count rates etc

 //   mapped[AOUTBLOCK] = OB_RSREG;		// switch reads to run statistics block of addresses
 // must be done by calling function
   for( k = 0; k < N_USED_RS_PAR; k ++ )
   {
      m[k]  = mapped[ARS0_MOD+k];
      c[0][k] = mapped[ARS0_CH0+k];
      c[1][k] = mapped[ARS0_CH1+k];
      c[2][k] = mapped[ARS0_CH2+k];
      c[3][k] = mapped[ARS0_CH3+k];
   }
   csr = m[1];    // more memorable name for CSR

   // compute and print useful output values
   // run time = total time and Count time
   ma = ((double)m[3]+(double)m[4]*TWOTO32)*1.0e-9;
   if(dest != 1) fprintf(fil,"RUN_TIME,%4.6G,COUNT_TIME",ma); 
   if(dest != 0) printf("{%s:\"RUN_TIME\",%s:%4.6G,%s:\"COUNT_TIME\"",N[0], N[1],ma,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      CT[k] = ((double)c[k][3] + (double)c[k][4]*TWOTO32)*1.0e-9;
      if(dest != 1) fprintf(fil,",%4.6G",CT[k]);
      if(dest != 0) printf(",%s:%4.6G",N[3+k],CT[k]);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");

   
   // Total time and ICR
   if(dest != 1) fprintf(fil,"TOTAL_TIME,%4.6G,INPUT_COUNT_RATE",ma); 
   if(dest != 0) printf("{%s:\"TOTAL_TIME\",%s:%4.6G,%s:\"INPUT_COUNT_RATE\"",N[0], N[1],ma,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      ca[k] = (double)c[k][5] + (double)c[k][6]*TWOTO32;               //Ntrig
      cb[k] = ((double)c[k][7] + (double)c[k][8]*TWOTO32)*1.0e-9;      //FTDT
      if((CT[k]-cb[k])==0)
         val = 0;                 // avoid division by zero
      else
         val = ca[k]/(CT[k]-cb[k]);
      if(dest != 1) fprintf(fil,",%4.6G",val);
      if(dest != 0) printf(",%s:%4.6G",N[3+k],val);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");

   // Event rate and OCR
   mb = (double)m[7]+(double)m[8]*TWOTO32;
   if(ma==0)
      val = 0;                 // avoid division by zero
   else
      val = mb/ma;
   if(dest != 1) fprintf(fil,"EVENT_RATE,%4.6G,OUTPUT_COUNT_RATE",val); 
   if(dest != 0) printf("{%s:\"EVENT_RATE\",%s:%4.6G,%s:\"OUTPUT_COUNT_RATE\"",N[0], N[1],val,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      ca[k] = (double)c[k][13] + (double)c[k][14]*TWOTO32;     // Nout
      if(CT[k]==0)
         val = 0;                 // avoid division by zero
      else
         val = ca[k]/CT[k];
      if(dest != 1) fprintf(fil,",%4.6G",val);
      if(dest != 0) printf(",%s:%4.6G",N[3+k],val);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");

   // FTDT
   if(dest != 1) fprintf(fil,"PS_CODE_VERSION,0x%X,FTDT",PS_CODE_VERSION); 
   if(dest != 0) printf("{%s:\"PS_CODE_VERSION\",%s:\"0x%X\",%s:\"FTDT\"",N[0], N[1],PS_CODE_VERSION,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      if(dest != 1) fprintf(fil,",%4.3E",cb[k]);
      if(dest != 0) printf(",%s:%4.3E",N[3+k],cb[k]);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");

   // Active bit, SFDT
   csrbit =  (csr & 0x00002000) >> 13;
   if(dest != 1) fprintf(fil,"ACTIVE,%d,SFDT*",csrbit ); 
   if(dest != 0) printf("{%s:\"ACTIVE\",%s:\"%d\",%s:\"SFDT*\"",N[0], N[1],csrbit,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      ca[k] = ((double)c[k][9] + (double)c[k][10]*TWOTO32)*1.0e-9;    // SFDT
      if(dest != 1) fprintf(fil,",%4.3E",ca[k]);
      if(dest != 0) printf(",%s:%4.3E",N[3+k],ca[k]);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");

   // PSA_LICENSED, PPR
   csrbit =  (csr & 0x00000400) >> 10;
   if(dest != 1) fprintf(fil,"PSA_LICENSED,%d,PASS_PILEUP_RATE*",csrbit); 
   if(dest != 0) printf("{%s:\"PSA_LICENSED\",%s:%d,%s:\"PASS_PILEUP_RATE*\"",N[0], N[1],csrbit,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      ca[k] = (double)c[k][17] + (double)c[k][18]*TWOTO32;     // NPPI
      if(CT[k]==0)
         val = 0;                 // avoid division by zero
      else
         val = ca[k]/CT[k];
      if(dest != 1) fprintf(fil,",%4.6G",val);
      if(dest != 0) printf(",%s:%4.6G",N[3+k],val);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");


   // PTP required, Gate rate
   csrbit =  (csr & 0x00000020) >> 5;
   if(dest != 1) fprintf(fil,"PTP_REQ,%d,GATE_RATE*",csrbit); 
   if(dest != 0) printf("{%s:\"PTP_REQ\",%s:%d,%s:\"GATE_RATE*\"",N[0], N[1],csrbit,N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      ca[k] = (double)c[k][11] + (double)c[k][12]*TWOTO32;     // GCOUNT
      if(CT[k]==0)
         val = 0;                 // avoid division by zero
      else
         val = ca[k]/CT[k];
      if(dest != 1) fprintf(fil,",%4.6G",val);
      if(dest != 0) printf(",%s:%4.6G",N[3+k],val);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");


   // Gate time
   if(dest != 1) fprintf(fil,"--,0,GDT*"); 
   if(dest != 0) printf("{%s:\"--\",%s:0,%s:\"GDT*\"",N[0], N[1],N[2]);
   for( k = 0; k < NCHANNELS; k ++ ) {
      ca[k] = ((double)c[k][15] + (double)c[k][16]*TWOTO32)*1.0e-9;    // GDT
      if(dest != 1) fprintf(fil,",%4.6G",ca[k]);
      if(dest != 0) printf(",%s:%4.6G",N[3+k],ca[k]);
   }
   if(dest != 1) fprintf(fil,"\n ");
   if(dest != 0) printf("},  \n");

   if(mode == 1) 
     lastrs = 3;
   else
   {
     lastrs = N_USED_RS_PAR;
     // temperatures
     m[14] = (int)board_temperature(mapped);
     m[15] = (int)zynq_temperature();
     m[16] = (int)(0xFFFF & (hwinfo(mapped) >> 16));          // this is a pretty slow I2C I/O
   }





   // print raw values also
   for( k = 0; k < lastrs; k ++ )
   {
      if(k==16 || k==11 || k==1) {   // print bit patterns for some parameters
         if(dest != 1) fprintf(fil,"%s,0x%X,%s,%u,%u,%u,%u\n ",Module_PLRS_Names[k],m[k],Channel_PLRS_Names[k],c[0][k],c[1][k],c[2][k],c[3][k]);
         if(dest != 0) printf("{%s:\"%s\",%s:\"0x%X\",%s:\"%s\",%s:%u,%s:%u,%s:%u,%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],m[k],N[2],Channel_PLRS_Names[k],N[3],c[0][k],N[4],c[1][k],N[5],c[2][k],N[6],c[3][k]);
      } else if(k==2) {    // ICR gets factor 15 to scale in cps
         if(dest != 1) fprintf(fil,"%s,0x%X,%s,%u,%u,%u,%u\n ",Module_PLRS_Names[k],m[k],Channel_PLRS_Names[k],ICRSCALE*c[0][k],ICRSCALE*c[1][k],ICRSCALE*c[2][k],ICRSCALE*c[3][k]);
         if(dest != 0) printf("{%s:\"%s\",%s:\"0x%X\",%s:\"%s\",%s:%u,%s:%u,%s:%u,%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],m[k],N[2],Channel_PLRS_Names[k],N[3],ICRSCALE*c[0][k],N[4],ICRSCALE*c[1][k],N[5],ICRSCALE*c[2][k],N[6],ICRSCALE*c[3][k]);
      } else  {
         if(dest != 1) fprintf(fil,"%s,%u,%s,%u,%u,%u,%u\n ",Module_PLRS_Names[k],m[k],Channel_PLRS_Names[k],c[0][k],c[1][k],c[2][k],c[3][k]);
         if(dest != 0) printf("{%s:\"%s\",%s:%u,%s:\"%s\",%s:%u,%s:%u,%s:%u,%s:%u},  \n",N[0],Module_PLRS_Names[k],N[1],m[k],N[2],Channel_PLRS_Names[k],N[3],c[0][k],N[4],c[1][k],N[5],c[2][k],N[6],c[3][k]);
      }
   }
      
       
 
 // clean up  
 if(dest != 1) fclose(fil);
 return 0;
}




