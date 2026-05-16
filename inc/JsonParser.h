//----------------------------------------------------------------------
// JsonParser.h:
//   JSON stream process class declaration
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include <stdio.h>

#define MAX_LINE_LENGTH 256
#define MAX_KEY_LENGTH (32-1)
#define MAX_STRING_LENGTH (192-1)
#define DEFAULT_INDENT 2

#define OBJ_ARRAY_INC_SIZE 128

typedef union
{
	double d_data;
	long long int l_data;
} JSON_NUMBER_UNION;

//----------------------------------------------------------------------
// JsonObject is basic unit holding a key/value combination or value only
// Depending on the type of value, Type can be one of enum ValueType
// The key string is stored in Key and is empty string for value only
// For different value type the value is stored as following
// ValueTypeObject: the value is a serial of JsonObject putting as
//    a link list starting at pObjectContent and concatenated with pNextObject
// ValueTypeArray: the value is a serial of value only JsonObject putting as
//    a link list starting at pObjectContent and concatenated with pNextObject
// ValueTypeString: the value is put in String
// ValueTypeIntNumber: the long long int type value is put in Number.l_data
// ValueTypeFloatNumber: the double type value is put in Number.d_data
// ValueTypeTrue: the value is true
// ValueTypeFalse: the value is false
// ValueTypeNull: the value is null
//----------------------------------------------------------------------

class JsonObject
{
// constructor / destructor
public:
	JsonObject();
	JsonObject(JsonObject *Parent);
//	JsonObject(const JsonObject& Object);
	~JsonObject();

// attributes
public:
	// define type of value
	enum ValueType { ValueTypeNull, ValueTypeObject, ValueTypeArray, ValueTypeString, ValueTypeIntNumber, ValueTypeFloatNumber, ValueTypeTrue, ValueTypeFalse };

	char Key[MAX_KEY_LENGTH+1];
	ValueType Type;
	JsonObject *pNextObject;	// pointer to next key/value pair with same parent object or in same array
	JsonObject *pObjectContent;	// pointer to content if value type is object or array
	JsonObject *pParent;		// pointer to parent object
	JSON_NUMBER_UNION Number;
	char String[MAX_STRING_LENGTH+1];

// implementations
	JsonObject *GetFirstObject() { return pObjectContent; }
	JsonObject *GetNextObject() { return pNextObject; }
	JsonObject *GetParent() { return pParent; }
	JsonObject *SearchSubitems(const char *KeyName, int SearchSubitem = 1, const char *StrValue = (const char *)NULL);
	int GetIndex();
	int GetSize();
	JsonObject *operator[](int Index);
	int ReplaceValue(const char *KeyName, void *p, int SearchSubitem = 1);
	int ReplaceValue(const char *KeyName, const char *Value, int SearchSubitem = 1);
	int ReplaceValue(const char *KeyName, int Value, int SearchSubitem = 1);
	int ReplaceValue(const char *KeyName, double Value, int SearchSubitem = 1);
	int ReplaceValue(const char *KeyName, bool Value, int SearchSubitem = 1);
	int AddObject(JsonObject* NewObject, const char* FollowingKey = (const char*)NULL);
	int InsertObject(JsonObject* NewObject, int InsertPosition);

	static void DeleteTree(JsonObject *Object, int DeleteLink = 0);
	static JsonObject *ReplaceObject(JsonObject *OldObject, JsonObject *NewObject);
	static void RemoveObject(JsonObject *Object);
};

#define GET_DOUBLE_VALUE(Object) ((Object->Type == JsonObject::ValueTypeIntNumber) ? (double)(Object->Number.l_data) : Object->Number.d_data)

class JsonStream;
typedef int(JsonStream::*GetStream)(void *source);

//----------------------------------------------------------------------
// JsonStream is a class to process JSON stream including read/write JSON file
// or from other sources, traverse JSON stream as a tree etc.
// After a complete JSON file/stream is loaded, the most top object is stored
// as a value only JsonObject at RootObject
// The instances of JsonObject withinthe tree will be allocated an increse automatically
// To traverse an object, call CurObject = GetFirstObject(Object) to get the
// first key/value combination and call CurObject = GetFirstObject(CurObject)
// to get following key/value combinations until get NULL pointer
//----------------------------------------------------------------------
class JsonStream
{
public:
	JsonStream();
	~JsonStream();

	void DeleteAllTree();
	// read/write with file name
	int ReadFile(const char *File);
	int WriteFile(const char *File);
	// read/write with existing FILE pointer
	int ReadFile(FILE *fpFile);
	int WriteFile(FILE *fpFile);
	JsonObject *ParseObject(JsonObject *Parent, int IsObject, GetStream GetStreamFunc, void *source);
	JsonObject *GetRootObject() { return RootObject; }
	int SetIndent(int Indent) { indent = Indent; }

private:
	JsonObject *RootObject;
	char stream[MAX_LINE_LENGTH];	// read file buffer
	const char *p;	// pointer of current processing character in stream
	int indent;

	int GetFileStream(void *source);
	int CopyString(char *dest, int MaxLength);
	int IsWhiteSpace(const char ch);
	char EscapeCharacter();
	int GetValueContent(JsonObject *Object);
	int GetNumber(JsonObject *Object);

	int OutputObject(FILE *fp, JsonObject *Object, int Depth, int HasKey);
	int OutputKeyValue(FILE *fp, JsonObject *Object, int Depth, int HasKey);
	int OutputString(FILE *fp, const char *str);
	void OutputIndent(FILE *fp, int Length);
};


#endif //__JSON_PARSER_H__
