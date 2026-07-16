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

JsonObject::JsonObject()
{
	Key[0] = '\0';
	Type = JsonObject::ValueTypeNull;	// initialize with NULL object
	pNextObject = pObjectContent = NULL;
	pParent = NULL;
}

JsonObject::JsonObject(JsonObject *Parent)
{
	Key[0] = '\0';
	Type = JsonObject::ValueTypeNull;	// initialize with NULL object
	pNextObject = pObjectContent = NULL;
	pParent = Parent;
}
/*
JsonObject::JsonObject(const JsonObject& Object)
{
	// copy all contents
	memcpy(this, &Object, sizeof(JsonObject));
	// this is a standalone object with only contents been copied
	pNextObject = pObjectContent = NULL;
	pParent = NULL;
}*/

JsonObject::~JsonObject()
{
}

// search objects matching specified Key
// if SearchSubitem is 1, search all subitems recursively (will only return the first found item)
// return pointer matching Key or NULL if not found
JsonObject *JsonObject::SearchSubitems(const char *KeyName, int SearchSubitem, const char *StrValue)
{
	JsonObject *MatchObject = NULL, *Content;
	bool Match = 0;

	Match = (strcmp(Key, KeyName) == 0) ? true : false;
	if (StrValue)
		Match = Match && (Type == JsonObject::ValueTypeString) && (strcmp(String, StrValue) == 0);

	if (Match)
		return this;
	else if (SearchSubitem && (Type == JsonObject::ValueTypeObject || Type == JsonObject::ValueTypeArray))
	{
		for (Content = GetFirstObject(); Content; Content = Content->GetNextObject())
		{
			if ((MatchObject = Content->SearchSubitems(KeyName, 1, StrValue)) != 0)
				break;
		}
	}
	return MatchObject;
}

// get the index of the current JSON node in the parent array or object
// return -1 if not in a array or object
int JsonObject::GetIndex()
{
	int i;
	JsonObject *pObject = pParent;
	if (!pObject || (pObject->Type != JsonObject::ValueTypeArray && pObject->Type != JsonObject::ValueTypeObject))
		return -1;
	for (i = 0, pObject = pObject->GetFirstObject(); pObject; i ++, pObject = pObject->GetNextObject())
	{
		if (pObject == this)
			return i;
	}
	return -1;
}

// get the size of current JSON object (number of key/value pairs or size of array)
// return 0 if not in a array or object or an empty object or array
int JsonObject::GetSize()
{
	int size = 0;
	JsonObject *pObject;
	if (Type != JsonObject::ValueTypeArray && Type != JsonObject::ValueTypeObject)
		return size;
	for (pObject = GetFirstObject(); pObject; pObject = pObject->GetNextObject())
		size ++;
	return size;
}

// get the indexed object in current JSON object (key/value pair for object or object for array)
// return NULL if not in a array or object or exceed the size
JsonObject *JsonObject::operator[](int Index)
{
	int i;
	JsonObject *pObject;
	if (Type != JsonObject::ValueTypeArray && Type != JsonObject::ValueTypeObject)
		return NULL;
	for (i = 0, pObject = GetFirstObject(); pObject; i ++, pObject = pObject->GetNextObject())
	{
		if (i == Index)
			break;
	}
	return pObject;
}

int JsonObject::ReplaceValue(const char *KeyName, void *p, int SearchSubitem)
{
	p;	// p is not used, only as a place holder to set NULL
	JsonObject *Object = SearchSubitems(KeyName, SearchSubitem);
	if (Object == NULL)
		return 0;
	Object->Type = JsonObject::ValueTypeNull;
	return 1;
}

int JsonObject::ReplaceValue(const char *KeyName, const char *Value, int SearchSubitem)
{
	JsonObject *Object = SearchSubitems(KeyName, SearchSubitem);
	if (Object == NULL)
		return 0;
	Object->Type = JsonObject::ValueTypeString;
	strcpy(Object->String, Value);
	return 1;
}

int JsonObject::ReplaceValue(const char *KeyName, int Value, int SearchSubitem)
{
	JsonObject *Object = SearchSubitems(KeyName, SearchSubitem);
	if (Object == NULL)
		return 0;
	Object->Type = JsonObject::ValueTypeIntNumber;
	Object->Number.l_data = Value;
	return 1;
}

