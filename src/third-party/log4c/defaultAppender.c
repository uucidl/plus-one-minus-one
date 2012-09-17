// $Id: defaultAppender.c,v 1.7 2006/10/20 05:31:56 nicolas Exp $
// Copyright (c) 2001, Bit Farm, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "log4c.h"
#include <stdio.h>

#if defined(PSP)
#include <pspdebug.h>
#endif

/**
 * The root category's default logging function.
 */

static char *priorityNames[] = {
    "Zero Priority",
    "TRACE",
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARNING",
    "ERROR",
    "CRITICAL ERROR",
    "ALERT",
    "EMERGENCY",
};

static void doAppend(struct LogAppender* this, struct LogEvent* ev);

static struct DefaultLogAppender {
    struct LogAppender appender;
    FILE *file;
} defaultLogAppender = { { doAppend }, NULL } ;

struct LogAppender* log_defaultLogAppender  = &defaultLogAppender.appender;

static void doAppend(struct LogAppender* this0, struct LogEvent* ev) {

    // TODO: define a format field in struct for timestamp, etc.
    char *pn = "default";
    char buf[20];
    struct DefaultLogAppender* this = (struct DefaultLogAppender*)this0;
    
    if (this->file == NULL) this->file = stderr;
    
    int priorityNames_n = sizeof(priorityNames)/sizeof(char*);

    if (ev->priority < 0) {
        pn = "Negative Priority NOT ALLOWED!!";
    }
    else if (ev->priority < priorityNames_n) {
        pn = priorityNames[ev->priority];
    } else {
        sprintf(buf, "%s+%ld",
                priorityNames[priorityNames_n-1],
                (long) ev->priority - priorityNames_n + 1);
    }
#if defined(KOS)
    printf("%-7s ", pn);
    printf("%s:%d: (%s) ", ev->fileName, ev->lineNum, ev->functionName);
    char buffer[1024];
    vsprintf(buffer, ev->fmt, ev->ap);
    printf(buffer);
    printf("\n");
#elif defined(PSP)
    printf("%-7s ", pn);
    printf("%s:%d: (%s) ", ev->fileName, ev->lineNum, ev->functionName);
    char buffer[1024];
    vsprintf(buffer, ev->fmt, ev->ap);
    pspDebugScreenPrintf(buffer);
    pspDebugScreenPrintf("\n");
#else
    fprintf(this->file, "%-7s ", pn);
    fprintf(this->file, "%s:%d: (%s) ", ev->fileName, ev->lineNum, ev->functionName);
    vfprintf(this->file, ev->fmt, ev->ap);
    fprintf(this->file, "\n");
#endif
}
