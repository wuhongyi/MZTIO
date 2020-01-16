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
 
#ifndef PixieNetConfig_h
#define PixieNetConfig_h

#include <stdint.h>

#include "PixieNetDefs.h"

/*  Functions and structs in this header are mainly concerned with configuring 
    the FPGA, data runs, and eventually reading other information out from the
    MCA.
 
    Note the functions in this header are implemented in c++, but can still be
    called from C programs.

    To add new parameters, its name has to be typed 5 times
    1) add line in ini files (2x)
    2) add element to struct in PixieNetConfig.h
    3) add parse/read line in PixieNetConfig.cpp
    4) use in progfippi or equivalent
 */

#ifdef __cplusplus
extern "C" {
#endif

  
/** Struct that represents information used to program the FPGA.
 
    Currently the PIXIE-NET system needs some of this information, during an
    actual data run, but things like TAU should eventually not be needed after
    doing the 'progfippi' step.
   
    Note variable names left the same as the config.ini originally provided to
    Sandia, for ease of changing things in the future.  
    Also note, this is just a first pass of creating this structure.
    Fields can be seperated by one or more spaces, tabs, commas or semicolons
 
    TODO: document meaning of various values.
    TODO: Make enums for the bitmask variables, so things are clearer.
    TODO: shrink variable types into smaller type. Ex Use float instead of 
          double, or uint16_t instead of usinged int, etc.
    TODO: rename member variables to better names.
    TODO: check that there is no double->int convertion issues for things like
          the rise times.
    TODO: implement writing settings files out to disk.
    TODO: consider implementing these settings as an opaque struct
 */
typedef struct PixieNetFippiConfig {
  //Currently unused parameters.
  //int SYS_U8, SYS_U7, SYS_U6, SYS_U5, SYS_U4, SYS_U3, SYS_U2, SYS_U1, SYS_U0;
  
  // ***** 16 system parameters ******************************************************


   /** Number of ADC channels on the ADC carrier board.
      Typical value of 4. May be overwritten by EEPROM value.
   */

  unsigned int NUMBER_CHANNELS;

    /** Reserved for options in the C code, e.g printing errors.
      Currently unused
   */

  unsigned int C_CONTROL;

    /** The clock/real time to aquire data for, in seconds.  This will be an
      approximation for list mode data collection.
   */
  double REQ_RUNTIME;//          300
  
  /** Number of data collection loops between grabbing the run statistic from 
      the FPGA.  This should probably be made in to a time in seconds, or 
      eliminated.  Typical value would be 900000
   */
  unsigned int POLL_TIME;

  /* SYS_U## : reserved parameters  */

  // ***** end of system parameters ********************************************

  /** Control bits for whole module.
      Typical value of 2048.
      Module control bit A.0: report only one event per Coinc. Window (LM402),
      Module control bit A.5: enables MMCX input as global Veto, NYI
      Module control bit A.7: toggles active edge for front panel pulse counter, NYI
   */
  unsigned int MODULE_CSRA;
  
  /** Control bits for whole module.  TODO: document
      Typical value of 0.
      Module control bit B.1: if 1, termination for ch.0,1 is 50 Ohm
      Module control bit B.2: if 1, termination for ch.2,3 is 50 Ohm
   */
  unsigned int MODULE_CSRB;
  
  
  /** Coincidence pattern for accepted between channels.
      0x0008 require channels 0 and 1 to be in coincidence.
      0x1000 require channels 2 and 3 to be in coincidence.
      0xFFFE = 65534 lets any hit pattern through.
   */
  unsigned int COINCIDENCE_PATTERN;
  
  /** Time between triggers, in micro-seconds, to consider the two hits to be a
      coincidence. 
      Typical value would be 0.040
   */
  double COINCIDENCE_WINDOW;

  /** Dictates what type of data is saved and the format.
      1280 (0x500) means save listmode data with waveforms.
      1281 (0x501) means save listmode without waveforms
       768 (0x301) Histogram only, do not save listmode data (I assume?)
   */
  unsigned int RUN_TYPE;
  
  /** The clock decimation factor for calculating trigger and energy values.
      May be from 1 to 6.  
      See section 6.5 of Pixie4e users manual for description
   */
  unsigned int FILTER_RANGE;

    /** The acceptance bitmask for individual events.
      0x01...0x08: If set, indicates that data for channel 0..3 have been recorded
      0x10: Logic level of FRONT panel input (unconfirmed)
      0x20: Result of LOCAL acceptance test
      0x40: Logic level of backplane STATUS line (unconfirmed)
      0x80: Logic level of backplane TOKEN line (= result of global coincidence test), see section 7 (unconfirmed)
   
      Will typically have value of 0x20 (dec 32).
   */
  unsigned int ACCEPT_PATTERN;
  
  /** If set to true, resets FPGA/DAC timers. */
  unsigned int SYNC_AT_START;
  
  /** Voltage, in volts, to output to DAC on from of PIXIE-NET system, intended
      to drive a high voltage power supply.
      Must be between 0 and 5.
   */
  double HV_DAC;
  
  /** Offboard serial.
      Typical value of 14000 
  */
  unsigned int SERIAL_IO;
  
  /** Typical value of 1 
  Bit0 : pulser enabled   */
  unsigned int AUX_CTRL;
  
  
  //Currently unused parameters.
  //unsigned int MOD_U3, MOD_U2, MOD_U1, MOD_U0;
  
  /** Options relating to the triggering of a single channel, as a bitmask bitmask.
      CCSRA_GROUP_00       0          if 1, respond to distributed group triggers, not local triggers
      CCSRA_U_01           0
      CCSRA_GOOD_02        1          if 0, channel will not be processed
      CCSRA_U_03           0
      CCSRA_TRIGENA_04     1          if 1, enable trigger (local and distributed)
      CCSRA_INVERT_05      1          if 1, ADC data is inverted before processing (for falling edge pulses)
      CCSRA_VETO_REJLO_06  0          if 1, reject events when global Veto is low
      CCSRA_U_07           0
      CCSRA_U_08           0
      CCSRA_NEGE_09        0          if 1, allow negative numbers as result of energy computation, NYI
      CCSRA_U_10           0
      CCSRA_U_11           0
      CCSRA_GATE_REJLO_12  0          if 1, reject events when channel-specific GATE signal is low
      CCSRA_U_13           0
      CCSRA_U_14           0
      CCSRA_TRIGGER16X_15  0          if 1, slow down trigger filter 16x
   
     To Read a PMT, you would typically use a value of
     (0x0004 | 0x0010 | 0x0020) = 180.
   */
  unsigned int CHANNEL_CSRA[NCHANNELS]; //         180    180    180      180
  
  /** Further channel specific options; has not been explored yet.
   Typically has a value of zero.
   Reserved for customized code and firmware 

   */
  unsigned int CHANNEL_CSRB[NCHANNELS]; //         0      0      0      0
  
  /** Further channel specific aquisition settings
      CCSRC_VETO_REJHI_00      0      if 1, reject events when global Veto is high
      CCSRC_GATE_REJHI_01      0      if 1, reject events when channel-specific Gate signal is high
      CCSRC_GATE_FROMVETO_02   0      if 1, use global Veto as the input for this channel's Gate logic
      CCSRC_PILEUP_DISABLE_03  0      if 1, disable pileup rejection
      CCSRC_RBAD_DISABLE_04    0      if 1, disable rejection of out-of-range events
      CCSRC_PILEUP_INVERT_05   0      if 1, accept only pulses that are piled up
      CCSRC_PILEUP_PAUSE_06    0      if 1, disable pileup inspection for 32 clock cycles after trigger. For ringing input signals.
      CCSRC_GATE_FEDGE_07      0      if 1, count Gate pulses on falling edge
      CCSRC_GATE_STATS_08      0      if 1, run statistics are in GATE mode, only counting while GATE in on
      CCSRC_VETO_FEDGE_09      0      if 1, count Veto pulses on falling edge
      CCSRC_GATE_ISPULSE_10    0      if 1, logic to re-pulse incoming Gate signal with specified GATE_WINDOW is enabled
      CCSRC_TRACE4X_11         0      if 1, captured traces and fast filter is 4x longer
      CCSRC_U_12               0
      CCSRC_U_13               0
      CCSRC_CPC2PSA_14         0      if 1, report gate pulse count as PSA value of list mode record 
      CCSRC_GATE_PULSEFEDGE_15 0      if 1, start pulse GATE_WINDOW at falling edge of Gate input signal
   
     Typical value would be 0, which enables pilup and rangebad (energy) rejection.
   */
  unsigned int CHANNEL_CSRC[NCHANNELS]; //         0      0    0      0
  
  double ENERGY_RISETIME[NCHANNELS]; //      0.256  0.256  0.256  0.256                Energy filter rise time
  double ENERGY_FLATTOP[NCHANNELS]; //       0.128  0.128  0.128  0.128                Energy filter flat top
  double TRIGGER_RISETIME[NCHANNELS]; //     0.048  0.048  0.048  0.048                Trigger filter rise time
  double TRIGGER_FLATTOP[NCHANNELS]; //      0.128  0.128  0.128  0.128                Trigger filter flat top
  double TRIGGER_THRESHOLD[NCHANNELS]; //    6.0      6.0      6.0      6.0            Trigger threshold
  double ANALOG_GAIN[NCHANNELS]; //          2.0    2.0    2.0    2.0                  Gain with switches/relays/VGAs
  double DIG_GAIN[NCHANNELS]; //             1.0   1.0   1.0   1.0                     Digital gain adjustment factor.
  double VOFFSET[NCHANNELS]; //              0.095515   0.103931  0.201588   0.207211  Offset
  double TRACE_LENGTH[NCHANNELS]; //         1.5  1.5  1.5  1.5                        Captured waveform length
  double TRACE_DELAY[NCHANNELS]; //          0.500  0.500  0.500  0.500                Pre-trigger delay
  unsigned int BINFACTOR[NCHANNELS]; //            1      1      1      1              MCA binning factor: divide by 2^N)
  double TAU[NCHANNELS]; //                  0.05   0.05   0.05   0.05                 Preamplifier decay time
  unsigned int BLCUT[NCHANNELS]; //                20     20     20     20             Threshold for bad baseline measurements
  double XDT[NCHANNELS]; //                  0.0667 0.0667 0.0667 0.0667               Sampling interval in untriggered traces, NYI
  double BASELINE_PERCENT[NCHANNELS]; //     10     10     10     10                   Target offset for baseline, nominally in percent, NYI
  unsigned int PSA_THRESHOLD[NCHANNELS]; //        25     25     25     25             Threshold in CFD and PSA,
  unsigned int INTEGRATOR[NCHANNELS]; //           0      0      0      0              Filter mode: 0-trapezoidal, 1-gap sum integral NYI, 2-ignore gap sum, NYI. 
  double GATE_WINDOW[NCHANNELS]; //          0.008  0.008  0.008  0.008                Coincidence window with gate
  double GATE_DELAY[NCHANNELS]; //           0.008  0.008  0.008  0.008                Delay of external gate signal
  double COINC_DELAY[NCHANNELS]; //          0.008  0.008  0.008  0.008                Delay of ADC signal before coincidence test
  unsigned int BLAVG[NCHANNELS]; //                65532  65532  65532  65532          Baseline averaging
  unsigned int QDC0_LENGTH[NCHANNELS]; //          0      0      0      0              Length of PSA sum
  unsigned int QDC1_LENGTH[NCHANNELS]; //          0      0      0      0              Length of PSA sum
  unsigned int QDC0_DELAY[NCHANNELS]; //           0      0      0      0              Delay of PSA sum relative to trigger point
  unsigned int QDC1_DELAY[NCHANNELS]; //           0      0      0      0              Delay of PSA sum relative to trigger point
  unsigned int QDC_DIV8[NCHANNELS];//             0      0      0      0              divide QDC sums by another factor 8, to fit in 64K max output number
  double MCA2D_SCALEX[NCHANNELS];    //     1      1      1      1                     scaling factor for 2D histogram (bin to increment is Ex/ MCA2D_SCALEX)
  double MCA2D_SCALEY[NCHANNELS];    //     1      1      1      1                     scaling factor for 2D histogram (bin to increment is Ey/ MCA2D_SCALEY)
  double PSA_NG_THRESHOLD[NCHANNELS];//     1      1      1      1                     threshold to distinguish neutrons and gammas in PSA parameter R=Q0/Q1
  unsigned int ADC_AVG[NCHANNELS]; //           0      0      0      0                 Number of samples to average for triggered oscilloscope mode
  unsigned int THRESH_ADC_AVG[NCHANNELS];//             0      0      0      0         Trigger thrshold (absolute ADC steps) for triggered oscilloscope mode 

  } PixieNetFippiConfig;


/** Parses the provided ini file into the provided PixieNetFipiConfig struct.
    \returns 0 upon success.
 
    Note that current requires that for each member variable of 
    PixieNetFipiConfig, the configuration file must have a line starting with 
    that identifier string, and followed by the expected number of numeric
    values.
 
    Blank lines, or lines starting with a '#' character are skipped, as are
    any lines with unrecognized identifiers.
 
    Integer numeric values that start with a '0x' prefix are assumed to be 
    hexidecimal.
 
    Currently does not due any range checking!  This is left to progfippi.

 */
int init_PixieNetFippiConfig_from_file( const char * const filename,
                                       int ignore_missing,
                                       struct PixieNetFippiConfig *config );
  
  
  
#ifdef __cplusplus
}
#endif

#endif
