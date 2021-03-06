/*
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The contents of this file or any work derived from this file
 *  may not be distributed under any other license whatsoever 
 *  without the express prior written permission of the original 
 *  author.
 *
 *  The Original Code was created by James A. Starkey
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c)  2004 James A. Starkey
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): James A. Starkey
 */

#include "fbdev.h"
#include "Table.h"
#include "Connection.h"
#include "Field.h"

Table::Table(DatabaseMetaData *metaData, const char *tableName)
{
	name = tableName;
	fields = NULL;
	ResultSet *resultSet = metaData->getColumns(NULL, NULL, name, "%");
	Field **ptr = &fields;
	
	while (resultSet->next())
		{
		Field *field = new Field(this, resultSet);
		*ptr = field;
		ptr = &field->next;
		}
	
	resultSet->close();
}

Table::~Table(void)
{
	for (Field *field; field = fields;)
		{
		fields = field->next;
		delete field;
		}
}

Field* Table::findField(const char* fieldName)
{
	for (Field *field = fields; field; field = field->next)
		if (field->name == fieldName)
			return field;

	return NULL;
}
