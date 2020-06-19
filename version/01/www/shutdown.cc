// shutdown.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Tue Jun  9 14:42:32 2020 (+0800)
// Last-Updated: Wed Jun 10 12:17:04 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 29
// URL: http://wuhongyi.cn 

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
int main()
{
  printf("MZTIO OS poweroff.\n");
  // printf("MZTIO OS is going down for poweroff at 1 min later.\n");
  sync();

  uid_t uid ,euid;

  uid = getuid() ;
  euid = geteuid();
  if(setreuid(euid, uid))  //change the two id
    perror("setreuid");
     
  // reboot now
  // shutdown
  system("shutdown -h now");
  // system("shutdown -h 1");
  
  return 0;
}

// 
// shutdown.cc ends here
