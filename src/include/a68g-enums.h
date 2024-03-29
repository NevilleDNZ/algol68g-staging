//! @file a68g-enums.h
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
//! Enumerated constants.

#if !defined (__A68G_ENUMS_H__)
#define __A68G_ENUMS_H__

// Enumerated constants

enum
{ UPPER_STROPPING = 1, QUOTE_STROPPING };

enum
{
  NO_OPTIMISE = 0,
  OPTIMISE_0,
  OPTIMISE_1,
  OPTIMISE_2,
  OPTIMISE_3,
  OPTIMISE_FAST,
};

enum
{
  STOP = 0,
  A68_PATTERN,
  ACCO_SYMBOL,
  ACTUAL_DECLARER_MARK,
  ALIF_IF_PART, ALIF_PART, ALIF_SYMBOL,
  ALT_DO_PART, ALT_DO_SYMBOL, ALT_EQUALS_SYMBOL, ALT_FORMAL_BOUNDS_LIST,
  ANDF_SYMBOL,
  AND_FUNCTION,
  ANONYMOUS,
  ARGUMENT, ARGUMENT_LIST,
  ASSERTION, ASSERT_SYMBOL,
  ASSIGNATION, ASSIGN_SYMBOL, ASSIGN_TO_SYMBOL,
  AT_SYMBOL,
  BEGIN_SYMBOL,
  BITS_C_PATTERN, BITS_DENOTATION, BITS_PATTERN, BITS_SYMBOL,
  BOLD_COMMENT_SYMBOL, BOLD_PRAGMAT_SYMBOL, BOLD_TAG,
  BOOLEAN_PATTERN,
  BOOL_SYMBOL,
  BOUND, BOUNDS, BOUNDS_LIST,
  BRIEF_OUSE_PART, BRIEF_CONFORMITY_OUSE_PART, BRIEF_ELIF_PART, BRIEF_OPERATOR_DECLARATION,
  BUS_SYMBOL,
  BYTES_SYMBOL,
  BY_PART, BY_SYMBOL,
  CALL,
  CASE_CHOICE_CLAUSE, CASE_CLAUSE, CASE_IN_PART, CASE_OUSE_PART, CASE_PART, CASE_SYMBOL, CAST,
  CHANNEL_SYMBOL,
  CHAR_C_PATTERN, CHAR_SYMBOL,
  CHOICE, CHOICE_PATTERN,
  CLASS_SYMBOL,
  CLOSED_CLAUSE,
  CLOSE_SYMBOL,
  CODE_CLAUSE, CODE_LIST, CODE_SYMBOL,
  COLLATERAL_CLAUSE,
  COLLECTION,
  COLON_SYMBOL,
  COLUMN_FUNCTION, COLUMN_SYMBOL,
  COMMA_SYMBOL,
  COMPLEX_PATTERN, COMPLEX_SYMBOL, COMPL_SYMBOL,
  CONDITIONAL_CLAUSE,
  CONFORMITY_CHOICE, CONFORMITY_CLAUSE, CONFORMITY_IN_PART, CONFORMITY_OUSE_PART,
  CONSTRUCT,
  DECLARATION_LIST, DECLARER,
  DEFINING_IDENTIFIER, DEFINING_INDICANT, DEFINING_OPERATOR,
  DENOTATION,
  DEPROCEDURING,
  DEREFERENCING,
  DIAGONAL_FUNCTION, DIAGONAL_SYMBOL,
  DOTDOT_SYMBOL,
  DOWNTO_SYMBOL,
  DO_PART, DO_SYMBOL,
  DYNAMIC_REPLICATOR,
  EDOC_SYMBOL,
  ELIF_IF_PART, ELIF_PART, ELIF_SYMBOL,
  ELSE_BAR_SYMBOL, ELSE_OPEN_PART, ELSE_PART, ELSE_SYMBOL,
  EMPTY_SYMBOL,
  ENCLOSED_CLAUSE,
  END_SYMBOL,
  ENQUIRY_CLAUSE,
  ENVIRON_NAME, ENVIRON_SYMBOL,
  EQUALS_SYMBOL,
  ERROR,
  ERROR_IDENTIFIER,
  ESAC_SYMBOL,
  EXIT_SYMBOL,
  EXPONENT_FRAME,
  FALSE_SYMBOL,
  FIELD, FIELD_IDENTIFIER,
  FILE_SYMBOL,
  FIRM,
  FIXED_C_PATTERN,
  FI_SYMBOL,
  FLEX_SYMBOL,
  FLOAT_C_PATTERN,
  FORMAL_BOUNDS, FORMAL_BOUNDS_LIST,
  FORMAL_DECLARERS, FORMAL_DECLARERS_LIST, FORMAL_DECLARER_MARK,
  FORMAT_CLOSE_SYMBOL,
  FORMAT_DELIMITER_SYMBOL,
  FORMAT_IDENTIFIER,
  FORMAT_A_FRAME, FORMAT_D_FRAME, FORMAT_E_FRAME, FORMAT_I_FRAME,
  FORMAT_ITEM_A, FORMAT_ITEM_B, FORMAT_ITEM_C, FORMAT_ITEM_D,
  FORMAT_ITEM_E, FORMAT_ITEM_F, FORMAT_ITEM_G, FORMAT_ITEM_H,
  FORMAT_ITEM_I, FORMAT_ITEM_J, FORMAT_ITEM_K, FORMAT_ITEM_L,
  FORMAT_ITEM_M, FORMAT_ITEM_N, FORMAT_ITEM_O, FORMAT_ITEM_P,
  FORMAT_ITEM_Q, FORMAT_ITEM_R, FORMAT_ITEM_S, FORMAT_ITEM_T,
  FORMAT_ITEM_U, FORMAT_ITEM_V, FORMAT_ITEM_W, FORMAT_ITEM_X,
  FORMAT_ITEM_Y, FORMAT_ITEM_Z,
  FORMAT_ITEM_ESCAPE, FORMAT_ITEM_MINUS, FORMAT_ITEM_PLUS, FORMAT_ITEM_POINT,
  FORMAT_OPEN_SYMBOL, FORMAT_PATTERN, FORMAT_POINT_FRAME, FORMAT_SYMBOL, FORMAT_TEXT, FORMAT_Z_FRAME,
  FORMULA,
  FOR_PART, FOR_SYMBOL,
  FROM_PART, FROM_SYMBOL,
  GENERAL_C_PATTERN, GENERAL_PATTERN,
  GENERATOR,
  GENERIC_ARGUMENT, GENERIC_ARGUMENT_LIST,
  GOTO_SYMBOL, GO_SYMBOL,
  GUARDED_CONDITIONAL_CLAUSE, GUARDED_LOOP_CLAUSE,
  HEAP_SYMBOL,
  IDENTIFIER, IDENTITY_DECLARATION, IDENTITY_RELATION,
  IF_PART, IF_SYMBOL,
  INDICANT,
  INITIALISER_SERIES,
  INSERTION,
  INTEGRAL_C_PATTERN, INTEGRAL_MOULD, INTEGRAL_PATTERN,
  INT_DENOTATION, INT_SYMBOL,
  IN_SYMBOL,
  IN_TYPE_MODE,
  ISNT_SYMBOL, IS_SYMBOL,
  JUMP,
  KEYWORD,
  LABEL, LABELED_UNIT, LABEL_IDENTIFIER, LABEL_SEQUENCE,
  LITERAL,
  LOCAL_LABEL,
  LOC_SYMBOL,
  LONGETY, LONG_SYMBOL,
  LOOP_CLAUSE, LOOP_IDENTIFIER,
  MAIN_SYMBOL,
  MEEK,
  MODE_BITS, MODE_BOOL, MODE_BYTES, MODE_CHAR,
  MODE_COMPLEX, MODE_DECLARATION, MODE_FILE, MODE_FORMAT,
  MODE_INT, MODE_LONG_LONG_BITS, MODE_LONG_LONG_COMPLEX, MODE_LONG_LONG_INT,
  MODE_LONG_LONG_REAL, MODE_LONG_BITS, MODE_LONG_BYTES, MODE_LONG_COMPLEX,
  MODE_LONG_INT, MODE_LONG_REAL, MODE_NO_CHECK, MODE_PIPE,
  MODE_REAL, MODE_SOUND,
  MODE_SYMBOL,
  MONADIC_FORMULA, MONAD_SEQUENCE,
  NEW_SYMBOL,
  NIHIL, NIL_SYMBOL,
  NORMAL_IDENTIFIER,
  NO_SORT,
  OCCA_SYMBOL,
  OD_SYMBOL,
  OF_SYMBOL,
  OPEN_PART, OPEN_SYMBOL,
  OPERATOR, OPERATOR_DECLARATION, OPERATOR_PLAN, OP_SYMBOL,
  ORF_SYMBOL,
  OR_FUNCTION,
  OUSE_PART, OUSE_SYMBOL,
  OUT_PART, OUT_SYMBOL,
  OUT_TYPE_MODE,
  PARALLEL_CLAUSE,
  PARAMETER, PARAMETER_IDENTIFIER, PARAMETER_LIST, PARAMETER_PACK,
  PARTICULAR_PROGRAM,
  PAR_SYMBOL,
  PICTURE, PICTURE_LIST,
  PIPE_SYMBOL,
  POINT_SYMBOL,
  PRIMARY,
  PRIORITY, PRIORITY_DECLARATION, PRIO_SYMBOL,
  PROCEDURE_DECLARATION, PROCEDURE_VARIABLE_DECLARATION, PROCEDURING, PROC_SYMBOL,
  QUALIFIER,
  RADIX_FRAME,
  REAL_DENOTATION, REAL_PATTERN, REAL_SYMBOL, REF_SYMBOL,
  REPLICATOR,
  ROUTINE_TEXT, ROUTINE_UNIT,
  ROWING,
  ROWS_SYMBOL,
  ROW_CHAR_DENOTATION,
  ROW_FUNCTION,
  ROW_SYMBOL,
  SECONDARY,
  SELECTION, SELECTOR,
  SEMA_SYMBOL,
  SEMI_SYMBOL,
  SERIAL_CLAUSE,
  SERIES_MODE,
  SHORTETY, SHORT_SYMBOL,
  SIGN_MOULD,
  SKIP,
  SKIP_SYMBOL,
  SLICE,
  SOFT,
  SOME_CLAUSE,
  SOUND_SYMBOL,
  SPECIFICATION, SPECIFIED_UNIT, SPECIFIED_UNIT_LIST, SPECIFIED_UNIT_UNIT, SPECIFIER, SPECIFIER_IDENTIFIER,
  STANDARD,
  STATIC_REPLICATOR,
  STOWED_MODE,
  STRING_C_PATTERN, STRING_PATTERN, STRING_SYMBOL,
  STRONG,
  STRUCTURED_FIELD, STRUCTURED_FIELD_LIST, STRUCTURE_PACK, STRUCT_SYMBOL,
  STYLE_II_COMMENT_SYMBOL,
  STYLE_I_COMMENT_SYMBOL,
  STYLE_I_PRAGMAT_SYMBOL,
  SUB_SYMBOL,
  SUB_UNIT,
  TERTIARY,
  THEN_BAR_SYMBOL, THEN_PART, THEN_SYMBOL,
  TO_PART, TO_SYMBOL,
  TRANSPOSE_FUNCTION, TRANSPOSE_SYMBOL,
  TRIMMER,
  TRUE_SYMBOL,
  UNION_DECLARER_LIST, UNION_PACK, UNION_SYMBOL,
  UNIT,
  UNITING,
  UNIT_LIST, UNIT_SERIES,
  UNTIL_PART, UNTIL_SYMBOL,
  VARIABLE_DECLARATION,
  VIRTUAL_DECLARER_MARK,
  VOIDING, VOID_SYMBOL,
  WEAK,
  WHILE_PART, WHILE_SYMBOL,
  WIDENING,
  WILDCARD
};

enum
{
  INPUT_BUFFER = 0, OUTPUT_BUFFER, EDIT_BUFFER, UNFORMATTED_BUFFER,
  FORMATTED_BUFFER, DOMAIN_BUFFER, PATH_BUFFER, REQUEST_BUFFER,
  CONTENT_BUFFER, STRING_BUFFER, PATTERN_BUFFER, REPLACE_BUFFER,
  FIXED_TRANSPUT_BUFFERS
};

enum
{
  NO_DEFLEXING = 1, SAFE_DEFLEXING, ALIAS_DEFLEXING, FORCE_DEFLEXING,
  SKIP_DEFLEXING
};

#endif
