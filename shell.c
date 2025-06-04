#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

/**
  * get_file_path - Get's the full path of the file
  * @file_name: Argument name
  *
  * Return: The full path argument to the file
  * 
  */

int fd, fd2;
char *custom_path = NULL; // Para o comando path
int stdout_backup = -1;   // Para restaurar stdout após redirecionamento

int startsWithForwardSlash(const char *str)
{
    if (str != NULL && str[0] == '/')
        return (1);

    return (0);
}

char *get_file_loc(char *path, char *file_name)
{
    char *path_copy, *token;
    struct stat file_path;
    char *path_buffer = NULL;

    path_copy = strdup(path);
    token = strtok(path_copy, ":");

    while (token)
    {
        if (path_buffer)
        {
            free(path_buffer);
            path_buffer = NULL;
        }
        path_buffer = malloc(strlen(token) + strlen(file_name) + 2);
        if (!path_buffer)
        {
            perror("Error: malloc failed");
            exit(EXIT_FAILURE);
        }
        strcpy(path_buffer, token);
        strcat(path_buffer, "/");
        strcat(path_buffer, file_name);
        strcat(path_buffer, "\0");

        if (stat(path_buffer, &file_path) == 0 && access(path_buffer, X_OK) == 0)
        {
            free(path_copy);
            return (path_buffer);
        }
        token = strtok(NULL, ":");
    }
    free(path_copy);
    if (path_buffer)
        free(path_buffer);
    return (NULL);
}

char *get_file_path(char *file_name)
{
    char *path;
    char *full_path;
    
    if (startsWithForwardSlash(file_name) && access(file_name, X_OK) == 0)
        return (strdup(file_name));

    // Usar custom_path se definido, senão usar PATH do ambiente
    if (custom_path)
        path = custom_path;
    else
        path = getenv("PATH");

    if (!path)
    {
        perror("Path not found");
        return (NULL);
    }
    full_path = get_file_loc(path, file_name);

    if (full_path == NULL)
    {
        perror("Absolute path not found");      
        return (NULL);
    }

    return (full_path);
}

void handle_pwd()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }
    else
    {
        perror("pwd");
    }
}

void handle_path(char **args, int num_args)
{
    if (custom_path)
    {
        free(custom_path);
        custom_path = NULL;
    }
    
    if (num_args == 1)
    {
        // Sem argumentos, limpa o path
        return;
    }
    
    // Calcula tamanho necessário
    int total_len = 0;
    for (int i = 1; i < num_args; i++)
    {
        total_len += strlen(args[i]) + 1; // +1 para ':'
    }
    
    custom_path = malloc(total_len);
    if (!custom_path)
    {
        perror("malloc failed");
        return;
    }
    
    strcpy(custom_path, args[1]);
    for (int i = 2; i < num_args; i++)
    {
        strcat(custom_path, ":");
        strcat(custom_path, args[i]);
    }
}

// Implementação do comando cat
void handle_cat(char **args, int num_args)
{
    if (num_args < 2)
    {
        printf("cat: missing file operand\n");
        return;
    }
    
    for (int i = 1; i < num_args; i++)
    {
        FILE *file = fopen(args[i], "r");
        if (!file)
        {
            perror(args[i]);
            continue;
        }
        
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file))
        {
            printf("%s", buffer);
        }
        
        fclose(file);
    }
}

// Implementação do comando ls
void handle_ls(char **args, int num_args)
{
    int show_hidden = 0;
    int long_format = 0;
    char *directory = ".";
    
    // Parse dos argumentos
    for (int i = 1; i < num_args; i++)
    {
        if (strcmp(args[i], "-a") == 0)
            show_hidden = 1;
        else if (strcmp(args[i], "-l") == 0)
            long_format = 1;
        else if (strcmp(args[i], "-la") == 0 || strcmp(args[i], "-al") == 0)
        {
            show_hidden = 1;
            long_format = 1;
        }
        else
            directory = args[i];
    }
    
    DIR *dir = opendir(directory);
    if (!dir)
    {
        perror("ls");
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Pula arquivos ocultos se -a não foi especificado
        if (!show_hidden && entry->d_name[0] == '.')
            continue;
            
        if (long_format)
        {
            struct stat file_stat;
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            
            if (stat(full_path, &file_stat) == 0)
            {
                // Formato longo simplificado
                printf("%c", (S_ISDIR(file_stat.st_mode)) ? 'd' : '-');
                printf("%c", (file_stat.st_mode & S_IRUSR) ? 'r' : '-');
                printf("%c", (file_stat.st_mode & S_IWUSR) ? 'w' : '-');
                printf("%c", (file_stat.st_mode & S_IXUSR) ? 'x' : '-');
                printf("%c", (file_stat.st_mode & S_IRGRP) ? 'r' : '-');
                printf("%c", (file_stat.st_mode & S_IWGRP) ? 'w' : '-');
                printf("%c", (file_stat.st_mode & S_IXGRP) ? 'x' : '-');
                printf("%c", (file_stat.st_mode & S_IROTH) ? 'r' : '-');
                printf("%c", (file_stat.st_mode & S_IWOTH) ? 'w' : '-');
                printf("%c", (file_stat.st_mode & S_IXOTH) ? 'x' : '-');
                printf(" %ld %ld %s\n", file_stat.st_nlink, file_stat.st_size, entry->d_name);
            }
            else
            {
                printf("%s\n", entry->d_name);
            }
        }
        else
        {
            printf("%s  ", entry->d_name);
        }
    }
    
    if (!long_format)
        printf("\n");
        
    closedir(dir);
}

