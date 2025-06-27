//----------------------------------------------------------------------
// XmlElement.cpp:
//   Definition of XML file operation class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "XmlElement.h"

// constructor of new dictionary object
CSimpleDict::CSimpleDict()
{
	DictItemNumber = 0;
	Dictionary = (DictItem *)malloc(sizeof(DictItem) * DICT_ITEM_INC);	// pre-allocate DICT_ITEM_INC dictionary items
	DictArraySize = DICT_ITEM_INC;
}

CSimpleDict::~CSimpleDict()
{
	free(Dictionary);
}

// copy constructor
CSimpleDict& CSimpleDict::operator=(const CSimpleDict& dict)	
{
	// if dictionary size does not match, reallocate memory
	if (DictArraySize != dict.DictArraySize)
	{
		DictArraySize = dict.DictArraySize;
		Dictionary = (DictItem *)realloc(Dictionary, sizeof(DictItem) * DictArraySize);
	}
	// copy content
	DictItemNumber = dict.DictItemNumber;
	memcpy(Dictionary, dict.Dictionary, sizeof(DictItem) * DictItemNumber);
	return *this;
}

// add a new key/value couple as dictionary item
bool CSimpleDict::Add(char *key, char *value)
{
	DictItem *NewDict;

	if (strlen(key) > MAX_KEY_LENGTH || strlen(value) > MAX_VALUE_LENGTH)
		return false;
	// if need more space, reallocate memory to larger size
	if (DictItemNumber == DictArraySize)
	{
		DictArraySize += DICT_ITEM_INC;
		NewDict = (DictItem *)realloc(Dictionary, sizeof(DictItem) * DictArraySize);
		if (NewDict == NULL)
			return false;
		Dictionary = NewDict;
	}
	// copy new item and increase number
	strcpy(Dictionary[DictItemNumber].key, key);
	strcpy(Dictionary[DictItemNumber].value, value);
	DictItemNumber ++;

	return true;
}

// delete one item with given key
// return false if key not found
bool CSimpleDict::Delete(char *key)
{
	int index;

	index = Find(key);

	if (index < 0)
		return false;

	// if deleted item is not the last one, move following items to overwrite deleted one
	if (index != (DictItemNumber - 1))
		memmove(Dictionary + index, Dictionary + index + 1, sizeof(DictItem) * (DictItemNumber - index - 1));
	DictItemNumber --;
	return true;
}

// find item with given key, return item index or -1 if not found
int CSimpleDict::Find(const char *key)
{
	int i;

	for (i = 0; i < DictItemNumber; i ++)
		if (strcmp(Dictionary[i].key, key) == 0)
			break;

	return (i == DictItemNumber) ? -1 : i;
}

// construct a new XML element with empty content
CXmlElement::CXmlElement()
{
	tag[0] = 0;
	text[0] = 0;
	iChildNumber = 0;
	ChildElements = SiblingElements = NULL;
	Completed = true;
}

// constructor with a given element
CXmlElement::CXmlElement(CXmlElement &element)
{
	// copy tag/text and duplicate attributes
	strcpy(this->tag, element.tag);
	strcpy(this->text, element.text);
	this->Attributes = element.Attributes;
	// this is a leaf element
	iChildNumber = 0;
	ChildElements = SiblingElements = NULL;
	Completed = true;
}

// constructor with given tag/text
CXmlElement::CXmlElement(char *tag, char *text)
{
	// copy tag/text
	strcpy(this->tag, tag);
	strcpy(this->text, text);
	// this is a leaf element
	iChildNumber = 0;
	ChildElements = SiblingElements = NULL;
	Completed = true;
}

CXmlElement::~CXmlElement()
{
	// delete all sub-elements before release current element
	DeleteTree();
}

// delete all sub-elements
bool CXmlElement::DeleteTree()
{
	// if it has child element, delete sub-elements of child element and then delete child element itself
	if (ChildElements)
	{
		ChildElements->DeleteTree();
		delete ChildElements;
		ChildElements = NULL;
	}
	// if it has sibling element, delete sub-elements of sibling element and then delete sibling element itself
	if (SiblingElements)
	{
		SiblingElements->DeleteTree();
		delete SiblingElements;
		SiblingElements = NULL;
	}

	return true;
}

