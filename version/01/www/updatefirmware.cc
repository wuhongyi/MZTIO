// updatefirmware.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Wed Jun 10 19:27:15 2020 (+0800)
// Last-Updated: Wed Jun 10 20:01:59 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 6
// URL: http://wuhongyi.cn 

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main()
{
  uid_t uid ,euid;

  uid = getuid() ;
  euid = geteuid();
  if(setreuid(euid, uid))  //change the two id
    perror("setreuid");
     
  if (!access("/root/xillydemo.bit",0))
    {
      system("mount /dev/mmcblk0p1 /mnt/sd");
      system("mv /root/xillydemo.bit /mnt/sd");
      system("umount /mnt/sd");
      system("shutdown -r 1");//reboot 1 min later
      printf("Update firmware success.\n");
      printf("MZTIO OS is going down for reboot at 1 min later.\n");
    }
  else
    {
      printf("file /root/xillydemo.bit don't exist.\n");
    }
  
  return 0;
}

// 
// updatefirmware.cc ends here
