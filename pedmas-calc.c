#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#define TOKEN_SIZE 100

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
            int tokenCount = 0;
            // Turn string into comprehensible array of strings
            char **tokens = tokenise_expression(equationStr, &tokenCount);
            if (tokens != NULL)
            {
                #ifndef DEBUG_MODE
                    double sum = 0;
                    char currentOperator = '+';
                #endif
                for (int i = 0; i < tokenCount; i++)
                {
                    char *token = tokens[i];

                    #ifdef DEBUG_MODE
                        printf("%s\n", token);
                    #else
                        // Even indexes contain operators
                        if (i % 2 != 0)
                            currentOperator = token[0];
                        else
                            // If operator is a not an addition sign add the negative result to sum
                            sum += (currentOperator == '+' ? 1 : -1) * compute_expression(token);
                    #endif

                    free(token);
                }
                // Print expression and answer in format "3 * 2 + 3 = 9"
                // Check if answer is integer or double
                #ifndef DEBUG_MODE
                    if ((int) sum == sum) // Integer
                        printf("%s = %d\n", equationStr, (int) sum);
                    else // Double
                        printf("%s = %f\n", equationStr, sum);
                #endif

                free(tokens);
            }
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

// Utility function to show a message and free allocated memory
// Need to have address to pointers to free them, hence the "***"
void free_tokens(const char *message, char ***tokens, int len)
{
    printf("%s\n", message);
    for (int i = 0; i < len; i++)
    {
        if ((*tokens)[i] != NULL)
            free((*tokens)[i]);
    }
    free(*tokens);
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

char **tokenise_expression(char *eq, int *length)
{
    remove_spaces(eq);

    int currentIndex = 0;

    char **tokens = malloc(sizeof(char*) * 1);
    tokens[0] = calloc(TOKEN_SIZE, sizeof(char));

    *length = 1;

    char c = eq[0], lastC = 0;
    for (int i = 0; i < strlen(eq); i++)
    {
        c = eq[i];
        int charIsSign = strchr("+-*/^.", c) != NULL;

        if (charIsSign)
        {
            // Check if last character and current character are signs
            if (strchr("+-*/^.", lastC) != NULL)
            {
                free_tokens("Invalid expression", &tokens, *length);
                return NULL;
            }

            // Check if operator is the last character
            if (eq[i + 1] == '\0')
            {
                free_tokens("Incomplete expression", &tokens, *length);
                return NULL;
            }
        }

        // Allow plus or minus sign at the start of the expression
        if (i == 0 && strchr("+-", c) != NULL)
            strncpy(tokens[currentIndex], &c, 1);

        // Treat numbers with "*/^." as single quantities to enforce PEDMAS
        else if (isdigit(c) || strchr("*/^.", c) != NULL)
        {
            // If character equals any of the operators
            if (strchr("*/^.", c) != NULL)
            {
                // If operator is the first character in string
                if (strlen(tokens[currentIndex]) == 0)
                {
                    free_tokens("Don't start an expression with an operator", &tokens, *length);
                    return NULL;
                }
            }

            strncat(tokens[currentIndex], &c, 1);
        }

        else if (strchr("+-", c))
        {
            // Put plus and minus operators on their own line
            currentIndex++;

            // Allocate space for two more rows
            tokens = realloc(tokens, sizeof(char*) * (currentIndex + 2));
            // Allocate memory for first row
            tokens[currentIndex] = calloc(1, sizeof(char));
            // Put sign in first row
            strncpy(tokens[currentIndex], &c, 1);

            // Allocate memory for second row
            tokens[++currentIndex] = calloc(TOKEN_SIZE, sizeof(char));
            
            // Two new rows added
            *length += 2;
        }

        // If character not supported
        else if (!isspace(c))
        {
            free_tokens("Invalid character found in expression", &tokens, *length);
            return NULL;
        }

        lastC = c;
    }

    return tokens;
}

void perform_operation(double operand, double *out, char operator)
{
    switch (operator)
    {
        case '+':
            *out += operand;
            break;

        case '-':
            *out -= operand;
            break;

        case '*':
            *out *= operand;
            break;

        case '/':
            *out /= operand;
            break;
        
        default:
            printf("Tokenisation error\n");
            exit(EXIT_FAILURE);
            break;
    }
}

double compute_expression(char *token)
{
    double result = 0, lastNum = 1;
    char c = token[0], currentSign = '+', lastSign = 0;
    for (int i = 0, len = strlen(token); i < len; i++)
    {
        c = token[i];

        // A number, plus or minus sign, or parenthesis
        if (isdigit(c) || (i == 0 && strchr("+-", c) != NULL))
        {
            double num = 0;
            if (isdigit(c) || strchr("+-", c) != NULL)
            {
                // Pointer to where strtod stopped parsing 
                char *end;
                num = strtod(token + i, &end);

                // Increment index to skip the parsed numbers
                i += (end - (token + i)) - 1;
            }
            
            // If current and next operator is not an exponent sign
            if (token[i + 1] != '^' && currentSign != '^')
                perform_operation(num, &result, currentSign);
            else if (currentSign == '^')
            {
                double exponentResult = pow(lastNum, num);
                if (lastSign == '^')
                {
                    printf("Unsupported use of exponent sign\n");
                    return result;
                }
                perform_operation(exponentResult, &result, lastSign);
            }
            
            // Save current number
            lastNum = num;
        }

        // Other signs
        else if (strchr("+-/*^", c) != NULL)
        {
            lastSign = currentSign;
            currentSign = c;
        }
    }

    return result;
}
