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

// Variáveis globais
int fd, fd2;                  //File descriptors para redirecionamento
char *custom_path = NULL;     //Variável para o path
int stdout_backup = -1;       //Backup do stdout original para redirecionamento

//Verifica se uma string começa com '/'
int startsWithForwardSlash(const char *str)
{
    //Verifica se a string não é NULL e se o primeiro caractere é /
    if (str != NULL && str[0] == '/')
        return (1);

    return (0);
}

//Procura por um executável nos diretórios do PATH
char *get_file_loc(char *path, char *file_name)
{
    char *path_copy, *token;     //Cópia do PATH e token para parsing
    struct stat file_path;       //Estrutura para informações do arquivo
    char *path_buffer = NULL;    //Buffer para construir o caminho completo

    path_copy = strdup(path);    //Duplica o PATH para não modificar o original
    token = strtok(path_copy, ":");  //Divide o PATH pelos ':' (separador de diretórios)

    //Percorre cada diretório no PATH
    while (token)
    {
        //Libera o buffer anterior se existir
        if (path_buffer)
        {
            free(path_buffer);
            path_buffer = NULL;
        }
        
        //Aloca memória para o caminho completo: diretório + '/' + nome do arquivo + '\0'
        path_buffer = malloc(strlen(token) + strlen(file_name) + 2);
        if (!path_buffer)
        {
            perror("Error: malloc failed");
            exit(EXIT_FAILURE);
        }
        
        //Constrói o caminho completo
        strcpy(path_buffer, token);      //Copia o diretório
        strcat(path_buffer, "/");        //Adiciona o separador
        strcat(path_buffer, file_name);  //Adiciona o nome do arquivo
        strcat(path_buffer, "\0");       

        //Verifica se o arquivo existe e é executável
        if (stat(path_buffer, &file_path) == 0 && access(path_buffer, X_OK) == 0)
        {
            free(path_copy);  //Libera a cópia do PATH
            return (path_buffer);  //Retorna o caminho encontrado
        }
        token = strtok(NULL, ":");  //Próximo diretório no PATH
    }
    
    //Se chegou aqui, não encontrou o arquivo
    free(path_copy);     //Libera a cópia do PATH
    if (path_buffer)
        free(path_buffer);  //Libera o buffer se alocado
    return (NULL);       //Retorna NULL indicando que não encontrou
}

//Função principal para obter o caminho completo de um executável
char *get_file_path(char *file_name)
{
    char *path;      //PATH a ser usado
    char *full_path; //Caminho completo encontrado
    
    //Se o nome do arquivo já é um caminho absoluto e é executável
    if (startsWithForwardSlash(file_name) && access(file_name, X_OK) == 0)
        return (strdup(file_name));  //Retorna uma cópia do caminho

    //Decide qual PATH usar: customizado ou do ambiente
    if (custom_path)
        path = custom_path;          //Usa PATH customizado se definido
    else
        path = getenv("PATH");       //Usa PATH do ambiente

    //Verifica se o PATH existe
    if (!path)
    {
        perror("Path not found");
        return (NULL);
    }
    
    //Procura o arquivo no PATH
    full_path = get_file_loc(path, file_name);

    if (full_path == NULL)
    {
        perror("Absolute path not found");      
        return (NULL);
    }

    return (full_path);  //Retorna o caminho encontrado
}

//pwd (print working directory)
void handle_pwd()
{
    char cwd[1024];  //Buffer para o diretório atual
    
    //Obtém o diretório atual
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);  //Imprime o diretório atual
    }
    else
    {
        perror("pwd");
    }
}

//path (define PATH customizado)
void handle_path(char **args, int num_args)
{
    //Libera o PATH customizado anterior se existir
    if (custom_path)
    {
        free(custom_path);
        custom_path = NULL;
    }
    
    //Se não há argumentos, apenas limpa o PATH
    if (num_args == 1)
    {
        return;  //PATH fica vazio (NULL)
    }
    
    //Calcula o tamanho total necessário para o novo PATH
    int total_len = 0;
    for (int i = 1; i < num_args; i++)
    {
        total_len += strlen(args[i]) + 1; // +1 para o ':' separador
    }
    
    //Aloca memória para o novo PATH
    custom_path = malloc(total_len);
    if (!custom_path)
    {
        perror("malloc failed");
        return;
    }
    
    //Constrói o novo PATH juntando os argumentos com ':'
    strcpy(custom_path, args[1]);        //Primeiro diretório
    for (int i = 2; i < num_args; i++)
    {
        strcat(custom_path, ":");        //Adiciona separador
        strcat(custom_path, args[i]);    //Adiciona próximo diretório
    }
}

