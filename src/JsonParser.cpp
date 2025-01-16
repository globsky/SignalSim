//----------------------------------------------------------------------
// JsonParser.cpp:
//   JSON stream process class implementation
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <malloc.h>
#include <math.h>
#include <string.h>
#include "JsonParser.h"

JsonStream::JsonStream()
{
	RootObject = NULL;
	stream[0] = '\0';
	p = stream;
}

JsonStream::~JsonStream()
{
	if (RootObject)
		DeleteTree(RootObject);
}

void JsonStream::DeleteTree(JsonObject *Object)
{
	JsonObject *NextObject;

	do
	{
		if (Object->Type == JsonObject::ValueTypeObject || Object->Type == JsonObject::ValueTypeArray)
			DeleteTree(Object->pObjectContent);
		NextObject = Object->pNextObject;
		delete Object;
	} while ((Object = NextObject) != NULL);
}

int JsonStream::ReadFile(const char *File)
{
	FILE *fp = fopen(File, "r");

	if (fp == NULL)
		return -1;
	p = stream;
	RootObject = ParseObject(0, &JsonStream::GetFileStream, (void *)fp);
	fclose(fp);

	return 0;
}

int JsonStream::WriteFile(const char *File)
{
	FILE *fp = fopen(File, "w");

	if (fp == NULL)
		return -1;
	if (RootObject != NULL)
	{
		OutputObject(fp, RootObject->pObjectContent, 1, 1);
	}
	fclose(fp);

	return 0;
}

JsonObject *JsonStream::ParseObject(int IsObject, GetStream GetStreamFunc, void *source)
{
	JsonObject *Object = NULL, *CurObject = NULL;
	int stage = IsObject ? 0 : 3;

	if (!IsObject)	// for array, create new object list
	{
		CurObject = Object = GetNewObject();
		if (Object == NULL)
			return NULL;
	}

	while (1)
	{
		if (*p == '\0')
			if ((this->*GetStreamFunc)(source) < 0)
				break;
		if (*p == '\0')
			continue;

		switch (stage)
		{
		case 0:	// waiting for '{'
			if (*p == '{')
			{
				stage = 1;
				CurObject = Object = GetNewObject();
				if (Object == NULL)
					return NULL;
			}
			break;
		case 1:	// waiting for '\"' as start of key
			if (*p == '\"')
			{
				CopyString(CurObject->Key, MAX_KEY_LENGTH);
				if (strcmp(CurObject->Key, "elevationAdjust") == 0)
					stage = stage;
				stage = 2;
			}
			break;
		case 2:	// waiting for ':'
			if (*p == ':')
				stage = 3;
			break;
		case 3:	// waiting for value
			if (*p == '{')	// value is an object
			{
				CurObject->Type = JsonObject::ValueTypeObject;
				CurObject->pObjectContent = ParseObject(1, GetStreamFunc, source);
			}
			else if (*p == '[')	// value is an array
			{
				p ++;
				CurObject->Type = JsonObject::ValueTypeArray;
				CurObject->pObjectContent = ParseObject(0, GetStreamFunc, source);
			}
			else if (!IsWhiteSpace(*p))
				GetValueContent(CurObject);
			else
				break;
			stage = 4;
			break;
		case 4:	// determine whether end of object or next key/value pair
			if (*p == '}' || *p == ']')
				return Object;
			else if (*p == ',')
			{
				CurObject->pNextObject = GetNewObject();
				if (CurObject->pNextObject == NULL)
					return Object;
				CurObject = CurObject->pNextObject;
				stage = IsObject ? 1 : 3;	// go to next key/value pair or value
			}
		}
		p ++;
	}

	return Object;
}

JsonObject *JsonStream::GetNewObject()
{
	JsonObject *Object = new JsonObject;
	Object->Key[0] = '\0';
	Object->Type = JsonObject::ValueTypeNull;	// initialize with NULL object
	Object->pNextObject = Object->pObjectContent = NULL;
	return Object;
}

int JsonStream::GetFileStream(void *source)
{
	FILE *fp = (FILE *)source;

	if (feof(fp))
		return -1;
	fgets(stream, 255, fp);
	p = stream;
	return 1;
}

int JsonStream::CopyString(char *dest, int MaxLength)
{
	int Length = 0;

	p ++;	// skip starting '\"'
	while (*p != '\0' && *p != '\"')
	{
		if (Length < MaxLength)
		{
			if (*p == '\\')
				*dest ++ = EscapeCharacter();
			else
				*dest ++ = *p;
			Length ++;
		}
		p ++;
	}
	*dest = '\0';
	if (*p == '\0')
		p --;
	return Length;
}