void handle_cd(char **args, int num_args)
{
    if (num_args == 1)
    {
        return;
    }
    
    else if (num_args == 2)
    {
        if (chdir(args[1]) != 0)
            perror("cd");
    }
    else
    {
        return;
    }
}

int handle_builtin_commands(char **args, int num_args, char *input)
{
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        handle_cd(args, num_args);
        return (1);
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        handle_pwd();
        return (1);
    }
    else if (strcmp(args[0], "path") == 0)
    {
        handle_path(args, num_args);
        return (1);
    }
    else if (strcmp(args[0], "cat") == 0)
    {
        handle_cat(args, num_args);
        return (1);
    }
    else if (strcmp(args[0], "ls") == 0)
    {
        handle_ls(args, num_args);
        return (1);
    }

    return (0);
}

void redirecionar(char *arquivo)
{
    if (stdout_backup == -1)
        stdout_backup = dup(STDOUT_FILENO);
        
    fd = open(arquivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");
        return;
    }
    
    dup2(fd, STDOUT_FILENO);
}

void restaurar_stdout()
{
    if (stdout_backup != -1)
    {
        dup2(stdout_backup, STDOUT_FILENO);
        close(stdout_backup);
        stdout_backup = -1;
    }
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

// Função para executar pipe
void execute_pipe(char *input)
{
    char *commands[1024];
    int num_commands = 0;
    
    // Dividir por |
    char *token = strtok(input, "|");
    while (token && num_commands < 1024)
    {
        commands[num_commands] = token;
        num_commands++;
        token = strtok(NULL, "|");
    }
    
    if (num_commands < 2)
        return;
    
    int pipes[num_commands - 1][2];
    pid_t pids[num_commands];
    
    // Criar pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return;
        }
    }
    
    // Executar comandos
    for (int i = 0; i < num_commands; i++)
    {
        // Parse do comando
        char **args = malloc(sizeof(char*) * 1024);
        int arg_count = 0;
        char *cmd_token = strtok(commands[i], " \n\t");
        
        while (cmd_token && arg_count < 1023)
        {
            args[arg_count] = cmd_token;
            arg_count++;
            cmd_token = strtok(NULL, " \n\t");
        }
        args[arg_count] = NULL;
        
        if (arg_count == 0)
        {
            free(args);
            continue;
        }
        
        pids[i] = fork();
        
        if (pids[i] == 0)
        {
            // Processo filho
            
            // Configurar stdin
            if (i > 0)
            {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Configurar stdout
            if (i < num_commands - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Fechar todos os pipes
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Verificar se é builtin
            if (handle_builtin_commands(args, arg_count, commands[i]))
            {
                free(args);
                exit(0);
            }
            
            // Executar comando externo
            char *path = get_file_path(args[0]);
            if (path)
            {
                execve(path, args, NULL);
                free(path);
            }
            
            perror("execve failed");
            free(args);
            exit(1);
        }
        
        free(args);
    }
    
    // Fechar pipes no processo pai
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Esperar todos os processos
    for (int i = 0; i < num_commands; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);
    }
}