//cat
void handle_cat(char **args, int num_args)
{
    //Verifica se há arquivos especificados
    if (num_args < 2)
    {
        printf("cat: missing file operand\n");
        return;
    }
    
    //Processa cada arquivo especificado
    for (int i = 1; i < num_args; i++)
    {
        FILE *file = fopen(args[i], "r");  //Abre arquivo para leitura (r)
        if (!file)
        {
            perror(args[i]);  //Erro se não conseguir abrir
            continue;         //Continua com próximo arquivo
        }
        
        char buffer[1024];  //Buffer para leitura
        //Lê e imprime o conteúdo do arquivo linha por linha
        while (fgets(buffer, sizeof(buffer), file))
        {
            printf("%s", buffer);  //Imprime a linha lida
        }
        
        fclose(file);  //Fecha o arquivo
    }
}

//ls
void handle_ls(char **args, int num_args)
{
    int show_hidden = 0;   //Flag para mostrar arquivos ocultos
    int long_format = 0;   //Flag para formato longo
    char *directory = "."; //Diretório padrão (atual)
    
    //Parse dos argumentos/flags
    for (int i = 1; i < num_args; i++)
    {
        if (strcmp(args[i], "-a") == 0)
            show_hidden = 1;  //Flag -a: mostrar arquivos ocultos
        else if (strcmp(args[i], "-l") == 0)
            long_format = 1;  //Flag -l: formato longo
        else if (strcmp(args[i], "-la") == 0 || strcmp(args[i], "-al") == 0)
        {
            show_hidden = 1;  //Combinação -la ou -al
            long_format = 1;
        }
        else
            directory = args[i];  //Assume que é um diretório
    }
    
    DIR *dir = opendir(directory);  //Abre o diretório
    if (!dir)
    {
        perror("ls");  //Erro se não conseguir abrir
        return;
    }
    
    struct dirent *entry;  //Estrutura para entradas do diretório
    //Lê cada entrada do diretório
    while ((entry = readdir(dir)) != NULL)
    {
        //Pula arquivos ocultos se -a não foi especificado
        if (!show_hidden && entry->d_name[0] == '.')
            continue;
            
        if (long_format)  //Formato longo
        {
            struct stat file_stat;  //Informações do arquivo
            char full_path[1024];   //Caminho completo do arquivo
            
            //Constrói o caminho completo
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            
            //Obtém informações do arquivo
            if (stat(full_path, &file_stat) == 0)
            {
                //Imprime as permissões em formato simplificado
                printf("%c", (S_ISDIR(file_stat.st_mode)) ? 'd' : '-');  //Tipo (diretório ou arquivo)
                printf("%c", (file_stat.st_mode & S_IRUSR) ? 'r' : '-'); //Read user
                printf("%c", (file_stat.st_mode & S_IWUSR) ? 'w' : '-'); //Write user
                printf("%c", (file_stat.st_mode & S_IXUSR) ? 'x' : '-'); //Execute user
                printf("%c", (file_stat.st_mode & S_IRGRP) ? 'r' : '-'); //Read group
                printf("%c", (file_stat.st_mode & S_IWGRP) ? 'w' : '-'); //Write group
                printf("%c", (file_stat.st_mode & S_IXGRP) ? 'x' : '-'); //Execute group
                printf("%c", (file_stat.st_mode & S_IROTH) ? 'r' : '-'); //Read others
                printf("%c", (file_stat.st_mode & S_IWOTH) ? 'w' : '-'); //Write others
                printf("%c", (file_stat.st_mode & S_IXOTH) ? 'x' : '-'); //Execute others
                //Imprime número de links, tamanho e nome
                printf(" %ld %ld %s\n", file_stat.st_nlink, file_stat.st_size, entry->d_name);
            }
            else
            {
                printf("%s\n", entry->d_name);  //Se stat falhar, só imprime o nome
            }
        }
        else
        {
            printf("%s  ", entry->d_name);  //Formato simples com espaçamento
        }
    }
    
    if (!long_format)
        printf("\n");  //Nova linha no final se não for formato longo
        
    closedir(dir);  //Fecha o diretório
}

