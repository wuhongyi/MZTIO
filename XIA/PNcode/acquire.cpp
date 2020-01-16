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
#define __STDC_FORMAT_MACROS

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
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <math.h>
#include <stdint.h>
// need to compile with -lm option

#include <fstream>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "PixieNetDefs.h"

#include "PixieNetConfig.h"
extern "C" {
   #include "PixieNetCommon.h"
   }
 
using namespace std;
     
//compile  options to speed up processing      
#define PixieNetHit_HAS_WAVEFORM 1     // optionally suppress waveforms
#define PixieNetHit_RECORD_HIT_PSA 1   // optionally ignore PSA results
#define MAX_ACQ_TL 512                 // set limit for waveforms length (abs max: 4092)
#define SUMMCA 1                       // 


/* ********************************************************************************
   ********************************************************************************
   ********************************************************************************
   * DECLARATIONS
   ********************************************************************************
   ******************************************************************************** */

/* typedef struct PixieNetHit400 {  
  uint8_t channel;
  uint32_t hit;
  uint32_t timeH;
  uint32_t timeL;
  uint16_t energy;
#if( PixieNetHit_RECORD_HIT_PSA )
  uint32_t psa0;
  uint32_t psa1;
  uint32_t cfd0;
  uint32_t cfd1;
#endif  
#if( PixieNetHit_HAS_WAVEFORM ) 
  uint16_t num_waveform;       //Lockless queue can not be used with non-trivial constructor, so can't use a vector.
  uint16_t waveform[MAX_TL];  //max size of MAX_TL (4092)
  uint16_t NumCurrTraceBlks;      // number of trace blocks
  uint16_t NumPrevTraceBlks;      // number of trace blocks
#endif
} PixieNetHit400;//struct PixieNetHit400
*/

typedef struct PixieNetHit402 {  
  uint8_t channel;
  uint32_t hit; 
  uint32_t evtimeH;
  uint32_t evtimeL;
  uint32_t PPStime;
  uint32_t time0;
  uint32_t time1;
  uint32_t time2;
  uint32_t time3;
  uint16_t energy0;
  uint16_t energy1;
  uint16_t energy2;
  uint16_t energy3;
  uint16_t NumUserDataBlks;
  uint16_t Esum;
#if( PixieNetHit_RECORD_HIT_PSA )   // really would need NCHANNELS of these, but currently no group PSA supported
  uint32_t psa0;
  uint32_t psa1;
  uint32_t cfd0;
  uint32_t cfd1;
#endif
#if( PixieNetHit_HAS_WAVEFORM ) 
  uint16_t NumCurrTraceBlks;      // number of trace blocks
  uint16_t NumPrevTraceBlks;      // number of trace blocks
  uint16_t num_waveform0;       //Lockless queue can not be used with non-trivial constructor, so can't use a vector.
  uint16_t waveform0[MAX_ACQ_TL];  //max size of MAX_TL (4092), but typically less as defined above
  uint16_t NumTraceBlks0;      // number of trace blocks
  uint16_t num_waveform1;       //Lockless queue can not be used with non-trivial constructor, so can't use a vector.
  uint16_t waveform1[MAX_ACQ_TL];  //max size of MAX_TL (4092)
  uint16_t NumTraceBlks1;      // number of trace blocks
  uint16_t num_waveform2;       //Lockless queue can not be used with non-trivial constructor, so can't use a vector.
  uint16_t waveform2[MAX_ACQ_TL];  //max size of MAX_TL (4092)
  uint16_t NumTraceBlks2;      // number of trace blocks
  uint16_t num_waveform3;       //Lockless queue can not be used with non-trivial constructor, so can't use a vector.
  uint16_t waveform3[MAX_ACQ_TL];  //max size of MAX_TL (4092)
  uint16_t NumTraceBlks3;      // number of trace blocks
#endif
} PixieNetHit402;//struct PixieNetHit402
 


/** A global variable that gets set to true if the ctrl-c interupt is detected.
    If this happens, the data collection loop is terminated, and everything else
    exits as normal.                                      */
boost::atomic<bool> g_datataking_stop_requested;


/** Writes listmode data in the queue to disk 
    Uses a lockfree queue to minimize delays of locking a conventional queue,
    however, when the queue becomes empty, the function then waits on the 
    condition_variable to recieve a notifiaction more data is available; this
    is to reduce CPU usage.
    Once taking_data is false, and the queue is empty, this function returns.
    Use this function for RUN_TYPE 0x400, 0x500 or 0x501  */
void write_lm_data400( FILE *outfile, 
                    uint32_t histogram[NCHANNELS+1][MAX_MCA_BINS],
                    uint32_t wmca[NCHANNELS+1][WEB_MCA_BINS],
                    boost::atomic<bool> &taking_data,
                    boost::lockfree::queue<PixieNetHit402> &hit_queue,
                    boost::condition_variable &cv,
                    boost::atomic<size_t> &num_wrote,
                    unsigned int BINFACTOR[NCHANNELS] );


/** equivalent for RUN_TYPE 0x402 */
void write_lm_data402( FILE *outfile, 
                    uint32_t histogram[NCHANNELS+1][MAX_MCA_BINS],
                    uint32_t wmca[NCHANNELS+1][WEB_MCA_BINS],
    //       #if(SUMMCA)
    //                uint32_t sumhistogram[MAX_MCA_BINS],
    //                uint32_t swmca[WEB_MCA_BINS],
    //       #endif
                    boost::atomic<bool> &taking_data,
                    boost::lockfree::queue<PixieNetHit402> &hit_queue,
                    boost::condition_variable &cv,
                    boost::atomic<size_t> &num_wrote,
                    unsigned int BINFACTOR[NCHANNELS] );


/** Accumulates data into histogram, and does not save list mode data.  
    Otherwise functions similar to write_lm_data.
    Use this function for RUN_TYPE 0x301   */
void histogram_lm_data( uint32_t histogram[NCHANNELS+1][MAX_MCA_BINS],
                        uint32_t wmca[NCHANNELS+1][WEB_MCA_BINS],
                        boost::atomic<bool> &taking_data,
                        boost::lockfree::queue<PixieNetHit402> &hit_queue,
                        boost::condition_variable &cv,
                        boost::atomic<size_t> &num_wrote,
                        unsigned int BINFACTOR[NCHANNELS] );


/** Sets g_datataking_stop_requested to true, which stops data taking so program
    can exit.   */
void handle_interupt( int s );


struct RunOptions
{
  string listmode_output_name, mca_output_name;
};//struct RunOptions


typedef struct PixieNetRunningStats {
  /** Number of times that it has been checked wether or not there are any 
      events waiting in the FPGAs buffer to copy over to the linux side of 
      things.    */
  uint64_t numchecks;
  
  /** Number of times there have been any events waiting in the FPGAs buffer,
      wether there was one or four, or they were rejected or kept.
      Note that at each checking, a maximum of one event from each channel is
      copied over.    */
  uint64_t collectionnum;
  
  /** Number of events accepted so far, for all channels. */
  uint64_t eventcount;
  
  /** Number of accepted events for each channel, so far*/
  uint64_t numaccepted[NCHANNELS];
  
  /** Number of rejected events for each channel, so far */
  uint64_t numrejected[NCHANNELS];

    /** Number of trace blocks to follow */
  uint64_t numtraceblocks;
  
  /** Some (I believe) temporary variables that I think will be moved to the
      FPGA, but we'll put them here for now */
  double baseline[NCHANNELS];
  double C0[NCHANNELS], C1[NCHANNELS], Cg[NCHANNELS];
} PixieNetRunningStats;


/** Zeros out PixieNetRunningStats */
void init_PixieNetRunningStats( PixieNetRunningStats *stats );

 
/** writes the data to file all RUN_TYPEs except 0x402 */
int PixieNetHit_write_400( FILE *instrm, const PixieNetHit402 * const hit );


