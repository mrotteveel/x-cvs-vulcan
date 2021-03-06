/*
 *	PROGRAM:	Dynamic SQL runtime support
 *	MODULE:		pass1.cpp
 *	DESCRIPTION:	First-pass compiler for request trees.
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
 *
 * 2001.5.26: Claudio Valderrama: field names should be skimmed from trailing
 *
 * 2001.5.26: Claudio Valderrama: COMPUTED fields will be skipped if a dummy
 *       "insert into tbl values(...)" sentence is issued.
 *
 * 2001.5.26: Claudio Valderrama: field names should be skimmed from trailing
 *
 * 2001.5.26: Claudio Valderrama: field names should be skimmed from trailing
 *		blanks to allow reliable comparisons in pass1_field. Same for table and
 *		and index names in plans.
 *
 * 2001.5.29: Claudio Valderrama: handle DROP VIEW case in pass1_statement().
 *
 * 2001.6.12: Claudio Valderrama: add basic BREAK capability to procedures.
 *
 * 2001.6.27: Claudio Valderrama: pass1_variable() now gives the name of the
 * variable it can't find in the error message.
 *
 * 2001.6.30: Claudio Valderrama: Enhanced again to provide (line, col), see node.h.
 *
 * 2001.7.28: John Bellardo: added code to handle nod_limit and associated fields.
 *
 * 2001.08.14 Claudio Valderrama: fixed crash with trigger and CURRENT OF <cursor> syntax.
 *
 * 2001.09.10 John Bellardo: fixed gen_rse to attribute skip/first nodes to the parent_rse
 *   if present instead of the child rse.  BUG #451798
 *
 * 2001.09.26 Claudio Valderrama: ambiguous field names are rejected from now.
 *
 * 2001.10.01 Claudio Valderrama: check constraints are allowed to have ambiguous field
 *   names because they use OLD and NEW as aliases of the same table. However, if the
 *   check constraint has an embedded ambiguous SELECT statement, it won't be detected.
 *   The code should be revisited if check constraints' before delete triggers are used
 *   for whatever reason. Currently they are never generated. The code can be improved
 *   to not report errors for fields between NEW and OLD contexts but complain otherwise.
 *
 * 2001.10.05 Neil McCalden: validate udf and parameters when comparing select list and
 *   group by list, to detect invalid SQL statements when grouping by UDFs.
 *
 * 2001.10.23 Ann Harrison:  allow reasonable checking of ambiguous names in unions.
 *   Remembering, later, that LLS_PUSH expects an object, not an LLS block.  Also
 *   stuck in the code for handling variables in pass1 - it apparently doesn't happen
 *   because the code returned an uninitialized pointer.
 *
 * 2001.11.17 Neil McCalden: Add aggregate_in_list procedure to handle cases
 *   where select statement has aggregate as a parameter to a udf which does
 *   not have to be in a group by clause.
 *
 * 2001.11.21 Claudio Valderrama: don't try to detect ambiguity in pass1_field()
 *   if the field or output procedure parameter has been fully qualified!!!
 *
 * 2001.11.27 Ann Harrison:  Redo the amiguity checking so as to give better
 *   error messages, return warnings for dialect 1, and simplify.
 *
 * 2001.11.28 Claudio Valderrama: allow udf arguments to be query parameters.
 *   Honor the code in the parser that already accepts those parameters.
 *   This closes SF Bug# 409769.
 *
 * 2001.11.29 Claudio Valderrama: make the nice new ambiguity checking code do the
 *   right thing instead of crashing the engine and restore fix from 2001.11.21.
 *
 * 2001.12.21 Claudio Valderrama: Fix SF Bug #494832 - pass1_variable() should work
 *   with def_proc, mod_proc, redef_proc, def_trig and mod_trig node types.
 *
 * 2002.07.30 Arno Brinkman: Added pass1_coalesce, pass1_simple_case, pass1_searched_case
 *   and pass1_put_args_on_stack
 *
 * 2002.08.04 Arno Brinkman: Added ignore_cast as parameter to node_match,
 *   Changed invalid_reference procedure for allow EXTRACT, SUBSTRING, CASE,
 *   COALESCE and NULLIF functions in GROUP BY and as select_items.
 *   Removed aggregate_in_list procedure.
 *
 * 2002.08.07 Dmitry Yemanov: Disabled BREAK statement in triggers
 *
 * 2002.08.10 Dmitry Yemanov: ALTER VIEW
 *
 * 2002.09.28 Dmitry Yemanov: Reworked internal_info stuff, enhanced
 *                            exception handling in SPs/triggers,
 *                            implemented ROWS_AFFECTED system variable
 *
 * 2002.09.29 Arno Brinkman: Adding more checking for aggregate functions
 *   and adding support for 'linking' from sub-selects to aggregate functions
 *   which are in an lower level. 
 *   Modified functions pass1_field, pass1_rse, copy_field, pass1_sort.
 *   Functions pass1_found_aggregate and pass1_found_field added.
 *
 * 2002.10.21 Nickolay Samofatov: Added support for explicit pessimistic locks
 *
 * 2002.10.25 Dmitry Yemanov: Re-allowed plans in triggers
 *
 * 2002.10.29 Nickolay Samofatov: Added support for savepoints
 *
 * 2002.12.03 Dmitry Yemanov: Implemented ORDER BY clause in subqueries
 *
 * 2002.12.18 Dmitry Yemanov: Fixed bug with BREAK and partially implemented
 *							  SQL-compliant labels and LEAVE statement
 *
 * 2003.01.11 Arno Brinkman: Reworked a lot of functions for bringing back backwards compatibilty
 *							 with sub-selects and aggregates.
 *
 * 2003.01.14 Dmitry Yemanov: Fixed bug with cursors in triggers
 *
 * 2003.01.15 Dmitry Yemanov: Added support for parametrized events
 *
 * 2003.04.05 Dmitry Yemanov: Changed logic of ORDER BY with collations
 *							  (because of the parser change)
 *
 * 2003.08.14 Arno Brinkman: Added derived table support.
 *
 * 2003.08.16 Arno Brinkman: Changed ambiguous column name checking.
 *
 * 2003.10.05 Dmitry Yemanov: Added support for explicit cursors in PSQL.
 */


// AB:Sync FB 1.208

#include <string.h>
#include <memory>
#include "fbdev.h"
#include "../jrd/ib_stdio.h"
#include "../jrd/ibase.h"
#include "../dsql/dsql.h"
#include "../jrd/thd.h"
#include "../jrd/intl.h"
#include "../jrd/blr.h"
#include "../dsql/alld_proto.h"
#include "../dsql/ddl_proto.h"
#include "../dsql/errd_proto.h"
#include "../dsql/hsh_proto.h"
#include "../dsql/make_proto.h"
#include "../dsql/metd_proto.h"
#include "../dsql/pass1_proto.h"
#include "../dsql/misc_func.h"
#include "../jrd/dsc_proto.h"
#include "../jrd/thd_proto.h"
#include "../jrd/Database.h"
#include "../jrd/Relation.h"
#include "../jrd/Field.h"
#include "../jrd/JVector.h"
#include "../jrd/Procedure.h"
#include "../jrd/ProcParam.h"
#include "../common/classes/array.h"

#include "old_vector.h"
#include "CsConvertArray.h"
#include "val.h"
#include "Cursor.h"
#include "Attachment.h"

#ifdef DEV_BUILD
void DSQL_pretty(const dsql_nod*, int);
#endif

class CStrCmp
{
public:
	static int greaterThan(const char* s1, const char* s2)
	{
		return strcmp(s1, s2) > 0;
	}
};


typedef firebird::SortedArray<const char*, 
			firebird::EmptyStorage<const char*>, const char*, 
			DefaultKeyValue<const char*>,
			CStrCmp>
		StrArray;
 

static bool aggregate_found(const CStatement*, const dsql_nod*);
static bool aggregate_found2(const CStatement*, const dsql_nod*, USHORT*,
	USHORT*, bool);
static dsql_nod* ambiguity_check(CStatement*, dsql_nod*, dsql_str*, Stack *stack);
static void assign_fld_dtype_from_dsc(dsql_fld*, const dsc*);
static void check_unique_fields_names(StrArray& names, const dsql_nod* fields);
static dsql_nod* compose(CStatement* request, dsql_nod*, dsql_nod*, NOD_TYPE);
static dsql_nod* explode_outputs(CStatement*, Procedure*);
static void field_error(const TEXT*, const TEXT*, const dsql_nod*);
static par* find_dbkey(const CStatement*, const dsql_nod*);
static par* find_record_version(const CStatement*, const dsql_nod*);
static bool invalid_reference(const dsql_ctx*, const dsql_nod*, 
	const dsql_nod*, bool, bool);
static bool node_match(const dsql_nod*, const dsql_nod*, bool);
static dsql_nod* pass1_alias_list(CStatement*, dsql_nod*);
static dsql_ctx* pass1_alias(CStatement*, Stack*, dsql_str*);
static dsql_str* pass1_alias_concat(CStatement* request, dsql_str*, dsql_str*);
static dsql_nod* pass1_any(CStatement*, dsql_nod*, NOD_TYPE);
static dsql_rel* pass1_base_table(CStatement*, dsql_rel*, dsql_str*);
static void pass1_blob(CStatement*, dsql_nod*);
static dsql_nod* pass1_coalesce(CStatement*, dsql_nod*, bool);
static dsql_nod* pass1_collate(CStatement*, dsql_nod*, dsql_str*);
static dsql_nod* pass1_constant(CStatement*, dsql_nod*);
static dsql_ctx* pass1_cursor_context(CStatement*, const dsql_nod*, const dsql_nod*);
static dsql_nod* pass1_cursor_name(CStatement*, dsql_str*, bool);
static dsql_nod* pass1_cursor_reference(CStatement*, const dsql_nod*, dsql_nod*);
static dsql_nod* pass1_dbkey(CStatement*, dsql_nod*);
static dsql_nod* pass1_delete(CStatement*, dsql_nod*);
static dsql_nod* pass1_derived_table(CStatement*, dsql_nod*, bool);
static dsql_nod* pass1_expand_select_list(CStatement*, dsql_nod*, dsql_nod*);
static void pass1_expand_select_node(CStatement*, dsql_nod*, Stack&);
static dsql_nod* pass1_field(CStatement*, dsql_nod*, const bool, dsql_nod*);
static bool pass1_found_aggregate(const dsql_nod*, USHORT, USHORT, bool);
static bool pass1_found_field(const dsql_nod*, USHORT, USHORT, bool*);
static bool pass1_found_sub_select(const dsql_nod*);
static dsql_nod* pass1_group_by_list(CStatement*, dsql_nod*, dsql_nod*);
static dsql_nod* pass1_insert(CStatement*, dsql_nod*);
static dsql_nod* pass1_join(CStatement*, dsql_nod*, bool);
static dsql_nod* pass1_label(CStatement*, dsql_nod*);
static dsql_nod* pass1_lookup_alias(CStatement*, const dsql_str*, dsql_nod*);
static dsql_nod* pass1_make_derived_field(CStatement*, TSQL, dsql_nod*);
static dsql_nod* pass1_not(CStatement*, const dsql_nod*, bool, bool);
static void	pass1_put_args_on_stack(CStatement*, dsql_nod*, Stack*, bool);
static dsql_nod* pass1_relation(CStatement*, dsql_nod*);
static dsql_nod* pass1_rse(CStatement*, dsql_nod*, dsql_nod*, dsql_nod*, dsql_nod*, USHORT);
static dsql_nod* pass1_savepoint(const CStatement*, dsql_nod*);
static dsql_nod* pass1_searched_case(CStatement*, dsql_nod*, bool);
static dsql_nod* pass1_sel_list(CStatement*, dsql_nod*);
static dsql_nod* pass1_simple_case(CStatement*, dsql_nod*, bool);
static dsql_nod* pass1_sort(CStatement*, dsql_nod*, dsql_nod*);
static dsql_nod* pass1_udf(CStatement*, dsql_nod*, bool);
static void pass1_udf_args(CStatement*, dsql_nod*, UserFunction*, USHORT, Stack*,
	bool);
static dsql_nod* pass1_union(CStatement*, dsql_nod*, dsql_nod*, dsql_nod*, USHORT);
static void pass1_union_auto_cast(CStatement* request, dsql_nod*, const dsc&, SSHORT,
	bool in_select_list = false);
static dsql_nod* pass1_update(CStatement*, dsql_nod*);
static dsql_nod* pass1_variable(CStatement*, dsql_nod*);
static bool check_field_type (const CStatement* , const dsql_fld*);
static dsql_fld* match_procedure_field (Procedure*, dsql_str*);
static dsql_fld* match_relation_field (dsql_rel*, dsql_str*);
static dsql_nod* post_map(CStatement* request, dsql_nod*, dsql_ctx*);
static dsql_nod* remap_field(CStatement*, dsql_nod*, dsql_ctx*, USHORT);
static dsql_nod* remap_fields(CStatement*, dsql_nod*, dsql_ctx*);
static void remap_streams_to_parent_context(dsql_nod*, dsql_ctx*);
static dsql_fld* resolve_context(CStatement*, dsql_str*, dsql_ctx*, bool);
static dsql_nod* resolve_variable_name(const dsql_nod* var_nodes, const dsql_str* var_name);
static bool set_parameter_type(CStatement* request, dsql_nod*, dsql_nod*, bool);
static void set_parameters_name(dsql_nod*, const dsql_nod*);
static void set_parameter_name(dsql_nod*, const dsql_nod*, dsql_rel*);
static TEXT* pass_exact_name(TEXT*);

const char* DB_KEY_STRING	= "DB_KEY"; // NTX: pseudo field name
const int MAX_MEMBER_LIST	= 1500;	// Maximum members in "IN" list.
									// For eg. SELECT * FROM T WHERE
									//         F IN (1, 2, 3, ...)
									// 
									// Bug 10061, bsriram - 19-Apr-1999

const int LIKE_PARAM_LEN = 30;	// CVC: This is a guess for the length of the
								// parameter for LIKE and others, when the
								// original dtype isn't string and force_varchar
								// is true.

enum field_match_val {
	FIELD_MATCH_TYPE_EQUAL = 0,
	FIELD_MATCH_TYPE_LOWER = 1,
	FIELD_MATCH_TYPE_LOWER_EQUAL = 2,
	FIELD_MATCH_TYPE_HIGHER = 3,
	FIELD_MATCH_TYPE_HIGHER_EQUAL = 4
};


/**
  
 	PASS1_make_context
  
    @brief	Generate a context for a request.
 

    @param request
    @param relation_node

 **/
dsql_ctx* PASS1_make_context(CStatement* request, dsql_nod* relation_node)
{
	dsql_rel* relation = NULL;
	Procedure* procedure = NULL;

	/* figure out whether this is a relation or a procedure
	   and give an error if it is neither */

	dsql_str* relation_name;

	if (relation_node->nod_type == nod_rel_proc_name)
		relation_name = (dsql_str*) relation_node->nod_arg[e_rpn_name];
	else if (relation_node->nod_type == nod_derived_table)
		relation_name = (dsql_str*) relation_node->nod_arg[e_derived_table_alias];
	else 
		relation_name = (dsql_str*) relation_node->nod_arg[e_rln_name];

    // CVC: Let's skim the context, too.
    if (relation_name && relation_name->str_data)
        pass_exact_name((TEXT*) relation_name->str_data);

	if (relation_node->nod_type == nod_derived_table) 
		{
		// No processing needed here for derived tables.
		}
	else if ((relation_node->nod_type == nod_rel_proc_name) &&
			relation_node->nod_arg[e_rpn_inputs])
		{
		//procedure = METD_get_procedure(request, relation_name);
		procedure = request->findProcedure (*relation_name);
		if (!procedure)
			{
			TEXT linecol[64];
			sprintf (linecol, "At line %d, column %d.",
					relation_node->nod_line, relation_node->nod_column);
			ERRD_post(isc_sqlerr, isc_arg_number, -204, isc_arg_gds,
						isc_dsql_procedure_err, isc_arg_gds, isc_random,
						isc_arg_string, relation_name->str_data, isc_arg_gds,
						isc_random, isc_arg_string, linecol, 0);
			}
		}
	else
		{
		//relation = METD_get_relation(request, relation_name);
		relation = request->findRelation (*relation_name);
		
		if (!relation && (relation_node->nod_type == nod_rel_proc_name))
			procedure = request->findProcedure (relation_name->str_data);
			//procedure = METD_get_procedure(request, relation_name);
			
		if (!relation && !procedure)
			{
			TEXT linecol[64];
			sprintf (linecol, "At line %d, column %d.",
					relation_node->nod_line, relation_node->nod_column);
			ERRD_post(isc_sqlerr, isc_arg_number, -204, isc_arg_gds,
						isc_dsql_relation_err, isc_arg_gds, isc_random,
						isc_arg_string, relation_name->str_data, isc_arg_gds, 
						isc_random, isc_arg_string, linecol, 0);
			}
		}

	if (procedure && procedure->findOutputCount() ==0) 
		{
		TEXT linecol[64];
		sprintf (linecol, "At line %d, column %d.",
				relation_node->nod_line, relation_node->nod_column);
		ERRD_post(isc_sqlerr, isc_arg_number, -84, isc_arg_gds, 
					isc_dsql_procedure_use_err, isc_arg_string, 
					relation_name->str_data,  isc_arg_gds, isc_random,
					isc_arg_string, linecol, 0);
		}

	// Set up context block.

	dsql_ctx* context = FB_NEW(*request->threadData->tsql_default) dsql_ctx;
	context->ctx_relation = relation;
	context->ctx_procedure = procedure;
	context->ctx_request = request;
	context->ctx_context = request->contextNumber++;
	context->ctx_scope_level = request->scopeLevel;

	// When we're in a outer-join part mark context for it.

	if (request->req_in_outer_join)
		context->ctx_flags |= CTX_outer_join;

	// find the context alias name, if it exists.

	dsql_str* string;

	if (relation_node->nod_type == nod_rel_proc_name)
		string = (dsql_str*) relation_node->nod_arg[e_rpn_alias];
	else if (relation_node->nod_type == nod_derived_table) 
		{
		string = (dsql_str*) relation_node->nod_arg[e_derived_table_alias];
		context->ctx_rse = relation_node->nod_arg[e_derived_table_rse];
		}
	else
		string = (dsql_str*) relation_node->nod_arg[e_rln_alias];

	if (string) 
		context->ctx_internal_alias = (TEXT*) string->str_data;

	if (request->aliasRelationPrefix && !(relation_node->nod_type == nod_derived_table))
		if (string)
			string = pass1_alias_concat(request, request->aliasRelationPrefix, string);
		else
			string = pass1_alias_concat(request, request->aliasRelationPrefix, relation_name);

	if (string) 
		{
		context->ctx_alias = (TEXT*) string->str_data;

		// check to make sure the context is not already used at this same  
		// query level (if there are no subqueries, this checks that the
		// alias is not used twice in the request).

		/***
		for (const Stack stack = request->context; stack; stack = stack->lls_next)
			{
			const dsql_ctx* conflict = (dsql_ctx*) stack->lls_object;
		***/
		FOR_STACK (dsql_ctx*, conflict, &request->context);

			if (conflict->ctx_scope_level != context->ctx_scope_level)
				continue;

			const TEXT* conflict_name;
			ISC_STATUS error_code;

			if (conflict->ctx_alias) 
				{
				conflict_name = conflict->ctx_alias;
				error_code = isc_alias_conflict_err;
				// alias %s conflicts with an alias in the same statement.
				}
			else if (conflict->ctx_procedure) 
				{
				conflict_name = (const TEXT *)conflict->ctx_procedure->findName();
				error_code = isc_procedure_conflict_error;
				// alias %s conflicts with a procedure in the same statement.
				}
			else if (conflict->ctx_relation) 
				{
				conflict_name = conflict->ctx_relation->rel_name;
				error_code = isc_relation_conflict_err;
				// alias %s conflicts with a relation in the same statement.
				}
			else
				continue;
			if (!strcmp(conflict_name, context->ctx_alias))
				ERRD_post(isc_sqlerr, isc_arg_number, -204,
						  isc_arg_gds, error_code,
						  isc_arg_string, conflict_name, 0);
			END_FOR
			//}
	}

	if (procedure) 
		{
		USHORT count = 0;

		if (relation_node->nod_arg[e_rpn_inputs])
			{
			context->ctx_proc_inputs = PASS1_node(request, relation_node->nod_arg[e_rpn_inputs], false);
			count = context->ctx_proc_inputs->nod_count;
			}

		if (!(request->flags & REQ_procedure)) 
			{
			if (count != procedure->findInputCount())
				ERRD_post(isc_prcmismat, isc_arg_string, relation_name->str_data, 0);

			if (count)
				{
				// Initialize this stack variable, and make it look like a node
				std::auto_ptr<dsql_nod> desc_node(FB_NEW_RPT(*request->threadData->tsql_default, 0) dsql_nod);

				dsql_nod* const* input = context->ctx_proc_inputs->nod_arg;

				for (ProcParam *param = procedure->findInputParameters();
					 param; input++, param = param->findNext())
					{
					// MAKE_desc_from_field(&desc_node.nod_desc, field);
					//	set_parameter_type(*input, &desc_node, false);
					//MAKE_desc_from_field(request->threadData, &(desc_node->nod_desc), param->dsqlField);
					dsql_nod temp;
					temp.nod_desc = param->paramDescriptor;
					set_parameter_type(request, *input, &temp, false);
					}
				}
			}
		}

	// push the context onto the request context stack 
	// for matching fields against

	//LLS_PUSH(context, &request->context);
	request->context.push(context);
	
	return context;
}


/**
  
 	PASS1_node
  
    @brief	Compile a parsed request into something more interesting.
 

    @param request
    @param input
    @param proc_flag

 **/
dsql_nod* PASS1_node(CStatement* request, dsql_nod* input, bool proc_flag)
{
	if (input == NULL)
		return NULL;

	dsql_nod* node;
	dsql_fld* field;
	dsql_nod* sub1;

// Dispatch on node type.  Fall thru on easy ones 

	switch (input->nod_type) 
		{
		case nod_alias:
			node = MAKE_node(request->threadData, input->nod_type, e_alias_count);
			node->nod_arg[e_alias_value] = sub1 =
				PASS1_node(request, input->nod_arg[e_alias_value], proc_flag);
			node->nod_arg[e_alias_alias] = input->nod_arg[e_alias_alias];
			node->nod_desc = sub1->nod_desc;
			return node;

		case nod_cast:
			node = MAKE_node(request->threadData, input->nod_type, e_cast_count);
			node->nod_arg[e_cast_source] = sub1 =
				PASS1_node(request, input->nod_arg[e_cast_source], proc_flag);
			node->nod_arg[e_cast_target] = input->nod_arg[e_cast_target];
			field = (dsql_fld*) node->nod_arg[e_cast_target];
			DDL_resolve_intl_type(request, field, NULL);
			MAKE_desc_from_field(request->threadData, &node->nod_desc, field);
			// If the source is nullable, so is the target 
			MAKE_desc(request->threadData, &sub1->nod_desc, sub1, NULL);
			
			if (sub1->nod_desc.dsc_flags & DSC_nullable)
				node->nod_desc.dsc_flags |= DSC_nullable;
			return node;

		case nod_coalesce:
			return pass1_coalesce(request, input, proc_flag);

		case nod_derived_field:
			return input;

		case nod_simple_case:
			return pass1_simple_case(request, input, proc_flag);
	    
		case nod_searched_case:
			return pass1_searched_case(request, input, proc_flag);

		case nod_gen_id:
		case nod_gen_id2:
			node = MAKE_node(request->threadData, input->nod_type, e_gen_id_count);
			node->nod_arg[e_gen_id_value] =
				PASS1_node(request, input->nod_arg[e_gen_id_value], proc_flag);
			node->nod_arg[e_gen_id_name] = input->nod_arg[e_gen_id_name];
			return node;

		case nod_collate:
			request->tempCollationName = (dsql_str*) input->nod_arg[e_coll_target];
			sub1 = PASS1_node(request, input->nod_arg[e_coll_source], proc_flag);
			request->tempCollationName = NULL;
			node =
				pass1_collate(request, sub1, (dsql_str*) input->nod_arg[e_coll_target]);
			return node;

		case nod_extract:

			/* Figure out the data type of the sub parameter, and make
			sure the requested type of information can be extracted */

			sub1 = PASS1_node(request, input->nod_arg[e_extract_value], proc_flag);
			MAKE_desc(request->threadData, &sub1->nod_desc, sub1, NULL);

			switch (*(SLONG*)input->nod_arg[e_extract_part]->nod_desc.dsc_address)
				{
				case blr_extract_year:
				case blr_extract_month:
				case blr_extract_day:
				case blr_extract_weekday:
				case blr_extract_yearday:
					if (sub1->nod_desc.dsc_dtype != dtype_sql_date &&
						sub1->nod_desc.dsc_dtype != dtype_timestamp)
						ERRD_post(isc_sqlerr, isc_arg_number, -105,
								isc_arg_gds, isc_extract_input_mismatch, 0);
					break;
					
				case blr_extract_hour:
				case blr_extract_minute:
				case blr_extract_second:
					if (sub1->nod_desc.dsc_dtype != dtype_sql_time &&
						sub1->nod_desc.dsc_dtype != dtype_timestamp)
					{
						ERRD_post(isc_sqlerr, isc_arg_number, -105,
								isc_arg_gds, isc_extract_input_mismatch, 0);
					}
					break;
					
				default:
					fb_assert(FALSE);
					break;
				}
				
			node = MAKE_node(request->threadData, input->nod_type, e_extract_count);
			node->nod_arg[e_extract_part] = input->nod_arg[e_extract_part];
			node->nod_arg[e_extract_value] = sub1;
			
			if (sub1->nod_desc.dsc_flags & DSC_nullable)
				node->nod_desc.dsc_flags |= DSC_nullable;
				
			return node;

		case nod_delete:
		case nod_insert:
		case nod_order:
		case nod_select:
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_dsql_command_err, 0);

		case nod_derived_table:
			return pass1_derived_table(request, input, proc_flag);

		case nod_select_expr:
			{
			if (proc_flag)
				ERRD_post(isc_sqlerr, isc_arg_number, -206,
						isc_arg_gds, isc_dsql_subselect_err, 0);

			//base = request->context;
			void *base = request->context.mark();
			node = MAKE_node(request->threadData, nod_via, e_via_count);
			dsql_nod* rse = PASS1_rse(request, input, NULL);
			node->nod_arg[e_via_rse] = rse;
			node->nod_arg[e_via_value_1] = rse->nod_arg[e_rse_items]->nod_arg[0];
			node->nod_arg[e_via_value_2] = MAKE_node(request->threadData, nod_null, (int) 0);
			request->context.pop (base);
				
			return node;
			}

		case nod_exists:
		case nod_singular:
			{
			void* base = request->context.mark();
			node = MAKE_node(request->threadData, input->nod_type, 1);
			input = input->nod_arg[0];
			node->nod_arg[0] = PASS1_rse(request, input, NULL);
			request->context.pop (base);
				
			return node;
			}
			
		case nod_field_name:
			if (proc_flag)
				return pass1_variable(request, input);
			return pass1_field(request, input, false, NULL);

		case nod_field:
			// AB: nod_field is an already passed node.
			// This could be done in expand_select_list.
			return input;

		case nod_array:
			if (proc_flag)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_dsql_invalid_array, 0);
			return pass1_field(request, input, false, NULL);

		case nod_variable:
			node = MAKE_node (request->threadData, input->nod_type, e_var_count);
			node->nod_arg[0] = input->nod_arg[0];
			node->nod_desc = input->nod_desc;
			return node;

		case nod_var_name:
			return pass1_variable(request, input);

		case nod_dbkey:
			return pass1_dbkey(request, input);

		case nod_relation_name:
		case nod_rel_proc_name:
			return pass1_relation(request, input);

		case nod_constant:
			return pass1_constant(request, input);

		case nod_parameter:
			if (proc_flag)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_dsql_command_err, 0);
						
			node = MAKE_node(request->threadData, input->nod_type, e_par_count);
			node->nod_count = 0;
			node->nod_arg[e_par_parameter] = 
				(dsql_nod*) request->makeParameter (request->sendMessage,
										true, true,
										// Pass 0 here to restore older parameter
										// ordering behavior unconditionally.
										(USHORT)(long) input->nod_arg[0]);
			return node;


	case nod_param_val:
		node = MAKE_node(request->threadData, input->nod_type, e_prm_val_count);
		node->nod_arg[e_prm_val_fld] = input->nod_arg[e_prm_val_fld];
		node->nod_arg[e_prm_val_val] = PASS1_node(request, input->nod_arg[e_prm_val_val], proc_flag);
		
		field	= (dsql_fld*) node->nod_arg[e_prm_val_fld]->nod_arg[e_dfl_field];
		DDL_resolve_intl_type(request, field, NULL);

		{
			dsql_nod *temp	= node->nod_arg[e_prm_val_val];
			// Initialize this stack variable, and make it look like a node
			std::auto_ptr<dsql_nod> desc_node(FB_NEW_RPT(*request->threadData->tsql_default, 0) dsql_nod);

			DEV_BLKCHK(field, dsql_type_fld);
			DEV_BLKCHK(temp, dsql_type_nod);
			MAKE_desc_from_field(request->threadData, &(desc_node->nod_desc), field);
			set_parameter_type(request, temp, desc_node.get(), false);
		}
		
		return node;

		case nod_udf:
			return pass1_udf(request, input, proc_flag);

		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_lss:
		case nod_leq:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_lss_any:
		case nod_leq_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_lss_all:
		case nod_leq_all:
			{
			dsql_nod* sub2 = input->nod_arg[1];
			if (sub2->nod_type == nod_list) 
				{
				USHORT list_item_count = 0;
				node = NULL;
				dsql_nod** ptr = sub2->nod_arg;

				for (const dsql_nod* const* const end = ptr + sub2->nod_count;
					ptr < end && list_item_count < MAX_MEMBER_LIST;
					list_item_count++, ptr++) 
					{
					dsql_nod* temp = MAKE_node(request->threadData, input->nod_type, 2);
					temp->nod_arg[0] = input->nod_arg[0];
					temp->nod_arg[1] = *ptr;
					node = compose(request, node, temp, nod_or);
					}

				if (list_item_count >= MAX_MEMBER_LIST)
					ERRD_post(isc_sqlerr, isc_arg_number, -901,
							isc_arg_gds, isc_imp_exc,
							isc_arg_gds, isc_random,
							isc_arg_string,
							"too many values (more than 1500) in member list to match against",
							0);
				return PASS1_node(request, node, proc_flag);
				}
				
			if (sub2->nod_type == nod_select_expr) 
				{
				if (proc_flag)
					ERRD_post(isc_sqlerr, isc_arg_number, -206,
							isc_arg_gds, isc_dsql_subselect_err, 0);

				if (sub2->nod_flags & NOD_SELECT_EXPR_SINGLETON)
					{
					void *base = request->context.mark();
					node = MAKE_node(request->threadData, input->nod_type, 2);
					node->nod_arg[0] = PASS1_node(request, input->nod_arg[0], false);
					dsql_nod* temp = MAKE_node(request->threadData, nod_via, e_via_count);
					node->nod_arg[1] = temp;
					dsql_nod* rse = PASS1_rse(request, sub2, NULL);
					temp->nod_arg[e_via_rse] = rse;
					temp->nod_arg[e_via_value_1] =
						rse->nod_arg[e_rse_items]->nod_arg[0];
					temp->nod_arg[e_via_value_2] = MAKE_node(request->threadData, nod_null, (int) 0);
					request->context.pop (base);

					return node;
					}
				else 
					switch (input->nod_type) 
						{
						case nod_equiv:
						case nod_eql:
						case nod_neq:
						case nod_gtr:
						case nod_geq:
						case nod_lss:
						case nod_leq:
							return pass1_any(request, input, nod_any);

						case nod_eql_any:
						case nod_neq_any:
						case nod_gtr_any:
						case nod_geq_any:
						case nod_lss_any:
						case nod_leq_any:
							return pass1_any(request, input, nod_ansi_any);

						case nod_eql_all:
						case nod_neq_all:
						case nod_gtr_all:
						case nod_geq_all:
						case nod_lss_all:
						case nod_leq_all:
							return pass1_any(request, input, nod_ansi_all);
							
						default:	// make compiler happy
							break;
						}
				}
			break;
			}

		case nod_agg_count:
		case nod_agg_min:
		case nod_agg_max:
		case nod_agg_average:
		case nod_agg_total:
		case nod_agg_average2:
		case nod_agg_total2:
			if (proc_flag) 
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_dsql_command_err, 0);

			if (!(request->req_in_select_list || request->req_in_where_clause  ||
				request->req_in_group_by_clause  || request->req_in_having_clause ||
				request->req_in_order_by_clause))
				{
				/* not part of a select list, where clause, group by clause,
				having clause, or order by clause */
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_dsql_agg_ref_err, 0);
				}
				
			node = MAKE_node(request->threadData, input->nod_type, e_agg_function_count);
			node->nod_count = input->nod_count;  // Copy count, because this must be exactly the same as input.
			node->nod_flags = input->nod_flags;
			
			if (input->nod_count) { 
				node->nod_arg[e_agg_function_expression] = PASS1_node(request, input->nod_arg[0], proc_flag);
			}
			else {
				// Scope level is needed to determine to which context COUNT(*) belongs.
				node->nod_arg[e_agg_function_scope_level] = (dsql_nod*) (IPTR) request->scopeLevel;
			}

			return node;

			// access plan node types 

		case nod_plan_item:
			node = MAKE_node(request->threadData, input->nod_type, 2);
			node->nod_arg[0] = sub1 = MAKE_node(request->threadData, nod_relation, e_rel_count);
			sub1->nod_arg[e_rel_context] =
				pass1_alias_list(request, input->nod_arg[0]);
			node->nod_arg[1] = PASS1_node(request, input->nod_arg[1], proc_flag);
			return node;

		case nod_index:
			node = MAKE_node(request->threadData, input->nod_type, 1);
			node->nod_arg[0] = input->nod_arg[0];
			return node;

		case nod_index_order:
			node = MAKE_node(request->threadData, input->nod_type, 2);
			node->nod_arg[0] = input->nod_arg[0];
			node->nod_arg[1] = input->nod_arg[1];
			return node;

		case nod_dom_value:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_desc = input->nod_desc;
			return node;

		case nod_internal_info:
			{
			const internal_info_id id =
				*reinterpret_cast<internal_info_id*>(input->nod_arg[0]->nod_desc.dsc_address);
			USHORT req_mask = InternalInfo::getMask(id);
			if (req_mask && !(request->flags & req_mask))
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_token_err, // Token unknown 
					isc_arg_gds, isc_random, isc_arg_string, InternalInfo::getAlias(id), 0);
			}
			break;

		case nod_join:
			return pass1_join(request, input, proc_flag);

		case nod_not:
			return pass1_not(request, input, proc_flag, true);

		default:
			break;
		}