// remove one sub-element at a given index
bool CXmlElement::RemoveElement(int index)
{
	CXmlElement *PrevElement = NULL, *CurElement = NULL;
	
	if (index < 0)
		return false;
	// if index is 0, sub-element to be deleted is first child element
	if (index == 0)
	{
		PrevElement = NULL;
		CurElement = ChildElements;
	}
	else
	{
		PrevElement = GetElement(index-1);
		CurElement = PrevElement->SiblingElements;
	}
	// now CurElement point to the element to be removed
	// PrevElement point to the element in the link list prior to CurElement

	if (!CurElement)
		return false;

	// remove current element from link list
	if (PrevElement)
		PrevElement->SiblingElements = CurElement->SiblingElements;
	else
		ChildElements = CurElement->SiblingElements;

	// delete current element
	CurElement->DeleteTree();
	delete CurElement;

	return true;
}

// return pointer to sub-element with given index
CXmlElement *CXmlElement::GetElement(int index)
{
	CXmlElement *Element = ChildElements;

	// pointer to first child element, and then trace link list
	while (Element && index > 0)
	{
		Element = Element->SiblingElements;
		index --;
	}

	return Element;
}

CXmlElement *CXmlElement::EnumSubElement(CXmlElement *PrevSubElement)
{
	if (PrevSubElement)
		return PrevSubElement->SiblingElements;
	else
		return ChildElements;
}

// recursive function call to pass a string and add element to tree
bool CXmlElement::parse_string(char *string, bool with_tag)
{
	char *str = string;
	char *p;
	CXmlElement *SubElement;

	// trim empty characters
	while (*str && (*str == ' ' || *str == '\t'))
		str ++;
	// for string passed in with tag of current element, first extract tag
	if (with_tag)
		str = process_tag(str);
	if (*str == 0)
		return Completed;
	if (!Completed)	// find text
	{
		// trim empty characters
		while (*str && (*str == ' ' || *str == '\t'))
			str ++;
		// whether followed with '<'
		if ((p = strchr(str, '<')) != NULL)
			*p = 0;
		if (*str && (p ? (p > str) : true))
			strcpy(text, str);
		// put str to end of text
		if (p)
		{
			*p = '<';	// restore '<'
			str = p;
		}
		else
			*str = 0;
	}
	if (*str != '<')	// if text not followed with '<', drop remnant of the line
		return Completed;

	if (Completed)	// for completed element, drop any following character
		return true;
	p = strchr(str, '>');
	if (!p)	// drop remnant of the line for '<' without '>'
		return Completed;
		
	// whether this is end of element
	if (str[1] == '/')
	{
		*p = 0;
		if (strcmp(tag, str + 2) == 0)
		{
			Completed = true;
			return true;
		}
		// not end of current element, restore '>'
		*p = '>';
		// pass to active sub-element
		SubElement = GetElement(iChildNumber - 1);
		if (SubElement && !(SubElement->Completed))
			SubElement->parse_string(str, false);
	}
	else	// this is a new tag
	{
		// create a new sub-element
		if (iChildNumber > 0)
		{
			SubElement = GetElement(iChildNumber - 1);
			if (SubElement->Completed)
			{
				SubElement->SiblingElements = new CXmlElement;
				SubElement = SubElement->SiblingElements;
				iChildNumber ++;
			}
		}
		else
		{
			ChildElements = new CXmlElement;
			SubElement = ChildElements;
			iChildNumber ++;
		}
		SubElement->parse_string(str, SubElement->Completed);
	}

	return Completed;
}

