// progfippi.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: Sat Jul 20 13:36:26 2019 (+0000)
// Last-Updated: Fri Jun 26 14:36:56 2020 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 4
// URL: http://wuhongyi.cn 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/file.h>


// need to compile with -lm option

#include "MZTIODefs.h"
#include "MZTIOCommon.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

// trim from start
static inline std::string &ltrim(std::string &s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    
  if( s.size() )
    {
      const size_t pos = s.find_first_not_of( '\0' );
      if( pos != 0 && pos != string::npos )
        s.erase( s.begin(), s.begin() + pos );
      else if( pos == string::npos )
        s.clear();
    }
    
  return s;
}
  
// trim from end
static inline std::string &rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    
  const size_t pos = s.find_last_not_of( '\0' );
  if( pos != string::npos && (pos+1) < s.size() )
    s.erase( s.begin() + pos + 1, s.end() );
  else if( pos == string::npos )
    s.clear();  //string is all '\0' characters
    
  return s;
}
  
// trim from both ends
void trim( std::string &s )
{
  ltrim( rtrim(s) );
}//trim(...)

std::istream &SafeGetLine( std::istream &is, std::string &t, const size_t maxlength )
{
  //adapted from  http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
  t.clear();
    
  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.
  std::istream::sentry se( is, true );
  std::streambuf *sb = is.rdbuf();
    
  for( ; !maxlength || (t.length() < maxlength); )
    {
      int c = sb->sbumpc(); //advances pointer to current location by one
      switch( c )
	{
        case '\r':
          c = sb->sgetc();  //does not advance pointer to current location
          if(c == '\n')
            sb->sbumpc();   //advances pointer to one current location by one
          return is;
        case '\n':
          return is;
        case EOF:
          is.setstate( ios::eofbit );
          return is;
        default:
          t += (char)c;
	}//switch( c )
    }//for(;;)
    
  return is;
}//safe_get_line(...)

bool SplitLabelValues( const string &line, string &label, string &values )
{
  string tmpline = line;
  label.clear();
  values.clear();
  const size_t pos1 = tmpline.find_first_of( " \t,;" );
  if( !pos1 || pos1 == string::npos )
    return false;
  label = tmpline.substr(0, pos1);

  tmpline.erase(0,pos1);
  trim(tmpline);
  
  const size_t pos2 = tmpline.find_first_of( " \t,;" );
  if( !pos2 || pos2 == string::npos )
    return false;
  
  values = tmpline.substr(0,pos2);
  trim( values );
 
  return true;
}

bool read_config_file(const char * const filename, map<unsigned int,unsigned int> &label_to_values)
{
  string line;
    
  ifstream input( filename, ios::in | ios::binary );

  std::stringstream tran;//sstream cstring

  
  if( !input )
    {
      cerr << "Failed to open '" << filename << "'" << endl;
      return false;
    }


  while( SafeGetLine( input, line, LINESZ ) )
    {
      trim( line );
      if( line.empty() || line[0] == '#' )
        continue;

      // std::cout<<line<<std::endl;
      
      string label, values;
      const bool success = SplitLabelValues( line, label, values );
      // std::cout<<"Orig "<<label<<"  "<<values<<std::endl;
      
      if( !success || label.empty() || values.empty() )
	{
	  cerr << "Warning: encountered invalid config file line '" << line
	       << "', skipping" << endl;
	  continue;
	}

      unsigned int first,second;
      first = std::strtoul(label.c_str(),NULL,0);
      second = std::strtoul(values.c_str(),NULL,0);
      label_to_values[first] = second;

      // std::cout<<label<<" "<<first<<"   "<<values<<" "<<second<<std::endl;
    }
  
  return true;
}