void *execute_command(void *arg)
{
    char *input = (char *)arg;
    char *token;
    char *token2;
    size_t count = 0;
    ssize_t nread;
    pid_t child_pid;
    int i, p, l, status;
    char **array;
    char **array2;
    char **array3;
    
    char *path;
    char *arquivo;
    
    int num_args;
    int has_redirection = 0;
    
    // Verificar se tem pipe
    if (strchr(input, '|'))
    {
        execute_pipe(input);
        pthread_exit(0);
    }
    
    // Verificar redirecionamento
    if (strchr(input, '>'))
    {
        has_redirection = 1;
        token = strtok(input, ">");

        array2 = malloc(sizeof(char*) * 1024);
        p = 0;

        while (token)
        {
            array2[p] = token;
            token = strtok(NULL, ">");
            p++;
        }

        array2[p] = NULL;
        
        if (array2[1])
        {
            token = strtok(array2[1], " \n\t");
            array3 = malloc(sizeof(char*) * 1024);
            l = 0;

            while (token)
            {
                array3[l] = token;
                token = strtok(NULL, " \n\t");
                l++;
            }

            array3[l] = NULL;
            if (array3[0])
            {
                redirecionar(array3[0]);
            }
            free(array3);
        }
        
        token = strtok(array2[0], " \n\t");
        free(array2);
    }
    else
    {
        token = strtok(input, " \n\t");
    }

    array = malloc(sizeof(char*) * 1024);
    i = 0;

    while (token)
    {
        array[i] = token;
        token = strtok(NULL, " \n\t");
        i++;
    }

    array[i] = NULL;
    num_args = i;

    if (num_args == 0)
    {
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }

    if (handle_builtin_commands(array, num_args, input))
    {
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }
    
    path = get_file_path(array[0]);
    if (!path)
    {
        printf("Command not found: %s\n", array[0]);
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }
    
    child_pid = fork();

    if (child_pid == -1)
    {
        perror("Failed to create process");
        free(path);
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }

    if (child_pid == 0)
    {
        if (execve(path, array, NULL) == -1)
        {
            perror("Failed to execute");
            exit(97);
        }
    }
    else
    {
        wait(&status);
        printf("Process exited with status: %d\n", WEXITSTATUS(status));
    }
    
    free(path);
    free(array);
    if (has_redirection)
        restaurar_stdout();
    
    pthread_exit(0);
}

void removeChar(char *str, char garbage) 
{
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

// Função para executar comandos de arquivo batch
void execute_batch_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open batch file");
        return;
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    while ((read = getline(&line, &len, file)) != -1)
    {
        // Remove newline
        if (line[read-1] == '\n')
            line[read-1] = '\0';
            
        printf("Executing: %s\n", line);
        
        // Processar comando igual ao modo interativo
        char *token = strtok(line, "&");
        char **array = malloc(sizeof(char*) * 1024);
        int i = 0;
        
        while (token)
        {
            array[i] = strdup(token); // Duplicar para evitar problemas com strtok
            token = strtok(NULL, "&");
            i++;
        }
        
        array[i] = NULL;
        int num_threads = i;
        
        if (num_threads > 0)
        {
            pthread_t workers[num_threads];
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            
            for (int j = 0; j < num_threads; j++)
            {
                pthread_create(&workers[j], &attr, execute_command, array[j]);
            }
            
            for (int k = 0; k < num_threads; k++)
            {
                pthread_join(workers[k], NULL);
                free(array[k]);
            }
            
            pthread_attr_destroy(&attr);
        }
        
        free(array);
    }
    
    if (line)
        free(line);
    fclose(file);
}

int main(int argc, char *argv[])
{
    char *token;
    char **array;
    char *buf = NULL;
    int num_threads;
    size_t count = 0;
    ssize_t nread;
    int i;
    
    // Verificar se foi passado arquivo batch
    if (argc == 2)
    {
        execute_batch_file(argv[1]);
        return 0;
    }

    while(1)
    {
        if (isatty(STDIN_FILENO))
            write(STDOUT_FILENO, "/> ", 3);
            
        nread = getline(&buf, &count, stdin);

        if (nread == -1)
        {
            exit(1);
        }
        
        // Remove newline
        if (buf[nread-1] == '\n')
            buf[nread-1] = '\0';
            
        token = strtok(buf, "&");
        array = malloc(sizeof(char*) * 1024);
        i = 0;
        
        while (token)
        {
            array[i] = strdup(token); // Duplicar string para evitar problemas
            token = strtok(NULL, "&");
            i++;
        }

        array[i] = NULL;
        num_threads = i;
        
        if (num_threads > 0)
        {
            pthread_t workers[num_threads];
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            
            // Bug corrigido: usar j em vez de i
            for (int j = 0; j < num_threads; j++)
            {
                pthread_create(&workers[j], &attr, execute_command, array[j]);
            }

            // Bug corrigido: usar k em vez de i
            for (int k = 0; k < num_threads; k++)
            {
                pthread_join(workers[k], NULL);
                free(array[k]); // Liberar strings duplicadas
            }
            
            pthread_attr_destroy(&attr);
        }
        
        free(array);
    }
    
    // Cleanup
    if (custom_path)
        free(custom_path);
    if (buf)
        free(buf);
    
    return 0;
}