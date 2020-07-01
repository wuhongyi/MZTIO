// cgisavesettings.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Fri Jun 26 12:59:38 2020 (+0800)
// Last-Updated: Fri Jun 26 16:32:03 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 12
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

#include <string>
#include <fstream>
#include <iostream>

int main(void)
{
  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;


  char * data;
  data = getenv("QUERY_STRING");                           // retrieve webpage arguments   
  std::string webdata(data);                               // turn into string
  // printf("%s\n",webdata.c_str());

  std::string ss = webdata.substr(3);
  int num = atoi(ss.c_str());
  // printf("%d\n",num);

  std::ofstream writedata;//
  char filename[32];
  sprintf(filename,"settings%d.ini",num);

  writedata.open(filename);//ios::bin ios::app
  if(!writedata.is_open())
    {
      printf("can't open file.");
      return 1;
    }
  
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

  unsigned int registeradd;
  unsigned int registervaule;

  registeradd = 0x2;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;
  registeradd = 0x3;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x100;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x105;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x101;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x106;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x102;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x107;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x103;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  registeradd = 0x104;
  registervaule = mapped[registeradd];
  writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;

  // 0x30 - 0x8F   48-143
  for (unsigned int i = 48; i < 144; ++i)
    {
      registeradd = i;
      registervaule = mapped[registeradd];
      writedata<<registeradd<<"   "<<registervaule<<"   xxxxxxxxxxxxxxxxxxxx"<<std::endl;
    }

  munmap(map_addr, size);
  close(fd);

  writedata.close();
  
  printf("Save experimental setup %d success.\n",num);
  
  return 0;
}



// 
// cgisavesettings.cc ends here