int main(void)
{
  int fd;
  void *map_addr;
  int size = 4096;
  volatile unsigned int *mapped;
  int k, lbit, fbit;
  long long int revsn;

  char * data;
  data = getenv("QUERY_STRING");                           // retrieve webpage arguments   
  std::string webdata(data);                               // turn into string
  // printf("%s\n",webdata.c_str());

  std::string ss = webdata.substr(3);
  int num = atoi(ss.c_str());
  // printf("%d\n",num);

  
  map<unsigned int,unsigned int> label_to_values;
  std::map<unsigned int,unsigned int>::iterator it;
  // const char *settings_file = "settings.ini";
  char settings_file[32];
  if(num==0) sprintf(settings_file,"settings.ini");
  else sprintf(settings_file,"settings%d.ini",num);
    
  
  bool rval = read_config_file(settings_file,label_to_values);

  if( rval == 0 )
    {
      printf( "Failed to parse FPGA settings\n");
      return -1;
    }

  //unsigned int  mval, CW;
  unsigned int i2cdata[8];

  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
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
  

  // ******************* Main code begins ********************
  // first, set CSR run control options   
  mapped[ACSRIN] = 0x0000; // all off
  mapped[AOUTBLOCK] = OB_IOREG;	  // read from IO block


  // ********** TRIGGER CONTROL PARAMETERS ******************

  // set outputs of FPGA to LVDS buffers as tristate before programming the buffers' direction via I2C (below)
  mapped[AOUTENA+AOFFFA] = FPGAOUT_IS_OFF;// write to FPGA,
  mapped[AOUTENA+AOFFFB] = FPGAOUT_IS_OFF;// write to FPGA, 
  mapped[AOUTENA+AOFFFC] = FPGAOUT_IS_OFF;// write to FPGA, 


  // for (it = label_to_values.begin(); it != label_to_values.end(); ++it)
  //   {
  //     if(it->first == 0x100 || it->first == 0x101 || it->first == 0x102) continue;
  //     mapped[it->first] = it->second;
  //     if(mapped[it->first] != it->second) printf("Error writing register %d\n",it->first);
  //   }



  
  // check if there is a conflict: LVDS out - FPGA out: ok, pass though FPGA -> front
  //     out = 1                   LVDS out - FPGA in: ok, signal from DB -> FPGA and front
  //     in =  0                   LVDS in  - FPGA out: not allowed, conflict
  //                               LVDS in  - FPGA in: ok, pass though front -> FPGA
  unsigned int LVDS_A_OUTENA;
  unsigned int LVDS_B_OUTENA;
  unsigned int LVDS_C_OUTENA;
  unsigned int FRONT_A_OUTENA;
  unsigned int FRONT_B_OUTENA;
  unsigned int FRONT_C_OUTENA;
  it = label_to_values.find(0x100);
  if (it == label_to_values.end())
    {
      printf("Can't find register 0x100\n");
      return -1;
    }
  FRONT_A_OUTENA = it->second;
  it = label_to_values.find(0x101);
  if (it == label_to_values.end())
    {
      printf("Can't find register 0x101\n");
      return -1;
    }
  FRONT_B_OUTENA = it->second;
  it = label_to_values.find(0x102);
  if (it == label_to_values.end())
    {
      printf("Can't find register 0x102\n");
      return -1;
    }
  FRONT_C_OUTENA = it->second;
  
  it = label_to_values.find(0x105);
  if (it == label_to_values.end())
    {
      printf("Can't find register 0x105\n");
      return -1;
    }
  LVDS_A_OUTENA = it->second;
  it = label_to_values.find(0x106);
  if (it == label_to_values.end())
    {
      printf("Can't find register 0x106\n");
      return -1;
    }
  LVDS_B_OUTENA = it->second;
  it = label_to_values.find(0x107);
  if (it == label_to_values.end())
    {
      printf("Can't find register 0x107\n");
      return -1;
    }
  LVDS_C_OUTENA = it->second;

  
  for( k = 0; k <16; k++ )     // check each bit
    {
      lbit = (LVDS_A_OUTENA  >> k ) & 0x0001;
      fbit = (FRONT_A_OUTENA >> k ) & 0x0001;
      if(lbit==1 && fbit==0)  printf("FRONT/LVDS_A_OUTENA settings (bit %d) expect input from daughtercard\n",k); 
      if(lbit==0 && fbit==1)  
	{
	  printf("FRONT/LVDS_A output settings (bit %d) create a conflict\n",k); 
	  return -1;
	}

      lbit = (LVDS_B_OUTENA  >> k ) & 0x0001;
      fbit = (FRONT_B_OUTENA >> k ) & 0x0001;
      if(lbit==1 && fbit==0)  printf("FRONT/LVDS_B_OUTENA settings (bit %d) expect input from daughtercard\n",k); 
      if(lbit==0 && fbit==1)  
	{
	  printf("FRONT/LVDS_B output settings (bit %d) create a conflict\n",k); 
	  return -1;
	}

      lbit = (LVDS_C_OUTENA  >> k ) & 0x0001;
      fbit = (FRONT_C_OUTENA >> k ) & 0x0001;
      if(lbit==1 && fbit==0)  printf("FRONT/LVDS_C_OUTENA settings (bit %d) expect input from daughtercard\n",k); 
      if(lbit==0 && fbit==1)  
	{
	  printf("FRONT/LVDS_C output settings (bit %d) create a conflict\n",k); 
	  return -1;
	}
      // other conditions ok
    }

  
  // ************************ I2C programming *********************************

  // LVDS buffer direction for FRONT A-C is applied via FPGA's I2C
  // another device on the I2C bus is the thermometer / 4-byte memory

  // ---------------------- program LVDS input vs output  -----------------------

  // lower A
  I2Cstart(mapped);

  // I2C addr byte
  i2cdata[7] = 0;
  i2cdata[6] = 1;
  i2cdata[5] = 0;
  i2cdata[4] = 0;
  i2cdata[3] = 1;   // A2       // Front IO A, addresses [A2:A0] =  000, 100
  i2cdata[2] = 0;   // A1
  i2cdata[1] = 0;   // A0
  i2cdata[0] = 0;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
    {
      i2cdata[k] = ( LVDS_A_OUTENA >> k ) & 0x0001 ;
    }
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
  I2Cslaveack(mapped);

  I2Cstop(mapped);

  // upper A   
  I2Cstart(mapped);

  // I2C addr byte
  i2cdata[7] = 0;
  i2cdata[6] = 1;
  i2cdata[5] = 0;
  i2cdata[4] = 0;
  i2cdata[3] = 0;   // A2       // Front IO A, addresses [A2:A0] =  000, 100
  i2cdata[2] = 0;   // A1
  i2cdata[1] = 0;   // A0
  i2cdata[0] = 0;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
    {
      i2cdata[k] = ( LVDS_A_OUTENA >> (k+8) ) & 0x0001 ;
    }
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
  I2Cslaveack(mapped);

  I2Cstop(mapped);

   
  // lower B
  I2Cstart(mapped);

  // I2C addr byte
  i2cdata[7] = 0;
  i2cdata[6] = 1;
  i2cdata[5] = 0;
  i2cdata[4] = 0;
  i2cdata[3] = 1;   // A2       // Front IO B, addresses [A2:A0] =  001, 101
  i2cdata[2] = 0;   // A1
  i2cdata[1] = 1;   // A0
  i2cdata[0] = 0;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
    {
      i2cdata[k] = ( LVDS_B_OUTENA >> k ) & 0x0001 ;
    }
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
  I2Cslaveack(mapped);

  I2Cstop(mapped);

  // upper B  
  I2Cstart(mapped);

  // I2C addr byte
  i2cdata[7] = 0;
  i2cdata[6] = 1;
  i2cdata[5] = 0;
  i2cdata[4] = 0;
  i2cdata[3] = 0;   // A2       // Front IO B, addresses [A2:A0] =  001, 101
  i2cdata[2] = 0;   // A1
  i2cdata[1] = 1;   // A0
  i2cdata[0] = 0;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
    {
      i2cdata[k] = ( LVDS_B_OUTENA >> (k+8) ) & 0x0001 ;
    }
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
  I2Cslaveack(mapped);

  I2Cstop(mapped);

  
  // lower C
  I2Cstart(mapped);

  // I2C addr byte
  i2cdata[7] = 0;
  i2cdata[6] = 1;
  i2cdata[5] = 0;
  i2cdata[4] = 0;
  i2cdata[3] = 1;   // A2       // Front IO C, addresses [A2:A0] =  010, 110
  i2cdata[2] = 1;   // A1
  i2cdata[1] = 0;   // A0
  i2cdata[0] = 0;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
    {
      i2cdata[k] = ( LVDS_C_OUTENA >> k ) & 0x0001 ;
    }
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
  I2Cslaveack(mapped);

  I2Cstop(mapped);

  // upper C   
  I2Cstart(mapped);

  // I2C addr byte
  i2cdata[7] = 0;
  i2cdata[6] = 1;
  i2cdata[5] = 0;
  i2cdata[4] = 0;
  i2cdata[3] = 0;   // A2       // Front IO C, addresses [A2:A0] =  010, 110
  i2cdata[2] = 1;   // A1
  i2cdata[1] = 0;   // A0
  i2cdata[0] = 0;   // R/W*
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  for( k = 0; k <8; k++ )     // NCHANNELS*2 gains, but 8 I2C bits
    {
      i2cdata[k] = ( LVDS_C_OUTENA >> (k+8) ) & 0x0001 ;
    }
  I2Cbytesend(mapped, i2cdata);
  I2Cslaveack(mapped);

  // I2C data byte
  I2Cbytesend(mapped, i2cdata);      // send same bits again for enable?
  I2Cslaveack(mapped);

  I2Cstop(mapped);

  
  // ************************ end I2C *****************************************

  // now enable FPGA front panel outputs

  for (it = label_to_values.begin(); it != label_to_values.end(); ++it)
    {
      mapped[it->first] = it->second;
      if(mapped[it->first] != it->second) printf("Error writing register %d\n",it->first);
    }


    
  // MZ TrigIO board temperature
  printf("Board temperature: %d C \n",(int)board_temperature(mapped) );

  // ***** ZYNQ temperature
  printf("Zynq temperature: %d C \n",(int)zynq_temperature() );

  // ***** check HW info *********
  revsn = hwinfo(mapped);
  printf("Board unique ID 0x%016llX, Serial Number %llu \n",revsn, (revsn>>32) & 0xFFFF);

 
  // clean up  
  flock( fd, LOCK_UN );
  munmap(map_addr, size);
  close(fd);
  return 0;
}

// 
// progfippi.cc ends here
