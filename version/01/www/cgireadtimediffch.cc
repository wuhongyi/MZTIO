// cgireadtimediffch.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Mon May 25 19:27:55 2020 (+0800)
// Last-Updated: Mon May 25 19:32:14 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 1
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

int main(void)
{
  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;

  unsigned int timediffch[2];
  
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

  timediffch[0] = mapped[0x90];
  timediffch[1] = mapped[0x91];

  munmap(map_addr, size);
  close(fd);

  timediffch[0] = timediffch[0] & 0xFF;
  timediffch[1] = timediffch[1] & 0xFF;

  
  printf("%d,%d\n",timediffch[0],timediffch[1]);
  
  return 0;
}

// 
// cgireadtimediffch.cc ends here
