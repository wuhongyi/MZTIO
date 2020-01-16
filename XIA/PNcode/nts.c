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

#include <stdlib.h>

#include "zhelpers.h"
#include "nts.h"
#include "log.h"

void nts_mark_event(const char *s)
{
    printf(s);
}

/*
 * Ring buffer to track triggers sent to the DM.
 */

NTSBuffer *nts_buffer_create()
{
    NTSBuffer *q = calloc(1, sizeof(NTSBuffer));
    if (!q)
        return NULL;

    q->size = NTS_MAX_WAIT;
    q->next = 0;
    q->start = -1;

    return q;
}

void nts_buffer_destroy(NTSBuffer **p)
{
    NTSBuffer *q = *p;

    // Free waveform memory for all triggers.
    if (q->start >= 0) {
        int i;
        for (i = 0; i < q->size; i++) {
            if (q->buf[i].ts)
                nts_trigger_close(&q->buf[i]);
        }
    }

    free(*p);
    *p = NULL;
}

static void nts_start_incr(NTSBuffer *q)
{
    // Free waveform memory piecewise as the ring buffer wraps around.
    nts_trigger_close(&q->buf[q->start]);

    q->start++;

    // Wrap start
    if (q->start == q->size) {
        nts_mark_event("W");
        q->start = 0;
    }
}

void nts_buffer_add(NTSBuffer *q, Trigger t)
{
    // Overwrite triggers
    if (q->next == q->start) {
        if (q->buf[q->start].stored)
            nts_mark_event("f"); // flush a stored trigger
        else
            nts_mark_event("x"); // overwrite a sent trigger

        nts_start_incr(q);
    }
    else {
        nts_mark_event("i");
    }

    // First trigger
    if (q->start == -1) {
        q->start = 0;
    }

    q->buf[q->next] = t;
    q->next++;

    // Wrap next
    if (q->next == q->size) {
        nts_mark_event("w");
        q->next = 0;
    }
}

/*
 * NTS class to manage the connections and buffers.
 */

/*
 * Close the sockets and destroy memory.
 */
void nts_destroy(NTS **nts)
{
    NTS *p = *nts;
    if (p->daq_trigger)
        zmq_close(p->daq_trigger);
    if (p->dm_ctrl)
        zmq_close(p->dm_ctrl);
    if (p->zctx)
        zmq_ctx_destroy(p->zctx);
    nts_buffer_destroy(&p->sent);
    free(p);
    *nts = NULL;
}

/*
 * Allocate an NTS object and open the sockets. This also blocks to
 * synchronize run start with the DM (it could be separated to a start
 * function).
 */
NTS *nts_open(const char *dm_host, int dm_port)
{
    int rc;

    NTS *nts = malloc(sizeof(NTS));
    if (!nts) {
        printf("No memory for NTS\n");
        return NULL;
    }

    nts->sent = nts->zctx = nts->dm_ctrl = nts->daq_trigger = NULL;

    nts->zctx = zmq_ctx_new();
    if (!nts->zctx) {
        printf("Failed opening zmq context\n");
        nts_destroy(&nts);
        return NULL;
    }

    printf("Connecting to DM at %s:%d\n", dm_host, dm_port);

    nts->dm_ctrl = zmq_socket(nts->zctx, ZMQ_SUB);
    if (!nts->dm_ctrl) {
        printf("Failed to open SUB socket for control\n");
        nts_destroy(&nts);
        return NULL;
    }

    char endpoint[100];
    sprintf(endpoint, "tcp://%s:%d", dm_host, dm_port);
    rc = zmq_connect(nts->dm_ctrl, endpoint);
    if (rc < 0) {
        printf("Connecting DM errno=%d\n", errno);
        nts_destroy(&nts);
        return NULL;
    }
    zmq_setsockopt(nts->dm_ctrl, ZMQ_SUBSCRIBE, "", 0);

    printf("Waiting for DM INIT\n");
    while (1) {
        char *s = s_recv(nts->dm_ctrl);
        if (s && strcmp(s, "INIT") == 0) {
            free(s);
            break;
        }
        if (s) {
            printf("DAQ init ignore %s\n", s);
            free(s);
        }
    }

    printf("DAQ recv DM INIT\n");

    nts->daq_trigger = zmq_socket(nts->zctx, ZMQ_PUSH);
    if (!nts->daq_trigger) {
        printf("Failed to open PUSH socket for triggers\n");
        nts_destroy(&nts);
        return NULL;
    }

    sprintf(endpoint, "tcp://%s:%d", dm_host, dm_port + 1);
    zmq_connect(nts->daq_trigger, endpoint);

    int linger = 0;
    zmq_setsockopt(nts->daq_trigger, ZMQ_LINGER, &linger, sizeof(linger));

    s_send(nts->daq_trigger, "HELLO");

    nts->sent = nts_buffer_create();
    if (!nts->sent) {
        printf("Failed creating sent queue\n");
        nts_destroy(&nts);
        return NULL;
    }

    printf("Waiting for DM START\n");
    while (1) {
        char *s = s_recv(nts->dm_ctrl);
        if (s && strcmp(s, "START") == 0) {
            free(s);
            break;
        }
        if (s) {
            if (strcmp(s, "INIT") == 0)
                nts_mark_event("."); // Wait for other DAQs to start
            else
                printf("DAQ idle ignore %s\n", s);

            free(s);
        }
    }

    return nts;
}