// Node is simply to be rebuilt -- just recurse merrily 

	node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
	const dsql_nod** ptr2 = const_cast<const dsql_nod**>(node->nod_arg);
	dsql_nod** ptr = input->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + input->nod_count; ptr < end; ptr++) 
		*ptr2++ = PASS1_node(request, *ptr, proc_flag);

	// Try to match parameters against things of known data type.
	
	dsql_nod* sub2 = NULL;
	dsql_nod* sub3 = NULL;

	switch (node->nod_type) 
		{
		case nod_between:
			sub3 = node->nod_arg[2];
			// FALLINTO 
		case nod_assign:
		case nod_equiv:
		case nod_eql:
		case nod_gtr:
		case nod_geq:
		case nod_leq:
		case nod_lss:
		case nod_neq:
		case nod_eql_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_leq_any:
		case nod_lss_any:
		case nod_neq_any:
		case nod_eql_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_leq_all:
		case nod_lss_all:
		case nod_neq_all:
			sub1 = node->nod_arg[0];
			sub2 = node->nod_arg[1];

			/* Try to force sub1 to be same type as sub2 eg: ? = FIELD case */
			
			if (!set_parameter_type(request, sub1, sub2, false)) {
				/* That didn't work - try to force sub2 same type as sub 1 eg: FIELD = ? case */
				set_parameter_type(request, sub2, sub1, false);
			}
				
			if (sub3)
				/* X BETWEEN Y AND ? case */
				if (!set_parameter_type(request, sub3, sub2, false)) {
					/* ? BETWEEN Y AND ? case */
					set_parameter_type(request, sub3, sub2, false);
				}
			break;

		case nod_like:
			if (node->nod_count == 3) 
				sub3 = node->nod_arg[2];
		case nod_containing:
		case nod_starting:
			sub1 = node->nod_arg[0];
			sub2 = node->nod_arg[1];

			/* Try to force sub1 to be same type as sub2 eg: ? LIKE FIELD case */
			
			if (!set_parameter_type(request, sub1, sub2, true))
				set_parameter_type(request, sub2, sub1, true);
				
			if (sub3)
				/* X LIKE Y ESCAPE ? case */
				set_parameter_type(request, sub3, sub2, true);
			break;

		default:
			break;
		}

	return node;
}


/**
  
 	PASS1_rse
  
    @brief	Compile a record selection expression, 
 	bumping up the request scope level 
 	everytime an rse is seen.  The scope
 	level controls parsing of aliases.
 

    @param request
    @param input
    @param order
    @param update_lock

 **/
dsql_nod* PASS1_rse(CStatement* request, dsql_nod* input, dsql_nod* update_lock)
{

	fb_assert(input->nod_type == nod_select_expr);

	request->scopeLevel++;
	dsql_nod* node = pass1_rse(request, input, NULL, NULL, update_lock, 0);
	request->scopeLevel--;

	return node;
}


/**
  
 	PASS1_statement
  
    @brief	Compile a parsed request into something more interesting.
 

    @param request
    @param input
    @param proc_flag

 **/
dsql_nod* PASS1_statement(CStatement* request, dsql_nod* input, bool proc_flag)
{

#ifdef DSQL_DEBUG
	if (DSQL_debug & 2) {
		dsql_trace("Node tree at DSQL pass1 entry:");
		DSQL_pretty(input, 0);
	}
#endif

	dsql_nod* node;
	void* base = request->context.mark();

// Dispatch on node type.  Fall thru on easy ones 

	switch (input->nod_type) 
		{
		case nod_def_relation:
		case nod_redef_relation:
		case nod_mod_relation:
		case nod_del_relation:
		case nod_def_index:
		case nod_mod_index:
		case nod_del_index:
		case nod_def_view:
		case nod_redef_view:
		case nod_mod_view:
		case nod_replace_view:
		case nod_del_view:
		case nod_def_constraint:
		case nod_def_exception:
		case nod_redef_exception:
		case nod_mod_exception:
		case nod_replace_exception:
		case nod_del_exception:
		case nod_grant:
		case nod_revoke:
		case nod_def_database:
		case nod_mod_database:
		case nod_def_generator:
		case nod_del_generator:
		case nod_def_role:
		case nod_del_role:
		case nod_def_filter:
		case nod_del_filter:
		case nod_def_domain:
		case nod_del_domain:
		case nod_mod_domain:
		case nod_def_udf:
		case nod_del_udf:
		case nod_def_shadow:
		case nod_del_shadow:
		case nod_set_statistics:
		case nod_def_user:
		case nod_mod_user:
		case nod_del_user:
		case nod_upg_user:
			request->req_type = REQ_DDL;
			return input;

		case nod_def_trigger:
		case nod_mod_trigger:
		case nod_replace_trigger:
		case nod_del_trigger:
			request->req_type = REQ_DDL;
			request->flags |= REQ_procedure;
			request->flags |= REQ_trigger;
			return input;

		case nod_del_procedure:
			request->req_type = REQ_DDL;
			request->flags |= REQ_procedure;
			return input;

		case nod_def_procedure:
		case nod_redef_procedure:
		case nod_mod_procedure:
		case nod_replace_procedure:
			{
			request->req_type = REQ_DDL;
			request->flags |= REQ_procedure;

			const dsql_nod* variables = input->nod_arg[e_prc_dcls];
			if (variables) {

				// insure that variable names do not duplicate parameter names

				const dsql_nod* const* ptr = variables->nod_arg;
				for (const dsql_nod* const* const end = ptr + variables->nod_count;
					ptr < end; ptr++)
				{
					if ((*ptr)->nod_type == nod_def_field) {

						dsql_fld* field = (dsql_fld*) (*ptr)->nod_arg[e_dfl_field];

						const dsql_nod* parameters = input->nod_arg[e_prc_inputs];
						if (parameters) {

							const dsql_nod* const* ptr2 = parameters->nod_arg;
							for (const dsql_nod* const* const end2 =
								ptr2 + parameters->nod_count; ptr2 < end2; ptr2++)
							{
								dsql_fld* field2 =  (dsql_fld*) (*ptr2)->nod_arg[e_dfl_field];
								if (!strcmp(field->fld_name, field2->fld_name))
									ERRD_post(isc_sqlerr, isc_arg_number,
											-901, isc_arg_gds,
											isc_dsql_var_conflict, isc_arg_string,
											(const char*) field->fld_name, 0);
							}
						}

						parameters = input->nod_arg[e_prc_outputs];
						if (parameters) {

							const dsql_nod* const* ptr2 = parameters->nod_arg;
							for (const dsql_nod* const* const end2 =
								ptr2 + parameters->nod_count; ptr2 < end2; ptr2++)
							{
								dsql_fld* field2 = 
									(dsql_fld*) (*ptr2)->nod_arg[e_dfl_field];
								if (!strcmp(field->fld_name, field2->fld_name))
									ERRD_post(isc_sqlerr, isc_arg_number,
											-901, isc_arg_gds,
											isc_dsql_var_conflict, isc_arg_string,
											(const char*) field->fld_name, 0);
							}
						}
					}
				}
			}
			return input;
			}

		case nod_assign:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[0] = PASS1_node(request, input->nod_arg[0], proc_flag);
			node->nod_arg[1] = PASS1_node(request, input->nod_arg[1], proc_flag);
			break;

		case nod_commit:
			if ((input->nod_arg[e_commit_retain]) &&
				(input->nod_arg[e_commit_retain]->nod_type == nod_commit_retain))
				request->req_type = REQ_COMMIT_RETAIN;
			else
				request->req_type = REQ_COMMIT;
			return input;

		case nod_delete:
			node = pass1_savepoint(request, pass1_delete(request, input));
			break;

		case nod_exec_procedure:
			{
			// AB:TODO Fix problem with self-referencing procedures 
			//   Procedures that call themself recursivly are not found
			//	 by request->findProcedure()


			dsql_str* name = (dsql_str*) input->nod_arg[e_exe_procedure];
			Procedure* procedure = request->findProcedure (*name);

			if (!procedure)
				ERRD_post(isc_sqlerr, isc_arg_number, -204,
						isc_arg_gds, isc_dsql_procedure_err,
						isc_arg_gds, isc_random,
						isc_arg_string, name->str_data, 0);
			
			if (!proc_flag)
				{
				request->procedure = procedure;							
				request->req_type = REQ_EXEC_PROCEDURE;
				}
				
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_exe_procedure] = input->nod_arg[e_exe_procedure];

			// handle input parameters

			USHORT count = input->nod_arg[e_exe_inputs] ?
				input->nod_arg[e_exe_inputs]->nod_count : 0;

			//if (count > procedure->prc_in_count || 
			//	count < procedure->prc_in_count - procedure->prc_def_count)
			if (count != procedure->findInputCount())
				ERRD_post(isc_prcmismat, isc_arg_string, name->str_data, 0);

			node->nod_arg[e_exe_inputs] =  PASS1_node(request, input->nod_arg[e_exe_inputs], proc_flag);

			if (count) 
				{
				// Initialize this stack variable, and make it look like a node
				std::auto_ptr<dsql_nod> desc_node(FB_NEW_RPT(*getDefaultMemoryPool(), 0) dsql_nod);
				dsql_nod** ptr = node->nod_arg[e_exe_inputs]->nod_arg;
				
				for (ProcParam *param = procedure->findInputParameters();
					param; ptr++, param = param->findNext()) 
					{
					// MAKE_desc_from_field(&desc_node.nod_desc, field);
					// set_parameter_type(*ptr, &desc_node, false);
					MAKE_desc_from_field(request->threadData, &(desc_node->nod_desc), param->getDsqlField());
					set_parameter_type(request, *ptr, desc_node.get(), false);
					}
				}

			// handle output parameters

			dsql_nod* temp = input->nod_arg[e_exe_outputs];
			if (proc_flag) 
				{
				count = temp ? temp->nod_count : 0;
				if (count != procedure->findOutputCount())
					ERRD_post(isc_prcmismat, isc_arg_string, name->str_data, 0);

				node->nod_arg[e_exe_outputs] = PASS1_node(request, temp, proc_flag);
				}
			else
				{
				if (temp)
					ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 104, isc_arg_gds, 
							isc_token_err, // Token unknown 
							isc_arg_gds, isc_random, isc_arg_string, "RETURNING_VALUES", 0);

				node->nod_arg[e_exe_outputs] =
					explode_outputs(request, procedure);
				}

			break;
			}

		case nod_exec_block: 
			if (input->nod_arg[e_exe_blk_outputs] &&
				input->nod_arg[e_exe_blk_outputs]->nod_count)
			{
				request->req_type = REQ_SELECT_BLOCK;
			}
			else
				request->req_type = REQ_EXEC_BLOCK;
			request->flags |= REQ_block;

			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_exe_blk_inputs] = PASS1_node(request, input->nod_arg[e_exe_blk_inputs], false);
			node->nod_arg[e_exe_blk_outputs] = input->nod_arg[e_exe_blk_outputs];

			node->nod_arg[e_exe_blk_dcls] = input->nod_arg[e_exe_blk_dcls];
			node->nod_arg[e_exe_blk_body] = input->nod_arg[e_exe_blk_body];

			{
				StrArray names( *getDefaultMemoryPool(),
					node->nod_arg[e_exe_blk_inputs] ? 
						node->nod_arg[e_exe_blk_inputs]->nod_count : 0 +
					node->nod_arg[e_exe_blk_outputs] ? 
						node->nod_arg[e_exe_blk_outputs]->nod_count : 0 +
					node->nod_arg[e_exe_blk_dcls] ? 
						node->nod_arg[e_exe_blk_dcls]->nod_count : 0
					);

				check_unique_fields_names(names, node->nod_arg[e_exe_blk_inputs]);
				check_unique_fields_names(names, node->nod_arg[e_exe_blk_outputs]);
				check_unique_fields_names(names, node->nod_arg[e_exe_blk_dcls]);
			}
			return node;

		case nod_for_select:
			{
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_flags = input->nod_flags;
			dsql_nod* cursor = node->nod_arg[e_flp_cursor] = input->nod_arg[e_flp_cursor];

			node->nod_arg[e_flp_select] = 
				PASS1_statement(request, input->nod_arg[e_flp_select], proc_flag);

			if (cursor) 
				{
				pass1_cursor_name(request, (dsql_str*) cursor->nod_arg[e_cur_name], false);
				cursor->nod_arg[e_cur_rse] = node;
				cursor->nod_arg[e_cur_number] = (dsql_nod*) (IPTR) request->cursorNumber++;
				//LLS_PUSH(cursor, &request->cursors);
				request->cursors.push (cursor);
				}

			dsql_nod* into_in = input->nod_arg[e_flp_into];
			dsql_nod* into_out = MAKE_node(request->threadData, into_in->nod_type, into_in->nod_count);
			node->nod_arg[e_flp_into] = into_out;
			const dsql_nod** ptr2 = const_cast<const dsql_nod**>(into_out->nod_arg);
			dsql_nod** ptr = into_in->nod_arg;
			
			for (const dsql_nod* const* const end = ptr + into_in->nod_count; ptr < end; ptr++) 
				*ptr2++ = PASS1_node(request, *ptr, proc_flag);

			if (input->nod_arg[e_flp_action]) 
				{
				// CVC: Let's add the ability to BREAK the for_select same as the while,
				// but only if the command is FOR SELECT, otherwise we have singular SELECT
				request->loopLevel++;
				node->nod_arg[e_flp_label] = pass1_label(request, input);
				node->nod_arg[e_flp_action] =
					PASS1_statement(request, input->nod_arg[e_flp_action], proc_flag);
				request->loopLevel--;
				request->labels.pop();
				}

			if (cursor) 
				{
				request->cursorNumber--;
				request->cursors.pop();
				}

			if (input->nod_flags & NOD_SINGLETON_SELECT) 
				node = pass1_savepoint(request, node);
			break;
			}

		case nod_get_segment:
		case nod_put_segment:
			pass1_blob(request, input);
			return input;

		case nod_if:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_if_condition] =
				PASS1_node(request, input->nod_arg[e_if_condition], proc_flag);
			node->nod_arg[e_if_true] =
				PASS1_statement(request, input->nod_arg[e_if_true], proc_flag);
				
			if (input->nod_arg[e_if_false]) 
				node->nod_arg[e_if_false] = PASS1_statement(request, input->nod_arg[e_if_false], proc_flag);
			else
				node->nod_arg[e_if_false] = NULL;
			break;

		case nod_exception_stmt:
			node = input;
			/* if exception value is defined, pass value node */
			
			if (input->nod_arg[e_xcps_msg])
				node->nod_arg[e_xcps_msg] = PASS1_node(request, input->nod_arg[e_xcps_msg], proc_flag);
			else
				node->nod_arg[e_xcps_msg] = 0;

			return pass1_savepoint(request, node);

		case nod_insert:
			node = pass1_savepoint(request, pass1_insert(request, input));
			break;

		case nod_block:
			if (input->nod_arg[e_blk_errs])
				request->req_error_handlers++;
			else if (input->nod_arg[e_blk_action]) {
				input->nod_count = 1;
				if (!request->req_error_handlers)
					input->nod_type = nod_list;
			}
			else {
				input->nod_count = 0;
				input->nod_type = nod_list;
			}

		case nod_list:
			{
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			const dsql_nod** ptr2 = const_cast<const dsql_nod**>(node->nod_arg);
			dsql_nod* const* ptr = input->nod_arg;
			
			for (const dsql_nod* const* const end = ptr + input->nod_count; ptr < end; ptr++)
				{
				if ((*ptr)->nod_type == nod_assign)
					*ptr2++ = PASS1_node(request, *ptr, proc_flag);
				else
					*ptr2++ = PASS1_statement(request, *ptr, proc_flag);
				}
				
			if (input->nod_type == nod_block && input->nod_arg[e_blk_errs])
				request->req_error_handlers--;
				
			return node;
			}

		case nod_on_error:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_err_errs] = input->nod_arg[e_err_errs];
			node->nod_arg[e_err_action] =
				PASS1_statement(request, input->nod_arg[e_err_action], proc_flag);
			return node;

		case nod_post:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_pst_event] =
				PASS1_node(request, input->nod_arg[e_pst_event], proc_flag);
			node->nod_arg[e_pst_argument] =
				PASS1_node(request, input->nod_arg[e_pst_argument], proc_flag);
			return node;

		case nod_exec_sql:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_exec_sql_stmnt] =
				PASS1_node(request, input->nod_arg[e_exec_sql_stmnt], proc_flag);
			return pass1_savepoint(request, node);

		case nod_exec_into:
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_exec_into_stmnt] =
				PASS1_node(request, input->nod_arg[e_exec_into_stmnt], proc_flag);
				
			if (input->nod_arg[e_exec_into_block]) 
				{
				request->loopLevel++;
				node->nod_arg[e_exec_into_label] = pass1_label(request, input);
				node->nod_arg[e_exec_into_block] =
					PASS1_statement(request, input->nod_arg[e_exec_into_block], proc_flag);
				request->loopLevel--;
				request->labels.pop();
				}

			node->nod_arg[e_exec_into_list] =
				PASS1_node(request, input->nod_arg[e_exec_into_list], proc_flag);
			return pass1_savepoint(request, node);

		case nod_rollback:
			request->req_type = REQ_ROLLBACK;
			return input;

		case nod_exit:
			return input;

		case nod_breakleave:
			if (!request->loopLevel)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_token_err,	// Token unknown 
					isc_arg_gds, isc_random, isc_arg_string, "BREAK/LEAVE", 0);
			input->nod_arg[e_breakleave_label] = pass1_label(request, input);
			return input;

		case nod_return:
			if (request->flags & REQ_trigger)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_token_err,	// Token unknown 
						isc_arg_gds, isc_random, isc_arg_string, "SUSPEND", 0);
			input->nod_arg[e_rtn_procedure] = 
				request->ddlNode ? request->ddlNode : request->execBlockNode;
			return input;

		case nod_select:
			{
			node = PASS1_rse(request, input->nod_arg[e_select_expr], input->nod_arg[e_select_lock]);

			if (input->nod_arg[e_select_update]) {
				request->req_type = REQ_SELECT_UPD;
				request->flags |= REQ_no_batch;
				break;
			}
			/*
			** If there is a union without ALL or order by or a select distinct 
			** buffering is OK even if stored procedure occurs in the select
			** list. In these cases all of stored procedure is executed under
			** savepoint for open cursor.
			*/
			if (node->nod_arg[e_rse_sort] || node->nod_arg[e_rse_reduced])
			{
				request->flags &= ~REQ_no_batch;
			}

			break;
			}

		case nod_trans:
			request->req_type = REQ_START_TRANS;
			return input;

		case nod_update:
			node = pass1_savepoint(request, pass1_update(request, input));
			break;

		case nod_while:
			{
			node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
			node->nod_arg[e_while_cond] =
				PASS1_node(request, input->nod_arg[e_while_cond], proc_flag);

			/* CVC: loop numbers should be incremented before analyzing the body
			to preserve nesting <==> increasing level number. */
			request->loopLevel++;
			node->nod_arg[e_while_label] = pass1_label(request, input);
			node->nod_arg[e_while_action] =
				PASS1_statement(request, input->nod_arg[e_while_action], proc_flag);
			request->loopLevel--;
			request->labels.pop();
			}
			break;

		case nod_abort:
		case nod_exception:
		case nod_sqlcode:
		case nod_gdscode:
			return input;
			
		case nod_user_savepoint:
			if (request->flags & REQ_procedure)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_token_err,	// Token unknown 
						isc_arg_gds, isc_random, isc_arg_string, "SAVEPOINT", 0);
			request->req_type = REQ_SAVEPOINT;	
			return input;

		case nod_release_savepoint:
			if (request->flags & REQ_procedure)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_token_err,	// Token unknown 
						isc_arg_gds, isc_random, isc_arg_string, "RELEASE", 0);
			request->req_type = REQ_SAVEPOINT;	
			return input;

		case nod_undo_savepoint:
			if (request->flags & REQ_procedure)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_token_err,	// Token unknown 
						isc_arg_gds, isc_random, isc_arg_string, "ROLLBACK", 0);
			request->req_type = REQ_SAVEPOINT;	
			return input;

		case nod_null:
			return NULL;

		case nod_set_generator:
			node = MAKE_node(request->threadData, input->nod_type, e_gen_id_count);
			node->nod_arg[e_gen_id_value] =
				PASS1_node(request, input->nod_arg[e_gen_id_value], proc_flag);
			node->nod_arg[e_gen_id_name] = input->nod_arg[e_gen_id_name];
			request->req_type = REQ_SET_GENERATOR;
			break;

		case nod_set_generator2:
			node = MAKE_node(request->threadData, input->nod_type, e_gen_id_count);
			node->nod_arg[e_gen_id_value] =
				PASS1_node(request, input->nod_arg[e_gen_id_value], proc_flag);
			node->nod_arg[e_gen_id_name] = input->nod_arg[e_gen_id_name];
			request->req_type = REQ_SET_GENERATOR;
			break;

		case nod_union:
			ERRD_post(isc_sqlerr, isc_arg_number, -901,
					isc_arg_gds, isc_dsql_command_err,
					isc_arg_gds, isc_union_err,	// union not supported 
					0);
			break;

		case nod_cursor:
			{
			// make sure the cursor doesn't exist
			pass1_cursor_name(request, (dsql_str*) input->nod_arg[e_cur_name], false);
			// temporarily hide unnecessary contexts and process our RSE
			//void* base_context = request->context.mark();
			Stack temp = request->cursors;
			Stack local;
			request->cursors = local;
			const dsql_nod* select = input->nod_arg[e_cur_rse];
			input->nod_arg[e_cur_rse] =
				PASS1_rse(request, select->nod_arg[e_select_expr],
					  select->nod_arg[e_select_lock]);
			local = request->context;
			request->context = temp;
			// assign number and store in the request stack
			input->nod_arg[e_cur_number] = (dsql_nod*) (IPTR) request->cursorNumber++;
			//LLS_PUSH(input, &request->cursors);
			request->cursors.push (input);
			}
			return input;

		case nod_cursor_open:
		case nod_cursor_close:
		case nod_cursor_fetch:
			// resolve the cursor
			input->nod_arg[e_cur_stmt_id] =
				pass1_cursor_name(request, (dsql_str*) input->nod_arg[e_cur_stmt_id], true);
			// process a seek node, if exists
			if (input->nod_arg[e_cur_stmt_seek]) {
				input->nod_arg[e_cur_stmt_seek] =
					PASS1_node(request, input->nod_arg[e_cur_stmt_seek], proc_flag);
			}
			// process an assignment node, if exists
			if (input->nod_arg[e_cur_stmt_into]) {
				input->nod_arg[e_cur_stmt_into] =
					PASS1_node(request, input->nod_arg[e_cur_stmt_into], proc_flag);
			}
			return input;

		default:
			ERRD_post(isc_sqlerr, isc_arg_number, -901,
					isc_arg_gds, isc_dsql_command_err,
					isc_arg_gds, isc_dsql_construct_err,	// Unsupported DSQL construct 
					0);
			break;
		}

// Finish off by cleaning up contexts 

	request->context.pop (base);

#ifdef DSQL_DEBUG
	if (DSQL_debug & 1) {
		dsql_trace("Node tree at DSQL pass1 exit:");
		DSQL_pretty(node, 0);
	}
#endif

	return node;
}


/**
  
 	aggregate_found
  
    @brief	Check for an aggregate expression in an 
 	rse select list.  It could be buried in 
 	an expression tree.
 

    @param request
    @param node

 **/
static bool aggregate_found( const CStatement* request, const dsql_nod* node)
{

	USHORT current_level = request->scopeLevel;
	USHORT deepest_level = 0;

	return aggregate_found2(request, node, &current_level, &deepest_level, false);
}


/**
  
 	aggregate_found2
  
    @brief	Check for an aggregate expression in an 
 	expression. It could be buried in an expression 
	tree and therefore call itselfs again. The level
	parameters (current_level & deepest_level) are used
	to see how deep we are with passing sub-queries
	(= scope_level).
 
 	field is true if a non-aggregate field reference is seen.
 

    @param request
    @param node
    @param current_level
    @param deepest_level
    @param ignore_sub_selects

 **/
static bool aggregate_found2(const CStatement* request, const dsql_nod* node,
								USHORT* current_level,
								USHORT* deepest_level, bool ignore_sub_selects)
{

	bool aggregate = false;

	switch (node->nod_type) {

		// handle the simple case of a straightforward aggregate

		case nod_agg_average:
		case nod_agg_average2:
		case nod_agg_total2:
		case nod_agg_max:
		case nod_agg_min:
		case nod_agg_total:
		case nod_agg_count:
			if (!ignore_sub_selects) {
				if (node->nod_count) {
					USHORT ldeepest_level = 0;
					// If we are already in a aggregate function don't search inside 
					// sub-selects and other aggregate-functions for the deepest field 
					// used else we would have a wrong deepest_level value.
					aggregate_found2(request, node->nod_arg[e_agg_function_expression], 
						current_level, &ldeepest_level, true);
					if (ldeepest_level == 0) {					 
						*deepest_level = *current_level;
					}
					else {
						*deepest_level = ldeepest_level;
					}
					// If the deepest_value is the same as the current scope_level
					// this an aggregate that belongs to the current context.
					if (*deepest_level == request->scopeLevel) {
						aggregate = true;
					}
					else {
						// Check also for a nested aggregate that could belong to this context
						aggregate |= aggregate_found2(request, node->nod_arg[e_agg_function_expression], 
							current_level, &ldeepest_level, false);
					}
				}
				else {
					// we have Count(*)
					if (request->scopeLevel == (int)(U_IPTR)node->nod_arg[e_agg_function_scope_level]) {
						aggregate = true;
					}
				}
			}
			return aggregate;

		case nod_field:
			{
				const dsql_ctx* lcontext =
					reinterpret_cast<dsql_ctx*>(node->nod_arg[e_fld_context]);
				if (*deepest_level < lcontext->ctx_scope_level) {
					*deepest_level = lcontext->ctx_scope_level;
				}
				return false;
			}

		case nod_alias:
				aggregate = aggregate_found2(request, node->nod_arg[e_alias_value], 
					current_level, deepest_level, ignore_sub_selects);
			return aggregate;

		case nod_derived_field:
			{
				// This is an derived table don't look further,
				// but don't forget to check for deepest scope_level.
				const USHORT lscope_level = (USHORT)(U_IPTR)node->nod_arg[e_derived_field_scope];
				if (*deepest_level < lscope_level) {
					*deepest_level = lscope_level;
				}
			}
			return aggregate;

		case nod_map:
			{
				const dsql_ctx* lcontext = reinterpret_cast<dsql_ctx*>(node->nod_arg[e_map_context]);
				if (lcontext->ctx_scope_level == request->scopeLevel) {
					return true;
				}
				else {
					const dsql_map* lmap = reinterpret_cast<dsql_map*>(node->nod_arg[e_map_map]);
					aggregate = aggregate_found2(request, lmap->map_node, current_level, 
						deepest_level, ignore_sub_selects);
					return aggregate;
				}
			}

			// for expressions in which an aggregate might
			// be buried, recursively check for one

		case nod_via:
			if (!ignore_sub_selects) {
				aggregate = aggregate_found2(request, node->nod_arg[e_via_rse], current_level, 
					deepest_level, ignore_sub_selects);
			}
			return aggregate;

		case nod_exists:
		case nod_singular:
			if (!ignore_sub_selects) {
				aggregate = aggregate_found2(request, node->nod_arg[0], current_level, 
					deepest_level, ignore_sub_selects);
			}
			return aggregate;

		case nod_aggregate:
			if (!ignore_sub_selects) {
				aggregate = aggregate_found2(request, node->nod_arg[e_agg_rse], current_level, 
					deepest_level, ignore_sub_selects);
			}
			return aggregate;

		case nod_rse:
			(*current_level)++;
			aggregate |= aggregate_found2(request, node->nod_arg[e_rse_streams], current_level, 
				deepest_level, ignore_sub_selects);
			if (node->nod_arg[e_rse_boolean]) {
				aggregate |= aggregate_found2(request, node->nod_arg[e_rse_boolean], 
					current_level, deepest_level, ignore_sub_selects);
			}
			if (node->nod_arg[e_rse_items]) {
				aggregate |= aggregate_found2(request, node->nod_arg[e_rse_items], 
					current_level, deepest_level, ignore_sub_selects);
			}
			(*current_level)--;
			return aggregate;	

		case nod_order:
			aggregate = aggregate_found2(request, node->nod_arg[e_order_field], current_level, 
				deepest_level, ignore_sub_selects);
			return aggregate;

		case nod_or:
		case nod_and:
		case nod_not:
		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_lss:
		case nod_leq:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_lss_any:
		case nod_leq_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_lss_all:
		case nod_leq_all:
		case nod_between:
		case nod_like:
		case nod_containing:
		case nod_starting:
		case nod_missing:
		case nod_add:
		case nod_add2:
		case nod_concatenate:
		case nod_divide:
		case nod_divide2:
		case nod_multiply:
		case nod_multiply2:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_subtract2:
		case nod_upcase:
		case nod_extract:
		case nod_coalesce:
		case nod_simple_case:
		case nod_searched_case:
		case nod_list:
		case nod_join:
		case nod_join_inner:
		case nod_join_left:
		case nod_join_right:
		case nod_join_full:
			{
			const dsql_nod* const* ptr = node->nod_arg;
			
			for (const dsql_nod* const* const end = ptr + node->nod_count; ptr < end; ++ptr)
				if (*ptr) 
					aggregate |= aggregate_found2(request, *ptr, current_level,
												  deepest_level, ignore_sub_selects);
				
			return aggregate;
			}

		case nod_cast:
		case nod_gen_id:
		case nod_gen_id2:
		case nod_udf:
			if (node->nod_count == 2) {
				return (aggregate_found2(request, node->nod_arg[1], current_level, 
					deepest_level, ignore_sub_selects));
			}
			else {
				return false;
			}

		case nod_constant:
			return false;

		case nod_relation:
			{
				const dsql_ctx* lrelation_context = reinterpret_cast<dsql_ctx*>(node->nod_arg[e_rel_context]);
				// Check if relation is a procedure
				if (lrelation_context->ctx_procedure) {
					// If input parameters exists check if a aggregate is buried inside
					if (lrelation_context->ctx_proc_inputs) {
						aggregate |= aggregate_found2(request, lrelation_context->ctx_proc_inputs, 
							current_level, deepest_level, ignore_sub_selects);
					}
				}
				return aggregate;
			}

		default:
			return false;
	}
}


