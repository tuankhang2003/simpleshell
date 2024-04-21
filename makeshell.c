#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

#define BUFSIZE 1024
char *shell_readline(void)
{
    int bufsize = BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * BUFSIZE);
    int c;
    if (!buffer)
    {
        fprintf(stderr, "Allocation error \n");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        c = getchar();
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position++] = c;
        }

        if (position >= bufsize)
        {
            bufsize += BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (buffer == NULL)
            {
                printf(stderr, "Allocation error \n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
char **shell_split_line(char *line)
{
    int bufsize = TOK_BUFSIZE;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;
    int position = 0;
    if (!tokens)
    {
        fprintf(stderr, "Allocation error \n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, TOK_DELIM);
    while (token != NULL)
    {
        tokens[position++] = token;
        if (position >= bufsize)
        {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "Allocation error \n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}
int shell_launch(char **args)
{
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0)
    { // child
        if (execvp(args[0], args) == -1)
        {
            perror("error in shell_launch");
        }
        exit(EXIT_FAILURE); // after execvp is not executed, when yes -> fail
    }
    else if (pid < 0)
    {
        perror("Error in shell_launch");
    }
    else
    {
        // parent process (wait)
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // while no interrupted
    }
    return 1;
}
char *builtin_str[] = {
    "cd", "help", "exit"};

char (*builtin_func[])(char **) = {
    &shell_cd, &shell_help, &shell_exit};

int num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("error, wrong directory");
        }
    }
    return 1;
}

int shell_help(char **args)
{
    printf("Khang simple shell\n");
    printf("The following are built in:\n");
    for (int i = 0; i < num_builtins(); i++)
    {
        printf(" %s\n", builtin_str[i]);
    }
    return 1;
}
int shell_exit(char **args)
{
    return 0;
}

int shell_execute(char **args)
{
    if (args[0] == NULL)
    {
        return 1;
    }
    for (int i = 0; i < num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}
void shell_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("$ ");
        line = shell_readline();
        args = shell_split_line(line);
        status = shell_execute(args);
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    shell_loop();
    return EXIT_SUCCESS;
}