#include <bullz.h>
#include <ctype.h>

BCZ_Token 
get_next_bcz_token(FILE *file) 
{
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (isspace(c)) continue;
        if (c == '%') {
            int next = fgetc(file);
            if (next == '%') return (BCZ_Token){BCZ_MARKER, "%%"};
            
            char buffer[64];
            int i = 0;
            buffer[i++] = '%';
            buffer[i++] = next;
            while (isalpha(c = fgetc(file)) && i < 63) buffer[i++] = c;
            ungetc(c, file);
            buffer[i] = '\0';

            if (strcmp(buffer, "%token") == 0) return (BCZ_Token){BCZ_TOKEN_DECL, "%token"};
        }

        if (c == ':') return (BCZ_Token){BCZ_COLON, ":"};
        if (c == ';') return (BCZ_Token){BCZ_SEMICOLON, ";"};
        if (c == '|') return (BCZ_Token){BCZ_PIPE, "|"};

        if (isalpha(c)) {
            char buffer[256];
            int i = 0;
            buffer[i++] = c;
            while (isalnum(c = fgetc(file)) || c == '_') buffer[i++] = c;
            ungetc(c, file);
            buffer[i] = '\0';
            return (BCZ_Token){BCZ_IDENTIFIER, strdup(buffer)};
        }

        if (c == '{') {
            char *block = malloc(4096);
            int i = 0, depth = 1;
            while (depth > 0 && (c = fgetc(file)) != EOF) {
                if (c == '{') depth++;
                if (c == '}') depth--;
                if (depth > 0) block[i++] = c;
            }
            block[i] = '\0';
            return (BCZ_Token){BCZ_C_BLOCK, block};
        }
    }
    return (BCZ_Token){BCZ_EOF, NULL};
}