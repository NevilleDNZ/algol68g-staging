//! @file parser-bottom-up.c
//! @author J. Marcel van der Veer

//! @section Copyright
//!
//! This file is part of Algol68G - an Algol 68 compiler-interpreter.
//! Copyright 2001-2024 J. Marcel van der Veer [algol68g@xs4all.nl].

//! @section License
//!
//! This program is free software; you can redistribute it and/or modify it 
//! under the terms of the GNU General Public License as published by the 
//! Free Software Foundation; either version 3 of the License, or 
//! (at your option) any later version.
//!
//! This program is distributed in the hope that it will be useful, but 
//! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
//! or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
//! more details. You should have received a copy of the GNU General Public 
//! License along with this program. If not, see [http://www.gnu.org/licenses/].

//! @section Synopsis 
//!
//! Hand-coded bottom-up parser for Algol 68.

// This code constitutes an effective "Algol 68 VW parser"; a pragmatic
// approach was chosen since in the early days of Algol 68, many "ab initio" 
// implementations failed.
// 
// This is a Mailloux-type parser, in the sense that it scans a "phrase" for
// definitions needed for parsing, and therefore allows for tags to be used
// before they are defined, which gives some freedom in top-down programming.
// 
//    B. J. Mailloux. On the implementation of Algol 68.
//    Thesis, Universiteit van Amsterdam (Mathematisch Centrum) [1968].
// 
// Technically, Mailloux's approach renders the two-level grammar LALR.
// 
// This is the bottom-up parser that resolves the structure of the program.

#include "a68g.h"
#include "a68g-parser.h"
#include "a68g-prelude.h"

// Bottom-up parser, reduces all constructs.

//! @brief Whether a series is serial or collateral.

int serial_or_collateral (NODE_T * p)
{
  int semis = 0, commas = 0, exits = 0;
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    if (IS (q, COMMA_SYMBOL)) {
      commas++;
    } else if (IS (q, SEMI_SYMBOL)) {
      semis++;
    } else if (IS (q, EXIT_SYMBOL)) {
      exits++;
    }
  }
  if (semis == 0 && exits == 0 && commas > 0) {
    return COLLATERAL_CLAUSE;
  } else if ((semis > 0 || exits > 0) && commas == 0) {
    return SERIAL_CLAUSE;
  } else if (semis == 0 && exits == 0 && commas == 0) {
    return SERIAL_CLAUSE;
  } else {
// Heuristic guess to give intelligible error message.
    return (semis + exits) >= (commas ? SERIAL_CLAUSE : COLLATERAL_CLAUSE);
  }
}

//! @brief Insert a node with attribute "a" after "p".

void pad_node (NODE_T * p, int a)
{
// This is used to fill information that Algol 68 does not require to be present.
// Filling in gives one format for such construct; this helps later passes.
  NODE_T *z = new_node ();
  *z = *p;
  if (GINFO (p) != NO_GINFO) {
    GINFO (z) = new_genie_info ();
  }
  PREVIOUS (z) = p;
  SUB (z) = NO_NODE;
  ATTRIBUTE (z) = a;
  MOID (z) = NO_MOID;
  if (NEXT (z) != NO_NODE) {
    PREVIOUS (NEXT (z)) = z;
  }
  NEXT (p) = z;
}

//! @brief Diagnose extensions.

void a68_extension (NODE_T * p)
{
  if (OPTION_PORTCHECK (&A68_JOB)) {
    diagnostic (A68_WARNING | A68_FORCE_DIAGNOSTICS, p, WARNING_EXTENSION);
  } else {
    diagnostic (A68_WARNING, p, WARNING_EXTENSION);
  }
}

//! @brief Diagnose for clauses not yielding a value.

void empty_clause (NODE_T * p)
{
  diagnostic (A68_SYNTAX_ERROR, p, ERROR_CLAUSE_WITHOUT_VALUE);
}

#if !defined (BUILD_PARALLEL_CLAUSE)

//! @brief Diagnose for parallel clause.

void par_clause (NODE_T * p)
{
  diagnostic (A68_SYNTAX_ERROR, p, ERROR_NO_PARALLEL_CLAUSE);
}

#endif

//! @brief Diagnose for missing symbol.

void strange_tokens (NODE_T * p)
{
  NODE_T *q = ((p != NO_NODE && NEXT (p) != NO_NODE) ? NEXT (p) : p);
  diagnostic (A68_SYNTAX_ERROR, q, ERROR_SYNTAX_STRANGE_TOKENS);
}

//! @brief Diagnose for strange separator.

void strange_separator (NODE_T * p)
{
  NODE_T *q = ((p != NO_NODE && NEXT (p) != NO_NODE) ? NEXT (p) : p);
  diagnostic (A68_SYNTAX_ERROR, q, ERROR_SYNTAX_STRANGE_SEPARATOR);
}

//! @brief If match then reduce a sentence, the core BU parser routine.

void reduce (NODE_T * p, void (*a) (NODE_T *), BOOL_T * z, ...)
{
  va_list list;
  va_start (list, z);
  int result = va_arg (list, int), expect;
  NODE_T *head = p, *tail = NO_NODE;
  while ((expect = va_arg (list, int)) != STOP)
  {
    BOOL_T keep_matching;
    if (p == NO_NODE) {
      keep_matching = A68_FALSE;
    } else if (expect == WILDCARD) {
// WILDCARD matches any Algol68G non terminal, but no keyword.
      keep_matching = (BOOL_T) (non_terminal_string (A68 (edit_line), ATTRIBUTE (p)) != NO_TEXT);
    } else {
      if (expect == SKIP) {
// Stray "~" matches expected SKIP.
        if (IS (p, OPERATOR) && IS_LITERALLY (p, "~")) {
          ATTRIBUTE (p) = SKIP;
        }
      }
      if (expect >= 0) {
        keep_matching = (BOOL_T) (expect == ATTRIBUTE (p));
      } else {
        keep_matching = (BOOL_T) (expect != ATTRIBUTE (p));
      }
    }
    if (keep_matching) {
      tail = p;
      FORWARD (p);
    } else {
      va_end (list);
      return;
    }
  }
// Print parser reductions.
  if (head != NO_NODE && OPTION_REDUCTIONS (&A68_JOB) && LINE_NUMBER (head) > 0) {
    A68_PARSER (reductions)++;
    WIS (head);
    ASSERT (a68_bufprt (A68 (output_line), SNPRINTF_SIZE, "\nReduction %d: %s<-", A68_PARSER (reductions), non_terminal_string (A68 (edit_line), result)) >= 0);
    WRITE (A68_STDOUT, A68 (output_line));
    int count = 0;
    for (NODE_T *q = head; q != NO_NODE && tail != NO_NODE && q != NEXT (tail); FORWARD (q), count++) {
      int gatt = ATTRIBUTE (q);
      char *str = non_terminal_string (A68 (input_line), gatt);
      if (count > 0) {
        WRITE (A68_STDOUT, ", ");
      }
      if (str != NO_TEXT) {
        WRITE (A68_STDOUT, str);
        if (gatt == IDENTIFIER || gatt == OPERATOR || gatt == DENOTATION || gatt == INDICANT) {
          ASSERT (a68_bufprt (A68 (output_line), SNPRINTF_SIZE, " \"%s\"", NSYMBOL (q)) >= 0);
          WRITE (A68_STDOUT, A68 (output_line));
        }
      } else {
        WRITE (A68_STDOUT, NSYMBOL (q));
      }
    }
  }
// Make reduction.
  if (a != NO_NOTE) {
    a (head);
  }
  make_sub (head, tail, result);
  va_end (list);
  if (z != NO_TICK) {
    *z = A68_TRUE;
  }
}

//! @brief Graciously ignore extra semicolons.

void ignore_superfluous_semicolons (NODE_T * p)
{
// This routine relaxes the parser a bit with respect to superfluous semicolons,
// for instance "FI; OD". These provoke only a warning.
  for (; p != NO_NODE; FORWARD (p)) {
    ignore_superfluous_semicolons (SUB (p));
    if (NEXT (p) != NO_NODE && IS (NEXT (p), SEMI_SYMBOL) && NEXT_NEXT (p) == NO_NODE) {
      diagnostic (A68_WARNING | A68_FORCE_DIAGNOSTICS, NEXT (p), WARNING_SKIPPED_SUPERFLUOUS, ATTRIBUTE (NEXT (p)));
      NEXT (p) = NO_NODE;
    } else if (IS (p, SEMI_SYMBOL) && is_semicolon_less (NEXT (p))) {
      diagnostic (A68_WARNING | A68_FORCE_DIAGNOSTICS, p, WARNING_SKIPPED_SUPERFLUOUS, ATTRIBUTE (p));
      if (PREVIOUS (p) != NO_NODE) {
        NEXT (PREVIOUS (p)) = NEXT (p);
      }
      PREVIOUS (NEXT (p)) = PREVIOUS (p);
    }
  }
}

//! @brief Driver for the bottom-up parser.

void bottom_up_parser (NODE_T * p)
{
  if (p != NO_NODE) {
    if (!setjmp (A68_PARSER (bottom_up_crash_exit))) {
      NODE_T *q;
      int error_count_0 = ERROR_COUNT (&A68_JOB);
      ignore_superfluous_semicolons (p);
// A program is "label sequence; particular program".
      extract_labels (p, SERIAL_CLAUSE);
// Parse the program itself.
      for (q = p; q != NO_NODE; FORWARD (q)) {
        BOOL_T siga = A68_TRUE;
        if (SUB (q) != NO_NODE) {
          reduce_branch (q, SOME_CLAUSE);
        }
        while (siga) {
          siga = A68_FALSE;
          reduce (q, NO_NOTE, &siga, LABEL, DEFINING_IDENTIFIER, COLON_SYMBOL, STOP);
          reduce (q, NO_NOTE, &siga, LABEL, LABEL, DEFINING_IDENTIFIER, COLON_SYMBOL, STOP);
        }
      }
// Determine the encompassing enclosed clause.
      for (q = p; q != NO_NODE; FORWARD (q)) {
#if defined (BUILD_PARALLEL_CLAUSE)
        reduce (q, NO_NOTE, NO_TICK, PARALLEL_CLAUSE, PAR_SYMBOL, COLLATERAL_CLAUSE, STOP);
#else
        reduce (q, par_clause, NO_TICK, PARALLEL_CLAUSE, PAR_SYMBOL, COLLATERAL_CLAUSE, STOP);
#endif
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, PARALLEL_CLAUSE, STOP);
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CLOSED_CLAUSE, STOP);
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, COLLATERAL_CLAUSE, STOP);
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CONDITIONAL_CLAUSE, STOP);
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CASE_CLAUSE, STOP);
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CONFORMITY_CLAUSE, STOP);
        reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, LOOP_CLAUSE, STOP);
      }
// Try reducing the particular program.
      q = p;
      reduce (q, NO_NOTE, NO_TICK, PARTICULAR_PROGRAM, LABEL, ENCLOSED_CLAUSE, STOP);
      reduce (q, NO_NOTE, NO_TICK, PARTICULAR_PROGRAM, ENCLOSED_CLAUSE, STOP);
      if (SUB (p) == NO_NODE || NEXT (p) != NO_NODE) {
        recover_from_error (p, PARTICULAR_PROGRAM, (BOOL_T) ((ERROR_COUNT (&A68_JOB) - error_count_0) > MAX_ERRORS));
      }
    }
  }
}

//! @brief Reduce code clause.

void reduce_code_clause (NODE_T * p)
{
  BOOL_T siga = A68_TRUE;
  while (siga) {
    siga = A68_FALSE;
    for (NODE_T *u = p; u != NO_NODE; FORWARD (u)) {
      reduce (u, NO_NOTE, &siga, CODE_LIST, CODE_SYMBOL, ROW_CHAR_DENOTATION, STOP);
      reduce (u, NO_NOTE, &siga, CODE_LIST, CODE_LIST, ROW_CHAR_DENOTATION, STOP);
      reduce (u, NO_NOTE, &siga, CODE_LIST, CODE_LIST, COMMA_SYMBOL, ROW_CHAR_DENOTATION, STOP);
      reduce (u, NO_NOTE, &siga, CODE_CLAUSE, CODE_LIST, EDOC_SYMBOL, STOP);
    }
  }
}

//! @brief Reduce the sub-phrase that starts one level down.