/** writes the data to file (mode 0x402) */
int PixieNetHit_write_402( FILE *instrm, const PixieNetHit402 * const hit );


/** reads the data from FPGA and puts into "hits" record */
unsigned int collect_PixieNet_lm_data400( volatile unsigned int *mapped,
                              PixieNetHit402 hits[NCHANNELS],
                              PixieNetRunningStats *runstats,
                              const PixieNetFippiConfig *fippiconfig );

/** reads the data from FPGA and puts into "hits" record, special for RUN_TYPE 0x402 */
unsigned int collect_PixieNet_lm_data402( volatile unsigned int *mapped,
                              PixieNetHit402 hits[NCHANNELS],
                              PixieNetRunningStats *runstats,
                              const PixieNetFippiConfig *fippiconfig );

/** Returns a zero or positive value on success */
int init_configurations( int argc, char **argv,
                         RunOptions &options,
                         PixieNetFippiConfig &fippiconfig );



/* ********************************************************************************
   ********************************************************************************
   ********************************************************************************
   * MAIN
   ********************************************************************************
   ******************************************************************************** */


int main( int argc, char **argv )
{
  
  int TL;
  unsigned int BLbad[NCHANNELS];
  unsigned int BLcut[NCHANNELS], BLavg[NCHANNELS];

  //Set the handler for if the user hits ctrl-c
  g_datataking_stop_requested = false;
  
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = handle_interupt;
  sigemptyset( &sigIntHandler.sa_mask );
  sigIntHandler.sa_flags = 0;
  sigaction( SIGINT, &sigIntHandler, NULL );
   
  
  // --------------------------------------------------------
  // ------ Start setting up the PIXIE-NET  -------
  // --------------------------------------------------------

  const string settings_file = "settings.ini";
  RunOptions options;
  PixieNetFippiConfig fippiconfig;
  
  if( init_configurations( argc, argv, options, fippiconfig ) < 0 )
  {
    return EXIT_FAILURE;
  }
  
  cout << "Succeded in parsing config/settings files" << endl;

  
  // *************** PS/PL IO initialization *********************
  // open the device for PD register I/O
  const int device_fd_PL = open("/dev/uio0", O_RDWR);
  if( device_fd_PL < 0 ) {
    perror("Failed to open PL devfile");
    return -1;
  }
  
  void *map_addr = mmap( NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, device_fd_PL, 0);
  
  if( map_addr == MAP_FAILED ) {
    perror("Failed to mmap");
    return -2;
  }
  
  //Lock the fipi device so multiple programs cant step on eachother.
  if( flock( device_fd_PL, LOCK_EX | LOCK_NB ) )
  {
    cerr << "Failed to get file lock on /dev/uio0" << endl;
    munmap( map_addr, 4096 );
    close( device_fd_PL );
    return -3;
  }
  
  volatile unsigned int *mapped = (unsigned int *) map_addr;
   
   // **********  Compute Coefficients for E Computation and other initialization ************
  PixieNetRunningStats runstats;
  init_PixieNetRunningStats( &runstats );             


  for( int k = 0; k < NCHANNELS; k ++ )
  {
    const double dt = 1.0 / FILTER_CLOCK_MHZ;

    // multiply time in us *  # ticks per us = time in ticks
    const int SL = (int)floor( fippiconfig.ENERGY_RISETIME[k]*FILTER_CLOCK_MHZ );
    //const int SG = (int)floor( fippiconfig.ENERGY_FLATTOP[k]*FILTER_CLOCK_MHZ );
    
    const double q = exp( -1.0 * dt / fippiconfig.TAU[k] );
    const double elm = exp( -1.0 * dt * SL / fippiconfig.TAU[k] );
    
    runstats.C0[k] = (q - 1.0) * elm / (1.0 - elm);
    runstats.Cg[k] = 1.0 - q;
    runstats.C1[k] = (1.0 - q) / (1.0 - elm);
    
    runstats.C0[k] = runstats.C0[k] * fippiconfig.DIG_GAIN[k];
    runstats.Cg[k] = runstats.Cg[k] * fippiconfig.DIG_GAIN[k];
    runstats.C1[k] = runstats.C1[k] * fippiconfig.DIG_GAIN[k];

    BLcut[k]       = fippiconfig.BLCUT[k];
    BLavg[k]       = 65536 - fippiconfig.BLAVG[k];
    if(BLavg[k]<0)          BLavg[k] = 0;
    if(BLavg[k]==65536)     BLavg[k] = 0;
    if(BLavg[k]>MAX_BLAVG)  BLavg[k] = MAX_BLAVG;

    BLbad[k] = MAX_BADBL;   // initialize to indicate no good BL found yet

    
    TL = (int)floor(fippiconfig.TRACE_LENGTH[k]*ADC_CLK_MHZ);
    if( (fippiconfig.RUN_TYPE != 0x301 ) && (TL >  MAX_ACQ_TL) )
    {
         cerr << "Compile option limits TRACE_LENGTH to " << MAX_ACQ_TL << " samples, exiting." << endl; 
         cerr << "(Shorten TRACE_LENGTHs or modify #defines in acquire.cpp and recompile.)" << endl; 
         flock( device_fd_PL, LOCK_UN );
         munmap( map_addr, 4096 );
         close( device_fd_PL );
         return -6;
    }
  }


  // ***** check HW info *********
  int revsn = hwinfo(mapped);
 
  cout << "Initialized filters" << endl;

  
  // ********************** Run Start **********************

  // run type check
  if( fippiconfig.RUN_TYPE == 0x301 ||
      fippiconfig.RUN_TYPE == 0x400 ||
      fippiconfig.RUN_TYPE == 0x402 )
  {    
      //ok, do nothing
  } else  {
     cerr << "This function only supports runtypes 0x301, 0x400, 0x402 for now, exiting" << endl; 
      flock( device_fd_PL, LOCK_UN );
      munmap( map_addr, 4096 );
      close( device_fd_PL );
      return -5;
  }

   // though we checked runtypes above, here for completeness treat all LM runtypes
  string listmodeoutname = options.listmode_output_name;
  if ( fippiconfig.RUN_TYPE == 0x400) listmodeoutname += ".b00";         // depends on runtype
  if ( fippiconfig.RUN_TYPE == 0x402) listmodeoutname += ".b00";         // depends on runtype
  if ( fippiconfig.RUN_TYPE == 0x500) listmodeoutname += ".txt";         // depends on runtype
  if ( fippiconfig.RUN_TYPE == 0x501) listmodeoutname += ".dat";         // depends on runtype
  if ( fippiconfig.RUN_TYPE == 0x502) listmodeoutname += ".dt2";         // depends on runtype
  if ( fippiconfig.RUN_TYPE == 0x503) listmodeoutname += ".dt4";         // depends on runtype
  
  FILE *lmout = NULL;
  FILE *filmca = NULL;
  
  if( fippiconfig.RUN_TYPE == 0x400 || 
      fippiconfig.RUN_TYPE == 0x402 || 
      fippiconfig.RUN_TYPE == 0x500 || 
      fippiconfig.RUN_TYPE == 0x501 || 
      fippiconfig.RUN_TYPE == 0x502 || 
      fippiconfig.RUN_TYPE == 0x503    )
  {
    lmout = fopen( listmodeoutname.c_str(), "wb");
    if( lmout == NULL )
    {
      cerr << "Failed to open '" << listmodeoutname << "', exiting" << endl;
      flock( device_fd_PL, LOCK_UN );
      munmap( map_addr, 4096 );
      close( device_fd_PL );
      return -4;
    }
    
     if( fippiconfig.RUN_TYPE == 0x400 ||
         fippiconfig.RUN_TYPE == 0x402 )
     {
        // write a 0x400, 0x402 header
        // fwrite is really slow (like very significant impact on thoroughput of events
        // slow), so we will write to a buffer, and then to the file.
        uint16_t buffer[FILE_HEAD_LENGTH_400] = {0};
        buffer[0] = BLOCKSIZE_400;
        buffer[1] = 0;                                       // module number (get from settings file?)
        buffer[2] = fippiconfig.RUN_TYPE;
        buffer[3] = CHAN_HEAD_LENGTH_400;
        buffer[4] = fippiconfig.COINCIDENCE_PATTERN;
        buffer[5] = fippiconfig.COINCIDENCE_WINDOW;
        buffer[7] = revsn>>16;               // HW revision from EEPROM
        buffer[12] = revsn & 0xFFFF;         // serial number from EEPROM
        for( unsigned int ch = 0; ch < NCHANNELS; ch++) {
            TL = (int)floor(fippiconfig.TRACE_LENGTH[ch]*ADC_CLK_MHZ);
            buffer[6]   +=(int)floor((TL + CHAN_HEAD_LENGTH_400) / BLOCKSIZE_400);         // combined event length, in blocks
            buffer[8+ch] =(int)floor((TL + CHAN_HEAD_LENGTH_400) / BLOCKSIZE_400);			// each channel's event length, in blocks
        }
        if( fippiconfig.RUN_TYPE == 0x402)  {
            buffer[6]   -=(NCHANNELS-1);     // only one event header for all 4 channels in 0x402
        }
        fwrite( buffer, 2, FILE_HEAD_LENGTH_400, lmout );     // write to file
     } else  {
         cerr << "This function only supports runtypes 0x301, 0x400, 0x402 for now, exiting" << endl;   // MCA run 0x301 also ok
         flock( device_fd_PL, LOCK_UN );
         munmap( map_addr, 4096 );
         close( device_fd_PL );
         fclose( lmout );
         return -5;
     }

  }//if( saving LM data header )
 

  const boost::posix_time::ptime starttime = boost::posix_time::second_clock::local_time();
  //cout << "Start time: " << starttime << endl;

  if( fippiconfig.SYNC_AT_START )
    mapped[ARTC_CLR] = 1;   // write to reset time counter

  mapped[AOUTBLOCK] = 2;
//  unsigned int startTS = mapped[AREALTIME];
  const std::string starttimestr = boost::posix_time::to_iso_extended_string( starttime );

//#if(SUMMCA) 
  unsigned int histogram[NCHANNELS+1][MAX_MCA_BINS] = { {0} };  // full 32K spectrum for final output, 1 extra for sum
  unsigned int wmca[NCHANNELS+1][WEB_MCA_BINS] = { {0} };       // 4K spectrum for faster web update
//#else
//  unsigned int histogram[NCHANNELS][MAX_MCA_BINS] = { {0} };  // full 32K spectrum for final output
//  unsigned int wmca[NCHANNELS][WEB_MCA_BINS] = { {0} };       // 4K spectrum for faster web update
//#endif

//#if(SUMMCA)  declare always and print to MCA always, even if unused and all zero
//  unsigned int sumhistogram[MAX_MCA_BINS] =  {0} ;  // full 32K spectrum for final output
//  unsigned int swmca[WEB_MCA_BINS] =  {0} ;       // 4K spectrum for faster web update      
//#endif

  boost::atomic<bool> taking_data( true );
  boost::lockfree::queue<PixieNetHit402> hit_queue(32*1024);
  boost::atomic<size_t> num_wrote( 0 );
  boost::condition_variable notifier;
  boost::scoped_ptr<boost::thread> writing_thread;
  
  if( fippiconfig.RUN_TYPE == 0x400 || fippiconfig.RUN_TYPE == 0x500 || fippiconfig.RUN_TYPE == 0x501 )
  {
    writing_thread.reset( new boost::thread( boost::bind(write_lm_data400,lmout,histogram,wmca,
                                                       boost::ref(taking_data),
                                                       boost::ref(hit_queue),
                                                       boost::ref(notifier),
                                                       boost::ref(num_wrote),
                                                       fippiconfig.BINFACTOR) ) );
  } 
  
  if( fippiconfig.RUN_TYPE == 0x402 )
  {
      writing_thread.reset( new boost::thread( boost::bind(write_lm_data402,lmout,histogram,wmca,
                                          //         #if(SUMMCA)
                                          //            sumhistogram,swmca,
                                          //         #endif
                                                       boost::ref(taking_data),
                                                       boost::ref(hit_queue),
                                                       boost::ref(notifier),
                                                       boost::ref(num_wrote),
                                                       fippiconfig.BINFACTOR) ) );
  }                                            


  
  if( fippiconfig.RUN_TYPE == 0x301 )
  {
    writing_thread.reset( new boost::thread( boost::bind(histogram_lm_data,histogram,wmca,
                                                      boost::ref(taking_data),
                                                      boost::ref(hit_queue),
                                                      boost::ref(notifier),
                                                      boost::ref(num_wrote),
                                                      fippiconfig.BINFACTOR) ) );
  }//
  

  const boost::posix_time::time_duration runtimelimit = boost::posix_time::millisec( static_cast<int>(1000*fippiconfig.REQ_RUNTIME) );
  cout << "Will run for " << runtimelimit << endl;

  mapped[ADSP_CLR] = 1;             // write to reset DAQ buffers
  mapped[ACOUNTER_CLR] = 1;         // write to reset RS counters
  mapped[ACSRIN] = 1;               // set RunEnable bit to start run
  mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers
    
  
  // ********************** Run Loop **********************

 PixieNetHit402 hits[NCHANNELS];
// PixieNetHit400 hits[NCHANNELS];

  do 
  {
  
    //----------- Periodically read BL and update average -----------
    // this will be moved into the FPGA soon
    if( (runstats.numchecks % BLREADPERIOD) == 0 )
    {
      for( unsigned int ch = 0; ch < NCHANNELS; ch++)
      {
        // read raw BL sums
        const unsigned int chaddr = ch*16+16;
        const unsigned int lsum = mapped[chaddr+CA_LSUMB];
        const unsigned int tsum = mapped[chaddr+CA_TSUMB];
        const unsigned int gsum = mapped[chaddr+CA_GSUMB];
        if( tsum > 0 )		// tum=0 indicates bad baseline
        {
          const double ph = runstats.C1[ch]*lsum + runstats.Cg[ch]*gsum + runstats.C0[ch]*tsum;
          if(  (BLcut[ch]==0) || 
               (abs(ph-runstats.baseline[ch])<BLcut[ch]) ||   // only accept "good" baselines < BLcut,
               (BLbad[ch] >=MAX_BADBL) )                          // or if too many bad in a row (to start over)
          {
               if( (BLavg[ch]==0) || (BLbad[ch] >=MAX_BADBL) )
               {
                   runstats.baseline[ch] = ph;
                   BLbad[ch] = 0;
               } else {
                   // BL average: // avg = old avg + (new meas - old avg)/2^BLavg
                   runstats.baseline[ch] = runstats.baseline[ch] + (ph-runstats.baseline[ch])/(1<<BLavg[ch]);
                   BLbad[ch] = 0;
               } // end BL avg
          } else {
               BLbad[ch] = BLbad[ch]+1;
          }    // end BLcut check
        }      // if( tsum > 0 )
      }        // for( loop over channels )
    }          // if( should update baseline )




    // -----------poll for events -----------
    // if data ready. read out, compute E, increment MCA *********
    unsigned int nhits;
    if( fippiconfig.RUN_TYPE == 0x402)
    {
        nhits = collect_PixieNet_lm_data402( mapped, hits, &runstats, &fippiconfig );
        for( size_t i = 0; i < nhits; ++i )    // nhits = 0 to 4 
          if( !hit_queue.push( hits[i] ) )
             cerr << "Failed to push onto queue" << endl;

    } else {
        nhits = collect_PixieNet_lm_data400( mapped, hits, &runstats, &fippiconfig );
        for( size_t i = 0; i < nhits; ++i )    // nhits = 0 to 4 
          if( !hit_queue.push( hits[i] ) )
            cerr << "Failed to push onto queue" << endl;
    }
 
    if( nhits )
      notifier.notify_one();

     
    // ----------- Periodically save MCA and RS -----------

   if( (runstats.numchecks % fippiconfig.POLL_TIME) == 0 )
   {
       // 1) Run Statistics 
       mapped[AOUTBLOCK] = OB_RSREG;         // read from RS registers
       read_print_runstats(1, 0, mapped);    // print (small) set of RS to file, visible to web
       mapped[AOUTBLOCK] = OB_EVREG;         // read from event registers
      
       // 2) MCA
       filmca = fopen("MCA.csv","w");
       if( fippiconfig.RUN_TYPE == 0x402) {
         fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3,MCAsum\n");
       } else {
         fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3\n");
       }
       int onlinebin = (int)floor(MAX_MCA_BINS/WEB_MCA_BINS);
       for( int k=0; k <WEB_MCA_BINS; k++)       // report the 4K spectra during the run (faster web update)
       {
         if( fippiconfig.RUN_TYPE == 0x402) {
            fprintf(filmca,"%d,%u,%u,%u,%u,%u\n ", k*onlinebin,wmca[0][k],wmca[1][k],wmca[2][k],wmca[3][k],wmca[4][k]);
         } else {
            fprintf(filmca,"%d,%u,%u,%u,%u\n ", k*onlinebin,wmca[0][k],wmca[1][k],wmca[2][k],wmca[3][k]);
         }
       }
       fclose(filmca); 
    }

    // ----------- check if we've run long enough -------------------
    const boost::posix_time::ptime currenttime = boost::posix_time::second_clock::local_time();
    const boost::posix_time::time_duration dur = currenttime - starttime;

    if( dur >= runtimelimit )
      break;
  } while ( !g_datataking_stop_requested );

  
  // ********************** Run Stop **********************
  //  const boost::posix_time::ptime endtime = boost::posix_time::second_clock::local_time();

  // clear RunEnable bit to stop run
  mapped[ACSRIN] = 0;

  size_t nwritensofar = num_wrote;
  cout << "Done taking data" << endl;

  // Grab any remaining events
  unsigned int nhitsnow;

    if( fippiconfig.RUN_TYPE == 0x402)
    {    
      do
      {  
        nhitsnow = collect_PixieNet_lm_data402( mapped, hits, &runstats, &fippiconfig );
        for( size_t i = 0; i < nhitsnow; ++i )
          hit_queue.push( hits[i] );
        
        notifier.notify_one();
      
      }while( nhitsnow );
      
      while( !hit_queue.empty() )
      {
         notifier.notify_one(); //just to make sure...
      }
    } else {
      do
      {  
        nhitsnow = collect_PixieNet_lm_data400( mapped, hits, &runstats, &fippiconfig );
        for( size_t i = 0; i < nhitsnow; ++i )
          hit_queue.push( hits[i] );
         
        notifier.notify_one();
      
      }while( nhitsnow );
      
      while( !hit_queue.empty() )
      {
         notifier.notify_one(); //just to make sure...
      }
    }     // end if RUN_TYPE

  
  

  
  taking_data = false;
  notifier.notify_one();  //make sure and wake that thread up
  writing_thread->join();
  
  size_t ntotalwritten = num_wrote;

  cout << "Done writing data, there were " << (ntotalwritten-nwritensofar) << " in queue by end" << endl;

  if( fippiconfig.RUN_TYPE != 0x301 )
  {
     // write EOR: special hit pattern, all zero except EORMARK and WM indicates end of run data
     uint8_t buffer[CHAN_HEAD_LENGTH_400*2] = {0};
     uint32_t wm = EORMARK;
     memcpy( buffer + 0, &(wm), 4 );
#if( PixieNetHit_HAS_WAVEFORM ) 
     // TODO: write PrevNumTraceBlks
#endif
     wm = WATERMARK;
     memcpy( buffer + 60, &(wm), 4 );
     fwrite( buffer, 1, CHAN_HEAD_LENGTH_400*2, lmout );
    
     if( lmout != NULL )
       fclose( lmout );
   }

  
  // ----------- Final save MCA and RS -----------
  // 1) Run Statistics 
  mapped[AOUTBLOCK] = OB_RSREG;         // read from RS registers
  read_print_runstats(0, 0, mapped);    // print (small) set of RS to file, visible to web
  mapped[AOUTBLOCK] = OB_EVREG;         // read from event registers
   
  // 2) MCA
  // once for the default filename
  filmca = fopen("MCA.csv","w");
  if( fippiconfig.RUN_TYPE == 0x402) {
      fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3,MCAsum\n");
  } else {
      fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3\n");
  }
  unsigned int k;
  for( k=0; k <MAX_MCA_BINS; k++)
  {
    if( fippiconfig.RUN_TYPE == 0x402) {
      fprintf(filmca,"%d,%u,%u,%u,%u,%u\n ", k,histogram[0][k],histogram[1][k],histogram[2][k],histogram[3][k],histogram[4][k] );
    } else {
      fprintf(filmca,"%d,%u,%u,%u,%u\n ", k,histogram[0][k],histogram[1][k],histogram[2][k],histogram[3][k] );
    }
     
  }
  fclose(filmca);

  /*
  // once for the named file name
  // if <different names>
  string mcaoutname = options.listmode_output_name;
  mcaoutname += ".csv";          
  FILE *filmca = NULL;
  filmca = fopen( mcaoutname.c_str(), "w");
  fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3\n");
  unsigned int k;
  for( k=0; k <MAX_MCA_BINS; k++)
  {
     fprintf(filmca,"%d,%u,%u,%u,%u\n ", k,histogram[0][k],histogram[1][k],histogram[2][k],histogram[3][k] );
  }
  fclose(filmca);
  */

  cout << "Done writing MCA and run statistics." << endl;

  // clean up
  flock( device_fd_PL, LOCK_UN );
  munmap( map_addr, 4096 );
  close( device_fd_PL );
  return 0;
}



