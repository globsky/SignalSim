//----------------------------------------------------------------------
// MessageOutput.h:
//   Definition of functions to output messages
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__MESSAGE_OUTPUT_H__)
#define __MESSAGE_OUTPUT_H__

#include <stdio.h>

#define MSG_LEVEL_INFO 0
#define MSG_LEVEL_WARNING 1
#define MSG_LEVEL_ERROR 2
#define MSG_LEVEL_CRITICAL 3
#define MSG_LEVEL_OFF 4	// used only in SetOutputLevel()

FILE *SetOutputFile(FILE *fp);
int SetOutputLevel(int Level);
void MessagePrint(int LogLevel, const char* format, ...);

#endif //!defined(__MESSAGE_OUTPUT_H__)
