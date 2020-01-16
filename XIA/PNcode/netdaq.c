/*----------------------------------------------------------------------
 * Copyright (c) 2019 XIA LLC
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


/*
 * NTS trigger/buffering output key:
 * . - received INIT while waiting for START
 * d - duplicate/overlap: accept range overlaps previously stored trigger
 * f - flush a stored trigger
 * i - insert trigger in used buffer slot
 * s - store trigger
 * u - unknown accept timestamp range
 * x - overwrite a sent but not-(yet)-accepted trigger
 * w - wrap next/back pointer
 * W - wrap start/front pointer
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/file.h>
// need to compile with -lm option

#include "PixieNetDefs.h"
#include "PixieNetCommon.h"
#include "PixieNetConfig.h"

#include "nts.h"
#include "log.h"

#define NTS_POLL_INTERVAL 1


int main(int argc, const char **argv) {
    int fd;
    void *map_addr;
    int size = 4096;
    volatile unsigned int *mapped;
    int k, ch, tmpS;
    FILE * filmca;
    FILE * fil;

    unsigned int Accept, RunType, SyncT, ReqRunTime, PollTime, CW;
    unsigned int SL[NCHANNELS];
    //unsigned int SG[NCHANNELS];
    float Tau[NCHANNELS], Dgain[NCHANNELS];
    unsigned int BLavg[NCHANNELS], BLcut[NCHANNELS], Binfactor[NCHANNELS], TL[NCHANNELS];
    double C0[NCHANNELS], C1[NCHANNELS], Cg[NCHANNELS];
    double baseline[NCHANNELS] = {0};
    double dt, ph, tmpD, bscale;
    double elm, q;
    double cfdlev;
    time_t starttime, currenttime;
    unsigned int startTS, m, c0, c1, c2, c3, w0, w1, tmpI, revsn;
    unsigned int evstats, R1, hit, timeL, timeH, psa0, psa1, cfd0;
    unsigned int psa_base, psa_Q0, psa_Q1, psa_ampl, psa_R;
    unsigned int cfdout, cfdlow, cfdhigh, cfdticks, cfdfrac, ts_max;
    unsigned int lsum, tsum, gsum, energy, bin, binx, biny;
    unsigned int mca[NCHANNELS][MAX_MCA_BINS] ={{0}};    // full MCA for end of run
    unsigned int wmca[NCHANNELS][WEB_MCA_BINS] ={{0}};    // smaller MCA during run
    unsigned int mca2D[NCHANNELS][MCA2D_BINS*MCA2D_BINS] ={{0}};    // 2D MCA for end of run
    unsigned int onlinebin;
    unsigned int wf[MAX_TL/2];    // two 16bit values per word
    unsigned int chaddr, loopcount, eventcount, NumPrevTraceBlks, TraceBlks;
    unsigned short buffer1[FILE_HEAD_LENGTH_400] = {0};
    unsigned char buffer2[CHAN_HEAD_LENGTH_400*2] = {0};
    unsigned int wm = WATERMARK;
    unsigned int BLbad[NCHANNELS];
    onlinebin=MAX_MCA_BINS/WEB_MCA_BINS;

    const char *nts_host = "192.168.1.215";
    unsigned int nts_triggered = 0, nts_sent = 0, nts_received = 0;
    int nts_run = 1;
    NTS *nts;
    void *nts_event_data;
    int poll_result;

    if (argc > 1) {
        nts_host = argv[1];
    }

    printf("NTS host: %s\n", nts_host);

    // ******************* set up logging ******************
    if (pn_log_open("netdaq.log")) {
        printf("Failed to open log\n");
        return -1;
    }

    // ******************* read ini file and fill struct with values ********************

    PixieNetFippiConfig fippiconfig;		// struct holding the input parameters
    const char *defaults_file = "defaults.ini";
    int rval = init_PixieNetFippiConfig_from_file( defaults_file, 0, &fippiconfig );   // first load defaults, do not allow missing parameters
    if( rval != 0 )
    {
        printf( "Failed to parse FPGA settings from %s, rval=%d\n", defaults_file, rval );
        return rval;
    }
    const char *settings_file = "settings.ini";
    rval = init_PixieNetFippiConfig_from_file( settings_file, 1, &fippiconfig );   // second override with user settings, do allow missing
    if( rval != 0 )
    {
        printf( "Failed to parse FPGA settings from %s, rval=%d\n", settings_file, rval );
        return rval;
    }

    // assign to local variables, including any rounding/discretization
    Accept       = fippiconfig.ACCEPT_PATTERN;
    RunType      = fippiconfig.RUN_TYPE;
    SyncT        = fippiconfig.SYNC_AT_START;
    ReqRunTime   = fippiconfig.REQ_RUNTIME;
    PollTime     = fippiconfig.POLL_TIME;
    CW           = (int)floor(fippiconfig.COINCIDENCE_WINDOW*FILTER_CLOCK_MHZ);       // multiply time in us *  # ticks per us = time in ticks

    if( (RunType==0x503) || (RunType==0x402) )  {      // grouped list mode run (equiv 0x402)
        printf( "This function does not support LM runtypes 0x503 or 0x402, use coincdaq or acquire\n");
        return(-1);
    }

    for( k = 0; k < NCHANNELS; k ++ )
    {
        SL[k]          = (int)floor(fippiconfig.ENERGY_RISETIME[k]*FILTER_CLOCK_MHZ);       // multiply time in us *  # ticks per us = time in ticks
        //    SG[k]          = (int)floor(fippiconfig.ENERGY_FLATTOP[k]*FILTER_CLOCK_MHZ);       // multiply time in us *  # ticks per us = time in ticks
        Dgain[k]       = fippiconfig.DIG_GAIN[k];
        TL[k]          = BLOCKSIZE_400*(int)floor(fippiconfig.TRACE_LENGTH[k]*ADC_CLK_MHZ/BLOCKSIZE_400);       // multiply time in us *  # ticks per us = time in ticks, multiple of 4
        Binfactor[k]   = fippiconfig.BINFACTOR[k];
        Tau[k]         = fippiconfig.TAU[k];
        BLcut[k]       = fippiconfig.BLCUT[k];
        BLavg[k]       = 65536 - fippiconfig.BLAVG[k];
        if(BLavg[k]<0)          BLavg[k] = 0;
        if(BLavg[k]==65536)     BLavg[k] = 0;
        if(BLavg[k]>MAX_BLAVG)  BLavg[k] = MAX_BLAVG;
        BLbad[k] = MAX_BADBL;   // initialize to indicate no good BL found yet
    }



    // *************** PS/PL IO initialization *********************
    // open the device for PD register I/O
    fd = open("/dev/uio0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open devfile");
        return -2;
    }

    //Lock the PL address space so multiple programs cant step on eachother.
    if( flock( fd, LOCK_EX | LOCK_NB ) )
    {
        printf( "Failed to get file lock on /dev/uio0\n" );
        return -3;
    }

    map_addr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (map_addr == MAP_FAILED) {
        perror("Failed to mmap");
        return -4;
    }

    mapped = (unsigned int *) map_addr;

    // --------------------------------------------------------
    // - Software triggering setup
    // --------------------------------------------------------

    nts = nts_open(nts_host, 5591);
    if (!nts) {
        printf("Failed to open NetTimeSync software triggering\n");
        return -5;
    }

    printf("DAQ starting\n");

    // --------------------------------------------------------
    // ------------------- Main code begins --------------------
    // --------------------------------------------------------


    // **********************  Compute Coefficients for E Computation  **********************
    dt = 1.0/FILTER_CLOCK_MHZ;
    for( k = 0; k < NCHANNELS; k ++ )
    {
        q = exp(-1.0*dt/Tau[k]);
        elm = exp(-1.0*dt*SL[k]/Tau[k]);
        C0[k] = (q-1.0)*elm/(1.0-elm);
        Cg[k] = 1.0-q;
        C1[k] = (1.0-q)/(1.0-elm);
        // printf("%f  %f   %f\n", C0[k], Cg[k], C1[k]);

        C0[k] = C0[k] * Dgain[k];
        Cg[k] = Cg[k] * Dgain[k];
        C1[k] = C1[k] * Dgain[k];
    }

    revsn = hwinfo(mapped);

    // ********************** Run Start **********************


    NumPrevTraceBlks = 0;
    loopcount =  0;
    eventcount = 0;
    starttime = currenttime = time(NULL);                         // capture OS start time
    pn_log("Run start");

    if( (RunType==0x500) || (RunType==0x501)  || (RunType==0x502) || (RunType==0x400) )  {    // list mode runtypes
        if(SyncT) mapped[ARTC_CLR] = 1;              // write to reset time counter
        mapped[AOUTBLOCK] = 2;
        startTS = mapped[AREALTIME];
        if(RunType==0x500)   {                       // generic runtype is one value per line
            fil = fopen("LMdata.txt","w");
            fprintf(fil, "Module:\t%hu\n",0);
            fprintf(fil, "Run Type:\t0x%x\n",RunType);
            fprintf(fil,"Run Start Time Stamp (ticks) :\t%u\n", startTS);	//
            fprintf(fil,"Run Start Time (s) :\t%lld\n", (long long)starttime);			// this is only precise to a second or so
        }
        if(RunType==0x501){                                     // compressed runtype has columns
            fil = fopen("LMdata.dat","w");
            fprintf(fil,"Module,Run_Type,Run_Start_ticks,Run_Start_sec,Unused1,Unused2\n");
            fprintf(fil,"%d,0x%x,%u,%lld,--,--\n",0,RunType,startTS,(long long)(starttime));
            fprintf(fil,"No,Ch,Hit,Time_H,Time_L,Energy\n");
        }
        if(RunType==0x502){                                     // compressed PSA runtype has more columns
            fil = fopen("LMdata.dt2","w");
            fprintf(fil,"Module,Run_Type,Run_Start_ticks,Run_Start_sec,Unused1,Unused2\n");
            fprintf(fil,"%d,0x%x,%u,%lld,--,--\n",0,RunType,startTS,(long long)(starttime));
            fprintf(fil,"Event_No,Channel_No,Hit_Pattern,Event_Time_H,Event_Time_L,Energy,Amplitude,CFD,Base,Q0,Q1,PSAvalue\n");
        }
        if(RunType==0x400){
            // write a 0x400 header
            // fwrite is slow so we will write to a buffer, and then to the file.
            fil = fopen("LMdata.b00","wb");
            buffer1[0] = BLOCKSIZE_400;
            buffer1[1] = 0;                                       // module number (get from settings file?)
            buffer1[2] = RunType;
            buffer1[3] = CHAN_HEAD_LENGTH_400;
            buffer1[4] = fippiconfig.COINCIDENCE_PATTERN;
            buffer1[5] = fippiconfig.COINCIDENCE_WINDOW;
            buffer1[7] = revsn>>16;               // HW revision from EEPROM
            buffer1[12] = revsn & 0xFFFF;         // serial number from EEPROM
            for( ch = 0; ch < NCHANNELS; ch++) {
                buffer1[6]   +=(int)floor((TL[ch] + CHAN_HEAD_LENGTH_400) / BLOCKSIZE_400);         // combined event length, in blocks
                buffer1[8+ch] =(int)floor((TL[ch] + CHAN_HEAD_LENGTH_400) / BLOCKSIZE_400);			// each channel's event length, in blocks
            }
            fwrite( buffer1, 2, FILE_HEAD_LENGTH_400, fil );     // write to file
        }
    }

    mapped[ADSP_CLR] = 1;             // write to reset DAQ buffers
    mapped[ACOUNTER_CLR] = 1;         // write to reset RS counters
    mapped[ACSRIN] = 1;               // set RunEnable bit to start run
    mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers


    // ********************** Run Loop **********************
    do {
        pn_log_loop(loopcount);

        //----------- Periodically read BL and update average -----------
        // this will be moved into the FPGA soon
        if(loopcount % BLREADPERIOD == 0) {  //|| (loopcount ==0) ) {     // sometimes 0 mod N not zero and first few events have wrong E? watch
            pn_log("Update baseline");
            for( ch=0; ch < NCHANNELS; ch++) {
                Stopwatch sw = sw_start();

                // read raw BL sums
                chaddr = ch*16+16;
                lsum  = mapped[chaddr+CA_LSUMB];
                tsum  = mapped[chaddr+CA_TSUMB];
                gsum  = mapped[chaddr+CA_GSUMB];
                if (tsum>0)		// tum=0 indicates bad baseline
                {
                    ph = C1[ch]*lsum+Cg[ch]*gsum+C0[ch]*tsum;
                    //if (ch==0) printf("ph %f, BLcut %d, BLavg %d, baseline %f\n",ph,BLcut[ch],BLavg[ch],baseline[ch] );
                    if( (BLcut[ch]==0) || (abs(ph-baseline[ch])<BLcut[ch]) || (BLbad[ch] >=MAX_BADBL) )       // only accept "good" baselines < BLcut, or if too many bad in a row (to start over)
                    {
                        if( (BLavg[ch]==0) || (BLbad[ch] >=MAX_BADBL) )
                        {
                            baseline[ch] = ph;
                            BLbad[ch] = 0;
                        } else {
                            // BL average: // avg = old avg + (new meas - old avg)/2^BLavg
                            baseline[ch] = baseline[ch] + (ph-baseline[ch])/(1<<BLavg[ch]);
                            BLbad[ch] = 0;
                        } // end BL avg
                    } else {
                        BLbad[ch] = BLbad[ch]+1;
                    }     // end BLcut check
                }       // end tsum >0 check

                sw_check(&sw, "Baseline ch=%u", ch);
            }          // end for loop
        }             // end periodicity check


        // -----------poll for events -----------
        // if data ready. read out, compute E, increment MCA *********
        Stopwatch sw_stats = sw_start();
        evstats = mapped[AEVSTATS];
        sw_check(&sw_stats, "AEVSTATS");

        //   printf("EVstats 0x%x\n",evstats);
        if(evstats) {					  // if there are events in any channel
            for( ch=0; ch < NCHANNELS; ch++)
            {
                Stopwatch sw_lm = sw_start();

                R1 = 1 << ch;
                if(evstats & R1)	{	 // check if there is an event in the FIFO
                    // read hit pattern and status info
                    chaddr = ch*16+16;
                    hit   = mapped[chaddr+CA_HIT];
                    //    printf("channel %d, hit 0x%x\n",ch,hit);
                    pn_log("ch=%d hit=0x%x", ch, hit);
                    if(hit & Accept) {
                        // read data not needed for pure MCA runs
                        timeL = mapped[chaddr+CA_TSL];
                        timeH = mapped[chaddr+CA_TSH];
                        psa0  = mapped[chaddr+CA_PSAA];        // Q0raw/4 | B
                        psa1  = mapped[chaddr+CA_PSAB];        // M       | Q1raw/4
                        cfd0  = mapped[chaddr+CA_CFDA];        // {ts_max[6:3],cfdticks[3:0],cfdhigh[11:0],cfdlow[11:0]}
                        //cfd1  = mapped[chaddr+CA_CFDB];

                        //printf("channel %d, hit 0x%x, timeL %d\n",ch,hit,timeL);
                        // read raw energy sums
                        lsum  = mapped[chaddr+CA_LSUM];        // leading, larger, "S1", past rising edge
                        tsum  = mapped[chaddr+CA_TSUM];        // trailing, smaller, "S0" before rising edge
                        gsum  = mapped[chaddr+CA_GSUM];		   // gap sum, "Sg", during rising edge; also advances FIFO and increments Nout etc

                        // compute and histogram E
                        ph = C1[ch]*(double)lsum+Cg[ch]*(double)gsum+C0[ch]*(double)tsum;
                        //  printf("ph %f, BLavg %f, E %f\n",ph,baseline[ch], ph-baseline[ch]);
                        ph = ph-baseline[ch];
                        if ((ph<0.0)|| (ph>65536.0))	ph =0.0;	   // out of range energies -> 0
                        energy = (int)floor(ph);
                        if ((hit & (1<< HIT_LOCALHIT))==0)	  	energy =0;	   // not a local hit -> 0

                        //  histogramming if E< max mcabin
                        bin = energy >> Binfactor[ch];
                        if( (bin<MAX_MCA_BINS) && (bin>0) ) {
                            mca[ch][bin] =  mca[ch][bin] + 1;	// increment mca
                            bin = bin >> WEB_LOGEBIN;
                            if(bin>0) wmca[ch][bin] = wmca[ch][bin] + 1;	// increment wmca

                            // TODO: add split spectrum n.g for 0x502
                        }

                        // cfd and psa need some recomputation, not fully implemented yet

                        // compute PSA results from raw data
                        // need to subtract baseline in correct scale (1/4) and length (QDC#_LENGTH[ch])
                        psa_base = psa0 & 0xFFFF;                                   // base only, in same scale as ADC samples
                        if( fippiconfig.QDC_DIV8[ch])
                            bscale = 32.0;
                        else
                            bscale = 4.0;

                        tmpI = (psa0 & 0xFFFF0000) >> 16;                           // raw Q0, scaled by 1/4, not BL corrected
                        tmpD = (double)tmpI - (double)psa_base/bscale * fippiconfig.QDC0_LENGTH[ch]; //  subtract QDCL0 x base/bscale from raw value
                        if( (tmpD>0) && (tmpD<65535))
                            psa_Q0 = (int)floor(tmpD);
                        else
                            psa_Q0 = 0;

                        tmpI = (psa1 & 0xFFFF);                                     // raw Q1, scaled by 1/4, not BL corrected
                        tmpD = (double)tmpI - (double)psa_base/bscale * fippiconfig.QDC1_LENGTH[ch]; //  subtract QDCL0 x base/bscale from raw value
                        if( (tmpD>0) && (tmpD<65535))
                            psa_Q1 = (int)floor(tmpD);
                        else
                            psa_Q1 = 0;

                        psa_ampl = ((psa1 & 0xFFFF0000) >> 16) - psa_base;

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
                        cfdlow =  (cfd0 & 0x00000FFF);
                        cfdhigh = (cfd0 & 0x00FFF000) >> 12;      // limited to 12 bits currently!
                        if((cfdhigh-cfdlow)>0) {
                            tmpD = ((cfdhigh-cfdlev)/(cfdhigh-cfdlow));  //   in units of clock cycles
                        } else {
                            tmpD = 0;
                        }
                        cfdfrac = (int)floor(tmpD*4.0*256.0) & 0x3FF;      //fraction 0..1 mapped to 0..1023, i.e. in units of 1/256ns

                        // add offset within 2-sample group and offset to trigger
                        cfdticks = (cfd0 & 0x0F000000) >> 24;          // cfd ticks has the # of 4ns ticks from cfd level to the block of 2 samples that includes the maximum
                        ts_max =  (cfd0 & 0xF0000000) >> 28;          // ft ticks has the 4 relevant bits of timestamp at maximum
                        tmpI = (timeL & 0x7F) >> 3;                    // 4 relevant bits of trigger time stamp, in 8ns steps
                        tmpS = ts_max - tmpI;
                        if(tmpS<0) tmpS = tmpS + 16;                   // build difference, tmps = time from trigger to max in 8ns steps
                        tmpS = 2*tmpS - cfdticks;                      // build difference, tmps = time from trigger to CFD high in 4ns steps
                        cfdout = (CW - tmpS)*4*256 + cfdfrac;          // time from CW end to CFD point in units of 1/256 ns

                        //  cfdout = cfdfrac + (cfdticks<<10);
                        //  printf("ts_max %d, cfdticks %d, trig_to_max %d, trig_to_cfd %d \n",ts_max, cfdticks, tmpS2, tmpS);

                        sw_check(&sw_lm, "List mode ch=%u", ch);

                        // now store list mode data
                        nts_event_data = NULL;

                        if(RunType==0x502)   {
                            // 2D spectrum R vs E
                            binx = (int)floor(ph/fippiconfig.MCA2D_SCALEX[ch]);
                            biny = (int)floor((double)psa_R/fippiconfig.MCA2D_SCALEY[ch]);
                            if( (binx<MCA2D_BINS) && (biny<MCA2D_BINS) && (binx>0) && (biny>0) )
                            {
                                mca2D[ch][binx+MCA2D_BINS*biny] =   mca2D[ch][binx+MCA2D_BINS*biny] +1; // increment 2D MCA
                            }
                            // not saving waveforms, events in a table
                            fprintf(fil,"%u,%d,0x%X,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",eventcount,ch,hit,timeH,timeL,energy,psa_ampl,cfdout,psa_base,psa_Q0,psa_Q1,psa_R );
                        }    // 0x502

                        if(RunType==0x501)   {
                            // not saving waveforms, events in a table
                            fprintf(fil,"%u,%d,0x%X,%u,%u,%u\n",eventcount,ch,hit,timeH,timeL,energy);
                        }    // 0x501

                        if(RunType==0x500)   {
                            // For NTS, buffer waveforms with trigger metadata pending acceptance.
                            // Also write files like startdaq.
                            unsigned int *wfp = malloc(sizeof(unsigned int) * MAX_TL/2); // two 16bit values per word

                            if (wfp) {
                                // saving 8 headers +  waveforms, one entry per line
                                fprintf(fil,"%u\n%d\n0x%X\n%u\n%u\n%u\n%u\n%u\n",eventcount,ch,hit,timeH,timeL,energy,psa_R,cfdout);
                                mapped[AOUTBLOCK] = 3;
                                wfp[0] = mapped[AWF0+ch];  // dummy read?
                                for( k=0; k < (TL[ch]/4); k++)
                                    //for( k=0; k < 10; k++)
                                {
                                    wfp[2*k+0] = mapped[AWF0+ch];
                                    wfp[2*k+1] = mapped[AWF0+ch];

                                    // re-order 2 sample words from 32bit FIFO
                                    fprintf(fil,"%u\n",(wfp[2*k+0] >> 16) );
                                    fprintf(fil,"%u\n",(wfp[2*k+0] & 0xFFFF) );
                                    fprintf(fil,"%u\n",(wfp[2*k+1] >> 16) );
                                    fprintf(fil,"%u\n",(wfp[2*k+1] & 0xFFFF) );
                                }
                                mapped[AOUTBLOCK] = OB_EVREG;

                                nts_event_data = wfp;
                            }
                            else {
                                printf("E: no mem for 0x500 waveform\n");
                            }
                        }    // 0x500

                        if(RunType==0x400)   {
                            TraceBlks = (int)floor(TL[ch]/BLOCKSIZE_400);
                            memcpy( buffer2 + 0, &(hit), 4 );
                            memcpy( buffer2 + 4, &(TraceBlks), 2 );
                            memcpy( buffer2 + 6, &(NumPrevTraceBlks), 2 );
                            memcpy( buffer2 + 8, &(timeL), 4 );
                            memcpy( buffer2 + 12, &(timeH), 4 );
                            memcpy( buffer2 + 16, &(energy), 2 );
                            memcpy( buffer2 + 18, &(ch), 2 );
                            memcpy( buffer2 + 20, &(psa_ampl), 2 );
                            memcpy( buffer2 + 22, &(cfdout), 2 );   // actually cfd time
                            memcpy( buffer2 + 24, &(psa_base), 2 );
                            memcpy( buffer2 + 26, &(psa_Q0), 2 );
                            memcpy( buffer2 + 28, &(psa_Q1), 2 );
                            memcpy( buffer2 + 30, &(psa_R), 2 );
                            memcpy( buffer2 + 32, &(cfdticks), 2 );      // debug
                            memcpy( buffer2 + 34, &(ts_max), 2 );
                            memcpy( buffer2 + 36, &(psa0), 4 );      // debug
                            memcpy( buffer2 + 40, &(psa1), 4 );
                            // no checksum  for now
                            memcpy( buffer2 + 60, &(wm), 4 );
                            fwrite( buffer2, 1, CHAN_HEAD_LENGTH_400*2, fil );
                            NumPrevTraceBlks = TraceBlks;

                            mapped[AOUTBLOCK] = 3;
                            //  w0 = mapped[AWF0+ch];  // dummy read?
                            for( k=0; k < (TL[ch]/4); k++)
                            {
                                w0 = mapped[AWF0+ch];
                                w1 = mapped[AWF0+ch];
                                // re-order 2 sample words from 32bit FIFO
                                wf[2*k+1] = (w1 >> 16) + ((w1 & 0xFFFF) << 16);
                                wf[2*k+0] = (w0 >> 16) + ((w0 & 0xFFFF) << 16);
                            }
                            mapped[AOUTBLOCK] = OB_EVREG;
                            fwrite( wf, TL[ch]/2, 4, fil );
                        }      // 0x400

                        // Send triggers to the NTS DM.
                        unsigned long long ts = ((unsigned long long)timeH << 32) + timeL;
                        nts_triggered++;
                        pn_log("Trigger t=%llu n=%u ch=%u", ts, nts_triggered, ch);
                        nts_trigger(nts, revsn, ch, ts, currenttime, nts_event_data);
                        nts_sent++;

                        eventcount++;
                    }
                    else { // event not acceptable (piled up )
                        R1 = mapped[chaddr+CA_REJECT];		// read this register to advance event FIFOs without incrementing Nout etc
                    }
                }     // end event in this channel
            }        //end for ch
        }           // end event in any channel



        // ----------- Periodically save MCA, PSA, and Run Statistics  -----------

        if(loopcount % PollTime == 0)
        {
            pn_log("Save statistics");

            // 1) Run Statistics
            mapped[AOUTBLOCK] = OB_RSREG;

            // for debug purposes, print to std out so we see what's going on
            k = 3;    // no loop for now
            {
                m  = mapped[ARS0_MOD+k];
                c0 = mapped[ARS0_CH0+k];
                c1 = mapped[ARS0_CH1+k];
                c2 = mapped[ARS0_CH2+k];
                c3 = mapped[ARS0_CH3+k];
                printf("%s,%u,%s,%u,%u,%u,%u\n ","RunTime",m,"COUNTTIME",c0,c1,c2,c3);
            }

            // print (small) set of RS to file, visible to web
            read_print_runstats(1, 0, mapped);

            mapped[AOUTBLOCK] = OB_EVREG;     // read from event registers

            // 2) MCA
            filmca = fopen("MCA.csv","w");
            fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3\n");
            for( k=0; k <WEB_MCA_BINS; k++)       // report the 4K spectra during the run (faster web update)
            {
                fprintf(filmca,"%d,%u,%u,%u,%u\n ", k*onlinebin,wmca[0][k],wmca[1][k],wmca[2][k],wmca[3][k]);
            }
            fclose(filmca);

            // 3) 2D MCA or PSA
            if(RunType==0x502)   {
                filmca = fopen("psa2D.csv","w");

                // title row (x index)
                for( ch=0; ch <NCHANNELS; ch++)
                {
                    for( binx=0;binx<MCA2D_BINS;binx++)
                    {
                        fprintf(filmca,",%d",binx+MCA2D_BINS*ch);
                    }
                }    // channel loop
                fprintf(filmca,"\n");

                for( biny=0;biny<MCA2D_BINS;biny++)
                {
                    fprintf(filmca, "%d",biny);        // beginning of line
                    for( ch=0; ch <NCHANNELS; ch++)
                    {
                        for( binx=0;binx<MCA2D_BINS;binx++)
                        {
                            fprintf(filmca,",%d",mca2D[ch][biny+MCA2D_BINS*binx]);
                        }  // binx loop
                    }    // channel loop
                    fprintf(filmca,"\n");            // end of line

                }  // biny loop

                fclose(filmca);
            }  // runtype 0x502
        }

        // Receive NTS accept/reject decisions
        while (nts_sent % NTS_POLL_INTERVAL == 0 && (poll_result = nts_poll(nts)) != 0) {
            pn_log("Poll ret=%d", poll_result);
            if (poll_result > 0 && poll_result != NTS_IGNORE) {
                nts_received += poll_result;
            }
            else if (poll_result < 0) {
                nts_run = 0;
            }
        }

        // ----------- loop housekeeping -----------

        loopcount ++;
        currenttime = time(NULL);
    } while (currenttime <= starttime+ReqRunTime && nts_run); // run for a fixed time
    //     } while (eventcount <= 20); // run for a fixed number of events

    pn_log_loop(UINT_MAX);

    // ********************** Run Stop **********************
    printf("Stopping the run\n");

    // clear RunEnable bit to stop run
    mapped[ACSRIN] = 0;
    // todo: there may be events left in the buffers. need to stop, then keep reading until nothing left

    // final save MCA and RS
    filmca = fopen("MCA.csv","w");
    fprintf(filmca,"bin,MCAch0,MCAch1,MCAch2,MCAch3\n");
    for( k=0; k <MAX_MCA_BINS; k++)
    {
        fprintf(filmca,"%d,%u,%u,%u,%u\n ", k,mca[0][k],mca[1][k],mca[2][k],mca[3][k] );
    }
    fclose(filmca);

    mapped[AOUTBLOCK] = OB_RSREG;
    read_print_runstats(0, 0, mapped);
    mapped[AOUTBLOCK] = OB_IOREG;

    // 3) 2D MCA
    if(RunType==0x502)   {
        filmca = fopen("psa2D.csv","w");

        // title row (x index)
        for( ch=0; ch <NCHANNELS; ch++)
        {
            for( binx=0;binx<MCA2D_BINS;binx++)
            {
                fprintf(filmca,",%d",binx+MCA2D_BINS*ch);
            }
        }    // channel loop
        fprintf(filmca,"\n");

        for( biny=0;biny<MCA2D_BINS;biny++)
        {
            fprintf(filmca, "%d",biny);        // beginning of line
            for( ch=0; ch <NCHANNELS; ch++)
            {
                for( binx=0;binx<MCA2D_BINS;binx++)
                {
                    fprintf(filmca,",%d",mca2D[ch][biny+MCA2D_BINS*binx]);
                }  // binx loop
            }    // channel loop
            fprintf(filmca,"\n");            // end of line

        }  // biny loop

        fclose(filmca);
    }  // runtype 0x502

    // clean up
    if(RunType==0x400)   {    // write EOR: special hit pattern, all zero except WM
        TraceBlks = 0;
        hit = EORMARK;
        memcpy( buffer2 + 0, &(hit), 4 );
        memcpy( buffer2 + 4, &(TraceBlks), 2 );
        memcpy( buffer2 + 6, &(NumPrevTraceBlks), 2 );
        memcpy( buffer2 + 8, &(TraceBlks), 4 );
        memcpy( buffer2 + 12, &(TraceBlks), 4 );
        memcpy( buffer2 + 16, &(TraceBlks), 2 );
        memcpy( buffer2 + 18, &(TraceBlks), 2 );
        memcpy( buffer2 + 20, &(TraceBlks), 4 );
        memcpy( buffer2 + 24, &(TraceBlks), 4 );
        memcpy( buffer2 + 28, &(TraceBlks), 4 );
        memcpy( buffer2 + 32, &(TraceBlks), 4 );
        // no checksum  for now
        memcpy( buffer2 + 60, &(wm), 4 );
        fwrite( buffer2, 1, CHAN_HEAD_LENGTH_400*2, fil );
    }

    // Drain NTS accept/reject decisions
    while ((poll_result = nts_poll(nts)) > 0) {
        if (poll_result != NTS_IGNORE) {
            nts_received += poll_result;
        }
    }

    // Clean up NTS networking.
    printf("Cleaning up trigger sockets\n");
    nts_destroy(&nts);
    printf("NTS triggered %u, sent %u, accepted %u\n", nts_triggered, nts_sent, nts_received);

    fflush(stdout);

    pn_log_close();

    if( (RunType==0x500) || (RunType==0x501)  || (RunType==0x502) || (RunType==0x400) )  {
        fclose(fil);
    }
    flock( fd, LOCK_UN );
    munmap(map_addr, size);
    close(fd);
    return 0;
}