void reduce_branch (NODE_T * q, int expect)
{
// If unsuccessful then the routine will at least copy the resulting attribute
// as the parser can repair some faults. This gives less spurious diagnostics.
  if (q != NO_NODE && SUB (q) != NO_NODE) {
    NODE_T *p = SUB (q), *u = NO_NODE;
    int error_count_0 = ERROR_COUNT (&A68_JOB), error_count_02;
    BOOL_T declarer_pack = A68_FALSE, no_error;
    switch (expect) {
    case STRUCTURE_PACK:
    case PARAMETER_PACK:
    case FORMAL_DECLARERS:
    case UNION_PACK:
    case SPECIFIER: {
        declarer_pack = A68_TRUE;
        break;
      }
    default: {
        declarer_pack = A68_FALSE;
        break;
      }
    }
// Sample all info needed to decide whether a bold tag is operator or indicant.
// Find the meaning of bold tags and quit in case of extra errors.
    extract_indicants (p);
    if (!declarer_pack) {
      extract_priorities (p);
      extract_operators (p);
    }
    error_count_02 = ERROR_COUNT (&A68_JOB);
    elaborate_bold_tags (p);
    if ((ERROR_COUNT (&A68_JOB) - error_count_02) > 0) {
      longjmp (A68_PARSER (bottom_up_crash_exit), 1);
    }
// Now we can reduce declarers, knowing which bold tags are indicants.
    reduce_declarers (p, expect);
// Parse the phrase, as appropriate.
    if (expect == CODE_CLAUSE) {
      reduce_code_clause (p);
    } else if (declarer_pack == A68_FALSE) {
      error_count_02 = ERROR_COUNT (&A68_JOB);
      extract_declarations (p);
      if ((ERROR_COUNT (&A68_JOB) - error_count_02) > 0) {
        longjmp (A68_PARSER (bottom_up_crash_exit), 1);
      }
      extract_labels (p, expect);
      for (u = p; u != NO_NODE; FORWARD (u)) {
        if (SUB (u) != NO_NODE) {
          if (IS (u, FORMAT_DELIMITER_SYMBOL)) {
            reduce_branch (u, FORMAT_TEXT);
          } else if (IS (u, FORMAT_OPEN_SYMBOL)) {
            reduce_branch (u, FORMAT_TEXT);
          } else if (IS (u, OPEN_SYMBOL)) {
            if (NEXT (u) != NO_NODE && IS (NEXT (u), THEN_BAR_SYMBOL)) {
              reduce_branch (u, ENQUIRY_CLAUSE);
            } else if (PREVIOUS (u) != NO_NODE && IS (PREVIOUS (u), PAR_SYMBOL)) {
              reduce_branch (u, COLLATERAL_CLAUSE);
            }
          } else if (is_one_of (u, IF_SYMBOL, ELIF_SYMBOL, CASE_SYMBOL, OUSE_SYMBOL, WHILE_SYMBOL, UNTIL_SYMBOL, ELSE_BAR_SYMBOL, ACCO_SYMBOL, STOP)) {
            reduce_branch (u, ENQUIRY_CLAUSE);
          } else if (IS (u, BEGIN_SYMBOL)) {
            reduce_branch (u, SOME_CLAUSE);
          } else if (is_one_of (u, THEN_SYMBOL, ELSE_SYMBOL, OUT_SYMBOL, DO_SYMBOL, ALT_DO_SYMBOL, STOP)) {
            reduce_branch (u, SERIAL_CLAUSE);
          } else if (IS (u, IN_SYMBOL)) {
            reduce_branch (u, COLLATERAL_CLAUSE);
          } else if (IS (u, THEN_BAR_SYMBOL)) {
            reduce_branch (u, SOME_CLAUSE);
          } else if (IS (u, LOOP_CLAUSE)) {
            reduce_branch (u, ENCLOSED_CLAUSE);
          } else if (is_one_of (u, FOR_SYMBOL, FROM_SYMBOL, BY_SYMBOL, TO_SYMBOL, DOWNTO_SYMBOL, STOP)) {
            reduce_branch (u, UNIT);
          }
        }
      }
      reduce_primary_parts (p, expect);
      if (expect != ENCLOSED_CLAUSE) {
        reduce_primaries (p, expect);
        if (expect == FORMAT_TEXT) {
          reduce_format_texts (p);
        } else {
          reduce_secondaries (p);
          reduce_formulae (p);
          reduce_tertiaries (p);
        }
      }
      for (u = p; u != NO_NODE; FORWARD (u)) {
        if (SUB (u) != NO_NODE) {
          if (IS (u, CODE_SYMBOL)) {
            reduce_branch (u, CODE_CLAUSE);
          }
        }
      }
      reduce_right_to_left_constructs (p);
// Reduce units and declarations.
      reduce_basic_declarations (p);
      reduce_units (p);
      reduce_erroneous_units (p);
      if (expect != UNIT) {
        if (expect == GENERIC_ARGUMENT) {
          reduce_generic_arguments (p);
        } else if (expect == BOUNDS) {
          reduce_bounds (p);
        } else {
          reduce_declaration_lists (p);
          if (expect != DECLARATION_LIST) {
            for (u = p; u != NO_NODE; FORWARD (u)) {
              reduce (u, NO_NOTE, NO_TICK, LABELED_UNIT, LABEL, UNIT, STOP);
              reduce (u, NO_NOTE, NO_TICK, SPECIFIED_UNIT, SPECIFIER, COLON_SYMBOL, UNIT, STOP);
            }
            if (expect == SOME_CLAUSE) {
              expect = serial_or_collateral (p);
            }
            if (expect == SERIAL_CLAUSE) {
              reduce_serial_clauses (p);
            } else if (expect == ENQUIRY_CLAUSE) {
              reduce_enquiry_clauses (p);
            } else if (expect == COLLATERAL_CLAUSE) {
              reduce_collateral_clauses (p);
            } else if (expect == ARGUMENT) {
              reduce_arguments (p);
            }
          }
        }
      }
      reduce_enclosed_clauses (p, expect);
    }
// Do something if parsing failed.
    if (SUB (p) == NO_NODE || NEXT (p) != NO_NODE) {
      recover_from_error (p, expect, (BOOL_T) ((ERROR_COUNT (&A68_JOB) - error_count_0) > MAX_ERRORS));
      no_error = A68_FALSE;
    } else {
      no_error = A68_TRUE;
    }
    ATTRIBUTE (q) = ATTRIBUTE (p);
    if (no_error) {
      SUB (q) = SUB (p);
    }
  }
}

//! @brief Driver for reducing declarers.

void reduce_declarers (NODE_T * p, int expect)
{
  NODE_T *q; BOOL_T siga; // Must be in this scope.
// Reduce lengtheties.
  for (q = p; q != NO_NODE; FORWARD (q)) {
    siga = A68_TRUE;
    reduce (q, NO_NOTE, NO_TICK, LONGETY, LONG_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, SHORTETY, SHORT_SYMBOL, STOP);
    while (siga) {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, LONGETY, LONGETY, LONG_SYMBOL, STOP);
      reduce (q, NO_NOTE, &siga, SHORTETY, SHORTETY, SHORT_SYMBOL, STOP);
    }
  }
// Reduce indicants.
  for (q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, INDICANT, INT_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, REAL_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, BITS_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, BYTES_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, COMPLEX_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, COMPL_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, BOOL_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, CHAR_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, FORMAT_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, STRING_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, FILE_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, CHANNEL_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, SEMA_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, PIPE_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, INDICANT, SOUND_SYMBOL, STOP);
  }
// Reduce standard stuff.
  for (q = p; q != NO_NODE; FORWARD (q)) {
    if (whether (q, LONGETY, INDICANT, STOP)) {
      int a;
      if (SUB_NEXT (q) == NO_NODE) {
        diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_EXPECTED, INFO_APPROPRIATE_DECLARER);
        reduce (q, NO_NOTE, NO_TICK, DECLARER, LONGETY, INDICANT, STOP);
      } else {
        a = ATTRIBUTE (SUB_NEXT (q));
        if (a == INT_SYMBOL || a == REAL_SYMBOL || a == BITS_SYMBOL || a == BYTES_SYMBOL || a == COMPLEX_SYMBOL || a == COMPL_SYMBOL) {
          reduce (q, NO_NOTE, NO_TICK, DECLARER, LONGETY, INDICANT, STOP);
        } else {
          diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_EXPECTED, INFO_APPROPRIATE_DECLARER);
          reduce (q, NO_NOTE, NO_TICK, DECLARER, LONGETY, INDICANT, STOP);
        }
      }
    } else if (whether (q, SHORTETY, INDICANT, STOP)) {
      int a;
      if (SUB_NEXT (q) == NO_NODE) {
        diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_EXPECTED, INFO_APPROPRIATE_DECLARER);
        reduce (q, NO_NOTE, NO_TICK, DECLARER, SHORTETY, INDICANT, STOP);
      } else {
        a = ATTRIBUTE (SUB_NEXT (q));
        if (a == INT_SYMBOL || a == REAL_SYMBOL || a == BITS_SYMBOL || a == BYTES_SYMBOL || a == COMPLEX_SYMBOL || a == COMPL_SYMBOL) {
          reduce (q, NO_NOTE, NO_TICK, DECLARER, SHORTETY, INDICANT, STOP);
        } else {
          diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_EXPECTED, INFO_APPROPRIATE_DECLARER);
          reduce (q, NO_NOTE, NO_TICK, DECLARER, LONGETY, INDICANT, STOP);
        }
      }
    }
  }
  for (q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, DECLARER, INDICANT, STOP);
  }
// Reduce declarer lists.
  for (q = p; q != NO_NODE; FORWARD (q)) {
    if (NEXT (q) != NO_NODE && SUB_NEXT (q) != NO_NODE) {
      if (IS (q, STRUCT_SYMBOL)) {
        reduce_branch (NEXT (q), STRUCTURE_PACK);
        reduce (q, NO_NOTE, NO_TICK, DECLARER, STRUCT_SYMBOL, STRUCTURE_PACK, STOP);
      } else if (IS (q, UNION_SYMBOL)) {
        reduce_branch (NEXT (q), UNION_PACK);
        reduce (q, NO_NOTE, NO_TICK, DECLARER, UNION_SYMBOL, UNION_PACK, STOP);
      } else if (IS (q, PROC_SYMBOL)) {
        if (whether (q, PROC_SYMBOL, OPEN_SYMBOL, STOP)) {
          if (!is_formal_bounds (SUB_NEXT (q))) {
            reduce_branch (NEXT (q), FORMAL_DECLARERS);
          }
        }
      } else if (IS (q, OP_SYMBOL)) {
        if (whether (q, OP_SYMBOL, OPEN_SYMBOL, STOP)) {
          if (!is_formal_bounds (SUB_NEXT (q))) {
            reduce_branch (NEXT (q), FORMAL_DECLARERS);
          }
        }
      }
    }
  }
// Reduce row, proc or op declarers.
  siga = A68_TRUE;
  while (siga) {
    siga = A68_FALSE;
    for (q = p; q != NO_NODE; FORWARD (q)) {
// FLEX DECL.
      if (whether (q, FLEX_SYMBOL, DECLARER, STOP)) {
        reduce (q, NO_NOTE, &siga, DECLARER, FLEX_SYMBOL, DECLARER, STOP);
      }
// FLEX [] DECL.
      if (whether (q, FLEX_SYMBOL, SUB_SYMBOL, DECLARER, STOP) && SUB_NEXT (q) != NO_NODE) {
        reduce_branch (NEXT (q), BOUNDS);
        reduce (q, NO_NOTE, &siga, DECLARER, FLEX_SYMBOL, BOUNDS, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, DECLARER, FLEX_SYMBOL, FORMAL_BOUNDS, DECLARER, STOP);
      }
// FLEX () DECL.
      if (whether (q, FLEX_SYMBOL, OPEN_SYMBOL, DECLARER, STOP) && SUB_NEXT (q) != NO_NODE) {
        if (!whether (q, FLEX_SYMBOL, OPEN_SYMBOL, DECLARER, COLON_SYMBOL, STOP)) {
          reduce_branch (NEXT (q), BOUNDS);
          reduce (q, NO_NOTE, &siga, DECLARER, FLEX_SYMBOL, BOUNDS, DECLARER, STOP);
          reduce (q, NO_NOTE, &siga, DECLARER, FLEX_SYMBOL, FORMAL_BOUNDS, DECLARER, STOP);
        }
      }
// [] DECL.
      if (whether (q, SUB_SYMBOL, DECLARER, STOP) && SUB (q) != NO_NODE) {
        reduce_branch (q, BOUNDS);
        reduce (q, NO_NOTE, &siga, DECLARER, BOUNDS, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, DECLARER, FORMAL_BOUNDS, DECLARER, STOP);
      }
// () DECL.
      if (whether (q, OPEN_SYMBOL, DECLARER, STOP) && SUB (q) != NO_NODE) {
        if (whether (q, OPEN_SYMBOL, DECLARER, COLON_SYMBOL, STOP)) {
// Catch e.g. (INT i) () INT:.
          if (is_formal_bounds (SUB (q))) {
            reduce_branch (q, BOUNDS);
            reduce (q, NO_NOTE, &siga, DECLARER, BOUNDS, DECLARER, STOP);
            reduce (q, NO_NOTE, &siga, DECLARER, FORMAL_BOUNDS, DECLARER, STOP);
          }
        } else {
          reduce_branch (q, BOUNDS);
          reduce (q, NO_NOTE, &siga, DECLARER, BOUNDS, DECLARER, STOP);
          reduce (q, NO_NOTE, &siga, DECLARER, FORMAL_BOUNDS, DECLARER, STOP);
        }
      }
    }
// PROC DECL, PROC () DECL, OP () DECL.
    for (q = p; q != NO_NODE; FORWARD (q)) {
      int a = ATTRIBUTE (q);
      if (a == REF_SYMBOL) {
        reduce (q, NO_NOTE, &siga, DECLARER, REF_SYMBOL, DECLARER, STOP);
      } else if (a == PROC_SYMBOL) {
        reduce (q, NO_NOTE, &siga, DECLARER, PROC_SYMBOL, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, DECLARER, PROC_SYMBOL, FORMAL_DECLARERS, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, DECLARER, PROC_SYMBOL, VOID_SYMBOL, STOP);
        reduce (q, NO_NOTE, &siga, DECLARER, PROC_SYMBOL, FORMAL_DECLARERS, VOID_SYMBOL, STOP);
      } else if (a == OP_SYMBOL) {
        reduce (q, NO_NOTE, &siga, OPERATOR_PLAN, OP_SYMBOL, FORMAL_DECLARERS, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, OPERATOR_PLAN, OP_SYMBOL, FORMAL_DECLARERS, VOID_SYMBOL, STOP);
      }
    }
  }