/**
  
 	ambiguity
  
    @brief	Check for ambiguity in a field
   reference. The list with contexts where the
   field was found is checked and the necessary
   message is build from it.
 

    @param request
    @param node
    @param name
    @param ambiguous_contexts

 **/
static dsql_nod* ambiguity_check(CStatement* request, dsql_nod* node,
	dsql_str* name, Stack *ambiguous_contexts)
{
    // If there are no relations or only 1 there's no ambiguity, thus return.
    
    if (ambiguous_contexts->count() < 2)
		return node;

	TEXT buffer[1024];
	USHORT loop = 0;

	buffer[0] = 0;
	TEXT* b = buffer;
	TEXT* p = 0;

	FOR_STACK (dsql_ctx*, context, ambiguous_contexts)
		dsql_rel* relation = context->ctx_relation;
		Procedure* procedure = context->ctx_procedure;
		
        if (strlen(b) > (sizeof(buffer) - 50)) 
            break;

		// if this is the second loop add "and " before relation.
		
        if (++loop > 2)
            strcat(buffer, "and ");

		// Process relation when present.
		
		if (relation) 
			{
			if (!(relation->rel_flags & REL_view))
		        strcat(buffer, "table ");
			else
				strcat(buffer, "view ");
	        strcat(buffer, relation->rel_name);
			}
		else if (procedure) 
			{
			strcat(b, "procedure ");
			strcat(b, (const TEXT *)procedure->findName());
			}
		else
			{
			// When there's no relation and no procedure it's a derived table.
			strcat(b, "derived table ");
			if (context->ctx_alias)
				strcat(b, context->ctx_alias);
			}
			
		strcat(buffer, " ");
		
		if (!p)
			p = b + strlen(b);
	END_FOR
	
	if (p) 
		{
		*--p = 0;
		}

	if (request->req_client_dialect >= SQL_DIALECT_V6) 
		{
		if (node) 
			delete node;
		ERRD_post(isc_sqlerr, isc_arg_number, -204,
					isc_arg_gds, isc_dsql_ambiguous_field_name,
					isc_arg_string, buffer,
					isc_arg_string, ++p,
					isc_arg_gds, isc_random,
					isc_arg_string, name->str_data,
					0);
		}

	ERRD_post_warning(isc_sqlwarn, isc_arg_number, (SLONG) 204,
						isc_arg_warning, isc_dsql_ambiguous_field_name,
						isc_arg_string, buffer,
						isc_arg_string, ++p,
						isc_arg_gds, isc_random,
						isc_arg_string, name->str_data,
						0);

	return node;
}


/**
  
 	assign_fld_dtype_from_dsc
  
    @brief	Set a field's descriptor from a DSC
 	(If dsql_fld* is ever redefined this can be removed)
 

    @param field
    @param nod_desc

 **/
static void assign_fld_dtype_from_dsc( dsql_fld* field, const dsc* nod_desc)
{
	field->fld_dtype = nod_desc->dsc_dtype;
	field->fld_scale = nod_desc->dsc_scale;
	field->fld_sub_type = nod_desc->dsc_sub_type;
	field->fld_length = nod_desc->dsc_length;
	if (nod_desc->dsc_dtype <= dtype_any_text) {
		field->fld_collation_id = DSC_GET_COLLATE(nod_desc);
		field->fld_character_set_id = DSC_GET_CHARSET(nod_desc);
	}
	else if (nod_desc->dsc_dtype == dtype_blob)
		field->fld_character_set_id = nod_desc->dsc_scale;
}


/** 
	check_unique_fields_names

	check fields (params, variables, cursors etc) names against
	sorted array 
	if success, add them into array 
 **/
static void check_unique_fields_names(StrArray& names, const dsql_nod* fields)
{
	if (!fields)
		return;
	
	const dsql_nod* const* ptr = fields->nod_arg;
	const dsql_nod* const* const end = ptr + fields->nod_count;
	const dsql_nod* temp;
	dsql_fld* field;
	const dsql_str* str;
	const char* name;

	for (; ptr < end; ptr++) {
		switch ((*ptr)->nod_type) {
			case nod_def_field:
				field = (dsql_fld*) (*ptr)->nod_arg[e_dfl_field];
				DEV_BLKCHK(field, dsql_type_fld);
				name = field->fld_name;
			break;

			case nod_param_val:
				temp = (*ptr)->nod_arg[e_prm_val_fld];
				field = (dsql_fld*) temp->nod_arg[e_dfl_field];
				DEV_BLKCHK(field, dsql_type_fld);
				name = field->fld_name;
			break;

			case nod_cursor:
				str = (dsql_str*) (*ptr)->nod_arg[e_cur_name];
				DEV_BLKCHK(str, dsql_type_str);
				name = str->str_data;
			break;

			default:
				fb_assert(false);
		}

		int pos;
		if (!names.find(name, pos))
			names.add(name);
		else {
			ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) -637,
					  isc_arg_gds, isc_dsql_duplicate_spec,
					  isc_arg_string, name, 0);
		}
	}
}


/**
  
 	compose
  
    @brief	Compose two booleans.
 

    @param expr1
    @param expr2
    @param operator_

 **/
static dsql_nod* compose(CStatement* request,  dsql_nod* expr1, dsql_nod* expr2, NOD_TYPE dsql_operator)
{
	if (!expr1)
		return expr2;

	if (!expr2)
		return expr1;

	dsql_nod* node = MAKE_node(request->threadData, dsql_operator, 2);
	node->nod_arg[0] = expr1;
	node->nod_arg[1] = expr2;

	return node;
}


/**
  
 	explode_outputs
  
    @brief	Generate a parameter list to correspond to procedure outputs.
 

    @param request
    @param procedure

 **/
static dsql_nod* explode_outputs( CStatement* request, Procedure* procedure)
{
	const SSHORT count = procedure->findOutputCount();
	dsql_nod* node = MAKE_node(request->threadData, nod_list, count);
	dsql_nod** ptr = node->nod_arg;
	
	for (ProcParam* param = procedure->findOutputParameters(); param; param = param->findNext(), ptr++)
		{
		dsql_nod* p_node = MAKE_node(request->threadData, nod_parameter, e_par_count);
		*ptr = p_node;
		p_node->nod_count = 0;
		par* parameter = request->makeParameter (request->receiveMessage, true, true, 0);
		p_node->nod_arg[e_par_parameter] = (dsql_nod*) parameter;
		parameter->par_desc = param->findDescriptor();
		parameter->par_name = parameter->par_alias = (const TEXT *)param->fld_name;
		parameter->par_rel_name = (const TEXT *)procedure->findName();
		parameter->par_owner_name = (const TEXT *)procedure->findOwner();
		}

	return node;
}


/**
  
 	field_error	
  
    @brief	Report a field parsing recognition error.
 

    @param qualifier_name
    @param field_name
    @param flawed_node

 **/
static void field_error(const TEXT* qualifier_name, const TEXT* field_name,
	const dsql_nod* flawed_node)
{
    TEXT field_string[64], linecol[64];

	if (qualifier_name) {
		sprintf(field_string, "%.31s.%.31s", qualifier_name,
				field_name ? field_name : "*");
		field_name = field_string;
	}

    if (flawed_node)
        sprintf (linecol, "At line %d, column %d.",
                 (int) flawed_node->nod_line, (int) flawed_node->nod_column);
    else
        sprintf (linecol, "At unknown line and column.");

	if (field_name)
		ERRD_post(isc_sqlerr, isc_arg_number, -206,
				  isc_arg_gds, isc_dsql_field_err,
				  isc_arg_gds, isc_random, isc_arg_string, field_name,
                  isc_arg_gds, isc_random,
                  isc_arg_string, linecol,
                  0);
	else
		ERRD_post(isc_sqlerr, isc_arg_number, -206,
				  isc_arg_gds, isc_dsql_field_err, 
                  isc_arg_gds, isc_random,
                  isc_arg_string, linecol,
                  0);
}


/**
  
 	find_dbkey
  
    @brief	Find dbkey for named relation in request's saved dbkeys.
 

    @param request
    @param relation_name

 **/
static par* find_dbkey(const CStatement* request, const dsql_nod* relation_name)
{
	dsql_msg* message = request->receiveMessage;
	par* candidate = NULL;
	dsql_str* rel_name = (dsql_str*) relation_name->nod_arg[e_rln_name];

	for (par* parameter = message->msg_parameters; parameter;
		 parameter = parameter->par_next)
		{
		const dsql_ctx* context = parameter->par_dbkey_ctx;
		
		if (context) 
			{
			dsql_rel* relation = context->ctx_relation;
			
			if (!strcmp(reinterpret_cast<const char*>(rel_name->str_data), relation->rel_name))
				{
				if (candidate)
					return NULL;
				candidate = parameter;
				}
			}
		}
		
	return candidate;
}


/**
  
 	find_record_version
  
    @brief	Find record version for relation in request's saved record version
 

    @param request
    @param relation_name

 **/
static par* find_record_version(const CStatement* request, const dsql_nod* relation_name)
{
	dsql_msg* message = request->receiveMessage;
	par* candidate = NULL;
	dsql_str* rel_name = (dsql_str*) relation_name->nod_arg[e_rln_name];

	for (par* parameter = message->msg_parameters; parameter;
		 parameter = parameter->par_next)
		{
		const dsql_ctx* context = parameter->par_rec_version_ctx;
		
		if (context) 
			{
			dsql_rel* relation = context->ctx_relation;
			
			if (!strcmp(reinterpret_cast<const char*>(rel_name->str_data), relation->rel_name))
				{
				if (candidate)
					return NULL;
				candidate = parameter;
				}
			}
		}
		
	return candidate;
}


/**
  
 	invalid_reference
  
    @brief	Validate that an expanded field / context
 	pair is in a specified list.  Thus is used
 	in one instance to check that a simple field selected 
 	through a grouping rse is a grouping field - 
 	thus a valid field reference.  For the sake of
       argument, we'll match qualified to unqualified
 	reference, but qualified reference must match
 	completely. 
 
 	A list element containing a simple CAST for collation purposes
 	is allowed.
 

    @param context
    @param node
    @param list
    @param inside_map

 **/
static bool invalid_reference(const dsql_ctx* context, const dsql_nod* node,
							  const dsql_nod* list, bool inside_own_map,
							  bool inside_higher_map)
{
	if (node == NULL) 
		return false;

	bool invalid = false;

	if (list) 
		{
		// Check if this node (with ignoring of CASTs) appear also 
		// in the list of group by. If yes then it's allowed 
		
		const dsql_nod* const* ptr = list->nod_arg;
		
		for (const dsql_nod* const* const end = ptr + list->nod_count; ptr < end; ptr++) 
			if (node_match(node, *ptr, true))
				return false;
		}

	switch (node->nod_type) 
		{
		default:
			fb_assert(false);
			// FALLINTO 

		case nod_map: 
			{
			const dsql_ctx* lcontext = reinterpret_cast<dsql_ctx*>(node->nod_arg[e_map_context]);
			const dsql_map* lmap = reinterpret_cast<dsql_map*>(node->nod_arg[e_map_map]);
			if (lcontext->ctx_scope_level == context->ctx_scope_level) {
				invalid |= invalid_reference(context, lmap->map_node, list, true, false);
			}
			else {
				bool linside_higher_map = lcontext->ctx_scope_level > context->ctx_scope_level;
				invalid |= invalid_reference(context, lmap->map_node, list, false, linside_higher_map);
			}
			break;
			}

		case nod_field:
			{
			const dsql_ctx* lcontext = reinterpret_cast<dsql_ctx*>(node->nod_arg[e_fld_context]);

			// Wouldn't it be better to call a error from this 
			// point where return is TRUE. Then we could give 
			// the fieldname that's making the trouble

			// If we come here then this Field is used inside a 
			// aggregate-function. The ctx_scope_level gives the 
			// info how deep the context is inside the request.

			// If the context-scope-level from this field is 
			// lower or the sameas the scope-level from the 
			// given context then it is an invalid field 
			if (lcontext->ctx_scope_level == context->ctx_scope_level) {
				// Return TRUE (invalid) if this Field isn't inside 
				// the GROUP BY clause , that should already been 
				// seen in the match_node above 
				invalid = true;
			}
			break;
			}

		case nod_agg_count:
		case nod_agg_average:
		case nod_agg_max:
		case nod_agg_min:
		case nod_agg_total:
		case nod_agg_average2:
		case nod_agg_total2:
			if (!inside_own_map) 
				{
				// We are not in an aggregate from the same scope_level so 
				// check for valid fields inside this aggregate
				if (node->nod_count) 
					{
					invalid |= invalid_reference(context, node->nod_arg[0], list, 
						inside_own_map, inside_higher_map);
					}
				} 
				
			if (!inside_higher_map) {
				if (node->nod_count) {
					// If there's another aggregate with the same scope_level or
					// an higher one then it's a invalid aggregate, because
					// aggregate-functions from the same context can't 
					// be part of each other. 
					if (pass1_found_aggregate(node->nod_arg[e_agg_function_expression], context->ctx_scope_level,
							FIELD_MATCH_TYPE_EQUAL, true)) 
					{
						ERRD_post(isc_sqlerr, isc_arg_number, -104,
							isc_arg_gds, isc_dsql_agg_nested_err, 0);
						// Nested aggregate functions are not allowed
					}
				}
			}			
			break;
			
		case nod_gen_id:
		case nod_gen_id2:
		case nod_cast:
		case nod_udf:
			// If there are no arguments given to the UDF then it's always valid
			if (node->nod_count == 2) {
				invalid |= invalid_reference(context, node->nod_arg[1], list, 
					inside_own_map, inside_higher_map);
			}
			break;

		case nod_via:
		case nod_exists:     
		case nod_singular:   
			{
				const dsql_nod* const* ptr = node->nod_arg;
				for (const dsql_nod* const* const end = ptr + node->nod_count;
					ptr < end; ptr++) 
				{
					invalid |= invalid_reference(context, *ptr, list, 
						inside_own_map, inside_higher_map);
				}
				break;
			}

		case nod_order:
			invalid |= invalid_reference(context, node->nod_arg[e_order_field], list, 
				inside_own_map, inside_higher_map);
			break;

		case nod_coalesce:
		case nod_simple_case:
		case nod_searched_case:
		case nod_add:
		case nod_add2:
		case nod_concatenate:
		case nod_divide:
		case nod_divide2:
		case nod_multiply:
		case nod_multiply2:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_subtract2:
		case nod_upcase:
		case nod_extract:
		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_leq:
		case nod_lss:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_leq_any:
		case nod_lss_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_leq_all:
		case nod_lss_all:
		case nod_between:
		case nod_like:
		case nod_missing:
		case nod_and:
		case nod_or:
		case nod_any:
		case nod_ansi_any:
		case nod_ansi_all:
		case nod_not:
		case nod_unique:
		case nod_containing:
		case nod_starting:
		case nod_rse:
		case nod_join:
		case nod_join_inner:
		case nod_join_left:
		case nod_join_right:
		case nod_join_full:
		case nod_list:
			{
				const dsql_nod* const* ptr = node->nod_arg;
				for (const dsql_nod* const* const end = ptr + node->nod_count;
					ptr < end; ptr++) 
				{
					invalid |= invalid_reference(context, *ptr, list, inside_own_map, inside_higher_map);
				}
			break;
			}

		case nod_alias:
				invalid |= invalid_reference(context, node->nod_arg[e_alias_value], 
					list, inside_own_map, inside_higher_map);
			break;

		case nod_derived_field:
			{
				const USHORT lscope_level = (USHORT)(U_IPTR)node->nod_arg[e_derived_field_scope];
				if (lscope_level == context->ctx_scope_level) {
					invalid |= true;
				}
				else if (context->ctx_scope_level < lscope_level) {
					invalid |= invalid_reference(context, node->nod_arg[e_derived_field_value],
						list, inside_own_map, inside_higher_map);
				}
			}
			break;

		case nod_aggregate:
			invalid |= invalid_reference(context, node->nod_arg[e_agg_rse], 
				list, inside_own_map, inside_higher_map);
			break;

		case nod_relation:
			{
				const dsql_ctx* lrelation_context = reinterpret_cast<dsql_ctx*>(node->nod_arg[e_rel_context]);
				// Check if relation is a procedure
				if (lrelation_context->ctx_procedure) {
					// If input parameters exists check if the parameters are valid
					if (lrelation_context->ctx_proc_inputs) {
						invalid |= invalid_reference(context, lrelation_context->ctx_proc_inputs, 
							list, inside_own_map, inside_higher_map);
					}
				}
				break;
			}

		case nod_variable:
		case nod_constant:
		case nod_parameter:
		case nod_null:
		case nod_current_date:
		case nod_current_time:
		case nod_current_timestamp:
		case nod_user_name:
		case nod_current_role:
		case nod_internal_info:
		case nod_dom_value:
		case nod_dbkey:
		case nod_derived_table:
		case nod_plan_expr:
			return false;
		}

	return invalid;
}


/**
  
 	node_match
  
    @brief	Compare two nodes for equality of value.
 
   [2002-08-04]--- Arno Brinkman
 	If ignore_map_cast is true and the node1 is
   type nod_cast or nod_map then node_match is
   calling itselfs again with the node1 
   CASTs source or map->node.
   This is for allow CAST to other datatypes
   without complaining that it's an unknown
   column reference. (Aggeregate functions) 
 

    @param node1
    @param node2
    @param ignore_map_cast

 **/
static bool node_match(const dsql_nod* node1, const dsql_nod* node2,
	bool ignore_map_cast)
{
	DEV_BLKCHK(node1, dsql_type_nod);
	DEV_BLKCHK(node2, dsql_type_nod);

	if ((!node1) && (!node2)) {
		return true;
	}

	if ((!node1) || (!node2)) {
		return false;
	}

	if (ignore_map_cast && node1->nod_type == nod_cast)	{
		// If node2 is also cast and same type continue with both sources.
		if (node2->nod_type == nod_cast && 
			node1->nod_desc.dsc_dtype == node2->nod_desc.dsc_dtype &&
			node1->nod_desc.dsc_scale == node2->nod_desc.dsc_scale &&
			node1->nod_desc.dsc_length == node2->nod_desc.dsc_length &&
			node1->nod_desc.dsc_sub_type == node2->nod_desc.dsc_sub_type)
		{
			return node_match(node1->nod_arg[e_cast_source], node2->nod_arg[e_cast_source], ignore_map_cast);
		} 
		else {
			return node_match(node1->nod_arg[e_cast_source], node2, ignore_map_cast);
		}
	}

	if (ignore_map_cast && node1->nod_type == nod_map) {
		const dsql_map* map1 = (dsql_map*)node1->nod_arg[e_map_map];
		DEV_BLKCHK(map1, dsql_type_map);
		if (node2->nod_type == nod_map) {
			const dsql_map* map2 = (dsql_map*)node2->nod_arg[e_map_map];
			DEV_BLKCHK(map2, dsql_type_map);
			if (node1->nod_arg[e_map_context] != node2->nod_arg[e_map_context]) {
				return false;
			}
			return node_match(map1->map_node, map2->map_node, ignore_map_cast);
		}
		else {
			return node_match(map1->map_node, node2, ignore_map_cast);
		}
	}

	// We don't care about the alias itself but only about its field.
	if ((node1->nod_type == nod_alias) && (node2->nod_type == nod_alias)) 
	{
		return node_match(node1->nod_arg[e_alias_value],
			node2->nod_arg[e_alias_value], ignore_map_cast);
	}
	else if (node1->nod_type == nod_alias) 
	{
		return node_match(node1->nod_arg[e_alias_value], node2, ignore_map_cast);
	}
	else if (node2->nod_type == nod_alias) 
	{
		return node_match(node1, node2->nod_arg[e_alias_value], ignore_map_cast);
	}

	// Handle derived fields.
	if ((node1->nod_type == nod_derived_field) && (node2->nod_type == nod_derived_field)) 
	{
		const USHORT scope_level1 = (USHORT)(U_IPTR)node1->nod_arg[e_derived_field_scope];
		const USHORT scope_level2 = (USHORT)(U_IPTR)node2->nod_arg[e_derived_field_scope];
		if (scope_level1 != scope_level2)
			return false;

		dsql_str* alias1 = (dsql_str*) node1->nod_arg[e_derived_field_name];
		dsql_str* alias2 = (dsql_str*) node2->nod_arg[e_derived_field_name];
		if (strcmp(alias1->str_data, alias2->str_data))
			return false;

		return node_match(node1->nod_arg[e_derived_field_value],
			node2->nod_arg[e_derived_field_value], ignore_map_cast);
	}
	else if (node1->nod_type == nod_derived_field) 
	{
		return node_match(node1->nod_arg[e_derived_field_value], node2, ignore_map_cast);
	}
	else if (node2->nod_type == nod_derived_field) 
	{
		return node_match(node1, node2->nod_arg[e_derived_field_value], ignore_map_cast);
	}

	if ((node1->nod_type != node2->nod_type) || (node1->nod_count != node2->nod_count)) {
		return false;
	}

	// This is to get rid of assertion failures when trying
	// to node_match nod_aggregate's children. This was happening because not
	// all of the children are of type "dsql_nod". Pointer to the first child
	// (argument) is actually a pointer to context structure.
	// To compare two nodes of type nod_aggregate we need first to see if they
	// both refer to same context structure. If they do not they are different
	// nodes, if they point to the same context they are the same (because
	// nod_aggregate is created for an rse that have aggregate expression, 
	// group by or having clause and each rse has its own context). But just in
	// case we compare two other subtrees.

	if (node1->nod_type == nod_aggregate) 
		{
		if (node1->nod_arg[e_agg_context] != node2->nod_arg[e_agg_context])
			return false;

		return node_match(	node1->nod_arg[e_agg_group],
							node2->nod_arg[e_agg_group], ignore_map_cast) &&
				node_match(	node1->nod_arg[e_agg_rse],
							node2->nod_arg[e_agg_rse], ignore_map_cast);
		}

	if (node1->nod_type == nod_relation) 
		{
		if (node1->nod_arg[e_rel_context] != node2->nod_arg[e_rel_context]) 
			return false;
		return true;
		}

	if (node1->nod_type == nod_field) {
		if (node1->nod_arg[e_fld_field] != node2->nod_arg[e_fld_field] ||
			node1->nod_arg[e_fld_context] != node2->nod_arg[e_fld_context]) {
			return false;
		}
		if (node1->nod_arg[e_fld_indices] || node2->nod_arg[e_fld_indices]) {
			return node_match(node1->nod_arg[e_fld_indices],
							  node2->nod_arg[e_fld_indices], ignore_map_cast);
		}
		return true;
	}

	if (node1->nod_type == nod_constant) 
		{
		if (node1->nod_desc.dsc_dtype != node2->nod_desc.dsc_dtype ||
			node1->nod_desc.dsc_length != node2->nod_desc.dsc_length ||
			node1->nod_desc.dsc_scale != node2->nod_desc.dsc_scale)
			return false;

		const UCHAR* p1 = node1->nod_desc.dsc_address;
		const UCHAR* p2 = node2->nod_desc.dsc_address;
		
		for (USHORT l = node1->nod_desc.dsc_length; l > 0; l--) 
			if (*p1++ != *p2++)
				return false;
				
		return true;
		}

	if (node1->nod_type == nod_map) 
		{
		const dsql_map* map1 = (dsql_map*)node1->nod_arg[e_map_map];
		const dsql_map* map2 = (dsql_map*)node2->nod_arg[e_map_map];
		DEV_BLKCHK(map1, dsql_type_map);
		DEV_BLKCHK(map2, dsql_type_map);
		return node_match(map1->map_node, map2->map_node, ignore_map_cast);
		}

	if ((node1->nod_type == nod_gen_id)		||
		(node1->nod_type == nod_gen_id2)	||
		(node1->nod_type == nod_udf)		||
		(node1->nod_type == nod_cast)) 
		{
		if (node1->nod_arg[0] != node2->nod_arg[0]) 
			return false;

		if (node1->nod_count == 2) 
			return node_match(node1->nod_arg[1], node2->nod_arg[1], ignore_map_cast);

		return true;
		}

	if ((node1->nod_type == nod_agg_count)		||
		(node1->nod_type == nod_agg_total)		||
		(node1->nod_type == nod_agg_total2)		||
		(node1->nod_type == nod_agg_average2)	||
		(node1->nod_type == nod_agg_average))
		if ((node1->nod_flags & NOD_AGG_DISTINCT) !=
			(node2->nod_flags & NOD_AGG_DISTINCT)) 
			return false;

	if (node1->nod_type == nod_variable) 
		{
		const var* var1 = reinterpret_cast<var*>(node1->nod_arg[e_var_variable]);
		const var* var2 = reinterpret_cast<var*>(node2->nod_arg[e_var_variable]);
		DEV_BLKCHK(var1, dsql_type_var);
		DEV_BLKCHK(var2, dsql_type_var);
		
		if ((strcmp(var1->var_name, var2->var_name))					||
			(var1->var_field != var2->var_field)						||
			(var1->var_variable_number != var2->var_variable_number)	||
			(var1->var_msg_item != var2->var_msg_item)					||
			(var1->var_msg_number != var2->var_msg_number)) 
			return false;

		return true;
		}

	if (node1->nod_type == nod_parameter) 
		{
		// Parameters are equal when there index is the same
		par* parameter1 = (par*) node1->nod_arg[e_par_parameter];
		par* parameter2 = (par*) node2->nod_arg[e_par_parameter];
		return (parameter1->par_index == parameter2->par_index);
		}

	const dsql_nod* const* ptr1 = node1->nod_arg;
	const dsql_nod* const* ptr2 = node2->nod_arg;

	for (const dsql_nod* const* const end = ptr1 + node1->nod_count; ptr1 < end; ptr1++, ptr2++) 
		if (!node_match(*ptr1, *ptr2, ignore_map_cast)) 
			return false;

	return true;
}


/**
  
 	pass1_any
  
    @brief	Compile a parsed request into something more interesting.
 

    @param request
    @param input
    @param ntype

 **/
static dsql_nod* pass1_any( CStatement* request, dsql_nod* input, NOD_TYPE ntype)
{
	DEV_BLKCHK(input, dsql_type_nod);

	// create a derived table representing our subquery
	dsql_nod* dt = MAKE_node(request->threadData, nod_derived_table, e_derived_table_count);
	// Ignore validation for columnames that must be exists for "user" derived tables.
	dt->nod_flags |= NOD_DT_IGNORE_COLUMN_CHECK;
	dt->nod_arg[e_derived_table_rse] = input->nod_arg[1];
	dsql_nod* from = MAKE_node(request->threadData, nod_list, 1);
	from->nod_arg[0] = dt;
	dsql_nod* query_spec = MAKE_node(request->threadData, nod_query_spec, e_qry_count);
	query_spec->nod_arg[e_qry_from] = from;
	dsql_nod* select_expr = MAKE_node(request->threadData, nod_select_expr, e_sel_count);
	select_expr->nod_arg[e_sel_query_spec] = query_spec;

	void* base = request->context.mark();
	dsql_nod* rse = PASS1_rse(request, select_expr, NULL);

	// create a conjunct to be injected
	dsql_nod* temp = MAKE_node(request->threadData, input->nod_type, 2);
	temp->nod_arg[0] = PASS1_node(request, input->nod_arg[0], false);
	temp->nod_arg[1] = rse->nod_arg[e_rse_items]->nod_arg[0];

	rse->nod_arg[e_rse_boolean] = temp;

	// create output node
	dsql_nod* node = MAKE_node(request->threadData, ntype, 1);
	node->nod_arg[0] = rse;

	request->context.pop (base);

	return node;
}


/**
  
 	pass1_blob
  
    @brief	Process a blob get or put segment.
 

    @param request
    @param input

 **/
