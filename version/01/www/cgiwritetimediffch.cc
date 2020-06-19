// cgiwritetimediffch.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Mon May 25 19:33:14 2020 (+0800)
// Last-Updated: Mon May 25 20:16:29 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 3
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

  int n0,n1;

  n0 = webdata.find("&v0="); 
  n1 = webdata.find("&v1=");
  // printf("%d  %d\n",n0,n1);

  string s0 = webdata.substr(n0+4, n1-n0-4);
  string s1 = webdata.substr(n1+4);

  // printf("%s  %s\n",s0.c_str(),s1.c_str());


  int data0 = atoi(s0.c_str());
  int data1 = atoi(s1.c_str());
  // printf("%d  %d\n",data0,data1);

  data0 = data0 & 0xff;
  data1 = data1 & 0xff;
  
   
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
  
  mapped[0x90] = data0;
  mapped[0x91] = data1;

  // clean up  
  flock( fd, LOCK_UN );
  munmap(map_addr, size);
  close(fd);

  printf("Change Time Difference CH Success.\n");
  
  return 0;
}

// 
// cgiwritetimediffch.cc ends here