/* ********************************************************************************
   ********************************************************************************
   ********************************************************************************
   * SUBROUTINES
   ********************************************************************************
   ******************************************************************************** */


/* ********************************************************************************
   *   interrupt handler
   ******************************************************************************** */

void handle_interupt( int s )
{
  printf( "Caught signal %d\nWill stop taking data.", s );
  g_datataking_stop_requested = true;
}//void handle_interupt( int s );


/* ********************************************************************************
   *   subroutine to histogram and write LM data to file (0x400)
   ******************************************************************************** */

void write_lm_data400( FILE *outfile, 
                    uint32_t histogram[NCHANNELS+1][MAX_MCA_BINS],
                    uint32_t wmca[NCHANNELS+1][WEB_MCA_BINS],
                    boost::atomic<bool> &taking_data,
                    boost::lockfree::queue<PixieNetHit402> &hit_queue,
                    boost::condition_variable &cv,
                    boost::atomic<size_t> &num_wrote,
                    unsigned int BINFACTOR[NCHANNELS] )
{
  PixieNetHit402 hit;
  boost::mutex local_mutex;
  
  while( taking_data )
  {
    boost::unique_lock<boost::mutex> lock( local_mutex );
    cv.wait( lock );
    
    while( hit_queue.pop(hit) )
    {
       // histogram
      const uint32_t energy_bin = (hit.energy0 >> BINFACTOR[hit.channel]);
      
      const unsigned int bin = std::max(std::min(energy_bin,static_cast<unsigned int>(MAX_MCA_BINS-1)),0u);
      histogram[hit.channel][bin] += 1;
      const unsigned int binw = bin >> WEB_LOGEBIN;
      wmca[hit.channel][binw] += 1;

      // write LM file
      PixieNetHit_write_400( outfile, &hit );

      num_wrote += 1;
    }
  }//
}//write_lm_data(...)