// Reduce packs etcetera.
  if (expect == STRUCTURE_PACK) {
    for (q = p; q != NO_NODE; FORWARD (q)) {
      siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, STRUCTURED_FIELD, DECLARER, IDENTIFIER, STOP);
        reduce (q, NO_NOTE, &siga, STRUCTURED_FIELD, STRUCTURED_FIELD, COMMA_SYMBOL, IDENTIFIER, STOP);
      }
    }
    for (q = p; q != NO_NODE; FORWARD (q)) {
      siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, STRUCTURED_FIELD_LIST, STRUCTURED_FIELD, STOP);
        reduce (q, NO_NOTE, &siga, STRUCTURED_FIELD_LIST, STRUCTURED_FIELD_LIST, COMMA_SYMBOL, STRUCTURED_FIELD, STOP);
        reduce (q, strange_separator, &siga, STRUCTURED_FIELD_LIST, STRUCTURED_FIELD_LIST, STRUCTURED_FIELD, STOP);
        reduce (q, strange_separator, &siga, STRUCTURED_FIELD_LIST, STRUCTURED_FIELD_LIST, SEMI_SYMBOL, STRUCTURED_FIELD, STOP);
      }
    }
    q = p;
    reduce (q, NO_NOTE, NO_TICK, STRUCTURE_PACK, OPEN_SYMBOL, STRUCTURED_FIELD_LIST, CLOSE_SYMBOL, STOP);
  } else if (expect == PARAMETER_PACK) {
    for (q = p; q != NO_NODE; FORWARD (q)) {
      siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, PARAMETER, DECLARER, IDENTIFIER, STOP);
        reduce (q, NO_NOTE, &siga, PARAMETER, PARAMETER, COMMA_SYMBOL, IDENTIFIER, STOP);
      }
    }
    for (q = p; q != NO_NODE; FORWARD (q)) {
      siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, PARAMETER_LIST, PARAMETER, STOP);
        reduce (q, NO_NOTE, &siga, PARAMETER_LIST, PARAMETER_LIST, COMMA_SYMBOL, PARAMETER, STOP);
      }
    }
    q = p;
    reduce (q, NO_NOTE, NO_TICK, PARAMETER_PACK, OPEN_SYMBOL, PARAMETER_LIST, CLOSE_SYMBOL, STOP);
  } else if (expect == FORMAL_DECLARERS) {
    for (q = p; q != NO_NODE; FORWARD (q)) {
      siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, FORMAL_DECLARERS_LIST, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, FORMAL_DECLARERS_LIST, FORMAL_DECLARERS_LIST, COMMA_SYMBOL, DECLARER, STOP);
        reduce (q, strange_separator, &siga, FORMAL_DECLARERS_LIST, FORMAL_DECLARERS_LIST, SEMI_SYMBOL, DECLARER, STOP);
        reduce (q, strange_separator, &siga, FORMAL_DECLARERS_LIST, FORMAL_DECLARERS_LIST, DECLARER, STOP);
      }
    }
    q = p;
    reduce (q, NO_NOTE, NO_TICK, FORMAL_DECLARERS, OPEN_SYMBOL, FORMAL_DECLARERS_LIST, CLOSE_SYMBOL, STOP);
  } else if (expect == UNION_PACK) {
    for (q = p; q != NO_NODE; FORWARD (q)) {
      siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, UNION_DECLARER_LIST, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, UNION_DECLARER_LIST, VOID_SYMBOL, STOP);
        reduce (q, NO_NOTE, &siga, UNION_DECLARER_LIST, UNION_DECLARER_LIST, COMMA_SYMBOL, DECLARER, STOP);
        reduce (q, NO_NOTE, &siga, UNION_DECLARER_LIST, UNION_DECLARER_LIST, COMMA_SYMBOL, VOID_SYMBOL, STOP);
        reduce (q, strange_separator, &siga, UNION_DECLARER_LIST, UNION_DECLARER_LIST, SEMI_SYMBOL, DECLARER, STOP);
        reduce (q, strange_separator, &siga, UNION_DECLARER_LIST, UNION_DECLARER_LIST, SEMI_SYMBOL, VOID_SYMBOL, STOP);
        reduce (q, strange_separator, &siga, UNION_DECLARER_LIST, UNION_DECLARER_LIST, DECLARER, STOP);
        reduce (q, strange_separator, &siga, UNION_DECLARER_LIST, UNION_DECLARER_LIST, VOID_SYMBOL, STOP);
      }
    }
    q = p;
    reduce (q, NO_NOTE, NO_TICK, UNION_PACK, OPEN_SYMBOL, UNION_DECLARER_LIST, CLOSE_SYMBOL, STOP);
  } else if (expect == SPECIFIER) {
    reduce (p, NO_NOTE, NO_TICK, SPECIFIER, OPEN_SYMBOL, DECLARER, IDENTIFIER, CLOSE_SYMBOL, STOP);
    reduce (p, NO_NOTE, NO_TICK, SPECIFIER, OPEN_SYMBOL, DECLARER, CLOSE_SYMBOL, STOP);
    reduce (p, NO_NOTE, NO_TICK, SPECIFIER, OPEN_SYMBOL, VOID_SYMBOL, CLOSE_SYMBOL, STOP);
  } else {
    for (q = p; q != NO_NODE; FORWARD (q)) {
      if (whether (q, OPEN_SYMBOL, COLON_SYMBOL, STOP) && !(expect == GENERIC_ARGUMENT || expect == BOUNDS)) {
        if (is_one_of (p, IN_SYMBOL, THEN_BAR_SYMBOL, STOP)) {
          reduce_branch (q, SPECIFIER);
        }
      }
      if (whether (q, OPEN_SYMBOL, DECLARER, COLON_SYMBOL, STOP)) {
        reduce_branch (q, PARAMETER_PACK);
      }
      if (whether (q, OPEN_SYMBOL, VOID_SYMBOL, COLON_SYMBOL, STOP)) {
        reduce_branch (q, PARAMETER_PACK);
      }
    }
  }
}

//! @brief Handle cases that need reducing from right-to-left.

void reduce_right_to_left_constructs (NODE_T * p)
{
// Here are cases that need reducing from right-to-left whereas many things
// can be reduced left-to-right. Assignations are a notable example; one could
// discuss whether it would not be more natural to write 1 =: k in stead of
// k := 1. The latter is said to be more natural, or it could be just computing
// history. Meanwhile we use this routine.
  if (p != NO_NODE) {
    reduce_right_to_left_constructs (NEXT (p));
// Assignations.
    if (IS (p, TERTIARY)) {
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, TERTIARY, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, IDENTITY_RELATION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, AND_FUNCTION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, OR_FUNCTION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, ROUTINE_TEXT, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, JUMP, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, SKIP, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, ASSIGNATION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ASSIGNATION, TERTIARY, ASSIGN_SYMBOL, CODE_CLAUSE, STOP);
    }
// Routine texts with parameter pack.
    else if (IS (p, PARAMETER_PACK)) {
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, ASSIGNATION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, IDENTITY_RELATION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, AND_FUNCTION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, OR_FUNCTION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, JUMP, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, SKIP, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, TERTIARY, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, ROUTINE_TEXT, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, DECLARER, COLON_SYMBOL, CODE_CLAUSE, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, ASSIGNATION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, IDENTITY_RELATION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, AND_FUNCTION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, OR_FUNCTION, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, JUMP, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, SKIP, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, TERTIARY, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, ROUTINE_TEXT, STOP);
      reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, PARAMETER_PACK, VOID_SYMBOL, COLON_SYMBOL, CODE_CLAUSE, STOP);
    }
// Routine texts without parameter pack.
    else if (IS (p, DECLARER)) {
      if (!(PREVIOUS (p) != NO_NODE && IS (PREVIOUS (p), PARAMETER_PACK))) {
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, ASSIGNATION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, IDENTITY_RELATION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, AND_FUNCTION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, OR_FUNCTION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, JUMP, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, SKIP, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, TERTIARY, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, ROUTINE_TEXT, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, DECLARER, COLON_SYMBOL, CODE_CLAUSE, STOP);
      }
    } else if (IS (p, VOID_SYMBOL)) {
      if (!(PREVIOUS (p) != NO_NODE && IS (PREVIOUS (p), PARAMETER_PACK))) {
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, ASSIGNATION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, IDENTITY_RELATION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, AND_FUNCTION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, OR_FUNCTION, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, JUMP, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, SKIP, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, TERTIARY, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, ROUTINE_TEXT, STOP);
        reduce (p, NO_NOTE, NO_TICK, ROUTINE_TEXT, VOID_SYMBOL, COLON_SYMBOL, CODE_CLAUSE, STOP);
      }
    }
  }
}

//! @brief Reduce primary elements.

void reduce_primary_parts (NODE_T * p, int expect)
{
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    if (whether (q, IDENTIFIER, OF_SYMBOL, STOP)) {
      ATTRIBUTE (q) = FIELD_IDENTIFIER;
    }
    reduce (q, NO_NOTE, NO_TICK, ENVIRON_NAME, ENVIRON_SYMBOL, ROW_CHAR_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, NIHIL, NIL_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, SKIP, SKIP_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, SELECTOR, FIELD_IDENTIFIER, OF_SYMBOL, STOP);
// JUMPs without GOTO are resolved later.
    reduce (q, NO_NOTE, NO_TICK, JUMP, GOTO_SYMBOL, IDENTIFIER, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, LONGETY, INT_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, LONGETY, REAL_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, LONGETY, BITS_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, SHORTETY, INT_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, SHORTETY, REAL_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, SHORTETY, BITS_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, INT_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, REAL_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, BITS_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, ROW_CHAR_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, TRUE_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, FALSE_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, DENOTATION, EMPTY_SYMBOL, STOP);
    if (expect == SERIAL_CLAUSE || expect == ENQUIRY_CLAUSE || expect == SOME_CLAUSE) {
      BOOL_T siga = A68_TRUE;
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, LABEL, DEFINING_IDENTIFIER, COLON_SYMBOL, STOP);
        reduce (q, NO_NOTE, &siga, LABEL, LABEL, DEFINING_IDENTIFIER, COLON_SYMBOL, STOP);
      }
    }
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
#if defined (BUILD_PARALLEL_CLAUSE)
    reduce (q, NO_NOTE, NO_TICK, PARALLEL_CLAUSE, PAR_SYMBOL, COLLATERAL_CLAUSE, STOP);
#else
    reduce (q, par_clause, NO_TICK, PARALLEL_CLAUSE, PAR_SYMBOL, COLLATERAL_CLAUSE, STOP);
#endif
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, PARALLEL_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CLOSED_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, COLLATERAL_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CONDITIONAL_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CASE_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CONFORMITY_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, LOOP_CLAUSE, STOP);
  }
}

//! @brief Reduce primaries completely.

void reduce_primaries (NODE_T * p, int expect)
{
  NODE_T *q = p;
  while (q != NO_NODE) {
    BOOL_T fwd = A68_TRUE, siga;
// Primaries excepts call and slice.
    reduce (q, NO_NOTE, NO_TICK, PRIMARY, IDENTIFIER, STOP);
    reduce (q, NO_NOTE, NO_TICK, PRIMARY, DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, CAST, DECLARER, ENCLOSED_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, CAST, VOID_SYMBOL, ENCLOSED_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, ASSERTION, ASSERT_SYMBOL, ENCLOSED_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, PRIMARY, CAST, STOP);
    reduce (q, NO_NOTE, NO_TICK, PRIMARY, ENCLOSED_CLAUSE, STOP);
    reduce (q, NO_NOTE, NO_TICK, PRIMARY, FORMAT_TEXT, STOP);
// Call and slice.
    siga = A68_TRUE;
    while (siga) {
      NODE_T *x = NEXT (q);
      siga = A68_FALSE;
      if (IS (q, PRIMARY) && x != NO_NODE) {
        if (IS (x, OPEN_SYMBOL)) {
          reduce_branch (NEXT (q), GENERIC_ARGUMENT);
          reduce (q, NO_NOTE, &siga, SPECIFICATION, PRIMARY, GENERIC_ARGUMENT, STOP);
          reduce (q, NO_NOTE, &siga, PRIMARY, SPECIFICATION, STOP);
        } else if (IS (x, SUB_SYMBOL)) {
          reduce_branch (NEXT (q), GENERIC_ARGUMENT);
          reduce (q, NO_NOTE, &siga, SPECIFICATION, PRIMARY, GENERIC_ARGUMENT, STOP);
          reduce (q, NO_NOTE, &siga, PRIMARY, SPECIFICATION, STOP);
        }
      }
    }
// Now that call and slice are known, reduce remaining ( .. ).
    if (IS (q, OPEN_SYMBOL) && SUB (q) != NO_NODE) {
      reduce_branch (q, SOME_CLAUSE);
      reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CLOSED_CLAUSE, STOP);
      reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, COLLATERAL_CLAUSE, STOP);
      reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CONDITIONAL_CLAUSE, STOP);
      reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CASE_CLAUSE, STOP);
      reduce (q, NO_NOTE, NO_TICK, ENCLOSED_CLAUSE, CONFORMITY_CLAUSE, STOP);
      if (PREVIOUS (q) != NO_NODE) {
        BACKWARD (q);
        fwd = A68_FALSE;
      }
    }
