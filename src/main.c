/*
*           main.c   :       This is the main file
*                            here are the main functions.
*   
*
* --------------------------------------------------------
*       @Copyright 2026 - APACHE-2.0 
*
*/

#include <bullz.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



typedef struct {
    int lhs;           
    int rhs[10];       
    int rhs_len;
    int id;
} Rule;

typedef struct {
    int rule_id;
    int dot; 
} Item;

typedef struct {
    int id;
    Item items[20];
    int item_count;
} State;


Rule grammar[100];
int rule_count = 0;
UserSymbol symbols[100];
int sym_count = 0;
State states[50];
int state_count = 0;
int action_table[50][300]; // [Status][TokenID]

void 
add_symbol(char *name) 
{
    symbols[sym_count].name = strdup(name);
    symbols[sym_count].id = 258 + sym_count;
    sym_count++;
}


void 
compute_closure(State *s) 
{
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < s->item_count; i++) {
            Item *it = &s->items[i];
            Rule *r = &grammar[it->rule_id];

           // If the period is not at the end of the rule
            if (it->dot < r->rhs_len) {
                int next_sym = r->rhs[it->dot];

                // If next_sym is a non-terminal (e.g., 'exp')
                // We search for all rules that begin with that non-terminal
                for (int g = 0; g < rule_count; g++) {
                    if (grammar[g].lhs == next_sym) {
                        int found = 0;
                        for (int k = 0; k < s->item_count; k++) {
                            if (s->items[k].rule_id == g && s->items[k].dot == 0) {
                                found = 1;
                                break;
                            }
                        }
                        // If it's not there, we add it and mark that something changed
                        if (!found && s->item_count < 20) {
                            s->items[s->item_count++] = (Item){g, 0};
                            changed = 1;
                        }
                    }
                }
            }
        }
    }
}

void
build_action_table() 
{
    // init table
    for(int i=0; i<50; i++) for(int j=0; j<300; j++) action_table[i][j] = 0;

    // State 0: If you receive NUMBER (258), SHIFT to state 1
    action_table[0][258] = 1; 
    // State 1: When the number ends, REDUCE rule 0 (negative value -1)
    action_table[1][0] = -1; 
}

char* 
patch_action_code(char* code) 
{
    char* patched = malloc(4096);
    char* src = code;
    char* dest = patched;
    while (*src) {
        if (*src == '$' && *(src + 1) == '$') {
            strcpy(dest, "yyval.val");
            dest += 9; src += 2;
        } else if (*src == '$' && isdigit(*(src+1))) {
            int index = *(src+1) - '0';
            sprintf(dest, "value_stack[top - %d].val", index - 1);
            dest = patched + strlen(patched);
            src += 2;
        } else { *dest++ = *src++; }
    }
    *dest = '\0';
    return patched;
}


void
write_header(FILE *out, FILE *header) 
{
    fprintf(header, "#ifndef _BULLZ_TAB_H\n#define _BULLZ_TAB_H\n\n");
    fprintf(header, "typedef union YYSTYPE {\n  int val;\n} YYSTYPE;\n\n");
    fprintf(header, "extern YYSTYPE yylval;\nextern YYSTYPE yyval;\nint yyparse();\n\n");
    for(int i=0; i<sym_count; i++) fprintf(header, "#define %s %d\n", symbols[i].name, symbols[i].id);
    fprintf(header, "#endif\n");

    fprintf(out, "#include \"bullz.tab.h\"\n#include <stdio.h>\n#include <stdlib.h>\n\n");
    fprintf(out, "YYSTYPE yylval;\nYYSTYPE yyval;\nint yylex();\n");
    fprintf(out, "void yyerror(const char *s) { fprintf(stderr, \"ERR: %%s\\n\", s); }\n\n");
}

void
 write_output_report() 
 {
    FILE *f = fopen("bullz.output", "w");
    fprintf(f, "BULLZ Output generated.\n=====================\n\n");
    fprintf(f, "State 0:\n  Press NUMBER shift, and jump to state 1\n");
    fprintf(f, "State 1:\n  $default  reduce using the zero rule\n");
    fclose(f);
}

int 
main(int argc, char **argv)
{
    if (argc < 2) return 1;


    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen("bullz.tab.c", "w");
    FILE *head = fopen("bullz.tab.h", "w");

    BCZ_Token t;
    int section = 0;

    while ((t = get_next_bcz_token(in)).type != BCZ_EOF) {
        if (t.type == BCZ_MARKER) {
            if (section == 0) write_header(out, head);
            section++; continue;
        }
        if (section == 0 && t.type == BCZ_TOKEN_DECL) {
            BCZ_Token name = get_next_bcz_token(in);
            add_symbol(name.text);
        }
        if (section == 1 && t.type == BCZ_IDENTIFIER) {
            grammar[rule_count].id = rule_count;
            grammar[rule_count].rhs_len = 1;
            grammar[rule_count].rhs[0] = 258;
        }
        if (section == 1 && t.type == BCZ_C_BLOCK) {
            char* code = patch_action_code(t.text);
            fprintf(out, "void do_action_%d(YYSTYPE *value_stack, int top) {\n %s \n}\n", rule_count, code);
            rule_count++;
        }
    }

    build_action_table();

    // Write the Action Table in the .c file
    fprintf(out, "\nint yy_action[2][300] = {\n");
    fprintf(out, "  [0] = { [258] = 1 },\n");
    fprintf(out, "  [1] = { [0] = -1 }\n};\n\n");

    // LALR Execution Engine
    fprintf(out, "int yyparse() {\n");
    fprintf(out, "    int state_stack[1024]; YYSTYPE value_stack[1024]; int top = 0;\n");
    fprintf(out, "    state_stack[0] = 0; int token = yylex();\n\n");
    fprintf(out, "    while(1) {\n");
    fprintf(out, "        int s = state_stack[top];\n");
    fprintf(out, "        int act = yy_action[s][token];\n\n");
    fprintf(out, "        if (act > 0) { // SHIFT\n");
    fprintf(out, "            value_stack[++top] = yylval;\n");
    fprintf(out, "            state_stack[top] = act;\n");
    fprintf(out, "            token = yylex();\n");
    fprintf(out, "        } else if (act < 0) {\n");
    fprintf(out, "            do_action_0(value_stack, top);\n");
    fprintf(out, "            printf(\"  BULLZ    Successful Reduction. $$ = %%d\\n\", yyval.val);\n");
    fprintf(out, "            return 0;\n");
    fprintf(out, "        } else {\n");
    fprintf(out, "            yyerror(\"ERROR\"); return 1;\n");
    fprintf(out, "        }\n    }\n}\n");

    write_output_report();
    fclose(in); fclose(out); fclose(head);
    printf("  Bullz     generated correctly.\n");
    return 0;
}