/*
 * Send trigger metadata to the DM and buffer it locally to await a
 * decision. If data is set, the buffer takes ownership and will free
 * it after the trigger expires out of the buffer.
 */
void nts_trigger(NTS *nts, unsigned int revsn, int ch, unsigned long long ts,
                 time_t currenttime, void *data)
{
    // ASCII encoding for now
    char msg_buf[1024];
    sprintf(msg_buf, "TRIGGER %u,%u,%llu", revsn, ch, ts);

    Stopwatch sw = sw_start();
    s_send(nts->daq_trigger, msg_buf);
    sw_check(&sw, "Trigger send ts=%llu", ts);

    Trigger trigger_item = {ts, currenttime, false, data};

    sw = sw_start();
    nts_buffer_add(nts->sent, trigger_item);
    sw_check(&sw, "nts_buffer_add ts=%llu", ts);
}

/*
 * Free event data associated with the trigger, if set.
 */
void nts_trigger_close(Trigger *t)
{
    if (t->data) {
        free(t->data);
        t->data = NULL;
    }
}

/*
 * Process an accept decision from the DM. Searches the local buffer
 * for events matching the time range and simulates storing the data.
 */
int nts_store(NTS *nts, const char *msg)
{
    unsigned long long t1, t2;
    int n = sscanf(msg, "ACCEPT %llu,%llu", &t1, &t2);
    if (n < 2) {
        printf("E: can't match ACCEPT: %s\n", msg);
        return 0;
    }

    pn_log("%s", msg);

    NTSBuffer *buf = nts->sent;
    if (buf->start < 0 || buf->start >= buf->size) {
        printf("E: invalid buffer start %d\n", buf->start);
        return 0;
    }

    bool any_match = false;
    int stored = 0;
    int i = buf->start;
    int j = 0; // Check total iterations
    do {
        j++;

        Trigger *t = &buf->buf[i];
        if (t->ts >= t1 && t->ts < t2) {
            if (t->stored) {
                nts_mark_event("d"); // duplicate accept
            }
            else {
                nts_mark_event("s");
                t->stored = true;
                stored++;
                pn_log("Stored t=%llu", t->ts);
            }

            any_match = true;

            // Advance the start pointer if there are no triggers behind it.
            if (i == buf->start)
                nts_start_incr(buf);
        }

        i++;
        if (i == buf->size) {
            i = 0;
        }
    } while (i != buf->next);

    if (j > buf->size)
        printf("E: too many store search iterations j=%d size=%d\n", j, buf->size);

    if (!any_match)
        nts_mark_event("u");

    pn_log("nts_store done");

    return stored;
}

/*
 * Poll for any control messages from the DM.
 * Returns:
 *   - negative for errors
 *   - -1 if the run END message is received
 *   - 0 if there's nothing to do
 *   - NTS_IGNORE if the message is unrecognized
 *   - positive N if N triggers were accepted and stored
 */
int nts_poll(NTS *nts)
{
    zmq_pollitem_t zpoll[1];
    zpoll[0].socket = nts->dm_ctrl;
    zpoll[0].events = ZMQ_POLLIN;
    zpoll[0].fd = 0;
    zpoll[0].revents = 0;

    Stopwatch sw = sw_start();
    int n = zmq_poll(zpoll, sizeof(zpoll) / sizeof(zpoll[0]), 0);
    sw_check(&sw, "poll dm_ctrl");

    if (n <= 0)
        return n;

    sw = sw_start();
    char *s = s_recv(nts->dm_ctrl);
    if (!s) {
        printf("DAQ recv null!\n");
        return 0;
    }
    sw_check(&sw, "s_recv dm_ctrl");

    int ret;
    if (strncmp(s, "ACCEPT", 6) == 0) {
        sw = sw_start();
        ret = nts_store(nts, s);
        sw_check(&sw, "nts_store");
    }
    else if (strncmp(s, "END", 3) == 0) {
        printf("DAQ END\n");
        ret = -1;
    }
    else {
        printf("DAQ recv [%s]\n", s);
        ret = NTS_IGNORE;
    }

    sw = sw_start();
    free(s);
    sw_check(&sw, "free s_recv msg");

    return ret;
}
