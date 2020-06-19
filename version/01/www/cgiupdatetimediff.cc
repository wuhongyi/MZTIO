// cgiupdatetimediff.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Mon May 25 19:55:59 2020 (+0800)
// Last-Updated: Mon May 25 20:53:00 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 7
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
  
  mapped[0x92] = 0x2;
  mapped[0x92] = 0;
  // usleep(100);
  
  uint32_t his[1024];
  for (int i = 0; i < 401; ++i)
    {
      mapped[0x003] = 0x0;	  // read from IO block
      mapped[0x93] = i;
      mapped[0x003] = 0x1;	  // read from IO block
      // usleep(10);
      his[i] = mapped[0x93];
      printf("%d, %d,",-4000+20*i,his[i]);
    }

  
  // clean up  
  flock( fd, LOCK_UN );
  munmap(map_addr, size);
  close(fd);

  
  return 0;
}
// 
// cgiupdatetimediff.cc ends here