int JsonStream::IsWhiteSpace(const char ch)
{
	if (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t')
		return 1;
	else
		return 0;
}

char JsonStream::EscapeCharacter()
{
	int hex;
	char ch = *(++p);

	switch (ch)
	{
	case '\"':	// ASCII 22
	case '\\':	// ASCII 5C
	case '/':	// ASCII 2F
		return ch;
	case 'b':	// ASCII 08
		return '\b';
	case 'f':	// ASCII 0C
		return '\f';
	case 'n':	// ASCII 0A
		return '\n';
	case 'r':	// ASCII 0D
		return '\r';
	case 't':	// ASCII 09
		return '\t';
	case 'u':
		hex = ((int)(*(p+1)) << 24) | ((int)(*(p+2)) << 16) | ((int)(*(p+3)) << 8) | ((int)(*(p+4)) << 0);
		p += 4;
		return (char)hex;
	}
	return '\0';
}

int JsonStream::GetValueContent(JsonObject *Object)
{
	if (*p == '\"')	// value is string
	{
		Object->Type = JsonObject::ValueTypeString;
		return CopyString(Object->String, MAX_STRING_LENGTH);
	}
	else if (*p == '-' || (*p >= '0' && *p <= '9'))	// value is number
		return GetNumber(Object);
	else if (*p == 't' || *p == 'T')
		Object->Type = JsonObject::ValueTypeTrue;
	else if (*p == 'f' || *p == 'F')
		Object->Type = JsonObject::ValueTypeFalse;
	else if (*p == 'n' || *p == 'N')
		Object->Type = JsonObject::ValueTypeNull;
	Object->pObjectContent = NULL;

	while (*p != '\0' && *p != ',' && !(IsWhiteSpace(*p)))
		p ++;
	p --;
	return 0;
}

int JsonStream::GetNumber(JsonObject *Object)
{
	int section = 0, exp = 0, sign = 0, exp_sign = 0, finish = 0;
	double fraction = 0.1;

	Object->Type = JsonObject::ValueTypeIntNumber;
	Object->Number.l_data = 0;
	if (*p == '-')
	{
		sign = 1;
		p ++;
	}
	while (*p != '\0')
	{
		switch (section)
		{
		case 0:	// integer part
			if (*p >= '0' && *p <= '9')
				Object->Number.l_data = Object->Number.l_data * 10 + (*p - '0');
			else if (*p == '.' || *p == 'e' || *p == 'E')
			{
				Object->Number.d_data = (double)Object->Number.l_data;
				Object->Type = JsonObject::ValueTypeFloatNumber;
				section = (*p == '.') ? 1 : 2;
			}
			else
				finish = 1;
			break;
		case 1:	// fraction part
			if (*p >= '0' && *p <= '9')
			{
				Object->Number.d_data += fraction * (*p - '0');
				fraction *= 0.1;
			}
			else if (*p == 'e' || *p == 'E')
				section = 2;
			else
				finish = 1;
			break;
		case 2:	// exponent part
			if (*p == '+' || *p == '-')
				exp_sign = (*p == '-') ? 1 : 0;
			else if (*p >= '0' && *p <= '9')
				exp = exp * 10 + (*p - '0');
			else
				finish = 1;
			break;
		}
		if (finish)
		{
			p --;
			break;
		}
		p ++;
	}
	if (Object->Type == JsonObject::ValueTypeFloatNumber)
	{
		if (exp_sign)
			exp = -exp;
		Object->Number.d_data *= pow(10., (double)exp);
		if (sign)
			Object->Number.d_data = -Object->Number.d_data;
	}
	else if (sign)	// negative integer
		Object->Number.l_data = -Object->Number.l_data;

	return 0;
}

int JsonStream::OutputObject(FILE *fp, JsonObject *Object, int Depth, int HasKey)
{
	int i;

	if (HasKey)
		fprintf(fp, "{\n");
	else
		fprintf(fp, "[\n");
	while (Object)
	{
		OutputKeyValue(fp, Object, Depth, HasKey);
		Object = Object->pNextObject;
		if (Object)
			fprintf(fp, ",\n");
	}
	fputc('\n', fp);
	for (i = 0; i < Depth - 1; i ++)
	fputc('\t', fp);
	fputc(HasKey ? '}' : ']', fp);
	return 0;
}

int JsonStream::OutputKeyValue(FILE *fp, JsonObject *Object, int Depth, int HasKey)
{
	int i;

	if (Object == NULL)
		return -1;
	for (i = 0; i < Depth; i ++)
		fputc('\t', fp);
	if (HasKey)
	{
		OutputString(fp, Object->Key);
		fprintf(fp, ": ");
	}
	switch (Object->Type)
	{
	case JsonObject::ValueTypeObject:
		OutputObject(fp, Object->pObjectContent, Depth + 1, 1);
		break;
	case JsonObject::ValueTypeArray:
		OutputObject(fp, Object->pObjectContent, Depth + 1, 0);
		break;
	case JsonObject::ValueTypeString:
		OutputString(fp, Object->String);
		break;
	case JsonObject::ValueTypeIntNumber:
		fprintf(fp, "%lld", Object->Number.l_data);
		break;
	case JsonObject::ValueTypeFloatNumber:
		fprintf(fp, "%lf", Object->Number.d_data);
		break;
	case JsonObject::ValueTypeTrue:
		fprintf(fp, "true");
		break;
	case JsonObject::ValueTypeFalse:
		fprintf(fp, "false");
		break;
	case JsonObject::ValueTypeNull:
		fprintf(fp, "null");
		break;
	}

	return 0;
}

int JsonStream::OutputString(FILE *fp, const char *str)
{
	fputc('\"', fp);
	while (*str)
	{
		switch (*str)
		{
		case '\b':
			fputc('\\', fp); fputc('b', fp); break;
		case '\t':
			fputc('\\', fp); fputc('t', fp); break;
		case '\n':
			fputc('\\', fp); fputc('n', fp); break;
		case '\f':
			fputc('\\', fp); fputc('f', fp); break;
		case '\r':
			fputc('\\', fp); fputc('r', fp); break;
		case '\"':
			fputc('\\', fp); fputc('\"', fp); break;
		case '/':
			fputc('\\', fp); fputc('/', fp); break;
		case '\\':
			fputc('\\', fp); fputc('\\', fp); break;
		default:
			if (*str < 0x20 || *str >= 0x7f)
				fprintf(fp, "\\u%04x", (unsigned int)(*str));
			else
				fputc(*str, fp);
		}
		str ++;
	}
	fputc('\"', fp);
	return 0;
}