// Format text items.
    if (expect == FORMAT_TEXT) {
      NODE_T *r;
      for (r = p; r != NO_NODE; FORWARD (r)) {
        reduce (r, NO_NOTE, NO_TICK, DYNAMIC_REPLICATOR, FORMAT_ITEM_N, ENCLOSED_CLAUSE, STOP);
        reduce (r, NO_NOTE, NO_TICK, GENERAL_PATTERN, FORMAT_ITEM_G, ENCLOSED_CLAUSE, STOP);
        reduce (r, NO_NOTE, NO_TICK, GENERAL_PATTERN, FORMAT_ITEM_H, ENCLOSED_CLAUSE, STOP);
        reduce (r, NO_NOTE, NO_TICK, FORMAT_PATTERN, FORMAT_ITEM_F, ENCLOSED_CLAUSE, STOP);
      }
    }
    if (fwd) {
      FORWARD (q);
    }
  }
}

//! @brief Enforce that ambiguous patterns are separated by commas.

void ambiguous_patterns (NODE_T * p)
{
// Example: printf (($+d.2d +d.2d$, 1, 2)) can produce either "+1.00 +2.00" or
// "+1+002.00". A comma must be supplied to resolve the ambiguity.
// 
// The obvious thing would be to weave this into the syntax, letting the BU parser
// sort it out. But the C-style patterns do not suffer from Algol 68 pattern
// ambiguity, so by solving it this way we maximise freedom in writing the patterns
// as we want without introducing two "kinds" of patterns, and so we have shorter
// routines for implementing formatted transput. This is a pragmatic system.
  NODE_T *q, *last_pat = NO_NODE;
  for (q = p; q != NO_NODE; FORWARD (q)) {
    switch (ATTRIBUTE (q)) {
    case INTEGRAL_PATTERN:     // These are the potentially ambiguous patterns
    case REAL_PATTERN:
    case COMPLEX_PATTERN:
    case BITS_PATTERN: {
        if (last_pat != NO_NODE) {
          diagnostic (A68_SYNTAX_ERROR, q, ERROR_COMMA_MUST_SEPARATE, ATTRIBUTE (last_pat), ATTRIBUTE (q));
        }
        last_pat = q;
        break;
      }
    case COMMA_SYMBOL: {
        last_pat = NO_NODE;
        break;
      }
    }
  }
}

//! @brief Reduce format texts completely.

void reduce_c_pattern (NODE_T * p, int pr, int let)
{
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, REPLICATOR, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_PLUS, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_PLUS, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_PLUS, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_PLUS, REPLICATOR, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, REPLICATOR, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, FORMAT_ITEM_PLUS, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, FORMAT_ITEM_PLUS, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, FORMAT_ITEM_PLUS, REPLICATOR, let, STOP);
    reduce (q, NO_NOTE, NO_TICK, pr, FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, FORMAT_ITEM_PLUS, REPLICATOR, FORMAT_ITEM_POINT, REPLICATOR, let, STOP);
  }
}

//! @brief Reduce format texts completely.

void reduce_format_texts (NODE_T * p)
{
// Replicators.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, REPLICATOR, STATIC_REPLICATOR, STOP);
    reduce (q, NO_NOTE, NO_TICK, REPLICATOR, DYNAMIC_REPLICATOR, STOP);
  }
// "OTHER" patterns.
  reduce_c_pattern (p, BITS_C_PATTERN, FORMAT_ITEM_B);
  reduce_c_pattern (p, BITS_C_PATTERN, FORMAT_ITEM_O);
  reduce_c_pattern (p, BITS_C_PATTERN, FORMAT_ITEM_X);
  reduce_c_pattern (p, CHAR_C_PATTERN, FORMAT_ITEM_C);
  reduce_c_pattern (p, FIXED_C_PATTERN, FORMAT_ITEM_F);
  reduce_c_pattern (p, FLOAT_C_PATTERN, FORMAT_ITEM_E);
  reduce_c_pattern (p, GENERAL_C_PATTERN, FORMAT_ITEM_G);
  reduce_c_pattern (p, INTEGRAL_C_PATTERN, FORMAT_ITEM_D);
  reduce_c_pattern (p, INTEGRAL_C_PATTERN, FORMAT_ITEM_I);
  reduce_c_pattern (p, STRING_C_PATTERN, FORMAT_ITEM_S);
// Radix frames.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, RADIX_FRAME, REPLICATOR, FORMAT_ITEM_R, STOP);
  }
// Insertions.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, INSERTION, FORMAT_ITEM_X, STOP);
    reduce (q, NO_NOTE, NO_TICK, INSERTION, FORMAT_ITEM_Y, STOP);
    reduce (q, NO_NOTE, NO_TICK, INSERTION, FORMAT_ITEM_L, STOP);
    reduce (q, NO_NOTE, NO_TICK, INSERTION, FORMAT_ITEM_P, STOP);
    reduce (q, NO_NOTE, NO_TICK, INSERTION, FORMAT_ITEM_Q, STOP);
    reduce (q, NO_NOTE, NO_TICK, INSERTION, FORMAT_ITEM_K, STOP);
    reduce (q, NO_NOTE, NO_TICK, INSERTION, LITERAL, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, INSERTION, REPLICATOR, INSERTION, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga = A68_TRUE;
    while (siga) {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, INSERTION, INSERTION, INSERTION, STOP);
    }
  }
// Replicated suppressible frames.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, FORMAT_A_FRAME, REPLICATOR, FORMAT_ITEM_S, FORMAT_ITEM_A, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_Z_FRAME, REPLICATOR, FORMAT_ITEM_S, FORMAT_ITEM_Z, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_D_FRAME, REPLICATOR, FORMAT_ITEM_S, FORMAT_ITEM_D, STOP);
  }
// Suppressible frames.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, FORMAT_A_FRAME, FORMAT_ITEM_S, FORMAT_ITEM_A, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_Z_FRAME, FORMAT_ITEM_S, FORMAT_ITEM_Z, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_D_FRAME, FORMAT_ITEM_S, FORMAT_ITEM_D, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_E_FRAME, FORMAT_ITEM_S, FORMAT_ITEM_E, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_POINT_FRAME, FORMAT_ITEM_S, FORMAT_ITEM_POINT, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_I_FRAME, FORMAT_ITEM_S, FORMAT_ITEM_I, STOP);
  }
// Replicated frames.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, FORMAT_A_FRAME, REPLICATOR, FORMAT_ITEM_A, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_Z_FRAME, REPLICATOR, FORMAT_ITEM_Z, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_D_FRAME, REPLICATOR, FORMAT_ITEM_D, STOP);
  }
// Frames.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, FORMAT_A_FRAME, FORMAT_ITEM_A, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_Z_FRAME, FORMAT_ITEM_Z, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_D_FRAME, FORMAT_ITEM_D, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_E_FRAME, FORMAT_ITEM_E, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_POINT_FRAME, FORMAT_ITEM_POINT, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_I_FRAME, FORMAT_ITEM_I, STOP);
  }
// Frames with an insertion.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, FORMAT_A_FRAME, INSERTION, FORMAT_A_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_Z_FRAME, INSERTION, FORMAT_Z_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_D_FRAME, INSERTION, FORMAT_D_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_E_FRAME, INSERTION, FORMAT_E_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_POINT_FRAME, INSERTION, FORMAT_POINT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMAT_I_FRAME, INSERTION, FORMAT_I_FRAME, STOP);
  }
// String patterns.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, STRING_PATTERN, REPLICATOR, FORMAT_A_FRAME, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, STRING_PATTERN, FORMAT_A_FRAME, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga = A68_TRUE;
    while (siga) {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, STRING_PATTERN, STRING_PATTERN, STRING_PATTERN, STOP);
      reduce (q, NO_NOTE, &siga, STRING_PATTERN, STRING_PATTERN, INSERTION, STRING_PATTERN, STOP);
    }
  }
// Integral moulds.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, INTEGRAL_MOULD, FORMAT_Z_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, INTEGRAL_MOULD, FORMAT_D_FRAME, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga = A68_TRUE;
    while (siga) {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, INTEGRAL_MOULD, INTEGRAL_MOULD, INTEGRAL_MOULD, STOP);
      reduce (q, NO_NOTE, &siga, INTEGRAL_MOULD, INTEGRAL_MOULD, INSERTION, STOP);
    }
  }
// Sign moulds.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, SIGN_MOULD, INTEGRAL_MOULD, FORMAT_ITEM_PLUS, STOP);
    reduce (q, NO_NOTE, NO_TICK, SIGN_MOULD, INTEGRAL_MOULD, FORMAT_ITEM_MINUS, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, SIGN_MOULD, FORMAT_ITEM_PLUS, STOP);
    reduce (q, NO_NOTE, NO_TICK, SIGN_MOULD, FORMAT_ITEM_MINUS, STOP);
  }
// Exponent frames.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, EXPONENT_FRAME, FORMAT_E_FRAME, SIGN_MOULD, INTEGRAL_MOULD, STOP);
    reduce (q, NO_NOTE, NO_TICK, EXPONENT_FRAME, FORMAT_E_FRAME, INTEGRAL_MOULD, STOP);
  }
// Real patterns.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, INTEGRAL_MOULD, FORMAT_POINT_FRAME, INTEGRAL_MOULD, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, INTEGRAL_MOULD, FORMAT_POINT_FRAME, INTEGRAL_MOULD, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, INTEGRAL_MOULD, FORMAT_POINT_FRAME, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, INTEGRAL_MOULD, FORMAT_POINT_FRAME, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, FORMAT_POINT_FRAME, INTEGRAL_MOULD, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, FORMAT_POINT_FRAME, INTEGRAL_MOULD, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, FORMAT_POINT_FRAME, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, FORMAT_POINT_FRAME, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, INTEGRAL_MOULD, FORMAT_POINT_FRAME, INTEGRAL_MOULD, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, INTEGRAL_MOULD, FORMAT_POINT_FRAME, INTEGRAL_MOULD, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, INTEGRAL_MOULD, FORMAT_POINT_FRAME, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, INTEGRAL_MOULD, FORMAT_POINT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, FORMAT_POINT_FRAME, INTEGRAL_MOULD, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, FORMAT_POINT_FRAME, INTEGRAL_MOULD, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, SIGN_MOULD, INTEGRAL_MOULD, EXPONENT_FRAME, STOP);
    reduce (q, NO_NOTE, NO_TICK, REAL_PATTERN, INTEGRAL_MOULD, EXPONENT_FRAME, STOP);
  }
// Complex patterns.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, COMPLEX_PATTERN, REAL_PATTERN, FORMAT_I_FRAME, REAL_PATTERN, STOP);
  }
// Bits patterns.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, BITS_PATTERN, RADIX_FRAME, INTEGRAL_MOULD, STOP);
  }
// Integral patterns.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, INTEGRAL_PATTERN, SIGN_MOULD, INTEGRAL_MOULD, STOP);
    reduce (q, NO_NOTE, NO_TICK, INTEGRAL_PATTERN, INTEGRAL_MOULD, STOP);
  }
// Patterns.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, BOOLEAN_PATTERN, FORMAT_ITEM_B, COLLECTION, STOP);
    reduce (q, NO_NOTE, NO_TICK, CHOICE_PATTERN, FORMAT_ITEM_C, COLLECTION, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, BOOLEAN_PATTERN, FORMAT_ITEM_B, STOP);
    reduce (q, NO_NOTE, NO_TICK, GENERAL_PATTERN, FORMAT_ITEM_G, STOP);
    reduce (q, NO_NOTE, NO_TICK, GENERAL_PATTERN, FORMAT_ITEM_H, STOP);
  }
  ambiguous_patterns (p);
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, BITS_C_PATTERN, STOP);
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, CHAR_C_PATTERN, STOP);
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, FIXED_C_PATTERN, STOP);
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, FLOAT_C_PATTERN, STOP);
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, GENERAL_C_PATTERN, STOP);
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, INTEGRAL_C_PATTERN, STOP);
    reduce (q, a68_extension, NO_TICK, A68_PATTERN, STRING_C_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, BITS_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, BOOLEAN_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, CHOICE_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, COMPLEX_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, FORMAT_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, GENERAL_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, INTEGRAL_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, REAL_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, A68_PATTERN, STRING_PATTERN, STOP);
  }