/* ********************************************************************************
   *   subroutine to histogram and write LM data to file (0x402)
   ******************************************************************************** */

void write_lm_data402( FILE *outfile, 
                    uint32_t histogram[NCHANNELS+1][MAX_MCA_BINS],
                    uint32_t wmca[NCHANNELS+1][WEB_MCA_BINS],
      //     #if(SUMMCA)
      //              uint32_t sumhistogram[MAX_MCA_BINS],
      //              uint32_t swmca[WEB_MCA_BINS],
      //     #endif
                    boost::atomic<bool> &taking_data,
                    boost::lockfree::queue<PixieNetHit402> &hit_queue,
                    boost::condition_variable &cv,
                    boost::atomic<size_t> &num_wrote,
                    unsigned int BINFACTOR[NCHANNELS] )
{
  PixieNetHit402 hit;
  boost::mutex local_mutex;
  uint32_t energy_bin[NCHANNELS];
  unsigned int bin, binw;
  
  while( taking_data )
  {
    boost::unique_lock<boost::mutex> lock( local_mutex );
    cv.wait( lock );
    
    while( hit_queue.pop(hit) )
    {
       // histogram singles
       energy_bin[0] = (hit.energy0 >> BINFACTOR[0]);         // get energies from record
       energy_bin[1] = (hit.energy1 >> BINFACTOR[1]);
       energy_bin[2] = (hit.energy2 >> BINFACTOR[2]);
       energy_bin[3] = (hit.energy3 >> BINFACTOR[3]);
   
       for( int k = 0; k < NCHANNELS; k ++ )                 // increment MCAs in a loop
       {          
            bin = std::max(std::min(energy_bin[k],static_cast<unsigned int>(MAX_MCA_BINS-1)),0u);
            histogram[k][bin] += 1;
            binw = bin >> WEB_LOGEBIN;
            wmca[k][binw] += 1;
       }
 
//#if(SUMMCA)     
       // histogram sum
       energy_bin[0] = energy_bin[0] + energy_bin[1] + energy_bin[2] + energy_bin[3];
       bin = std::max(std::min(energy_bin[0],static_cast<unsigned int>(MAX_MCA_BINS-1)),0u);
       histogram[4][bin] += 1;
       binw = bin >> WEB_LOGEBIN;
       wmca[4][binw] += 1;
//#endif
  
       // write LM file
       PixieNetHit_write_402( outfile, &hit );
       num_wrote += 1;
    }
  }//
}//write_lm_data(...)