int JsonObject::ReplaceValue(const char *KeyName, double Value, int SearchSubitem)
{
	JsonObject *Object = SearchSubitems(KeyName, SearchSubitem);
	if (Object == NULL)
		return 0;
	Object->Type = JsonObject::ValueTypeFloatNumber;
	Object->Number.d_data = Value;
	return 1;
}

int JsonObject::ReplaceValue(const char *KeyName, bool Value, int SearchSubitem)
{
	JsonObject *Object = SearchSubitems(KeyName, SearchSubitem);
	if (Object == NULL)
		return 0;
	Object->Type = Value ? JsonObject::ValueTypeTrue : JsonObject::ValueTypeFalse;
	return 1;
}

// because delete tree may delete current object and the whole link list
// so this function is a static function with current object as parameter
// if DeleteLink is 0, will only delete Object and subitems
// if DeleteLink is 1, will delete Object and whole link list from Object
void JsonObject::DeleteTree(JsonObject *Object, int DeleteLink)
{
	JsonObject *NextObject;

	if (Object == NULL)
		return;
	do
	{
		if (Object->Type == JsonObject::ValueTypeObject || Object->Type == JsonObject::ValueTypeArray)
			DeleteTree(Object->pObjectContent, 1);
		NextObject = Object->pNextObject;
		delete Object;
	} while (DeleteLink && (Object = NextObject) != NULL);
}

// replace the position of OldObject with NewObject
// need to set correct pointer to maintain link list
// OldObject will be destroyed after replacement
// return NewObject if place successfully or OldObject if failed
JsonObject *JsonObject::ReplaceObject(JsonObject *OldObject, JsonObject *NewObject)
{
	if (OldObject == NULL)
		return NULL;
	else if (OldObject == NewObject)
		return NewObject;
	JsonObject *ParentObject = OldObject->pParent, *CurObject = OldObject;

	// if parent object exist, find the pointer of the previous one
	if (ParentObject)
	{
		CurObject = ParentObject->pObjectContent;
		if (CurObject == OldObject)
		{
			ParentObject->pObjectContent = NewObject;
			NewObject->pNextObject = OldObject->pNextObject;
		}
		else
		{
			while (CurObject && CurObject->pNextObject != OldObject)
				CurObject = CurObject->pNextObject;
			if (CurObject)	// found previous object
			{
				// do replacement
				CurObject->pNextObject = NewObject;
				NewObject->pNextObject = OldObject->pNextObject;
				// let CurObject point to OldObject
				CurObject = OldObject;	
			}
		}
	}

	// if replace successfully CurObject has the value of OldObject otherwise it is NULL
	if (CurObject)
	{
		NewObject->pParent = ParentObject;
		DeleteTree(CurObject);
		return NewObject;
	}
	else
		return OldObject;
}

// remove current object from tree
// if this object has parent remove it from the content link before delete the tree
void JsonObject::RemoveObject(JsonObject *Object)
{
	JsonObject *ParentObject = Object->pParent, *CurObject = Object;

	// if parent object exist, find the pointer of the previous one
	if (ParentObject)
	{
		CurObject = ParentObject->pObjectContent;
		if (CurObject == Object)
		{
			ParentObject->pObjectContent = Object->pNextObject;
			Object->pNextObject = NULL;
		}
		else
		{
			while (CurObject && CurObject->pNextObject != Object)
				CurObject = CurObject->pNextObject;
			if (CurObject)	// found previous object
			{
				// skip object to be removed
				CurObject->pNextObject = Object->pNextObject;
				Object->pNextObject = NULL;
			}
		}
	}
	DeleteTree(Object);
}

// add a new key/value paire to current object or array 
// current object must be the type of object or array
// if FollowingKey is NULL, the NewObject will be the first object
// otherwise the NewObject will be added to the position following object with corresponding key
// if no match for FollowingKey, NewObject will be added to last
// if current object is an array, NULL pointer FollowingKey will do inserting first
// otherwise will do appending last
int JsonObject::AddObject(JsonObject *NewObject, const char *FollowingKey)
{
	JsonObject *pObj;

	if (Type != JsonObject::ValueTypeObject && Type != JsonObject::ValueTypeArray)
		return 0;
	pObj = GetFirstObject();
	if (FollowingKey == NULL || pObj == NULL)
	{
		pObjectContent = NewObject;
		NewObject->pNextObject = pObj;
		NewObject->pParent = this;
	}
	else
	{
		while((FollowingKey[0] == '\0' || pObj->Key[0] == '\0' || strcmp(pObj->Key, FollowingKey) != 0) && pObj->pNextObject)
			pObj = pObj->pNextObject;
		NewObject->pNextObject = pObj->pNextObject;
		NewObject->pParent = this;
		pObj->pNextObject = NewObject;
	}

	return 1;
}

