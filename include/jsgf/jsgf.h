/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2007 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced
 * Research Projects Agency and the National Science Foundation of the
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

#ifndef __JSGF_H__
#define __JSGF_H__

/**
 * @file jsgf.h JSGF grammar compiler
 *
 * This file defines the data structures for parsing JSGF grammars
 * into Sphinx finite-state grammars.
 **/

#include <stdio.h>

/* Win32/WinCE DLL gunk */
#include <jsgf/hash_table.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jsgf_s jsgf_t;
typedef struct jsgf_rule_s jsgf_rule_t;
typedef struct jsgf_slu_s{ jsgf_t * grammar; } jsgf_slu_t;

typedef struct jsgf_tag_s {
    char *text;
    int offset;
    int count;
    glist_t tag;
} jsgf_tag_t;


/**
 * Create a new JSGF grammar.
 *
 * @param parent optional parent grammar for this one (NULL, usually).
 * @return new JSGF grammar object, or NULL on failure.
 */
 
jsgf_t *jsgf_grammar_new(jsgf_t *parent);

/**
 * Parse a JSGF grammar from a file.
 *
 * @param filename the name of the file to parse.
 * @param parent optional parent grammar for this one (NULL, usually).
 * @return new JSGF grammar object, or NULL on failure.
 */
 
jsgf_t *jsgf_parse_file(const char *filename, jsgf_t *parent);


/**
 * Get the grammar name from the file.
 */
 
char const *jsgf_grammar_name(jsgf_t *jsgf);

/**
 * Free a JSGF grammar.
 */
 
void jsgf_grammar_free(jsgf_t *jsgf);

/**
 * Copy a JSGF grammar.
 */
 
jsgf_t* copy_grammar(jsgf_t* grammar);

/**
 * Iterator over rules in a grammar.
 */
typedef hash_iter_t jsgf_rule_iter_t;

/**
 * Get an iterator over all rules in a grammar.
 */
 
jsgf_rule_iter_t *jsgf_rule_iter(jsgf_t *grammar);

/**
 * Advance an iterator to the next rule in the grammar.
 */
#define jsgf_rule_iter_next(itor) hash_table_iter_next(itor)

/**
 * Get the current rule in a rule iterator.
 */
#define jsgf_rule_iter_rule(itor) ((jsgf_rule_t *)(itor)->ent->val)

/**
 * Free a rule iterator (if the end hasn't been reached).
 */
#define jsgf_rule_iter_free(itor) hash_table_iter_free(itor)

/**
 * Get a rule by name from a grammar.
 */
 
jsgf_rule_t *jsgf_get_rule(jsgf_t *grammar, char const *name);

/**
 * Get the rule name from a rule.
 */
 
char const *jsgf_rule_name(jsgf_rule_t *rule);

/**
 * Test if a rule is public or not.
 */
 
int jsgf_rule_public(jsgf_rule_t *rule);

/**
 * Get tag from json
 */
 
glist_t jsgf_get_tag_list(const char *json_tag);

/**
 * Free jsgf tag
 */
 
void jsgf_tag_free(jsgf_tag_t *jsgf_tag);

/**
 * Free jsgf tag list
 */
 
void jsgf_tag_list_free(glist_t jsgt_tag_list);

char * jsgf_fullname(jsgf_t *jsgf, const char *name);

#ifdef __cplusplus
}
#endif


#endif /* __JSGF_H__ */