// Pictures.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, PICTURE, INSERTION, STOP);
    reduce (q, NO_NOTE, NO_TICK, PICTURE, A68_PATTERN, STOP);
    reduce (q, NO_NOTE, NO_TICK, PICTURE, COLLECTION, STOP);
    reduce (q, NO_NOTE, NO_TICK, PICTURE, REPLICATOR, COLLECTION, STOP);
  }
// Picture lists.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    if (IS (q, PICTURE)) {
      BOOL_T siga = A68_TRUE;
      reduce (q, NO_NOTE, NO_TICK, PICTURE_LIST, PICTURE, STOP);
      while (siga) {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, PICTURE_LIST, PICTURE_LIST, COMMA_SYMBOL, PICTURE, STOP);
// We filtered ambiguous patterns, so commas may be omitted 
        reduce (q, NO_NOTE, &siga, PICTURE_LIST, PICTURE_LIST, PICTURE, STOP);
      }
    }
  }
}

//! @brief Reduce secondaries completely.

void reduce_secondaries (NODE_T * p)
{
  NODE_T *q; BOOL_T siga;
  for (q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, SECONDARY, PRIMARY, STOP);
    reduce (q, NO_NOTE, NO_TICK, GENERATOR, LOC_SYMBOL, DECLARER, STOP);
    reduce (q, NO_NOTE, NO_TICK, GENERATOR, HEAP_SYMBOL, DECLARER, STOP);
    reduce (q, NO_NOTE, NO_TICK, GENERATOR, NEW_SYMBOL, DECLARER, STOP);
    reduce (q, NO_NOTE, NO_TICK, SECONDARY, GENERATOR, STOP);
  }
  siga = A68_TRUE;
  while (siga) {
    siga = A68_FALSE;
    for (q = p; NEXT (q) != NO_NODE; FORWARD (q)) {
      ;
    }
    for (; q != NO_NODE; BACKWARD (q)) {
      reduce (q, NO_NOTE, &siga, SELECTION, SELECTOR, SECONDARY, STOP);
      reduce (q, NO_NOTE, &siga, SECONDARY, SELECTION, STOP);
    }
  }
}

//! @brief Whether "q" is an operator with priority "k".

int operator_with_priority (NODE_T * q, int k)
{
  return NEXT (q) != NO_NODE && ATTRIBUTE (NEXT (q)) == OPERATOR && PRIO (INFO (NEXT (q))) == k;
}

//! @brief Reduce formulae.

void reduce_formulae (NODE_T * p)
{
  NODE_T *q = p;
  while (q != NO_NODE) {
    if (is_one_of (q, OPERATOR, SECONDARY, STOP)) {
      q = reduce_dyadic (q, STOP);
    } else {
      FORWARD (q);
    }
  }
// Reduce the expression.
  for (int prio = MAX_PRIORITY; prio >= 0; prio--) {
    for (q = p; q != NO_NODE; FORWARD (q)) {
      if (operator_with_priority (q, prio)) {
        BOOL_T siga = A68_FALSE;
        NODE_T *op = NEXT (q);
        if (IS (q, SECONDARY)) {
          reduce (q, NO_NOTE, &siga, FORMULA, SECONDARY, OPERATOR, SECONDARY, STOP);
          reduce (q, NO_NOTE, &siga, FORMULA, SECONDARY, OPERATOR, MONADIC_FORMULA, STOP);
          reduce (q, NO_NOTE, &siga, FORMULA, SECONDARY, OPERATOR, FORMULA, STOP);
        } else if (IS (q, MONADIC_FORMULA)) {
          reduce (q, NO_NOTE, &siga, FORMULA, MONADIC_FORMULA, OPERATOR, SECONDARY, STOP);
          reduce (q, NO_NOTE, &siga, FORMULA, MONADIC_FORMULA, OPERATOR, MONADIC_FORMULA, STOP);
          reduce (q, NO_NOTE, &siga, FORMULA, MONADIC_FORMULA, OPERATOR, FORMULA, STOP);
        }
        if (prio == 0 && siga) {
          diagnostic (A68_SYNTAX_ERROR, op, ERROR_NO_PRIORITY);
        }
        siga = A68_TRUE;
        while (siga) {
          NODE_T *op2 = NEXT (q);
          siga = A68_FALSE;
          if (operator_with_priority (q, prio)) {
            reduce (q, NO_NOTE, &siga, FORMULA, FORMULA, OPERATOR, SECONDARY, STOP);
          }
          if (operator_with_priority (q, prio)) {
            reduce (q, NO_NOTE, &siga, FORMULA, FORMULA, OPERATOR, MONADIC_FORMULA, STOP);
          }
          if (operator_with_priority (q, prio)) {
            reduce (q, NO_NOTE, &siga, FORMULA, FORMULA, OPERATOR, FORMULA, STOP);
          }
          if (prio == 0 && siga) {
            diagnostic (A68_SYNTAX_ERROR, op2, ERROR_NO_PRIORITY);
          }
        }
      }
    }
  }
}

//! @brief Reduce dyadic expressions.

NODE_T *reduce_dyadic (NODE_T * p, int u)
{
// We work inside out - higher priority expressions get reduced first.
  if (u > MAX_PRIORITY) {
    if (p == NO_NODE) {
      return NO_NODE;
    } else if (IS (p, OPERATOR)) {
// Reduce monadic formulas.
      NODE_T *q = p;
      BOOL_T siga;
      do {
        PRIO (INFO (q)) = 10;
        siga = (BOOL_T) ((NEXT (q) != NO_NODE) && (IS (NEXT (q), OPERATOR)));
        if (siga) {
          FORWARD (q);
        }
      } while (siga);
      reduce (q, NO_NOTE, NO_TICK, MONADIC_FORMULA, OPERATOR, SECONDARY, STOP);
      while (q != p) {
        BACKWARD (q);
        reduce (q, NO_NOTE, NO_TICK, MONADIC_FORMULA, OPERATOR, MONADIC_FORMULA, STOP);
      }
    }
    FORWARD (p);
  } else {
    p = reduce_dyadic (p, u + 1);
    while (p != NO_NODE && IS (p, OPERATOR) && PRIO (INFO (p)) == u) {
      FORWARD (p);
      p = reduce_dyadic (p, u + 1);
    }
  }
  return p;
}

//! @brief Reduce tertiaries completely.

void reduce_tertiaries (NODE_T * p)
{
  BOOL_T siga;
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, TERTIARY, NIHIL, STOP);
    reduce (q, NO_NOTE, NO_TICK, FORMULA, MONADIC_FORMULA, STOP);
    reduce (q, NO_NOTE, NO_TICK, TERTIARY, FORMULA, STOP);
    reduce (q, NO_NOTE, NO_TICK, TERTIARY, SECONDARY, STOP);
  }
  siga = A68_TRUE;
  while (siga) {
    siga = A68_FALSE;
    for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
      reduce (q, NO_NOTE, &siga, TRANSPOSE_FUNCTION, TRANSPOSE_SYMBOL, TERTIARY, STOP);
      reduce (q, NO_NOTE, &siga, DIAGONAL_FUNCTION, TERTIARY, DIAGONAL_SYMBOL, TERTIARY, STOP);
      reduce (q, NO_NOTE, &siga, DIAGONAL_FUNCTION, DIAGONAL_SYMBOL, TERTIARY, STOP);
      reduce (q, NO_NOTE, &siga, COLUMN_FUNCTION, TERTIARY, COLUMN_SYMBOL, TERTIARY, STOP);
      reduce (q, NO_NOTE, &siga, COLUMN_FUNCTION, COLUMN_SYMBOL, TERTIARY, STOP);
      reduce (q, NO_NOTE, &siga, ROW_FUNCTION, TERTIARY, ROW_SYMBOL, TERTIARY, STOP);
      reduce (q, NO_NOTE, &siga, ROW_FUNCTION, ROW_SYMBOL, TERTIARY, STOP);
    }
    for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
      reduce (q, a68_extension, &siga, TERTIARY, TRANSPOSE_FUNCTION, STOP);
      reduce (q, a68_extension, &siga, TERTIARY, DIAGONAL_FUNCTION, STOP);
      reduce (q, a68_extension, &siga, TERTIARY, COLUMN_FUNCTION, STOP);
      reduce (q, a68_extension, &siga, TERTIARY, ROW_FUNCTION, STOP);
    }
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, IDENTITY_RELATION, TERTIARY, IS_SYMBOL, TERTIARY, STOP);
    reduce (q, NO_NOTE, NO_TICK, IDENTITY_RELATION, TERTIARY, ISNT_SYMBOL, TERTIARY, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, AND_FUNCTION, TERTIARY, ANDF_SYMBOL, TERTIARY, STOP);
    reduce (q, NO_NOTE, NO_TICK, OR_FUNCTION, TERTIARY, ORF_SYMBOL, TERTIARY, STOP);
  }
}

//! @brief Reduce units.

void reduce_units (NODE_T * p)
{
// Stray ~ is a SKIP.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    if (IS (q, OPERATOR) && IS_LITERALLY (q, "~")) {
      ATTRIBUTE (q) = SKIP;
    }
  }
// Reduce units.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, UNIT, ASSIGNATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, IDENTITY_RELATION, STOP);
    reduce (q, a68_extension, NO_TICK, UNIT, AND_FUNCTION, STOP);
    reduce (q, a68_extension, NO_TICK, UNIT, OR_FUNCTION, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, ROUTINE_TEXT, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, JUMP, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, SKIP, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, TERTIARY, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, ASSERTION, STOP);
    reduce (q, NO_NOTE, NO_TICK, UNIT, CODE_CLAUSE, STOP);
  }
}

//! @brief Reduce_generic arguments.

void reduce_generic_arguments (NODE_T * p)
{
  NODE_T *q; BOOL_T siga; // In this scope.
  for (q = p; q != NO_NODE; FORWARD (q)) {
    if (IS (q, UNIT)) {
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, COLON_SYMBOL, UNIT, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, COLON_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, COLON_SYMBOL, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, COLON_SYMBOL, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, DOTDOT_SYMBOL, UNIT, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, DOTDOT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, DOTDOT_SYMBOL, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, DOTDOT_SYMBOL, STOP);
    } else if (IS (q, COLON_SYMBOL)) {
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, COLON_SYMBOL, UNIT, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, COLON_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, COLON_SYMBOL, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, COLON_SYMBOL, STOP);
    } else if (IS (q, DOTDOT_SYMBOL)) {
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, DOTDOT_SYMBOL, UNIT, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, DOTDOT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, DOTDOT_SYMBOL, AT_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, NO_TICK, TRIMMER, DOTDOT_SYMBOL, STOP);
    }
  }
  for (q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, TRIMMER, UNIT, AT_SYMBOL, UNIT, STOP);
  }
  for (q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, TRIMMER, AT_SYMBOL, UNIT, STOP);
  }
  for (q = p; q && NEXT (q); FORWARD (q)) {
    if (IS (q, COMMA_SYMBOL)) {
      if (!(ATTRIBUTE (NEXT (q)) == UNIT || ATTRIBUTE (NEXT (q)) == TRIMMER)) {
        pad_node (q, TRIMMER);
      }
    } else {
      if (IS (NEXT (q), COMMA_SYMBOL)) {
        if (!IS (q, UNIT) && !IS (q, TRIMMER)) {
          pad_node (q, TRIMMER);
        }
      }
    }
  }
  q = NEXT (p);
  ABEND (q == NO_NODE, ERROR_INTERNAL_CONSISTENCY, __func__);
  reduce (q, NO_NOTE, NO_TICK, GENERIC_ARGUMENT_LIST, UNIT, STOP);
  reduce (q, NO_NOTE, NO_TICK, GENERIC_ARGUMENT_LIST, TRIMMER, STOP);
  do {
    siga = A68_FALSE;
    reduce (q, NO_NOTE, &siga, GENERIC_ARGUMENT_LIST, GENERIC_ARGUMENT_LIST, COMMA_SYMBOL, UNIT, STOP);
    reduce (q, NO_NOTE, &siga, GENERIC_ARGUMENT_LIST, GENERIC_ARGUMENT_LIST, COMMA_SYMBOL, TRIMMER, STOP);
    reduce (q, strange_separator, &siga, GENERIC_ARGUMENT_LIST, GENERIC_ARGUMENT_LIST, UNIT, STOP);
    reduce (q, strange_separator, &siga, GENERIC_ARGUMENT_LIST, GENERIC_ARGUMENT_LIST, TRIMMER, STOP);
  } while (siga);
}

//! @brief Reduce bounds.