// insert a new key/value paire to current object or array 
// current object must be the type of object or array
// InsertPosition specifies the position to insert:
// 0 means insert as first content
// 1 means insert after the first content etc.
// if InsertPosition is larger than the total number of key/value pairs for object
// or larger than the total size of array, NewObject will be added at the end
int JsonObject::InsertObject(JsonObject* NewObject, int InsertPosition)
{
	JsonObject* pObj;

	if (Type != JsonObject::ValueTypeObject && Type != JsonObject::ValueTypeArray)
		return 0;
	pObj = GetFirstObject();
	if (InsertPosition <= 0 || pObj == NULL)
	{
		pObjectContent = NewObject;
		NewObject->pNextObject = pObj;
		NewObject->pParent = this;
	}
	else
	{
		for (int CurIndex = 1; CurIndex < InsertPosition && pObj->pNextObject; CurIndex ++)
			pObj = pObj->pNextObject;
		NewObject->pNextObject = pObj->pNextObject;
		NewObject->pParent = this;
		pObj->pNextObject = NewObject;
	}

	return 1;
}

JsonStream::JsonStream()
{
	RootObject = NULL;
	stream[0] = '\0';
	p = stream;
	indent = DEFAULT_INDENT;
}

JsonStream::~JsonStream()
{
	DeleteAllTree();
}

void JsonStream::DeleteAllTree()
{
	if (RootObject)
		JsonObject::DeleteTree(RootObject);
	RootObject = NULL;
}

int JsonStream::ReadFile(const char *File)
{
	FILE *fp = fopen(File, "r");

	if (fp == NULL)
		return -1;
	ReadFile(fp);
	fclose(fp);

	return 0;
}

int JsonStream::WriteFile(const char *File)
{
	FILE *fp = fopen(File, "w");

	if (fp == NULL)
		return -1;
	WriteFile(fp);
	fclose(fp);

	return 0;
}

int JsonStream::ReadFile(FILE *fpFile)
{
	if (fpFile == NULL)
		return -1;
	p = stream;
	RootObject = ParseObject(NULL, 0, &JsonStream::GetFileStream, (void *)fpFile);
	stream[0] = '\0';
	return 0;
}

int JsonStream::WriteFile(FILE *fpFile)
{
	if (fpFile == NULL)
		return -1;
	if (RootObject != NULL)
	{
		OutputObject(fpFile, RootObject->pObjectContent, 1, 1);
	}
	return 0;
}

JsonObject *JsonStream::ParseObject(JsonObject *Parent, int IsObject, GetStream GetStreamFunc, void *source)
{
	JsonObject *Object = NULL, *CurObject = NULL;
	int stage = IsObject ? 0 : 3;

	if (!IsObject)	// for array, create new object list
	{
		CurObject = Object = new JsonObject(Parent);
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
				CurObject = Object = new JsonObject(Parent);
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
				CurObject->pObjectContent = ParseObject(CurObject, 1, GetStreamFunc, source);
			}
			else if (*p == '[')	// value is an array
			{
				p ++;
				CurObject->Type = JsonObject::ValueTypeArray;
				CurObject->pObjectContent = ParseObject(CurObject, 0, GetStreamFunc, source);
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
				CurObject->pNextObject = new JsonObject(Parent);
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

JsonObject *JsonStream::GetRootObject(bool CreateOnEmpty)
{
	if (RootObject == NULL && CreateOnEmpty)
	{
		if ((RootObject = new JsonObject(NULL)) != NULL)
			RootObject->Type = JsonObject::ValueTypeObject;
	}
	return RootObject;
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
	OutputIndent(fp, Depth - 1);
	fputc(HasKey ? '}' : ']', fp);
	return 0;
}

int JsonStream::OutputKeyValue(FILE *fp, JsonObject *Object, int Depth, int HasKey)
{
	if (Object == NULL)
		return -1;
	OutputIndent(fp, Depth);
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

void JsonStream::OutputIndent(FILE *fp, int Length)
{
	int i;

	if (indent >= 0)
	{
		for (i = 0; i < Length * indent; i ++)
		fputc(' ', fp);
	}
	else
	{
		for (i = 0; i < Length; i ++)
		fputc('\t', fp);
	}
}
