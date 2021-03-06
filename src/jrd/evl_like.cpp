#ifndef EVL_LIKE_INCLUDED_BY_EVL_CPP

#pragma message ("Do not compile this file stand-alone. It gets included by evl.cpp!")

#else // EVL_LIKE_INCLUDED_BY_EVL_CPP

/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		evl_like.cpp
 *	DESCRIPTION:	Expression evaluation (international)
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

#define SLEUTH_insensitive	1
#define COND_UPPER(obj,c)	((flags & SLEUTH_insensitive) ?	(obj).to_upper(c) : (c))

#define SQL_MATCH_ONE		'_'
#define SQL_MATCH_ANY		'%'

#define GDML_MATCH_ONE		'?'
#define GDML_MATCH_ANY		'*'

#define GDML_QUOTE		'@'
#define GDML_NOT		'~'
#define GDML_RANGE		'-'
#define GDML_CLASS_START	'['
#define GDML_CLASS_END		']'
#define GDML_SUBSTITUTE		'='
#define GDML_FLAG_SET		'+'
#define GDML_FLAG_CLEAR		'-'
#define GDML_COMMA		','
#define GDML_LPAREN		'('
#define GDML_RPAREN		')'


USHORT LIKENAME(thread_db* tdbb, TextType obj, const LIKETYPE* p1, SSHORT l1_bytes,	/* byte count */
				const LIKETYPE* p2, SSHORT l2_bytes,	/* byte count */
				UCS2_CHAR escape_char)
{
/**************************************
 *
 *	E V L _ ? ? _ l i k e
 *	E V L _ w c _ l i k e
 *	E V L _ n c _ l i k e
 *
 **************************************
 *
 * Functional description
 *	Return true if a string (p1, l1) matches a given pattern (p2, l2).
 *	The character '_' in the pattern may match any single character
 *	in the the string, and the character '%' may match any sequence
 *	of characters.
 *
 *	Wide char version operates on USHORT-based buffer,
 *	instead of UCHAR-based.
 *
 *	Note Bene: LIKETYPE is defined by including file (evl.c) to either
 *	    UCS2_CHAR or NCHAR, depending on the varient being compiled here.
 *
 *	(escape_char == 0) means no escape character is specified.
 *
 **************************************/
	LIKETYPE c;
	USHORT escape;
	SSHORT l1, l2;

	fb_assert(p1 != NULL);
	fb_assert(p2 != NULL);
	fb_assert((l1_bytes % sizeof(LIKETYPE)) == 0);
	fb_assert((l2_bytes % sizeof(LIKETYPE)) == 0);

	l1 = l1_bytes / sizeof(LIKETYPE);
	l2 = l2_bytes / sizeof(LIKETYPE);

	escape = FALSE;

	while (l2-- > 0) {
		c = *p2++;
		if (escape_char && ((UCS2_CHAR) c == escape_char)) {
			if (l2-- > 0) {
				c = *p2++;
				/* Note: SQL II says <escape_char><escape_char> is error condition */
				if (((UCS2_CHAR) c == escape_char) ||
					((UCS2_CHAR) c == (UCS2_CHAR) SQL_MATCH_ANY) ||
					((UCS2_CHAR) c == (UCS2_CHAR) SQL_MATCH_ONE))
					escape = TRUE;
			}
			if (!escape)
				ERR_post(isc_like_escape_invalid, 0);
		}
		if (!escape && (c == (LIKETYPE) SQL_MATCH_ANY)) {
			while ((l2 > 0) && (*p2 == (LIKETYPE) SQL_MATCH_ANY)) {
				l2--;
				p2++;
			}
			if (l2 == 0)
				return TRUE;
			while (l1)
				if (LIKENAME(tdbb, obj, p1++, l1-- * sizeof(LIKETYPE),
							 p2, l2 * sizeof(LIKETYPE), escape_char))
					return TRUE;
			return FALSE;
		}
		if (l1-- == 0)
			return FALSE;
		if ((escape || c != (LIKETYPE) SQL_MATCH_ONE) && c != *p1)
			return FALSE;
		escape = FALSE;
		p1++;
	}

	return (l1) ? FALSE : TRUE;
}