static void pass1_blob( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	//TSQL tdsql = GET_THREAD_DATA;

	PASS1_make_context(request, input->nod_arg[e_blb_relation]);
	dsql_nod* field = pass1_field(request, input->nod_arg[e_blb_field], false, NULL);
	if (field->nod_desc.dsc_dtype != dtype_blob)
		ERRD_post(isc_sqlerr, isc_arg_number, -206,
				  isc_arg_gds, isc_dsql_blob_err, 0);

	request->req_type = (input->nod_type == nod_get_segment) ?
		REQ_GET_SEGMENT : REQ_PUT_SEGMENT;
		 
	dsql_blb* blob = FB_NEW(*request->threadData->tsql_default) dsql_blb;
	request->blob = blob;
	blob->blb_field = field;
	blob->blb_open_in_msg = request->sendMessage;
	blob->blb_open_out_msg = FB_NEW(*request->threadData->tsql_default) dsql_msg;
	blob->blb_segment_msg = request->receiveMessage;

// Create a parameter for the blob segment 

	par* parameter = request->makeParameter (blob->blb_segment_msg, true, true, 0);
	blob->blb_segment = parameter;
	parameter->par_desc.dsc_dtype = dtype_text;
	parameter->par_desc.dsc_ttype = ttype_binary;
	parameter->par_desc.dsc_length =
		((dsql_fld*) field->nod_arg[e_fld_field])->fld_seg_length;

/* The Null indicator is used to pass back the segment length,
 * set DSC_nullable so that the SQL_type is set to SQL_TEXT+1 instead
 * of SQL_TEXT.
 */
	if (input->nod_type == nod_get_segment)
		parameter->par_desc.dsc_flags |= DSC_nullable;

// Create a parameter for the blob id 

	dsql_msg* temp_msg = (input->nod_type == nod_get_segment) ?
		blob->blb_open_in_msg : blob->blb_open_out_msg;
	blob->blb_blob_id = parameter = request->makeParameter (temp_msg, true, true, 0);
	parameter->par_desc = field->nod_desc;
	parameter->par_desc.dsc_dtype = dtype_quad;
	parameter->par_desc.dsc_scale = 0;

	dsql_nod* list = input->nod_arg[e_blb_filter];
	if (list) {
		if (list->nod_arg[0]) {
			blob->blb_from = PASS1_node(request, list->nod_arg[0], false);
		}
		if (list->nod_arg[1]) {
			blob->blb_to = PASS1_node(request, list->nod_arg[1], false);
		}
	}
	if (!blob->blb_from) {
		blob->blb_from = MAKE_constant(request->threadData, (dsql_str*) 0, CONSTANT_SLONG);
	}
	if (!blob->blb_to) {
		blob->blb_to = MAKE_constant(request->threadData, (dsql_str*) 0, CONSTANT_SLONG);
	}

	for (parameter = blob->blb_open_in_msg->msg_parameters; parameter;
		 parameter = parameter->par_next)
	{
		if (parameter->par_index >
			((input->nod_type == nod_get_segment) ? 1 : 0)) 
		{
			parameter->par_desc.dsc_dtype = dtype_short;
			parameter->par_desc.dsc_scale = 0;
			parameter->par_desc.dsc_length = sizeof(SSHORT);
		}
	}
}


/**
  
 	pass1_coalesce
  
    @brief	Handle a reference to a coalesce function.

	COALESCE(expr-1, expr-2 [, expr-n])
	is the same as :
	CASE WHEN (expr-1 IS NULL) THEN expr-2 ELSE expr-1 END

    @param request
    @param input
    @param proc_flag

 **/
static dsql_nod* pass1_coalesce( CStatement* request, dsql_nod* input, bool proc_flag)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(input->nod_arg[0], dsql_type_nod);

	dsql_nod* node = MAKE_node(request->threadData, nod_coalesce, 2);

	// Pass list of arguments 2..n on stack and make a list from it
	
	Stack stack; // = NULL;
	pass1_put_args_on_stack(request, input->nod_arg[0], &stack, proc_flag);
	pass1_put_args_on_stack(request, input->nod_arg[1], &stack, proc_flag);
	node->nod_arg[0] = MAKE_list(request->threadData, &stack);

	// Parse the items again for the return values.
	// We can't copy else we get an 'context in use error' with sub-selects.
	
	stack.clear();
	pass1_put_args_on_stack(request, input->nod_arg[0], &stack, proc_flag);
	pass1_put_args_on_stack(request, input->nod_arg[1], &stack, proc_flag);
	node->nod_arg[1] = MAKE_list(request->threadData, &stack);

	// Set descriptor for output node
	
	MAKE_desc(request->threadData, &node->nod_desc, node, NULL);

	// Set parameter-types if parameters are there
	
	dsql_nod** ptr = node->nod_arg[0]->nod_arg;
	const dsql_nod* const* end = ptr + node->nod_arg[0]->nod_count;
	
	for (; ptr < end; ptr++) 
		set_parameter_type(request, *ptr, node, false);
	
	ptr = node->nod_arg[1]->nod_arg;
	end = ptr + node->nod_arg[1]->nod_count;
	
	for (; ptr < end; ptr++) 
		set_parameter_type(request, *ptr, node, false);

	return node;
}


/**
  
 	pass1_collate
  
    @brief	Turn a collate clause into a cast clause.
 	If the source is not already text, report an error.
 	(SQL 92: Section 13.1, pg 308, item 11)
 

    @param request
    @param sub1
    @param collation

 **/
static dsql_nod* pass1_collate( CStatement* request, dsql_nod* sub1,
	dsql_str* collation)
{
	//DEV_BLKCHK(sub1, dsql_type_nod);
	//DEV_BLKCHK(collation, dsql_type_str);

	dsql_nod* node = MAKE_node(request->threadData, nod_cast, e_cast_count);
	dsql_fld* field = new dsql_fld;
	node->nod_arg[e_cast_target] = (dsql_nod*) field;
	node->nod_arg[e_cast_source] = sub1;
	MAKE_desc(request->threadData, &sub1->nod_desc, sub1, NULL);
	
	if (sub1->nod_desc.dsc_dtype <= dtype_any_text) 
		{
		assign_fld_dtype_from_dsc(field, &sub1->nod_desc);
		field->fld_character_length = 0;
		if (sub1->nod_desc.dsc_dtype == dtype_varying)
			field->fld_length += sizeof(USHORT);
		}
	else 
		ERRD_post(isc_sqlerr, isc_arg_number, -204,
				  isc_arg_gds, isc_dsql_datatype_err,
				  isc_arg_gds, isc_collation_requires_text, 0);

	DDL_resolve_intl_type(request, field, collation);
	MAKE_desc_from_field(request->threadData, &node->nod_desc, field);
	return node;
}


/**
  
 	pass1_constant
  
    @brief	Turn an international string reference into internal
 	subtype ID.
 

    @param request
    @param constant

 **/
static dsql_nod* pass1_constant( CStatement* request, dsql_nod* constant)
{

	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(constant, dsql_type_nod);

	if (constant->nod_desc.dsc_dtype > dtype_any_text) {
		return constant;
	}

	dsql_str* string = (dsql_str*) constant->nod_arg[0];
	DEV_BLKCHK(string, dsql_type_str);
	if (!string || !string->str_charset) {
		return constant;
	}

	const CharSetContainer* resolved = request->findCharset (string->str_charset);
		//METD_get_charset(request, strlen(string->str_charset), string->str_charset);
		
	if (!resolved)
		{
		// character set name is not defined 
		ERRD_post(isc_sqlerr, isc_arg_number, -504, isc_arg_gds,
				  isc_charset_not_found, isc_arg_string, string->str_charset,
				  0);
		}

	if (request->tempCollationName)
		{
		const CharSetContainer* resolved_collation = request->findCollation (*(request->tempCollationName));

		if (!resolved_collation)
			{
			/* 
			   ** Specified collation not found 
			 */
			ERRD_post(isc_sqlerr, isc_arg_number, -204, isc_arg_gds,
					  isc_dsql_datatype_err, isc_arg_gds,
					  isc_collation_not_found, isc_arg_string,
					  request->tempCollationName->str_data, 0);
			}
		resolved = resolved_collation;
		}

	INTL_ASSIGN_TTYPE(&constant->nod_desc, resolved->id);

	return constant;
}


/**
  
 	pass1_cursor_context
  
    @brief	Turn a cursor reference into a record selection expression.
 

    @param request
    @param cursor
    @param relation_name

 **/
static dsql_ctx* pass1_cursor_context( CStatement* request, const dsql_nod* cursor,
	const dsql_nod* relation_name)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(cursor, dsql_type_nod);
	DEV_BLKCHK(relation_name, dsql_type_nod);

	dsql_str* rname = (dsql_str*) relation_name->nod_arg[e_rln_name];
	DEV_BLKCHK(rname, dsql_type_str);

	dsql_str* string = (dsql_str*) cursor->nod_arg[e_cur_name];
	DEV_BLKCHK(string, dsql_type_str);

	// this function must throw an error if no cursor was found
	dsql_nod* node = pass1_cursor_name(request, string, true);
	fb_assert(node);

	const dsql_nod* temp = node->nod_arg[e_cur_rse];
	if (temp->nod_type == nod_for_select)
		temp = temp->nod_arg[e_flp_select];
	else
		ERRD_post(isc_sqlerr, isc_arg_number,
				  -504, isc_arg_gds,
				  isc_dsql_cursor_err,
				  isc_arg_string, string->str_data,
				  isc_arg_string, "is not updatable",
				  0);

	if (temp->nod_arg[e_rse_reduced]) {
		// cursor with DISTINCT is not updatable
		ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 510,
				  isc_arg_gds, isc_dsql_cursor_update_err,
				  isc_arg_string, string->str_data, 0);
	}

	temp = temp->nod_arg[e_rse_streams];
	
	dsql_ctx* context = NULL;
	dsql_nod* const* ptr = temp->nod_arg;
	for (const dsql_nod* const* const end = ptr + temp->nod_count;
		 ptr < end; ptr++)
	{
		DEV_BLKCHK(*ptr, dsql_type_nod);
		dsql_nod* r_node = *ptr;
		if (r_node->nod_type == nod_relation) {
			dsql_ctx* candidate = (dsql_ctx*) r_node->nod_arg[e_rel_context];
			DEV_BLKCHK(candidate, dsql_type_ctx);
			dsql_rel* relation = candidate->ctx_relation;
			DEV_BLKCHK(rname, dsql_type_str);
			if (!strcmp
				(reinterpret_cast<const char*>(rname->str_data),
				 relation->rel_name))
			{
				if (context)
					ERRD_post(isc_sqlerr, isc_arg_number,
							  -504, isc_arg_gds,
							  isc_dsql_cursor_err,
							  isc_arg_string, string->str_data,
							  isc_arg_string, "is ambiguous",
							  0);
				else
					context = candidate;
			}
		}
		else if (r_node->nod_type == nod_aggregate) {
			// cursor with aggregation is not updatable
			ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 510,
					  isc_arg_gds, isc_dsql_cursor_update_err,
					  isc_arg_string, string->str_data, 0);
		}
		// note that nod_union and nod_join will cause the error below,
		// as well as derived tables. Some cases deserve fixing in the future
	}

	if (!context)
		ERRD_post(isc_sqlerr, isc_arg_number,
				  -504, isc_arg_gds,
				  isc_dsql_cursor_err,
				  isc_arg_string, string->str_data,
				  isc_arg_string, "cannot be matched",
				  0);

	return context;
}


/**
  
 	pass1_cursor_name
  
    @brief	Find a cursor.
 

    @param request
    @param string

 **/
static dsql_nod* pass1_cursor_name(CStatement* request, dsql_str* string,
	bool existance_flag)
{
	DEV_BLKCHK(string, dsql_type_str);
	dsql_nod *cursor = NULL;
	
	FOR_STACK (dsql_nod*, node, &request->cursors)
		dsql_str* cname = (dsql_str*) node->nod_arg[e_cur_name];
		if (!strcmp(string->str_data, cname->str_data)) {
			cursor = node;
			break;
		}
	END_FOR

	if ((!cursor && existance_flag) || (cursor && !existance_flag))
		ERRD_post(isc_sqlerr, isc_arg_number, -504,
				  isc_arg_gds, isc_dsql_cursor_err,
				  isc_arg_string, string->str_data,
				  isc_arg_string, (existance_flag) ? "is not defined" : "already exists",
				  0);

	return cursor;
}


/**
  
 	pass1_cursor_reference
  
    @brief	Turn a cursor reference into a record selection expression.
 

    @param request
    @param cursor
    @param relation_name

 **/
static dsql_nod* pass1_cursor_reference(CStatement* request,
										const dsql_nod* cursorNode, 
										dsql_nod* relation_name)
{
	// Lookup parent request 

	dsql_str* string = (dsql_str*) cursorNode->nod_arg[e_cur_name];
	JString cursorName = Cursor::getCursorName(*string);
	Cursor *cursor = request->attachment->findCursor(cursorName);
	
	if (!cursor) 
		ERRD_post(isc_sqlerr, isc_arg_number, -504,
				  isc_arg_gds, isc_dsql_cursor_err,
				  isc_arg_string, string->str_data,
				  isc_arg_string, "is not defined",
				  0);

	DStatement* parentInstance = cursor->statement;
	CStatement* parent = parentInstance->statement;
	
	// Verify that the cursor is appropriate and updatable 

	par* rv_source = find_record_version(parent, relation_name);
	par* source;
	
	if (parent->req_type != REQ_SELECT_UPD ||
		 !(source = find_dbkey(parent, relation_name)) ||
		 (!rv_source && !(request->database->dbb_flags & DBB_v3)))
		ERRD_post(isc_sqlerr, isc_arg_number, -510,
				  isc_arg_gds, isc_dsql_cursor_update_err, 0);

	request->cursorName = cursorName;
	request->parent = parent;
	request->parentDbkey = source;
	request->parentRecordVersion = rv_source;
	//request->sibling = parent->offspring;
	//parent->offspring = request;

	// Build record selection expression 

	dsql_nod* rse = MAKE_node(request->threadData, nod_rse, e_rse_count);
	dsql_nod* temp = MAKE_node(request->threadData, nod_list, 1);
	rse->nod_arg[e_rse_streams] = temp;
	dsql_nod* relation_node = pass1_relation(request, relation_name);
	temp->nod_arg[0] = relation_node;
	dsql_nod* node = MAKE_node(request->threadData, nod_eql, 2);
	rse->nod_arg[e_rse_boolean] = node;
	node->nod_arg[0] = temp = MAKE_node(request->threadData, nod_dbkey, 1);
	temp->nod_arg[0] = relation_node;

	node->nod_arg[1] = temp = MAKE_node(request->threadData, nod_parameter, e_par_count);
	temp->nod_count = 0;
	par* parameter = request->req_dbkey = request->makeParameter (request->sendMessage, false, false, 0);
	temp->nod_arg[e_par_parameter] = (dsql_nod*) parameter;
	parameter->par_desc = source->par_desc;

	// record version will be set only for V4 - for the parent select cursor 
	
	if (rv_source) 
		{
		node = MAKE_node(request->threadData, nod_eql, 2);
		node->nod_arg[0] = temp = MAKE_node(request->threadData, nod_rec_version, 1);
		temp->nod_arg[0] = relation_node;
		node->nod_arg[1] = temp = MAKE_node(request->threadData, nod_parameter, e_par_count);
		temp->nod_count = 0;
		parameter = request->recordVersion = request->makeParameter (request->sendMessage, false, false, 0);
		temp->nod_arg[e_par_parameter] = (dsql_nod*) parameter;
		parameter->par_desc = rv_source->par_desc;
		rse->nod_arg[e_rse_boolean] = compose(request, rse->nod_arg[e_rse_boolean], node, nod_and);
		}

	return rse;
}


/**
  
 	pass1_dbkey
  
    @brief	Resolve a dbkey to an available context.
 

    @param request
    @param input

 **/
static dsql_nod* pass1_dbkey( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	dsql_str* qualifier = (dsql_str*) input->nod_arg[0];
	
	if (!qualifier) 
		{
		DEV_BLKCHK(qualifier, dsql_type_str);
		// No qualifier, if only one context then use, else error 

		//Stack stack = request->context;
		//if (stack && !stack->lls_next)
		
		if (request->context.count() == 1)
			{
			dsql_ctx* context = (dsql_ctx*) request->context.peek();
			DEV_BLKCHK(context, dsql_type_ctx);
			dsql_nod* node = MAKE_node(request->threadData, nod_dbkey, 1);
			dsql_nod* rel_node = MAKE_node(request->threadData, nod_relation, e_rel_count);
			rel_node->nod_arg[0] = (dsql_nod*) context;
			node->nod_arg[0] = rel_node;
			return node;
			}
		}
	else 
		FOR_STACK (dsql_ctx*, context, &request->context)
			DEV_BLKCHK(context, dsql_type_ctx);
			if ((context->ctx_relation && 
				 qualifier->isEqual (context->ctx_relation->rel_name)) ||
				(context->ctx_alias && qualifier->isEqual (context->ctx_alias)))
				{
				dsql_nod* node = MAKE_node(request->threadData, nod_dbkey, 1);
				dsql_nod* rel_node = MAKE_node(request->threadData, nod_relation, e_rel_count);
				rel_node->nod_arg[0] = (dsql_nod*) context;
				node->nod_arg[0] = rel_node;
				return node;
				}
		END_FOR;

	// field unresolved 

	field_error(reinterpret_cast<const char*>(qualifier ? qualifier->str_data : 0),
				DB_KEY_STRING, input);

	return NULL;
}


/**
  
 	pass1_delete
  
    @brief	Process INSERT statement.
 

    @param request
    @param input

 **/
static dsql_nod* pass1_delete( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	const dsql_nod* cursor = input->nod_arg[e_del_cursor];
	dsql_nod* relation = input->nod_arg[e_del_relation];
	if (cursor && (request->flags & REQ_procedure)) {
		dsql_nod* anode = MAKE_node(request->threadData, nod_erase_current, e_erc_count);
		anode->nod_arg[e_erc_context] =
			(dsql_nod*) pass1_cursor_context(request, cursor, relation);
		return anode;
	}

	request->req_type = (cursor) ? REQ_DELETE_CURSOR : REQ_DELETE;
	dsql_nod* node = MAKE_node(request->threadData, nod_erase, e_era_count);

// Generate record selection expression 

	dsql_nod* rse;
	if (cursor)
		rse = pass1_cursor_reference(request, cursor, relation);
	else {
		rse = MAKE_node(request->threadData, nod_rse, e_rse_count);
		dsql_nod* temp = MAKE_node(request->threadData, nod_list, 1);
		rse->nod_arg[e_rse_streams] = temp;
		temp->nod_arg[0] = PASS1_node(request, relation, false);
		temp = input->nod_arg[e_del_boolean];

		if ( (temp = input->nod_arg[e_del_boolean]) ) {
			rse->nod_arg[e_rse_boolean] = PASS1_node(request, temp, false);
		}

		if ( (temp = input->nod_arg[e_del_plan]) ) {
			rse->nod_arg[e_rse_plan] = PASS1_node(request, temp, false);
		}

		if ( (temp = input->nod_arg[e_del_sort]) ) {
			rse->nod_arg[e_rse_sort] = pass1_sort(request, temp, NULL);
		}

		if ( (temp = input->nod_arg[e_del_rows]) ) {
			rse->nod_arg[e_rse_first] =
				PASS1_node(request, temp->nod_arg[e_rows_length], false);
			rse->nod_arg[e_rse_skip] =
				PASS1_node(request, temp->nod_arg[e_rows_skip], false);
		}
	}

	node->nod_arg[e_era_rse] = rse;
	dsql_nod* temp = rse->nod_arg[e_rse_streams];
	node->nod_arg[e_era_relation] = temp->nod_arg[0];

	request->context.pop();
	return node;
}


/**
  
 	pass1_derived_table
  
    @brief	Process derived table which is part of a from clause.
 

    @param request
    @param input

 **/
 
static dsql_nod* pass1_derived_table(CStatement* request, dsql_nod* input, bool proc_flag)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	//TSQL tdsql = GET_THREAD_DATA;

	dsql_nod* node = MAKE_node (request->threadData, nod_derived_table, e_derived_table_count);
	dsql_str* alias = (dsql_str*) input->nod_arg[e_derived_table_alias];
	node->nod_arg[e_derived_table_alias] = (dsql_nod*) alias;
	node->nod_arg[e_derived_table_column_alias] = input->nod_arg[e_derived_table_column_alias];

	// Create the context now, because we need to know it for the tables inside.	
	dsql_ctx* context = PASS1_make_context(request, node);

	// Save some values to restore after rse process.	
	dsql_str* const aliasRelationPrefix = request->aliasRelationPrefix;

	// Change req_context, because when we are processing the derived table rse
	// it may not reference to other streams in the same scope_level.
	Stack temp;
	while (!request->context.isEmpty())
		{
		void* object = request->context.pop();
		temp.push(object);
		}

	// Put special contexts (NEW/OLD) also on the stack
	FOR_STACK (dsql_ctx*, context, &temp)
		if ((context->ctx_scope_level < request->scopeLevel) || 
			(context->ctx_flags & CTX_system))
			{
			request->context.push(context);
			}
	END_FOR
	void* baseContext = request->context.mark();

	request->aliasRelationPrefix = pass1_alias_concat(request, aliasRelationPrefix, alias);

	// AB: 2005-01-06
	// If our derived table contains a single query with a sub-select buried
	// inside the select items then we need a special handling, because we don't
	// want creating a new sub-select for every reference outside the derived 
	// table to that sub-select.
	// To handle this we simple create a UNION ALL with derived table inside it.
	// Due this mappings are created and we simple reference to these mappings.
	// Optimizer effects: 
	//   Good thing is that only 1 recordstream is made for the sub-select, but
	//   the worse thing is that a UNION curently can't be used in 
	//   deciding the JOIN order.
	dsql_nod* const select_expr = input->nod_arg[e_derived_table_rse];
	dsql_nod* query = select_expr->nod_arg[e_sel_query_spec];
	bool foundSubSelect = false;
	if (query->nod_type == nod_query_spec)
		foundSubSelect = pass1_found_sub_select(query->nod_arg[e_qry_list]);	

	dsql_nod* rse = NULL;
	if (foundSubSelect) 
		{
		dsql_nod* union_expr = MAKE_node(request->threadData, nod_list, 1);
		union_expr->nod_arg[0] = select_expr;
		union_expr->nod_flags = NOD_UNION_ALL;
		rse = pass1_union(request, union_expr, NULL, NULL, 0);
		}
	else 
		rse = PASS1_rse(request, select_expr, NULL);
	
	context->ctx_rse = node->nod_arg[e_derived_table_rse] = rse;

	// Finish off by cleaning up contexts and put them into 
	// ctx_childs_derived_table so others (create view ddl) 
	// can deal with it.
	while (!request->context.isEmpty() && (!request->context.isMark(baseContext)))
		{
		void* object = request->context.pop();
		context->ctx_childs_derived_table.push(object);
		}
	while (!request->context.isEmpty())
		{
		request->context.pop();
		}

	while (!temp.isEmpty())
		{
		void* object = temp.pop();
		request->context.push(object);		
		}

	delete request->aliasRelationPrefix;
	// Restore our original values.
	request->aliasRelationPrefix = aliasRelationPrefix;

	// If an alias-list is specified process it.
	bool ignoreColumnChecks = (input->nod_flags & NOD_DT_IGNORE_COLUMN_CHECK);
	if (node->nod_arg[e_derived_table_column_alias] && 
		node->nod_arg[e_derived_table_column_alias]->nod_count) 
		{
		dsql_nod* list = node->nod_arg[e_derived_table_column_alias];

		// Have both lists the same number of items?
		if (list->nod_count != rse->nod_arg[e_rse_items]->nod_count) 
			{
			// Column list by derived table %s [alias-name] has %s [more/fewer] columns 
			// than the number of items.
			// 
			TEXT err_message[200], aliasname[100];
			aliasname[0] = 0;
			if (alias) 
				{
				int length = alias->str_length;
				if (length > 99) {
					length = 99;
				}
				const TEXT* src = alias->str_data;
				TEXT* dest = aliasname;
				for (; length; length--) 
					*dest++ = *src++;

				*dest = 0;
				}
			if (list->nod_count > rse->nod_arg[e_rse_items]->nod_count) 
				{
				sprintf (err_message, "list by derived table %s has more columns than the number of items.", 
					aliasname);
				}
			else 
				{
				sprintf (err_message, "list by derived table %s has fewer columns than the number of items.", 
					aliasname);
				}
			//
			// !!! THIS MESSAGE SHOULD BE CHANGED !!!
			//
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_dsql_command_err,
					isc_arg_gds, isc_field_name,
					isc_arg_string, err_message, 0);
			}

		// Generate derived fields and assign alias-name to it.
		for (int count = 0; count < list->nod_count; count++) 
			{
			dsql_nod* select_item = rse->nod_arg[e_rse_items]->nod_arg[count];
			// Make new derived field node
			dsql_nod* derived_field = MAKE_node(request->threadData, nod_derived_field, e_derived_field_count);
			derived_field->nod_arg[e_derived_field_value] = select_item;
			derived_field->nod_arg[e_derived_field_name] = list->nod_arg[count];
			derived_field->nod_arg[e_derived_field_scope] = (dsql_nod*)(IPTR) request->scopeLevel;
			derived_field->nod_desc = select_item->nod_desc;

			rse->nod_arg[e_rse_items]->nod_arg[count] = derived_field;
			}
		}
	else 
		{
		// For those select-items where no alias is specified try
		// to generate one from the field_name.
		for (int count = 0; count < rse->nod_arg[e_rse_items]->nod_count; count++) 
			{
			dsql_nod* select_item = 
				pass1_make_derived_field(request, request->threadData, rse->nod_arg[e_rse_items]->nod_arg[count]);

			// Auto-create dummy column name for pass_any()
			if (ignoreColumnChecks && (select_item->nod_type != nod_derived_field)) 
				{
				// Make new derived field node
				dsql_nod* derived_field = 
					MAKE_node(request->threadData, nod_derived_field, e_derived_field_count);
				derived_field->nod_arg[e_derived_field_value] = select_item;

				// Construct dummy fieldname
				char fieldname[25];
				sprintf (fieldname, "f%d", count);
				dsql_str* alias = FB_NEW_RPT(*request->threadData->tsql_default, 
					strlen(fieldname)) dsql_str;
				strcpy(alias->str_data, fieldname);
				alias->str_length = strlen(fieldname);

				derived_field->nod_arg[e_derived_field_name] = (dsql_nod*) alias;
				derived_field->nod_arg[e_derived_field_scope] = 
					(dsql_nod*)(IPTR) request->scopeLevel;
				derived_field->nod_desc = select_item->nod_desc;
				select_item = derived_field;
				}

			rse->nod_arg[e_rse_items]->nod_arg[count] = select_item;
			}
		}

	int count;
	// Check if all root select-items have an derived field else show a message.
	for (count = 0; count < rse->nod_arg[e_rse_items]->nod_count; count++) 
		{
		const dsql_nod* select_item = rse->nod_arg[e_rse_items]->nod_arg[count];
		if (select_item->nod_type != nod_derived_field) 
			{
			// No columnname specified for column number %d
			//
			// !!! THIS MESSAGE SHOULD BE CHANGED !!!
			//
			TEXT columnnumber[80];
			sprintf (columnnumber, "%d is specified without a name", count + 1);
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_dsql_command_err,
					isc_arg_gds, isc_field_name,
					isc_arg_string, columnnumber, 0);
			}		
		}

	// Check for ambiguous columnnames inside this derived table.
	for (count = 0; count < rse->nod_arg[e_rse_items]->nod_count; count++) 
		{
		const dsql_nod* select_item1 = rse->nod_arg[e_rse_items]->nod_arg[count];
		for (int count2 = (count + 1); count2 < rse->nod_arg[e_rse_items]->nod_count; count2++)
			{
			const dsql_nod* select_item2 = rse->nod_arg[e_rse_items]->nod_arg[count2];
			dsql_str* name1 = (dsql_str*) select_item1->nod_arg[e_derived_field_name];
			dsql_str* name2 = (dsql_str*) select_item2->nod_arg[e_derived_field_name];
			if (!strcmp(reinterpret_cast<const char*>(name1->str_data),
				reinterpret_cast<const char*>(name2->str_data)))
				{			
				// The column %s was specified multiple times for derived table %s
				//
				// !!! THIS MESSAGE SHOULD BE CHANGED !!!
				//
				TEXT columnnumber[80];
				if (alias) 
					{
					sprintf (columnnumber, 
						"The column %s was specified multiple times for derived table %s", 
						name1->str_data, alias->str_data);
					}
				else 
					{
					sprintf (columnnumber, 
						"The column %s was specified multiple times for derived table %s", 
						name1->str_data, "unnamed");
					}
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						isc_arg_gds, isc_dsql_command_err,
						isc_arg_gds, isc_field_name,
						isc_arg_string, columnnumber, 0);
				}
			}		
		}

	return node;
}


/**
  
 	pass1_expand_select_list
  
    @brief	Expand asterisk nodes into fields.
 

    @param request
    @param list
    @param streams

 **/
static dsql_nod* pass1_expand_select_list(CStatement* request, dsql_nod* list,
	dsql_nod* streams)
{
	if (!list)
		list = streams;
		
	Stack stack;
	dsql_nod** ptr = list->nod_arg;
	for (const dsql_nod* const* const end = ptr + list->nod_count;
		ptr < end; ptr++)
	{
		pass1_expand_select_node(request, *ptr, stack);
	}
	dsql_nod* node = MAKE_list(request->threadData, &stack);
	return node;
}


/**
  
 	pass1_expand_select_node
  
    @brief	Expand a select item node.
 

    @param request
    @param node
    @param stack

 **/
static void pass1_expand_select_node(CStatement* request, dsql_nod* node, Stack& stack)
{
	DEV_BLKCHK(node, dsql_type_nod);

	if (node->nod_type == nod_join) {
		pass1_expand_select_node(request, node->nod_arg[e_join_left_rel], stack);
		pass1_expand_select_node(request, node->nod_arg[e_join_rght_rel], stack);
	}
	else if (node->nod_type == nod_derived_table) {
		// AB: Derived table support
		dsql_nod* sub_items = node->nod_arg[e_derived_table_rse]->nod_arg[e_rse_items];
		dsql_nod** ptr = sub_items->nod_arg;
		for (const dsql_nod* const* const end = ptr + sub_items->nod_count;
			ptr < end; ++ptr) 
		{
			// Create a new alias else mappings would be mangled.
			dsql_nod* select_item = *ptr;
			// select-item should always be a derived field!
			if (select_item->nod_type != nod_derived_field) {
				// Internal dsql error: alias type expected by exploding.
				//
				// !!! THIS MESSAGE SHOULD BE CHANGED !!!
				//
				ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 104,
					  isc_arg_gds, isc_dsql_command_err, 0);
			}
			stack.push(select_item);
		}
	}
	else if (node->nod_type == nod_relation) {
		dsql_ctx* context = (dsql_ctx*) node->nod_arg[e_rel_context];
		DEV_BLKCHK(context, dsql_type_ctx);
		Procedure* procedure;
		dsql_rel* relation = context->ctx_relation;
		if (relation) {
			for (dsql_fld* field = relation->rel_fields; field;
				field = field->fld_next)
			{
				DEV_BLKCHK(field, dsql_type_fld);
				dsql_nod* select_item = MAKE_field(request->threadData, context, field, 0);
				stack.push(select_item);
			}
		}
		else if (procedure = context->ctx_procedure) {
			for (ProcParam *param = procedure->findOutputParameters(); param; param = param->findNext()) 
				{
				dsql_nod* select_item = MAKE_field(request->threadData, context, param->getDsqlField(), 0);
				select_item->nod_desc = param->paramDescriptor;
				stack.push(select_item);
				}
		}
	}
	else if (node->nod_type == nod_field_name) {
		dsql_nod* select_item = pass1_field(request, node, true, NULL);
		// The node could be a relation so call recursively.
		pass1_expand_select_node(request, select_item, stack);
	}
	else {
		stack.push(node);
	}
}