// extract tag name from the string
char *CXmlElement::process_tag(char *string)
{
	char *left, *right;
	char *p;

	left = strchr(string, '<');
	right = strchr(string, '>');

	if (left[1] == '/')		// for </tag>, this is not start of new tag
		return NULL;
	p = left + 1;
	while (*p != ' ' && *p != '\t' && *p != '>')	// tag name ended with '>' or space between attributes
		p ++;
	*p = 0;
	strcpy(tag, left + 1);
	p ++;
	// loop to process attributes
	while (*p && *p != '>')
	{
		// trim empty characters
		while (*p && (*p == ' ' || *p == '\t'))
			p ++;
		if ((left = strchr(p, '=')) == NULL)	// do not find '='
			break;
		*left = 0;
		left ++;
		// find contents with in ""
		left = strchr(left, '"');
		if (!left)
			break;
		left ++;
		right = strchr(left, '"');
		if (!right)
			break;
		*right = 0;
		Attributes.Add(p, left);
		p = right + 1;
	}
	if (*p == '/' && p[1] == '>')	// format of <tag attribut list/> as element without text, current element completed
	{
		p ++;
		Completed = true;
	}
	else
		Completed = false;
	return (*p == '>') ? p + 1 : p;	// return character following '>' (text or NULL)
}

// output element to file as tree format
bool CXmlElement::output(FILE *fp, int indent)
{
	CXmlElement *Elements;
	int i;

	// no text and no sub-element
	if (strlen(text) == 0 && iChildNumber == 0)
	{
		for (i = 0; i < indent; i ++)
			fprintf(fp, "    ");
		fprintf(fp, "<%s", tag);
		for (i = 0; i < Attributes.DictItemNumber; i ++)
			fprintf(fp, " %s=\"%s\"", Attributes.Dictionary[i].key, Attributes.Dictionary[i].value);
		fprintf(fp, "/>\n");
		return true;
	}
	// no sub-element
	else if (iChildNumber == 0)
	{
		for (i = 0; i < indent; i ++)
			fprintf(fp, "    ");
		fprintf(fp, "<%s", tag);
		for (i = 0; i < Attributes.DictItemNumber; i ++)
			fprintf(fp, " %s=\"%s\"", Attributes.Dictionary[i].key, Attributes.Dictionary[i].value);
		fprintf(fp, ">");
		fprintf(fp, "%s", text);
		fprintf(fp, "</%s>\n", tag);
		return true;
	}
	else
	{
		for (i = 0; i < indent; i ++)
			fprintf(fp, "    ");
		fprintf(fp, "<%s", tag);
		for (i = 0; i < Attributes.DictItemNumber; i ++)
			fprintf(fp, " %s=\"%s\"", Attributes.Dictionary[i].key, Attributes.Dictionary[i].value);
		fprintf(fp, ">\n");
		if (strlen(text) > 0)
			fprintf(fp, "%s\n", text);
		Elements = ChildElements;
		while (Elements)
		{
			Elements->output(fp, indent + 1);
			Elements = Elements->SiblingElements;
		}
		for (i = 0; i < indent; i ++)
			fprintf(fp, "    ");
		fprintf(fp, "</%s>\n", tag);
		return true;
	}
}

// append one new XML element as sub-element at end of link list, copy element content, do not add given element's sub-element
bool CXmlElement::AppendElement(CXmlElement element)
{
	CXmlElement *NewElement;

	// if there is no child, the appended element is first child element
	if (iChildNumber == 0)
		NewElement = ChildElements = new CXmlElement(element);
	else	// trace to the end of link list
	{
		NewElement = ChildElements;
		while (NewElement->SiblingElements)
			NewElement = NewElement->SiblingElements;
		NewElement->SiblingElements = new CXmlElement(element);
		NewElement = NewElement->SiblingElements;
	}
	iChildNumber ++;
	NewElement->ChildElements = NewElement->SiblingElements = NULL;
	NewElement->iChildNumber = 0;

	return true;
}

// append one new XML element as sub-element with given tag and text at end of link list
bool CXmlElement::AppendElement(char *tag, char *text)
{
	CXmlElement NewElement(tag, text);
	return AppendElement(NewElement);
}

// insert one new XML element at a given position
bool CXmlElement::InsertElement(int index, CXmlElement element)
{
	CXmlElement *CurElement = NULL, *NewElement;

	// find the sub-element pointing to inserted position
	if (index > 0)
		CurElement = GetElement(index-1);
	else
		CurElement = ChildElements;
	if (!CurElement && iChildNumber > 0)
		return false;
	NewElement = new CXmlElement(element);
	if (iChildNumber == 0)
		ChildElements = NewElement;
	else if (index == 0)
	{
		ChildElements = NewElement;
		NewElement->SiblingElements = CurElement;
	}
	else
	{
		NewElement->SiblingElements = CurElement->SiblingElements;
		CurElement->SiblingElements = NewElement;
	}
	iChildNumber ++;

	return true;
}

