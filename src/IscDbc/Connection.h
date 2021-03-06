// Connection.h: interface for the Connection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTION_H__BD560E62_B194_11D3_AB9F_0000C01D2301__INCLUDED_)
#define AFX_CONNECTION_H__BD560E62_B194_11D3_AB9F_0000C01D2301__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Blob.h"
#include "Properties.h"
#include "SQLException.h"

class Statement;
class PreparedStatement;
class CallableStatement;
class ResultSet;
class ResultList;
class DatabaseMetaData;
class ResultSetMetaData;
class DateTime;
class TimeStamp;
class Time;
class Clob;

#define CONNECTION_VERSION	1

class Connection  
{
public:
	virtual void		close() = 0;
	virtual void		addRef() = 0;
	virtual int			release() = 0;
	virtual bool		isConnected() = 0;

	virtual void		prepareTransaction() = 0;
	virtual void		rollback() = 0;
	virtual void		commit() = 0;

	virtual Clob*		genHTML (Properties *context, long genHeaders) = 0;

	virtual Statement*	createStatement() = 0;
	virtual PreparedStatement* prepareStatement (const char *sqlString) = 0;
	virtual DatabaseMetaData* getMetaData() = 0;
	virtual void		ping() = 0;
	virtual int			hasRole (const char *schemaName, const char *roleName) = 0;
	virtual void		openDatabase (const char *database, Properties *context) = 0;
	virtual void		createDatabase (const char *host, const char *dbName, Properties *context) = 0;
	virtual Properties*	allocProperties() = 0;
	virtual int			objectVersion() = 0;
	virtual Connection*	clone() = 0;
	virtual void		setAutoCommit (bool setting) = 0;
	virtual bool		getAutoCommit() = 0;
	virtual void		setTransactionIsolation (int level) = 0;
	virtual int			getTransactionIsolation() = 0;
	virtual CallableStatement* prepareCall (const char *sql) = 0;
};

#define DATABASEMETADATA_VERSION	1

