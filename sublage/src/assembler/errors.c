#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "sublage/mem.h"
#include "sublage/loader.h"
#include "errors.h"

const char* asErrors[] = { 
		"internal error",
		"error while reading the source file", 
		"syntax error",
		"incorrect decimal number", 
		"incorrect hexadecimal number",
		"expected identifier", 
		"missing '<<'",
		"identifier too log, maximum %d characters allowed", 
		"missing '\"'",
		"expected a string constant", 
		"incorrect espace sequence in string",
		"unterminated string", 
		"`do` expected",
		"`while` expected",
		"error reading `%s` for import", 
		"undefined reference to `%s`",
		"internal error, reserved word `%s` not implemented",
        "`%s` used outside a loop",
        "expected `read `or `write` keyword",
        "unexpected end of file in class definition",
        "`%s` not a function or variable name, can't link binary",
        "super class `%s` not found, can't link binary"
};

void asError(LexicalContext *lc, ErrorCode err, ...) {
	char msg[640];
	va_list ap;
	va_start(ap, err);
	vsnprintf(msg, 640, asErrors[err], ap);
	va_end(ap);
	if (err == ERROR_CANTLOADIMPORT) {
		fprintf(stderr, "error on line %ld: %s (was: `%s`)\n",
				(long)lexicalCurrentLine(lc), msg, loaderGetErrorMessage());
	} else {
		fprintf(stderr, "error on line %ld: %s\n",
				(long)lexicalCurrentLine(lc), msg);
	}
	exit(1);
}


void asErrorLinkMultiple(char *name) {
        fprintf(stderr, 
                "multiple definition of function `%s`, please use library name prefixing.",
                name);
	exit(1);
}


void asErrorLink(Function *function, ErrorCode err, ...) {
	char msg[640];
	va_list ap;
	va_start(ap, err);
	vsnprintf(msg, 640, asErrors[err], ap);
	va_end(ap);
	fprintf(stderr, "in function '%s': %s\n", functionGetName(function), msg);
	//exit(1);
}
