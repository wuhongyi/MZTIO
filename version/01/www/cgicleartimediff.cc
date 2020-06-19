// cgicleartimediff.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Mon May 25 19:38:29 2020 (+0800)
// Last-Updated: Mon May 25 20:16:46 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 2
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
  
  mapped[0x92] = 1;
  usleep(100);
  mapped[0x92] = 0;

  // clean up  
  flock( fd, LOCK_UN );
  munmap(map_addr, size);
  close(fd);

  printf("Clear Time Difference Histogram Success.\n");
  
  return 0;
}
// 
// cgicleartimediff.cc ends here
