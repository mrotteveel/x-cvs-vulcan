// SymbolManager.cpp: implementation of the SymbolManager class.
//
//////////////////////////////////////////////////////////////////////

#include <memory.h>
#include "fbdev.h"
#include "common.h"
#include "SymbolManager.h"
#include "Sync.h"

#define HASH_MULTIPLIER		11

static char caseTable [256];
static int init();
static int foo = init();

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int init()
{
	for (int n = 0; n < 256; ++n)
		caseTable [n] = n;

	for (char c = 'a'; c <= 'z'; ++c)
		caseTable [c] = c - 'a' + 'A';

	return 0;
}

SymbolManager::SymbolManager()
{
	sections = new SymbolSection;
	sections->next = NULL;
	next = sections->space;
	memset (hashTable, 0, sizeof (hashTable));
}

SymbolManager::~SymbolManager()
{
	for (SymbolSection *section; section = sections;)
		{
		sections = section->next;
		delete section;
		}
}

const char* SymbolManager::findSymbol(const char *string)
{
	//Sync sync (&syncObject, "SymbolManager::findSymbol");
	//sync.lock (Shared);
	int	value = 0, c;
	const char *p;

	for (p = string; c = caseTable [*p++];)
		value = value * HASH_MULTIPLIER + c;

	if (value < 0)
		value = -value;

	for (Symbol *symbol = hashTable [value % SYMBOL_HASH_SIZE]; symbol; symbol = symbol->collision)
		{
		const char *s = symbol->symbol;
		const char *q = string;
		for (; *s && *q && *p == caseTable [*s]; ++s, ++q)
			;
		if (*s == *q)
			return symbol->symbol;
		}

	return NULL;
}

const char* SymbolManager::getSymbol(const char *string)
{
	for (SymbolSection *section = sections; section; section = section->next)
		if (section->space <= string && string < section->space + sizeof (section->space))
			return string;

	Sync sync (&syncObject, "SymbolManager::getSymbol");
	sync.lock (Shared);
	int	value = 0, c;
	const char *p;

	for (p = string; c = caseTable [*p++];)
		value = value * HASH_MULTIPLIER + c;

	if (value < 0)
		value = -value;

	int slot = value % SYMBOL_HASH_SIZE;
	Symbol *symbol;

	for (symbol = hashTable [slot]; symbol; symbol = symbol->collision)
		{
		const char *s = symbol->symbol;
		const char *q = string;;
		for (; *s && *q && *s == caseTable [*q]; ++s, ++q)
			;
		if (*s == *q)
			return symbol->symbol;
		}

	sync.unlock();
	sync.lock (Exclusive);
	symbol = (Symbol*) ((int)(next + 3) & ~3);
	int length = p - string;

	if (symbol->symbol + length > sections->space + sizeof (sections->space))
		{
		SymbolSection *section = new SymbolSection;
		section->next = sections;
		sections = section;
		symbol = (Symbol*) section->space;
		}

	p = string;

	for (char *s = symbol->symbol; *s++ = caseTable [*p++];)
		;

	next = symbol->symbol + length;
	symbol->collision = hashTable [slot];
	hashTable [slot] = symbol;

	return symbol->symbol;			
}


const char* SymbolManager::getString(const char *string)
{
	for (SymbolSection *section = sections; section; section = section->next)
		if (section->space <= string && string < section->space + sizeof (section->space))
			return string;

	Sync sync (&syncObject, "SymbolManager::getSymbol");
	sync.lock (Shared);
	int	value = 0, c;
	const char *p;

	for (p = string; c = *p++;)
		value = value * HASH_MULTIPLIER + c;

	if (value < 0)
		value = -value;

	int slot = value % SYMBOL_HASH_SIZE;
	Symbol *symbol;

	for (symbol = hashTable [slot]; symbol; symbol = symbol->collision)
		{
		const char *s = symbol->symbol;
		const char *q = string;
		for (; *s && *q && *s == *q; ++s, ++q)
			;
		if (*s == *q)
			return symbol->symbol;
		}

	sync.unlock();
	sync.lock (Exclusive);
	symbol = (Symbol*) ((int)(next + 3) & ~3);
	int length = p - string;

	if (symbol->symbol + length > sections->space + sizeof (sections->space))
		{
		SymbolSection *section = new SymbolSection;
		section->next = sections;
		sections = section;
		symbol = (Symbol*) section->space;
		}

	p = string;

	for (char *s = symbol->symbol; *s++ = *p++;)
		;

	next = symbol->symbol + length;
	symbol->collision = hashTable [slot];
	hashTable [slot] = symbol;

	return symbol->symbol;			
}

bool SymbolManager::isSymbol(const char *string)
{
	for (SymbolSection *section = sections; section; section = section->next)
		if (section->space <= string && string < section->space + sizeof (section->space))
			return true;

	return false;
}

/*** No unicode in Firebird
const char* SymbolManager::getSymbol(const WCString *string)
{
	Sync sync (&syncObject, "SymbolManager::getSymbol(WC)");
	sync.lock (Shared);
	int	value = 0, c;
	const unsigned short *p;
	const unsigned short *end = string->string + string->count;

	for (p = string->string; p < end;)
		{
		c = caseTable [(*p++) & 0xff];
		value = value * HASH_MULTIPLIER + c;
		}

	if (value < 0)
		value = -value;

	int slot = value % SYMBOL_HASH_SIZE;
	Symbol *symbol;

	for (symbol = hashTable [slot]; symbol; symbol = symbol->collision)
		{
		const char *s = symbol->symbol;
		const unsigned short *q = string->string;
		for (; *s && q < end && *s == caseTable [(*q) & 0xff]; ++s, ++q)
			;
		if (*s == 0 && q == end)
			return symbol->symbol;
		}

	sync.unlock();
	sync.lock (Exclusive);
	symbol = (Symbol*) ((int)(next + 3) & ~3);
	int length = string->count + 1;

	if (symbol->symbol + length > sections->space + sizeof (sections->space))
		{
		SymbolSection *section = new SymbolSection;
		section->next = sections;
		sections = section;
		symbol = (Symbol*) section->space;
		}

	p = string->string;
	char *s = symbol->symbol;

	while (p < end)
		*s++ = caseTable [(*p++) & 0xff];

	*s = 0;
	next = symbol->symbol + length;
	symbol->collision = hashTable [slot];
	hashTable [slot] = symbol;

	return symbol->symbol;			
}
***/

const char* SymbolManager::findString(const char *string)
{
	//Sync sync (&syncObject, "SymbolManager::findString");
	//sync.lock (Shared);
	int	value = 0, c;
	const char *p;

	for (p = string; c = *p++;)
		value = value * HASH_MULTIPLIER + c;

	if (value < 0)
		value = -value;

	for (Symbol *symbol = hashTable [value % SYMBOL_HASH_SIZE]; symbol; symbol = symbol->collision)
		{
		const char *s = symbol->symbol;
		const char *q = string;
		for (; *s && *q && *p == *s; ++s, ++q)
			;
		if (*s == *q)
			return symbol->symbol;
		}

	return NULL;
}
