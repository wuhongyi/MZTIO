/*
 * A logging system to log to a file with elapsed time and run loop tags.
 */

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

#include <sys/time.h>
#include "log.h"

static FILE *OUT;
static struct timeval START;
static unsigned int LOOP = 0;

/*
 * Open a log file with the given fname. The start time is set to the
 * current time and loop counter is set to zero.
 */
int pn_log_open(const char *fname)
{
    pn_log_close();

    gettimeofday(&START, NULL);
    pn_log_loop(0);

    OUT = fopen(fname, "wb");

    return OUT ? 0 : -1;
}

/*
 * Close the file.
 */
void pn_log_close()
{
    if (OUT) {
        fclose(OUT);
        OUT = NULL;
    }
}

/*
 * Set the run loop counter to use for subsequent log messages.
 */
void pn_log_loop(unsigned int loop)
{
    LOOP = loop;
}

/*
 * Write a formatted message to the log. Does nothing if the file is
 * not open. The message is prefixed by the elapsed time in seconds
 * with microseconds precision and the current loop tag.
 */
void pn_log(char *fmt, ...)
{
    if (!OUT)
        return;

    va_list args;
    va_start(args, fmt);

    struct timeval now;

    gettimeofday(&now, NULL);
    long t = ((now.tv_sec - START.tv_sec) * 1000000L) +
        now.tv_usec - START.tv_usec;

    fprintf(OUT, "%ld.%06ld [%u] ", t / 1000000L, t % 1000000L, LOOP);

    vfprintf(OUT, fmt, args);
    va_end(args);

    fprintf(OUT, "\n");
}

/*
 * Elapsed time helpers
 */

/*
 * Diff two timevals in microseconds.
 */
long difftimeofday(struct timeval end, struct timeval start)
{
    long dt = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    return dt;
}

/*
 * Return a stopwatch initialized to the current time. Use with sw_elapsed or sw_check.
 */
Stopwatch sw_start()
{
    Stopwatch sw;
    gettimeofday(&sw.t0, NULL);
    return sw;
}

/*
 * Get the elapsed time in microseconds.
 */
long sw_elapsed(Stopwatch *sw)
{
    gettimeofday(&sw->t1, NULL);
    return difftimeofday(sw->t1, sw->t0);
}

/*
 * Check the elapsed time and print a message if it's longer than the default 1ms.
 */
void sw_check(Stopwatch *sw, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    long dt = sw_elapsed(sw);

    if (dt > 1000) {
        vprintf(fmt, args);
        printf(" dt=%ld\n", dt);
    }
}
