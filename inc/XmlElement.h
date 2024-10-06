//----------------------------------------------------------------------
// XmlElement.h:
//   Declaration of XML file operation class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined __XML_ELEMENT_H__
#define __XML_ELEMENT_H__

#include <stdio.h>

#define DICT_ITEM_INC 8
#define MAX_KEY_LENGTH (32-1)
#define MAX_VALUE_LENGTH (96-1)
#define MAX_TAG_LENGTH (64-1)
#define MAX_TEXT_LENGTH (192-1)
#define MAX_HEADER_LENGTH (256-1)
#define MAX_COMMENT_LENGTH (256-1)

struct DictItem
{
	char key[MAX_KEY_LENGTH+1];
	char value[MAX_VALUE_LENGTH+1];
};

// dictionary class, array of key/value couple without order
class CSimpleDict
{
public:
	CSimpleDict();
	~CSimpleDict();
	CSimpleDict& operator=(const CSimpleDict& dict);
	bool Add(char *key, char *value);
	bool Delete(char *key);
	int Find(const char *key);

	int DictItemNumber;
	DictItem *Dictionary;
private:
	int DictArraySize;
};

// XML element class
class CXmlElement
{
public:
	CXmlElement();
	CXmlElement(CXmlElement &element);
	CXmlElement(char *tag, char *text);
	~CXmlElement();
	bool DeleteTree();
	bool RemoveElement(int index);
	CXmlElement *GetElement(int index);
	CXmlElement *EnumSubElement(CXmlElement *PrevSubElement);
	bool parse_string(char *string, bool with_tag);
	char *process_tag(char *string);
	bool output(FILE *fp, int indent);
	bool AppendElement(CXmlElement element);
	bool AppendElement(char *tag, char *text);
	bool InsertElement(int index, CXmlElement element);
	bool SetTag(char *tag);
	bool SetText(char *text);
	char *GetTag() { return tag; }
	char *GetText() { return text; }
	CSimpleDict *GetAttributes() { return &Attributes; }

private:
	char tag[MAX_TAG_LENGTH+1];
	char text[MAX_TEXT_LENGTH+1];
	int iChildNumber;
	CSimpleDict Attributes;
	CXmlElement *ChildElements;
	CXmlElement *SiblingElements;
	bool Completed;
};

// XML tree class
class CXmlElementTree
{
public:
	char header[MAX_HEADER_LENGTH+1];
	char comment[MAX_COMMENT_LENGTH+1];

	CXmlElementTree();
	CXmlElementTree(CXmlElement element);
	~CXmlElementTree();
	void SetRootElement(CXmlElement element);
	void ClearTree();
	bool parse(const char *filename);
	bool write(const char *filename);
	CXmlElement *getroot() { return root; }

private:
	CXmlElement *root;
	char *found_header(char *string);
	char *found_comment(char *string);
	bool found_tag(char *string);
};

#endif //__XML_ELEMENT_H__