#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#define TOKEN_SIZE 100

const char signs[] = "+-*/^";

typedef struct Operation
{
    char operator;
    double op1, op2;
    struct Operation *nestedOp1, *nestedOp2;
} Operation;

char* get_expression(const char **, int);
char** tokenise_expression(const char *, int *);
Operation* parse_tokens(char **, int);
void print_parse_tree(const Operation *, int);
void free_parse_tree(Operation *);
void free_tokens(char **, int);

int main(int argc, const char *argv[])
{
    char *expressionStr = get_expression(argv, argc);
    do
    {
        if (expressionStr != NULL && strlen(expressionStr) > 0)
        {
            int tokenCount = 0;
            char **tokens = tokenise_expression(expressionStr, &tokenCount);
            if (tokens != NULL)
            {
                Operation *parseTree = parse_tokens(tokens, tokenCount);

                // Compute

                free_parse_tree(parseTree);
                free_tokens(tokens, tokenCount);
            }
            free(expressionStr);
        }
        expressionStr = get_expression(NULL, 0);
    } while (strchr("qex", expressionStr[0]) == NULL);

    if (expressionStr != NULL)
        free(expressionStr);
    
    return EXIT_SUCCESS;
}

char* get_expression(const char **args, int argCount)
{
    // Memory allocation size for the string
    size_t outSize = TOKEN_SIZE;
    char *expression = calloc(outSize, sizeof(char));

    // If args supplied
    if (argCount > 1 && args != NULL)
    {
        // Skip first arg--which is the file name
        for (int i = 1; i < argCount; i++)
        {
            // Add each word in args to expression string
            strcat(expression, args[i]);
            // Add space after concatenation, except on last word
            if (i != argCount - 1)
                strcat(expression, " ");
        }
    }

    // If no arg, get input from user
    else
    {
        printf("Type expression: "), fflush(stdout);
        getline(&expression, &outSize, stdin);
        // Remove new line character at the end of the string
        expression[strlen(expression) - 1] = '\0';
    }

    return expression;
}

bool contains_chars(const char *str, const char *chars)
{
    for (int i = 0; chars[i] != '\0'; i++)
    {
        if (strchr(str, chars[i]) != NULL)
            return true;
    }
    return false;
}

void remove_spaces(char *str)
{
    if (!contains_chars(str, " \t\r")) return;

    char *duplicate = str;
    do
    {
        // Iterate until a non-space character is found
        while (isspace(*duplicate))
            duplicate++; // Go to next address / character
    } while((*str++ = *duplicate++)); // Set str[i] to non-space duplicate[i] until null character
}

char* remove_parenthesis_part(const char *str)
{
    char *newStr = calloc(strlen(str) + 1, sizeof(char));
    if (newStr == NULL) return NULL;

    if (!contains_chars(str, "()"))
        strcpy(newStr, str);
    else
    {
        int i = 0, len = strlen(str);
        do
        {
            if (str[i] == '(')
            {
                while (str[i++] != ')')
                    if (str[i] == '\0') break;
            }
            strncat(newStr, &str[i++], 1);
        } while (i < len);
    }

    return newStr;
}