//cd (change directory)
void handle_cd(char **args, int num_args)
{
    if (num_args == 1)
    {
        return;  //Sem argumentos, não faz nada
    }
    
    else if (num_args == 2)
    {
        //Tenta mudar para o diretório especificado
        if (chdir(args[1]) != 0)
            perror("cd");  //Erro se chdir falhar
    }
    else
    {
        return;  //Muitos argumentos, não faz nada
    }
}

//Trata comandos built-in (internos do shell)
int handle_builtin_commands(char **args, int num_args, char *input)
{
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);  //Termina o programa
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        handle_cd(args, num_args);  //Executa cd
        return (1);  //Retorna 1 indicando que foi um built-in
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        handle_pwd();  //Executa pwd
        return (1);
    }
    else if (strcmp(args[0], "path") == 0)
    {
        handle_path(args, num_args);  //Executa path
        return (1);
    }
    else if (strcmp(args[0], "cat") == 0)
    {
        handle_cat(args, num_args);  //Executa cat
        return (1);
    }
    else if (strcmp(args[0], "ls") == 0)
    {
        handle_ls(args, num_args);  // Executa ls
        return (1);
    }

    return (0);  //Retorna 0 se não foi um built-in
}

//Função para redirecionar stdout para um arquivo
void redirecionar(char *arquivo)
{
    //Faz backup do stdout original se ainda não foi feito
    if (stdout_backup == -1)
        stdout_backup = dup(STDOUT_FILENO);
        
    //Abre o arquivo para escrita (cria se não existir, trunca se existir)
    fd = open(arquivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");  //Erro se não conseguir abrir
        return;
    }
    
    //Redireciona stdout para o arquivo
    dup2(fd, STDOUT_FILENO);
}

//Função para restaurar stdout original
void restaurar_stdout()
{
    //Se há backup do stdout
    if (stdout_backup != -1)
    {
        dup2(stdout_backup, STDOUT_FILENO);  //Restaura stdout original
        close(stdout_backup);               //Fecha o backup
        stdout_backup = -1;                 //Reset da variável
    }
    //Fecha o arquivo de redirecionamento se aberto
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

//pipe
void execute_pipe(char *input)
{
    char *commands[1024];  //Array para armazenar os comandos
    int num_commands = 0;  //Contador de comandos
    
    //Divide a entrada pelos pipes '|'
    char *token = strtok(input, "|");
    while (token && num_commands < 1024)
    {
        commands[num_commands] = token;  //Armazena cada comando
        num_commands++;
        token = strtok(NULL, "|");
    }
    
    //Precisa de pelo menos 2 comandos para fazer pipe
    if (num_commands < 2)
        return;
    
    int pipes[num_commands - 1][2];  //Array de pipes (cada pipe tem 2 descritores)
    pid_t pids[num_commands];        //Array de PIDs dos processos filhos
    
    //Cria todos os pipes necessários
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");  //Erro se não conseguir criar pipe
            return;
        }
    }
    
    //Executa cada comando em um processo separado
    for (int i = 0; i < num_commands; i++)
    {
        //Parse do comando atual
        char **args = malloc(sizeof(char*) * 1024);  //Array de argumentos
        int arg_count = 0;
        char *cmd_token = strtok(commands[i], " \n\t");  //Divide por espaços/tabs/newlines
        
        //Coleta todos os argumentos do comando
        while (cmd_token && arg_count < 1023)
        {
            args[arg_count] = cmd_token;
            arg_count++;
            cmd_token = strtok(NULL, " \n\t");
        }
        args[arg_count] = NULL;  //Termina o array com NULL
        
        //Se não há argumentos, pula este comando
        if (arg_count == 0)
        {
            free(args);
            continue;
        }
        
        pids[i] = fork();  //Cria processo filho
        
        if (pids[i] == 0)  //Código do processo filho
        {
            //Configura stdin (entrada)
            if (i > 0)  //Se não é o primeiro comando
            {
                //Recebe entrada do pipe anterior
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            //Configura stdout (saída)
            if (i < num_commands - 1)  //Se não é o último comando
            {
                //Envia saída para o próximo pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            //Fecha todos os descritores de pipe (não precisa mais deles)
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);  //Fecha lado de leitura
                close(pipes[j][1]);  //Fecha lado de escrita
            }
            
            //Verifica se é um comando built-in
            if (handle_builtin_commands(args, arg_count, commands[i]))
            {
                free(args);
                exit(0);  //Sai do processo filho
            }
            
            //Executa comando externo
            char *path = get_file_path(args[0]);  //Encontra o executável
            if (path)
            {
                execve(path, args, NULL);  //Executa o comando
                free(path);
            }
            
            perror("execve failed");  //Se chegou aqui, execve falhou
            free(args);
            exit(1);
        }
        
        free(args);  //Libera array de argumentos no processo pai
    }
    
    //Processo pai: fecha todos os pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipes[i][0]);  //Fecha lado de leitura
        close(pipes[i][1]);  //Fecha lado de escrita
    }
    
    //Espera todos os processos filhos terminarem
    for (int i = 0; i < num_commands; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);  //Espera o processo filho específico
    }
}

