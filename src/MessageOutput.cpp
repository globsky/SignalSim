//----------------------------------------------------------------------
// MessageOutput.h:
//   Implementation of functions to output messages
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <stdio.h>
#include <cstdarg>

#include "MessageOutput.h"

static int OutputLevel = MSG_LEVEL_ERROR;
static FILE *fpOutput = NULL;
static const char *LevelString[] = {
    "[INFO] ", "[WARNING]", "[ERROR] ", "[CRITICAL] ",
};

FILE *SetOutputFile(FILE *fp)
{
	return (fpOutput = fp);
}

int SetOutputLevel(int Level)
{
    if (Level >= MSG_LEVEL_INFO && Level <= MSG_LEVEL_OFF)
        OutputLevel = Level;
	return OutputLevel;
}

void MessagePrint(int LogLevel, const char* format, ...)
{
    if (LogLevel < OutputLevel || LogLevel > MSG_LEVEL_CRITICAL || fpOutput == NULL)
        return;

    fprintf(fpOutput, "%s", LevelString[LogLevel]);
    
    va_list args;
    va_start(args, format);
    vfprintf(fpOutput, format, args);
    va_end(args);
}
