/*----------------------------------------------------------------------
 * Copyright (c) 2018 XIA LLC
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

#include "MZTIODefs.h"

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
 

 */
typedef struct PixieNetFippiConfig {

  // ***** 16 system parameters (for C program only)  ******************************************************


   /** Number of ADC channels on the ADC carrier board.
      Typical value of 4. May be overwritten by EEPROM value.
   */

  unsigned int NUMBER_CHANNELS;     // unused

    /** Reserved for options in the C code, e.g printing errors.
      Currently unused
   */

  unsigned int C_CONTROL;         // unused

    /** The clock/real time to aquire data for, in seconds.  This will be an
      approximation for list mode data collection.
   */
  double REQ_RUNTIME;//         

  /** Number of data collection loops between grabbing the run statistic from 
      the FPGA.  This should probably be made in to a time in seconds, or 
      eliminated.  Typical value would be 900000
   */
  unsigned int POLL_TIME;
  
  // ***** 8 local control parameters (MZ FPGA control) ******************************************************
  // these are 32 bit numbers for now, but not all are used. 
  // they can eb floats etc if deined that wat, then the FPGA programming routine (progfippi)
  // must convert them in appropriate integers before writing to FPGA
  // TODO: give them meaningful names

  // define individual bits for reg 0, as demonstration principle.  

  unsigned int LOCAL_CONTROL_00;
  unsigned int LOCAL_CONTROL_01;// defines the signalling direction for each of the 16 LVDS drivers
  unsigned int LOCAL_CONTROL_02;
  unsigned int LOCAL_CONTROL_03;
  unsigned int LOCAL_CONTROL_04;
  unsigned int LOCAL_CONTROL_05;
  unsigned int LOCAL_CONTROL_06;
  unsigned int LOCAL_CONTROL_07;


     
  // ***** trigger control parameters (controls input/output routing ******************************************************


  unsigned int FRONT_A_OUTENA               ;    // enables output from MZ to frontpanle or backplane 
  unsigned int FRONT_B_OUTENA               ;
  unsigned int FRONT_C_OUTENA               ;
  unsigned int TRIGGERALL_OUTENA            ;
  unsigned int EBDATA_OUTENA                ;
  unsigned int FRONT_A_COINC_MASK           ;    // incoming signals are bit-wise ANDed with this mask 
  unsigned int FRONT_B_COINC_MASK           ;    // before being used in the coincidence test
  unsigned int FRONT_C_COINC_MASK           ;
  unsigned int TRIGGERALL_COINC_MASK        ;
  unsigned int EBDATA_COINC_MASK            ;
  unsigned int FRONT_A_MULT_MASK            ;     // incoming signals are bit-wise ANDed with this mask 
  unsigned int FRONT_B_MULT_MASK            ;     // before being used in the multiplicity test
  unsigned int FRONT_C_MULT_MASK            ;
  unsigned int TRIGGERALL_MULT_MASK         ;
  unsigned int EBDATA_MULT_MASK             ;
  unsigned int FRONT_A_COINC_PATTERN        ;     // test pattern for coincidence test
  unsigned int FRONT_B_COINC_PATTERN        ;     // masked coincidence pattern == pattern defined here --> logic high
  unsigned int FRONT_C_COINC_PATTERN        ;
  unsigned int TRIGGERALL_COINC_PATTERN     ;
  unsigned int EBDATA_COINC_PATTERN         ;
  unsigned int FRONT_A_MULT_THRESHOLD       ;    // threshold for multiplicity test
  unsigned int FRONT_B_MULT_THRESHOLD       ;    // multiplicity >= N defined here --> logic high
  unsigned int FRONT_C_MULT_THRESHOLD       ;
  unsigned int TRIGGERALL_MULT_THRESHOLD    ;
  unsigned int EBDATA_MULT_THRESHOLD        ;
  unsigned int FRONT_A_OUTPUT_SELECT        ;    // if used as output, choose here which signal to send out (see manual)
  unsigned int FRONT_B_OUTPUT_SELECT        ;
  unsigned int FRONT_C_OUTPUT_SELECT        ;
  unsigned int TRIGGERALL_OUTPUT_SELECT     ;
  unsigned int EBDATA_OUTPUT_SELECT         ;


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
