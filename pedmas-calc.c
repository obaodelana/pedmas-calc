// Made by Obaloluwa Odelana (March 6 2022)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#define TOKEN_SIZE 20
#define ptr_check(ptr) if ((ptr) == NULL)\
                        {\
                            printf("Memory allocation failed\nExiting...\n");\
                            exit(EXIT_FAILURE);\
                        } 

const char signs[] = "+-*/^";

char* get_expression(void);
char** tokenise_expression(const char *, int *);
double compute_tokens(char **, int);
void free_tokens(char **, int);

int main(int argc, const char *argv[])
{
    // Get expression string
    char *expressionStr = get_expression();
    // Loop until 'q/Q','x/X' is detected as the first character
    while (strchr("qQxX", expressionStr[0]) == NULL)
    {
        // Check if previous function call is successful
        if (expressionStr != NULL && strlen(expressionStr) > 0)
        {
            int tokenCount = 0;
            char **tokens = tokenise_expression(expressionStr, &tokenCount);
            free(expressionStr);
            if (tokens != NULL)
            {
                double result = 0;
                if (tokenCount > 1)
                    result = compute_tokens(tokens, tokenCount);
                // If [tokenCount] is 1 that means only a number was typed and there's no need for parsing
                else
                    // Skip parenthesis if found at beginning
                    result = strtod((tokens[0][0] == '(') ? &tokens[0][1] : tokens[0], NULL);

                if (result == (int) result)
                    printf("= %i\n", (int) result);
                else
                    printf("= %f\n", result);

                free_tokens(tokens, tokenCount);
            }
        }

        expressionStr = get_expression();
        ptr_check(expressionStr);
    }

    if (expressionStr != NULL)
        free(expressionStr);
    
    return EXIT_SUCCESS;
}

char* get_expression(void)
{
    // Allocate enough memory for a character, new line and null character because getline will realloc 
    size_t outSize = 3;
    char *expression = calloc(outSize, sizeof(char));
    ptr_check(expression);

    printf("Type expression: "), fflush(stdout);
    getline(&expression, &outSize, stdin);
    ptr_check(expression);
    // Remove new line character at the end of the string
    expression[strlen(expression) - 1] = '\0';

    return expression;
}

// Util function to check if a string (str) contains any one of the characters in (chars)
bool contains_chars(const char *str, const char *chars)
{
    for (int i = 0; chars[i] != '\0'; i++)
    {
        // If string contains current char
        if (strchr(str, chars[i]) != NULL)
            return true;
    }
    // Return false after going through all chars
    return false;
}

// Util function to remove any form of whitespace:
// space (' '), tab (\v, \t), new line (\n), form feed (\f) or carriage return (\r) 
void remove_spaces(char *str)
{
    // If str is null or doesn't contain whitespace, exit
    if (str == NULL || !contains_chars(str, " \t\r\v\f\n")) return;

    char *duplicate = str;
    do
    {
        // Iterate until a non-space character is found
        while (isspace(*duplicate))
            duplicate++; // Go to next address / character
    } while((*str++ = *duplicate++) != '\0'); // Set str[i] to non-space duplicate[i] until null character (0)
}

// Util function to remove all characters in between a parenthesis '()'
char* remove_parenthesis_part(char *str)
{
    // If str doesn't contain a parenthesis return original string 
    if (!contains_chars(str, "()"))
        return str;
    else
    {
        // Allocate memory the same size as [str] (+1 to accommodate null character) 
        char *newStr = calloc(strlen(str) + 1, sizeof(char));
        ptr_check(newStr);
        int i = 0, openParentheses = 0, len = strlen(str);
        do
        {
            // Check for start of parenthesis
            if (str[i] == '(')
            {
                openParentheses++;
                // Loop till all parentheses are closed
                while (openParentheses > 0)
                {
                    // If end of string reached exit loop
                    if (str[i++] == '\0')
                    {
                        openParentheses = 0;
                        break;
                    }

                    if (str[i] == '(') openParentheses++;
                    if (str[i] == ')') openParentheses--;
                }
                // Make sure not to write a closing parenthesis
                if (str[i] == ')') i++;
                // At this point [i] is incremented past the parenthesis portion
            }
            // Add to new string unparenthesised portion of str
            strncat(newStr, &str[i++], 1);
        } while (i < len); // Loop till end of string
        
        return newStr;
    }
}

