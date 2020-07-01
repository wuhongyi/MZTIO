// register.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Wed Jun 10 22:13:37 2020 (+0800)
// Last-Updated: Wed Jul  1 12:35:28 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 11
// URL: http://wuhongyi.cn 

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

#include "MZTIODefs.h"
#include "MZTIOCommon.h"

struct dataflag
{
  char name[32];
  char flag[16];
  unsigned int reg;
};

int read_print_logic(volatile unsigned int *mapped);
int main(void)
{
  /* printf("Content-type:text/html\n\n"); */
  
  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
 
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

  // ************** user code begins **************************

  mapped[AOUTBLOCK] = OB_EVREG;
  read_print_logic(mapped);
  mapped[AOUTBLOCK] = OB_IOREG;


  // clean up  
  munmap(map_addr, size);


  return 0;
}


int read_print_logic(volatile unsigned int *mapped)
{
  int k, lastrs;
  unsigned int m1[N_PL_RS_PAR], m2[N_PL_RS_PAR], m3[N_PL_RS_PAR], m4[N_PL_RS_PAR], m5[N_PL_RS_PAR]; 
  
  char N[10][32] =
    {      // names for the cgi array
      "Parameter1",
      "Data1",
      "Parameter2",
      "Data2",
      "Parameter3",
      "Data3",
      "Parameter4",
      "Data4",
      "Parameter5",
      "Data5"
    };

  // \"0x%X\"
  // \"%u\"
  // \"%f\" please don't use it

  struct dataflag data1[N_PL_RS_PAR] = 
    {
      {"IN_FRONTA",	    "0x%X",	  0x100},   //00
      {"LVDSIO_A",	    "0x%X",	    0x105},   //01
      {"IN_FRONTB",	    "0x%X",	    0x101},   //02
      {"LVDSIO_B",	    "0x%X",	    0x106},   //03
      {"IN_FRONTC",	    "0x%X",	    0x102},   //04
      {"LVDSIO_C",	    "0x%X",	    0x107},   //05	
      {"IN_TRIGGERALL",	    "0x%X",	    0x103},   //06
      {"IN_EBDATA",	    "0x%X",	    0x104},   //07

      {"reserved",	 "%u",	 0x000},	   //08
      {"reserved",	 "%u",	 0x000},	   //09
      {"reserved",	 "%u",	 0x000},	   //10
      {"reserved",	 "%u",	 0x000},	   //11
      {"reserved",	 "%u",	 0x000},	   //12
      {"reserved",	 "%u",	 0x000},	   //13
      {"reserved",	 "%u",	 0x000},	   //14
      {"reserved",	 "%u",	 0x000},	   //15
      {"reserved",	 "%u",	 0x000},	   //16
      {"reserved",	 "%u",	 0x000},	   //17
      {"reserved",	 "%u",	 0x000},	   //18
      {"reserved",	 "0x%X",     0x000},	   //19
      {"reserved",	 "0x%X",     0x000},	   //20
      {"reserved",	 "0x%X",     0x000},	   //21
      {"reserved",	 "0x%X",     0x000},	   //22
      {"reserved",	 "0x%X",     0x000},	   //23
      {"reserved",	 "0x%X",     0x000},	   //24
      {"reserved",	 "0x%X",     0x000},	   //25
      {"reserved",	 "0x%X",     0x000},	   //26
      {"reserved",	 "0x%X",     0x000},	   //27
      {"reserved",	 "0x%X",     0x000},	   //28
      {"reserved",	 "0x%X",     0x000},	   //29
      {"reserved",	 "0x%X",     0x000},	   //30
      {"reserved",	 "0x%X",     0x000}	   //31
    };

  struct dataflag data2[N_PL_RS_PAR] = 
    {
      {"TriggerModeFP",     "%u",         0x050},	  //00	
      {"TriggerModeBP1",    "%u",	  0x051},	  //01 
      {"TriggerModeBP2",    "%u",	  0x052},	  //02 
      {"TriggerModeBP3",    "%u",	  0x053},	  //03 
      {"TriggerModeBP4",    "%u",	  0x054},	  //04 
      {"reserved",	    "%u",	  0x000},	  //05 
      {"reserved",	    "%u",	  0x000},	  //06 
      {"Ext Clk Source",    "%u",	  0x045},	  //07 
      {"LEMO CH 1",	    "%u",	  0x040},	  //08 
      {"LEMO CH 2",	    "%u",	  0x041},	  //09 
      {"LEMO CH 3",	    "%u",	  0x042},	  //10 
      {"LEMO CH 4",	    "%u",	  0x043},	  //11 
      {"reserved",	    "%u",	  0x000},	  //12 
      {"reserved",	    "%u",	  0x000},	  //13 
      {"reserved",	    "%u",	  0x000},	  //14 
      {"reserved",	    "%u",	  0x000},	  //15 
      {"reserved",	    "%u",	  0x000},	  //16 
      {"reserved",	    "%u",	  0x000},	  //17 
      {"reserved",	    "%u",	  0x000},	  //18 
      {"reserved",	    "%u",	  0x000},	  //19
      {"reserved",	    "%u",	  0x000},	  //20 
      {"reserved",	    "0x%X",	    0x000},   //21
      {"reserved",	    "0x%X",	    0x000},   //22
      {"reserved",	    "0x%X",	    0x000},   //23
      {"reserved",	    "0x%X",	    0x000},   //24
      {"reserved",	    "0x%X",	    0x000},   //25
      {"reserved",	    "0x%X",	    0x000},   //26
      {"reserved",	    "0x%X",	    0x000},   //27
      {"reserved",	    "0x%X",	    0x000},   //28
      {"reserved",	    "0x%X",	    0x000},   //29
      {"reserved",	    "0x%X",	    0x000},   //30
      {"reserved",	    "0x%X",	    0x000}    //31
    };     

  struct dataflag data3[N_PL_RS_PAR] =
    {
      {"AND_A",	    "0x%X",	    0x070},	//00
      {"AND_B",	    "0x%X",	    0x071},	//01 
      {"reserved",	    "0x%X",	    0x000},	//02 
      {"reserved",	    "0x%X",	    0x000},	//03
      {"reserved",	    "0x%X",	    0x000},	//04 
      {"reserved",	    "0x%X",	    0x000},	//05 
      {"reserved",	    "0x%X",	    0x000},	//06 
      {"reserved",	    "0x%X",	    0x000},	//07 
      {"reserved",	    "0x%X",	    0x000},	//08 
      {"reserved",	    "0x%X",	    0x000},	//09 
      {"reserved",	    "0x%X",	    0x000},	//10 
      {"reserved",	    "0x%X",	    0x000},	//11 
      {"reserved",	    "0x%X",	    0x000},	//12 
      {"reserved",	    "0x%X",	    0x000},	//13 
      {"reserved",	    "0x%X",	    0x000},	//14 
      {"reserved",	    "0x%X",	    0x000},	//15 
      {"reserved",	    "%x%X",	    0x000},	//16 
      {"reserved",	    "%x%X",	    0x000},	//17 
      {"reserved",	    "%x%X",	    0x000},	//18 
      {"reserved",	    "%x%X",	    0x000},	//19
      {"reserved",	    "0x%X",	    0x000},	//20 
      {"reserved",	    "0x%X",	    0x000},	//21 
      {"reserved",	    "0x%X",	    0x000},	//22 
      {"reserved",	    "0x%X",	    0x000},	//23 
      {"reserved",	    "0x%X",	    0x000},	//24 
      {"reserved",	    "0x%X",	    0x000},	//25 
      {"reserved",	    "0x%X",	    0x000},	//26 
      {"reserved",	    "0x%X",	    0x000},	//27 
      {"reserved",	    "0x%X",	    0x000},	//28 
      {"reserved",	    "0x%X",	    0x000},	//29
      {"reserved",	    "0x%X",	    0x000},	//30
      {"reserved",	    "0x%X",	    0x000}	//31
    }; 

  struct dataflag data4[N_PL_RS_PAR] =
    {
      {"DelayAndExtend1",   "0x%X",	    0x030},     //00  
      {"DelayAndExtend2",   "0x%X",	    0x031},	//01 
      {"DelayAndExtend3",   "0x%X",	    0x032},	//02 
      {"DelayAndExtend4",   "0x%X",	    0x033},	//03 
      {"DelayAndExtend5",   "0x%X",	    0x034},	//04 
      {"DelayAndExtend6",   "0x%X",	    0x035},	//05 
      {"DelayAndExtend7",   "0x%X",	    0x036},	//06 
      {"DelayAndExtend8",   "0x%X",	    0x037},	//07 
      {"DelayAndExtend9",   "0x%X",	    0x038},	//08 
      {"DelayAndExtend10",  "0x%X",	    0x039},	//09 
      {"DelayAndExtend11",  "0x%X",	    0x03A},	//10 
      {"DelayAndExtend12",  "0x%X",	    0x03B},	//11 
      {"DelayAndExtend13",  "0x%X",	    0x03C},	//12 
      {"DelayAndExtend14",  "0x%X",	    0x03D},	//13 
      {"DelayAndExtend15",  "0x%X",	    0x03E},	//14 
      {"DelayAndExtend16",  "0x%X",	    0x03F},	//15 
      {"reserved",	    "%u",	    0x000},	//16 
      {"reserved",	    "%u",	    0x000},	//17 
      {"reserved",	    "%u",	    0x000},	//18 
      {"reserved",	    "%u",	    0x000},	//19
      {"reserved",	    "%u",	    0x000},	//20 
      {"reserved",	    "%u",	    0x000},	//21 
      {"reserved",	    "%u",	    0x000},	//22 
      {"reserved",	    "%u",	    0x000},	//23 
      {"reserved",	    "%u",	    0x000},	//24 
      {"reserved",	    "%u",	    0x000},	//25 
      {"reserved",	    "%u",	    0x000},	//26 
      {"reserved",	    "%u",	    0x000},	//27 
      {"reserved",	    "%u",	    0x000},	//28 
      {"reserved",	    "%u",	    0x000},	//29
      {"reserved",	    "%u",	    0x000},	//30
      {"reserved",	    "%u",	    0x000}	//31
    }; 



  struct dataflag data5[N_PL_RS_PAR] =
    {
      {"Multi_A",	    "0x%X",	    0x060},   //00
      {"Multi_B",	    "0x%X",	    0x061},   //01
      {"Multi_C",	    "0x%X",	    0x062},   //02
      {"Multi_D",           "0x%X",	    0x063},   //03
      {"Multi_E",	    "0x%X",	    0x064},   //04
      {"Multi_F",	    "0x%X",	    0x065},   //05
      {"Multi_G",	    "0x%X",	    0x066},   //06
      {"Multi_H",	    "0x%X",	    0x067},   //07
      {"OR_A",              "0x%X",	    0x068},   //08
      {"OR_B",	            "0x%X",	    0x069},   //09
      {"OR_C",	            "0x%X",	  0x06A},     //10 
      {"OR_D",	            "0x%X",	  0x06B},     //11 
      {"OR_E",	            "0x%X",	  0x06C},     //12 
      {"OR_F",	            "0x%X",	  0x06D},     //13 
      {"OR_G",	            "0x%X",	  0x06E},     //14 
      {"OR_H",	            "0x%X",	  0x06F},     //15 
      {"reserved",	    "%u",	  0x000},	//16 
      {"reserved",	    "%u",	  0x000},	//17 
      {"reserved",	    "%u",	  0x000},	//18 
      {"reserved",	    "%u",	  0x000},	//19
      {"reserved",	    "%u",	  0x000},	//20 
      {"reserved",	    "%u",	  0x000},	//21 
      {"reserved",	    "%u",	  0x000},	//22 
      {"reserved",	    "%u",	  0x000},	//23 
      {"reserved",	    "%u",	  0x000},	//24 
      {"reserved",	    "%u",	  0x000},	//25 
      {"reserved",	    "%u",	  0x000},	//26 
      {"reserved",	    "%u",	  0x000},	//27 
      {"reserved",	    "%u",	  0x000},	//28 
      {"reserved",	    "%u",	  0x000},	//29
      {"reserved",	    "%u",	  0x000},	//30
      {"reserved",	    "%u",	  0x000}	//31
    }; 

  
  // ************** user code begins **************************

  for(k = 0; k < N_PL_RS_PAR; k ++)
    {
      m1[k] = data1[k].reg>0?mapped[data1[k].reg]:0;
      m2[k] = data2[k].reg>0?mapped[data2[k].reg]:0;
      m3[k] = data3[k].reg>0?mapped[data3[k].reg]:0;
      m4[k] = data4[k].reg>0?mapped[data4[k].reg]:0;
      m5[k] = data5[k].reg>0?mapped[data5[k].reg]:0;
      // m1[k] = mapped[data1[k].reg];
      // m2[k] = mapped[data2[k].reg];
      // m3[k] = mapped[data3[k].reg];
      // m4[k] = mapped[data4[k].reg];
      // m5[k] = mapped[data5[k].reg];
    }
   
  lastrs = N_USED_RS_PAR;

  printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",N[0],N[1],N[2],N[3],N[4],N[5],N[6],N[7],N[8],N[9]);
  for( k = 0; k < lastrs; k ++ )
    {
      char buff[1024];
      char tmp[128] = "%s";
      sprintf(buff,"%s, %s, %s, %s, %s, %s, %s, %s, %s, %s  \n",tmp,data1[k].flag,tmp,data2[k].flag,tmp,data3[k].flag,tmp,data4[k].flag,tmp,data5[k].flag);

      printf(buff,data1[k].name,m1[k],data2[k].name,m2[k],data3[k].name,m3[k],data4[k].name,m4[k],data5[k].name,m5[k]);
       
    }

  return 0;
}


// 
// register.cc ends here