/**
  
 	pass1_field
  
    @brief	Resolve a field name to an available context.
 	If list is true, then this function can detect and
 	return a relation node if there is no name.   This
 	is used for cases of "SELECT <table_name>. ...".

   CVC: The function attempts to detect
   if an unqualified field appears in more than one context
   and hence it returns the number of occurrences. This was
   added to allow the caller to detect ambiguous commands like
   select  from t1 join t2 on t1.f = t2.f order by common_field.
   While inoffensive on inner joins, it changes the result on outer joins.
 

    @param request
    @param input
    @param list

 **/
static dsql_nod* pass1_field( CStatement* request, dsql_nod* input, 
							 const bool list, dsql_nod* select_list)
{
	DEV_BLKCHK(input, dsql_type_nod);

	// CVC: This shameful hack added to allow CHECK constraint implementation via triggers
	// to be able to work.

	bool is_check_constraint = false;
	const dsql_nod* ddl_node = request->ddlNode;
	
	if (ddl_node && ddl_node->nod_type == nod_def_constraint) 
		is_check_constraint = true;

	// handle an array element.
	
	dsql_nod* indices;
	
	if (input->nod_type == nod_array) 
		{
		indices = input->nod_arg[e_ary_indices];
		input = input->nod_arg[e_ary_array];
		}
	else 
		indices = NULL;

	dsql_str* name;
	dsql_str* qualifier; // We assume the qualifier was stripped elsewhere.
	
	if (input->nod_count == 1) 
		{
		name = (dsql_str*) input->nod_arg[0];
		qualifier = NULL;
		}
	else 
		{
		name = (dsql_str*) input->nod_arg[1];
		qualifier = (dsql_str*) input->nod_arg[0];
		}
		
	DEV_BLKCHK(name, dsql_type_str);
	DEV_BLKCHK(qualifier, dsql_type_str);

    // CVC: Let's strip trailing blanks or comparisons may fail in dialect 3.
    
	if (name && name->str_data) 
		pass_exact_name((TEXT*) name->str_data);

    /* CVC: PLEASE READ THIS EXPLANATION IF YOU NEED TO CHANGE THIS CODE.
       You should ensure that this function:
       1.- Never returns NULL. In such case, it such fall back to an invocation
       to field_error() near the end of this function. None of the multiple callers
       of this function (inside this same module) expect a null pointer, hence they
       will crash the engine in such case.
       2.- Doesn't allocate more than one field in "node". Either you put a break,
       keep the current "continue" or call ALLD_release if you don't want nor the
       continue neither the break if node is already allocated. If it isn't evident,
       but this variable is initialized to zero in the declaration above. You
       may write an explicit line to set it to zero here, before the loop.

       3.- Doesn't waste cycles if qualifier is not null. The problem is not the cycles
       themselves, but the fact that you'll detect an ambiguity that doesn't exist: if
       the field appears in more than one context but it's always qualified, then
       there's no ambiguity. There's PASS1_make_context() that prevents a context's
       alias from being reused. However, other places in the code don't check that you
       don't create a join or subselect with the same context without disambiguating it
       with different aliases. This is the place where resolve_context() is called for
       that purpose. In the future, it will be fine if we force the use of the alias as
       the only allowed qualifier if the alias exists. Hopefully, we will eliminate
       some day this construction: "select table.field from table t" because it
       should be "t.field" instead.

       AB: 2004-01-09
       The explained query directly above doesn't work anymore, thus the day has come ;-)
	   It's allowed to use the same fieldname between different scope levels (sub-queries)
	   without being hit by the ambiguity check. The field uses the first match starting
	   from it's own level (of course ambiguity-check on each level is done).

       4.- Doesn't verify code derived automatically from check constraints. They are
       ill-formed by nature but making that code generation more orthodox is not a
       priority. Typically, they only check a field against a contant. The problem
       appears when they check a field against a subselect, for example. For now,
       allow the user to write ambiguous subselects in check() statements.
       Claudio Valderrama - 2001.1.29.
    */

	if (select_list && !qualifier && name && name->str_data) 
		{
		// AB: Check first against the select list for matching column.
		// When no matches at all are found we go on with our 
		// normal way of field name lookup.
		dsql_nod* node = pass1_lookup_alias(request, name, select_list);

		if (node)
			return node;
		}

	/* Try to resolve field against various contexts;
	   if there is an alias, check only against the first matching */

	Stack ambiguous_ctx_stack;
	dsql_nod* node = 0; 
	
	// AB: Loop through the scope_levels starting by its own.
	bool done = false;
	USHORT current_scope_level = request->scopeLevel + 1;

	for (; (current_scope_level > 0) && !done; current_scope_level--) 
		{
		// If we've found a node we're done.
		if (node) 
			break;

		FOR_STACK (dsql_ctx*, context, &request->context)

			if (context->ctx_scope_level != (current_scope_level - 1)) 
				continue;
		
			dsql_fld* field = resolve_context(request, qualifier, context, is_check_constraint);

			// AB: When there's no relation and no procedure then we have a derived table.
			bool is_derived_table = (!context->ctx_procedure && !context->ctx_relation && context->ctx_rse);		

			if (field)
				{
				// If there's no name then we have most probable an asterisk that
				// needs to be exploded. This should be handled by the caller and
				// when the caller can handle this, list is true.
				
				if (!name) 
					{
					if (list) 
						{
						node = MAKE_node(request->threadData, nod_relation, e_rel_count);
						node->nod_arg[e_rel_context] = (dsql_nod*) context;
						return node;
						}
					break;
					}


				// at this point the context could be a table, a procedure, or 
				// a derived table.  Find a matching field and quit there.
				if (context->ctx_relation)
					field = match_relation_field (context->ctx_relation, name);
				else if (context->ctx_procedure)	
					field = match_procedure_field (context->ctx_procedure, name);
				else
					{
					for (; field; field = field->fld_next) 
						if (!strcmp(name->str_data, field->fld_name)) 
							break;
					}

				if (field)
					ambiguous_ctx_stack.push(context);

				// If a qualifier was present and we don't have found
				// a matching field then we should stop searching.
				// Column unknown error will be raised at bottom of function.

				if (qualifier && !field) 
					{
					done = true;
					break;
					}

				if (field) 
					{
					if (!check_field_type (request, field))
						return NULL;

					// CVC: Stop here if this is our second or third iteration.
					// Anyway, we can't report more than one ambiguity to the status vector.
					// AB: But only if we're on different scope level, because a
					// node inside the same context should have priority.
					if (node)
						continue;

					if (indices) 
						indices = PASS1_node(request, indices, false);
					node = MAKE_field(request->threadData, context, field, indices);
					}
				}
			else if (is_derived_table) 
				{
				// if an qualifier is present check if we have the same derived 
				// table else continue;
				
				if (qualifier) 
					{
					if (!context->ctx_alias)
						continue;
					else if (strcmp(qualifier->str_data, context->ctx_alias)) 
						continue;
					}

				// If there's no name then we have most probable a asterisk that
				// needs to be exploded. This should be handled by the caller and
				// when the caller can handle this, list is true.

				if (!name) 
					{
					if (list) 
						{
						// Node is created so caller pass1_sel_list() can deal with it.
						node = MAKE_node(request->threadData, nod_derived_table, e_derived_table_count);
						node->nod_arg[e_derived_table_rse] = context->ctx_rse;
						return node;
						}
					break;
					}

				// Because every select item has an alias we can just walk 
				// through the list and return the correct node when found.
				
				const dsql_nod* rse_items = context->ctx_rse->nod_arg[e_rse_items];
				dsql_nod* const* ptr = rse_items->nod_arg;
				
				for (const dsql_nod* const* const end = ptr + rse_items->nod_count;
					ptr < end; ptr++)
					{
					dsql_nod* node_select_item = *ptr;
					
					// select-item should always be a alias!
					
					if (node_select_item->nod_type == nod_derived_field) 
						{
						dsql_str* string = (dsql_str*) node_select_item->nod_arg[e_derived_field_name];

						// If this matches,  add the context to the ambiguous list, but
						// only once. Stop if this is our second iteration.

						if (!strcmp(name->str_data, string->str_data))
							{
							ambiguous_ctx_stack.push (context);

							if (node)
								break;

							node = node_select_item;
							break;
							}
						}
					else 
						{
						// Internal dsql error: alias type expected by field.
						//
						// !!! THIS MESSAGE SHOULD BE CHANGED !!!
						//
						ERRD_post(isc_sqlerr, isc_arg_number, -104,
							isc_arg_gds, isc_dsql_command_err, 0);
						}
					}

				// If a qualifier was present and we don't have found
				// a matching field then we should stop searching.
				// Column unknown error will be raised at bottom of function.
				if (!node && qualifier) 
					{
					done = true;
					break;
					}
				}
				
		END_FOR
		}

	// CVC: We can't return blindly if this is a check constraint, because there's
	// the possibility of an invalid field that wasn't found. The multiple places that
	// call this function pass1_field() don't expect a NULL pointer, hence will crash.
	// Don't check ambiguity if we don't have a field.

	if (node && name) 
		node = ambiguity_check(request, node, name, &ambiguous_ctx_stack);

    if (node)
        return node;    

	field_error(qualifier ? (TEXT*) qualifier->str_data : NULL,
				name ? (TEXT*) name->str_data : NULL, input);

	return NULL;
}


/**
  
 	pass1_found_aggregate
  
    @brief	Check the fields inside an aggregate 
   and check if the field scope_level 
   meets the specified conditions.
   In the first call current_scope_level_equal
   should always be true, because this is used
   internally!
 

    @param node
    @param check_scope_level
    @param match_type
    @param current_scope_level_equal

 **/
static bool pass1_found_aggregate(const dsql_nod* node, USHORT check_scope_level,
									 USHORT match_type, bool current_scope_level_equal)
{
	DEV_BLKCHK(node, dsql_type_nod);

	if (node == NULL) return false;

	bool found = false;

	switch (node->nod_type) {
		case nod_gen_id:
		case nod_gen_id2:
		case nod_cast:
		case nod_udf:
			// If arguments are given to the UDF then there's a node list 
			if (node->nod_count == 2) {
				found |= pass1_found_aggregate(node->nod_arg[1],
					check_scope_level, match_type, current_scope_level_equal);
			}
			break;

		case nod_exists:     
		case nod_singular:   
		case nod_coalesce:
		case nod_simple_case:
		case nod_searched_case:
		case nod_add:
		case nod_concatenate:
		case nod_divide:
		case nod_multiply:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_upcase:
		case nod_extract:
		case nod_add2:
		case nod_divide2:
		case nod_multiply2:
		case nod_subtract2:
		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_leq:
		case nod_lss:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_leq_any:
		case nod_lss_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_leq_all:
		case nod_lss_all:
		case nod_between:
		case nod_like:
		case nod_missing:
		case nod_and:
		case nod_or:
		case nod_any:
		case nod_ansi_any:
		case nod_ansi_all:
		case nod_not:
		case nod_unique:
		case nod_containing:
		case nod_starting:
		case nod_list:
		case nod_join:
		case nod_join_inner:
		case nod_join_left:
		case nod_join_right:
		case nod_join_full:
			{
				const dsql_nod* const* ptr = node->nod_arg;
				for (const dsql_nod* const* const end = ptr + node->nod_count;
					ptr < end; ++ptr)
				{
					found |= pass1_found_aggregate(*ptr, check_scope_level, 
						match_type, current_scope_level_equal);
				}
				break;
			}

		case nod_via:
			// Pass only the rse from the nod_via 
			found |= pass1_found_aggregate(node->nod_arg[e_via_rse],
				check_scope_level, match_type, current_scope_level_equal);
			break;

		case nod_rse:
			// Pass rse_boolean (where clause) and rse_items (select items)
			found |= pass1_found_aggregate(node->nod_arg[e_rse_boolean],
				check_scope_level, match_type, false);
			found |= pass1_found_aggregate(node->nod_arg[e_rse_items], 
				check_scope_level, match_type, false);
			break;

		case nod_alias:
			found |= pass1_found_aggregate(node->nod_arg[e_alias_value],
				check_scope_level, match_type, current_scope_level_equal);
			break;

		case nod_aggregate:
			// Pass only rse_group (group by clause)
			found |= pass1_found_aggregate(node->nod_arg[e_agg_group],
				check_scope_level, match_type, current_scope_level_equal);
			break;

		case nod_agg_average:
		case nod_agg_count:
		case nod_agg_max:
		case nod_agg_min:
		case nod_agg_total:
		case nod_agg_average2:
		case nod_agg_total2:
			{
				bool field = false;
				if (node->nod_count) {
					found |= pass1_found_field(node->nod_arg[0], check_scope_level, 
						match_type, &field);
				}
				if (!field) {
					/* For example COUNT(*) is always same scope_level (node->nod_count = 0) 
					   Normaly COUNT(*) is the only way to come here but something stupid
					   as SUM(5) is also possible.
					   If current_scope_level_equal is FALSE scope_level is always higher */
					switch (match_type) {
						case FIELD_MATCH_TYPE_LOWER_EQUAL:
						case FIELD_MATCH_TYPE_EQUAL:
							if (current_scope_level_equal) {
								return true;
							}
							else {
								return false;
							}

						case FIELD_MATCH_TYPE_HIGHER_EQUAL:
							return true;

						case FIELD_MATCH_TYPE_LOWER:
						case FIELD_MATCH_TYPE_HIGHER:
							return false;

						default:
							fb_assert(false);
					}
				}
				break;
			}

		case nod_map:
			{
				const dsql_map* map =
					reinterpret_cast<dsql_map*>(node->nod_arg[e_map_map]);
				found |= pass1_found_aggregate(map->map_node,
					check_scope_level, match_type, current_scope_level_equal);
				break;
			}

		case nod_derived_field:
		case nod_dbkey:
		case nod_field:
		case nod_parameter:
		case nod_relation:
		case nod_variable:
		case nod_constant:
		case nod_null:
		case nod_current_date:
		case nod_current_time:
		case nod_current_timestamp:
		case nod_user_name:
		case nod_current_role:
		case nod_internal_info:
		case nod_dom_value:
			return false;

		default:
			fb_assert(false);
		}

	return found;
}

/**
  
 	pass1_found_field
  
    @brief	Check the fields inside an aggregate 
   and check if the field scope_level 
   meets the specified conditions.
 

    @param node
    @param check_scope_level
    @param match_type
    @param field

 **/
static bool pass1_found_field(const dsql_nod* node, USHORT check_scope_level,
								 USHORT match_type, bool* field)
{
	DEV_BLKCHK(node, dsql_type_nod);

	if (node == NULL)
		return false;

	bool found = false;

	switch (node->nod_type) {
		case nod_field:
			{
				const dsql_ctx* field_context = (dsql_ctx*) node->nod_arg[e_fld_context];
				DEV_BLKCHK(field_context, dsql_type_ctx);
				*field = true;
				switch (match_type) {
					case FIELD_MATCH_TYPE_EQUAL:
						return (field_context->ctx_scope_level == check_scope_level);

					case FIELD_MATCH_TYPE_LOWER:
						return (field_context->ctx_scope_level < check_scope_level);

					case FIELD_MATCH_TYPE_LOWER_EQUAL:
						return (field_context->ctx_scope_level <= check_scope_level);

					case FIELD_MATCH_TYPE_HIGHER:
						return (field_context->ctx_scope_level > check_scope_level);

					case FIELD_MATCH_TYPE_HIGHER_EQUAL:
						return (field_context->ctx_scope_level >= check_scope_level);

					default:
						fb_assert(false);
				}
				break;
			}
			
		case nod_gen_id:
		case nod_gen_id2:
		case nod_cast:
		case nod_udf:
			// If arguments are given to the UDF then there's a node list 
			if (node->nod_count == 2) {
				found |= pass1_found_field(node->nod_arg[1], check_scope_level,
					match_type, field);
			}
			break;

		case nod_exists:     
		case nod_singular:   
		case nod_coalesce:
		case nod_simple_case:
		case nod_searched_case:
		case nod_add:
		case nod_concatenate:
		case nod_divide:
		case nod_multiply:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_upcase:
		case nod_extract:
		case nod_add2:
		case nod_divide2:
		case nod_multiply2:
		case nod_subtract2:
		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_leq:
		case nod_lss:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_leq_any:
		case nod_lss_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_leq_all:
		case nod_lss_all:
		case nod_between:
		case nod_like:
		case nod_missing:
		case nod_and:
		case nod_or:
		case nod_any:
		case nod_ansi_any:
		case nod_ansi_all:
		case nod_not:
		case nod_unique:
		case nod_containing:
		case nod_starting:
		case nod_list:
			{
				const dsql_nod* const* ptr = node->nod_arg;
				for (const dsql_nod* const* const end = ptr + node->nod_count;
					ptr < end; ptr++)
				{
					found |= pass1_found_field(*ptr, check_scope_level, 
						match_type, field);
				}
				break;
			}

		case nod_via:
			// Pass only the rse from the nod_via
			found |= pass1_found_field(node->nod_arg[e_via_rse], 
				check_scope_level, match_type, field);
			break;

		case nod_rse:
			// Pass rse_boolean (where clause) and rse_items (select items)
			found |= pass1_found_field(node->nod_arg[e_rse_boolean], 
				check_scope_level, match_type, field);
			found |= pass1_found_field(node->nod_arg[e_rse_items], 
				check_scope_level, match_type, field);
			break;

		case nod_alias:
			found |= pass1_found_field(node->nod_arg[e_alias_value], 
				check_scope_level, match_type, field);
			break;

		case nod_derived_field:
			{
				// This is a "virtual" field
				*field = true;
				const USHORT df_scope_level = (USHORT)(U_IPTR)node->nod_arg[e_derived_field_scope];

				switch (match_type) {
					case FIELD_MATCH_TYPE_EQUAL:
						return (df_scope_level == check_scope_level);

					case FIELD_MATCH_TYPE_LOWER:
						return (df_scope_level < check_scope_level);

					case FIELD_MATCH_TYPE_LOWER_EQUAL:
						return (df_scope_level <= check_scope_level);

					case FIELD_MATCH_TYPE_HIGHER:
						return (df_scope_level > check_scope_level);

					case FIELD_MATCH_TYPE_HIGHER_EQUAL:
						return (df_scope_level >= check_scope_level);

					default:
						fb_assert(false);
				}
			}
			break;

		case nod_aggregate:
			// Pass only rse_group (group by clause)
			found |= pass1_found_field(node->nod_arg[e_agg_group], 
				check_scope_level, match_type, field);
			break;

		case nod_agg_average:
		case nod_agg_count:
		case nod_agg_max:
		case nod_agg_min:
		case nod_agg_total:
		case nod_agg_average2:
		case nod_agg_total2:
			if (node->nod_count) {
				found |= pass1_found_field(node->nod_arg[0], check_scope_level, 
					match_type, field);
			}
			break;

		case nod_map:
			{
				const dsql_map* map =
					reinterpret_cast <dsql_map*>(node->nod_arg[e_map_map]);
				found |= pass1_found_field(map->map_node, check_scope_level, 
					match_type, field);
				break;
			}

		case nod_dbkey:
		case nod_parameter:
		case nod_relation:
		case nod_variable:
		case nod_constant:
		case nod_null:
		case nod_current_date:
		case nod_current_time:
		case nod_current_timestamp:
		case nod_user_name:
		case nod_current_role:
		case nod_internal_info:
		case nod_dom_value:
			return false;

		default:
			fb_assert(false);
		}

	return found;
}


/**
  
 	pass1_found_sub_select
  
    @brief	Search if a sub select is buried inside 
   an select list from an query expression.

    @param node

 **/
static bool pass1_found_sub_select(const dsql_nod* node)
{
	DEV_BLKCHK(node, dsql_type_nod);

	if (node == NULL) return false;

	switch (node->nod_type) {
		case nod_gen_id:
		case nod_gen_id2:
		case nod_cast:
		case nod_udf:
			// If arguments are given to the UDF then there's a node list 
			if (node->nod_count == 2) {
				if (pass1_found_sub_select(node->nod_arg[1])) {
					return true;
				}
			}
			break;

		case nod_exists:
		case nod_singular:   
		case nod_coalesce:
		case nod_simple_case:
		case nod_searched_case:
		case nod_add:
		case nod_concatenate:
		case nod_divide:
		case nod_multiply:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_upcase:
		case nod_extract:
		case nod_add2:
		case nod_divide2:
		case nod_multiply2:
		case nod_subtract2:
		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_leq:
		case nod_lss:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_leq_any:
		case nod_lss_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_leq_all:
		case nod_lss_all:
		case nod_between:
		case nod_like:
		case nod_missing:
		case nod_and:
		case nod_or:
		case nod_any:
		case nod_ansi_any:
		case nod_ansi_all:
		case nod_not:
		case nod_unique:
		case nod_containing:
		case nod_starting:
		case nod_list:
		case nod_join:
		case nod_join_inner:
		case nod_join_left:
		case nod_join_right:
		case nod_join_full:
			{
				const dsql_nod* const* ptr = node->nod_arg;
				for (const dsql_nod* const* const end = ptr + node->nod_count;
					ptr < end; ++ptr)
				{
					if (pass1_found_sub_select(*ptr)) {
						return true;
					}
				}
				break;
			}

		case nod_via:
			return true;

		case nod_alias:
			if (pass1_found_sub_select(node->nod_arg[e_alias_value])) {
				return true;
			}
			break;

		case nod_aggregate:
		case nod_agg_average:
		case nod_agg_count:
		case nod_agg_max:
		case nod_agg_min:
		case nod_agg_total:
		case nod_agg_average2:
		case nod_agg_total2:
		case nod_map:

		case nod_derived_field:
		case nod_dbkey:
		case nod_field:
		case nod_parameter:
		case nod_relation:
		case nod_variable:
		case nod_constant:
		case nod_null:
		case nod_current_date:
		case nod_current_time:
		case nod_current_timestamp:
		case nod_user_name:
		case nod_current_role:
		case nod_internal_info:
		case nod_dom_value:
		case nod_field_name:
			return false;

		default:
			return true;
		}

	return false;
}


/**
  
 	pass1_group_by_list
  
    @brief	Process INSERT statement.
 

    @param request
    @param input
    @param select_list

 **/
static dsql_nod* pass1_group_by_list(CStatement* request, dsql_nod* input, dsql_nod* selectList)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(select_list, dsql_type_nod);

	if (input->nod_count > MAX_SORT_ITEMS) // sort and group have the same limit for now.
	{
		ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 104,
			isc_arg_gds, isc_dsql_command_err,
			isc_arg_gds, isc_dsql_max_group_items, 0);
			// cannot group on more than 255 items
	}

	// For each node in the list, if it's a field node, see if it's of
   // the form <tablename>.*.   If so, explode the asterisk.   
	// If it's a position pick it from the select list.
	// If none of both then just stack up the node.

	Stack stack;
	dsql_nod** ptr = input->nod_arg;
	for (const dsql_nod* const* const end = ptr + input->nod_count;
		ptr < end; ptr++)
	{
		DEV_BLKCHK(*ptr, dsql_type_nod);
		dsql_nod* sub = (*ptr);
		if (sub->nod_type == nod_field_name) {
			// check for alias or field node
			dsql_nod* frnode = pass1_field(request, sub, true, selectList);
			stack.push(frnode);
		}
		else if ((sub->nod_type == nod_constant) && (sub->nod_desc.dsc_dtype == dtype_long)) {
			const ULONG position = (long) (sub->nod_arg[0]);
			
			if ((position < 1) || !selectList || 
				(position > (ULONG) selectList->nod_count))
			{
					ERRD_post(isc_sqlerr, isc_arg_number, -104,
							isc_arg_gds, isc_dsql_column_pos_err, 
							isc_arg_string, "GROUP BY", 0);
					// Invalid column position used in the GROUP BY clause
			}
			stack.push(PASS1_node(request, selectList->nod_arg[position - 1], false));
		}
		else 
		{
			stack.push(PASS1_node(request, *ptr, false));
		}
	}
		
	// Finally make the complete list.	
	dsql_nod* node = MAKE_list(request->threadData, &stack);

	return node;
}

/**
  
 	pass1_insert
  
    @brief	Process INSERT statement.
 

    @param request
    @param input

 **/
static dsql_nod* pass1_insert( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	//DEV_BLKCHK(input, dsql_type_nod);

	request->req_type = REQ_INSERT;
	dsql_nod* node = MAKE_node(request->threadData, nod_store, e_sto_count);

	// Process SELECT expression, if present 

	dsql_nod* values;
	dsql_nod* rse = input->nod_arg[e_ins_select];
	
	if (rse) 
		{
		node->nod_arg[e_sto_rse] = rse = PASS1_rse(request, rse, NULL);
		values = rse->nod_arg[e_rse_items];
		}
	else
		values = PASS1_node(request, input->nod_arg[e_ins_values], false);

	// Process relation 

	dsql_nod* temp_rel = pass1_relation(request, input->nod_arg[e_ins_relation]);
	node->nod_arg[e_sto_relation] = temp_rel;
	dsql_ctx* context = (dsql_ctx*) temp_rel->nod_arg[0];
	DEV_BLKCHK(context, dsql_type_ctx);
	dsql_rel* relation = context->ctx_relation;

#ifdef SHARED_CACHE
	Sync sync(&relation->syncFields, "pass1_insert");
	sync.lock(Shared);
#endif

	// If there isn't a field list, generate one 

	dsql_nod* fields = input->nod_arg[e_ins_fields];
	
	if (fields) 
		{
		fields = PASS1_node(request, fields, false);
        // begin IBO hack 
                // 02-May-2004, Nickolay Samofatov. Do not constify ptr further e.g. to
		// const dsql_nod* const* .... etc. It chokes GCC 3.4.0 ...
        dsql_nod** ptr = fields->nod_arg;
        
        for (const dsql_nod* const* const end = ptr + fields->nod_count; ptr < end; ptr++)
			{
            const dsql_ctx* tmp_ctx = 0;
            //DEV_BLKCHK (*ptr, dsql_type_nod);
            const dsql_nod* temp2 = *ptr;

            if (temp2->nod_type == nod_field &&
                 (tmp_ctx = (dsql_ctx*) temp2->nod_arg[e_fld_context]) != 0 &&
                  tmp_ctx->ctx_relation &&
                  strcmp (tmp_ctx->ctx_relation->rel_name, relation->rel_name))
				{
                dsql_rel* bad_rel = tmp_ctx->ctx_relation;
                dsql_fld* bad_fld = (dsql_fld*) temp2->nod_arg[e_fld_field];
                // At this time, "fields" has been replaced by the processed list in
                // the same variable, so we refer again to input->nod_arg[e_ins_fields].

                field_error (bad_rel ? (const TEXT*) bad_rel->rel_name : NULL,
                             bad_fld ? (const TEXT*) bad_fld->fld_name : NULL,
                             input->nod_arg[e_ins_fields]->nod_arg[(dsql_nod **)ptr - fields->nod_arg]);
				}
			}
		}
	else 
		{
        // CVC: Ann Harrison requested to skip COMPUTED fields in INSERT w/o field list. 
		Stack stack;
		
		for (dsql_fld* field = relation->rel_fields; field; field = field->fld_next) 
			{
            if (field->fld_flags & FLD_computed)
                continue;
			LLS_PUSH(MAKE_field(request->threadData, context, field, 0), &stack);
			}
			
		fields = MAKE_list(request->threadData, &stack);
		}

	// Match field fields and values 

	if (fields->nod_count != values->nod_count) 
		// count of column list and value list don't match 
		ERRD_post(isc_sqlerr, isc_arg_number, -804,
				  isc_arg_gds, isc_dsql_var_count_err, 0);

	Stack stack;
	dsql_nod** ptr = fields->nod_arg;
	dsql_nod** ptr2 = values->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + fields->nod_count; ptr < end; ptr++, ptr2++)
		{
		//DEV_BLKCHK(*ptr, dsql_type_nod);
		//DEV_BLKCHK(*ptr2, dsql_type_nod);
		dsql_nod* temp = MAKE_node(request->threadData, nod_assign, 2);
		temp->nod_arg[0] = *ptr2;
		temp->nod_arg[1] = *ptr;
		LLS_PUSH(temp, &stack);
		temp = *ptr2;
		set_parameter_type(request, temp, *ptr, false);
		}

	node->nod_arg[e_sto_statement] = MAKE_list(request->threadData, &stack);

	set_parameters_name(node->nod_arg[e_sto_statement],
						node->nod_arg[e_sto_relation]);

	return node;
}


/**
  
 	pass1_join
  
    @brief	Make up join node and mark relations as "possibly NULL" 
	if they are in outer joins (req_in_outer_join).
 

    @param request
    @param input
    @param proc_flag

 **/
static dsql_nod* pass1_join(CStatement* request, dsql_nod* input, bool proc_flag)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	dsql_nod* node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
	// First process type.
	node->nod_arg[e_join_type] = 
		PASS1_node(request, input->nod_arg[e_join_type], proc_flag);
	// Process relations.
	switch (node->nod_arg[e_join_type]->nod_type) {
		case nod_join_inner: 
			node->nod_arg[e_join_left_rel] = 
				PASS1_node(request, input->nod_arg[e_join_left_rel], proc_flag);
			node->nod_arg[e_join_rght_rel] = 
				PASS1_node(request, input->nod_arg[e_join_rght_rel], proc_flag);
		break;
		case nod_join_left:
			node->nod_arg[e_join_left_rel] = 
				PASS1_node(request, input->nod_arg[e_join_left_rel], proc_flag);
			request->req_in_outer_join++;
			node->nod_arg[e_join_rght_rel] = 
				PASS1_node(request, input->nod_arg[e_join_rght_rel], proc_flag);
			request->req_in_outer_join--;
		break;
		case nod_join_right:
			request->req_in_outer_join++;
			node->nod_arg[e_join_left_rel] = 
				PASS1_node(request, input->nod_arg[e_join_left_rel], proc_flag);
			request->req_in_outer_join--;
			node->nod_arg[e_join_rght_rel] = 
				PASS1_node(request, input->nod_arg[e_join_rght_rel], proc_flag);
		break;
		case nod_join_full:
			request->req_in_outer_join++;
			node->nod_arg[e_join_left_rel] = 
				PASS1_node(request, input->nod_arg[e_join_left_rel], proc_flag);
			node->nod_arg[e_join_rght_rel] = 
				PASS1_node(request, input->nod_arg[e_join_rght_rel], proc_flag);
			request->req_in_outer_join--;
		break;

		default:
			fb_assert(false);	// join type expected.
		break;
	}
	// Process boolean (ON clause).

	dsql_nod* boolean = input->nod_arg[e_join_boolean];
	if (boolean && (boolean->nod_type == nod_flag || boolean->nod_type == nod_list))
	{
		// Process NATURAL JOIN or USING clause
		ERRD_post(isc_wish_list, 0);
	}

	node->nod_arg[e_join_boolean] = 
		PASS1_node(request, input->nod_arg[e_join_boolean], proc_flag);

	return node;
}


/**
  
 	pass1_label
  
    @brief	Process loop interruption
 

    @param request
    @param input

 **/
