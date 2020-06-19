// cgireadregister.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Tue Jan 28 11:31:19 2020 (+0000)
// Last-Updated: Tue Jan 28 12:59:48 2020 (+0000)
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

  unsigned int readdata;

  char * data;
  data = getenv("QUERY_STRING");                           // retrieve webpage arguments  
  std::string webdata(data);
  std::string ss = webdata.substr(7);

  int datan;
  sscanf(ss.c_str(), "%x", &datan);    
  
  // printf("%s  %d\n",ss.c_str(),datan);
  
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

  readdata = mapped[datan];


  munmap(map_addr, size);
  close(fd);


  printf("0x%x\n",readdata);
  
  return 0;
}


// 
// cgireadregister.cc ends here
