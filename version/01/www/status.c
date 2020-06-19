/* status.c --- 
 * 
 * Description: 
 * Author: Hongyi Wu(吴鸿毅)
 * Email: wuhongyi@qq.com 
 * Created: Sat Jul 20 04:58:39 2019 (+0000)
 * Last-Updated: Fri Jun 12 22:37:13 2020 (+0800)
 *           By: Hongyi Wu(吴鸿毅)
 *     Update #: 67
 * URL: http://wuhongyi.cn */

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

int read_print_runstats(volatile unsigned int *mapped);
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
  read_print_runstats(mapped);
  mapped[AOUTBLOCK] = OB_IOREG;


  // clean up  
  munmap(map_addr, size);


  return 0;
}


int read_print_runstats(volatile unsigned int *mapped)
{
  int k, lastrs;
  unsigned int m1[N_PL_RS_PAR], m2[N_PL_RS_PAR], m3[N_PL_RS_PAR], m4[N_PL_RS_PAR], m5[N_PL_RS_PAR]; 
  long long int revsn;
  double rtime;
  // double trate; 
  double dpmtime;
  
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
      {"S/N",       "%u",     0x012},	   //00
      {"FW_VERSION",	 "0x%X",     0x001},	   //01
      {"SW_VERSION",	 "0x%X",     0x002},	   //02
      {"DateOfExpiry",	 "0x%X",     0x003},	   //03
      {"UNIQUE_ID",	 "0x%X",     0x000},	   //04
      {"UNIQUE_ID",	 "0x%X",     0x000},	   //05
      {"DPMFULL",	 "%u",	 0x006},	   //06
      {"DPMFULL",	 "%u",	 0x007},	   //07
      {"NUMVTRIGS",	 "%u",	 0x008},	   //08
      {"NUMVTRIGS",	 "%u",	 0x009},	   //09
      {"NUMFTRIGS",	 "%u",	 0x00A},	   //10
      {"NUMFTRIGS",	 "%u",	 0x00B},	   //11
      {"RUNTICKS",	 "%u",	 0x00C},	   //12
      {"RUNTICKS",	 "%u",	 0x00D},	   //13
      {"RUNTIME[s]",	 "%u",	 0x000},	   //14
      {"DPM[%]",	 "%u",	 0x000},	   //15
      {"T_ZYNQ",	 "%u",	 0x000},	   //16
      {"T_BOARD",	 "%u",	 0x000},	   //17
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
      {"LEMO IN 1",	    "%u",	    0x180},   //00
      {"LEMO IN 2",	    "%u",	    0x181},   //01
      {"LEMO IN 3",	    "%u",	    0x182},   //02
      {"LEMO IN 4",	    "%u",	    0x183},   //03
      {"LEMO OUT 1",	    "%u",	    0x184},   //04
      {"LEMO OUT 2",	    "%u",	    0x185},   //05	
      {"LEMO OUT 3",	    "%u",	    0x186},   //06
      {"LEMO OUT 4",	    "%u",	    0x187},   //07
      {"reserved",	    "%u",	    0x000},   //08
      {"reserved",	    "%u",	    0x000},   //09
      {"reserved",	    "%u",	    0x000},   //10
      {"reserved",	    "%u",	    0x000},   //11
      {"reserved",	    "%u",	    0x000},   //12
      {"reserved",	    "%u",	    0x000},   //13
      {"reserved",	    "%u",	    0x000},   //14
      {"reserved",	    "%u",	    0x000},   //15
      {"reserved",	    "%u",	    0x000},   //16
      {"reserved",	    "%u",	    0x000},   //17
      {"reserved",	    "%u",	    0x000},   //18
      {"reserved",	    "%u",	    0x000},   //19
      {"reserved",	    "%u",	    0x000},   //20
      {"reserved",	    "%u",	    0x000},   //21
      {"reserved",	    "%u",	    0x000},   //22
      {"reserved",	    "%u",	    0x000},   //23
      {"reserved",	    "%u",	    0x000},   //24
      {"reserved",	    "%u",	    0x000},   //25
      {"reserved",	    "%u",	    0x000},   //26
      {"reserved",	    "%u",	    0x000},   //27
      {"reserved",	    "%u",	    0x000},   //28
      {"reserved",	    "%u",	    0x000},   //29
      {"reserved",	    "%u",	    0x000},   //30
      {"reserved",	    "%u",	    0x000}    //31
    };     

  struct dataflag data3[N_PL_RS_PAR] =
    {
      {"Front Trigger",	    "%u",	    0x1A0},   //00
      {"Back Trigger",	    "%u",	    0x1A1},   //01
      {"Front || Back",	    "%u",	    0x1A2},   //02
      {"Front && Back",	    "%u",	    0x1A3},   //03
      {"reserved",	    "%u",	    0x000},   //04
      {"reserved",	    "%u",	    0x000},   //05	
      {"reserved",	    "%u",	    0x000},   //06
      {"reserved",	    "%u",	    0x000},   //07
      {"reserved",	    "%u",	    0x000},   //08
      {"reserved",	    "%u",	    0x000},   //09
      {"reserved",	    "%u",	    0x000},   //10
      {"reserved",	    "%u",	    0x000},   //11
      {"reserved",	    "%u",	    0x000},   //12
      {"reserved",	    "%u",	    0x000},   //13
      {"reserved",	    "%u",	    0x000},   //14
      {"reserved",	    "%u",	    0x000},   //15
      {"reserved",	    "%u",	    0x000},	  //16 
      {"reserved",	    "%u",	    0x000},	  //17 
      {"reserved",	    "%u",	    0x000},	  //18 
      {"reserved",	    "%u",	    0x000},	  //19
      {"reserved",	    "%u",	    0x000},	  //20 
      {"reserved",	    "%u",	    0x000},	  //21 
      {"reserved",	    "%u",	    0x000},	  //22 
      {"reserved",	    "%u",	    0x000},	  //23 
      {"reserved",	    "%u",	    0x000},	  //24 
      {"reserved",	    "%u",	    0x000},	  //25 
      {"reserved",	    "%u",	    0x000},	  //26 
      {"reserved",	    "%u",	    0x000},	  //27 
      {"reserved",	    "%u",	    0x000},	  //28 
      {"reserved",	    "%u",	    0x000},	  //29
      {"reserved",	    "%u",	    0x000},	  //30
      {"reserved",	    "%u",	    0x000}	  //31
    }; 

  struct dataflag data4[N_PL_RS_PAR] =
    {
      {"BackPlaneFT",       "%u",           0x1C0},	  //00	
      {"BackPlaneVT",	    "%u",           0x1C1},	  //01 
      {"A1_1",              "%u",           0x1C2},	  //02 
      {"A2_1",		    "%u",	    0x1C3},	  //03 
      {"A3_1",		    "%u",	    0x1C4},	  //04 
      {"A4_1",		    "%u",	    0x1C5},	  //05 
      {"B1_1",		    "%u",	    0x1C6},	  //06 
      {"B2_1",		    "%u",	    0x1C7},	  //07 
      {"B3_1",		    "%u",	    0x1C8},	  //08 
      {"B4_1",		    "%u",	    0x1C9},	  //09 
      {"C1_1",		    "%u",	    0x1CA},	  //10 
      {"C2_1",		    "%u",	    0x1CB},	  //11 
      {"C3_1",		    "%u",	    0x1CC},	  //12 
      {"C4_1",		    "%u",	    0x1CD},	  //13 
      {"ValidationFP",	    "%u",	    0x1CE},	  //14 
      {"reserved",	    "%u",	    0x000},	  //15
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
      {"reserved",	    "%u",	  0x1E0},     //00  
      {"reserved",	    "%u",	  0x1E1},     //01 
      {"A1_2",	            "%u",	  0x1E2},     //02 
      {"A2_2",	            "%u",	  0x1E3},     //03 
      {"A3_2",	            "%u",	  0x1E4},     //04 
      {"A4_2",	            "%u",	  0x1E5},     //05 
      {"B1_2",	            "%u",	  0x1E6},     //06 
      {"B2_2",	            "%u",	  0x1E7},     //07 
      {"B3_2",	            "%u",	  0x1E8},     //08 
      {"B4_2",	            "%u",	  0x1E9},     //09 
      {"C1_2",	            "%u",	  0x1EA},     //10 
      {"C2_2",	            "%u",	  0x1EB},     //11 
      {"C3_2",	            "%u",	  0x1EC},     //12 
      {"C4_2",	            "%u",	  0x1ED},     //13 
      {"ValidationBP1",	    "%u",	  0x1EE},     //14 
      {"ValidationBP2",	    "%u",	  0x1EF},     //15 
      {"ValidationBP3",	    "%u",	  0x1F0},	//16 
      {"ValidationBP4",	    "%u",	  0x1F1},	//17 
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

  // read _used_ RS values (32bit) from FPGA 
  // at this point, raw binary values; later conversion into count rates etc

  //   mapped[AOUTBLOCK] = OB_RSREG;		// switch reads to run statistics block of addresses
  // must be done by calling function
  for(k = 0; k < N_PL_RS_PAR; k ++)
    {
      m1[k] = data1[k].reg>0?mapped[data1[k].reg]:0;
      m2[k] = data2[k].reg>0?mapped[data2[k].reg]:0;
      m3[k] = data3[k].reg>0?mapped[data3[k].reg]:0;
      m4[k] = data4[k].reg>0?mapped[data4[k].reg]:0;
      m5[k] = data5[k].reg>0?mapped[data5[k].reg]:0;
      /* m1[k] = mapped[data1[k].reg]; */
      /* m2[k] = mapped[data2[k].reg]; */
      /* m3[k] = mapped[data3[k].reg]; */
      /* m4[k] = mapped[data4[k].reg]; */
      /* m5[k] = mapped[data5[k].reg]; */
    }
  //  csr = m[1];    // more memorable name for CSR
  /* m[2] = PS_CODE_VERSION;   // overwrite with SW version */

  // condense the outputs a bit, many unused fields in "c"


  // compute and print useful output values
  // run time in s
  rtime = ((double)m1[12]+(double)m1[13]*TWOTO32)*1.0e-6/SYSTEM_CLOCK_MHZ;
  dpmtime = ((double)m1[6]+(double)m1[7]*TWOTO32)*1.0e-6/SYSTEM_CLOCK_MHZ;
   
  // trigger rate in 1/s
  if(rtime==0)
    {
      m1[15] = 0;
      // trate = 0;
    }
  else
    {
      m1[15] = (int)(dpmtime/rtime*100);
      // trate = ((double)m1[10]+(double)m1[11]*TWOTO32) / rtime;
    }
   
  m1[14] = (int)rtime;
   
   
  lastrs = N_USED_RS_SCALER;
  // temperatures and other I2C data
  m1[16] = (int)zynq_temperature();
  m1[17] = (int)board_temperature(mapped);
  revsn = hwinfo(mapped);                      // this is a pretty slow I2C I/O
  m1[4] = (int)(0xFFFFFFFF &  revsn );          
  m1[5] = (int)(0xFFFFFFFF & (revsn>>32) );  


  
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




/* status.c ends here */