void reduce_bounds (NODE_T * p)
{
  NODE_T *q; BOOL_T siga;
  for (q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, BOUND, UNIT, COLON_SYMBOL, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, BOUND, UNIT, DOTDOT_SYMBOL, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, BOUND, UNIT, STOP);
  }
  q = NEXT (p);
  reduce (q, NO_NOTE, NO_TICK, BOUNDS_LIST, BOUND, STOP);
  reduce (q, NO_NOTE, NO_TICK, FORMAL_BOUNDS_LIST, COMMA_SYMBOL, STOP);
  reduce (q, NO_NOTE, NO_TICK, ALT_FORMAL_BOUNDS_LIST, COLON_SYMBOL, STOP);
  reduce (q, NO_NOTE, NO_TICK, ALT_FORMAL_BOUNDS_LIST, DOTDOT_SYMBOL, STOP);
  do {
    siga = A68_FALSE;
    reduce (q, NO_NOTE, &siga, BOUNDS_LIST, BOUNDS_LIST, COMMA_SYMBOL, BOUND, STOP);
    reduce (q, NO_NOTE, &siga, FORMAL_BOUNDS_LIST, FORMAL_BOUNDS_LIST, COMMA_SYMBOL, STOP);
    reduce (q, NO_NOTE, &siga, ALT_FORMAL_BOUNDS_LIST, FORMAL_BOUNDS_LIST, COLON_SYMBOL, STOP);
    reduce (q, NO_NOTE, &siga, ALT_FORMAL_BOUNDS_LIST, FORMAL_BOUNDS_LIST, DOTDOT_SYMBOL, STOP);
    reduce (q, NO_NOTE, &siga, FORMAL_BOUNDS_LIST, ALT_FORMAL_BOUNDS_LIST, COMMA_SYMBOL, STOP);
    reduce (q, strange_separator, &siga, BOUNDS_LIST, BOUNDS_LIST, BOUND, STOP);
  } while (siga);
}

//! @brief Reduce argument packs.

void reduce_arguments (NODE_T * p)
{
  if (NEXT (p) != NO_NODE) {
    NODE_T *q = NEXT (p);
    BOOL_T siga;
    reduce (q, NO_NOTE, NO_TICK, ARGUMENT_LIST, UNIT, STOP);
    do {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, ARGUMENT_LIST, ARGUMENT_LIST, COMMA_SYMBOL, UNIT, STOP);
      reduce (q, strange_separator, &siga, ARGUMENT_LIST, ARGUMENT_LIST, UNIT, STOP);
    } while (siga);
  }
}

//! @brief Reduce declarations.

void reduce_basic_declarations (NODE_T * p)
{
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, ENVIRON_NAME, ENVIRON_SYMBOL, ROW_CHAR_DENOTATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, PRIORITY_DECLARATION, PRIO_SYMBOL, DEFINING_OPERATOR, EQUALS_SYMBOL, PRIORITY, STOP);
    reduce (q, NO_NOTE, NO_TICK, MODE_DECLARATION, MODE_SYMBOL, DEFINING_INDICANT, EQUALS_SYMBOL, DECLARER, STOP);
    reduce (q, NO_NOTE, NO_TICK, MODE_DECLARATION, MODE_SYMBOL, DEFINING_INDICANT, EQUALS_SYMBOL, VOID_SYMBOL, STOP);
    reduce (q, NO_NOTE, NO_TICK, PROCEDURE_DECLARATION, PROC_SYMBOL, DEFINING_IDENTIFIER, EQUALS_SYMBOL, ROUTINE_TEXT, STOP);
    reduce (q, NO_NOTE, NO_TICK, PROCEDURE_VARIABLE_DECLARATION, PROC_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, ROUTINE_TEXT, STOP);
    reduce (q, NO_NOTE, NO_TICK, PROCEDURE_VARIABLE_DECLARATION, QUALIFIER, PROC_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, ROUTINE_TEXT, STOP);
    reduce (q, NO_NOTE, NO_TICK, BRIEF_OPERATOR_DECLARATION, OP_SYMBOL, DEFINING_OPERATOR, EQUALS_SYMBOL, ROUTINE_TEXT, STOP);
// Errors.
    reduce (q, strange_tokens, NO_TICK, PRIORITY_DECLARATION, PRIO_SYMBOL, -DEFINING_OPERATOR, -EQUALS_SYMBOL, -PRIORITY, STOP);
    reduce (q, strange_tokens, NO_TICK, MODE_DECLARATION, MODE_SYMBOL, DEFINING_INDICANT, EQUALS_SYMBOL, -DECLARER, STOP);
    reduce (q, strange_tokens, NO_TICK, PROCEDURE_DECLARATION, PROC_SYMBOL, DEFINING_IDENTIFIER, EQUALS_SYMBOL, -ROUTINE_TEXT, STOP);
    reduce (q, strange_tokens, NO_TICK, PROCEDURE_VARIABLE_DECLARATION, PROC_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, -ROUTINE_TEXT, STOP);
    reduce (q, strange_tokens, NO_TICK, PROCEDURE_VARIABLE_DECLARATION, QUALIFIER, PROC_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, -ROUTINE_TEXT, STOP);
    reduce (q, strange_tokens, NO_TICK, BRIEF_OPERATOR_DECLARATION, OP_SYMBOL, DEFINING_OPERATOR, EQUALS_SYMBOL, -ROUTINE_TEXT, STOP);
// Errors. WILDCARD catches TERTIARY which catches IDENTIFIER.
    reduce (q, strange_tokens, NO_TICK, PROCEDURE_DECLARATION, PROC_SYMBOL, WILDCARD, ROUTINE_TEXT, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga;
    do {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, ENVIRON_NAME, ENVIRON_NAME, COMMA_SYMBOL, ROW_CHAR_DENOTATION, STOP);
      reduce (q, NO_NOTE, &siga, PRIORITY_DECLARATION, PRIORITY_DECLARATION, COMMA_SYMBOL, DEFINING_OPERATOR, EQUALS_SYMBOL, PRIORITY, STOP);
      reduce (q, NO_NOTE, &siga, MODE_DECLARATION, MODE_DECLARATION, COMMA_SYMBOL, DEFINING_INDICANT, EQUALS_SYMBOL, DECLARER, STOP);
      reduce (q, NO_NOTE, &siga, MODE_DECLARATION, MODE_DECLARATION, COMMA_SYMBOL, DEFINING_INDICANT, EQUALS_SYMBOL, VOID_SYMBOL, STOP);
      reduce (q, NO_NOTE, &siga, PROCEDURE_DECLARATION, PROCEDURE_DECLARATION, COMMA_SYMBOL, DEFINING_IDENTIFIER, EQUALS_SYMBOL, ROUTINE_TEXT, STOP);
      reduce (q, NO_NOTE, &siga, PROCEDURE_VARIABLE_DECLARATION, PROCEDURE_VARIABLE_DECLARATION, COMMA_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, ROUTINE_TEXT, STOP);
      reduce (q, NO_NOTE, &siga, BRIEF_OPERATOR_DECLARATION, BRIEF_OPERATOR_DECLARATION, COMMA_SYMBOL, DEFINING_OPERATOR, EQUALS_SYMBOL, ROUTINE_TEXT, STOP);
// Errors. WILDCARD catches TERTIARY which catches IDENTIFIER.
      reduce (q, strange_tokens, &siga, PROCEDURE_DECLARATION, PROCEDURE_DECLARATION, COMMA_SYMBOL, WILDCARD, ROUTINE_TEXT, STOP);
    } while (siga);
  }
}

//! @brief Reduce declaration lists.

void reduce_declaration_lists (NODE_T * p)
{
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, IDENTITY_DECLARATION, DECLARER, DEFINING_IDENTIFIER, EQUALS_SYMBOL, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, VARIABLE_DECLARATION, QUALIFIER, DECLARER, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, VARIABLE_DECLARATION, QUALIFIER, DECLARER, DEFINING_IDENTIFIER, STOP);
    reduce (q, NO_NOTE, NO_TICK, VARIABLE_DECLARATION, DECLARER, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, VARIABLE_DECLARATION, DECLARER, DEFINING_IDENTIFIER, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga;
    do {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, IDENTITY_DECLARATION, IDENTITY_DECLARATION, COMMA_SYMBOL, DEFINING_IDENTIFIER, EQUALS_SYMBOL, UNIT, STOP);
      reduce (q, NO_NOTE, &siga, VARIABLE_DECLARATION, VARIABLE_DECLARATION, COMMA_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, UNIT, STOP);
      if (!whether (q, VARIABLE_DECLARATION, COMMA_SYMBOL, DEFINING_IDENTIFIER, ASSIGN_SYMBOL, UNIT, STOP)) {
        reduce (q, NO_NOTE, &siga, VARIABLE_DECLARATION, VARIABLE_DECLARATION, COMMA_SYMBOL, DEFINING_IDENTIFIER, STOP);
      }
    } while (siga);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, OPERATOR_DECLARATION, OPERATOR_PLAN, DEFINING_OPERATOR, EQUALS_SYMBOL, UNIT, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga;
    do {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, OPERATOR_DECLARATION, OPERATOR_DECLARATION, COMMA_SYMBOL, DEFINING_OPERATOR, EQUALS_SYMBOL, UNIT, STOP);
    } while (siga);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, MODE_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, PRIORITY_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, BRIEF_OPERATOR_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, OPERATOR_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, IDENTITY_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, PROCEDURE_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, PROCEDURE_VARIABLE_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, VARIABLE_DECLARATION, STOP);
    reduce (q, NO_NOTE, NO_TICK, DECLARATION_LIST, ENVIRON_NAME, STOP);
  }
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
    BOOL_T siga;
    do {
      siga = A68_FALSE;
      reduce (q, NO_NOTE, &siga, DECLARATION_LIST, DECLARATION_LIST, COMMA_SYMBOL, DECLARATION_LIST, STOP);
    } while (siga);
  }
}

//! @brief Reduce serial clauses.

void reduce_serial_clauses (NODE_T * p)
{
  if (NEXT (p) != NO_NODE) {
    NODE_T *q = NEXT (p), *u;
    BOOL_T siga, label_seen;
// Check wrong exits.
    for (u = q; u != NO_NODE; FORWARD (u)) {
      if (IS (u, EXIT_SYMBOL)) {
        if (NEXT (u) == NO_NODE || !IS (NEXT (u), LABELED_UNIT)) {
          diagnostic (A68_SYNTAX_ERROR, u, ERROR_LABELED_UNIT_MUST_FOLLOW);
        }
      }
    }
// Check wrong jumps and declarations.
    for (u = q, label_seen = A68_FALSE; u != NO_NODE; FORWARD (u)) {
      if (IS (u, LABELED_UNIT)) {
        label_seen = A68_TRUE;
      } else if (IS (u, DECLARATION_LIST)) {
        if (label_seen) {
          diagnostic (A68_SYNTAX_ERROR, u, ERROR_LABEL_BEFORE_DECLARATION);
        }
      }
    }
// Reduce serial clauses.
    reduce (q, NO_NOTE, NO_TICK, SERIAL_CLAUSE, LABELED_UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, SERIAL_CLAUSE, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, INITIALISER_SERIES, DECLARATION_LIST, STOP);
    do {
      siga = A68_FALSE;
      if (IS (q, SERIAL_CLAUSE)) {
        reduce (q, NO_NOTE, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, SEMI_SYMBOL, UNIT, STOP);
        reduce (q, NO_NOTE, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, EXIT_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, NO_NOTE, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, SEMI_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, NO_NOTE, &siga, INITIALISER_SERIES, SERIAL_CLAUSE, SEMI_SYMBOL, DECLARATION_LIST, STOP);
// Errors 
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, COMMA_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, COMMA_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, SERIAL_CLAUSE, COMMA_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, COLON_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, COLON_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, SERIAL_CLAUSE, COLON_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, UNIT, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, SERIAL_CLAUSE, LABELED_UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, SERIAL_CLAUSE, DECLARATION_LIST, STOP);
      } else if (IS (q, INITIALISER_SERIES)) {
        reduce (q, NO_NOTE, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, SEMI_SYMBOL, UNIT, STOP);
        reduce (q, NO_NOTE, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, SEMI_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, NO_NOTE, &siga, INITIALISER_SERIES, INITIALISER_SERIES, SEMI_SYMBOL, DECLARATION_LIST, STOP);
// Errors 
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, COMMA_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, COMMA_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, INITIALISER_SERIES, COMMA_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, COLON_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, COLON_SYMBOL, LABELED_UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, INITIALISER_SERIES, COLON_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, UNIT, STOP);
        reduce (q, strange_separator, &siga, SERIAL_CLAUSE, INITIALISER_SERIES, LABELED_UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, INITIALISER_SERIES, DECLARATION_LIST, STOP);
      }
    }
    while (siga);
  }
}

//! @brief Reduce enquiry clauses.

