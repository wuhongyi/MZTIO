// cgiwriteoscilloscope.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Sat Jan 18 11:30:14 2020 (+0000)
// Last-Updated: Sat Jan 18 12:33:25 2020 (+0000)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 17
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
#include <sys/file.h>

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

int main(void)
{

  char * data;
  string label, values;
   
  data = getenv("QUERY_STRING");                           // retrieve webpage arguments   
  std::string webdata(data);                               // turn into string

  // printf("input data: %s\n",data);

  int n0,n1,n2,n3;

  n0 = webdata.find("&v0="); 
  n1 = webdata.find("&v1=");
  n2 = webdata.find("&v2=");
  n3 = webdata.find("&v3="); 
  // printf("%d  %d  %d  %d\n",n0,n1,n2,n3);

  string s0 = webdata.substr(n0+4, n1-n0-4);
  string s1 = webdata.substr(n1+4, n2-n1-4);
  string s2 = webdata.substr(n2+4, n3-n2-4);
  string s3 = webdata.substr(n3+4);
  // printf("%s  %s  %s  %s\n",s0.c_str(),s1.c_str(),s2.c_str(),s3.c_str());


  int data0 = atoi(s0.c_str());
  int data1 = atoi(s1.c_str());
  int data2 = atoi(s2.c_str());
  int data3 = atoi(s3.c_str());
  // printf("%d  %d  %d  %d\n",data0,data1,data2,data3);

   
   
  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;

  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0) {
    perror("Failed to open devfile");
    return -1;
  }
  
  //Lock the PL address space so multiple programs cant step on eachother.
  if(flock(fd, LOCK_EX | LOCK_NB))
    {
      printf( "Failed to get file lock on /dev/uio0\n" );
      return -2;
    }
  
  map_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map_addr == MAP_FAILED)
    {
      perror("Failed to mmap");
      return -3;
    }

  mapped = (unsigned int *) map_addr;

  mapped[0x003] = 0x0;	  // read from IO block
  
  mapped[0x40] = data0;
  mapped[0x41] = data1;
  mapped[0x42] = data2;
  mapped[0x43] = data3;

  // clean up  
  flock( fd, LOCK_UN );
  munmap(map_addr, size);
  close(fd);

  printf("Change Oscilloscope Monitoring Success.\n");
  
  return 0;
}


// 
// cgiwriteoscilloscope.cc ends here