/* ********************************************************************************
   *   subroutine to histogram only
   ******************************************************************************** */
 
void histogram_lm_data(uint32_t histogram[NCHANNELS+1][MAX_MCA_BINS],
                       uint32_t wmca[NCHANNELS+1][WEB_MCA_BINS],
                       boost::atomic<bool> &taking_data,
                       boost::lockfree::queue<PixieNetHit402> &hit_queue,
                       boost::condition_variable &cv,
                       boost::atomic<size_t> &num_wrote,
                       unsigned int BINFACTOR[NCHANNELS] )
{
  PixieNetHit402 hit;
  boost::mutex local_mutex;
  
  while( taking_data )
  {
    boost::unique_lock<boost::mutex> lock( local_mutex );
    cv.wait( lock );
    
    while( hit_queue.pop(hit) )
    {
      const uint32_t energy_bin = (hit.energy0 >> BINFACTOR[hit.channel]);
      const unsigned int bin = std::max(std::min(energy_bin,static_cast<unsigned int>(MAX_MCA_BINS-1)),0u);
      histogram[hit.channel][bin] += 1;
      const unsigned int binw = bin >> WEB_LOGEBIN;
      wmca[hit.channel][binw] += 1;
      num_wrote += 1;
    }
  }//
}//write_lm_data(...)



  /* ********************************************************************************
   *   subroutine to apply command line options and read settings file
   ******************************************************************************** */

    
int init_configurations( int argc, char **argv,
                    RunOptions &options,
                    PixieNetFippiConfig &fippiconfig)
{

  string settingsfile;
  
  namespace po = boost::program_options;
  
  po::options_description cl_desc("Allowed options");
  cl_desc.add_options()
    ("help,h",  "produce this help message")
    ("settings,s", po::value<string>(&settingsfile)->default_value("settings.ini"), "Input settings file ")
    ("lmout,l", po::value<string>(&options.listmode_output_name)->default_value("LMdata"), "Name of listmode output file")
    ("mcaout,m", po::value<string>(&options.mca_output_name)->default_value("MCA"), "Name of MCA output file")
  ;
  

  //po::positional_options_description p;
  
  po::variables_map cl_vm;
  
  try
  {
    po::parsed_options parsed_opts
      = po::command_line_parser(argc,argv)
          //.allow_unregistered()
          .options(cl_desc)
          .run();
    
    po::store( parsed_opts, cl_vm );
    po::notify( cl_vm );
  }catch( std::exception &e )
  {
    cerr << "Error parsing command line arguments: " << e.what() << endl;
    cout << cl_desc << endl;
    return 1;
  }//try catch
  
  
  if( cl_vm.count("help") )
  {
    cout << cl_desc << endl;
    return -1;
  }//if( cl_vm.count("help") )

  const char *defaults_file = "defaults.ini";
  int rval = init_PixieNetFippiConfig_from_file( defaults_file, 0, &fippiconfig );   // first load defaults, do not allow missing parameters
  if( rval != 0 )
  {
    cerr << "Failed to parse FPGA settings from default file" << endl;
    return -2;
  }
  if( init_PixieNetFippiConfig_from_file( settingsfile.c_str(), 1, &fippiconfig ) )     // second override with user settings, do allow missing
  {
    cerr << "Failed to parse FPGA settings from " << settingsfile << endl;
    return -2;
  }

  
  return 0;
}//init_configurations

/* ********************************************************************************
   *   subroutine to initialize variables used during run
   ******************************************************************************** */

void init_PixieNetRunningStats( PixieNetRunningStats *stats )
{
  stats->numchecks = 0;
  stats->collectionnum = 0;
  stats->eventcount = 0;
  stats->numtraceblocks =0;
  memset( stats->numaccepted, 0, sizeof(stats->numaccepted) );
  memset( stats->numrejected, 0, sizeof(stats->numrejected) );  
  memset( stats->baseline, 0, sizeof(stats->baseline) );
  memset( stats->C0, 0, sizeof(stats->C0) );
  memset( stats->C1, 0, sizeof(stats->C1) );
  memset( stats->Cg, 0, sizeof(stats->Cg) );
}



/* ********************************************************************************
   *   data collection and processing subroutine RUN_TYPE 0x400 and 0x301
   ******************************************************************************** */

