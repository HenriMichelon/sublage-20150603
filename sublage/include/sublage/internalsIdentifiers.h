/*
 Virtual machine internal functions identifiers
 */
#pragma once

#include "sublage/internals.h"

typedef struct {
    Internal internal;
    const char* mnemonic;
} InternalIdentifier;

static InternalIdentifier internalsIdentifiers[] = {
    { INTERNAL_NOP, "nop"},
    { INTERNAL_ADD, "+"},
    { INTERNAL_SUB, "-"},
    { INTERNAL_MUL, "*"},
    { INTERNAL_DIV, "/"},
    { INTERNAL_SWAP, "swap"},
    { INTERNAL_DROP, "drop"},
    { INTERNAL_DUP, "dup"},
    { INTERNAL_EQ, "="},
    { INTERNAL_NEQ, "!="},
    { INTERNAL_NOT, "not"},
    { INTERNAL_AND, "and"},
    { INTERNAL_OR, "or"},
    { INTERNAL_GT, ">"},
    { INTERNAL_LT, "<"},
    { INTERNAL_GE, ">="},
    { INTERNAL_LE, "<="},
    { INTERNAL_EXEC, "exec"},
    { INTERNAL_DUPN, "dupn"},
    { INTERNAL_OVER, "over"},
    { INTERNAL_CSTR, "->string"},
    { INTERNAL_CINT, "->int"},
    { INTERNAL_CFLOAT, "->float"},
    { INTERNAL_MODULO, "%" },
    { INTERNAL_DEPTH, "depth" },
    { INTERNAL_ROLL, "roll" },
    { INTERNAL_ROLLD, "rolld" },
    { INTERNAL_ROLL3, "roll3" },
    { INTERNAL_ROLLD3, "rolld3" },
    { INTERNAL_PICK, "pick" },
    { INTERNAL_PICK3, "pick3" },
    { INTERNAL_DUP2, "dup2" },
    { INTERNAL_DUP3, "dup3" },
    { INTERNAL_DROPN, "dropn" },
    { INTERNAL_DROP2, "drop2" },
    { INTERNAL_DROP3, "drop3" },
    { INTERNAL_CLEAR, "clear" },
    { INTERNAL_UNPICK, "unpick" },
    { INTERNAL_CARRAY, "->[]" },
    { INTERNAL_PICK2, "pick2" },
    { INTERNAL_ISNULL, "isnull" },
    { INTERNAL_ISNOTNULL, "isnotnull" },
    { INTERNAL_EXPLODE, "explode" },
    { INTERNAL_NEW, "new" },
    { INTERNAL_SELF, "self" },
    { INTERNAL_SUPER, "super" },
    { INTERNAL_STOPFLAG, NULL},
};

