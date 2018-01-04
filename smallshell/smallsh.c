#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


void parseLine(char*,char* []);
void printCommandLine(char* cmd, char* args[]);
int isBackground(char* args[]);
void haveEnded(int);
void statusCommand(void);
void exitCommand(void);
void cdCommand(char* command,char* arguments[]);
void notBuiltInCommand(char* command,char* arguments[]);
void execBackground(char* command, char* arguments[]);
void execForeground(char* command, char* arguments[]);
void getInput(char* input, char* arguments[], char* line[]);
int pop_bg();
void trap_a_sig(int _);
void cleanup(char*);
bool is_bg(int pid);
void checkProcesses(void);
void IntHandler(int signo);
void catchKill();
void killHandler(int);
void sig_term_handler(int signum);

int background = 0;
int processes[112];
int status_code;
int quit_code;
int isNullInput = 0;
int count = 0;
const char dev[] = "/dev/null";
char *output = NULL;
volatile sig_atomic_t foregroundOnlyMode = 0;

void trap_a_sig(int sig)
{
    int child_status;
    pid_t child;
    
    signal(sig, SIG_IGN);
    
    while ((child = pop_bg()))
    {
        kill(child, SIGKILL);
        waitpid(child, &child_status, 0);
    }
    
    printf("terminated by signal %i\n",sig);
    
    signal(SIGINT, trap_a_sig);
    
    return;
}

int pop_bg()
{
    if (processes > 0)
    {
        background--;
        return processes[background + 1];
    }
    else
        return 0;
}

void killHandler(int signum)
{
    int child;
    pid_t pid = waitpid(0, &child, 0);
    int found = 0;
    int i;
    
    for (i = 0; i < background; i++)
    {
        if (pid == processes[i])
        {
            found = 1;
        }
    }
    
    if (found == 1)
    {
        printf("background pid %d is done: exit value %d ",pid, signum);
    }
}

//sourced and modified from piazza discussion
void sig_term_handler(int signum)
{
    signal(SIGTSTP, SIG_IGN);
    
    if (foregroundOnlyMode == 1)
    {
        printf("\nExiting foreground only mode.\n");
        foregroundOnlyMode = 0;
    }
    else
    {
        printf("\nEntering foreground only mode.\n");
        foregroundOnlyMode = 1;
    }
    
    signal(SIGTSTP, sig_term_handler);
    
    return;
}