void reduce_enquiry_clauses (NODE_T * p)
{
  if (NEXT (p) != NO_NODE) {
    NODE_T *q = NEXT (p);
    BOOL_T siga;
    reduce (q, NO_NOTE, NO_TICK, ENQUIRY_CLAUSE, UNIT, STOP);
    reduce (q, NO_NOTE, NO_TICK, INITIALISER_SERIES, DECLARATION_LIST, STOP);
    do {
      siga = A68_FALSE;
      if (IS (q, ENQUIRY_CLAUSE)) {
        reduce (q, NO_NOTE, &siga, ENQUIRY_CLAUSE, ENQUIRY_CLAUSE, SEMI_SYMBOL, UNIT, STOP);
        reduce (q, NO_NOTE, &siga, INITIALISER_SERIES, ENQUIRY_CLAUSE, SEMI_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, ENQUIRY_CLAUSE, ENQUIRY_CLAUSE, COMMA_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, ENQUIRY_CLAUSE, COMMA_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, ENQUIRY_CLAUSE, ENQUIRY_CLAUSE, COLON_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, ENQUIRY_CLAUSE, COLON_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, ENQUIRY_CLAUSE, ENQUIRY_CLAUSE, UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, ENQUIRY_CLAUSE, DECLARATION_LIST, STOP);
      } else if (IS (q, INITIALISER_SERIES)) {
        reduce (q, NO_NOTE, &siga, ENQUIRY_CLAUSE, INITIALISER_SERIES, SEMI_SYMBOL, UNIT, STOP);
        reduce (q, NO_NOTE, &siga, INITIALISER_SERIES, INITIALISER_SERIES, SEMI_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, ENQUIRY_CLAUSE, INITIALISER_SERIES, COMMA_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, INITIALISER_SERIES, COMMA_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, ENQUIRY_CLAUSE, INITIALISER_SERIES, COLON_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, INITIALISER_SERIES, COLON_SYMBOL, DECLARATION_LIST, STOP);
        reduce (q, strange_separator, &siga, ENQUIRY_CLAUSE, INITIALISER_SERIES, UNIT, STOP);
        reduce (q, strange_separator, &siga, INITIALISER_SERIES, INITIALISER_SERIES, DECLARATION_LIST, STOP);
      }
    }
    while (siga);
  }
}

//! @brief Reduce collateral clauses.

void reduce_collateral_clauses (NODE_T * p)
{
  if (NEXT (p) != NO_NODE) {
    NODE_T *q = NEXT (p);
    if (IS (q, UNIT)) {
      BOOL_T siga;
      reduce (q, NO_NOTE, NO_TICK, UNIT_LIST, UNIT, STOP);
      do {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, UNIT_LIST, UNIT_LIST, COMMA_SYMBOL, UNIT, STOP);
        reduce (q, strange_separator, &siga, UNIT_LIST, UNIT_LIST, UNIT, STOP);
      } while (siga);
    } else if (IS (q, SPECIFIED_UNIT)) {
      BOOL_T siga;
      reduce (q, NO_NOTE, NO_TICK, SPECIFIED_UNIT_LIST, SPECIFIED_UNIT, STOP);
      do {
        siga = A68_FALSE;
        reduce (q, NO_NOTE, &siga, SPECIFIED_UNIT_LIST, SPECIFIED_UNIT_LIST, COMMA_SYMBOL, SPECIFIED_UNIT, STOP);
        reduce (q, strange_separator, &siga, SPECIFIED_UNIT_LIST, SPECIFIED_UNIT_LIST, SPECIFIED_UNIT, STOP);
      } while (siga);
    }
  }
}

//! @brief Reduces enclosed clauses.

void reduce_enclosed_clauses (NODE_T * q, int expect)
{
  NODE_T *p = q;
  if (SUB (p) == NO_NODE) {
    if (IS (p, FOR_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, FOR_PART, FOR_SYMBOL, DEFINING_IDENTIFIER, STOP);
    } else if (IS (p, OPEN_SYMBOL)) {
      if (expect == ENQUIRY_CLAUSE) {
        reduce (p, NO_NOTE, NO_TICK, OPEN_PART, OPEN_SYMBOL, ENQUIRY_CLAUSE, STOP);
      } else if (expect == ARGUMENT) {
        reduce (p, NO_NOTE, NO_TICK, ARGUMENT, OPEN_SYMBOL, CLOSE_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, ARGUMENT, OPEN_SYMBOL, ARGUMENT_LIST, CLOSE_SYMBOL, STOP);
        reduce (p, empty_clause, NO_TICK, ARGUMENT, OPEN_SYMBOL, INITIALISER_SERIES, CLOSE_SYMBOL, STOP);
      } else if (expect == GENERIC_ARGUMENT) {
        if (whether (p, OPEN_SYMBOL, CLOSE_SYMBOL, STOP)) {
          pad_node (p, TRIMMER);
          reduce (p, NO_NOTE, NO_TICK, GENERIC_ARGUMENT, OPEN_SYMBOL, TRIMMER, CLOSE_SYMBOL, STOP);
        }
        reduce (p, NO_NOTE, NO_TICK, GENERIC_ARGUMENT, OPEN_SYMBOL, GENERIC_ARGUMENT_LIST, CLOSE_SYMBOL, STOP);
      } else if (expect == BOUNDS) {
        reduce (p, NO_NOTE, NO_TICK, FORMAL_BOUNDS, OPEN_SYMBOL, CLOSE_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, BOUNDS, OPEN_SYMBOL, BOUNDS_LIST, CLOSE_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, FORMAL_BOUNDS, OPEN_SYMBOL, FORMAL_BOUNDS_LIST, CLOSE_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, FORMAL_BOUNDS, OPEN_SYMBOL, ALT_FORMAL_BOUNDS_LIST, CLOSE_SYMBOL, STOP);
      } else {
        reduce (p, NO_NOTE, NO_TICK, CLOSED_CLAUSE, OPEN_SYMBOL, SERIAL_CLAUSE, CLOSE_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, COLLATERAL_CLAUSE, OPEN_SYMBOL, UNIT_LIST, CLOSE_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, COLLATERAL_CLAUSE, OPEN_SYMBOL, CLOSE_SYMBOL, STOP);
        reduce (p, empty_clause, NO_TICK, CLOSED_CLAUSE, OPEN_SYMBOL, INITIALISER_SERIES, CLOSE_SYMBOL, STOP);
      }
    } else if (IS (p, SUB_SYMBOL)) {
      if (expect == GENERIC_ARGUMENT) {
        if (whether (p, SUB_SYMBOL, BUS_SYMBOL, STOP)) {
          pad_node (p, TRIMMER);
          reduce (p, NO_NOTE, NO_TICK, GENERIC_ARGUMENT, SUB_SYMBOL, TRIMMER, BUS_SYMBOL, STOP);
        }
        reduce (p, NO_NOTE, NO_TICK, GENERIC_ARGUMENT, SUB_SYMBOL, GENERIC_ARGUMENT_LIST, BUS_SYMBOL, STOP);
      } else if (expect == BOUNDS) {
        reduce (p, NO_NOTE, NO_TICK, FORMAL_BOUNDS, SUB_SYMBOL, BUS_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, BOUNDS, SUB_SYMBOL, BOUNDS_LIST, BUS_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, FORMAL_BOUNDS, SUB_SYMBOL, FORMAL_BOUNDS_LIST, BUS_SYMBOL, STOP);
        reduce (p, NO_NOTE, NO_TICK, FORMAL_BOUNDS, SUB_SYMBOL, ALT_FORMAL_BOUNDS_LIST, BUS_SYMBOL, STOP);
      }
    } else if (IS (p, BEGIN_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, COLLATERAL_CLAUSE, BEGIN_SYMBOL, UNIT_LIST, END_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, COLLATERAL_CLAUSE, BEGIN_SYMBOL, END_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CLOSED_CLAUSE, BEGIN_SYMBOL, SERIAL_CLAUSE, END_SYMBOL, STOP);
      reduce (p, empty_clause, NO_TICK, CLOSED_CLAUSE, BEGIN_SYMBOL, INITIALISER_SERIES, END_SYMBOL, STOP);
    } else if (IS (p, FORMAT_DELIMITER_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, FORMAT_TEXT, FORMAT_DELIMITER_SYMBOL, PICTURE_LIST, FORMAT_DELIMITER_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, FORMAT_TEXT, FORMAT_DELIMITER_SYMBOL, FORMAT_DELIMITER_SYMBOL, STOP);
    } else if (IS (p, FORMAT_OPEN_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, COLLECTION, FORMAT_OPEN_SYMBOL, PICTURE_LIST, FORMAT_CLOSE_SYMBOL, STOP);
    } else if (IS (p, IF_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, IF_PART, IF_SYMBOL, ENQUIRY_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, IF_PART, IF_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, THEN_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, THEN_PART, THEN_SYMBOL, SERIAL_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, THEN_PART, THEN_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, ELSE_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, ELSE_PART, ELSE_SYMBOL, SERIAL_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, ELSE_PART, ELSE_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, ELIF_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, ELIF_IF_PART, ELIF_SYMBOL, ENQUIRY_CLAUSE, STOP);
    } else if (IS (p, CASE_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, CASE_PART, CASE_SYMBOL, ENQUIRY_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, CASE_PART, CASE_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, IN_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, CASE_IN_PART, IN_SYMBOL, UNIT_LIST, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_IN_PART, IN_SYMBOL, SPECIFIED_UNIT_LIST, STOP);
    } else if (IS (p, OUT_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, OUT_PART, OUT_SYMBOL, SERIAL_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, OUT_PART, OUT_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, OUSE_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, OUSE_PART, OUSE_SYMBOL, ENQUIRY_CLAUSE, STOP);
    } else if (IS (p, THEN_BAR_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, CHOICE, THEN_BAR_SYMBOL, SERIAL_CLAUSE, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_CHOICE_CLAUSE, THEN_BAR_SYMBOL, UNIT_LIST, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CHOICE, THEN_BAR_SYMBOL, SPECIFIED_UNIT_LIST, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CHOICE, THEN_BAR_SYMBOL, SPECIFIED_UNIT, STOP);
      reduce (p, empty_clause, NO_TICK, CHOICE, THEN_BAR_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, ELSE_BAR_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, ELSE_OPEN_PART, ELSE_BAR_SYMBOL, ENQUIRY_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, ELSE_OPEN_PART, ELSE_BAR_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, FROM_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, FROM_PART, FROM_SYMBOL, UNIT, STOP);
    } else if (IS (p, BY_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, BY_PART, BY_SYMBOL, UNIT, STOP);
    } else if (IS (p, TO_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, TO_PART, TO_SYMBOL, UNIT, STOP);
    } else if (IS (p, DOWNTO_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, TO_PART, DOWNTO_SYMBOL, UNIT, STOP);
    } else if (IS (p, WHILE_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, WHILE_PART, WHILE_SYMBOL, ENQUIRY_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, WHILE_PART, WHILE_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, UNTIL_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, UNTIL_PART, UNTIL_SYMBOL, ENQUIRY_CLAUSE, STOP);
      reduce (p, empty_clause, NO_TICK, UNTIL_PART, UNTIL_SYMBOL, INITIALISER_SERIES, STOP);
    } else if (IS (p, DO_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, DO_PART, DO_SYMBOL, SERIAL_CLAUSE, UNTIL_PART, OD_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, DO_PART, DO_SYMBOL, SERIAL_CLAUSE, OD_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, DO_PART, DO_SYMBOL, UNTIL_PART, OD_SYMBOL, STOP);
    } else if (IS (p, ALT_DO_SYMBOL)) {
      reduce (p, NO_NOTE, NO_TICK, ALT_DO_PART, ALT_DO_SYMBOL, SERIAL_CLAUSE, UNTIL_PART, OD_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, ALT_DO_PART, ALT_DO_SYMBOL, SERIAL_CLAUSE, OD_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, ALT_DO_PART, ALT_DO_SYMBOL, UNTIL_PART, OD_SYMBOL, STOP);
    }
  }
  p = q;
  if (SUB (p) != NO_NODE) {
    if (IS (p, OPEN_PART)) {
      reduce (p, NO_NOTE, NO_TICK, CONDITIONAL_CLAUSE, OPEN_PART, CHOICE, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONDITIONAL_CLAUSE, OPEN_PART, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONDITIONAL_CLAUSE, OPEN_PART, CHOICE, BRIEF_ELIF_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_CLAUSE, OPEN_PART, CASE_CHOICE_CLAUSE, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_CLAUSE, OPEN_PART, CASE_CHOICE_CLAUSE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_CLAUSE, OPEN_PART, CASE_CHOICE_CLAUSE, BRIEF_OUSE_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CLAUSE, OPEN_PART, CONFORMITY_CHOICE, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CLAUSE, OPEN_PART, CONFORMITY_CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CLAUSE, OPEN_PART, CONFORMITY_CHOICE, BRIEF_CONFORMITY_OUSE_PART, STOP);
    } else if (IS (p, ELSE_OPEN_PART)) {
      reduce (p, NO_NOTE, NO_TICK, BRIEF_ELIF_PART, ELSE_OPEN_PART, CHOICE, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_ELIF_PART, ELSE_OPEN_PART, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_ELIF_PART, ELSE_OPEN_PART, CHOICE, BRIEF_ELIF_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_OUSE_PART, ELSE_OPEN_PART, CASE_CHOICE_CLAUSE, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_OUSE_PART, ELSE_OPEN_PART, CASE_CHOICE_CLAUSE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_OUSE_PART, ELSE_OPEN_PART, CASE_CHOICE_CLAUSE, BRIEF_OUSE_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_CONFORMITY_OUSE_PART, ELSE_OPEN_PART, CONFORMITY_CHOICE, CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_CONFORMITY_OUSE_PART, ELSE_OPEN_PART, CONFORMITY_CHOICE, CLOSE_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, BRIEF_CONFORMITY_OUSE_PART, ELSE_OPEN_PART, CONFORMITY_CHOICE, BRIEF_CONFORMITY_OUSE_PART, STOP);
    } else if (IS (p, IF_PART)) {
      reduce (p, NO_NOTE, NO_TICK, CONDITIONAL_CLAUSE, IF_PART, THEN_PART, ELSE_PART, FI_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONDITIONAL_CLAUSE, IF_PART, THEN_PART, ELIF_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONDITIONAL_CLAUSE, IF_PART, THEN_PART, FI_SYMBOL, STOP);
    } else if (IS (p, ELIF_IF_PART)) {
      reduce (p, NO_NOTE, NO_TICK, ELIF_PART, ELIF_IF_PART, THEN_PART, ELSE_PART, FI_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, ELIF_PART, ELIF_IF_PART, THEN_PART, FI_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, ELIF_PART, ELIF_IF_PART, THEN_PART, ELIF_PART, STOP);
    } else if (IS (p, CASE_PART)) {
      reduce (p, NO_NOTE, NO_TICK, CASE_CLAUSE, CASE_PART, CASE_IN_PART, OUT_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_CLAUSE, CASE_PART, CASE_IN_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_CLAUSE, CASE_PART, CASE_IN_PART, CASE_OUSE_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CLAUSE, CASE_PART, CONFORMITY_IN_PART, OUT_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CLAUSE, CASE_PART, CONFORMITY_IN_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_CLAUSE, CASE_PART, CONFORMITY_IN_PART, CONFORMITY_OUSE_PART, STOP);
    } else if (IS (p, OUSE_PART)) {
      reduce (p, NO_NOTE, NO_TICK, CASE_OUSE_PART, OUSE_PART, CASE_IN_PART, OUT_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_OUSE_PART, OUSE_PART, CASE_IN_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CASE_OUSE_PART, OUSE_PART, CASE_IN_PART, CASE_OUSE_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_OUSE_PART, OUSE_PART, CONFORMITY_IN_PART, OUT_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_OUSE_PART, OUSE_PART, CONFORMITY_IN_PART, ESAC_SYMBOL, STOP);
      reduce (p, NO_NOTE, NO_TICK, CONFORMITY_OUSE_PART, OUSE_PART, CONFORMITY_IN_PART, CONFORMITY_OUSE_PART, STOP);
    } else if (IS (p, FOR_PART)) {
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, BY_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, BY_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, BY_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, BY_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, BY_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, BY_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, FROM_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, BY_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, BY_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FOR_PART, ALT_DO_PART, STOP);
    } else if (IS (p, FROM_PART)) {
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, BY_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, BY_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, BY_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, BY_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, FROM_PART, ALT_DO_PART, STOP);
    } else if (IS (p, BY_PART)) {
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, BY_PART, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, BY_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, BY_PART, TO_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, BY_PART, ALT_DO_PART, STOP);
    } else if (IS (p, TO_PART)) {
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, TO_PART, WHILE_PART, ALT_DO_PART, STOP);
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, TO_PART, ALT_DO_PART, STOP);
    } else if (IS (p, WHILE_PART)) {
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, WHILE_PART, ALT_DO_PART, STOP);
    } else if (IS (p, DO_PART)) {
      reduce (p, NO_NOTE, NO_TICK, LOOP_CLAUSE, DO_PART, STOP);
    }
  }
}