class DatabaseMetaData 
{
public:
	virtual ResultSet* getIndexInfo (const char * catalog, const char * schemaPattern, const char * tableNamePattern, bool unique, bool approximate) = 0;
	virtual ResultSet* getImportedKeys (const char * catalog, const char * schemaPattern, const char * tableNamePattern) = 0;
	virtual ResultSet* getPrimaryKeys (const char * catalog, const char * schemaPattern, const char * tableNamePattern) = 0;
	virtual ResultSet* getColumns (const char *catalog, const char *schema, const char *table, const char *fieldNamePattern) = 0;
	virtual ResultSet* getTables (const char *catalog, const char *schemaPattern, const char *tableNamePattern, int typeCount, const char **types) = 0;
	virtual ResultSet* getObjectPrivileges (const char *catalog, const char *schemaPattern, const char *namePattern, int objectType) = 0;
	virtual ResultSet* getUserRoles (const char *user) = 0;
	virtual ResultSet* getRoles (const char * catalog, const char * schema, const char *rolePattern) = 0;
	virtual ResultSet* getUsers (const char * catalog, const char *userPattern) = 0;
	virtual bool allProceduresAreCallable() = 0;
	virtual bool allTablesAreSelectable() = 0;
	virtual const char* getURL() = 0;
	virtual const char* getUserName() = 0;
	virtual bool isReadOnly() = 0;
	virtual bool nullsAreSortedHigh() = 0;
	virtual bool nullsAreSortedLow() = 0;
	virtual bool nullsAreSortedAtStart() = 0;
	virtual bool nullsAreSortedAtEnd() = 0;
	virtual const char* getDatabaseProductName() = 0;
	virtual const char* getDatabaseProductVersion() = 0;
	virtual const char* getDriverName() = 0;
	virtual const char* getDriverVersion() = 0;
	virtual int getDriverMajorVersion() = 0;
	virtual int getDriverMinorVersion() = 0;
	virtual bool usesLocalFiles() = 0;
	virtual bool usesLocalFilePerTable() = 0;
	virtual bool supportsMixedCaseIdentifiers() = 0;
	virtual bool storesUpperCaseIdentifiers() = 0;
	virtual bool storesLowerCaseIdentifiers() = 0;
	virtual bool storesMixedCaseIdentifiers() = 0;
	virtual bool supportsMixedCaseQuotedIdentifiers() = 0;
	virtual bool storesUpperCaseQuotedIdentifiers() = 0;
	virtual bool storesLowerCaseQuotedIdentifiers() = 0;
	virtual bool storesMixedCaseQuotedIdentifiers() = 0;
	virtual const char* getIdentifierQuoteString() = 0;
	virtual const char* getSQLKeywords() = 0;
	virtual const char* getNumericFunctions() = 0;
	virtual const char* getStringFunctions() = 0;
	virtual const char* getSystemFunctions() = 0;
	virtual const char* getTimeDateFunctions() = 0;
	virtual const char* getSearchStringEscape() = 0;
	virtual const char* getExtraNameCharacters() = 0;
	virtual bool supportsAlterTableWithAddColumn() = 0;
	virtual bool supportsAlterTableWithDropColumn() = 0;
	virtual bool supportsColumnAliasing() = 0;
	virtual bool nullPlusNonNullIsNull() = 0;
	virtual bool supportsConvert() = 0;
	virtual bool supportsConvert(int fromType, int toType) = 0;
	virtual bool supportsTableCorrelationNames() = 0;
	virtual bool supportsDifferentTableCorrelationNames() = 0;
	virtual bool supportsExpressionsInOrderBy() = 0;
	virtual bool supportsOrderByUnrelated() = 0;
	virtual bool supportsGroupBy() = 0;
	virtual bool supportsGroupByUnrelated() = 0;
	virtual bool supportsGroupByBeyondSelect() = 0;
	virtual bool supportsLikeEscapeClause() = 0;
	virtual bool supportsMultipleResultSets() = 0;
	virtual bool supportsMultipleTransactions() = 0;
	virtual bool supportsNonNullableColumns() = 0;
	virtual bool supportsMinimumSQLGrammar() = 0;
	virtual bool supportsCoreSQLGrammar() = 0;
	virtual bool supportsExtendedSQLGrammar() = 0;
	virtual bool supportsANSI92EntryLevelSQL() = 0;
	virtual bool supportsANSI92IntermediateSQL() = 0;
	virtual bool supportsANSI92FullSQL() = 0;
	virtual bool supportsIntegrityEnhancementFacility() = 0;
	virtual bool supportsOuterJoins() = 0;
	virtual bool supportsFullOuterJoins() = 0;
	virtual bool supportsLimitedOuterJoins() = 0;
	virtual const char* getSchemaTerm() = 0;
	virtual const char* getProcedureTerm() = 0;
	virtual const char* getCatalogTerm() = 0;
	virtual bool isCatalogAtStart() = 0;
	virtual const char* getCatalogSeparator() = 0;
	virtual bool supportsSchemasInDataManipulation() = 0;
	virtual bool supportsSchemasInProcedureCalls() = 0;
	virtual bool supportsSchemasInTableDefinitions() = 0;
	virtual bool supportsSchemasInIndexDefinitions() = 0;
	virtual bool supportsSchemasInPrivilegeDefinitions() = 0;
	virtual bool supportsCatalogsInDataManipulation() = 0;
	virtual bool supportsCatalogsInProcedureCalls() = 0;
	virtual bool supportsCatalogsInTableDefinitions() = 0;
	virtual bool supportsCatalogsInIndexDefinitions() = 0;
	virtual bool supportsCatalogsInPrivilegeDefinitions() = 0;
	virtual bool supportsPositionedDelete() = 0;
	virtual bool supportsPositionedUpdate() = 0;
	virtual bool supportsSelectForUpdate() = 0;
	virtual bool supportsStoredProcedures() = 0;
	virtual bool supportsSubqueriesInComparisons() = 0;
	virtual bool supportsSubqueriesInExists() = 0;
	virtual bool supportsSubqueriesInIns() = 0;
	virtual bool supportsSubqueriesInQuantifieds() = 0;
	virtual bool supportsCorrelatedSubqueries() = 0;
	virtual bool supportsUnion() = 0;
	virtual bool supportsUnionAll() = 0;
	virtual bool supportsOpenCursorsAcrossCommit() = 0;
	virtual bool supportsOpenCursorsAcrossRollback() = 0;
	virtual bool supportsOpenStatementsAcrossCommit() = 0;
	virtual bool supportsOpenStatementsAcrossRollback() = 0;
	virtual int getMaxCharLiteralLength() = 0;
	virtual int getMaxColumnNameLength() = 0;
	virtual int getMaxColumnsInGroupBy() = 0;
	virtual int getMaxColumnsInIndex() = 0;
	virtual int getMaxColumnsInOrderBy() = 0;
	virtual int getMaxColumnsInSelect() = 0;
	virtual int getMaxColumnsInTable() = 0;
	virtual int getMaxConnections() = 0;
	virtual int getMaxCursorNameLength() = 0;
	virtual int getMaxIndexLength() = 0;
	virtual int getMaxSchemaNameLength() = 0;
	virtual int getMaxProcedureNameLength() = 0;
	virtual int getMaxCatalogNameLength() = 0;
	virtual int getMaxRowSize() = 0;
	virtual bool doesMaxRowSizeIncludeBlobs() = 0;
	virtual int getMaxStatementLength() = 0;
	virtual int getMaxStatements() = 0;
	virtual int getMaxTableNameLength() = 0;
	virtual int getMaxTablesInSelect() = 0;
	virtual int getMaxUserNameLength() = 0;
	virtual int getDefaultTransactionIsolation() = 0;
	virtual bool supportsTransactions() = 0;
	virtual bool supportsTransactionIsolationLevel(int level) = 0;
	virtual bool supportsDataDefinitionAndDataManipulationTransactions() = 0;
	virtual bool supportsDataManipulationTransactionsOnly() = 0;
	virtual bool dataDefinitionCausesTransactionCommit() = 0;
	virtual bool dataDefinitionIgnoredInTransactions() = 0;
	virtual ResultSet* getProcedures(const char* catalog, const char* schemaPattern,
			const char* procedureNamePattern) = 0;

