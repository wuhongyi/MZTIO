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


int main(void) {

  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  int k;
  FILE * fil;
  char line[LINESZ];
 
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

  // ************** XIA code begins **************************

  // read the webpage template and print 
  fil = fopen("rspage.html","r");
  for( k = 0; k < 96; k ++ )
  {
      fgets(line, LINESZ, fil);     // read from template, first part
      if(k==6)
         printf("<title>Pixie-Net Run Statistics (current)</title>\n");
      else if(k==60)      
         printf("<p> <h1> Pixie-Net Run Statistics (current) </h1>\n");
      else if(k==75)  
         printf("<i><p>This page displays the run statistics just read from the Pixie-Net</p>\n");            // "print" to webserver on stdout  
      else if(k==76)  
         printf("<p><b>Do not execute or refresh while a DAQ is in progress</b></p>\n");                     // "print" to webserver on stdout  
      else
         printf("%s",line);            // "print" to webserver on stdout  
  }   
   
  // print runstats to stdout
  printf("  var csv = [                  \n");
  //   printf("{ParameterM:\"rest\", Module:123, ParameterC:\"test\", Channel0:1, Channel1:1, Channel2:1, Channel3:1},  \n");
  mapped[AOUTBLOCK] = OB_RSREG;
  read_print_runstats(0, 1, mapped);
  mapped[AOUTBLOCK] = OB_IOREG;
  printf("  ];                 \n");


  // finish printing the webpage
  for( k = 96; k < 130; k ++ )
  {
      fgets(line, LINESZ, fil);        // read from template
      printf("%s",line);               // "print" to webserver on stdout
  }   
   
 // clean up  
 munmap(map_addr, size);
 close(fd);
 fclose(fil);
 return 0;
}
