// InternalCrossReferenceResultSet.cpp: implementation of the InternalCrossReferenceResultSet class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include "fbdev.h"
#include "InternalCrossReferenceResultSet.h"

#define SQL_CASCADE                      0
#define SQL_RESTRICT                     1
#define SQL_SET_NULL                     2
#define SQL_NO_ACTION			 3
#define SQL_SET_DEFAULT			 4

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InternalCrossReferenceResultSet::InternalCrossReferenceResultSet(InternalDatabaseMetaData *metaData)
		: InternalMetaDataResultSet(metaData)
{

}

InternalCrossReferenceResultSet::~InternalCrossReferenceResultSet()
{

}

void InternalCrossReferenceResultSet::getCrossReference (const char * primaryCatalog, 
													const char * primarySchema, 
													const char * primaryTable, 
													const char * foreignCatalog, 
													const char * foreignSchema, 
													const char * foreignTable)
{
	JString sql = 
		"select NULL as pktable_cat,\n"						// 1
				" NULL as pktable_schem,\n"					// 2
				" pidx.rdb$relation_name as pktable_name,\n"// 3
				" pseg.rdb$field_name as pkcolumn_name,\n"	// 4
				" NULL as fktable_cat,\n"					// 5
				" NULL as fktable_schem,\n"					// 6
				" fidx.rdb$relation_name as fktable_name,\n"// 7
				" fseg.rdb$field_name as pkcolumn_name,\n"	// 8
				" pseg.rdb$field_position as key_seq,\n"	// 9
				" refc.rdb$update_rule as update_rule,\n"	// 10
				" refc.rdb$delete_rule as delete_rule,\n"	// 11
				" fkey.rdb$constraint_name as fk_name,\n"	// 12
				" refc.rdb$const_name_uq as pk_name,\n"		// 13
				" 7 as deferrability\n"						// 14	SQL_NOT_DEFERRABLE

		"from rdb$relation_constraints fkey,\n"
		"     rdb$indices fidx,\n"
		"     rdb$indices pidx,\n"
		"     rdb$index_segments fseg,\n"
		"     rdb$index_segments pseg,\n"
		"     rdb$ref_constraints refc\n"
		"where fkey.rdb$constraint_type = 'FOREIGN KEY'\n"
		"  and fkey.rdb$index_name = fidx.rdb$index_name\n"
		"  and fidx.rdb$foreign_key = pidx.rdb$index_name\n"
		"  and fidx.rdb$index_name = fseg.rdb$index_name\n"
		"  and pidx.rdb$index_name = pseg.rdb$index_name\n"
		"  and pseg.rdb$field_position = fseg.rdb$field_position"
		"  and refc.rdb$constraint_name = fkey.rdb$constraint_name"
		;

	if (primaryTable)
		sql += expandPattern (" and pidx.rdb$relation_name %s '%s'", primaryTable);

	if (foreignTable)
		sql += expandPattern (" and fkey.rdb$relation_name %s '%s'", foreignTable);

	sql += " order by pidx.rdb$relation_name, pseg.rdb$field_position";
	prepareStatement (sql);
	numberColumns = 14;
}

bool InternalCrossReferenceResultSet::next()
{
	if (!resultSet->next())
		return false;

	resultSet->setValue (10, getRule (resultSet->getString (10)));
	resultSet->setValue (11, getRule (resultSet->getString (11)));

	trimBlanks (3);			// primary key table name
	trimBlanks (4);			// primary key field name
	trimBlanks (7);			// foreign key table name
	trimBlanks (8);			// foreign key field name
	trimBlanks (12);		// foreign key name
	trimBlanks (13);		// primary key name

	return true;
}

int InternalCrossReferenceResultSet::getRule(const char * rule)
{
	if (stringEqual (rule, "CASCADE"))
		return SQL_CASCADE;

	if (stringEqual (rule, "SET NULL"))
		return SQL_SET_NULL;
	
	if (stringEqual (rule, "SET DEFAULT"))
		return SQL_SET_DEFAULT;

	return SQL_NO_ACTION;
}

bool InternalCrossReferenceResultSet::stringEqual(const char * p1, const char * p2)
{
	while (*p1 && *p2)
		if (*p1++ != *p2++)
			return false;

	while (*p1)
		if (*p1++ != ' ')
			return false;

	while (*p2)
		if (*p2++ != ' ')
			return false;

	return true;
}