USHORT MATCHESNAME(thread_db* tdbb,
				   TextType obj,
				   const MATCHESTYPE* p1,
				   SSHORT l1_bytes, const MATCHESTYPE* p2, SSHORT l2_bytes)
{
/**************************************
 *
 *	E V L _ ? ? _ m a t c h e s
 *	E V L _ w c _ m a t c h e s
 *	E V L _ n c _ m a t c h e s
 *
 **************************************
 *
 * Functional description
 *	Return true if a string (p1, l1) matches a given pattern (p2, l2).
 *	The character '?' in the pattern may match any single character
 *	in the the string, and the character '*' may match any sequence
 *	of characters.
 *
 *	Wide SCHAR version operates on short-based buffer,
 *	instead of SCHAR-based.
 *
 * Matches is not a case-sensitive operation, thus it has no
 * 8-bit international impact.
 *
 **************************************/
	MATCHESTYPE c;
	SSHORT l1, l2;

	fb_assert(p1 != NULL);
	fb_assert(p2 != NULL);
	fb_assert((l1_bytes % sizeof(MATCHESTYPE)) == 0);
	fb_assert((l2_bytes % sizeof(MATCHESTYPE)) == 0);

	l1 = l1_bytes / sizeof(MATCHESTYPE);
	l2 = l2_bytes / sizeof(MATCHESTYPE);

	while (l2-- > 0) {
		c = *p2++;
		if (c == (MATCHESTYPE) GDML_MATCH_ANY) {
			while ((l2 > 0) && (*p2 == (MATCHESTYPE) GDML_MATCH_ANY)) {
				l2--;
				p2++;
			}
			if (l2 == 0)
				return TRUE;
			while (l1)
				if (MATCHESNAME(tdbb, obj, p1++,
								l1-- * sizeof(MATCHESTYPE), p2,
								l2 * sizeof(MATCHESTYPE))) return TRUE;
			return FALSE;
		}
		if (l1-- == 0)
			return FALSE;
		if ((c != (MATCHESTYPE) GDML_MATCH_ONE && c != *p1))
			return FALSE;
		p1++;
	}

	return (l1) ? FALSE : TRUE;
}


USHORT SLEUTHNAME(thread_db* tdbb_dummy,
				  TextType obj,
				  USHORT flags,
				  const SLEUTHTYPE* search,
				  USHORT search_len, const SLEUTHTYPE* match, USHORT match_len)
{
/**************************************
 *
 *	E V L _ ? ? _ s l e u t h _ c h e c k
 *
 **************************************
 *
 * Functional description
 *	Evaluate the "sleuth" search operator.
 *
 *	Turn the (pointer, byte length) input parameters into
 *	(pointer, end_pointer) for use in SLEUTH_AUX
 *
 **************************************/
	fb_assert((match_len % sizeof(SLEUTHTYPE)) == 0);
	fb_assert((search_len % sizeof(SLEUTHTYPE)) == 0);

	const SLEUTHTYPE* const end_match = match + (match_len / sizeof(SLEUTHTYPE));
	const SLEUTHTYPE* const end_search = search + (search_len / sizeof(SLEUTHTYPE));

	return ((USHORT)
			SLEUTH_AUX(obj, flags, search, end_search, match, end_match));
}


