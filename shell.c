#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
/**
  * get_file_path - Get's the full path of the file
  * @file_name: Argument name
  *
  * Return: The full path argument to the file
  * 
  */

  
  
  int startsWithForwardSlash(const char *str)
  {
          if (str != NULL || str[0] == '/')
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
      
          char *path = getenv("PATH");
          char *full_path;
          
          if (startsWithForwardSlash(file_name) && access(file_name, X_OK) == 0)
              return (strdup(file_name));
  
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
  void handle_cd(char **args, int num_args)
{
	
	if (num_args == 1)
	{
		return ;
	}
	
	else if (num_args==2)
	{
		if (chdir(args[1]) != 0)
			perror("cd");
	}
        else{
                return ;
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
	

	return (0);
}
void *execute_command(char *input){
        char *token;
        size_t count = 0;
        ssize_t nread;
        pid_t child_pid;
        int i, status;
        char **array;
        char *path;
        int num_args;
        token = strtok(input, " \n");

                array = malloc(sizeof(char*) * 1024);
                i = 0;

                while (token)
                {
                        array[i] = token;
                        token = strtok(NULL, " \n");
                        i++;
                }

                array[i] = NULL;
                num_args=i;
                if (handle_builtin_commands(array, num_args, input) == 1)
                return ;
                path = get_file_path(array[0]);
                child_pid = fork();

                if (child_pid == -1)
                {
                        perror("Failed to create.");
                        exit (41);
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
                }
        
        pthread_exit(0);
        
        //free(path);
        

  }

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
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
       
        

    while(1){
        if (isatty(STDIN_FILENO))
        write(STDOUT_FILENO, "/> ", 3);
        nread = getline(&buf, &count, stdin);

                if (nread ==  -1)
                {
                      
                        exit(1);
                }
        token = strtok(buf, "&");

        array = malloc(sizeof(char*) * 1024);
        i = 0;
        
        while (token)
        {
         array[i] = token;
         token = strtok(NULL, "&");
         i++;
        }
        
        array[i] = NULL;
        num_threads=i;
        pthread_t workers [num_threads];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
       
       
        for (int j=0;j<num_threads;j++){
        pthread_create(&workers[i],&attr,execute_command,array[j]);
     
       
    
           
    
    
        }

        for (int k=0;k<num_threads;k++){
   
                pthread_join(workers[i],NULL);
               
            
                   
            
            
                }
        //free(buf);

        }
        
        
    
    return 0;
}