/*
 *	PROGRAM:	Dynamic SQL runtime support
 *	MODULE:		hsh.cpp
 *	DESCRIPTION:	Hash table and symbol manager
 *
 * The contents of this file are subject to the Interbase Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy
 * of the License at http://www.Inprise.com/IPL.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code was created by Inprise Corporation
 * and its predecessors. Portions created by Inprise Corporation are
 * Copyright (C) Inprise Corporation.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 */

#include "fbdev.h"
#include <string.h>
#include "../dsql/dsql.h"
//#include "../jrd/y_ref.h"
#include "../jrd/ibase.h"
#include "../jrd/gds_proto.h"
#include "../dsql/alld_proto.h"
#include "../dsql/errd_proto.h"
#include "../dsql/hsh_proto.h"
#include "../jrd/sch_proto.h"
#include "../jrd/thd_proto.h"
#include "Procedure.h"
#include "SyncObject.h"
#include "Sync.h"


const int HASH_SIZE = 211;
static SSHORT hash(const SCHAR*, USHORT);
static bool remove_symbol(dsql_sym**, dsql_sym*);
static bool scompare(const TEXT*, USHORT, const TEXT*, const USHORT);

static DSQL_SYM* hash_table;

/*
   SUPERSERVER can end up with many hands in the pie, so some
   protection is provided via a mutex.   This ensures the integrity
   of lookups, inserts and removals, and also allows teh traversing
   of symbol chains for marking of relations and procedures.
   Otherwise, one DSQL user won't know what the other is doing
   to the same object.
*/

static SyncObject	syncObject;

#ifdef OBSOLETE
#ifdef  SUPERSERVER
static MUTX_T hash_mutex;
static USHORT hash_mutex_inited = 0;
static inline void lock_hash()
{
	THD_mutex_lock (&hash_mutex);
}

static inline void unlock_hash()
{
	THD_mutex_unlock (&hash_mutex);
}
#else
static inline void lock_hash()
{
}

static inline void unlock_hash()
{
}
#endif	// SUPERSERVER
#endif	// OBSOLETE

/**
  
 	HSHD_init
  
    @brief	create a new hash table
 


 **/
void HSHD_init(void)
{

	UCHAR* p = (UCHAR *) gds__alloc(sizeof(DSQL_SYM) * HASH_SIZE);
	memset(p, 0, sizeof(DSQL_SYM) * HASH_SIZE);

	hash_table = (DSQL_SYM *) p;
}


#ifdef DEV_BUILD

#include "../jrd/ib_stdio.h"

/**
  
 	HSHD_debug
  
    @brief	Print out the hash table for debugging.
 


 **/
void HSHD_debug(void)
{
	// dump each hash table entry 
	Sync sync(&syncObject, "HSHD_debug");
	sync.lock(Exclusive);

	for (SSHORT h = 0; h < HASH_SIZE; h++) 
		{
		for (DSQL_SYM collision = hash_table[h]; collision; collision = collision->sym_collision)
			{
			// check any homonyms first 

			ib_fprintf(ib_stderr, "Symbol type %d: %s %p\n",
					   collision->sym_type, collision->sym_string,
					   collision->sym_dbb);
			for (DSQL_SYM homptr = collision->sym_homonym; homptr;
				 homptr = homptr->sym_homonym)
				ib_fprintf(ib_stderr, "Homonym Symbol type %d: %s %p\n",
						   homptr->sym_type, homptr->sym_string,
						   homptr->sym_dbb);
			}
		}

	//unlock_hash();
}
#endif


/**
  
 	HSHD_fini
  
    @brief	Clear out the symbol table.  All the 
 	symbols are deallocated with their pools.
 


 **/
void HSHD_fini(void)
{
	for (SSHORT i = 0; i < HASH_SIZE; i++)
	{
		hash_table[i] = NULL;
	}

	gds__free(hash_table);
	hash_table = NULL;
}