//! @brief Substitute reduction when a phrase could not be parsed.

void recover_from_error (NODE_T * p, int expect, BOOL_T suppress)
{
// This routine does not do fancy things as that might introduce more errors.
  NODE_T *q = p;
  if (p == NO_NODE) {
    return;
  }
  if (expect == SOME_CLAUSE) {
    expect = serial_or_collateral (p);
  }
  if (!suppress) {
// Give an error message.
    NODE_T *w = p;
    char *seq = phrase_to_text (p, &w);
    if (strlen (seq) == 0) {
      if (ERROR_COUNT (&A68_JOB) == 0) {
        diagnostic (A68_SYNTAX_ERROR, w, ERROR_SYNTAX_EXPECTED, expect);
      }
    } else {
      diagnostic (A68_SYNTAX_ERROR, w, ERROR_INVALID_SEQUENCE, seq, expect);
    }
    if (ERROR_COUNT (&A68_JOB) >= MAX_ERRORS) {
      longjmp (A68_PARSER (bottom_up_crash_exit), 1);
    }
  }
// Try to prevent spurious diagnostics by guessing what was expected.
  while (NEXT (q) != NO_NODE) {
    FORWARD (q);
  }
  if (is_one_of (p, BEGIN_SYMBOL, OPEN_SYMBOL, STOP)) {
    if (expect == ARGUMENT || expect == COLLATERAL_CLAUSE || expect == PARAMETER_PACK || expect == STRUCTURE_PACK || expect == UNION_PACK) {
      make_sub (p, q, expect);
    } else if (expect == ENQUIRY_CLAUSE) {
      make_sub (p, q, OPEN_PART);
    } else if (expect == FORMAL_DECLARERS) {
      make_sub (p, q, FORMAL_DECLARERS);
    } else {
      make_sub (p, q, CLOSED_CLAUSE);
    }
  } else if (IS (p, FORMAT_DELIMITER_SYMBOL) && expect == FORMAT_TEXT) {
    make_sub (p, q, FORMAT_TEXT);
  } else if (IS (p, CODE_SYMBOL)) {
    make_sub (p, q, CODE_CLAUSE);
  } else if (is_one_of (p, THEN_BAR_SYMBOL, CHOICE, STOP)) {
    make_sub (p, q, CHOICE);
  } else if (is_one_of (p, IF_SYMBOL, IF_PART, STOP)) {
    make_sub (p, q, IF_PART);
  } else if (is_one_of (p, THEN_SYMBOL, THEN_PART, STOP)) {
    make_sub (p, q, THEN_PART);
  } else if (is_one_of (p, ELSE_SYMBOL, ELSE_PART, STOP)) {
    make_sub (p, q, ELSE_PART);
  } else if (is_one_of (p, ELIF_SYMBOL, ELIF_IF_PART, STOP)) {
    make_sub (p, q, ELIF_IF_PART);
  } else if (is_one_of (p, CASE_SYMBOL, CASE_PART, STOP)) {
    make_sub (p, q, CASE_PART);
  } else if (is_one_of (p, OUT_SYMBOL, OUT_PART, STOP)) {
    make_sub (p, q, OUT_PART);
  } else if (is_one_of (p, OUSE_SYMBOL, OUSE_PART, STOP)) {
    make_sub (p, q, OUSE_PART);
  } else if (is_one_of (p, FOR_SYMBOL, FOR_PART, STOP)) {
    make_sub (p, q, FOR_PART);
  } else if (is_one_of (p, FROM_SYMBOL, FROM_PART, STOP)) {
    make_sub (p, q, FROM_PART);
  } else if (is_one_of (p, BY_SYMBOL, BY_PART, STOP)) {
    make_sub (p, q, BY_PART);
  } else if (is_one_of (p, TO_SYMBOL, DOWNTO_SYMBOL, TO_PART, STOP)) {
    make_sub (p, q, TO_PART);
  } else if (is_one_of (p, WHILE_SYMBOL, WHILE_PART, STOP)) {
    make_sub (p, q, WHILE_PART);
  } else if (is_one_of (p, UNTIL_SYMBOL, UNTIL_PART, STOP)) {
    make_sub (p, q, UNTIL_PART);
  } else if (is_one_of (p, DO_SYMBOL, DO_PART, STOP)) {
    make_sub (p, q, DO_PART);
  } else if (is_one_of (p, ALT_DO_SYMBOL, ALT_DO_PART, STOP)) {
    make_sub (p, q, ALT_DO_PART);
  } else if (non_terminal_string (A68 (edit_line), expect) != NO_TEXT) {
    make_sub (p, q, expect);
  }
}

//! @brief Heuristic aid in pinpointing errors.

void reduce_erroneous_units (NODE_T * p)
{
// Constructs are reduced to units in an attempt to limit spurious diagnostics.
  for (NODE_T *q = p; q != NO_NODE; FORWARD (q)) {
// Some implementations allow selection from a tertiary, when there is no risk
// of ambiguity. Algol68G follows RR, so some extra attention here to guide an
// unsuspecting user.
    if (whether (q, SELECTOR, -SECONDARY, STOP)) {
      diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_SYNTAX_EXPECTED, SECONDARY);
      reduce (q, NO_NOTE, NO_TICK, UNIT, SELECTOR, WILDCARD, STOP);
    }
// Attention for identity relations that require tertiaries.
    if (whether (q, -TERTIARY, IS_SYMBOL, TERTIARY, STOP) || whether (q, TERTIARY, IS_SYMBOL, -TERTIARY, STOP) || whether (q, -TERTIARY, IS_SYMBOL, -TERTIARY, STOP)) {
      diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_SYNTAX_EXPECTED, TERTIARY);
      reduce (q, NO_NOTE, NO_TICK, UNIT, WILDCARD, IS_SYMBOL, WILDCARD, STOP);
    } else if (whether (q, -TERTIARY, ISNT_SYMBOL, TERTIARY, STOP) || whether (q, TERTIARY, ISNT_SYMBOL, -TERTIARY, STOP) || whether (q, -TERTIARY, ISNT_SYMBOL, -TERTIARY, STOP)) {
      diagnostic (A68_SYNTAX_ERROR, NEXT (q), ERROR_SYNTAX_EXPECTED, TERTIARY);
      reduce (q, NO_NOTE, NO_TICK, UNIT, WILDCARD, ISNT_SYMBOL, WILDCARD, STOP);
    }
  }
}

// A posteriori checks of the syntax tree built by the BU parser.

//! @brief Driver for a posteriori error checking.

void bottom_up_error_check (NODE_T * p)
{
  for (; p != NO_NODE; FORWARD (p)) {
    if (IS (p, BOOLEAN_PATTERN)) {
      int k = 0;
      count_pictures (SUB (p), &k);
      if (!(k == 0 || k == 2)) {
        diagnostic (A68_SYNTAX_ERROR, p, ERROR_FORMAT_PICTURE_NUMBER, ATTRIBUTE (p));
      }
    } else {
      bottom_up_error_check (SUB (p));
    }
  }
}

// Next part rearranges and checks the tree after the symbol tables are finished.

//! @brief Transfer IDENTIFIER to JUMP where appropriate.

void rearrange_goto_less_jumps (NODE_T * p)
{
  for (; p != NO_NODE; FORWARD (p)) {
    if (IS (p, UNIT)) {
      NODE_T *q = SUB (p);
      if (IS (q, TERTIARY)) {
        NODE_T *tertiary = q;
        q = SUB (q);
        if (q != NO_NODE && IS (q, SECONDARY)) {
          q = SUB (q);
          if (q != NO_NODE && IS (q, PRIMARY)) {
            q = SUB (q);
            if (q != NO_NODE && IS (q, IDENTIFIER)) {
              if (is_identifier_or_label_global (TABLE (q), NSYMBOL (q)) == LABEL) {
                ATTRIBUTE (tertiary) = JUMP;
                SUB (tertiary) = q;
              }
            }
          }
        }
      }
    } else if (IS (p, TERTIARY)) {
      NODE_T *q = SUB (p);
      if (q != NO_NODE && IS (q, SECONDARY)) {
        NODE_T *secondary = q;
        q = SUB (q);
        if (q != NO_NODE && IS (q, PRIMARY)) {
          q = SUB (q);
          if (q != NO_NODE && IS (q, IDENTIFIER)) {
            if (is_identifier_or_label_global (TABLE (q), NSYMBOL (q)) == LABEL) {
              ATTRIBUTE (secondary) = JUMP;
              SUB (secondary) = q;
            }
          }
        }
      }
    } else if (IS (p, SECONDARY)) {
      NODE_T *q = SUB (p);
      if (q != NO_NODE && IS (q, PRIMARY)) {
        NODE_T *primary = q;
        q = SUB (q);
        if (q != NO_NODE && IS (q, IDENTIFIER)) {
          if (is_identifier_or_label_global (TABLE (q), NSYMBOL (q)) == LABEL) {
            ATTRIBUTE (primary) = JUMP;
            SUB (primary) = q;
          }
        }
      }
    } else if (IS (p, PRIMARY)) {
      NODE_T *q = SUB (p);
      if (q != NO_NODE && IS (q, IDENTIFIER)) {
        if (is_identifier_or_label_global (TABLE (q), NSYMBOL (q)) == LABEL) {
          make_sub (q, q, JUMP);
        }
      }
    }
    rearrange_goto_less_jumps (SUB (p));
  }
}