static dsql_nod* pass1_label(CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	dsql_nod* label = NULL;

	// retrieve a label

	switch (input->nod_type) 
		{
		case nod_breakleave:
			label = input->nod_arg[e_breakleave_label];
			break;
			
		case nod_for_select:
			label = input->nod_arg[e_flp_label];
			break;
			
		case nod_exec_into:
			label = input->nod_arg[e_exec_into_label];
			break;
			
		case nod_while:
			label = input->nod_arg[e_while_label];
			break;
			
		default:
			fb_assert(false);
		}

	// look for a label, if specified

	dsql_str* string = NULL;
	USHORT position = 0;

	if (label) 
		{
		fb_assert(label->nod_type == nod_label);
		string = (dsql_str*) label->nod_arg[e_label_name];
		const TEXT* label_string = (TEXT*) string->str_data;
		int index = request->loopLevel;
		
		FOR_STACK (dsql_str*, obj, &request->labels)
			if (obj) 
				{
				const TEXT* obj_string = (TEXT*) obj->str_data;
				if (!strcmp(label_string, obj_string)) 
					{
					position = index;
					break;
					}
				}
			index--;
		END_FOR;
		}

	USHORT number = 0;
	
	if (input->nod_type == nod_breakleave) 
		{
		if (position > 0) 
			{
			// break the specified loop
			number = position;
			}
		else if (label) 
			// ERROR: Label %s is not found in the current scope
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
				isc_arg_gds, isc_dsql_command_err,
				isc_arg_gds, isc_dsql_invalid_label,
				isc_arg_string, string->str_data,
				isc_arg_string, "is not found", 0);
		else
			// break the current loop
			number = request->loopLevel;
		}
	else 
		{
		if (position > 0) 
			// ERROR: Label %s already exists in the current scope
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
				isc_arg_gds, isc_dsql_command_err,
				isc_arg_gds, isc_dsql_invalid_label,
				isc_arg_string, string->str_data,
				isc_arg_string, "already exists", 0);
		else 
			{
			// store label name, if specified
			//LLS_PUSH(string, &request->labels);
			request->labels.push (string);
			number = request->loopLevel;
			}
		}

	fb_assert(number > 0 && number <= request->loopLevel);

	if (!label) {
		label = MAKE_node(request->threadData, nod_label, e_label_count);
		// this label is unnamed, i.e. its nod_arg[e_label_name] is NULL
	}
	label->nod_arg[e_label_number] = (dsql_nod*) (IPTR) number;

	return label;
}


/**
  
 	pass1_lookup_alias
  
    @brief	Lookup a matching item in the select list.
			Return node if found else return NULL.
			If more matches are found we raise ambiguity error.

    @param request
    @param name
    @param selectList

 **/
static dsql_nod* pass1_lookup_alias(CStatement* request, const dsql_str* name, dsql_nod* selectList)
{
	dsql_nod* returnNode = NULL;	
	dsql_nod** ptr = selectList->nod_arg;
	const dsql_nod* const* const end = ptr + selectList->nod_count;
	for (; ptr < end; ptr++) {
		dsql_nod* matchingNode = NULL;
		dsql_nod* node = *ptr;
		switch (node->nod_type) {
			case nod_alias: {
				dsql_str* alias = (dsql_str*) node->nod_arg[e_alias_alias];
				if (!strcmp(alias->str_data, name->str_data)) {
					matchingNode = PASS1_node(request, node, false);
				}
				break;
			}

			case nod_field: {
				dsql_fld* field = (dsql_fld*) node->nod_arg[e_fld_field];
				if (!strcmp(field->fld_name, name->str_data)) {
					matchingNode = PASS1_node(request, node, false);
				}
				break;
			}

			case nod_derived_field: {
				dsql_str* alias = (dsql_str*) node->nod_arg[e_derived_field_name];
				if (!strcmp(alias->str_data, name->str_data)) {
					matchingNode = PASS1_node(request, node, false);
				}
				break;
			}

			default:
				break;
		}
		if (matchingNode) {
			if (returnNode) {
				// There was already a node matched, thus raise ambiguous field name error.				
				TEXT buffer1[256];
				buffer1[0] = 0;
				switch (returnNode->nod_type) {
					case nod_field:
						strcat(buffer1, "a field");
						break;

					case nod_alias:
						strcat(buffer1, "an alias");
						break;

					case nod_derived_field:
						strcat(buffer1, "a derived field");
						break;

					default:
						strcat(buffer1, "an item");
						break;
				}

				TEXT buffer2[256];
				buffer2[0] = 0;
				switch (matchingNode->nod_type) {
					case nod_field:
						strcat(buffer2, "a field");
						break;

					case nod_alias:
						strcat(buffer2, "an alias");
						break;

					case nod_derived_field:
						strcat(buffer2, "a derived field");
						break;

					default:
						strcat(buffer2, "an item");
						break;
				}
				strcat(buffer2, " in the select list with name");

				ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) -204,							
					isc_arg_gds, isc_dsql_ambiguous_field_name,
					isc_arg_string, buffer1,
					isc_arg_string, buffer2,
					isc_arg_gds, isc_random,
					isc_arg_string, name->str_data,
					0);
			}
			returnNode = matchingNode;
		}
	}

	if (returnNode) {
		return returnNode;
	}
	return NULL;
}


/**
  
 	pass1_make_derived_field
  
    @brief	Create a derived field based on underlying expressions
 

    @param request
    @param tdsql
    @param field

 **/
static dsql_nod* pass1_make_derived_field(CStatement* request, TSQL tdsql, dsql_nod* select_item)
{
	DEV_BLKCHK(select_item, dsql_type_nod);

	switch (select_item->nod_type) {
		case nod_derived_field: 
			{
				// Create a derived field that points to a derived field
				dsql_nod* derived_field = MAKE_node(request->threadData, nod_derived_field, e_derived_field_count);
				derived_field->nod_arg[e_derived_field_value] = select_item;
				derived_field->nod_arg[e_derived_field_name] = select_item->nod_arg[e_derived_field_name];
				derived_field->nod_arg[e_derived_field_scope] = (dsql_nod*) (IPTR) request->scopeLevel;
				derived_field->nod_desc = select_item->nod_desc;
				return derived_field;
			}

		case nod_field :
			{
				dsql_fld* field = (dsql_fld*) select_item->nod_arg[e_fld_field];

				// Copy fieldname to a new string.
				dsql_str* alias = FB_NEW_RPT(*tdsql->tsql_default, strlen(field->fld_name)) dsql_str;
				strcpy(alias->str_data, field->fld_name);
				alias->str_length = strlen(field->fld_name);
			
				// Create a derived field and hook in.
				dsql_nod* derived_field = MAKE_node(request->threadData, nod_derived_field, e_derived_field_count);
				derived_field->nod_arg[e_derived_field_value] = select_item;
				derived_field->nod_arg[e_derived_field_name] = (dsql_nod*) alias;
				derived_field->nod_arg[e_derived_field_scope] = (dsql_nod*)(IPTR) request->scopeLevel;
				derived_field->nod_desc = select_item->nod_desc;
				return derived_field;
			}


		case nod_alias:
			{
				// Copy aliasname to a new string.
				dsql_str* alias_alias = (dsql_str*) select_item->nod_arg[e_alias_alias];
				dsql_str* alias = FB_NEW_RPT(*tdsql->tsql_default, strlen(alias_alias->str_data)) dsql_str;
				strcpy(alias->str_data, alias_alias->str_data);
				alias->str_length = strlen(alias_alias->str_data);
			
				// Create a derived field and ignore alias node.
				dsql_nod* derived_field = MAKE_node(request->threadData, nod_derived_field, e_derived_field_count);
				derived_field->nod_arg[e_derived_field_value] = select_item->nod_arg[e_alias_value];
				derived_field->nod_arg[e_derived_field_name] = (dsql_nod*) alias;
				derived_field->nod_arg[e_derived_field_scope] = (dsql_nod*)(IPTR) request->scopeLevel;
				derived_field->nod_desc = select_item->nod_desc;
				return derived_field;
			}

		case nod_map :
			{
				// Aggregate's have map on top.
				dsql_map* map = (dsql_map*) select_item->nod_arg[e_map_map];
				dsql_nod* derived_field = pass1_make_derived_field(request, tdsql, map->map_node);

				// If we had succesfully made a derived field node change it 
				// with orginal map.
				if (derived_field->nod_type == nod_derived_field) {
					derived_field->nod_arg[e_derived_field_value] = select_item;
					derived_field->nod_arg[e_derived_field_scope] = (dsql_nod*)(IPTR) request->scopeLevel;
					derived_field->nod_desc = select_item->nod_desc;
					return derived_field;
				}
				else {
					return select_item;
				}
			} 

		case nod_via :
			{
				// Try to generate derived field from sub-select
				dsql_nod* derived_field = pass1_make_derived_field(request, tdsql, 
					select_item->nod_arg[e_via_value_1]);
				derived_field->nod_arg[e_derived_field_value] = select_item;
				return derived_field;
			}

		default:
			break;
	}

	return select_item;
}


/**
  
 	pass1_not
  
    @brief	Replace NOT with an appropriately inverted condition, if
			possible. Get rid of redundant nested NOT predicates.
 

    @param request
    @param input
	@param proc_flag
	@param invert

 **/
static dsql_nod* pass1_not(CStatement* request,
						   const dsql_nod* input,
						   bool proc_flag,
						   bool invert)
{
	DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	fb_assert(input->nod_type == nod_not);
	dsql_nod* sub = input->nod_arg[0];

	if (sub->nod_type == nod_not) {
		// recurse until different node is found
		// (every even call means no inversion required)
		return pass1_not(request, sub, proc_flag, !invert);
	}

	dsql_nod* node;
	nod_t node_type = input->nod_type;
	bool is_between = false, invert_args = false, no_op = false;

	if (invert) {
		// invert the given boolean
		switch (sub->nod_type) {
		case nod_eql:
			node_type = nod_neq;
			break;
		case nod_neq:
			node_type = nod_eql;
			break;
		case nod_lss:
			node_type = nod_geq;
			break;
		case nod_gtr:
			node_type = nod_leq;
			break;
		case nod_leq:
			node_type = nod_gtr;
			break;
		case nod_geq:
			node_type = nod_lss;
			break;
		case nod_eql_all:
			node_type = nod_neq_any;
			break;
		case nod_neq_all:
			node_type = nod_eql_any;
			break;
		case nod_lss_all:
			node_type = nod_geq_any;
			break;
		case nod_gtr_all:
			node_type = nod_leq_any;
			break;
		case nod_leq_all:
			node_type = nod_gtr_any;
			break;
		case nod_geq_all:
			node_type = nod_lss_any;
			break;
		case nod_eql_any:
			if (sub->nod_arg[1]->nod_type == nod_list) {
				// this is NOT IN (<list>), don't change it
				no_op = true;
			}
			else {
				node_type = nod_neq_all;
			}
			break;
		case nod_neq_any:
			node_type = nod_eql_all;
			break;
		case nod_lss_any:
			node_type = nod_geq_all;
			break;
		case nod_gtr_any:
			node_type = nod_leq_all;
			break;
		case nod_leq_any:
			node_type = nod_gtr_all;
			break;
		case nod_geq_any:
			node_type = nod_lss_all;
			break;
		case nod_between:
			node_type = nod_or;
			is_between = true;
			break;
		case nod_and:
			node_type = nod_or;
			invert_args = true;
			break;
		case nod_or:
			node_type = nod_and;
			invert_args = true;
			break;
		case nod_not:
			// this case is handled in the beginning
			fb_assert(false);
		default:
			no_op = true;
			break;
		}
	}
	else {
		// subnode type hasn't been changed
		node_type = sub->nod_type;
	}

	if (no_op) {
		// no inversion is possible, so just recreate the input node
		// and return immediately to avoid infinite recursion later
		fb_assert(node_type == nod_not);
		node = MAKE_node(request->threadData, input->nod_type, 1);
		node->nod_arg[0] = PASS1_node(request, sub, proc_flag);
		return node;
	}
	else if (is_between) {
		// handle the special BETWEEN case
		fb_assert(node_type == nod_or);
		node = MAKE_node(request->threadData, node_type, 2);
		node->nod_arg[0] = MAKE_node(request->threadData, nod_lss, 2);
		node->nod_arg[0]->nod_arg[0] = sub->nod_arg[0];
		node->nod_arg[0]->nod_arg[1] = sub->nod_arg[1];
		node->nod_arg[1] = MAKE_node(request->threadData, nod_gtr, 2);
		node->nod_arg[1]->nod_arg[0] = sub->nod_arg[0];
		node->nod_arg[1]->nod_arg[1] = sub->nod_arg[2];
	}
	else {
		// create new (possibly inverted) node
		node = MAKE_node(request->threadData, node_type, sub->nod_count);
		dsql_nod* const* src = sub->nod_arg;
		dsql_nod** dst = node->nod_arg;
		for (const dsql_nod* const* end = src + sub->nod_count;
			src < end; src++)
		{
			if (invert_args) {
				dsql_nod* temp = MAKE_node(request->threadData, nod_not, 1);
				temp->nod_arg[0] = *src;
				*dst++ = temp;
			}
			else {
				*dst++ = *src;
			}
		}
	}

	return PASS1_node(request, node, proc_flag);
}


/**
  
 	pass1_put_args_on_stack
  
    @brief	Put recursive non list nodes on the stack
 

    @param request
    @param input
    @param stack
    @param proc_flag

 **/
static void pass1_put_args_on_stack( CStatement* request, dsql_nod* input,
	Stack* stack, bool proc_flag)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	if (input->nod_type != nod_list) {
		LLS_PUSH(PASS1_node(request, input, proc_flag), stack);
		return;
	}

	dsql_nod** ptr = input->nod_arg;
	for (const dsql_nod* const* const end = ptr + input->nod_count;
		ptr < end; ptr++)
	{
		pass1_put_args_on_stack(request, *ptr, stack, proc_flag);
	}
}


/**
  
 	pass1_relation
  
    @brief	Prepare a relation name for processing.  
 	Allocate a new relation node.
 

    @param request
    @param input

 **/
static dsql_nod* pass1_relation( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	dsql_nod* node = MAKE_node(request->threadData, nod_relation, e_rel_count);

	node->nod_arg[e_rel_context] = (dsql_nod*) PASS1_make_context(request, input);

	return node;
}


/**
  
 	pass1_alias_list
  
    @brief	The passed alias list fully specifies a relation.
 	The first alias represents a relation specified in
 	the from list at this scope levels.  Subsequent
 	contexts, if there are any, represent base relations
 	in a view stack.  They are used to fully specify a 
 	base relation of a view.  The aliases used in the 
 	view stack are those used in the view definition.
 

    @param request
    @param alias_list

 **/
static dsql_nod* pass1_alias_list(CStatement* request, dsql_nod* alias_list)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(alias_list, dsql_type_nod);

	dsql_nod** arg = alias_list->nod_arg;
	const dsql_nod* const* const end = arg + alias_list->nod_count;

	// Loop through every alias and find the context for that alias.
	// All aliasses should have a corresponding context.
	int aliasCount = alias_list->nod_count;
	USHORT savedScopeLevel = request->scopeLevel;
	dsql_ctx* context = NULL;
	while (aliasCount > 0)
		{
		if (context) 
			{
			if (context->ctx_rse && !context->ctx_relation && !context->ctx_procedure) 
				{
				// Derived table
				request->scopeLevel++;
				context = pass1_alias(request, &context->ctx_childs_derived_table, (dsql_str*) *arg);
				}
			else
				{
				// This must be a VIEW
				dsql_nod** startArg = arg;
				dsql_rel* relation = context->ctx_relation;
				// find the base table using the specified alias list, skipping the first one
				// since we already matched it to the context.
				for (; arg < end; arg++, aliasCount--)
					if (!(relation = pass1_base_table(request, relation, (dsql_str*) *arg))) 
						break;

				// Found base relation
				if ((aliasCount == 0) && relation)
					{
					// make up a dummy context to hold the resultant relation.
					dsql_ctx* new_context = FB_NEW(*request->threadData->tsql_default) dsql_ctx;
					new_context->ctx_context = context->ctx_context;
					new_context->ctx_relation = relation;

					// concatenate all the contexts to form the alias name;
					// calculate the length leaving room for spaces and a null
					USHORT alias_length = alias_list->nod_count;
					dsql_nod** aliasArg = startArg;
					for (; aliasArg < end; aliasArg++) 
						{
						DEV_BLKCHK(*aliasArg, dsql_type_str);
						alias_length += static_cast<USHORT>(((dsql_str*) *aliasArg)->str_length);
						}

					dsql_str* alias = FB_NEW_RPT(*request->threadData->tsql_default, alias_length) dsql_str;
					alias->str_length = alias_length;

					TEXT* p = new_context->ctx_alias = (TEXT*) alias->str_data;
					
					for (aliasArg = startArg; aliasArg < end; aliasArg++) 
						{
						for (const TEXT* q = (TEXT*) ((dsql_str*) *aliasArg)->str_data; *q;)
							*p++ = *q++;
						*p++ = ' ';
						}
					p[-1] = 0;

					context = new_context;
					}
				else 
					context = NULL;				

				}
			}
		else
			context = pass1_alias(request, &request->context, (dsql_str*) *arg);

		if (!context)
			break;

		arg++;
		aliasCount--;
		}
	request->scopeLevel = savedScopeLevel;

	if (!context) 
		{
		// there is no alias or table named %s at this scope level.
		ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 104,
			isc_arg_gds, isc_dsql_command_err,
			isc_arg_gds, isc_dsql_no_relation_alias,
			isc_arg_string, ((dsql_str*) *arg)->str_data, 0);
		}

	return (dsql_nod*) context;



/*
	dsql_str* internal_qualifier;
	
	if (request->aliasRelationPrefix) 
		internal_qualifier = 
			pass1_alias_concat(request, request->aliasRelationPrefix, (dsql_str*) *arg);
	else 
		internal_qualifier = (dsql_str*) *arg;

	// check the first alias in the list with the relations
	// in the current context for a match.
	
	dsql_rel* relation = 0;
	dsql_ctx* context = pass1_alias(request, &request->context, internal_qualifier);

	if (context) 
		{
		if (alias_list->nod_count == 1) 
			return (dsql_nod*) context;
		relation = context->ctx_relation;
		}

	// if the first alias didn't specify a table in the context stack, 
	// look through all contexts to find one which might be a view with
	// a base table having a matching table name or alias.

	if (!context) 
		FOR_STACK (dsql_ctx*, ctx, &request->context)
			DEV_BLKCHK(ctx, dsql_type_ctx);
			
			if (ctx->ctx_scope_level == request->scopeLevel && ctx->ctx_relation)
				{
				if (relation = pass1_base_table(request, ctx->ctx_relation, (dsql_str*) *arg)) 
					{
					context = ctx;
					break;
					}
				}
		END_FOR

	if (!context) 
		// there is no alias or table named %s at this scope level.
		ERRD_post(isc_sqlerr, isc_arg_number, -104,
				  isc_arg_gds, isc_dsql_command_err,
				  isc_arg_gds, isc_dsql_no_relation_alias,
				  isc_arg_string, ((dsql_str*) *arg)->str_data, 0);

	// Is this context a derived table.
	
	if (!context->ctx_relation && !context->ctx_procedure && context->ctx_rse) 
		{
		request->scopeLevel++;
		dsql_str* tmp_qualifier = (dsql_str*) *arg;
		arg++;
		tmp_qualifier = pass1_alias_concat(request, tmp_qualifier, (dsql_str*) *arg);
		dsql_ctx* dt_context =
			pass1_alias(request, &context->ctx_childs_derived_table, tmp_qualifier);
			
		if (!dt_context) 
			{
			dt_context = 
				pass1_alias(request, &context->ctx_childs_derived_table, (dsql_str*) *arg);
			}

		request->scopeLevel--;

		if (!dt_context) 
			// there is no alias or table named %s at this scope level.
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
				  isc_arg_gds, isc_dsql_command_err,
				  isc_arg_gds, isc_dsql_no_relation_alias,
				  isc_arg_string, ((dsql_str*) *arg)->str_data, 0);

		return (dsql_nod*) dt_context;
	}

	// find the base table using the specified alias list, skipping the first one
	// since we already matched it to the context.
	for (arg++; arg < end; arg++) {
		if (!(relation = pass1_base_table(request, relation, (dsql_str*) *arg))) {
			break;
		}
	}

	if (!relation) {
		// there is no alias or table named %s at this scope level.
		ERRD_post(isc_sqlerr, isc_arg_number, -104,
				  isc_arg_gds, isc_dsql_command_err,
				  isc_arg_gds, isc_dsql_no_relation_alias,
				  isc_arg_string, ((dsql_str*) *arg)->str_data, 0);
	}

	// make up a dummy context to hold the resultant relation.
	//TSQL tdsql = GET_THREAD_DATA;
	dsql_ctx* new_context = FB_NEW(*request->threadData->tsql_default) dsql_ctx;
	new_context->ctx_context = context->ctx_context;
	new_context->ctx_relation = relation;

// concatenate all the contexts to form the alias name;
   //calculate the length leaving room for spaces and a null 

	USHORT alias_length = alias_list->nod_count;
	for (arg = alias_list->nod_arg; arg < end; arg++) {
		DEV_BLKCHK(*arg, dsql_type_str);
		alias_length += static_cast < USHORT > (((dsql_str*) *arg)->str_length);
	}

	dsql_str* alias = FB_NEW_RPT(*request->threadData->tsql_default, alias_length) dsql_str;
	alias->str_length = alias_length;

	TEXT* p = new_context->ctx_alias = (TEXT*) alias->str_data;
	
	for (arg = alias_list->nod_arg; arg < end; arg++) 
		{
		DEV_BLKCHK(*arg, dsql_type_str);
		for (const TEXT* q = (TEXT*) ((dsql_str*) *arg)->str_data; *q;)
			*p++ = *q++;
		*p++ = ' ';
		}

	p[-1] = 0;

	return (dsql_nod*) new_context;
*/
}


/**
  
 	pass1_alias
  
    @brief	The passed relation or alias represents 
 	a context which was previously specified
 	in the from list.  Find and return the 
 	proper context.
 

    @param request
    @param alias

 **/
static dsql_ctx* pass1_alias(CStatement* request, Stack* stack, dsql_str* alias)
{
	dsql_ctx* relation_context = NULL;

	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(alias, dsql_type_str);
	
	// CVC: Getting rid of trailing spaces.
	if (alias && alias->str_data)
		pass_exact_name(reinterpret_cast<TEXT*>(alias->str_data));

	// look through all contexts at this scope level
	// to find one that has a relation name or alias
	// name which matches the identifier passed.  
	
	FOR_STACK (dsql_ctx*, context, stack) 
		if (context->ctx_scope_level != request->scopeLevel) 
			continue;

		// check for matching alias.
		if (context->ctx_internal_alias) 
			{
			if (strcmp(context->ctx_internal_alias, alias->str_data) == 0)
				return context;

			continue;
			}
		else 
			{
			// If an unnamed derived table and empty alias
			if (context->ctx_rse && !context->ctx_relation && 
				!context->ctx_procedure && (alias->str_length == 0))
				{
				relation_context = context;
				}
			}	

		// check for matching relation name; aliases take priority so
		// save the context in case there is an alias of the same name;
		// also to check that there is no self-join in the query.
		
		if (context->ctx_relation && context->ctx_relation->rel_name == alias->str_data)
			{
			if (relation_context)
				/* the table %s is referenced twice; use aliases to differentiate */
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						  isc_arg_gds, isc_dsql_command_err,
						  isc_arg_gds, isc_dsql_self_join,
						  isc_arg_string, alias->str_data, 0);
			relation_context = context;
			}
	END_FOR

	return relation_context;
}


/**
  
 	pass1_alias_concat
  
    @brief	Concatenate 2 input strings together for
	a new alias string. 
	Note: Both input params can be empty.
 

    @param input1
    @param input2

 **/
static dsql_str* pass1_alias_concat(CStatement* request, dsql_str* input1, dsql_str* input2)
{
	//TSQL tdsql = GET_THREAD_DATA;

	DEV_BLKCHK(input1, dsql_type_str);
	DEV_BLKCHK(input2, dsql_type_str);

	int length = 0; 
	if (input1) {
		length += input1->str_length;
	}
	if (input1 && input2) {
		length++; // Room for space character.
	}
	if (input2) {
		length += input2->str_length;
	}
	dsql_str* output = FB_NEW_RPT(*request->threadData->tsql_default, length) dsql_str;
	output->str_length = length;
	TEXT* ptr = output->str_data;
	if (input1) {
		strcat(ptr, input1->str_data);
	}
	if (input1 && input2) {
		strcat(ptr, " ");
	}
	if (input2) {
		strcat(ptr, input2->str_data);
	}
	return output;
}


/**
  
 	pass1_base_table
  
    @brief	Check if the relation in the passed context
 	has a base table which matches the passed alias.
 

    @param request
    @param relation
    @param alias

 **/
static dsql_rel* pass1_base_table( CStatement* request, dsql_rel* relation, dsql_str* alias)
{

	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(alias, dsql_type_str);

	/***
	return METD_get_view_relation(request,
								  relation->rel_name,
								  alias->str_data, 0);
	***/

	return request->findViewRelation (relation->rel_name, *alias, 0);
}


/**
  
 	pass1_rse
  
    @brief	Compile a record selection expression.  
 	The input node may either be a "select_expression" 
 	or a "list" (an implicit union).
 

    @param request
    @param input
    @param order
    @param rows
    @param update_lock

 **/
static dsql_nod* pass1_rse( CStatement* request, dsql_nod* input, dsql_nod* order,
	dsql_nod* rows, dsql_nod* update_lock, USHORT flags)
{
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(order, dsql_type_nod);

	// Handle implicit union case first.  Maybe it's not a union 

	if (input->nod_type == nod_select_expr)
	{
		return pass1_rse(request, input->nod_arg[e_sel_query_spec],
						 input->nod_arg[e_sel_order], input->nod_arg[e_sel_rows],
						 update_lock, input->nod_flags);
	}
	else if (input->nod_type == nod_list) 
	{
		fb_assert(input->nod_count > 1);
			
		if (update_lock)
			ERRD_post(isc_sqlerr, isc_arg_number, -104, isc_arg_gds, 
						isc_token_err, // Token unknown 
						isc_arg_gds, isc_random, isc_arg_string, "WITH LOCK", 0);
						
		return pass1_union(request, input, order, rows, flags);
	}
	else 
	{
		fb_assert(input->nod_type == nod_query_spec);
	}


	// Save the original base of the context stack and process relations 

	dsql_nod* target_rse = MAKE_node(request->threadData, nod_rse, e_rse_count);
	dsql_nod* rse = target_rse;
	rse->nod_arg[e_rse_lock] = update_lock;
	
	dsql_nod* list = rse->nod_arg[e_rse_streams] = 
		PASS1_node(request, input->nod_arg[e_qry_from], false);

	dsql_rel* relation;
	
	if (update_lock && (list->nod_count != 1 || list->nod_arg[0]->nod_type != nod_relation ||
	     !(relation = ((dsql_ctx*)list->nod_arg[0]->nod_arg[e_rel_context])->ctx_relation) ||
	     (relation->rel_flags & REL_view) || (relation->rel_flags & REL_external) )) 
		ERRD_post(isc_sqlerr, isc_arg_number, -104,
				isc_arg_gds, isc_token_err,	// Token unknown 
				isc_arg_gds, isc_random, isc_arg_string, "WITH LOCK", 0);
	
	// Process LIMIT and/or ROWS, if any 
	
	dsql_nod* node = input->nod_arg[e_qry_limit];
	
	if (node && rows) 
		ERRD_post(isc_sqlerr, isc_arg_number, -104,
				  isc_arg_gds, isc_token_err,	// Token unknown 
				  isc_arg_gds, isc_random, isc_arg_string, "ROWS", 0);
    else if (node || (node = rows) ) 
		{
		/* CVC: This line is a hint because set_parameter_type() doesn't receive
           the dialect currently as an argument. */
        node->nod_desc.dsc_scale = request->req_client_dialect;

		const int length_index = rows ? e_rows_length : e_limit_length;
		const int skip_index = rows ? e_rows_skip : e_limit_skip;

        if (node->nod_arg[length_index]) 
			{
            dsql_nod* sub = PASS1_node(request, node->nod_arg[length_index], false);
            rse->nod_arg[e_rse_first] = sub;
            set_parameter_type(request, sub, node, false);
			}
			
        if (node->nod_arg[skip_index]) 
			{
            dsql_nod* sub = PASS1_node(request, node->nod_arg[skip_index], false);
            rse->nod_arg[e_rse_skip] = sub;
            set_parameter_type(request, sub, node, false);
			}
		}

	// Process boolean, if any 

	if (node = input->nod_arg[e_qry_where]) 
		{
		++request->req_in_where_clause;
		rse->nod_arg[e_rse_boolean] = PASS1_node(request, node, false);
		--request->req_in_where_clause;

		/* AB: An aggregate pointing to it's own parent_context isn't
		   allowed, HAVING should be used instead */
		   
		if (pass1_found_aggregate(rse->nod_arg[e_rse_boolean], 
				request->scopeLevel, FIELD_MATCH_TYPE_EQUAL, true))
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
				isc_arg_gds, isc_dsql_agg_where_err, 0);
			// Cannot use an aggregate in a WHERE clause, use HAVING instead
		}

#ifdef DSQL_DEBUG
	if (DSQL_debug & 16) {
		dsql_trace("PASS1_rse input tree:");
		DSQL_pretty(input, 0);
	}