/**
  
 	HSHD_finish
  
    @brief	Remove symbols used by a particular database.
 	Don't bother to release them since their pools
 	will be released.
 

    @param database

 **/
void HSHD_finish( const void* database)
{
// check each hash table entry 

	Sync sync(&syncObject, "HSHD_debug");
	sync.lock(Exclusive);

	for (SSHORT h = 0; h < HASH_SIZE; h++) {
		for (DSQL_SYM* collision = &hash_table[h]; *collision;) {
			// check any homonyms first 

			DSQL_SYM chain = *collision;
			for (DSQL_SYM* homptr = &chain->sym_homonym; *homptr;) {
				DSQL_SYM symbol = *homptr;
				if (symbol->sym_dbb == database) {
					*homptr = symbol->sym_homonym;
					symbol = symbol->sym_homonym;
				}
				else
					homptr = &symbol->sym_homonym;
			}

			// now, see if the root entry has to go 

			if (chain->sym_dbb == database) {
				if (chain->sym_homonym) {
					chain->sym_homonym->sym_collision = chain->sym_collision;
					*collision = chain->sym_homonym;
				}
				else
					*collision = chain->sym_collision;
				chain = *collision;
			}
			else
				collision = &chain->sym_collision;
		}
	}
}


/**
  
 	HSHD_insert
  
    @brief	Insert a symbol into the hash table.
 

    @param symbol

 **/
void HSHD_insert(DSQL_SYM symbol)
{
	Sync sync(&syncObject, "HSHD_insert");
	sync.lock(Exclusive);

	const SSHORT h = hash(symbol->sym_string, symbol->sym_length);
	const void* database = symbol->sym_dbb;

	fb_assert(symbol->sym_type >= SYM_statement && symbol->sym_type <= SYM_eof);

	for (DSQL_SYM old = hash_table[h]; old; old = old->sym_collision)
		if ((!database || (database == old->sym_dbb)) &&
			scompare(symbol->sym_string, symbol->sym_length, old->sym_string,
					 old->sym_length)) 
			{
			symbol->sym_homonym = old->sym_homonym;
			old->sym_homonym = symbol;
			return;
			}

	symbol->sym_collision = hash_table[h];
	hash_table[h] = symbol;
}


/**
  
 	HSHD_lookup
  
    @brief	Perform a string lookup against hash table.
 	Make sure to only return a symbol of the desired type.
 

    @param database
    @param string
    @param length
    @param type
    @param parser_version

 **/
DSQL_SYM HSHD_lookup(const void*    database,
				const TEXT*    string,
				SSHORT   length,
				SYM_TYPE type,
				USHORT   parser_version)
{

	Sync sync(&syncObject, "HSHD_lookup");
	sync.lock(Shared);
	const SSHORT h = hash(string, length);

	for (DSQL_SYM symbol = hash_table[h]; symbol; symbol = symbol->sym_collision)
		if ((database == symbol->sym_dbb) &&
			scompare(string, length, symbol->sym_string, symbol->sym_length))
			{
			// Search for a symbol of the proper type 

			while (symbol && symbol->sym_type != type) 
				symbol = symbol->sym_homonym;


			/* If the symbol found was not part of the list of keywords for the
			 * client connecting, then assume nothing was found
			 */

			if (symbol)
				if (parser_version < symbol->sym_version && type == SYM_keyword)
					return NULL;

			return symbol;
			}


	return NULL;
}


/**
  
 	HSHD_remove
  
    @brief	Remove a symbol from the hash table.
 

    @param symbol

 **/
void HSHD_remove(DSQL_SYM symbol)
{
	Sync sync(&syncObject, "HSHD_remove");
	sync.lock(Exclusive);
	const SSHORT h = hash(symbol->sym_string, symbol->sym_length);

	for (DSQL_SYM* collision = &hash_table[h]; *collision; collision = &(*collision)->sym_collision)
		if (remove_symbol(collision, symbol)) 
			return;

	ERRD_error(-1, "HSHD_remove failed");
}