// Util function to insert a character at a specified index
void insert_char(char *str, char newC, int index)
{
    int len = strlen(str);
    // If index is greater than string length
    if (index > len - 1)
    {
        printf("Internal error: insert_char(): index is out of bounds");
        return;
    }

    // Allocate more memory for new character
    str = realloc(str, sizeof(char) * (len + 1 /*new char*/ + 1 /*null character*/));

    char prev = str[index];
    str[index] = newC; // Change character at insertion index
    // Move each character after insertion index forward including null character
    for (int i = index + 1; i <= len + 1; i++)
    {
        // Store character that's about to be changed
        char current = str[i];
        // Change current character to previous character
        str[i] = prev;
        // Update previous character
        prev = current;
    }
}

// i.e. 6(20) => 6*(20), (38)(2) => (38)*(2)
void add_multiplication_sign_btw_parenthesis(char *str)
{
    char *parenthesisPos = str;
    int i = 0;
    // While an opening parenthesis can be found
    while((parenthesisPos = strchr(parenthesisPos, '(')) != NULL)
    {
        i = parenthesisPos - str;
        if (i != 0)
        {
            //  6(20) or 2(3)^2 || (30)(2) or (3+4)(3+5)
            if (isdigit(str[i - 1]) || str[i - 1] == ')')
                insert_char(str, '*', i);
        }
        // Move to next character
        parenthesisPos++;
    }

    #ifdef DEBUG_MODE
        printf("New string: %s\n", str);
    #endif
}

// Returns an array of strings (tokens)
// e.g. 3*2+2 =>
    // 3*2 (recursively called) =>
        // 3
        // *
        // 2
    // +
    // 2 
