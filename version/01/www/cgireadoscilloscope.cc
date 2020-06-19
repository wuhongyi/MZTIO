// cgireadoscilloscope.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Sat Jan 18 11:13:26 2020 (+0000)
// Last-Updated: Sat Jan 18 11:25:17 2020 (+0000)
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

int main(void)
{
  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;

  unsigned int monitor[4];
  
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

  monitor[0] = mapped[0x40];
  monitor[1] = mapped[0x41];
  monitor[2] = mapped[0x42];
  monitor[3] = mapped[0x43];

  munmap(map_addr, size);
  close(fd);


  printf("%d,%d,%d,%d\n",monitor[0],monitor[1],monitor[2],monitor[3]);
  
  return 0;
}
  
// 
// cgireadoscilloscope.cc ends here