int main(int argc, char *argv[], char *envp[])
{
    signal(SIGTERM, killHandler);
    signal(SIGINT, trap_a_sig);
    signal(SIGCHLD, haveEnded);
    signal(SIGTSTP, sig_term_handler);
    
    char *input = NULL;
    char *line[2048];
    size_t inputChars = 0;
    size_t bufferSize = 0;
    
    char *command = NULL;
    char *arguments[2048];
    

    while(quit_code == 0){
        
        fflush(stdout);
        fflush(stdin);
        printf(": ");
        fflush(stdin);
        
        inputChars = getline(&input, &bufferSize , stdin);
        
            command = NULL;
        
            count = 1;
        
            memset(line, '\0' , sizeof(line));
            memset(arguments, '\0', sizeof(arguments));
        
        
        
            if(inputChars == -1)
            {
                clearerr(stderr);
                return 0;
            }
    
            parseLine(input,line);
        
            command = line[0];
        
            int i = 0;
            int j = 1;
        
            while(line[j] != NULL)
            {
                arguments[i] = line[j];
                i++;
                j++;
            }
        
            if((*line[0] != '#') && (*line[0] != '\n'))
            {
                
                if (strcmp(command, "status") == 0)
                {
                    if (WIFEXITED(status_code))
                        printf("Exited: %d\n", WEXITSTATUS(status_code));
                    if (WIFSIGNALED(status_code))
                        printf("Stop signal: %d\n", WSTOPSIG(status_code));
                    if (WTERMSIG(status_code))
                        printf("termination signal: %d\n",
                               WTERMSIG(status_code));
                }
                else if (strcmp(command, "exit") == 0)
                {
                    trap_a_sig(-1);
                    quit_code = 1;
                    exit(1);
                }
                else if (strncmp("cd", command, strlen("cd")) == 0)
                {
                    if (arguments[0] != NULL)
                    {
                        char cwd[1024];
                        getcwd(cwd, sizeof(cwd));
                        
                        sprintf(cwd, "%s/%s",cwd,arguments[0]);
                        
                    }
                    else{
                        char *directory = getenv("HOME");
                        chdir(directory);
                    }
                }
                else if (isBackground(arguments))
                {
                    pid_t pid;
                    int redirect = 0;
                    char *execArgs[512];
                    memset(execArgs, '\0', sizeof(execArgs));
                    
                    int pos = 0;
                    
                    while (arguments[pos] != NULL)
                    {
                        pos++;
                    }
                    
                    arguments[pos - 1] = NULL;
                    pos = 0;
                    
                    while (arguments[pos] != NULL)
                    {
                        execArgs[pos + 1] = arguments[pos];
                        pos++;
                    }
                    
                    execArgs[0] = command;
                    
                    if((pid = fork()) < 0)
                    {
                        perror("Fork Error");
                        exit(1);
                    }
                    if(pid == 0)
                    {
                        signal(SIGTSTP, SIG_DFL);
                        
                        redirect = 0;
                        int i = 0;
                        
                        while (arguments[i] != NULL)
                        {
                            int fd = open(dev,O_RDWR);
                            dup2(fd, STDIN_FILENO);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                            
                            if ((strcmp(execArgs[i], "<") == 0))
                            {
                                if ((access(execArgs[i + 1], R_OK) == -1))
                                {
                                    printf("Cannot open %s for input\n", execArgs[i + 1]);
                                    fflush(stdout);
                                    exit(1);
                                }
                                else
                                {
                                    
                                    int fd = -1;
                                    fd = open(execArgs[ i + 1], O_RDONLY, 0);
                                    if(fd == -1)
                                    {
                                        fd = open(dev, O_RDWR);
                                    }
                                    dup2(fd, STDIN_FILENO);
                                    close(fd);
                                    
                                    redirect = 1;
                                    
                                }
                                
                            }
                            if ((strcmp(execArgs[i], ">") == 0))
                            {
                                
                                int fd = -1;
                                fd = creat(execArgs[ i + 1], 0644);
                                if(fd == -1)
                                {
                                    fd = open(dev, O_RDWR);
                                }
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                                redirect = 1;
                                
                                
                            }
                            
                            i++;
                            
                        }
                        if (redirect == 0)
                        {
                            execvp(command, execArgs);
                        }
                        else if((redirect = 1))
                        {
                            execvp(command,execArgs);
                        }
                        printf("%s no such file or directory.\n", command);
                        fflush(stdout);
                        exit(1);
                    }
                    else{
                        if(foregroundOnlyMode == 0)
                        {
                            int status;
                            printf("background pid is %d\n",pid);
                            fflush(stdout);
                            processes[background] = pid;
                            background ++;
                            waitpid(pid, &status, WNOHANG);
                        }
                        else
                        {
                            int status;
                            waitpid(pid, &status, 0);
                        }
                    
                    }
                }
                else{
              
                    
                        pid_t pid;
                        int redirect = 0;
                        char *execArgs[512];
                        memset(execArgs, '\0', sizeof(execArgs));
                        char* argumentsCopy[512];
                        memset(argumentsCopy, '\0', sizeof(argumentsCopy));
                        
                       
                        
                        int pos = 0;
                        
                        while (arguments[pos] != NULL)
                        {
                            argumentsCopy[pos] = arguments[pos];
                            execArgs[pos + 1] = arguments[pos];
                            pos++;
                        }
                        
                        execArgs[0] = command;
                        
                        
                        if((pid = fork()) < 0)
                        {
                            perror("Fork Error");
                            exit(1);
                        }
                        if(pid == 0)
                        {
                            signal(SIGTSTP, SIG_DFL);
        
                            redirect = 0;
                            int i = 0;
            
                            
                            while (arguments[i] != NULL)
                            {
                                

                                if ((strcmp(execArgs[i], "<") == 0))
                                {
                                    if ((access(execArgs[i + 1], R_OK) == -1))
                                    {
                                        printf("Cannot open %s for input\n", execArgs[i + 1]);
                                        fflush(stdout);
                                        exit(1);
                                    }
                                    else
                                    {
                                        
                                        int fd = -1;
                                        fd = open(execArgs[ i + 1], O_RDONLY, 0);
                                        if(fd == -1)
                                            break;
                                        else
                                        {
                                            dup2(fd, STDIN_FILENO);
                                            close(fd);
                                        }
                                        redirect = 1;
                                        
                                    }
                                    
                                }
                                if ((strcmp(execArgs[i], ">") == 0))
                                {
                                
                                    int fd = -1;
                                    fd = creat(execArgs[ i + 1], 0644);
                                    if (fd == -1)
                                       break;
                                    else
                                    {
                                    dup2(fd, STDOUT_FILENO);
                                    close(fd);
                                    redirect = 1;
                                    
                                    }
                                }
                                
                                i++;
                              
                            }
                            if (redirect == 0)
                            {
                                
                                execvp(command, execArgs);
                            }
                            else if((redirect = 1))
                            {
                                execvp(command, &argv[0]);
                            }
                            printf("%s no such file or directory.\n", command);
                            fflush(stdout);
                            exit(0);
                        }
                        else{
                            
                            int status;
                            
                            waitpid(pid, &status, 0);
                            
                            if (WIFEXITED(status))
                            {
                                status_code = WEXITSTATUS(status);
                            }
    
                       
                        }
 
                    }
                
                
                }
        
    
            }
    
    return 0;
}


    


void parseLine(char *input,char* newInput[2048])
{
    char* wordBuffer;
    int count = 0;
    
    strtok(input,"\n");
    wordBuffer = strtok(input, " ");
    newInput[0] = wordBuffer;
    
    while (wordBuffer != NULL)
    {
        newInput[count] = wordBuffer;
        wordBuffer = strtok(NULL, " ");
        count++;
    }
    
    
}

int isBackground(char* words[])
{
    int i = 0;
    int pos = 0;
    
    if (words[pos] == NULL)
    {
        return false;
    }
    while (words[i] != NULL)
    {
        pos++;
        i++;
    }
    if (*words[pos - 1] == '&')
    {
        return true;
    }
    else return false;
}


void printCommandLine(char* cmd, char* values[])
{
    
    printf("Command: %s\n",cmd);
    fflush(stdout);
    
    printf("Arguments:\n");
    fflush(stdout);
    
    int i = 0;
    
    while (values[i] != NULL)
    {
        printf("%s\n",values[i]);
        fflush(stdout);
        i++;
    }
}

bool is_bg(int pid)
{
    int i;
    
    for (i = 0; i < background; i++)
    {
        if (pid == processes[i])
        {
            return true;
        }
    }
    
    return false;
    
}

void haveEnded(int proc){
    
    int child;
    pid_t pid = waitpid(0, &child, 0);
    int found = 0;
    int i;
    
    for (i = 0; i < background; i++)
    {
        if (pid == processes[i])
        {
            found = 1;
        }
    }
    
    if (found == 1)
    {
        printf("\nbackground pid %d is done: exit value %d\n: ",pid,WEXITSTATUS(child));
    }
    
   
    
}