//Função principal para executar um comando (executada em thread separada)
void *execute_command(void *arg)
{
    char *input = (char *)arg;  //Comando a ser executado
    char *token;
    char *token2;
    size_t count = 0;
    ssize_t nread;
    pid_t child_pid;            //PID do processo filho
    int i, p, l, status;
    char **array;               //Array de argumentos do comando
    char **array2;              //Array para parsing de redirecionamento
    char **array3;              //Array para argumentos do arquivo de redirecionamento
    
    char *path;                 //Caminho do executável
    char *arquivo;              //Nome do arquivo para redirecionamento
    
    int num_args;               //Número de argumentos
    int has_redirection = 0;    //Flag para redirecionamento
    
    //Verifica se tem pipe no comando
    if (strchr(input, '|'))
    {
        execute_pipe(input);  //Executa pipe
        pthread_exit(0);      //Termina a thread
    }
    
    //Verifica se há redirecionamento de saída '>'
    if (strchr(input, '>'))
    {
        has_redirection = 1;
        token = strtok(input, ">");  //Divide pelo '>'

        array2 = malloc(sizeof(char*) * 1024);
        p = 0;

        //Coleta as partes antes e depois do '>'
        while (token)
        {
            array2[p] = token;
            token = strtok(NULL, ">");
            p++;
        }

        array2[p] = NULL;
        
        //Processa a parte depois do '>' (nome do arquivo)
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
                redirecionar(array3[0]);  //Redireciona para o arquivo
            }
            free(array3);
        }
        
        //Processa a parte antes do '>' (o comando)
        token = strtok(array2[0], " \n\t");
        free(array2);
    }
    else
    {
        //Sem redirecionamento, faz parsing normal
        token = strtok(input, " \n\t");
    }

    //Coleta todos os argumentos do comando
    array = malloc(sizeof(char*) * 1024);
    i = 0;

    while (token)
    {
        array[i] = token;
        token = strtok(NULL, " \n\t");
        i++;
    }

    array[i] = NULL;  //Termina com NULL
    num_args = i;     //Conta os argumentos

    //Se não há argumentos, não faz nada
    if (num_args == 0)
    {
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }

    //Verifica se é um comando built-in
    if (handle_builtin_commands(array, num_args, input))
    {
        free(array);
        if (has_redirection)
            restaurar_stdout();  //Restaura stdout se houve redirecionamento
        pthread_exit(0);
    }
    
    //Procura o executável no PATH
    path = get_file_path(array[0]);
    if (!path)
    {
        printf("Command not found: %s\n", array[0]);
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }
    
    child_pid = fork();  //Cria processo filho

    if (child_pid == -1)  //Erro no fork
    {
        perror("Failed to create process");
        free(path);
        free(array);
        if (has_redirection)
            restaurar_stdout();
        pthread_exit(0);
    }

    if (child_pid == 0)  //Processo filho
    {
        //Executa o comando
        if (execve(path, array, NULL) == -1)
        {
            perror("Failed to execute");
            exit(97);  //Código de erro específico
        }
    }
    else  //Processo pai
    {
        wait(&status);  //Espera o processo filho terminar
        printf("Process exited with status: %d\n", WEXITSTATUS(status));
    }
    
    //Cleanup
    free(path);
    free(array);
    if (has_redirection)
        restaurar_stdout();  //Restaura stdout se necessário
    
    pthread_exit(0);  //Termina a thread
}

//Função utilitária para remover um caractere específico de uma string
void removeChar(char *str, char garbage) 
{
    char *src, *dst;  //Ponteiros para origem e destino
    //Percorre a string copiando apenas caracteres que não são 'garbage'
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;  //Só avança destino se não for garbage
    }
    *dst = '\0';  //Termina a string
}

