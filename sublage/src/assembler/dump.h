#pragma once

#include <stdio.h>
#include "sublage/lexical.h"

void dumpLexicalToken(FILE *f, LexicalToken *lt);
void dumpLexicalContext(FILE *f, LexicalContext *lc);
