/*************************************************************************/
/* Copyright (c) 2008, 2009, 2014 Linas Vepstas                          */
/* All rights reserved                                                   */
/*                                                                       */
/* Use of the link grammar parsing system is subject to the terms of the */
/* license set forth in the LICENSE file included with this software.    */
/* This license allows free redistribution and use in source and binary  */
/* forms, with or without modification, subject to certain conditions.   */
/*                                                                       */
/*************************************************************************/
/*
 * lisjuncts.c
 *
 * Miscellaneous utilities for returning the list of disjuncts that
 * were actually used in a given parse of a sentence.
 */

#include <stdlib.h>
#include <string.h>
#include "api-structures.h"
#include "connectors.h"
#include "disjunct-utils.h"
#include "linkage.h"
#include "lisjuncts.h"
#include "string-set.h"

/* Links are *always* less than 10 chars long . For now. The estimate
 * below is somewhat dangerous .... could be  fixed. */
#define MAX_LINK_NAME_LENGTH 10

/**
 * lg_compute_disjunct_strings -- Given sentence, compute disjuncts.
 *
 * This routine will compute the string representation of the disjunct
 * used for each word in parsing the given sentence. A string
 * representation of the disjunct is needed for most of the corpus
 * statistics functions: this string, together with the subscripted
 * word, is used as a key to index the statistics information in the
 * database.
 *
 * The connectors are extracted from link_array (and not chosen_disjuncts)
 * so the lexical links remain hidden when HIDE_MORPHO is true (see
 * compute_chosen_disjuncts()).
 *
 * In order that multi-connectors will not be extracted several times
 * for each disjunct (if they connect to multiple words) their address
 * is checked for duplication.
 */
void lg_compute_disjunct_strings(Linkage lkg)
{
	char djstr[MAX_LINK_NAME_LENGTH*20]; /* no word will have more than 20 links */
	size_t nwords = lkg->num_words;

	if (lkg->disjunct_list_str) return;
	lkg->disjunct_list_str = (char **) malloc(nwords * sizeof(char *));
	memset(lkg->disjunct_list_str, 0, nwords * sizeof(char *));

	for (WordIdx w = 0; w < nwords; w++)
	{
		size_t len = 0;

		for (int dir = 0; dir < 2; dir++)
		{
			Connector *last_ms = NULL; /* last multi-connector */

			for (LinkIdx i = lkg->num_links-1; i < lkg->num_links; i--)
			{
				Link *lnk = &lkg->link_array[i];
				Connector *c;

				if (0 == dir)
				{
					if (lnk->rw != w) continue;
					c = lnk->rc;
				}
				else
				{
					if (lnk->lw != w) continue;
					c = lnk->lc;
				}

				if (c->multi)
				{
					if (last_ms == c) continue; /* already included */
					last_ms = c;
					djstr[len++] = '@';
				}
				len += lg_strlcpy(djstr+len, connector_string(c), sizeof(djstr)-len);

				if (len >= sizeof(djstr) - 3)
				{
					len = sizeof(djstr) - 1;
					break;
				}
				djstr[len++] = (dir == 0) ? '-' : '+';
				djstr[len++] = ' ';
			}
		}
		djstr[len++] = '\0';

		lkg->disjunct_list_str[w] = strdup(djstr);
	}
}
