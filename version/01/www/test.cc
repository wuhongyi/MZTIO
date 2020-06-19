// test.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Sat Jul 20 07:35:44 2019 (+0000)
// Last-Updated: Sat Jul 20 08:10:49 2019 (+0000)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 22
// URL: http://wuhongyi.cn 

# include <stdio.h>

struct dataflag
{
  char name[32];
  char flag[16];
};

int main(int argc, char *argv[])
{
  struct dataflag data[4] =
    {
      {"qqq","\"0x%X\""},
      {"www","\"%u\""},
      {"eee","\"%f\""},
      {"rrr","\"%f\""}
    } ;



  char buff[128];
  sprintf(buff,"%s  \n",data[0].flag);
  printf(buff,13465);
  printf("%s \n",data[0].flag);

  sprintf(buff,"%s  \n",data[1].flag);
  printf(buff,13465);
  printf("%s \n",data[1].flag);

  sprintf(buff,"%s  \n",data[2].flag);
  printf(buff,1465.5);
  printf("%s \n",data[2].flag);

  char tmp[128] = "%s:\"%s\",%s:";
  sprintf(buff,"{%s %s, %s %s, %s %s, %s %s},  \n",tmp,data[0].flag,tmp,data[1].flag,tmp,data[2].flag,tmp,data[3].flag);
  printf("%s  \n",buff);

  printf(buff,"zzz",data[0].name,"aaa",79846513,"xxx",data[1].name,"sss",79846513,"ccc",data[2].name,"ddd",798.513,"vvv",data[3].name,"fff",79846513.12);
  
  return 0;
}


// 
// test.cc ends here
