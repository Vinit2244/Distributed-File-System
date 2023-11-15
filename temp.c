#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

char **tokenize(const char *str, const char ch)
{
    // Counting the number of delimiters
    int num_of_ch = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ch)
            num_of_ch++;
    }

    // Number of tokens would be 1 more than the number of delimiters present in the string
    int num_of_tokens = num_of_ch + 1;

    // Allocating num_of_tokens + 1 memory because we need to store last token as NULL token to mark the end of tokens
    char **tokens = (char **)malloc((num_of_tokens + 1) * sizeof(char *));
    for (int i = 0; i < num_of_tokens; i++)
    {
        tokens[i] = (char *)calloc(1024, sizeof(char));
    }
    // The last token will be kept null so that when traversing we would know when the tokens end by checking for NULL token
    tokens[num_of_tokens] = NULL;

    int token_idx = 0;     // Index of the token being stored
    int token_str_idx = 0; // Index where the next character is to be stored on token
    for (int i = 0; i < strlen(str); i++)
    {
        // If the delimiter character is encountered increment the token index by 1 to start storing the next token and reset the token string index to 0 to start storing from the starting of the string
        if (str[i] == ch)
        {
            token_idx++;
            token_str_idx = 0;
            continue;
        }
        else
        {
            tokens[token_idx][token_str_idx++] = str[i];
        }
    }

    return tokens;
}

int main()
{
    char buffer[1024] = {0};
    printf("Enter request string : ");
    scanf("%s", buffer);

    char** request_tkns = tokenize(buffer, '|');

    char *file_path = request_tkns[0];
    char *file_content = request_tkns[1];

    // Creating intermediate directories if not already present
    // First tokenising the file_path on "/"

    char** dirs = tokenize(file_path, '/');

    // Calculating the number of intermediate dirs
    int n_tkns = 0;
    while (dirs[n_tkns] != NULL)
    {
        n_tkns++;
    }

    // Final number of dirs is 1 less than the number of tokens as the last one is the file
    int n_dirs = n_tkns - 1;

    // Now creating all the intermediate dirs one by one
    for (int i = 0; i < n_dirs; i++)
    {
        if (mkdir(dirs[i], 0777) == 0)
        {
            // If the directory did not exist already then it got created
        }
        else if (errno == EEXIST)
        {
            // If the directory already exists then do nothing and just move into it
        }
        else
        {
            // Error creating the directory
            fprintf(stderr, "mkdir : %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        // Moving into that directory to create the next directory in hierarchy
        chdir(dirs[i]);
    }

    // Moving out back to the pwd
    chdir("/Users/vinitmehta/Desktop/IIITH_Main/Sem3/OSN/mini_projects/final-project-43");

    // Now opening the file in write mode, if it does not exist it would be created otherwise the old data would be overwritten
    FILE *fptr = fopen(file_path, "w");

    fprintf(fptr, "%s", file_content);

    fclose(fptr);

    return 0;
}