	virtual ResultSet* getProcedureColumns(const char* catalog,
			const char* schemaPattern,
			const char* procedureNamePattern, 
			const char* columnNamePattern) = 0;

	virtual ResultSet* getSchemas() = 0;
	virtual ResultSet* getCatalogs() = 0;
	virtual ResultSet* getTableTypes() = 0;
	virtual ResultSet* getColumnPrivileges(const char* catalog, const char* schema,
		const char* table, const char* columnNamePattern) = 0;

	virtual ResultSet* getTablePrivileges(const char* catalog, const char* schemaPattern,
				const char* tableNamePattern) = 0;

	virtual ResultSet* getBestRowIdentifier(const char* catalog, const char* schema,
		const char* table, int scope, bool nullable) = 0;

	virtual ResultSet* getVersionColumns(const char* catalog, const char* schema,
				const char* table) = 0;

	virtual ResultSet* getExportedKeys(const char* catalog, const char* schema,
				const char* table) = 0;

	virtual ResultSet* getCrossReference(
		const char* primaryCatalog, const char* primarySchema, const char* primaryTable,
		const char* foreignCatalog, const char* foreignSchema, const char* foreignTable
		) = 0;

	virtual ResultSet* getTypeInfo() = 0;
	virtual bool supportsResultSetConcurrency(int type, int concurrency) = 0;
	virtual bool ownUpdatesAreVisible(int type) = 0;
	virtual bool ownDeletesAreVisible(int type) = 0;
	virtual bool ownInsertsAreVisible(int type) = 0;
	virtual bool othersUpdatesAreVisible(int type) = 0;
	virtual bool othersDeletesAreVisible(int type) = 0;
	virtual bool othersInsertsAreVisible(int type) = 0;
	virtual bool updatesAreDetected(int type) = 0;
	virtual bool deletesAreDetected(int type) = 0;
	virtual bool insertsAreDetected(int type) = 0;
	virtual bool supportsBatchUpdates() = 0;
	virtual ResultSet* getUDTs(const char* catalog, const char* schemaPattern, 
			  const char* typeNamePattern, int* types) = 0;
	virtual int		objectVersion() = 0;
};

#define STATEMENT_VERSION	1

class Statement  
{
public:
	virtual bool		execute (const char *sqlString) = 0;
	virtual ResultSet*	executeQuery (const char *sqlString) = 0;
	virtual int			getUpdateCount() = 0;
	virtual bool		getMoreResults() = 0;
	virtual void		setCursorName (const char *name) = 0;
	virtual ResultSet*	getResultSet() = 0;
	virtual ResultList* search (const char *searchString) = 0;
	virtual int			executeUpdate (const char *sqlString) = 0;
	virtual void		close() = 0;
	virtual int			release() = 0;
	virtual void		addRef() = 0;
	virtual int			objectVersion() = 0;
};

#define STATEMENTMETADATA_VERSION	1

class StatementMetaData  
{
public:
	virtual int			getParameterCount() = 0;
	virtual int			getParameterType (int index) = 0;
	virtual int			getPrecision(int index) = 0;
	virtual int			getScale(int index) = 0;
	virtual bool		isNullable (int index) = 0;
	virtual int			objectVersion() = 0;
};

#define PREPAREDSTATEMENT_VERSION	1

class PreparedStatement : public Statement  
{
public:
	virtual bool		execute() = 0;
	virtual ResultSet*	executeQuery() = 0;
	virtual int			executeUpdate() = 0;
	virtual void		setString(int index, const char * string) = 0;
	virtual void		setByte (int index, char value) = 0;
	virtual void		setShort (int index, short value) = 0;
	virtual void		setInt (int index, long value) = 0;
	virtual void		setLong (int index, INT64 value) = 0;
	virtual void		setBytes (uint64 index, uint64 length, const void *bytes) = 0;
	virtual void		setFloat (int index, float value) = 0;
	virtual void		setDouble (int index, double value) = 0;
	virtual void		setNull (int index, int type) = 0;
	virtual void		setDate (int index, DateTime value) = 0;
	virtual void		setTime (int index, Time value) = 0;
	virtual void		setTimestamp (int index, TimeStamp value) = 0;
	virtual void		setBlob (int index, Blob *value) = 0;
	virtual void		setClob (int index, Clob *value) = 0;
	virtual StatementMetaData*
						getStatementMetaData() = 0;
	virtual int			objectVersion() = 0;
};

