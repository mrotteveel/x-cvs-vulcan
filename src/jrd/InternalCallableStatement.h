/*
 *  
 *     The contents of this file are subject to the Initial 
 *     Developer's Public License Version 1.0 (the "License"); 
 *     you may not use this file except in compliance with the 
 *     License. You may obtain a copy of the License at 
 *     http://www.ibphoenix.com/idpl.html. 
 *
 *     Software distributed under the License is distributed on 
 *     an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
 *     express or implied.  See the License for the specific 
 *     language governing rights and limitations under the License.
 *
 *     The contents of this file or any work derived from this file
 *     may not be distributed under any other license whatsoever 
 *     without the express prior written permission of the original 
 *     author.
 *
 *
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 1997 - 2000, 2001, 2003 James A. Starkey
 *  Copyright (c) 1997 - 2000, 2001, 2003 Netfrastructure, Inc.
 *  All Rights Reserved.
 */

#ifndef _INTERNALCALLABLESTATEMENT_H_
#define _INTERNALCALLABLESTATEMENT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Connection.h"
#include "InternalPreparedStatement.h"
#include "Values.h"
#include "DateTime.h"	// Added by ClassView
#include "SqlTime.h"
#include "TimeStamp.h"	// Added by ClassView
#include "JString.h"	// Added by ClassView

class InternalCallableStatement : public InternalPreparedStatement, public CallableStatement  
{
public:
	void getToken (const char **ptr, char *token);
	const char *rewriteSql (const char *originalSql, char *buffer, int length);
	typedef InternalPreparedStatement	Parent;

	InternalCallableStatement(InternalConnection *connection);
	virtual ~InternalCallableStatement();

	virtual void		registerOutParameter(int parameterIndex, int sqlType, int scale);
	virtual void		registerOutParameter(int parameterIndex, int sqlType);
	virtual const char*	getString (int id);
	virtual bool		wasNull();
	virtual TimeStamp	getTimestamp (int id);
	virtual DateTime	getDate(int id);
	virtual SqlTime		getTime (int id);
	virtual Blob*		getBlob (int id);
	virtual Clob*		getClob (int id);
	virtual double		getDouble  (int id);
	virtual float		getFloat (int id);
	virtual INT64		getLong  (int id);
	virtual int			getInt (int id);
	virtual char		getByte  (int id);
	virtual short		getShort (int id);

	virtual int			objectVersion();
	virtual void		setClob (int index, Clob *value);
	virtual StatementMetaData* getStatementMetaData();
	virtual bool		execute (const char *sqlString);
	virtual ResultSet*	executeQuery (const char *sqlString);
	virtual int			getUpdateCount();
	virtual bool		getMoreResults();
	virtual void		setCursorName (const char *name);
	virtual ResultSet*	getResultSet();
	virtual ResultList* search (const char *searchString);
	virtual int			executeUpdate (const char *sqlString);
	virtual void		close();
	virtual int			release();
	virtual void		addRef();

	virtual void		setNull (int index, int type);
	virtual void		setString(int index, const char * string);
	virtual void		setByte (int index, char value);
	virtual void		setShort (int index, short value);
	virtual void		setInt (int index, int value);
	virtual void		setLong (int index, INT64 value);
	virtual void		setFloat (int index, float value);
	virtual void		setDouble (int index, double value);
	virtual void		setDate (int index, DateTime value);
	virtual void		setTimestamp (int index, TimeStamp value);
	virtual void		setTime (int index, SqlTime value);
	virtual void		setBlob (int index, Blob *value);
	virtual void		setBytes (int index, int length, const void *bytes);

	virtual int			executeUpdate();
	virtual bool		execute();
	virtual ResultSet*	executeQuery();

	Value*				getValue (int index);
	virtual void		prepare (const char *sql);

	Values			values;
	bool			valueWasNull;
	int				minOutputVariable;
};

#endif