// set tag of element
bool CXmlElement::SetTag(char *tag)
{
	if (strlen(tag) > MAX_TAG_LENGTH)
		return false;
	strcpy(this->tag, tag);
	return true;
}

// set text of element
bool CXmlElement::SetText(char *text)
{
	if (strlen(text) > MAX_TEXT_LENGTH)
		return false;
	strcpy(this->text, text);
	return true;
}

// initialize an empty XML tree
CXmlElementTree::CXmlElementTree()
{
	root = NULL;
	header[0] = 0;
	comment[0] = 0;
}

// initialize an XML tree with a given root element
CXmlElementTree::CXmlElementTree(CXmlElement element)
{
	root = new CXmlElement;
	*root = element;
	header[0] = 0;
	comment[0] = 0;
}

CXmlElementTree::~CXmlElementTree()
{
	ClearTree();
}

// re-initialize an XML tree with a given root element
void CXmlElementTree::SetRootElement(CXmlElement element)
{
	ClearTree();
	root = new CXmlElement;
	*root = element;
}

// clear the whole XML tree
void CXmlElementTree::ClearTree()
{
	if (root)
	{
		root->DeleteTree();
		delete root;
		root = NULL;
	}
}

// read a file content into XML tree
bool CXmlElementTree::parse(const char *filename)
{
	FILE *fp;
	char string[512], *content;

	if ((fp = fopen(filename, "r")) == NULL)
		return false;
	ClearTree();

	// read each line and parse the line
	while (!feof(fp))
	{
		fgets(string, 511, fp);
		content = string + strlen(string) - 1;
		// trim LF/CR
		while (*content == '\r' || *content == '\n')
			*content -- = 0;
		// if this is header line, copy header
		if (((content = found_header(string)) != NULL) && strlen(content) <= MAX_HEADER_LENGTH)
			strcpy(header, content);
		// if this is comment line, copy comment
		else if (((content = found_comment(string)) != NULL) && strlen(content) <= MAX_COMMENT_LENGTH)
			strcpy(comment, content);
		// call parse_string for root element
		else if (root)
			root->parse_string(string, false);
		else if (found_tag(string))
		{
			root = new CXmlElement;
			root->parse_string(string, true);
		}
	}

	return true;
}

bool CXmlElementTree::write(const char *filename)
{
	FILE *fp = fopen(filename, "w");
	if (!fp)
		return false;
	if (strlen(header) > 0)
		fprintf(fp, "<?xml %s?>\n", header);
	if (strlen(comment) > 0)
		fprintf(fp, "<!-- %s-->\n", comment);
	if (root)
		root->output(fp, 0);
	fclose(fp);
	return true;
}

char *CXmlElementTree::found_header(char *string)
{
	char *left, *right;

	left = strchr(string, '<');
	right = strchr(string, '>');
	if (!left || left[1] != '?')
		return NULL;
	if (right < string || right[-1] != '?')
		return NULL;
	if (left[2] != 'x' && left[2] != 'X')
		return NULL;
	if (left[3] != 'm' && left[3] != 'M')
		return NULL;
	if (left[4] != 'l' && left[4] != 'L')
		return NULL;
	// trim off end
	right[-1] = 0;

	left += 5;
	// trim empty characters
	while (*left && (*left == ' ' || *left == '\t'))
		left ++;

	return left;
}

char *CXmlElementTree::found_comment(char *string)
{
	char *left, *right;

	left = strchr(string, '<');
	right = strchr(string, '>');
	if (!left || left[1] != '!')
		return NULL;
	if (left[2] != '-' || left[3] != '-')
		return NULL;
	if (right < string || right[-1] != '-' || right[-2] != '-')
		return NULL;
	// trim off end
	right[-2] = 0;

	left += 4;
	// trim empty characters
	while (*left && (*left == ' ' || *left == '\t'))
		left ++;

	return left;
}

bool CXmlElementTree::found_tag(char *string)
{
	char *left, *right;

	left = strchr(string, '<');
	right = strchr(string, '>');

	return (left && right && right > left);
}