unsigned int collect_PixieNet_lm_data400( volatile unsigned int *mapped,
                             PixieNetHit402 hits[NCHANNELS],
                             PixieNetRunningStats *stats,
                             const PixieNetFippiConfig *fippiconfig )
{
  stats->numchecks += 1;
  
  unsigned int eventcount = 0;
  const unsigned int evstats = mapped[AEVSTATS];
  unsigned int R1;
#if( PixieNetHit_RECORD_HIT_PSA )
  unsigned int psa_base, psa_Q0, psa_Q1, psa_ampl, psa_R, tmpI;
  unsigned int psatmp0, psatmp1, cfdtmp0;
  unsigned int ts_max, CW;
  unsigned int cfdout, cfdlow, cfdhigh, cfdticks, cfdfrac;
  double cfdlev, tmpD, bscale;
  int tmpS;

  CW           = (int)floor(fippiconfig->COINCIDENCE_WINDOW*FILTER_CLOCK_MHZ);       // multiply time in us *  # ticks per us = time in ticks

#endif
  if( !evstats )
    return eventcount;
  
  stats->collectionnum += 1;
  
  //Make sure to force a read for CA_REJECT events; this volatile my not be necassary.
  //volatile unsigned int dummy = 0;
  
  for( unsigned int ch = 0; ch < NCHANNELS; ++ch )
  {
    R1 = 1 << ch;
    
    // check if there is an event in the FIFO
    if( evstats & R1 )
    {
      // read hit pattern and status info
      const unsigned int chaddr = ch*16+16;
      //cout << "chaddr=" << chaddr << endl;
      
      const unsigned int hit = mapped[chaddr+CA_HIT];
      //cout << "hit=" << hit << endl;
      
      // printf("channel %d, hit 0x%x\n",ch,hit);
      
      if( hit & (fippiconfig->ACCEPT_PATTERN) )
      {
        stats->numaccepted[ch] += 1;
        stats->eventcount += 1;
        
        // read data not needed for pure MCA runs
        PixieNetHit402 *info = &(hits[eventcount]);
        info->channel = ch;
        info->hit = hit;
        info->evtimeL = mapped[chaddr+CA_TSL];
        info->evtimeH = mapped[chaddr+CA_TSH];
#if( PixieNetHit_RECORD_HIT_PSA )
        psatmp0 = mapped[chaddr+CA_PSAA];     // cfd and psa need some recomputation
        psatmp1 = mapped[chaddr+CA_PSAB];
        cfdtmp0 = mapped[chaddr+CA_CFDA];

         // compute PSA results from raw data
         // need to subtract baseline in correct scale (1/4) and length (QDC#_LENGTH[ch])
         psa_base = psatmp0 & 0xFFFF;                                   // base only, in same scale as ADC samples
         if( fippiconfig->QDC_DIV8[ch]) 
            bscale = 32.0;
         else
            bscale = 4.0;
         
         tmpI = (psatmp0 & 0xFFFF0000) >> 16;                           // raw Q0, scaled by 1/4, not BL corrected
         tmpD = (double)tmpI - (double)psa_base/bscale * (double)fippiconfig->QDC0_LENGTH[ch]; //  subtract QDCL0 x base/bscale from raw value
         if( (tmpD>0) && (tmpD<65535))
            psa_Q0 = (int)floor(tmpD);
         else
            psa_Q0 = 0;
         
         tmpI = (psatmp1 & 0xFFFF);                                     // raw Q1, scaled by 1/4, not BL corrected
         tmpD = (double)tmpI - (double)psa_base/bscale * fippiconfig->QDC1_LENGTH[ch]; //  subtract QDCL0 x base/bscale from raw value
         if( (tmpD>0) && (tmpD<65535))
            psa_Q1 = (int)floor(tmpD);
         else
            psa_Q1 = 0;
         
         psa_ampl = ((psatmp1 & 0xFFFF0000) >> 16) - psa_base;

         if(psa_Q0!=0)
            psa_R = (int)floor(1000.0*(double)psa_Q1/(double)psa_Q0);
         else
            psa_R = 0;

          // compute CFD fraction
         // Normally x = dt * (cfd level - cfd low) / (cfd high - cfd low) = time after lower sample
         // However, here CFD is latched before time stamp and we need to compute CFD time BEFORE timestamp. 
         // So we are interested in the time before higher sample = dt-x = dt*(cfd high - cfd level) / (cfd high - cfd low)
         // result is in units of 1/256th ns
         
         // compute 1-x
         cfdlev = (double)psa_ampl/2.0 + (double)psa_base;       // compute 50% level
         cfdlow =  (cfdtmp0 & 0x00000FFF);
         cfdhigh = (cfdtmp0 & 0x00FFF000) >> 12;      // limited to 12 bits currently!
         if((cfdhigh-cfdlow)>0) {
            tmpD = ((cfdhigh-cfdlev)/(cfdhigh-cfdlow));  //   in units of clock cycles
         } else {
            tmpD = 0;
         }
         cfdfrac = (int)floor(tmpD*4.0*256.0) & 0x3FF;      //fraction 0..1 mapped to 0..1023, i.e. in units of 1/256ns
         
         // add offset within 2-sample group and offset to trigger
         cfdticks = (cfdtmp0 & 0x0F000000) >> 24;          // cfd ticks has the # of 4ns ticks from cfd level to the block of 2 samples that includes the maximum 
         ts_max =  (cfdtmp0 & 0xF0000000) >> 28;          // ft ticks has the 4 relevant bits of timestamp at maximum
         tmpI = (info->evtimeL & 0x7F) >> 3;              // 4 relevant bits of trigger time stamp, in 8ns steps
         tmpS = ts_max - tmpI;
         if(tmpS<0) tmpS = tmpS + 16;                   // build difference, tmps = time from trigger to max in 8ns steps
         tmpS = 2*tmpS - cfdticks;                      // build difference, tmps = time from trigger to CFD high in 4ns steps
         cfdout = (CW - tmpS)*4*256 + cfdfrac;          // time from CW end to CFD point in units of 1/256 ns
         
         //assemble/save for write-to-file routine
          info->psa0 = psa_ampl + (cfdout << 16);    
          info->psa1 = psa_base + (psa_Q0 << 16);
          info->cfd0 = psa_Q1   + (psa_R  << 16);
          info->cfd1 = psatmp0;                    // debug
#endif       

        // read raw energy sums
        const unsigned int lsum  = mapped[chaddr+CA_LSUM]; // leading, larger, "S1", past rising edge
        const unsigned int tsum  = mapped[chaddr+CA_TSUM]; // trailing, smaller, "S0" before rising edge
        const unsigned int gsum  = mapped[chaddr+CA_GSUM];	// gap sum, "Sg", during rising edge; also advances FIFO and increments Nout etc      
         
        double ph = stats->C1[ch] * (double)lsum + stats->Cg[ch] * (double)gsum + stats->C0[ch] * (double)tsum - stats->baseline[ch];
        if ((ph<0.0) | (ph>65536.0))	ph =0.0;	   // out of range energies -> 0
        info->energy0 = (int)floor(ph);
        if ((info->hit & (1<<HIT_LOCALHIT))==0)	info->energy0 =0;	   // not a local hit -> 0

        if( fippiconfig->RUN_TYPE == 0x501 || fippiconfig->RUN_TYPE == 0x301 )  //decimal 1281 or ...
        {
          //Dont save waveform
        }else if( fippiconfig->RUN_TYPE == 0x400 || fippiconfig->RUN_TYPE == 0x500 )  //decimal 1280
        {
#if( PixieNetHit_HAS_WAVEFORM )
          mapped[AOUTBLOCK] = 3;
          // R1 = mapped[AWF0+ch];  // dummy read?
          
          // multiply time in us *  # ticks per us = time in ticks. must be multiple of BLOCKSIZE_400
          const int TL = BLOCKSIZE_400*(int)floor(fippiconfig->TRACE_LENGTH[ch]*ADC_CLK_MHZ/BLOCKSIZE_400);
          assert( TL <= MAX_ACQ_TL  );
          
          info->num_waveform0 = TL;
          info->NumCurrTraceBlks = TL/BLOCKSIZE_400;        // current blocksize for channel header
          info->NumPrevTraceBlks = stats->numtraceblocks;   // prev blocksize for channel header
          stats->numtraceblocks = TL/BLOCKSIZE_400;         // current -> prev for next channel header
          for( int k = 0; k < (TL/4); ++k )
          {
            unsigned int wone = mapped[AWF0+ch];
            unsigned int wtwo = mapped[AWF0+ch];
            
            info->waveform0[4*k+0] = (uint16_t)(wone >> 16);
            info->waveform0[4*k+1] = (uint16_t)(wone & 0xFFFF);
            info->waveform0[4*k+2] = (uint16_t)(wtwo >> 16);
            info->waveform0[4*k+3] = (uint16_t)(wtwo & 0xFFFF);
          }
          
          mapped[AOUTBLOCK] = OB_EVREG;
#else
          static size_t ntimeswarned = 0;
          if( ++ntimeswarned < 2 )
            printf( "Saving of wavform requested, but has not been enabled at compile time, see PixieNetHit_HAS_WAVEFORM\n" );
#endif
        }else
        {
          printf( "Unsupported run type\n" );
        }//if( RunType = ... ) / else
        
        eventcount++;
      }else // event not acceptable (piled up, out of range,  ?)
      {
        stats->numrejected[ch] += 1;
        R1 = mapped[chaddr+CA_REJECT];		// read this register to advance event FIFOs without incrementing Nout etc
      }
    }// end event in this channel
  }//for( int ch = 0; ch < NCHANNELS; ++ch )
  
  return eventcount;
}//unsigned int collect_PixieNet_lm_data400


