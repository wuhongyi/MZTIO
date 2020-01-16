/*----------------------------------------------------------------------
 * Copyright (c) 2017 XIA LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided 
 * that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above 
 *     copyright notice, this list of conditions and the 
 *     following disclaimer.
 *   * Redistributions in binary form must reproduce the 
 *     above copyright notice, this list of conditions and the 
 *     following disclaimer in the documentation and/or other 
 *     materials provided with the distribution.
 *   * Neither the name of XIA LLC
 *     nor the names of its contributors may be used to endorse 
 *     or promote products derived from this software without 
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *----------------------------------------------------------------------*/
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

#include "PixieNetDefs.h"
#include "PixieNetCommon.h"


int main(void) {		 

  int k, m;
  FILE * fil1;
  FILE * fil2;
  unsigned int wf[MAX_TL] = {0};
//  unsigned int wf1[MAX_TL] = {0};
//  unsigned int wf2[MAX_TL] = {0};
//  unsigned int wf3[MAX_TL] = {0};
  char line[LINESZ];
  char name[LINESZ];
  char name2[LINESZ];
  char key[5] = {"0x"};
  char * data;
  int nsamples;
  unsigned int mval;
  unsigned int evno =0 ; // = 3;
  int ferr;



   // **************** code begins **********************

  // get form data
  data = getenv("QUERY_STRING");
  if( (data != NULL) && (sscanf(data,"evno=%d",&mval)==1) )  {
        evno =  mval;
  }


   // -------- runtype 0x500, LMdata.txt ---------
   // read info from LM file
    fil2 = fopen("LMdata.txt","r");
    fgets(line, LINESZ, fil2);     // read from file, header info
    fgets(line, LINESZ, fil2);     // read and check run type
      sscanf(line, "%s %s %x", name, name2, &mval);
      if(mval != 0x500) {
         printf("Invalid %s %s = %x, should be %x\n",name, name2, mval,0x500);
         return -1;
     }
     fgets(line, LINESZ, fil2); // skip start time
     fgets(line, LINESZ, fil2);

     for( k = 0; k < 8; k ++ )
        fgets(line, LINESZ, fil2); // skip event header #1

     k=0;
     memcpy(name, &key[0], 4); // extract first 2 characters
     do { // read through event #1 to determine waveform length, assuming all are the same!!
        fgets(line, LINESZ, fil2); // read wf sample
        memcpy(name, &line[0], 2); // extract first 2 characters
        //name[3] = '\0';
        //printf("%s %s \n",name,key);
        k++;
     } while ((strcmp(name,key) != 0) && (k<MAX_TL));     // stop at next hit pattern 
     nsamples=k-3;
//     printf("waveform length is %d\n",nsamples);
     fclose(fil2);


   // read numbered waveform from LM file 
   fil2 = fopen("LMdata.txt","r");
   for( k = 0; k < 4; k ++ )
        fgets(line, LINESZ, fil2); // skip file header 

   if(evno >0) { 
      //for( m = 0; m < evno; m ++ ) {      // skip unwanted events
      m=0;
      ferr=0;
      while ((m< evno) && (ferr==0) ) {
        m++;
        for( k = 0; k < 8; k ++ )    
            if(!fgets(line, LINESZ, fil2))    // skip event headers
               ferr=1;                        // check for file read error
        for( k = 0; k < nsamples; k ++ )    
            if(!fgets(line, LINESZ, fil2))    // skip event waveforms
               ferr=1;  
      }
   }

   if(ferr==0)  {
      for( k = 0; k < 8; k ++ )    {
           fgets(line, LINESZ, fil2); // skip event header #1
           if(k==1)  sscanf(line, "%d", &mval);   // extract channel number
      }
   //   printf("event %d, channel %d\n",evno,mval);
   
       for( k = 0; k < nsamples; k ++ ) {   
          fgets(line, LINESZ, fil2); // skip event waveform
          sscanf(line, "%d", &mval);   // extract channel number
          wf[k] = mval;
      }
   }
   // else 
   // wf = 0;


   // -------- read the webpage template and print  ---------

   fil1 = fopen("adcpage.html","r");
   for( k = 0; k < 82; k ++ )
   {
     
      fgets(line, LINESZ, fil1);     // read from template, first part
      if(k==6)
         printf("<title>Pixie-Net Waveforms </title>\n");            // "print" to webserver on stdout  
      else if(k==55)
         printf("<h1>Pixie-Net Waveforms</h1>\n");            // "print" to webserver on stdout  
      else if(k==70)
         printf("<i><p>This page displays the List mode waveforms from LMdata.txt</p>\n");            // "print" to webserver on stdout  
      else if(k==73)
         printf("<p>4 nanoseconds between samples </p></i>\n");            // "print" to webserver on stdout  
      else
         printf("%s",line);            // "print" to webserver on stdout  
   }   


 

      // print the form to refresh with another trace
   printf("   <form action=\"cgiwaveforms.cgi\"                        ");
   printf("   method=\"GET\">                                            ");
   printf("   Event Number: <input type=\"text\" name=\"evno\" value=\"%d\">         ",evno);
   printf("    <input type=\"submit\" value=\"submit\" >              ");
   printf("    </form>                                                 ");

   for( k = 82; k < 95; k ++ )
   {
      fgets(line, LINESZ, fil1);     // read from template, first part
      printf("%s",line);            // "print" to webserver on stdout  
   } 

   
   fgets(line, LINESZ, fil1);        // read from template, the line listing the ADC.csv file. This is not printed
   printf("       \"sample,wf\\n\"  +  \n");

   // print the data
   for( k = 0; k < nsamples; k ++ )
   {
        printf("      \"%d,%d\\n \"  + \n",k,wf[k]);
   }
   // comma, not + requred in last line
   printf("      \"%d,%d\\n \"  ,  \n",k,wf[k-1]);
 
   // finish printing the webpage
   for( k = 96; k < 124; k ++ )
   {
      fgets(line, LINESZ, fil1);        // read from template
  //    if(k==74) 
  //       printf("<p>4 nanoseconds between samples </p>\n");
  //    else
      printf("%s",line);               // "print" to webserver on stdout
   }   





   // finish printing the webpage
   for( k = 75; k < 124; k ++ )
   {
      fgets(line, LINESZ, fil1);        // read from template
      printf("%s",line);               // "print" to webserver on stdout
   }   
   

// clean up  

fclose(fil1);
fclose(fil2);
return 0;
}