USHORT SLEUTH_MERGE_NAME(thread_db* tdbb_dummy,
						 TextType obj,
						 const SLEUTHTYPE* match,
						 USHORT match_bytes,
						 const SLEUTHTYPE* control,
						 USHORT control_bytes,
						 SLEUTHTYPE* combined, USHORT combined_bytes)
{
/**************************************
 *
 *	E V L _ ? ? _ s l e u t h _ m e r g e
 *
 **************************************
 *
 * Functional description
 *	Merge the matching pattern and control strings to give a cannonical
 *	matching pattern.  Return the length of the combined string. 
 *
 * 	What this routine does is to take the language template, strip off 
 *	the prefix and put it in the output string, then parse the definitions
 *	into an array of character pointers.  The index array is the defined
 *	character.   The routine then takes the actual match pattern and uses
 *	the characters in it to index into the definitions to produce an equivalent
 *	pattern in the cannonical language.
 *
 *	The silly loop setting *v++ to zero initializes the array up to the
 *	highest character defined (also max_op).  Believe it or not, that part
 *	is not a bug.
 *
 **************************************/
	SLEUTHTYPE *vector[256], **end_vector, *p,
		temp[256];

	fb_assert(match != NULL);
	fb_assert(control != NULL);
	fb_assert(combined != NULL);

	fb_assert((match_bytes % sizeof(SLEUTHTYPE)) == 0);
	fb_assert((control_bytes % sizeof(SLEUTHTYPE)) == 0);

	const SLEUTHTYPE* const end_match = match + (match_bytes / sizeof(SLEUTHTYPE));
	const SLEUTHTYPE* const end_control = control + (control_bytes / sizeof(SLEUTHTYPE));

	SLEUTHTYPE max_op = 0;
	SLEUTHTYPE* comb = combined;
	SLEUTHTYPE** v = vector;
	SLEUTHTYPE* t = temp;

/* Parse control string into substitution strings and initializing string */

	while (control < end_control) {
		SLEUTHTYPE c = *control++;
		if (*control == GDML_SUBSTITUTE) {
			/* Note: don't allow substitution characters larger than vector */
			end_vector = vector + (((int)c < FB_NELEM(vector)) ? c : 0);
			while (v <= end_vector)
				*v++ = 0;
			*end_vector = t;
			++control;
			while (control < end_control) {
				c = *control++;
				if ((t > temp && t[-1] == GDML_QUOTE)
					|| ((c != GDML_COMMA) && (c != GDML_RPAREN)))
				{
					*t++ = c;
				}
				else
					break;
			}
			*t++ = 0;
		}
		else if (c == GDML_QUOTE && control < end_control)
			*comb++ = *control++;
		else if (c == GDML_RPAREN)
			break;
		else if (c != GDML_LPAREN)
			*comb++ = c;
	}

	max_op = v - vector;

/* Interpret matching string, substituting where appropriate */

	while (match < end_match) {
		const SLEUTHTYPE c = *match++;

		/* if we've got a defined character, slurp the definition */

		if (c <= max_op && (p = vector[c])) {
			while (*p)
				*comb++ = *p++;

			/* if we've got the definition of a quote character, 
			   slurp the next character too */

			if (comb > combined && comb[-1] == GDML_QUOTE && *match)
				*comb++ = *match++;
		}

		/* at this point we've got a non-match, but as it might be one of ours, 
		   quote it. */

		else {
			if ((((size_t) c) < sizeof(special)) && special[c] &&
				comb > combined && comb[-1] != GDML_QUOTE)
			{
				*comb++ = GDML_QUOTE;
			}
			*comb++ = c;
		}
	}

/* Put in trailing stuff */

	while (control < end_control)
		*comb++ = *control++;

	/* YYY - need to add code watching for overflow of combined */

	return (comb - combined) * sizeof(SLEUTHTYPE);
}


