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

typedef struct Operation
{
    char operator; // +, -, *, /, ^
    double op1, op2; // operand 1, operand 2
    struct Operation *nestedOp1, *nestedOp2; // inner operations
} Operation;

char* get_expression(void);
char** tokenise_expression(const char *, int *);
Operation* parse_tokens(char **, int);
void print_parse_tree(const Operation *, int);
void free_parse_tree(Operation *);
void free_tokens(char **, int);
double compute_parse_tree(const Operation *);

int main(int argc, const char *argv[])
{
    // Get expression string
    char *expressionStr = get_expression();
    // Loop until 'q','e','x' is detected as the first character
    while (strchr("qex", expressionStr[0]) == NULL)
    {
        // Check if previous function call is successful
        if (expressionStr != NULL && strlen(expressionStr) > 0)
        {
            int tokenCount = 0;
            char **tokens = tokenise_expression(expressionStr, &tokenCount);
            if (tokens != NULL)
            {
                double result = 0;
                if (tokenCount > 1)
                {
                    Operation *parseTree = parse_tokens(tokens, tokenCount);
                    if (parseTree != NULL)
                    {
                        result = compute_parse_tree(parseTree);
                        free_parse_tree(parseTree);
                    }
                }
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
            free(expressionStr);
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

// Returns an array of strings (tokens) that is then converted into an Abstract Syntax Tree (AST)
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
    remove_spaces(str);
    add_multiplication_sign_btw_parenthesis(str);

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
                goto checkSigns; // Go back to the top
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

// Turns tokens into an Abstract Syntax Tree (AST)
// e.g. 3*2+2 =>
    // op1:
        // op1: 3
        // operator: *
        // op2: 2
    // operator: +
    // op2: 2
Operation* parse_tokens(char **tokens, int length)
{
    Operation *head = calloc(1, sizeof(Operation)); // Starting node
    ptr_check(head);
    Operation *pointer = head; // Points to current node

    for (int i = 0; i < length; i++)
    {
        char *token = tokens[i]; // Current token string
        bool operandLine = (i % 2 != 0); // Operands (+-*/^) are found in odd indexes
        bool nextIsSign = (i + 1 < length && strchr(signs, tokens[i + 1][0]) != NULL);
        
        // Number or expression
        if (!operandLine)
        {
            // If current token is an expression meaning it contains signs or (parentheses and signs)
            if (contains_chars(token, signs) || (contains_chars(token, signs) && contains_chars(token, "()")))
            {
                char tokenExpr[strlen(token) + 1]; // String to store expression
                char *withoutParenthesis = remove_parenthesis_part(token);
                ptr_check(withoutParenthesis);
                // If expression starts with a parenthesis and there are no signs outside parenthesis
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
                if (withoutParenthesis != token)
                    free(withoutParenthesis);

                int tokenCount = 0;
                // Get tokens of inner expression
                char **nestedTokens = tokenise_expression(tokenExpr, &tokenCount);

                // Get parse tree of inner expression
                Operation *op = parse_tokens(nestedTokens, tokenCount);
                free_tokens(nestedTokens, tokenCount);

                // If operator is not set, first operand has not been set
                if (pointer->operator == 0)
                    // Set the current node's op1 to new node
                    pointer->nestedOp1 = op;
                else
                {
                    // If there is a sign after expression, create a nested node
                    if (nextIsSign)
                    {
                        Operation *nestedOp = calloc(1, sizeof(Operation));
                        ptr_check(nestedOp);
                        // Set the nested node's op1 to newly created node
                        nestedOp->nestedOp1 = op;
                        
                        // Set the current node's op2 to nested node
                        pointer->nestedOp2 = nestedOp;
                        // Make nested node the current node
                        pointer = nestedOp;
                    }
                    else
                        // Set the current node's op2 to new node
                        pointer->nestedOp2 = op;
                }
            }

            // Just a number
            else
            {
                // Get number (avoid parenthesis if it's there)
                double num = strtod((token[0] == '(' ? (token + 1) : token), NULL);
                // If operator is not set, first operand has not been set
                if (pointer->operator == 0)
                    // Set current node's op1 to number
                    pointer->op1 = num;
                else
                {
                    // If there is a sign after expression, create a nested node
                    if (nextIsSign)
                    {
                        Operation *newOp = calloc(1, sizeof(Operation));
                        ptr_check(newOp);
                        // Set nested node's op1 to number
                        newOp->op1 = num;

                        // Set current node's op1 to nested node
                        pointer->nestedOp2 = newOp;
                        // Make nested node the current node
                        pointer = newOp;
                    }
                    else
                        // Set current node's op2 to number
                        pointer->op2 = num;
                }
            }
        }

        // Operators (+, -, *, /, ^)
        else
            // Set current node's operator to first character of token  
            pointer->operator = token[0];
    }

    // Print parse tree if in debug mode
    #ifdef DEBUG_MODE
        printf("Parse tree\n--------------------------------------\n");
        print_parse_tree(head, 0);
        printf("--------------------------------------\n");
    #endif

    return head;
}

// Helps to show nesting in print_parse_tree function
void print_tabs(int n)
{
    for (int i = 0; i < n; i++)
        printf("\t");
}

// Debugging function to see parse tree
void print_parse_tree(const Operation *head, int currentLevel)
{
    // Operand 1
    print_tabs(currentLevel);
    printf("op1: "), fflush(stdout);
    if (head->nestedOp1 == NULL)
        printf("%f\n", head->op1);
    // Show nesting if any
    else
    {
        printf("\n");
        print_parse_tree(head->nestedOp1, currentLevel + 1);
    }
    
    // Operator
    print_tabs(currentLevel);
    printf("operator: %c\n", head->operator);

    // Operator 2
    print_tabs(currentLevel);
    printf("op2: "), fflush(stdout);
    if (head->nestedOp2 == NULL)
        printf("%f\n", head->op2);
    // Show nesting if any
    else
    {
        printf("\n");
        print_parse_tree(head->nestedOp2, currentLevel + 1);
    }
}

void free_parse_tree(Operation *head)
{
    // Free inner nodes first
    if (head->nestedOp1 != NULL)
        free_parse_tree(head->nestedOp1);
    if (head->nestedOp2 != NULL)
        free_parse_tree(head->nestedOp2);

    // Free node
    free(head);
}

double compute_parse_tree(const Operation *tree)
{
    double result = 0;
    double op1 = (tree->nestedOp1 != NULL)
        ? compute_parse_tree(tree->nestedOp1)
        : tree->op1;
    double op2 = (tree->nestedOp2 != NULL)
        ? compute_parse_tree(tree->nestedOp2)
        : tree->op2;

    switch (tree->operator)
    {
        case '+':
            result = op1 + op2;
            break;

        case '-':
            result = op1 - op2;
            break;

        case '*':
            result = op1 * op2;
            break;

        case '/':
            result = op1 / op2;
            break;
        
        case '^':
            result = pow(op1, op2);
            break;
        
        default:
            printf("Ahh!!!\nInternal error\n");
            exit(EXIT_FAILURE);
            break;
    }

    return result;
}
