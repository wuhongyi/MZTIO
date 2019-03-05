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
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/file.h>
// need to compile with -lm option

#include "MZTIODefs.h"
#include "MZTIOCommon.h"


int read_print_runstats(int mode, int dest, volatile unsigned int *mapped );

int main(void)
{

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  unsigned int  ReqRunTime, PollTime;
  time_t starttime, currenttime;
  unsigned int m, c ;
  int loopcount; 
  // int eventcount;
 

  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0) {
    perror("Failed to open devfile");
    return -2;
  }

  //Lock the PL address space so multiple programs cant step on eachother.
  if( flock( fd, LOCK_EX | LOCK_NB ) )
  {
    printf( "Failed to get file lock on /dev/uio0\n" );
    return -3;
  }

  map_addr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (map_addr == MAP_FAILED) {
    perror("Failed to mmap");
    return -4;
  }

  mapped = (unsigned int *) map_addr;

  // --------------------------------------------------------
  // ------------------- Main code begins --------------------
  // --------------------------------------------------------



    // ********************** Monitoring Start **********************
   ReqRunTime   = 10;
   PollTime     = 900000;
   loopcount =  0;
 //  eventcount = 0;
   starttime = time(NULL);           // capture OS start time
   mapped[ACOUNTER_CLR] = 1;         // write to reset RS counters
   mapped[ACSRIN] = 1;               // set RunEnable bit to start run
   mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers
    

    // ********************** Run Loop **********************
   do {

 
        // ----------- Periodically save Run Status  -----------
       
        if(loopcount % PollTime == 0) 
        {
            // for debug purposes, print to std out so we see what's going on
            m = mapped[ARS_LOC+0xC]; // RUN_TICKS_L
            c = mapped[ARS_TIO+3];   // IN_TRIGGERALL
            printf("%s,%u,%s,0x%08X\n ","RUN_TICKS_L",m,"IN_TRIGGERALL",c);        


            // print (small) set of RS to file, visible to web
            mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers
            read_print_runstats(1, 0, mapped);
            mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers
          
        }

          // ----------- loop housekeeping -----------

         loopcount ++;
         currenttime = time(NULL);
      } while (currenttime <= starttime+ReqRunTime); // run for a fixed time   
  //     } while (eventcount <= 20); // run for a fixed number of events   



   // ********************** Run Stop **********************

   // clear RunEnable bit to stop run
   mapped[ACSRIN] = 0;               
 
   // print RS.csv one last time
   mapped[AOUTBLOCK] = OB_EVREG;
   read_print_runstats(0, 0, mapped);
   mapped[AOUTBLOCK] = OB_IOREG;

 flock( fd, LOCK_UN );
 munmap(map_addr, size);
 close(fd);
 return 0;
}











int read_print_runstats(int mode, int dest, volatile unsigned int *mapped )
{
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
   c[15] = mapped[480];
   c[16] = mapped[480];
   c[17] = mapped[480];
   c[18] = mapped[480];//wuhongyi


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
