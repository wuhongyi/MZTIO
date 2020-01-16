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

#include <stdbool.h>

struct _Trigger {
    unsigned long long ts;  // trigger timestamp
    time_t queue_time;      // time inserted
    bool stored;            // set after a trigger is accepted
    void *data;             // mode-specific data block
};
typedef struct _Trigger Trigger;

#define NTS_MAX_WAIT 200
struct _NTSBuffer {
    Trigger buf[NTS_MAX_WAIT];
    int size;
    int next;
    int start;
};
typedef struct _NTSBuffer NTSBuffer;

struct _NTS {
    NTSBuffer *sent;
    void *zctx;
    void *dm_ctrl;     // run control and accept/reject SUB
    void *daq_trigger; // DAQ triggers PUSH
};
typedef struct _NTS NTS;

NTS *nts_open(const char *dm_host, int dm_port);
void nts_destroy(NTS **nts);
void nts_trigger(NTS *nts, unsigned int revsn, int ch, unsigned long long ts,
                 time_t currenttime, void *data);
void nts_trigger_close(Trigger *t);
int nts_poll(NTS *nts);

#define NTS_IGNORE 0xFFFFFFFF