#endif

	// Process select list, if any. If not, generate one 
	dsql_nod* selectList = input->nod_arg[e_qry_list];
	// First expand select list, this will expand nodes with asterisk.
	++request->req_in_select_list;
	selectList = pass1_expand_select_list(request, selectList, rse->nod_arg[e_rse_streams]);

	if ((flags & NOD_SELECT_EXPR_VALUE) && (!selectList || selectList->nod_count > 1))
	{
		// More than one column (or asterisk) is specified in column_singleton
		ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 104,
				  isc_arg_gds, isc_dsql_command_err,
				  isc_arg_gds, isc_dsql_count_mismatch, 0);
	}

	// Pass select list
	rse->nod_arg[e_rse_items] = pass1_sel_list(request, selectList);
	--request->req_in_select_list;

	// Process ORDER clause, if any 
	if (order) {
		++request->req_in_order_by_clause;
		rse->nod_arg[e_rse_sort] = pass1_sort(request, order, selectList);
		--request->req_in_order_by_clause;
	}

	// A GROUP BY, HAVING, or any aggregate function in the select list 
	// will force an aggregate
	dsql_ctx* parent_context = NULL;
	dsql_nod* parent_rse = NULL;
	dsql_nod* aggregate = NULL;

	if (input->nod_arg[e_qry_group] || 
		input->nod_arg[e_qry_having] ||
		(rse->nod_arg[e_rse_items] && aggregate_found(request, rse->nod_arg[e_rse_items])) ||
		(rse->nod_arg[e_rse_sort] && aggregate_found(request, rse->nod_arg[e_rse_sort])))
		{
		// dimitr: don't allow WITH LOCK for aggregates
		if (update_lock) 
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_token_err,	// Token unknown 
					isc_arg_gds, isc_random, isc_arg_string, "WITH LOCK", 0);

		parent_context = FB_NEW(*request->threadData->tsql_default) dsql_ctx;
		parent_context->ctx_context = request->contextNumber++;
		parent_context->ctx_scope_level = request->scopeLevel;
		aggregate = MAKE_node(request->threadData, nod_aggregate, e_agg_count);
		aggregate->nod_arg[e_agg_context] = (dsql_nod*) parent_context;
		aggregate->nod_arg[e_agg_rse] = rse;
		parent_rse = target_rse = MAKE_node(request->threadData, nod_rse, e_rse_count);
		parent_rse->nod_arg[e_rse_streams] = list = MAKE_node(request->threadData, nod_list, 1);
		list->nod_arg[0] = aggregate;

		if (rse->nod_arg[e_rse_first]) 
			{
			parent_rse->nod_arg[e_rse_first] = rse->nod_arg[e_rse_first];
			rse->nod_arg[e_rse_first] = NULL;
			}
			
		if (rse->nod_arg[e_rse_skip]) 
			{
			parent_rse->nod_arg[e_rse_skip] = rse->nod_arg[e_rse_skip];
			rse->nod_arg[e_rse_skip] = NULL;
			}

		request->context.push(parent_context);
		// replace original contexts with parent context
		remap_streams_to_parent_context(rse->nod_arg[e_rse_streams], parent_context);
		}

	// Process GROUP BY clause, if any 

	if (node = input->nod_arg[e_qry_group])
		{
		// if there are positions in the group by clause then replace them 
		// by the (newly pass) items from the select_list
		++request->req_in_group_by_clause;
		aggregate->nod_arg[e_agg_group] = 
			pass1_group_by_list(request, input->nod_arg[e_qry_group], selectList);
		--request->req_in_group_by_clause;

		/* AB: An field pointing to another parent_context isn't
		   allowed and GROUP BY items can't contain aggregates */
		bool field;
		
		if (pass1_found_field(aggregate->nod_arg[e_agg_group], request->scopeLevel, FIELD_MATCH_TYPE_LOWER, &field) ||
		    pass1_found_aggregate(aggregate->nod_arg[e_agg_group], request->scopeLevel, FIELD_MATCH_TYPE_LOWER_EQUAL, true))
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
				isc_arg_gds, isc_dsql_agg_group_err, 0); // Cannot use an aggregate in a GROUP BY clause
		}

	// Parse a user-specified access PLAN
	rse->nod_arg[e_rse_plan] =
		PASS1_node(request, input->nod_arg[e_qry_plan], false);

	// AB: Pass select-items for distinct operation again, because for 
	// sub-selects a new contextnumber should be generated
	
	if (input->nod_arg[e_qry_distinct]) 
		{
		if (update_lock) 
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_token_err,	// Token unknown 
					isc_arg_gds, isc_random, isc_arg_string, "WITH LOCK", 0);

		++request->req_in_select_list;
		target_rse->nod_arg[e_rse_reduced] = pass1_sel_list(request, selectList);
		--request->req_in_select_list;
		}

	// Unless there was a parent, we're done 

	if (!parent_context) 
		{
		rse->nod_flags = flags;
		return rse;
		}

	// Reset context of select items to point to the parent stream 
	
	parent_rse->nod_arg[e_rse_items] = remap_fields(request, rse->nod_arg[e_rse_items], parent_context);
	rse->nod_arg[e_rse_items] = NULL;

	// AB: Check for invalid contructions inside selected-items list
	
	list = parent_rse->nod_arg[e_rse_items];
	const dsql_nod* const* ptr = list->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + list->nod_count; ptr < end; ptr++)
		if (invalid_reference(parent_context, *ptr,
			aggregate->nod_arg[e_agg_group], false, false))
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
				isc_arg_gds, isc_dsql_agg_column_err,
				isc_arg_string, "select list", 0); // Invalid expression in the select list
				// (not contained in either an aggregate or the GROUP BY clause)

	// Reset context of order items to point to the parent stream 
	
	if (order) 
		{
		parent_rse->nod_arg[e_rse_sort] = remap_fields(request, rse->nod_arg[e_rse_sort], parent_context);
		rse->nod_arg[e_rse_sort] = NULL;

		// AB: Check for invalid contructions inside the ORDER BY clause
		
		list = target_rse->nod_arg[e_rse_sort];
		const dsql_nod* const* ptr = list->nod_arg;
		
		for (const dsql_nod* const* const end = ptr + list->nod_count; ptr < end; ptr++)
			if (invalid_reference(parent_context, *ptr, aggregate->nod_arg[e_agg_group], false, false))
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_dsql_agg_column_err,
					isc_arg_string, "ORDER BY clause", 0); // Invalid expression in the ORDER BY clause
					// (not contained in either an aggregate or the GROUP BY clause)
		}

	// And, of course, reduction clauses must also apply to the parent 
	
	if (input->nod_arg[e_qry_distinct]) 
		parent_rse->nod_arg[e_rse_reduced] = remap_fields(request, parent_rse->nod_arg[e_rse_reduced], parent_context);

	// Process HAVING clause, if any 

	if (node = input->nod_arg[e_qry_having]) 
		{
		++request->req_in_having_clause;
		parent_rse->nod_arg[e_rse_boolean] = PASS1_node(request, node, false);
		--request->req_in_having_clause;

		parent_rse->nod_arg[e_rse_boolean] =
			remap_fields(request, parent_rse->nod_arg[e_rse_boolean], parent_context);

		// AB: Check for invalid contructions inside the HAVING clause
		
		list = parent_rse->nod_arg[e_rse_boolean];
		dsql_nod** ptr = list->nod_arg;
		
		for (const dsql_nod* const* const end = ptr + list->nod_count; ptr < end; ptr++)
			if (invalid_reference(parent_context, *ptr, aggregate->nod_arg[e_agg_group], false, false))
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
					isc_arg_gds, isc_dsql_agg_having_err,
					isc_arg_string, "HAVING clause", 0); // Invalid expression in the HAVING clause
					// (neither an aggregate nor contained in the GROUP BY clause)

#ifdef	CHECK_HAVING
		if (aggregate)
			if (invalid_reference(parent_rse->nod_arg[e_rse_boolean],
								  aggregate->nod_arg[e_agg_group]))
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						  isc_arg_gds, isc_field_ref_err, 0); // invalid field reference 
#endif
	}

	parent_rse->nod_flags = flags;
	return parent_rse;
}


/**
  
 	pass1_searched_case
  
    @brief	Handle a reference to a searched case expression.
 

    @param request
    @param input
    @param proc_flag

 **/
static dsql_nod* pass1_searched_case( CStatement* request, dsql_nod* input, bool proc_flag)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(input->nod_arg[0], dsql_type_nod);

	dsql_nod* node = MAKE_node(request->threadData, nod_searched_case, 2);
	dsql_nod* list = input->nod_arg[0];

	// build boolean-expression list 
	{ // scope block
	Stack stack;
	dsql_nod** ptr = list->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + list->nod_count; ptr < end; ptr += 2)
		pass1_put_args_on_stack(request, *ptr, &stack, proc_flag);

	node->nod_arg[e_searched_case_search_conditions] = MAKE_list(request->threadData, &stack);
	} // end scope block

	// build when_result list including else_result at the end 
	/* else_result is included for easy handling in MAKE_desc() */
	
	{ // scope block
	Stack stack;
	dsql_nod** ptr = list->nod_arg;
	const dsql_nod* const* const end = ptr + list->nod_count;
	
	for (++ptr; ptr < end; ptr += 2) 
		pass1_put_args_on_stack(request, *ptr, &stack, proc_flag);

	pass1_put_args_on_stack(request, input->nod_arg[1], &stack, proc_flag);
	node->nod_arg[e_searched_case_results] = MAKE_list(request->threadData, &stack);
	} // end scope block

	// Set describer for output node 
	MAKE_desc(request->threadData, &node->nod_desc, node, NULL);

	// Set parameter-types if parameters are there in the result nodes 
	dsql_nod* case_results = node->nod_arg[e_searched_case_results];
	dsql_nod** ptr = case_results->nod_arg;
	for (const dsql_nod* const* const end = ptr + case_results->nod_count; ptr < end; ptr++)
		set_parameter_type(request, *ptr, node, false);

	return node;
}


/**
  
 	pass1_sel_list
  
    @brief	Compile a select list, which may contain things
 	like "<table_name>.".
 

    @param request
    @param input

 **/
static dsql_nod* pass1_sel_list( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	Stack stack;
	dsql_nod** ptr = input->nod_arg;
	for (const dsql_nod* const* const end = ptr + input->nod_count;
		ptr < end; ptr++)
	{
		DEV_BLKCHK(*ptr, dsql_type_nod);
		stack.push(PASS1_node(request, *ptr, false));
	}
	dsql_nod* node = MAKE_list(request->threadData, &stack);

	return node;
}


/**
  
 	pass1_simple_case
  
    @brief	Handle a reference to a simple case expression.
 

    @param request
    @param input
    @param proc_flag

 **/
static dsql_nod* pass1_simple_case( CStatement* request, dsql_nod* input, bool proc_flag)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(input->nod_arg[0], dsql_type_nod);

	dsql_nod* node = MAKE_node(request->threadData, nod_simple_case, 3);

	// build case_operand node
	node->nod_arg[e_simple_case_case_operand] = 
		PASS1_node(request, input->nod_arg[0], proc_flag);

	dsql_nod* list = input->nod_arg[1];

	// build when_operand list
	{ // scope block
	Stack stack;
	dsql_nod** ptr = list->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + list->nod_count; ptr < end; ptr += 2)
		pass1_put_args_on_stack(request, *ptr, &stack, proc_flag);

	node->nod_arg[e_simple_case_when_operands] = MAKE_list(request->threadData, &stack);
	} // end scope block

	// build when_result list including else_result at the end
	// else_result is included for easy handling in MAKE_desc()
	{ // scope block
	Stack stack;
	dsql_nod** ptr = list->nod_arg;
	const dsql_nod* const* const end = ptr + list->nod_count;
	for (++ptr; ptr < end; ptr += 2) 
		pass1_put_args_on_stack(request, *ptr, &stack, proc_flag);

	pass1_put_args_on_stack(request, input->nod_arg[2], &stack, proc_flag);
	node->nod_arg[e_simple_case_results] = MAKE_list(request->threadData, &stack);
	} // end scope block

	// Check if there is a parameter in the case/when operand list
	bool setParameters = (node->nod_arg[e_simple_case_case_operand]->nod_type == nod_parameter);
	if (!setParameters) 
	{
		list = node->nod_arg[e_simple_case_when_operands];
		dsql_nod** ptr = list->nod_arg;
		for (const dsql_nod* const* const end = ptr + list->nod_count;
			ptr < end; ++ptr)
		{
			if ((*ptr)->nod_type == nod_parameter) 
			{
				setParameters = true;
				break;
			}
		}
	}

	// build list for making describe information from 
	// case_operand and when_operands this is used for
	// setting parameter describers if used in this case
	if (setParameters) 
	{
		list = node->nod_arg[e_simple_case_when_operands];
		dsql_nod* node1 = MAKE_node(request->threadData, nod_list, list->nod_count + 1);

		{ // scope block
			int i = 0;
			node1->nod_arg[i++] = node->nod_arg[e_simple_case_case_operand];
			dsql_nod** ptr = list->nod_arg;

			for (const dsql_nod* const* const end = ptr + list->nod_count;
				ptr < end; ++ptr, ++i)
			{
				node1->nod_arg[i] = *ptr;
			}

			MAKE_desc_from_list(request->threadData, &node1->nod_desc, node1, NULL, "CASE");
			// Set parameter describe information
			set_parameter_type(request, node->nod_arg[e_simple_case_case_operand], node1, false);
		} // end scope block

		{ // scope block
			dsql_nod* simple_when = node->nod_arg[e_simple_case_when_operands];
			dsql_nod** ptr = simple_when->nod_arg;
			for (const dsql_nod* const* const end = ptr + simple_when->nod_count;
				ptr < end; ptr++) 
			{
				set_parameter_type(request, *ptr, node1, false);
			}
		} // end scope block
	
		// Clean up temporary used node
		delete node1;
	}

	// Set describer for output node	
	MAKE_desc(request->threadData, &node->nod_desc, node, NULL);
	
	// Set parameter describe information for evt. results parameters	
	dsql_nod* simple_res = node->nod_arg[e_simple_case_results];
	dsql_nod** ptr = simple_res->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + simple_res->nod_count; ptr < end; ptr++) 
		set_parameter_type(request, *ptr, node, false);

	return node;
}


/**
  
 	pass1_sort
  
    @brief	Compile a parsed sort list
 

    @param request
    @param input
    @param s_list

 **/
static dsql_nod* pass1_sort( CStatement* request, dsql_nod* input, dsql_nod* selectList)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(selectList, dsql_type_nod);

	if (input->nod_type != nod_list) {
		ERRD_post(isc_sqlerr, isc_arg_number, -104, isc_arg_gds, 
			isc_dsql_command_err, isc_arg_gds, isc_order_by_err, 0);
			// invalid ORDER BY clause 				  
	}

	if (input->nod_count > MAX_SORT_ITEMS)
	{
		ERRD_post(isc_sqlerr, isc_arg_number, -104, isc_arg_gds,
			isc_dsql_command_err, isc_arg_gds, isc_order_by_err,
			isc_arg_gds, isc_dsql_max_sort_items, 0);
			// invalid ORDER BY clause, cannot sort on more than 255 items
	}

// Node is simply to be rebuilt -- just recurse merrily 

	dsql_nod* node = MAKE_node(request->threadData, input->nod_type, input->nod_count);
	dsql_nod** ptr2 = node->nod_arg;

	dsql_nod** ptr = input->nod_arg;

	for (const dsql_nod* const* const end = ptr + input->nod_count;
		ptr < end; ptr++)
		{
		DEV_BLKCHK(*ptr, dsql_type_nod);
		dsql_nod* node1 = *ptr;
		if (node1->nod_type != nod_order) {
			ERRD_post(isc_sqlerr, isc_arg_number, -104, isc_arg_gds, 
				isc_dsql_command_err, isc_arg_gds, isc_order_by_err, 0);
				// invalid ORDER BY clause 
		}					  
		dsql_nod* node2 = MAKE_node(request->threadData, nod_order, e_order_count);
		node2->nod_arg[e_order_flag] = node1->nod_arg[e_order_flag]; // asc/desc flag 
		node2->nod_arg[e_order_nulls] = node1->nod_arg[e_order_nulls]; // nulls first/last flag 

		dsql_str* collate = 0;
		
		// get node of value to be ordered by
		node1 = node1->nod_arg[e_order_field];

		if (node1->nod_type == nod_collate) 
			{
			collate = (dsql_str*) node1->nod_arg[e_coll_target];
			// substitute nod_collate with its argument (real value)
			node1 = node1->nod_arg[e_coll_source];
			}

		// check for alias or field node
		if (node1->nod_type == nod_field_name) 
			node1 = pass1_field(request, node1, false, selectList); 
		else if (node1->nod_type == nod_constant && 
			node1->nod_desc.dsc_dtype == dtype_long) 
			{
			const ULONG position = *((ULONG*) &(node1->nod_arg[0]));
			if ((position < 1) || !selectList || 
				(position > (ULONG) selectList->nod_count))
				{
				ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 104,
					isc_arg_gds, isc_dsql_column_pos_err,
					isc_arg_string, "ORDER BY", 0);
				// Invalid column position used in the ORDER BY clause
				}
			// substitute ordinal with appropriate field
			node1 = PASS1_node(request, selectList->nod_arg[position - 1], false);
			}
		else 
			node1 = PASS1_node(request, node1, false);
		

		// finally apply collation order, if necessary
		if (collate) 
			node1 = pass1_collate(request, node1, collate);
		

		// store actual value to be ordered by
		node2->nod_arg[e_order_field] = node1;
		*ptr2++ = node2;
		}

	return node;
}


/**
  
 	pass1_udf
  
    @brief	Handle a reference to a user defined function.
 

    @param request
    @param input
    @param proc_flag

 **/
static dsql_nod* pass1_udf( CStatement* request, dsql_nod* input, bool proc_flag)
{
	dsql_str* name = (dsql_str*) input->nod_arg[0];
	//dsql_udf* userFunc = METD_get_function(request, name);
	UserFunction* userFunc = request->findFunction (*name);

	if (!userFunc)
		ERRD_post(isc_sqlerr, isc_arg_number, -804,
				  isc_arg_gds, isc_dsql_function_err,
				  isc_arg_gds, isc_random,
				  isc_arg_string, name->str_data, 0);

	dsql_nod* node = MAKE_node(request->threadData, nod_udf, input->nod_count);
	node->nod_arg[0] = (dsql_nod*) userFunc;
	
	if (input->nod_count == 2) 
		{
		Stack stack;
		pass1_udf_args(request, input->nod_arg[1], userFunc, 0, &stack, proc_flag);
		node->nod_arg[1] = MAKE_list(request->threadData, &stack);
		}

	return node;
}


/**
  
 	pass1_udf_args
  
    @brief	Handle references to function arguments.
 

    @param request
    @param input
    @param udf
    @param arg_pos
    @param stack
    @param proc_flag

 **/
static void pass1_udf_args(CStatement* request, dsql_nod* input,
						   UserFunction* userFunc, USHORT arg_pos, Stack* stack, bool proc_flag)
{
	if (input->nod_type == nod_list)
		{
		dsql_nod** ptr = input->nod_arg;
		
		for (const dsql_nod* const* const end = ptr + input->nod_count; ptr < end; ptr++)
			pass1_udf_args(request, *ptr, userFunc, arg_pos++, stack, proc_flag);
		
		return;
		}

    dsql_nod* temp = PASS1_node (request, input, proc_flag);
    //dsql_nod* args = userFunc->udf_arguments;
    
    if (temp->nod_type == nod_parameter) 
		{
        //if (args && args->nod_count > arg_pos)
        if (arg_pos <= userFunc->fun_count)
			{
            //set_parameter_type (request, temp, args->nod_arg[arg_pos], false);
			int pos = (arg_pos < userFunc->fun_return_arg) ? arg_pos : arg_pos + 1;
			dsql_nod arg;
            arg.nod_desc = userFunc->fun_rpt[pos].fun_desc;
            set_parameter_type (request, temp, &arg, false);
            }
        else 
			{
            ; 
            /* We should complain here in the future! The parameter is
                out of bounds or the function doesn't declare input params. */
			}
		}
		
    LLS_PUSH (temp, stack);
}


/**
  
 	pass1_union
  
    @brief	Handle a UNION of substreams, generating
 	a mapping of all the fields and adding an
 	implicit PROJECT clause to ensure that all 
 	the records returned are unique.
 

    @param request
    @param input
    @param order_list
	@param rows

 **/
static dsql_nod* pass1_union( CStatement* request, dsql_nod* input,
	dsql_nod* order_list, dsql_nod* rows, USHORT flags)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(order_list, dsql_type_nod);

	//TSQL tdsql = GET_THREAD_DATA;

	// set up the rse node for the union.
	dsql_nod* union_rse = MAKE_node(request->threadData, nod_rse, e_rse_count);
	dsql_nod* union_node = union_rse->nod_arg[e_rse_streams] =
		MAKE_node(request->threadData, nod_union, input->nod_count);

	// process all the sub-rse's.
	{ // scope block
	dsql_nod** uptr = union_node->nod_arg;
	void* base = request->context.mark();
	dsql_nod** ptr = input->nod_arg;
	
	for (const dsql_nod* const* const end = ptr + input->nod_count; ptr < end;
		 ++ptr, ++uptr)
		{
		request->scopeLevel++;
		*uptr = pass1_rse(request, *ptr, NULL, NULL, NULL, 0);
		request->scopeLevel--;
        while (!request->context.isMark (base)) 
            request->unionContext.push(request->context.pop());
		}
		
	} // end scope block

	// generate a context for the union itself.
	dsql_ctx* union_context = FB_NEW(*request->threadData->tsql_default) dsql_ctx;
	union_context->ctx_context = request->contextNumber++;

	// generate the list of fields to select.
	dsql_nod* items = union_node->nod_arg[0]->nod_arg[e_rse_items];

	// loop through the list nodes, checking to be sure that they have the 
	// same number of items

	{ // scope block
	for (int i = 1; i < union_node->nod_count; i++) {
		const dsql_nod* nod1 = union_node->nod_arg[i]->nod_arg[e_rse_items];
		if (items->nod_count != nod1->nod_count) {
			ERRD_post(isc_sqlerr, isc_arg_number, -104,
					  isc_arg_gds, isc_dsql_command_err,
					  isc_arg_gds, isc_dsql_count_mismatch,	// overload of msg 
					  0);
		}
	}
	} // end scope block

	// Comment below is belongs to the old code (way a union was handled).

		/* SQL II, section 9.3, pg 195 governs which data types
		 * are considered equivalent for a UNION
		 * The following restriction is in some ways more restrictive
		 *  (cannot UNION CHAR with VARCHAR for instance)
		 *  (or cannot union CHAR of different lengths)
		 * and in someways less restrictive
		 *  (SCALE is not looked at)
		 * Workaround: use a direct CAST() statement in the SQL
		 * statement to force desired datatype.
		 */

	// loop through the list nodes and cast whenever possible.
	dsql_nod* tmp_list = MAKE_node(request->threadData, nod_list, union_node->nod_count);
	for (int j = 0; j < items->nod_count; j++) {
	    int i; // please the MS compiler
		for (i = 0; i < union_node->nod_count; i++) {
			dsql_nod* nod1 = union_node->nod_arg[i]->nod_arg[e_rse_items];
			MAKE_desc(request->threadData, &nod1->nod_arg[j]->nod_desc, nod1->nod_arg[j], NULL);
			tmp_list->nod_arg[i] = nod1->nod_arg[j];

			// We look only at the items->nod_arg[] when creating the
			// output descriptors. Make sure that the sub-rses
			// descriptor flags are copied onto items->nod_arg[]->nod_desc.
			// Bug 65584.
			// -Sudesh 07/28/1999.
			if (i > 0) {
				if (nod1->nod_arg[j]->nod_desc.dsc_flags & DSC_nullable) {
					items->nod_arg[j]->nod_desc.dsc_flags |= DSC_nullable;
				}
			}
		}
		dsc desc;
		MAKE_desc_from_list(request->threadData, &desc, tmp_list, NULL, "UNION");
		// Only mark upper node as a NULL node when all sub-nodes are NULL
		items->nod_arg[j]->nod_desc.dsc_flags &= ~DSC_null;
		items->nod_arg[j]->nod_desc.dsc_flags |= (desc.dsc_flags & DSC_null);
		for (i = 0; i < union_node->nod_count; i++) {
			pass1_union_auto_cast(request, union_node->nod_arg[i], desc, j);
		}
	}
	items = union_node->nod_arg[0]->nod_arg[e_rse_items];

	// Create mappings for union.
	dsql_nod* union_items = MAKE_node(request->threadData, nod_list, items->nod_count);
	{ // scope block
	SSHORT count = 0;
	dsql_nod** uptr = items->nod_arg;
	dsql_nod** ptr = union_items->nod_arg;
	for (const dsql_nod* const* const end = ptr + union_items->nod_count;
		 ptr < end; ptr++)
	{
	    dsql_nod* map_node = MAKE_node(request->threadData, nod_map, e_map_count);
		*ptr = map_node;
		map_node->nod_arg[e_map_context] = (dsql_nod*) union_context;
		dsql_map* map = FB_NEW(*request->threadData->tsql_default) dsql_map;
		map_node->nod_arg[e_map_map] = (dsql_nod*) map;

		// set up the dsql_map* between the sub-rses and the union context.
		map->map_position = count++;
		map->map_node = *uptr++;
		map->map_next = union_context->ctx_map;
		union_context->ctx_map = map;
	}
	union_rse->nod_arg[e_rse_items] = union_items;
	} // end scope block

	// Process ORDER clause, if any.
	if (order_list) 
		{
		dsql_nod* sort = MAKE_node(request->threadData, nod_list, order_list->nod_count);
		dsql_nod** uptr = sort->nod_arg;
		dsql_nod** ptr = order_list->nod_arg;
		
		for (const dsql_nod* const* const end = ptr + order_list->nod_count;
			 ptr < end; ptr++, uptr++)
			{
			dsql_nod* order1 = *ptr;
			dsql_str* collate = 0;
			const dsql_nod* position = order1->nod_arg[e_order_field];

			if (position->nod_type == nod_collate)
				 {
				collate = (dsql_str*) position->nod_arg[e_coll_target];
				position = position->nod_arg[e_coll_source];
				}

			if (position->nod_type != nod_constant ||
				position->nod_desc.dsc_dtype != dtype_long)
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						  isc_arg_gds, isc_dsql_command_err,
						  isc_arg_gds, isc_order_by_err,	// invalid ORDER BY clause.
						  0);
				
			const SLONG number = *(SLONG *) (position->nod_arg); //(long) position->nod_arg[0];
			
			if (number < 1 || number > union_items->nod_count) 
				ERRD_post(isc_sqlerr, isc_arg_number, -104,
						  isc_arg_gds, isc_dsql_command_err,
						  isc_arg_gds, isc_order_by_err,	// invalid ORDER BY clause.
						  0);

			// make a new order node pointing at the Nth item in the select list.
			
			dsql_nod* order2 = MAKE_node(request->threadData, nod_order, e_order_count);
			*uptr = order2;
			order2->nod_arg[e_order_field] = union_items->nod_arg[number - 1];
			order2->nod_arg[e_order_flag] = order1->nod_arg[e_order_flag];
			
			if (collate) 
				order2->nod_arg[e_order_field] = pass1_collate(request, order2->nod_arg[e_order_field], collate);

			order2->nod_arg[e_order_nulls] = order1->nod_arg[e_order_nulls];
			}
			
		union_rse->nod_arg[e_rse_sort] = sort;
		}

	if (rows) 
		{
		/* CVC: This line is a hint because set_parameter_type() doesn't receive
           the dialect currently as an argument. */
        rows->nod_desc.dsc_scale = request->req_client_dialect;

        if (rows->nod_arg[e_rows_length]) 
			{
            dsql_nod* sub = PASS1_node(request, rows->nod_arg[e_rows_length], false);
            union_rse->nod_arg[e_rse_first] = sub;
            set_parameter_type(request, sub, rows, false);
			}
			
        if (rows->nod_arg[e_rows_skip]) 
			{
            dsql_nod* sub = PASS1_node(request, rows->nod_arg[e_rows_skip], false);
            union_rse->nod_arg[e_rse_skip] = sub;
            set_parameter_type(request, sub, rows, false);
			}
		}

	// PROJECT on all the select items unless UNION ALL was specified.
	
	if (!(input->nod_flags & NOD_UNION_ALL)) 
		union_rse->nod_arg[e_rse_reduced] = union_items;

	union_rse->nod_flags = flags;
	return union_rse;
}


/**
  
 	pass1_union_auto_cast
  
    @brief	Auto cast types to the same type by the rules from 
	MAKE_desc_from_list. SELECT X1 FROM .. UNION SELECT X2 FROM ..
	Items X1..Xn are collected together to make the cast-descriptor, this
	was done by the caller (param desc and input is the collection).
	Then is a cast generated (or reused) for every X item if it has
	another descriptor than the param desc.
	Position tells us which column-nr we are processing.

    @param input
    @param desc
    @param position
    @param in_select_list

 **/
static void pass1_union_auto_cast(CStatement* request, dsql_nod* input, const dsc& desc,
	SSHORT position, bool in_select_list)
{
	DEV_BLKCHK(input, dsql_type_nod);

	switch (input->nod_type) {

		case nod_list:
		case nod_union:
			if (in_select_list) {
				if ((position < 0) || (position >= input->nod_count)) {
					// Internal dsql error: position out of range.
					//
					// !!! THIS MESSAGE SHOULD BE CHANGED !!!
					//
					ERRD_post(isc_sqlerr, isc_arg_number, -104,
						  isc_arg_gds, isc_dsql_command_err, 0);
				}
				else {
					dsql_nod* select_item = input->nod_arg[position];
					if ((select_item->nod_desc.dsc_dtype != desc.dsc_dtype) ||
						(select_item->nod_desc.dsc_length != desc.dsc_length) ||
						(select_item->nod_desc.dsc_scale != desc.dsc_scale) ||
						(select_item->nod_desc.dsc_sub_type != desc.dsc_sub_type))
					{

						// Because this select item has a different descriptor then
						// our finally descriptor CAST it.
						dsql_nod* cast_node = NULL;
						dsql_nod* alias_node = NULL;

						// Pick a existing cast if available else make a new one.
						if ((select_item->nod_type == nod_alias) &&
							 (select_item->nod_arg[e_alias_value]) &&
							 (select_item->nod_arg[e_alias_value]->nod_type == nod_cast))
							cast_node = select_item->nod_arg[e_alias_value];
						else if ((select_item->nod_type == nod_derived_field) &&
							(select_item->nod_arg[e_derived_field_value]) &&
							(select_item->nod_arg[e_derived_field_value]->nod_type == nod_cast))
							cast_node = select_item->nod_arg[e_derived_field_value];
						else if (select_item->nod_type == nod_cast)
							cast_node = select_item;
						else {
							cast_node = MAKE_node(request->threadData, nod_cast, e_cast_count);
							dsql_fld* afield = new dsql_fld;
							cast_node->nod_arg[e_cast_target] = (dsql_nod*) afield;
							
							// We want to leave the ALIAS node on his place, because a UNION
							// uses the select_items from the first sub-rse to determine the
							// columnname.
							// We want to leave the ALIAS node on his place, because a UNION
							// uses the select_items from the first sub-rse to determine the
							// columnname.
							if (select_item->nod_type == nod_alias) 
								cast_node->nod_arg[e_cast_source] = select_item->nod_arg[e_alias_value];
							else if (select_item->nod_type == nod_derived_field)
								cast_node->nod_arg[e_cast_source] = select_item->nod_arg[e_derived_field_value];
							else 
								cast_node->nod_arg[e_cast_source] = select_item;							

							// When a cast is created we're losing our fieldname, thus
							// create a alias to keep it.
							if (select_item->nod_type == nod_field) {
								dsql_fld* sub_field = (dsql_fld*) select_item->nod_arg[e_fld_field];
								// Create new node for alias and copy fieldname
								alias_node = MAKE_node(request->threadData, nod_alias, e_alias_count);
								// Copy fieldname to a new string.
								dsql_str* str_alias = FB_NEW_RPT(*request->threadData->tsql_default, 
									strlen(sub_field->fld_name)) dsql_str;
								strcpy(str_alias->str_data, sub_field->fld_name);
								str_alias->str_length = strlen(sub_field->fld_name);
								alias_node->nod_arg[e_alias_alias] = (dsql_nod*) str_alias;
							}
						}							
						
						dsql_fld* field = (dsql_fld*) cast_node->nod_arg[e_cast_target];
						
						// Copy the descriptor to a field, because the gen_cast
						// uses a dsql field type.
						
						field->fld_dtype = desc.dsc_dtype;
						field->fld_scale = desc.dsc_scale;
						field->fld_sub_type = desc.dsc_sub_type;
						field->fld_length = desc.dsc_length;
						field->fld_flags = (desc.dsc_flags & DSC_nullable) ? FLD_nullable : 0;
						
						if (desc.dsc_dtype <= dtype_any_text) 
							{
							field->fld_ttype = desc.dsc_sub_type;
							field->fld_character_set_id = INTL_GET_CHARSET(&desc);
							field->fld_collation_id = INTL_GET_COLLATE(&desc);
							}
						else if (desc.dsc_dtype == dtype_blob) 
							field->fld_character_set_id = desc.dsc_scale;

						// Finally copy the descriptors to the root nodes and swap
						// the necessary nodes.
						
						cast_node->nod_desc = desc;
						
						if (select_item->nod_desc.dsc_flags & DSC_nullable) 
							cast_node->nod_desc.dsc_flags |= DSC_nullable;

						if (select_item->nod_type == nod_alias) 
							{
							select_item->nod_arg[e_alias_value] = cast_node;
							select_item->nod_desc = desc;
							}
						else if (select_item->nod_type == nod_derived_field) 
							{
							select_item->nod_arg[e_derived_field_value] = cast_node;
							select_item->nod_desc = desc;
							}
						else 
							{ 
							// If a new alias was created for keeping original field-name
							// make the alias the "top" node.
							if (alias_node) 
								{
								alias_node->nod_arg[e_alias_value] = cast_node;
								alias_node->nod_desc = cast_node->nod_desc;
								input->nod_arg[position] = alias_node;
								}
							else 
								{
								input->nod_arg[position] = cast_node;
								}
							}
						}
					}
				}
			else 
				{
				dsql_nod** ptr = input->nod_arg;
				
				for (const dsql_nod* const* const end = ptr + input->nod_count; ptr < end; ptr++)
					pass1_union_auto_cast(request, *ptr, desc, position);
				}
		break;

		case nod_rse:
			{
			dsql_nod* streams = input->nod_arg[e_rse_streams];
			pass1_union_auto_cast(request, streams, desc, position);
			
			if (streams->nod_type == nod_union) 
				{
				// We're now in a UNION under a UNION so don't change the existing mappings.
				// Only replace the node where the map points to, because they could be changed.
				
				dsql_nod* union_items = input->nod_arg[e_rse_items];
				dsql_nod* sub_rse_items = streams->nod_arg[0]->nod_arg[e_rse_items];
				dsql_map* map = (dsql_map*) union_items->nod_arg[position]->nod_arg[e_map_map];
				map->map_node = sub_rse_items->nod_arg[position];
				union_items->nod_arg[position]->nod_desc = desc;
				}
			else 
				pass1_union_auto_cast(request, input->nod_arg[e_rse_items], desc, position, true);
			}
		break;


		default:
			// Nothing
		;
	}
}