/**
  
 HSHD_set_flag
  
    @brief      Set a flag in all similar objects in a chain.   This
       is used primarily to mark relations and procedures
       as deleted.   The object must have the same name and
       type, but not the same database, and must belong to
       some database.   Later access to such an object by
       another user or thread should result in that object's
       being refreshed.   Note that even if the relation name
       and ID, or the procedure name and ID both match, it
       may still not represent an exact match.   This is because
       there's no way at present for DSQL to tell if two databases
       as represented in DSQL are attachments to the same physical
       database.
 

    @param database
    @param string
    @param length
    @param type
    @param flag

 **/
void HSHD_set_flag(const void* database,
				   const TEXT* string, SSHORT length, SYM_TYPE type, SSHORT flag)
{
	/* as of now, there's no work to do if there is no database or if
	   the type is not a relation or procedure */

	if (!database)
		return;
		
	switch (type) 
		{
		case SYM_relation:
		case SYM_procedure:
			break;
			
		default:
			return;
		}

	Sync sync(&syncObject, "HSHD_set_flag");
	sync.lock(Exclusive);
	const SSHORT h = hash(string, length);
	
	for (DSQL_SYM symbol = hash_table[h]; symbol; symbol = symbol->sym_collision)
		if (symbol->sym_dbb && (database != symbol->sym_dbb) &&
			scompare(string, length, symbol->sym_string, symbol->sym_length)) 
			// the symbol name matches and it's from a different database 
			for (DSQL_SYM homonym = symbol; homonym; homonym = homonym->sym_homonym)
				if (homonym->sym_type == type) 
					/* the next check is for the same relation or procedure ID,
					   which indicates that it MAY be the same relation or
					   procedure */

					switch (type) 
						{
						case SYM_relation:
							{
							dsql_rel* sym_rel = (dsql_rel*) homonym->sym_object;
							sym_rel->rel_flags |= flag;
							break;
							}

						case SYM_procedure:
							{
							Procedure* sym_prc = (Procedure*) homonym->sym_object;
							sym_prc->addFlags (flag);
							break;
							}
						}
}


/**
  
 	hash
  
    @brief	Returns the hash function of a string.
 

    @param 
    @param 

 **/
static SSHORT hash(const SCHAR* string, USHORT length)
{
	SLONG value = 0;

	while (length--) {
		const SCHAR c = *string++;
		value = (value << 1) + c;
	}

	return ((value >= 0) ? value : -value) % HASH_SIZE;
}


/**
  
 	remove_symbol
  
    @brief	Given the address of a collision,
 	remove a symbol from the collision 
 	and homonym linked lists.
 

    @param collision
    @param symbol

 **/
static bool remove_symbol(DSQL_SYM* collision, DSQL_SYM symbol)
{
	if (symbol == *collision) {
	    DSQL_SYM homonym = symbol->sym_homonym;
		if (homonym != NULL) {
			homonym->sym_collision = symbol->sym_collision;
			*collision = homonym;
		}
		else
			*collision = symbol->sym_collision;

		return true;
	}

	for (DSQL_SYM* ptr = &(*collision)->sym_homonym; *ptr;
		ptr = &(*ptr)->sym_homonym)
	{
		if (symbol == *ptr) {
			*ptr = symbol->sym_homonym;
			return true;
		}
	}

	return false;
}


/**
  
 	scompare
  
    @brief	Compare two symbolic strings
 	The character set for these strings is either ASCII or
 	Unicode in UTF format.
 	Symbols are case-significant - so no uppercase operation
 	is performed.
 

    @param string1
    @param length1
    @param string2
    @param length2

 **/
static bool scompare(const TEXT* string1,
						USHORT length1,
						const TEXT* string2, const USHORT length2)
{

	if (length1 != length2)
		return false;

	while (length1--) {
		if ((*string1++) != (*string2++))
			return false;
	}

	return true;
}