char** tokenise_expression(const char *exp, int *length)
{
    char *str = calloc(strlen(exp) + 1, sizeof(char));
    strcpy(str, exp);
    remove_spaces(str);

    char **tokens = calloc(1, sizeof(char*));
    tokens[0] = calloc(TOKEN_SIZE + 1, sizeof(char));
    *length = 1;
    
    int currentToken = 0, parenthesisOpen = 0;
    char c, lastC;
    char sameLineSigns[10], newLineSigns[10];

    char *withoutParenthesis = remove_parenthesis_part(str);
    if (contains_chars(withoutParenthesis, "+-"))
    {
        strcpy(sameLineSigns, "*/^()");
        strcpy(newLineSigns, "+-");
    }

    else if (contains_chars(withoutParenthesis, "*/"))
    {
        strcpy(sameLineSigns, "^()");
        strcpy(newLineSigns, "*/");
    }

    else if (contains_chars(withoutParenthesis, "^"))
    {
        strcpy(sameLineSigns, "()");
        strcpy(newLineSigns, "^");
    }
    
    else
    {
        strcpy(sameLineSigns, "*/^()");
        strcpy(newLineSigns, "+-");
    }
    free(withoutParenthesis);

    for (int i = 0, len = strlen(exp); i < len; i++)
    {
        if (strlen(tokens[currentToken]) >= 100)
            tokens[currentToken] = realloc(tokens[currentToken], strlen(tokens[currentToken]) * 2 + 1);

        c = str[i];

        if ((i == 0 && !isdigit(c) && c != '(')
            || (strchr("*/^+-", c) != NULL
                && (strchr("*/^+-(", lastC) != NULL || str[i + 1] == '\0')))
        {
            printf("Invalid expression\n");
            free_tokens(tokens, *length);
            *length = 0;
            return NULL;
        }

        if (isdigit(c)
            || strchr(sameLineSigns, c) != NULL
            || parenthesisOpen > 0)
        {
            if (c == '(')
            {
                if (lastC == ')')
                {
                    printf("No operator between parentheses\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    return NULL;                    
                }

                parenthesisOpen++;
            }
            
            else if (c == ')')
            {
                if (parenthesisOpen <= 0)
                {
                    printf("Matching parenthesis not found\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    return NULL;
                }

                else if (lastC == '(')
                {
                    printf("Nothing inside parentheses\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    return NULL;
                }

                else if (strchr("*+/^-", lastC) != NULL)
                {
                    printf("Invalid expression\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    return NULL;
                }

                parenthesisOpen--;
            }

            strncat(tokens[currentToken], &c, 1);
        }

        else if (strchr(newLineSigns, c) != NULL)
        {
            if (str[i + 1] == '\0')
            {
                printf("Incomplete expression\n");
                free_tokens(tokens, *length);
                *length = 0;
                return NULL;
            }

            // Allocate memory for 2 new rows
            tokens = realloc(tokens, sizeof(char*) * (++currentToken + 2));
            
            // First row
            // Allocate memory for one character
            tokens[currentToken] = calloc(1, sizeof(char));
            strncpy(tokens[currentToken], &c, 1);

            // Second row
            // Allocate enough memory
            tokens[++currentToken] = calloc(TOKEN_SIZE + 1, sizeof(char));

            *length += 2;
        }

        else
        {
            printf("Invalid character detected: '%c'\n", c);
            free_tokens(tokens, *length);
            *length = 0;
            return NULL;
        }

        lastC = c;
    }

    if (parenthesisOpen > 0)
    {
        printf("Matching parenthesis not found\n");
        free_tokens(tokens, *length);
        *length = 0;
        return NULL;
    }

    free(str);

    #ifdef DEBUG_MODE
        printf("Tokens:\n-------------------\n");
        for (int i = 0; i < tokenCount; i++)
            printf("%s\n", tokens[i]);
        printf("-------------------\n")
    #endif

    return tokens;
}

void free_tokens(char **tokens, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (tokens[i] != NULL)
            free(tokens[i]);
    }
    free(tokens);
}

Operation* parse_tokens(char **tokens, int length)
{
    Operation *head = calloc(1, sizeof(Operation));
    Operation *pointer = head;

    for (int i = 0; i < length; i++)
    {
        char *token = tokens[i];
        bool operandLine = (i % 2 != 0);
        bool nextIsSign = (i + 1 < length && strchr(signs, tokens[i + 1][0]) != NULL);
        
        // Digit or expression
        if (!operandLine)
        {
            if (contains_chars(token, signs) || (contains_chars(token, signs) && contains_chars(token, "()")))
            {
                char tokenExpr[strlen(token) + 1];
                if (token[0] == '(')
                {
                    char *lastBracket = strrchr(token, ')');
                    int exprCount = lastBracket - (token + 1);
                    strncpy(tokenExpr, token + 1, exprCount);
                    tokenExpr[exprCount] = '\0';
                }
                else
                    strcpy(tokenExpr, token);

                int tokenCount = 0;
                char **nestedTokens = tokenise_expression(tokenExpr, &tokenCount);

                Operation *op = parse_tokens(nestedTokens, tokenCount);
                free_tokens(nestedTokens, tokenCount);

                if (pointer->operator == 0)
                    pointer->nestedOp1 = op;
                else
                {
                    if (nextIsSign)
                    {
                        Operation *newOp = calloc(1, sizeof(Operation));
                        newOp->nestedOp1 = op;

                        pointer->nestedOp2 = newOp;
                        pointer = newOp;
                    }
                    else
                        pointer->nestedOp2 = op;
                }
            }

            else
            {
                double num = strtod((token[0] == '(' ? (token + 1) : token), NULL);
                if (pointer->operator == 0)
                    pointer->op1 = num;
                else
                {
                    if (nextIsSign)
                    {
                        Operation *newOp = calloc(1, sizeof(Operation));
                        newOp->op1 = num;

                        pointer->nestedOp2 = newOp;
                        pointer = newOp;
                    }
                    else
                        pointer->op2 = num;
                }
            }
        }

        else
            pointer->operator = token[0];
    }

    #ifdef DEBUG_MODE
        printf("Parse tree\n--------------------------------------\n");
        print_parse_tree(parseTree, 0);
        printf("--------------------------------------\n");
    #endif

    return head;
}

void print_tabs(int n)
{
    for (int i = 0; i < n; i++)
        printf("    ");
}

void print_parse_tree(const Operation *head, int currentLevel)
{
    print_tabs(currentLevel);
    printf("op1: "), fflush(stdout);
    if (head->nestedOp1 == NULL)
        printf("%f\n", head->op1);
    else
    {
        print_tabs(currentLevel);
        printf("\n");
        print_parse_tree(head->nestedOp1, currentLevel + 1);
    }

    print_tabs(currentLevel);
    printf("operator: %c\n", head->operator);

    print_tabs(currentLevel);
    printf("op2: "), fflush(stdout);
    if (head->nestedOp2 == NULL)
        printf("%f\n", head->op2);
    else
    {
        print_tabs(currentLevel);
        printf("\n");
        print_parse_tree(head->nestedOp2, currentLevel + 1);
    }
}

void free_parse_tree(Operation *head)
{
    if (head->nestedOp1 != NULL)
        free_parse_tree(head->nestedOp1);
    if (head->nestedOp2 != NULL)
        free_parse_tree(head->nestedOp2);

    free(head);
}
