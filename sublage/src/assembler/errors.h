#pragma once

#include "sublage/lexical.h"
#include "function.h"

typedef enum {
  ERROR_INTERNAL,
  ERROR_IO,
  ERROR_SYNTAX,
  ERROR_DECIMAL,
  ERROR_HEXADECIMAL,
  ERROR_EXPECTEDIDENTIFIER,
  ERROR_EXPECTEDSTARTFUNC,
  ERROR_IDENTIERTOOLONG,
  ERROR_MISSINGDOUBLEQUOTE,
  ERROR_STRINGEXPECTED,
  ERROR_BADESCAPESEQUENCE,
  ERROR_UNTERMINATEDSTRING,
  ERROR_DOEXPECTED,
  ERROR_WHILEEXPECTED,
  ERROR_CANTLOADIMPORT,
  ERROR_UNDEFINEDREFERENCE,
  ERROR_UNKNOWNRESERVEDWORD,
  ERROR_BREAKCONTINUEOUTSIDELOOP,
  ERROR_EXPECTEDREADORWRITE,
  ERROR_UNEXPECTEDEOFINCLASS,
  ERROR_UNKNOWIDENTIFIER,
  ERROR_UNKNOWNSUPERCLASS,
} ErrorCode;


/* Print an error message on stderr and exit */
void asError(LexicalContext *lc, ErrorCode ec, ...);
/* Print an error message on stderr and exit */
void asErrorLink(Function *function, ErrorCode ec, ...);
/* Print an error message on stderr and exit */
void asErrorLinkMultiple(char *name);