#define RESULTSET_VERSION	1

class ResultSet  
{
public:
	virtual const char* getString (int id) = 0;
	virtual const char* getString (const char *columnName) = 0;
	virtual char		getByte (int id) = 0;
	virtual char		getByte (const char *columnName) = 0;
	virtual short		getShort (int id) = 0;
	virtual short		getShort (const char *columnName) = 0;
	virtual long		getInt (int id) = 0;
	virtual long		getInt (const char *columnName) = 0;
	virtual INT64		getLong (int id) = 0;
	virtual INT64		getLong (const char *columnName) = 0;
	virtual float		getFloat (int id) = 0;
	virtual float		getFloat (const char *columnName) = 0;
	virtual double		getDouble (int id) = 0;
	virtual double		getDouble (const char *columnName) = 0;
	virtual DateTime	getDate (int id) = 0;
	virtual DateTime	getDate (const char *columnName) = 0;
	virtual Time		getTime (int id) = 0;
	virtual Time		getTime (const char *columnName) = 0;
	virtual TimeStamp	getTimestamp (int id) = 0;
	virtual TimeStamp	getTimestamp (const char *columnName) = 0;
	virtual Blob*		getBlob (int index) = 0;
	virtual Blob*		getBlob (const char *columnName) = 0;
	virtual Clob*		getClob (int index) = 0;
	virtual Clob*		getClob (const char *columnName) = 0;
	virtual int			findColumn (const char *columName) = 0;
	virtual ResultSetMetaData* getMetaData() = 0;

	virtual void		close() = 0;
	virtual bool		next() = 0;
	virtual int			release() = 0;
	virtual void		addRef() = 0;
	virtual bool		wasNull() = 0;
	virtual int			objectVersion() = 0;
};

#define RESULTSETMETADATA_VERSION	1

class ResultSetMetaData  
{
public:
	virtual const char*	getTableName (int index) = 0;
	virtual const char*	getColumnName (int index) = 0;
	virtual int			getColumnDisplaySize (int index) = 0;
	virtual int			getColumnType (int index) = 0;
	virtual int			getColumnCount() = 0;
	virtual int			getPrecision(int index) = 0;
	virtual int			getScale(int index) = 0;
	virtual bool		isNullable (int index) = 0;
	virtual int			objectVersion() = 0;
	virtual const char*	getColumnLabel (int index) = 0;
	virtual bool		isSigned (int index) = 0;
	virtual bool		isReadOnly (int index) = 0;
	virtual bool		isWritable (int index) = 0;
	virtual bool		isDefinitelyWritable (int index) = 0;
};

#define RESULTLIST_VERSION		1

class ResultList  
{
public:
	virtual const char*	getTableName() = 0;
	virtual double		getScore() = 0;
	virtual int			getCount() = 0;
	virtual ResultSet*	fetchRecord() = 0;
	virtual bool		next() = 0;
	virtual void		close() = 0;
	virtual void		addRef() = 0;
	virtual int			release() = 0;
	virtual int			objectVersion() = 0;
};

#define CALLABLESTATEMENT_VERSION	1

class CallableStatement : public PreparedStatement
{
public:
	virtual void		registerOutParameter(int parameterIndex, int sqlType) = 0;
	virtual void		registerOutParameter(int parameterIndex, int sqlType, int scale) = 0;
	virtual bool		wasNull() = 0;
	virtual const char* getString(int parameterIndex) = 0;
	//virtual bool		getBoolean(int parameterIndex) = 0;
	virtual char		getByte(int parameterIndex) = 0;
	virtual short		getShort(int parameterIndex) = 0;
	virtual long		getInt(int parameterIndex) = 0;
	virtual INT64		getLong(int parameterIndex) = 0;
	virtual float		getFloat(int parameterIndex) = 0;
	virtual double		getDouble(int parameterIndex) = 0;
	//virtual byte[]	getBytes(int parameterIndex) = 0;
	virtual DateTime	getDate(int parameterIndex) = 0;
	virtual Time		getTime(int parameterIndex) = 0;
	virtual TimeStamp	getTimestamp(int parameterIndex) = 0;
	virtual Blob*		getBlob (int i) = 0;
	virtual Clob*		getClob (int i) = 0;
    //void		registerOutParameter (int paramIndex, int sqlType, const char* typeName) = 0;
};

extern "C" Connection*	createConnection();

#endif // !defined(AFX_CONNECTION_H__BD560E62_B194_11D3_AB9F_0000C01D2301__INCLUDED_)