static BOOLEAN SLEUTH_AUX(
						  TextType obj,
						  USHORT flags,
						  const SLEUTHTYPE* search,
						  const SLEUTHTYPE* end_search,
const SLEUTHTYPE* match, const SLEUTHTYPE* end_match)
{
/**************************************
 *
 *	s l e u t h _ a u x
 *
 **************************************
 *
 * Functional description
 *	Evaluate the "sleuth" search operator.
 *
 **************************************/
	SLEUTHTYPE c, d;

	fb_assert(search != NULL);
	fb_assert(end_search != NULL);
	fb_assert(match != NULL);
	fb_assert(end_match != NULL);
	fb_assert(search <= end_search);
	fb_assert(match <= end_match);

	while (match < end_match) {
		c = *match++;
		if ((c == GDML_QUOTE && (c = *match++)) ||
			((((size_t) c) < sizeof(special)) && !special[c])) {
			c = COND_UPPER(obj, c);
			if (match >= end_match || *match != GDML_MATCH_ANY) {
				if (search >= end_search)
					return FALSE;
				d = *search++;
				if (c != COND_UPPER(obj, d))
					return FALSE;
			}
			else {
				++match;
				for (;;)
					if (SLEUTH_AUX
						(obj, flags, search, end_search, match,
						 end_match)) return TRUE;
					else if (search < end_search) {
						d = *search++;
						if (c != COND_UPPER(obj, d))
							return FALSE;
					}
					else
						return FALSE;
			}
		}
		else if (c == GDML_MATCH_ONE)
			if (match >= end_match || *match != GDML_MATCH_ANY) {
				if (search >= end_search)
					return FALSE;
				search++;
			}
			else {
				if (++match >= end_match)
					return TRUE;
				for (;;)
					if (SLEUTH_AUX
						(obj, flags, search, end_search, match,
						 end_match)) return TRUE;
					else if (++search >= end_search)
						return FALSE;
			}
		else if (c == GDML_CLASS_START) {
			const SLEUTHTYPE* const class_ = match;
			while (*match++ != GDML_CLASS_END) {
				if (match >= end_match)
					return FALSE;
			}
			const SLEUTHTYPE* const end_class = match - 1;
			if (match >= end_match || *match != GDML_MATCH_ANY) {
				if (!SLEUTH_CLASS_NAME
					(obj, flags, class_, end_class, *search++))
					return FALSE;
			}
			else {
				++match;
				for (;;)
					if (SLEUTH_AUX
						(obj, flags, search, end_search, match,
						 end_match)) return TRUE;
					else if (search < end_search) {
						if (!SLEUTH_CLASS_NAME
							(obj, flags, class_, end_class, *search++))
							return FALSE;
					}
					else
						return FALSE;
			}
		}
		else if (c == GDML_FLAG_SET) {
			c = *match++;
			if (c == 's' || c == 'S')
				flags &= ~SLEUTH_insensitive;
		}
		else if (c == GDML_FLAG_CLEAR) {
			c = *match++;
			if (c == 's' || c == 'S')
				flags |= SLEUTH_insensitive;
		}
	}

	if (search < end_search)
		return FALSE;

	return TRUE;
}


static BOOLEAN SLEUTH_CLASS_NAME(
								 TextType obj,
								 USHORT flags,
								 const SLEUTHTYPE* class_,
								 const SLEUTHTYPE* const end_class, 
								 SLEUTHTYPE character)
{
/**************************************
 *
 *	s l e u t h _ c l a s s
 *
 **************************************
 *
 * Functional description
 *	See if a character is a member of a class.
 *	Japanese version operates on short-based buffer,
 *	instead of SCHAR-based.
 *
 **************************************/
	fb_assert(class_ != NULL);
	fb_assert(end_class != NULL);
	fb_assert(class_ <= end_class);

	USHORT result = TRUE;
	character = COND_UPPER(obj, character);

	if (*class_ == GDML_NOT) {
		++class_;
		result = FALSE;
	}

	while (class_ < end_class) {
		const SLEUTHTYPE c = *class_++;
		if (c == GDML_QUOTE) {
			if (*class_++ == character)
				return TRUE;
		}
		else if (*class_ == GDML_RANGE) {
			class_ += 2;
			if (character >= c && character <= class_[-1])
				return result;
		}
		else if (character == c)
			return result;
	}

	return (result) ? FALSE : TRUE;
}


#endif // EVL_LIKE_INCLUDED_BY_EVL_CPP