/**
  
 	pass1_update
  
    @brief	Process UPDATE statement.
 

    @param request
    @param input

 **/
static dsql_nod* pass1_update( CStatement* request, dsql_nod* input)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

	dsql_nod* cursor = input->nod_arg[e_upd_cursor];
	dsql_nod* relation = input->nod_arg[e_upd_relation];
	if (cursor && (request->flags & REQ_procedure)) {
		dsql_nod* anode = MAKE_node(request->threadData, nod_modify_current, e_mdc_count);
		anode->nod_arg[e_mdc_context] =
			(dsql_nod*) pass1_cursor_context(request, cursor, relation);
		anode->nod_arg[e_mdc_update] = PASS1_node(request, relation, false);
		anode->nod_arg[e_mdc_statement] =
			PASS1_statement(request, input->nod_arg[e_upd_statement], false);
		request->context.pop();
		return anode;
	}

	request->req_type = (cursor) ? REQ_UPDATE_CURSOR : REQ_UPDATE;
	
	dsql_nod* node = MAKE_node(request->threadData, nod_modify, e_mod_count);
	node->nod_arg[e_mod_update] = PASS1_node(request, relation, false);
	node->nod_arg[e_mod_statement] =
		PASS1_statement(request, input->nod_arg[e_upd_statement], false);

	set_parameters_name(node->nod_arg[e_mod_statement],
						node->nod_arg[e_mod_update]);

// Generate record selection expression 

	dsql_nod* rse;
	if (cursor)
		rse = pass1_cursor_reference(request, cursor, relation);
	else {
		rse = MAKE_node(request->threadData, nod_rse, e_rse_count);
		dsql_nod* temp = MAKE_node(request->threadData, nod_list, 1);
		rse->nod_arg[e_rse_streams] = temp;
		temp->nod_arg[0] = node->nod_arg[e_mod_update];

		if ( (temp = input->nod_arg[e_upd_boolean]) ) {
			rse->nod_arg[e_rse_boolean] = PASS1_node(request, temp, false);
		}

		if ( (temp = input->nod_arg[e_upd_plan]) ) {
			rse->nod_arg[e_rse_plan] = PASS1_node(request, temp, false);
		}

		if ( (temp = input->nod_arg[e_upd_sort]) ) {
			rse->nod_arg[e_rse_sort] = pass1_sort(request, temp, NULL);
		}

		if ( (temp = input->nod_arg[e_upd_rows]) ) {
			rse->nod_arg[e_rse_first] =
				PASS1_node(request, temp->nod_arg[e_rows_length], false);
			rse->nod_arg[e_rse_skip] =
				PASS1_node(request, temp->nod_arg[e_rows_skip], false);
		}
	}

	node->nod_arg[e_mod_source] = rse->nod_arg[e_rse_streams]->nod_arg[0];
	node->nod_arg[e_mod_rse] = rse;

	request->context.pop();
	return node;
}



/**
   check_field_type
**/

static bool check_field_type (const CStatement* request, const dsql_fld* field)
{
	// Intercept any reference to a field with datatype that
	// did not exist prior to V6 and post an error

	if (request->req_client_dialect <= SQL_DIALECT_V5 &&
		(field->fld_dtype == dtype_sql_date ||
		field->fld_dtype == dtype_sql_time ||
		field->fld_dtype == dtype_int64))
		{
			ERRD_post(isc_sqlerr, isc_arg_number,  -206,
					isc_arg_gds, isc_dsql_field_err,
					isc_arg_gds, isc_random,
					isc_arg_string,  (const char*) field->fld_name,
					isc_arg_gds, isc_sql_dialect_datatype_unsupport,
					isc_arg_number, request->req_client_dialect,
					isc_arg_string, DSC_dtype_tostring(field->fld_dtype), 0);
			return false;
		}
	return true;
}

/**
	resolve_variable_name

 **/
static dsql_nod* resolve_variable_name(const dsql_nod* var_nodes, const dsql_str* var_name)
{
	dsql_nod* const* ptr = var_nodes->nod_arg;
	dsql_nod* const* const end = ptr + var_nodes->nod_count;

	for (; ptr < end; ptr++) 
		{
		dsql_nod* var_node = *ptr;
		if (var_node->nod_type == nod_variable)
			{
			const var* variable = (var*) var_node->nod_arg[e_var_variable];
			if (!strcmp(reinterpret_cast<const char*>(var_name->str_data), 
				variable->var_name)) 
				{
				return var_node;
				}
			}
		}
	return NULL;
}


/**
  
 	pass1_variable
  
    @brief	Resolve a variable name to an available variable.
 

    @param request
    @param input

 **/
static dsql_nod* pass1_variable( CStatement* request, dsql_nod* input)
{
	// CVC: I commented this variable and its usage because it wasn't useful for
	// anything. I didn't delete it in case it's an implementation in progress
	// by someone.
	//SSHORT position;

	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(input, dsql_type_nod);

    dsql_str* var_name = 0;
	if (input->nod_type == nod_field_name) {
		if (input->nod_arg[e_fln_context]) {
			if (request->flags & REQ_trigger)
				return pass1_field(request, input, false, NULL);
			else
				field_error(0, 0, input);
		}
		var_name = (dsql_str*) input->nod_arg[e_fln_name];
	}
	else
		var_name = (dsql_str*) input->nod_arg[e_vrn_name];

	DEV_BLKCHK(var_name, dsql_type_str);

    dsql_nod* var_nodes;
	if (request->flags & REQ_procedure) {
		dsql_nod* procedure_node = request->ddlNode;
		fb_assert(procedure_node);
		if (!(request->flags & REQ_trigger)) {
			// try to resolve variable name against input and output parameters
			var_nodes = procedure_node->nod_arg[e_prc_inputs];
			if (var_nodes)
			{
				//position = 0;
				dsql_nod** ptr = var_nodes->nod_arg;
				for (const dsql_nod* const* const end =
					ptr + var_nodes->nod_count; ptr < end; ptr++) //, position++)
				{
					dsql_nod* var_node = *ptr;
					const var* variable = (var*) var_node->nod_arg[e_var_variable];
					DEV_BLKCHK(variable, dsql_type_var);
					if (!strcmp
						(reinterpret_cast<const char*>(var_name->str_data),
						 variable->var_name))
					{
						return var_node;
					}
				}
			}
			var_nodes = procedure_node->nod_arg[e_prc_outputs];
			if (var_nodes)
			{
				//position = 0;
				dsql_nod** ptr = var_nodes->nod_arg;
				for (const dsql_nod* const* const end =
					ptr + var_nodes->nod_count; ptr < end; ptr++) //, position++)
				{
					dsql_nod* var_node = *ptr;
					const var* variable = (var*) var_node->nod_arg[e_var_variable];
					DEV_BLKCHK(variable, dsql_type_var);
					if (!strcmp
						(reinterpret_cast<const char*>(var_name->str_data),
						 variable->var_name))
					{
						return var_node;
					}
				}
			}
			var_nodes = procedure_node->nod_arg[e_prc_dcls];
		}
		else
			var_nodes = procedure_node->nod_arg[e_trg_actions]->nod_arg[e_trg_act_dcls];

		if (var_nodes)
		{
			// try to resolve variable name against local variables
			//position = 0;
			dsql_nod** ptr = var_nodes->nod_arg;
			for (const dsql_nod* const* const end =
				ptr + var_nodes->nod_count; ptr < end; ptr++) //, position++)
			{
				dsql_nod* var_node = *ptr;
				if (var_node->nod_type == nod_variable)
				{
					const var* variable = (var*) var_node->nod_arg[e_var_variable];
					DEV_BLKCHK(variable, dsql_type_var);
					if (!strcmp
						(reinterpret_cast<const char*>(var_name->str_data),
						 variable->var_name))
					{
						return var_node;
					}
				}
			}
		}
	}

	if (request->execBlockNode) {
		dsql_nod* var_node;

		if (var_nodes = request->execBlockNode->nod_arg[e_exe_blk_dcls])
			if (var_node = resolve_variable_name(var_nodes, var_name))
				return var_node;

		if (var_nodes = request->execBlockNode->nod_arg[e_exe_blk_inputs])
			if (var_node = resolve_variable_name(var_nodes, var_name))
				return var_node;

		if (var_nodes = request->execBlockNode->nod_arg[e_exe_blk_outputs])
			if (var_node = resolve_variable_name(var_nodes, var_name))
				return var_node;
	}

    // field unresolved 
    // CVC: That's all [the fix], folks!

    if (var_name)
        field_error(0, (TEXT*) var_name->str_data, input);
    else 
        field_error(0, 0, input);

	return NULL;
}


/**
  
 	post_map
  
    @brief	Post an item to a map for a context.
 

    @param node
    @param context

 **/
static dsql_nod* post_map(CStatement* request,  dsql_nod* node, dsql_ctx* context)
{
	DEV_BLKCHK(node, dsql_type_nod);
	DEV_BLKCHK(context, dsql_type_ctx);

	//TSQL tdsql = GET_THREAD_DATA;

// Check to see if the item has already been posted 

	int count = 0;
	dsql_map* map;
	for (map = context->ctx_map; map; map = map->map_next, ++count)
		if (node_match(node, map->map_node, false))
			break;

	if (!map) {
		map = FB_NEW(*request->threadData->tsql_default) dsql_map;
		map->map_position = (USHORT) count;
		map->map_next = context->ctx_map;
		context->ctx_map = map;
		map->map_node = node;
	}

	dsql_nod* new_node = MAKE_node(request->threadData, nod_map, e_map_count);
	new_node->nod_count = 0;
	new_node->nod_arg[e_map_context] = (dsql_nod*) context;
	new_node->nod_arg[e_map_map] = (dsql_nod*) map;
	new_node->nod_desc = node->nod_desc;

	return new_node;
}


/**
  
 	remap_field
  
    @brief	Called to map fields used in an aggregate-context
	after all pass1 calls (SELECT-, ORDER BY-lists).	
    Walk completly through the given node 'field' and
	map the fields with same scope_level as the given context
	to the given context with the post_map function.
	

    @param request
    @param field
    @param context
    @param current_level

 **/
static dsql_nod* remap_field(CStatement* request, dsql_nod* field,
	dsql_ctx* context, USHORT current_level)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(field, dsql_type_nod);
	DEV_BLKCHK(context, dsql_type_ctx);

	switch (field->nod_type) {

		case nod_alias:
			field->nod_arg[e_alias_value] = 
				remap_field(request, field->nod_arg[e_alias_value], context, current_level); 
			return field;

		case nod_derived_field:
			{
				// If we got a field from a derived table we should not remap anything
				// deeper in the alias, but this "virtual" field should be mapped to
				// the give context (ofcourse only if we're in the same scope-level).
				const USHORT lscope_level =
					(USHORT)(U_IPTR)field->nod_arg[e_derived_field_scope];
				if (lscope_level == context->ctx_scope_level) {
					return post_map(request, field, context);
				}
				else if (context->ctx_scope_level < lscope_level) {
					field->nod_arg[e_derived_field_value] = 
						remap_field(request, field->nod_arg[e_derived_field_value], 
							context, current_level);
				}
				return field;
			}

		case nod_field:
			{
				const dsql_ctx* lcontext =
					reinterpret_cast<dsql_ctx*>(field->nod_arg[e_fld_context]);
				if (lcontext->ctx_scope_level == context->ctx_scope_level) {
					return post_map(request, field, context);
				}
				else {
					return field;
				}
			}

		case nod_map:
			{
				dsql_ctx* lcontext =
					reinterpret_cast<dsql_ctx*>(field->nod_arg[e_map_context]);
				if (lcontext->ctx_scope_level != context->ctx_scope_level) {
					dsql_map* lmap = reinterpret_cast<dsql_map*>(field->nod_arg[e_map_map]);
					lmap->map_node = remap_field(request, lmap->map_node, context,
						lcontext->ctx_scope_level);
				}
				return field;
			}

		case nod_agg_count:
		case nod_agg_min:
		case nod_agg_max:
		case nod_agg_average:
		case nod_agg_total:
		case nod_agg_average2:
		case nod_agg_total2:
			{
				USHORT ldeepest_level = request->scopeLevel;
				USHORT lcurrent_level = current_level;
				if (aggregate_found2(request, field, &lcurrent_level, &ldeepest_level, false)) {
					if (request->scopeLevel == ldeepest_level) {
						return post_map(request, field, context);
					}
					else {
						if (field->nod_count) {
							field->nod_arg[e_agg_function_expression] =
								 remap_field(request, field->nod_arg[e_agg_function_expression], 
								 context, current_level);
						}
						return field;
					}
				}
				else {
					if (field->nod_count) {
						field->nod_arg[e_agg_function_expression] =
							 remap_field(request, field->nod_arg[e_agg_function_expression], 
							 context, current_level);
					}
					return field;
				}
			}
 			
		case nod_via:
			field->nod_arg[e_via_rse] =
				remap_field(request, field->nod_arg[e_via_rse], context, current_level);
			field->nod_arg[e_via_value_1] = field->nod_arg[e_via_rse]->nod_arg[e_rse_items]->nod_arg[0];
			return field;

		case nod_rse:
			current_level++;
			field->nod_arg[e_rse_streams] = 
				remap_field(request, field->nod_arg[e_rse_streams], context, current_level);
			if (field->nod_arg[e_rse_boolean]) {
				field->nod_arg[e_rse_boolean] = 
					remap_field(request, field->nod_arg[e_rse_boolean], context, current_level);
			}
			if (field->nod_arg[e_rse_items]) {
				field->nod_arg[e_rse_items] = 
					remap_field(request, field->nod_arg[e_rse_items], context, current_level);
			}
			current_level--;
			return field;

		case nod_coalesce:
		case nod_simple_case:
		case nod_searched_case:
			{
				dsql_nod** ptr = field->nod_arg;
				for (const dsql_nod* const* const end = ptr + field->nod_count;
					ptr < end; ptr++)
				{
					*ptr = remap_field(request, *ptr, context, current_level);
				}
				return field;
			}

		case nod_aggregate:
			field->nod_arg[e_agg_rse] = 
				remap_field(request, field->nod_arg[e_agg_rse], context, current_level);
			return field;

		case nod_order:
			field->nod_arg[e_order_field] = 
				remap_field(request, field->nod_arg[e_order_field], context, current_level);
			return field;

		case nod_or:
		case nod_and:
		case nod_not:
		case nod_equiv:
		case nod_eql:
		case nod_neq:
		case nod_gtr:
		case nod_geq:
		case nod_lss:
		case nod_leq:
		case nod_eql_any:
		case nod_neq_any:
		case nod_gtr_any:
		case nod_geq_any:
		case nod_lss_any:
		case nod_leq_any:
		case nod_eql_all:
		case nod_neq_all:
		case nod_gtr_all:
		case nod_geq_all:
		case nod_lss_all:
		case nod_leq_all:
		case nod_any:
		case nod_ansi_any:
		case nod_ansi_all:
		case nod_between:
		case nod_like:
		case nod_containing:
		case nod_starting:
		case nod_exists:
		case nod_singular:
		case nod_missing:
		case nod_add:
		case nod_add2:
		case nod_concatenate:
		case nod_divide:
		case nod_divide2:
		case nod_multiply:
		case nod_multiply2:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_subtract2:
		case nod_upcase:
		case nod_internal_info:
		case nod_extract:
		case nod_list:
		case nod_join:
		case nod_join_inner:
		case nod_join_left:
		case nod_join_right:
		case nod_join_full:
			{
				dsql_nod** ptr = field->nod_arg;
				for (const dsql_nod* const* const end = ptr + field->nod_count;
					ptr < end; ptr++)
				{
					*ptr = remap_field(request, *ptr, context, current_level);
				}
				return field;
			}

		case nod_cast:
		case nod_gen_id:
		case nod_gen_id2:
		case nod_udf:
			if (field->nod_count == 2) {
				field->nod_arg[1] = remap_field(request, field->nod_arg[1],
					context, current_level);
			}
			return field;

		case nod_relation:
			{
				dsql_ctx* lrelation_context =
					reinterpret_cast<dsql_ctx*>(field->nod_arg[e_rel_context]);
				// Check if relation is a procedure
				if (lrelation_context->ctx_procedure) {
					// If input parameters exists remap those
					if (lrelation_context->ctx_proc_inputs) {
						lrelation_context->ctx_proc_inputs =
							remap_field(request, lrelation_context->ctx_proc_inputs, context, current_level);
					}
				}
				return field;
			}
	
		case nod_derived_table:
			remap_field(request, field->nod_arg[e_derived_table_rse], context, current_level);
			return field;

		default:
			return field;
	}
}


/**
  
 	remap_fields
  
    @brief	Remap fields inside a field list against 
	an artificial context.
 

    @param request
    @param fields
    @param context

 **/
static dsql_nod* remap_fields(CStatement* request, dsql_nod* fields, dsql_ctx* context)
{
	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(fields, dsql_type_nod);
	DEV_BLKCHK(context, dsql_type_ctx);

	if (fields->nod_type == nod_list) {
		for (int i = 0; i < fields->nod_count; i++) {
			fields->nod_arg[i] = remap_field(request, fields->nod_arg[i], context,
				request->scopeLevel);
		}
	}
	else {
		fields = remap_field(request, fields, context, request->scopeLevel);
	}

	return fields;
}


/**
  
 remap_streams_to_parent_context
  
    @brief	For each relation in the list, flag the relation's context
 	as having a parent context.  Be sure to handle joins
 	(Bug 6674).
 

    @param input
    @param parent_context

 **/
static void remap_streams_to_parent_context( dsql_nod* input, dsql_ctx* parent_context)
{
	DEV_BLKCHK(input, dsql_type_nod);
	DEV_BLKCHK(parent_context, dsql_type_ctx);

	switch (input->nod_type) {
	case nod_list:
		{
		    dsql_nod** ptr = input->nod_arg;
			for (const dsql_nod* const* const end = ptr + input->nod_count;
				ptr < end; ptr++)
			{
				remap_streams_to_parent_context(*ptr, parent_context);
				}
		break;
		}

	case nod_relation:
		{
			dsql_ctx* context = (dsql_ctx*) input->nod_arg[e_rel_context];
			DEV_BLKCHK(context, dsql_type_ctx);
			context->ctx_parent = parent_context;
			break;
		}

	case nod_join:
		remap_streams_to_parent_context(input->nod_arg[e_join_left_rel],
										parent_context);
		remap_streams_to_parent_context(input->nod_arg[e_join_rght_rel],
										parent_context);
		break;

	case nod_derived_table:
		// nothing to do here.
		break;

	default:
		fb_assert(false);
		break;
	}
}


/**
  
 	resolve_context
  
    @brief	Attempt to resolve field against context.  
 	Return first field in context if successful, 
 	NULL if not.
 

    @param request
    @param name
    @param qualifier
    @param context

 **/
static dsql_fld* resolve_context( CStatement* request, dsql_str* qualifier, 
								dsql_ctx* context, bool isCheckConstraint)
{
	// CVC: Warning: the second param, "name" was is not used anymore and
	// therefore it was removed. Thus, the local variable "table_name"
	// is being stripped here to avoid mismatches due to trailing blanks.

	//DEV_BLKCHK(request, dsql_type_req);
	DEV_BLKCHK(qualifier, dsql_type_str);
	DEV_BLKCHK(context, dsql_type_ctx);

	dsql_rel* relation = context->ctx_relation;
	Procedure* procedure = context->ctx_procedure;
	
	if (!relation && !procedure) 
		return NULL;

// if there is no qualifier, then we cannot match against
// a context of a different scoping level
// AB: Yes we can, but the scope level where the field is has priority.
//	if (!qualifier && context->ctx_scope_level != request->req_scope_level) {
//		return NULL;
//	}

	// AB: If this context is a system generated context as in NEW/OLD inside 
	// triggers, the qualifier by the field is mandatory. While we can't 
	// fall back from a higher scope-level to the NEW/OLD contexts without 
	// the qualifier present.
	// An exception is a check-constraint that is allowed to reference fields
	// without the qualifier.
	if (!isCheckConstraint && (context->ctx_flags & CTX_system) && (!qualifier)) {
		return NULL;
	}

	const TEXT* table_name = NULL;
	if (context->ctx_internal_alias) 
		table_name = context->ctx_internal_alias;

// AB: For a check constraint we should ignore the alias if the alias
//     contains the "NEW" alias. This is because it is possible
//     to reference a field by the complete table-name as alias 
//     (see EMPLOYEE table in examples for a example).

	if (isCheckConstraint && table_name) 
		{
		// If a qualifier is present and it's equal to the alias then we've already the right table-name
		// Only use the OLD context if it is explicit used. That means the 
		// qualifer should hold the "OLD" alias.
		if (!(qualifier && !strcmp(reinterpret_cast<const char*>(qualifier->str_data), table_name))) 
			{
			if (!strcmp(table_name, NEW_CONTEXT)) 
				table_name = NULL;
			else if (!strcmp(table_name, OLD_CONTEXT))
				return NULL;
			}
		}
	

	if (table_name == NULL) 
		{
		if (relation) 
			table_name = (const char*)relation->rel_name;
		else 
			table_name = (const TEXT*)procedure->findName();		
		}

	//pass_exact_name(table_name);

	/* If a context qualifier is present, make sure this is the
  	   proper context */

	if (qualifier && strcmp(*qualifier, table_name))
		return NULL;

	if (relation)
		return relation->rel_fields;
	
	return (procedure->findOutputParameters())->getDsqlField();
}


/**
  
 	set_parameter_type
  
    @brief	Setup the datatype of a parameter.
 

    @param in_node
    @param node
    @param force_varchar

 **/
static bool set_parameter_type(CStatement* request, dsql_nod* in_node, dsql_nod* node, bool force_varchar)
{
	//DEV_BLKCHK(in_node, dsql_type_nod);
	//DEV_BLKCHK(node, dsql_type_nod);

	if (in_node == NULL)
		return false;

	switch (in_node->nod_type) 
		{
		case nod_parameter:
			{
			if (node->nod_type == nod_dom_value)
				{
				/* Error on value = ?, where value is not quoted */
				ERRD_post(isc_sqlerr, isc_arg_number, (SLONG) - 901,
							isc_arg_gds, isc_dsql_domain_err, 0);    
				
				}
			MAKE_desc(request->threadData, &in_node->nod_desc, node, NULL);
			par* parameter = (par*) in_node->nod_arg[e_par_parameter];
			DEV_BLKCHK(parameter, dsql_type_par);
			parameter->par_desc = in_node->nod_desc;
			parameter->par_node = in_node;

			// Parameters should receive precisely the data that the user
			// passes in.  Therefore for text strings lets use varying
			// strings to insure that we don't add trailing blanks. 

			// However, there are situations this leads to problems - so
			// we use the force_varchar parameter to prevent this
			// datatype assumption from occuring.

			if (force_varchar) 
				{
				if (parameter->par_desc.dsc_dtype == dtype_text) 
					{
					parameter->par_desc.dsc_dtype = dtype_varying;
					parameter->par_desc.dsc_length += sizeof(USHORT);
					}
				else if (parameter->par_desc.dsc_dtype > dtype_any_text) 
					{
					// The LIKE & similar parameters must be varchar type
					// strings - so force this parameter to be varchar
					// and take a guess at a good length for it.
					parameter->par_desc.dsc_dtype = dtype_varying;
					parameter->par_desc.dsc_length = LIKE_PARAM_LEN + sizeof(USHORT);
					parameter->par_desc.dsc_sub_type = 0;
					parameter->par_desc.dsc_scale = 0;
					parameter->par_desc.dsc_ttype = ttype_dynamic;
					}
				}
			return true;
			}

		case nod_cast:
			// Unable to guess parameters within a CAST() statement - as
			// any datatype could be the input to cast.
			return false;

		case nod_add:
		case nod_add2:
		case nod_concatenate:
		case nod_divide:
		case nod_divide2:
		case nod_multiply:
		case nod_multiply2:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_subtract2:
		case nod_upcase:
		case nod_extract:
		case nod_limit:
		case nod_rows:
			{
			bool result = false;
			dsql_nod** ptr = in_node->nod_arg;
			
			for (const dsql_nod* const* const end = ptr + in_node->nod_count; ptr < end; ptr++)
				result |= set_parameter_type(request, *ptr, node, force_varchar);

			return result;
			}

	default:
		return false;
	}
}


/**
  
 set_parameters_name
  
    @brief      Setup parameter parameters name.
 

    @param list_node
    @param rel_node

 **/
static void set_parameters_name( dsql_nod* list_node, const dsql_nod* rel_node)
{
	DEV_BLKCHK(list_node, dsql_type_nod);
	DEV_BLKCHK(rel_node, dsql_type_nod);

	const dsql_ctx* context = (dsql_ctx*) rel_node->nod_arg[0];
	DEV_BLKCHK(context, dsql_type_ctx);
	dsql_rel* relation = context->ctx_relation;

    dsql_nod** ptr = list_node->nod_arg;
	for (const dsql_nod* const* const end = ptr + list_node->nod_count;
		 ptr < end; ptr++)
	{
		DEV_BLKCHK(*ptr, dsql_type_nod);
		if ((*ptr)->nod_type == nod_assign)
			set_parameter_name((*ptr)->nod_arg[0],
							   (*ptr)->nod_arg[1], relation);
		else
			fb_assert(FALSE);
	}
}


/**
  
 set_parameter_name
  
    @brief      Setup parameter parameter name.
 	This function was added as a part of array data type
 	support for InterClient. It is	called when either
 	"insert" or "update" statements are parsed. If the
 	statements have input parameters, than the parameter
 	is assigned the name of the field it is being inserted
 	(or updated). The same goes to the name of a relation.
 	The names are assigned to the parameter only if the
 	field is of array data type.
 

    @param par_node
    @param fld_node
    @param relation

 **/
static void set_parameter_name( dsql_nod* par_node, const dsql_nod* fld_node,
	dsql_rel* relation)
{
	DEV_BLKCHK(par_node, dsql_type_nod);
	DEV_BLKCHK(fld_node, dsql_type_nod);
	//DEV_BLKCHK(relation, dsql_type_dsql_rel);

	/* Could it be something else ??? */
	fb_assert(fld_node->nod_type == nod_field);

	if (fld_node->nod_desc.dsc_dtype != dtype_array)
		return;

	switch (par_node->nod_type) 
		{
		case nod_parameter:
			{
			par* parameter = (par*) par_node->nod_arg[e_par_parameter];
			DEV_BLKCHK(parameter, dsql_type_par);
			dsql_fld* field = (dsql_fld*) fld_node->nod_arg[e_fld_field];
			parameter->par_name = field->fld_name;
			parameter->par_rel_name = relation->rel_name;
			return;
			}

		case nod_add:
		case nod_add2:
		case nod_concatenate:
		case nod_divide:
		case nod_divide2:
		case nod_multiply:
		case nod_multiply2:
		case nod_negate:
		case nod_substr:
		case nod_subtract:
		case nod_subtract2:
		case nod_upcase:
		case nod_extract:
		case nod_limit:
		case nod_rows:
			{
			dsql_nod** ptr = par_node->nod_arg;
			for (const dsql_nod* const* const end = ptr + par_node->nod_count; ptr < end; ptr++)
				set_parameter_name(*ptr, fld_node, relation);
				return;
			}

		default:
			return;
		}
}


/**
  
 pass_exact_name
  
    @brief      Skim trailing blanks from identifiers.
 	CVC: I had to add this function because metd_exact_name
 	usage forced the inclusion of metd.cpp that currently
 	only exposes metadata functions and is a product of metd.epp.
 

    @param str

 **/
static TEXT* pass_exact_name (TEXT* str)
{
	TEXT* p;
	for (p = str; *p; ++p) { // go to end of string
		;
	}

	for (--p; p >= str && *p == '\x20'; --p) { // remove spaces from end
		;
	}

	// Write *++p = 0; if you like instead of two lines.

	++p;
	*p = 0;

	return str;
}


/**

 pass1_savepoint

    @brief      Add savepoint pair of nodes 
				to request having error handlers.


    @param request
	@param node

 **/
static dsql_nod* pass1_savepoint(const CStatement* request, dsql_nod* node) {
	if (request->req_error_handlers) {
		dsql_nod* temp = MAKE_node(request->threadData, nod_list, 3);
		temp->nod_arg[0] = MAKE_node(request->threadData, nod_start_savepoint, 0);
		temp->nod_arg[1] = node;
		temp->nod_arg[2] = MAKE_node(request->threadData, nod_end_savepoint, 0);
		node = temp;
	}
	return node;
}


static dsql_fld* match_procedure_field (Procedure* procedure, 
							  dsql_str *name)
{
#ifdef SHARED_CACHE
	Sync sync (&(procedure->syncOutputs), "match_procedure_field");
	sync.lock (Shared);
#endif

	for (ProcParam* parameter = procedure->findOutputParameters(); 
			parameter; parameter =  parameter->findNext()) 
		if (parameter->isNamed (name->str_data)) 
			return parameter->getDsqlField();

	return NULL;
}


static dsql_fld* match_relation_field (dsql_rel *relation, 
							  dsql_str *name)
{
#ifdef SHARED_CACHE
	Sync sync (&(relation->syncFields), "match_relation_field");
	sync.lock (Shared);
#endif

	for (dsql_fld* field = relation->rel_fields; field; field = field->fld_next) 
		if (name->isEqual(field->fld_name)) 
			return field;

	return NULL;
}


bool dsql_str::isEqual(const char* string)
{
	return strcmp (str_data, string) == 0;
}
