#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 4
#define MAX_LINE 20

/* the available amount of each resource */
int available[NUM_RESOURCES];
/*the maximum demand of each customer */
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];
/* the amount currently allocated to each customer */
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];
/* the remaining need of each customer */
int need[NUM_CUSTOMERS][NUM_RESOURCES];

int request_resources(int customer_num, int request[]);
void release_resources(int customer_num, int release[]);
void print();
void parse_file(FILE *fptr, int matrix[][NUM_RESOURCES], int rows, int cols);
void copy_matrix(int dest[][NUM_RESOURCES], int src[][NUM_RESOURCES], int rows, int cols);

int main(int argc, char *argv[])
{
    if (argc != NUM_RESOURCES + 1)
    {
        printf("Usage: %s <available resources>\n", argv[0]);
        return 1;
    }
    // intialize the available array from arguments
    for (int i = 0; i < NUM_RESOURCES; ++i)
        available[i] = atoi(argv[i + 1]);

    FILE *fptr = fopen("./resources.txt", "r");
    parse_file(fptr, maximum, NUM_CUSTOMERS, NUM_RESOURCES);
    fclose(fptr);
    // copy the maximum matrix to need matrix since allocation is empty
    copy_matrix(need, maximum, NUM_CUSTOMERS, NUM_RESOURCES);
    print();

    char input[100];
    char trimmed_input[sizeof input];
    int resources[NUM_RESOURCES];
    char *command;
    while (1)
    {
        if (fgets(input, sizeof input, stdin) != NULL)
        {
            // trim input from whitespaces
            sscanf(input, " %99[^\n]", trimmed_input);
            // exit if user types exit
            if (strcmp(trimmed_input, "exit") == 0)
                break;
            // print the current state
            if (strcmp(trimmed_input, "*") == 0)
            {
                print();
                continue;
            }

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
                resources[i] = atoi(token);
                i++;
            }
            // check the command RQ and RL
            if (strcmp(command, "RQ") == 0)
            {
                request_resources(resources[0], resources + 1);
            }
            else if (strcmp(token, "RL") == 0)
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
    for (int i = 0; i < NUM_CUSTOMERS; i++)
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
    int i = 0;
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
            printf("Request is unsafe\n");
            return -1;
        }
    }

    // update resources
    for (i = 0; i < NUM_RESOURCES; i++)
    {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }
    return 0;
}
void release_resources(int customer_num, int release[])
{
    int i;
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
}