/* ********************************************************************************
   *   data collection and processing subroutine RUN_TYPE 0x402
   ******************************************************************************** */

unsigned int collect_PixieNet_lm_data402( volatile unsigned int *mapped,
                             PixieNetHit402 hits[NCHANNELS],
                             PixieNetRunningStats *stats,
                             const PixieNetFippiConfig *fippiconfig )
{
  stats->numchecks += 1;
  
  unsigned int eventcount = 0;
  unsigned int R1;
  unsigned int Nchok = 0;
  unsigned int chaddr;
  unsigned int hit[NCHANNELS];
  unsigned int ch;
  unsigned int Etemp[NCHANNELS];
  unsigned int wone, wtwo;
  unsigned int lsum, tsum, gsum;
  int TL;
  

  const unsigned int evstats = mapped[AEVSTATS];
  
  if( !( (evstats & 0xF)==0xF  )   )     // LM402:  process only if there are events in _every_ channel
     return eventcount;


  for( ch = 0; ch < NCHANNELS; ++ch )      // first loop: check if ALL acceptable
  {
      // read hit pattern and status info
      chaddr = ch*16+16;    
      hit[ch] = mapped[chaddr+CA_HIT];
      if( hit[ch] & (fippiconfig->ACCEPT_PATTERN) )
      {
          Nchok = Nchok+1;
      }
  }

   if(Nchok !=4)        // event not acceptable (e.g. piled up )
   { 
      for( ch=0; ch < NCHANNELS; ch++)          // second loop: clear ALL
      {
         chaddr = ch*16+16;
         R1 = mapped[chaddr+CA_REJECT];		// read this register to advance event FIFOs without incrementing Nout etc
         stats->numrejected[ch] += 1;
      }
      return eventcount;
   } 
   else
   {
  
     stats->collectionnum += 1;
     stats->eventcount += 1;
     
     //Make sure to force a read for CA_REJECT events; this volatile my not be necassary.
     //volatile unsigned int dummy = 0;

     // write out parts of the read loop, see above comment about not using vectors?
     PixieNetHit402 *info = &(hits[eventcount]);    // assign a local variable to the event record defined in the main polling loop
     
     info->evtimeL = mapped[AEVTSL];     // event time low
     info->evtimeH = mapped[AEVTSH];     // event time high
     info->PPStime = mapped[AEVPPS];     // PPS time latched by event
     info->time0   = mapped[0*16+16+CA_TSL] >> 8;     // local time, lower 24 bits
     info->time1   = mapped[1*16+16+CA_TSL] >> 8;
     info->time2   = mapped[2*16+16+CA_TSL] >> 8;
     info->time3   = mapped[3*16+16+CA_TSL] >> 8;
     
     for( unsigned int ch = 0; ch < NCHANNELS; ++ch )      // third loop: read and store
     {

         // read hit pattern and status info
           chaddr = ch*16+16;
           stats->numaccepted[ch] += 1;           
     
           // read raw energy sums
           lsum  = mapped[chaddr+CA_LSUM]; // leading, larger, "S1", past rising edge
           tsum  = mapped[chaddr+CA_TSUM]; // trailing, smaller, "S0" before rising edge
           gsum  = mapped[chaddr+CA_GSUM];	// gap sum, "Sg", during rising edge; also advances FIFO and increments Nout etc      
            
           double ph = stats->C1[ch] * (double)lsum + stats->Cg[ch] * (double)gsum + stats->C0[ch] * (double)tsum - stats->baseline[ch];
           if ((ph<0.0) | (ph>65536.0))	ph =0.0;	   // out of range energies -> 0
           Etemp[ch] = (int)floor(ph);
           if ((hit[ch] & (1<<HIT_LOCALHIT))==0)	  	Etemp[ch] =0;	   // not a local hit -> 0
      } // for loop (E)
      info->energy0   = Etemp[0];
      info->energy1   = Etemp[1];
      info->energy2   = Etemp[2];
      info->energy3   = Etemp[3];
      info->NumUserDataBlks   = 0;
      info->Esum   = Etemp[0]+Etemp[1]+Etemp[2]+Etemp[3];

      R1 = hit[0] | hit[1];   // or hits together and clear channel specific bits
      R1 = R1 | hit[2];
      R1 = R1 | hit[3];
      R1 = R1 & 0x00FFFFFF;
      info->hit = R1;
   
   #if( PixieNetHit_HAS_WAVEFORM )
       mapped[AOUTBLOCK] = 3;
       
       
       // Channel 0 waveforms        
       R1 = mapped[AWF0+0];  // dummy read?
       // multiply time in us *  # ticks per us = time in ticks. must be multiple of BLOCKSIZE_400
       TL = BLOCKSIZE_400*(int)floor(fippiconfig->TRACE_LENGTH[0]*ADC_CLK_MHZ/BLOCKSIZE_400);
       assert( TL <= MAX_ACQ_TL  );
       info->num_waveform0 = TL;
       info->NumTraceBlks0 = TL/BLOCKSIZE_400;        // current blocksize for channel header
       for( int k = 0; k < (TL/4); ++k )
       {
         wone = mapped[AWF0+0];
         wtwo = mapped[AWF0+0];
         info->waveform0[4*k+0] = (uint16_t)(wone >> 16);
         info->waveform0[4*k+1] = (uint16_t)(wone & 0xFFFF);
         info->waveform0[4*k+2] = (uint16_t)(wtwo >> 16);
         info->waveform0[4*k+3] = (uint16_t)(wtwo & 0xFFFF);
       }

       // Channel 1 waveforms         
       R1 = mapped[AWF0+1];  // dummy read?
       // multiply time in us *  # ticks per us = time in ticks. must be multiple of BLOCKSIZE_400

       TL = BLOCKSIZE_400*(int)floor(fippiconfig->TRACE_LENGTH[2]*ADC_CLK_MHZ/BLOCKSIZE_400);
       assert( TL <= MAX_ACQ_TL  );
       info->num_waveform1 = TL;
       info->NumTraceBlks1 = TL/BLOCKSIZE_400;        // current blocksize for channel header
       for( int k = 0; k < (TL/4); ++k )
       {
         wone = mapped[AWF0+1];
         wtwo = mapped[AWF0+1];
         info->waveform1[4*k+0] = (uint16_t)(wone >> 16);
         info->waveform1[4*k+1] = (uint16_t)(wone & 0xFFFF);
         info->waveform1[4*k+2] = (uint16_t)(wtwo >> 16);
         info->waveform1[4*k+3] = (uint16_t)(wtwo & 0xFFFF);
       }

       // Channel 2 waveforms
       R1 = mapped[AWF0+2];  // dummy read?
       // multiply time in us *  # ticks per us = time in ticks. must be multiple of BLOCKSIZE_400
       TL = BLOCKSIZE_400*(int)floor(fippiconfig->TRACE_LENGTH[2]*ADC_CLK_MHZ/BLOCKSIZE_400);
       assert( TL <= MAX_ACQ_TL  );
       info->num_waveform2 = TL;
       info->NumTraceBlks2 = TL/BLOCKSIZE_400;        // current blocksize for channel header
       for( int k = 0; k < (TL/4); ++k )
       {
         wone = mapped[AWF0+2];
         wtwo = mapped[AWF0+2];
         info->waveform2[4*k+0] = (uint16_t)(wone >> 16);
         info->waveform2[4*k+1] = (uint16_t)(wone & 0xFFFF);
         info->waveform2[4*k+2] = (uint16_t)(wtwo >> 16);
         info->waveform2[4*k+3] = (uint16_t)(wtwo & 0xFFFF);
       }

       // Channel 3 waveforms
       R1 = mapped[AWF0+3];  // dummy read?
       // multiply time in us *  # ticks per us = time in ticks. must be multiple of BLOCKSIZE_400
       TL = BLOCKSIZE_400*(int)floor(fippiconfig->TRACE_LENGTH[2]*ADC_CLK_MHZ/BLOCKSIZE_400);
       assert( TL <= MAX_ACQ_TL  );
       info->num_waveform3 = TL;
       info->NumTraceBlks3 = TL/BLOCKSIZE_400;        // current blocksize for channel header
       for( int k = 0; k < (TL/4); ++k )
       {
         wone = mapped[AWF0+3];
         wtwo = mapped[AWF0+3];
         info->waveform3[4*k+0] = (uint16_t)(wone >> 16);
         info->waveform3[4*k+1] = (uint16_t)(wone & 0xFFFF);
         info->waveform3[4*k+2] = (uint16_t)(wtwo >> 16);
         info->waveform3[4*k+3] = (uint16_t)(wtwo & 0xFFFF);
       }

       // Summary info
       info->NumCurrTraceBlks = info->NumTraceBlks0 + info->NumTraceBlks1 + info->NumTraceBlks2 + info->NumTraceBlks3;        // current blocksize for channel header
       info->NumPrevTraceBlks = stats->numtraceblocks;   // prev blocksize for channel header
       stats->numtraceblocks = info->NumCurrTraceBlks;    // current -> prev for next channel header
        
       mapped[AOUTBLOCK] = OB_EVREG;
   #else
       static size_t ntimeswarned = 0;
       if( ++ntimeswarned < 2 )
         printf( "Saving of wavform requested, but has not been enabled at compile time, see PixieNetHit_HAS_WAVEFORM\n" );
   #endif
         
     eventcount++;
   } // event acceptable
  
  return eventcount;
}//unsigned int collect_PixieNet_lm_data402



