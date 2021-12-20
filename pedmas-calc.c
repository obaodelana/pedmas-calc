#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#define TOKEN_SIZE 100

typedef struct Operation
{

} Operation;

char* get_expression(const char **, int);
char **tokenise_expression(char *, int *);
double compute_expression(char *);

int main(int argc, const char *argv[])
{
    char *equationStr = get_expression(argv, argc);
    do
    {
        if (equationStr != NULL && strlen(equationStr) > 0)
        {
            
        }
        free(equationStr);
        equationStr = get_expression(NULL, 0);
    } while (equationStr[0] != 'q' && equationStr[0] != 'e' && equationStr[0] != 'x');

    if (equationStr != NULL)
        free(equationStr);
    
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

void remove_spaces(char *str)
{
    char *duplicate = str;
    do
    {
        // Move to the next non-space character
        while (isspace(*duplicate))
            duplicate++;
    // Set str[i] (may be a space) to duplicate[i] (which is a non-space char)
    // Until at NULL character which is zero
    } while((*str++ = *duplicate++));
}

