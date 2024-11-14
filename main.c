#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*** defines ***/
#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 4
#define MAX_LINE 20

/*** global variables ***/
/* the available amount of each resource */
int available[NUM_RESOURCES];
/*the maximum demand of each customer */
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];
/* the amount currently allocated to each customer */
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];
/* the remaining need of each customer */
int need[NUM_CUSTOMERS][NUM_RESOURCES];

/*** function declarations ***/

int request_resources(int customer_num, int request[]);
void release_resources(int customer_num, int release[]);
// function to print the current state of the system (invoked on * command)
void print();

// function to parse the file and store the data in a matrix
void parse_file(FILE *fptr, int matrix[][NUM_RESOURCES], int rows, int cols);

// function to copy a matrix to another matrix
void copy_matrix(int dest[][NUM_RESOURCES], int src[][NUM_RESOURCES], int rows, int cols);

// function to trim the input from leading and trailing whitespaces
void trim(const char *input, char *output, int output_size);

int main(int argc, char *argv[])
{
    // check if the number of arguments is correct
    if (argc != NUM_RESOURCES + 1)
    {
        printf("Usage: %s <available resources>\n", argv[0]);
        return 1;
    }
    // intialize the available array from arguments
    for (int i = 0; i < NUM_RESOURCES; ++i)
        available[i] = atoi(argv[i + 1]);
    // read contents of the file
    FILE *fptr = fopen("./resources.txt", "r");
    parse_file(fptr, maximum, NUM_CUSTOMERS, NUM_RESOURCES);
    fclose(fptr);

    // copy the maximum matrix to need matrix since allocation is empty
    copy_matrix(need, maximum, NUM_CUSTOMERS, NUM_RESOURCES);
    print();

    char input[100];
    char trimmed_input[100] = {0};
    int resources[NUM_RESOURCES + 1];
    char *command;
    while (1)
    {
        printf("Enter command: ");
        // read a line of input from the user
        if (fgets(input, sizeof input, stdin) != NULL)
        {
            // trim input from whitespaces
            trim(input, trimmed_input, sizeof trimmed_input);
            // exit if user types exit
            if (strcmp(trimmed_input, "exit") == 0)
                break;
            // print the current state
            if (strcmp(trimmed_input, "*") == 0)
            {
                print();
                continue;
            }
            // split the input into command and resources
            char *token = strtok(trimmed_input, " ");
            int i = 0;
            if (token == NULL)
            {
                printf("Invalid command\n");
                continue;
            }
            command = token;
            // read the data into resources array
            while (token != NULL)
            {
                token = strtok(NULL, " ");
                if (token != NULL)
                {
                    resources[i] = atoi(token);
                    i++;
                }
            }
            // check the command RQ or RL
            if (strcmp(command, "RQ") == 0)
            {
                if (request_resources(resources[0], resources + 1) == 0)
                    printf("Request is granted\n");
                else
                    printf("Request is denied\n");
            }
            else if (strcmp(command, "RL") == 0)
            {
                release_resources(resources[0], resources + 1);
            }
            else
            {
                printf("Invalid command\n");
            }
        }
    }

    return 0;
}

void parse_file(FILE *fptr, int matrix[][NUM_RESOURCES], int rows, int cols)
{
    // the function reads the file line by line and splits the string by comma and stroes the values in the matrix
    char line[MAX_LINE];
    int i = 0;
    while (fgets(line, 20, fptr) && i < rows)
    {

        char *token = strtok(line, ",");
        int j = 0;
        while (token != NULL)
        {
            matrix[i][j] = atoi(token);
            token = strtok(NULL, ",");
            ++j;
        }
        ++i;
    }
}

void print()
{
    printf("Maximum\t\tAllocation\tNeed\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++)
    {
        for (int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", maximum[i][j]);
        printf("\t");
        for (int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", allocation[i][j]);
        printf("\t");
        for (int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }
    printf("Available: ");
    for (int i = 0; i < NUM_RESOURCES; i++)
    {
        printf("%d ", available[i]);
    }
    printf("\n\n");
}

void copy_matrix(int dest[][NUM_RESOURCES], int src[][NUM_RESOURCES], int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
            dest[i][j] = src[i][j];
    }
}

int request_resources(int customer_num, int request[])
{
    int i;
    // check if request is greater than need or available
    for (i = 0; i < NUM_RESOURCES; i++)
    {
        if (request[i] > need[customer_num][i])
        {
            printf("Request is greater than need\n");
            return -1;
        }
        if (request[i] > available[i])
        {
            printf("Request is greater than available\n");
            return -1;
        }
    }

    // pretend to allocate requested resources
    for (i = 0; i < NUM_RESOURCES; i++)
    {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    // safety algorithm
    // STEP1: initialize work and finish arrays
    int work[NUM_RESOURCES];
    int finish[NUM_CUSTOMERS];
    for (i = 0; i < NUM_RESOURCES; i++)
    {
        work[i] = available[i];
    }
    for (i = 0; i < NUM_CUSTOMERS; i++)
    {
        finish[i] = 0;
    }
    // STEP2: find an i such that both finish[i] == 0 and need[i] <= work
    i = 0;
    while (i < NUM_CUSTOMERS)
    {
        if (finish[i] == 0)
        {
            int j;
            for (j = 0; j < NUM_RESOURCES; ++j)
            {
                if (need[i][j] > work[j])
                    break;
            }
            // if need[i] <= work and finish[i] == 0, then work += allocation[i] and finish[i] = 1
            if (j == NUM_RESOURCES)
            {
                for (j = 0; j < NUM_RESOURCES; ++j)
                {
                    work[j] += allocation[i][j];
                }
                finish[i] = 1;
                i = 0;
                continue;
            }
        }
        ++i;
    }
    // if finish[i] == 1 for all i, then the system is in a safe state
    for (i = 0; i < NUM_RESOURCES; i++)
    {
        if (finish[i] == 0)
        {
            // restore old state
            for (i = 0; i < NUM_RESOURCES; i++)
            {
                available[i] += request[i];
                allocation[customer_num][i] -= request[i];
                need[customer_num][i] += request[i];
            }
            printf("Request is unsafe\n");
            return -1;
        }
    }
    return 0;
}

void release_resources(int customer_num, int release[])
{
    int i;
    // check if release is greater than allocation for any of the resources
    for (i = 0; i < NUM_RESOURCES; ++i)
    {
        if (release[i] > allocation[customer_num][i])
        {
            printf("Release is greater than allocation\n");
            return;
        }
    }
    for (i = 0; i < NUM_RESOURCES; ++i)
    {
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i];
    }
    printf("Resources are released\n");
}

void trim(const char *input, char *output, int output_size)
{
    if (!input || !output || output_size == 0)
        return;

    // Skip leading whitespace
    while (*input && isspace(*input))
        input++;

    // Copy non-whitespace characters
    const char *end = input + strlen(input) - 1;
    // Find last non-whitespace character
    while (end > input && isspace(*end))
        end--;

    // Calculate length to copy (add 1 to include the last char)
    int len = end - input + 1;
    if (len >= output_size)
        len = output_size - 1;

    // Copy the trimmed portion
    strncpy(output, input, len);
    output[len] = '\0';
}