/* ********************************************************************************
   *   data write to file subroutine (runtype 0x400)
   ******************************************************************************** */

int PixieNetHit_write_400( FILE *outstrm, const PixieNetHit402 * const hit )
{
  //fwrite is really slow (like very significant impact on throughput of events
  //  slow), so we will write to a buffer, and then to the file.
  uint8_t buffer[CHAN_HEAD_LENGTH_400*2] = {0};
  uint32_t wm = WATERMARK;
  memcpy( buffer + 0, &(hit->hit), 4 );
#if( PixieNetHit_HAS_WAVEFORM ) 
  memcpy( buffer + 4, &(hit->NumCurrTraceBlks), 2 );
  memcpy( buffer + 6, &(hit->NumPrevTraceBlks), 2 );
#endif
  memcpy( buffer + 8, &(hit->evtimeL), 4 );
  memcpy( buffer + 12, &(hit->evtimeH), 4 );
  memcpy( buffer + 16, &(hit->energy0), 2 );
  memcpy( buffer + 18, &(hit->channel), 2 );
#if( PixieNetHit_RECORD_HIT_PSA )
  memcpy( buffer + 20, &(hit->psa0), 4 );
  memcpy( buffer + 24, &(hit->psa1), 4 );
  memcpy( buffer + 28, &(hit->cfd0), 4 );
  memcpy( buffer + 32, &(hit->cfd1), 4 );
#endif 
  // no checksum  for now
  memcpy( buffer + 60, &(wm), 4 );
  fwrite( buffer, 1, CHAN_HEAD_LENGTH_400*2, outstrm );
  
#if( PixieNetHit_HAS_WAVEFORM )
  fwrite( hit->waveform0, hit->num_waveform0, sizeof(hit->waveform0[0]), outstrm );
#endif
  
  return 1;
}//void PixieNetHit_write_400( FILE *outstrm, const PixieNetHit400 * const hit )


/* ********************************************************************************
   *   data write to file subroutine (runtype 0x402)
   ******************************************************************************** */

int PixieNetHit_write_402( FILE *outstrm, const PixieNetHit402 * const hit )
{
  //fwrite is really slow (like very significant impact on throughput of events
  //  slow), so we will write to a buffer, and then to the file.
  uint8_t buffer[CHAN_HEAD_LENGTH_400*2] = {0};
  uint32_t wm = WATERMARK;
  memcpy( buffer + 0, &(hit->hit), 4 );
#if( PixieNetHit_HAS_WAVEFORM ) 
  memcpy( buffer + 4, &(hit->NumCurrTraceBlks), 2 );
  memcpy( buffer + 6, &(hit->NumPrevTraceBlks), 2 );
#endif
  memcpy( buffer + 8, &(hit->evtimeH), 4 );
  memcpy( buffer + 12, &(hit->Esum), 2 );
  memcpy( buffer + 14, &(hit->NumUserDataBlks), 2 );
  memcpy( buffer + 16, &(hit->time0), 4 );
  memcpy( buffer + 20, &(hit->energy0), 2 );
#if( PixieNetHit_HAS_WAVEFORM ) 
  memcpy( buffer + 22, &(hit->NumTraceBlks0), 2 );
#endif
  memcpy( buffer + 24, &(hit->time1), 4 );
  memcpy( buffer + 28, &(hit->energy1), 2 );
#if( PixieNetHit_HAS_WAVEFORM ) 
  memcpy( buffer + 30, &(hit->NumTraceBlks1), 2 );
#endif
  memcpy( buffer + 32, &(hit->time2), 4 );
  memcpy( buffer + 36, &(hit->energy2), 2 );
#if( PixieNetHit_HAS_WAVEFORM ) 
  memcpy( buffer + 38, &(hit->NumTraceBlks2), 2 );
#endif
  memcpy( buffer + 40, &(hit->time3), 4 );
  memcpy( buffer + 44, &(hit->energy3), 2 );
#if( PixieNetHit_HAS_WAVEFORM ) 
  memcpy( buffer + 46, &(hit->NumTraceBlks3), 2 );
#endif
  // 48-51: channel hit detail
  memcpy( buffer + 52, &(hit->evtimeL), 4 );
  // no checksum  for now
  memcpy( buffer + 60, &(wm), 4 );
  fwrite( buffer, 1, CHAN_HEAD_LENGTH_400*2, outstrm );
  
#if( PixieNetHit_HAS_WAVEFORM )
  fwrite( hit->waveform0, hit->num_waveform0, sizeof(hit->waveform0[0]), outstrm );
  fwrite( hit->waveform1, hit->num_waveform1, sizeof(hit->waveform1[0]), outstrm );
  fwrite( hit->waveform2, hit->num_waveform2, sizeof(hit->waveform2[0]), outstrm );
  fwrite( hit->waveform3, hit->num_waveform3, sizeof(hit->waveform3[0]), outstrm );
#endif
  
  return 1;
}//void PixieNetHit_write_402( FILE *outstrm, const PixieNetHit400 * const hit )