//executar comandos de um arquivo batch
void execute_batch_file(char *filename)
{
    FILE *file = fopen(filename, "r");  //Abre arquivo para leitura
    if (!file)
    {
        perror("Failed to open batch file");
        return;
    }
    
    char *line = NULL;    //Buffer para a linha lida
    size_t len = 0;       //Tamanho do buffer
    ssize_t read;         //Número de caracteres lidos
    
    //Lê o arquivo linha por linha
    while ((read = getline(&line, &len, file)) != -1)
    {
        //Remove o newline do final da linha
        if (line[read-1] == '\n')
            line[read-1] = '\0';
            
        printf("Executing: %s\n", line);  //Mostra qual comando está executando
        
        //Processa a linha igual ao modo interativo
        char *token = strtok(line, "&");  //Divide por '&' (comandos paralelos)
        char **array = malloc(sizeof(char*) * 1024);
        int i = 0;
        
        //Coleta todos os comandos separados por '&'
        while (token)
        {
            array[i] = strdup(token);  //Duplica para evitar problemas com strtok
            token = strtok(NULL, "&");
            i++;
        }
        
        array[i] = NULL;
        int num_threads = i;  //Número de threads = número de comandos
        
        //Se há comandos para executar
        if (num_threads > 0)
        {
            pthread_t workers[num_threads];  //Array de threads
            pthread_attr_t attr;
            pthread_attr_init(&attr);       //Inicializa atributos da thread
            
            //Cria uma thread para cada comando
            for (int j = 0; j < num_threads; j++)
            {
                pthread_create(&workers[j], &attr, execute_command, array[j]);
            }
            
            //Espera todas as threads terminarem
            for (int k = 0; k < num_threads; k++)
            {
                pthread_join(workers[k], NULL);
                free(array[k]);  //Libera strings duplicadas
            }
            
            pthread_attr_destroy(&attr);  //Destroi atributos
        }
        
        free(array);
    }
    
    //Cleanup
    if (line)
        free(line);
    fclose(file);
}

//Função principal do programa
int main(int argc, char *argv[])
{
    char *token;
    char **array;          //Array para armazenar comandos
    char *buf = NULL;      //Buffer para entrada do usuário
    int num_threads;       //Número de threads (comandos paralelos)
    size_t count = 0;      //Tamanho do buffer
    ssize_t nread;         //Número de caracteres lidos
    int i;
    
    //Verifica se foi passado um arquivo batch como argumento
    if (argc == 2)
    {
        execute_batch_file(argv[1]);  //Executa arquivo batch
        return 0;                     //Termina o programa
    }

    //Loop principal do shell interativo
    while(1)
    {
        //Mostra prompt apenas se entrada é um terminal (não pipe/redirecionamento)
        if (isatty(STDIN_FILENO))
            write(STDOUT_FILENO, "/> ", 3);
            
        //Lê uma linha da entrada
        nread = getline(&buf, &count, stdin);

        //Se getline falhou (EOF), termina
        if (nread == -1)
        {
            exit(1);
        }
        
        //Remove o newline do final
        if (buf[nread-1] == '\n')
            buf[nread-1] = '\0';
            
        //Divide a linha pelos '&' (comandos para executar em paralelo)
        token = strtok(buf, "&");
        array = malloc(sizeof(char*) * 1024);
        i = 0;
        
        //Coleta todos os comandos
        while (token)
        {
            array[i] = strdup(token);  //Duplica string para evitar problemas
            token = strtok(NULL, "&");
            i++;
        }

        array[i] = NULL;
        num_threads = i;  //Número de threads = número de comandos
        
        //Se há comandos para executar
        if (num_threads > 0)
        {
            pthread_t workers[num_threads];  //Array de threads
            pthread_attr_t attr;
            pthread_attr_init(&attr);       //Inicializa atributos
            
            //Cria uma thread para cada comando
            for (int j = 0; j < num_threads; j++)
            {
                pthread_create(&workers[j], &attr, execute_command, array[j]);
            }

            //Espera todas as threads terminarem
            for (int k = 0; k < num_threads; k++)
            {
                pthread_join(workers[k], NULL);  //Espera thread terminar
                free(array[k]);                  //Libera strings duplicadas
            }
            
            pthread_attr_destroy(&attr);  //Destroi atributos das threads
        }
        
        free(array);  //Libera array de comandos
    }
    
    //Cleanup final (nunca executado devido ao while(1), mas boa prática)
    if (custom_path)
        free(custom_path);  //Libera PATH customizado
    if (buf)
        free(buf);          //Libera buffer de entrada
    
    return 0;  //Retorna 0 indicando sucesso
}