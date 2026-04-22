
#ifndef BULLZ_H
#define BULLZ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    BCZ_TOKEN_DECL, BCZ_UNION_DECL, BCZ_MARKER, 
    BCZ_IDENTIFIER, BCZ_C_BLOCK, BCZ_COLON, 
    BCZ_SEMICOLON, BCZ_PIPE, BCZ_EOF
} BCZ_TokenType;

typedef struct {
    BCZ_TokenType type;
    char *text;
} BCZ_Token;

typedef struct {
    char *name;
    int id;
} UserSymbol;

// prototypes
BCZ_Token get_next_bcz_token(FILE *file);

#endif