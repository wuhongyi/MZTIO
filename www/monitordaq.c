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

#include "PixieNetDefs.h"
#include "PixieNetCommon.h"
#include "PixieNetConfig.h"


int main(void) {

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  unsigned int  ReqRunTime, PollTime;
  time_t starttime, currenttime;
  unsigned int m, c ;
  int loopcount; 
  // int eventcount;
 
  
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
   ReqRunTime   = fippiconfig.REQ_RUNTIME;
   PollTime     = fippiconfig.POLL_TIME;
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