char** tokenise_expression(const char *exp, int *length)
{
    // Create modifiable duplicate of [exp]
    char *str = calloc(strlen(exp) + 1, sizeof(char));
    ptr_check(str);
    strcpy(str, exp);
    remove_spaces(str), add_multiplication_sign_btw_parenthesis(str);

    // Array of strings
    char **tokens = calloc(1, sizeof(char*));
    ptr_check(tokens);
    // First string
    tokens[0] = calloc(TOKEN_SIZE + 1, sizeof(char));
    ptr_check(tokens[0]);

    *length = 1; // Keeps track of number of strings in [tokens]
    
    int currentTokenIndex = 0, currentTokenCapacity = TOKEN_SIZE, openParentheses = 0;
    char c = 0, lastC = 0;
    // Determines which signs to treat as demarcations (check example at the top of function)
    char sameLineSigns[10], newLineSigns[10];

    checkSigns:
        ; // Satisfy compiler
        // Remove parenthesis portion to not get confused
        // e.g in "3*(3+2)" function should not see the '+' because it's inside parenthesis
        char *withoutParenthesis = remove_parenthesis_part(str);
        if (contains_chars(withoutParenthesis, "+-"))
        {
            // Treat numbers with "*/^()" as single quantities rather than operations
            // Break up numbers with "+-" into separate tokens
            strcpy(sameLineSigns, "*/^()"), strcpy(newLineSigns, "+-");
        }

        else if (contains_chars(withoutParenthesis, "*/"))
        {
            // Treat numbers with "^()" as single quantities rather than operations
            // Break up numbers with "*/" into separate tokens
            strcpy(sameLineSigns, "^()"), strcpy(newLineSigns, "*/");
        }

        else if (contains_chars(withoutParenthesis, "^"))
        {
            // Treat numbers with "()" as single quantities rather than operations
            // Break up numbers with "^" into separate tokens
            strcpy(sameLineSigns, "()"), strcpy(newLineSigns, "^");
        }
        
        // If no signs seen
        else
        {
            // Whole expression is wrapped inside a parenthesis
            if (contains_chars(str, "()"))
            {
                // Copy string without starting parenthesis
                strcpy(withoutParenthesis, str + 1);
                // Remove closing parenthesis
                if (withoutParenthesis[strlen(str) - 2] == ')')
                    withoutParenthesis[strlen(str) - 2] = '\0';
                // Clear [str]
                memset(str, 0, strlen(str));
                // Copy contents of [withoutParenthesis] into [str]
                strcpy(str, withoutParenthesis);
                free(withoutParenthesis);
                goto checkSigns; // Go back to the top (loops until all unnecessary parentheses are removed)
            }
            // Else a number or invalid expression
        }

    #ifdef DEBUG_MODE
        printf("Without parenthesis: %s\n", withoutParenthesis);
        printf("Same line signs: %s, New line signs: %s\n", sameLineSigns, newLineSigns);
    #endif
    // Free only if [withoutParenthesis] is not the same as [str]
    if (withoutParenthesis != str)
        free(withoutParenthesis);

    for (int i = 0, len = strlen(str); i < len; i++)
    {
        // If current token's length is almost greater than it's capacity, allocate more memory
        if (strlen(tokens[currentTokenIndex]) >= currentTokenCapacity)
        {
            #ifdef DEBUG_MODE
                printf("Resized token #%i\n", currentTokenIndex);
            #endif
            tokens[currentTokenIndex] = realloc(tokens[currentTokenIndex], (currentTokenCapacity *= 2) + 1);
            ptr_check(tokens[currentTokenIndex]);
        }

        c = str[i]; // Current character

        // If
            // The first character is not a digit or parenthesis opening
            // Current and last characters are signs
            // Current character is a sign and the last character in the string
            // Decimal sign detected without a number before it
        if ((i == 0 && !isdigit(c) && c != '(')
            || (strchr("*/^+-.", c) != NULL
                && (strchr("*/^+-(.", lastC) != NULL || str[i + 1] == '\0'))
            || (c == '.' && !isdigit(lastC)))
        {
            printf("Invalid expression\n");
            free_tokens(tokens, *length);
            *length = 0;
            free(str);
            return NULL;
        }

        if (isdigit(c)
            || c == '.'
            || strchr(sameLineSigns, c) != NULL // Current character is one of [sameLineSigns]
            || openParentheses > 0)
        {
            if (c == '(')
                openParentheses++;
            else if (c == ')')
            {
                // That is no '(' found before current character
                if (openParentheses <= 0)
                {
                    printf("Matching parenthesis not found\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    free(str);
                    return NULL;
                }

                // i.e. "()"
                else if (lastC == '(')
                {
                    printf("Nothing inside parentheses\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    free(str);
                    return NULL;
                }
                
                // Check if a sign is before the closing i.e. "(2*)"
                else if (strchr("*+/^-", lastC) != NULL)
                {
                    printf("Invalid expression\n");
                    free_tokens(tokens, *length);
                    *length = 0;
                    free(str);
                    return NULL;
                }

                openParentheses--;
            }

            strncat(tokens[currentTokenIndex], &c, 1); // Add character to current token
        }

        else if (strchr(newLineSigns, c) != NULL) // Current character is one of [newLineSigns]
        {
            // If next character is end of string
            if (str[i + 1] == '\0')
            {
                printf("Incomplete expression\n");
                free_tokens(tokens, *length);
                *length = 0;
                free(str);
                return NULL;
            }

            // Allocate memory for 2 new rows
            tokens = realloc(tokens, sizeof(char*) * (++currentTokenIndex + 2));
            ptr_check(tokens);
            
            // First row
            // Allocate memory for one character
            tokens[currentTokenIndex] = calloc(1 + 1, sizeof(char));
            ptr_check(tokens[currentTokenIndex]);
            strncpy(tokens[currentTokenIndex], &c, 1);

            // Second row
            // Allocate enough memory
            tokens[++currentTokenIndex] = calloc(TOKEN_SIZE + 1, sizeof(char));
            ptr_check(tokens[currentTokenIndex]);
            // Reset token capacity
            currentTokenCapacity = TOKEN_SIZE;

            *length += 2; // Increment length
        }

        // Character not recognised
        else
        {
            printf("Invalid character detected: '%c'\n", c);
            free_tokens(tokens, *length);
            *length = 0;
            free(str);
            return NULL;
        }

        lastC = c; // Save current character
    }

    // If after looping through all characters a parenthesis wasn't closed
    if (openParentheses > 0)
    {
        printf("Matching parenthesis not found\n");
        free_tokens(tokens, *length);
        *length = 0;
        free(str);
        return NULL;
    }

    free(str);
    
    // Prints tokens in debug mode
    #ifdef DEBUG_MODE
        printf("Tokens:\n-------------------\n");
        for (int i = 0; i < *length; i++)
            printf("%s\n", tokens[i]);
        printf("-------------------\n");
    #endif

    return tokens;
}

void free_tokens(char **tokens, int length)
{
    // Free each token string
    for (int i = 0; i < length; i++)
    {
        if (tokens[i] != NULL)
            free(tokens[i]);
    }
    // Free token array
    free(tokens);
}

double compute_tokens(char **tokens, int length)
{
    double result = 0;
    char currentOperator = '+';

    for (int i = 0; i < length; i++)
    {
        char *token = tokens[i]; // Current token string
        bool operandLine = (i % 2 != 0); // Operands (+-*/^) are found in odd indexes
        
        // Number or expression
        if (!operandLine)
        {
            double num = 0;
            // If current token is an expression meaning it contains signs or (parentheses and signs)
            if (contains_chars(token, signs) || (contains_chars(token, signs) && contains_chars(token, "()")))
            {
                char tokenExpr[strlen(token) + 1]; // String to store expression
                char *withoutParenthesis = remove_parenthesis_part(token);
                ptr_check(withoutParenthesis);
                // If expression starts with a parenthesis and there are no signs outside parenthesis
                // i.e. whole expression is encapsulated in parenthesis
                // Remove parenthesis and copy to [tokenExpr]
                if (token[0] == '(' && !contains_chars(withoutParenthesis, signs))
                {
                    // Get location of last bracket
                    char *lastBracket = strrchr(token, ')');
                    // Compute length of new string by subtracting the location of last bracket from first character
                    int exprCount = lastBracket - token - 1 /*-1 ensures last parenthesis is not added*/;
                    // Copy specifed length [exprCount] of characters in [token] from first character 
                    strncpy(tokenExpr, token + 1, exprCount);
                    tokenExpr[exprCount] = '\0'; // Add null character
                }
                else
                    // Copy whole string
                    strcpy(tokenExpr, token);

                // Free only if [withoutParenthesis] is not the same as [token]
                if (withoutParenthesis != token) free(withoutParenthesis);

                int tokenCount = 0;
                // Get tokens of inner expression
                char **nestedTokens = tokenise_expression(tokenExpr, &tokenCount);

                // Call function recursively to compute inner tokens
                num = compute_tokens(nestedTokens, tokenCount);
            }

            // Just a number
            else
                // Get number (avoid parenthesis if it's there)
                num = strtod((token[0] == '(' ? (token + 1) : token), NULL);

            switch (currentOperator)
            {
                case '+':
                    result += num;
                    break;

                case '-':
                    result -= num;
                    break;

                case '*':
                    result *= num;
                    break;

                case '/':
                    result /= num;
                    break;
                
                case '^':
                    result = pow(result, num);
                    break;
                
                default:
                    printf("Ahh!!!\nInternal error\n");
                    exit(EXIT_FAILURE);
                    break;
            }
        }

        // Operator (+, -, *, /, ^)
        else
            currentOperator = token[0];
    }

    return result;
}
