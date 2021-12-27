#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#define TOKEN_SIZE 100

const char allowedOperators[] = "+-*/^";

typedef struct Operation
{
    char ope;
    double op1, op2;
    struct Operation *nestedOperation;
} Operation;

char* get_expression(const char **, int);
char** tokenise_expression(const char *, int *);
double compute_expression(char *);

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
                #ifdef DEBUG_MODE
                    for (int i = 0; i < tokenCount; i++)
                    {
                        printf("%s\n", tokens[i]);
                    }
                #endif

                free(tokens);
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
    char *newStr = calloc(strlen(str), sizeof(char));

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

void free_tokens(char **tokens, int length, const char *message)
{
    printf("%s\n", message);
    for (int i = 0; i < length; i++)
    {
        if (tokens[i] != NULL)
            free(tokens[i]);
    }
    free(tokens);
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
    free(withoutParenthesis);

    for (int i = 0, len = strlen(exp); i < len; i++)
    {
        if (strlen(tokens[currentToken]) >= 100)
            tokens[currentToken] = realloc(tokens[currentToken], strlen(tokens[currentToken]) * 2 + 1);

        c = str[i];

        if ((i == 0 && !isdigit(c))
            || (strchr("*/^+-", c) != NULL
                && (strchr("*/^+-(", lastC) != NULL || str[i + 1] == '\0')))
        {
            free_tokens(tokens, *length, "Invalid expression");
            *length = 0;
            return NULL;
        }

        if (isdigit(c)
            || strchr(sameLineSigns, c) != NULL
            || parenthesisOpen > 0)
        {
            if (c == '(') parenthesisOpen++;
            else if (c == ')')
            {
                if (parenthesisOpen <= 0)
                {
                    free_tokens(tokens, *length, "Matching parenthesis not found");
                    *length = 0;
                    return NULL;
                }

                else if (lastC == '(')
                {
                    free_tokens(tokens, *length, "Nothing inside parentheses");
                    *length = 0;
                    return NULL;
                }

                else if (strchr("*+/^-", lastC) != NULL)
                {
                    free_tokens(tokens, *length, "Invalid expression");
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
                free_tokens(tokens, *length, "Incomplete expression");
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

        lastC = c;
    }

    if (parenthesisOpen > 0)
    {
        free_tokens(tokens, *length, "Matching parenthesis not found");
        *length = 0;
        return NULL;
    }

    return tokens;
}
