#include "../bullz.tab.h"
#include <stdio.h>

int 
yylex() 
{
    static int count = 0;
    if (count == 0) {
        yylval.val = 42;
        count++;
        printf("  [LEXER]    I FOUND issue number 42\n     sending Bullz...\n");
        return NUMBER;
    }
    return 0;
}

int 
main() 
{
    printf("  [SYS]    Starting Bullz test\n");
    yyparse();
    return 0;
}