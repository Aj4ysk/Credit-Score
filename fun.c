#include"main.h"
extern char prompt[100];
extern char backup_input_string[100];
extern char input_string[100];
extern char path[100];
extern int status;
extern pid_t pid;
extern char command[20];
int stopped_process_count=0;
typedef struct BACKUP
{
    pid_t pid_backup;
    char command_backup[20];
    struct BACKUP *link;
}backup;
backup *head = NULL;

char *get_command(char *input_string)
{
    //char *command = calloc(50,sizeof(char));
    int i=0;
    int j=0;
    
    //to exclude the space before command
    while(input_string[i] == ' ')
    {
        i++;
    }

    //till '\0' or space
    while(input_string[i] != '\0' && input_string[i] != ' ')
    {
        command[j++] = input_string[i++];
    }
    command[j] = '\0';
    return command;
}

int check_command_type(char *command)
{
    //validate whether it is builtin command or not
    for(int i=0;i<30;i++)
    {
        if(strcmp(builtins[i],command) == 0)
        {
            //if true
            return BUILTIN;
        }
    }
    //if not builtin command
    //validate whether it is external command or not
    for(int i=0;i<external_command_index;i++)
    {
        if(strcmp(external_commands[i],command) == 0)
        {
            //if true
            return EXTERNAL;
        }
    }

    //if not builtin & external, its not a command
    return NO_COMMAND;
}

void delete_at_last()
{
    //if head empty 
    if(head == NULL)
    {
        return;
    }
    
    //else single node
    if(head->link == NULL)
    {
        free(head);
        stopped_process_count--;
        head = NULL;
        return;
    }
    //traverse
    backup *temp = head;
    backup *backup_temp = head;
    while(temp->link != NULL)
    {
        backup_temp = temp;
        temp = temp->link;
    }
    backup_temp->link = NULL;       //update last node link part with NULL after reaching end
    
    //free the memory for deleted node
    free(temp);
    stopped_process_count--;

    return;
}

void fg()
{
    //check if head is NULL or not
    if(head == NULL)
    {
        printf("-minishell: fg current: no such job\n");
        return;
    }
    
    //traverse to the end of the list
    backup *temp = head;
    while(temp->link != NULL)
    temp = temp->link;
    
    //continue the last stopped process in the foreground
    printf("%s\n",temp->command_backup);
    kill(temp->pid_backup,SIGCONT);
    
    // waitpid(temp->pid_backup,&status,0);
    waitpid(temp->pid_backup,&status,WUNTRACED);
    delete_at_last();
}

void bg()
{
    if(head == NULL)
    {
        printf("-minishell: bg current: no such job\n");
        return;
    }

    //traverse to the end of the list
    backup *temp = head;
    while(temp->link != NULL)
        temp = temp->link;
    
    //continue the last stopped process in background
    kill(temp->pid_backup,SIGCONT);
    signal(SIGCHLD,signal_handler);

    // delete_at_last();
}

void execute_internal_commands(char *command,char *input_string)
{
    
    char *ptr;
    if(strcmp(command,"pwd") == 0)
    {
        printf("%s\n",getcwd(path,100));    //for command  pwd use getcwd function that return present working directory path
    }
    else if(strcmp(command,"cd") == 0)
    {
        ptr = strstr(input_string,"cd");
        while(*(ptr++) != '\0' && *ptr != ' '); //to avoid charcters in cd 
        while(*(ptr++) != '\0' && *ptr == ' '); //to avoid starting spaces after cd 
        ptr--;      //for false condition move back one index.

        //if space is found consider it as error
        char *token = strtok(ptr," ");
        int no_of_tokens = 0;
        
        while(token != NULL)
        {
            no_of_tokens++;
            token = strtok(NULL," ");
        }
        
        //if no_of tokens are grater then 1 then it is an invalid input prompt and return
        if(no_of_tokens > 1)
        {
            printf("-bash: cd: too many arguments\n");
            return;
        }

        //change the directory path of given input
        //meanwhile check for return value of chdir   -1 on error(error number)    0 on sucess
        if(chdir(strtok(ptr," ")))
        {
            //if chdir fails prompt the reason and return
            perror("chdir");
            return;
        }
    }
    //execute exit command
    else if(strcmp(command,"exit") == 0)
    {
        //simpley call exit function
        printf("\nlogout\n");
        exit(0);
    }
    //execute echo command
    else if(strcmp(command,"echo") == 0)
    {
        ptr = input_string;
        
        while(*ptr++ != '\0' && *ptr != ' ');   //move index to point after character of echo
        while(*ptr++ != '\0' && *ptr == ' ');   //move index next subcommand by avoiding the spaces
        ptr--;      //for false condition move back 1-index position

        //check for $$ using strtok
        //if true print pid of 
        if(strcmp(strtok(ptr," "),"$$") == 0)
        {
            printf("\n%d\n",getpid());
        }
        //if it is $? print the ststus of last command /process 
        else if(strcmp(strtok(ptr," "),"$?") == 0)
        {
            printf("\n%d\n",status);
        }
        //if it is $SHELL get the path for default shell i.e /bin/bash
        else if(strcmp(strtok(ptr," "),"$SHELL") == 0)
        {
            printf("\n%s\n",getenv("SHELL"));
        }
        else
        {
            printf("\n");
        }
    }
    //if it is jobs print the single linked list of all stopped process
    else if(strcmp(command,"jobs") == 0)
    {
        print_list();
    }
    else if(strcmp(command,"fg") == 0)
    {
        //continue
        //wait for coontinued process
        //prompt
        //delete last node of the list
        fg();
    }
    else if(strcmp(command,"bg") == 0)
    {
        //continue
        //prompt
        //delete last node of the list
        bg();
    }
}

void insert_at_last()
{
    //create a node
    backup *ptr = malloc(sizeof(backup));
    
    //update parts of nodes
    ptr->pid_backup = pid;
    strcpy(ptr->command_backup,backup_input_string);
    ptr->link = NULL;

    //if head is NULL then update the created node to Head
    if(head == NULL)
    {
        head = ptr;
    }
    //else travere and update
    else
    {
        //temperory for traversing
        backup *temp = head;

        //till reaching last node to access and update link of newly created node
        while(temp->link != NULL)
        {
            temp = temp->link;
        }

        //update last node link part
        temp->link = ptr;
    }
    return;
}

void print_list()
{
    //if head == NULL list is empty retun back
    if(head == NULL)
    {
        return;
    }
    //else travese and print respective node data
    else
    {
        backup *temp = head;
        int no_of_stopped_process=0;
        while(temp != NULL)
        {
            no_of_stopped_process++;
            if(temp->link != NULL)
                printf("[%d]\tstopped  \tpid: %d\tcommand: %s\n",no_of_stopped_process,temp->pid_backup,temp->command_backup);
            else
                printf("[%d]+\tstopped \tpid: %d\tcommand: %s\n",no_of_stopped_process,temp->pid_backup,temp->command_backup);
            temp = temp->link;
        }
    }
    return;
}

void signal_handler(int signum)
{
    if(signum == SIGINT)
    {
        //print prompt
        if(input_string[0] == '\0')
        {
            printf("\n");
            printf(ANSI_COLOR_GREEN"%s",prompt);
            printf(ANSI_COLOR_BLUE"%s",getcwd(path,100) );
            printf(ANSI_COLOR_RESET" ");
        }
        fflush(stdout);
        __fpurge(stdin);
    }
    else if(signum == SIGTSTP)
    {
        if(input_string[0] == '\0')
        {
            //print prompt
            printf("\n");
            printf(ANSI_COLOR_GREEN"%s",prompt);
            printf(ANSI_COLOR_BLUE"%s",getcwd(path,100) );
            printf(ANSI_COLOR_RESET" ");
        }

        //insert at last if input string not equal to '\0'
        if(input_string[0] != '\0')
        {
            stopped_process_count++;
            printf("\n[%d]+\tstopped \t%s\n",stopped_process_count,backup_input_string);
            insert_at_last();
        }
        __fpurge(stdin);
        fflush(stdout);
    }
    else if(signum == SIGCHLD)
    {
        int ret;
        if((ret=waitpid(-1,NULL,WNOHANG))>0)
        {

            backup *temp = head;
            while(temp->link != NULL)
            {
                temp = temp->link;
            }
            if(ret == temp->pid_backup)
            {    
                printf("\n[%d]-\tDone\tpid: %d\tcommand: %s",stopped_process_count,temp->pid_backup,temp->command_backup);
            //print prompt
            printf("\n");
            printf(ANSI_COLOR_GREEN"%s",prompt);
            printf(ANSI_COLOR_BLUE"%s",getcwd(path,100) );
            printf(ANSI_COLOR_RESET" ");
            __fpurge(stdin);
            fflush(stdout);
            delete_at_last();
            }
        }
    }
}

int validate_pipes(char *input_string)
{
    int i = 0;
    int len = strlen(input_string);

    // Check if string ends with a pipe
    i = len - 1;
    while (i >= 0 && input_string[i] == ' ')        //avoid all tailing spaces.
        i--;

    if (i >= 0 && input_string[i] == '|')        //check iput is ending with pipe or not
    {
        printf("Invalid: command ending with a pipe\n");
        return -1;
    }

    i = 0;
    while (input_string[i] != '\0')
    {
        if(input_string[i] == '|')
        {
            int j=i+1;
            while(input_string[j] != '\0' && input_string[j] == ' ')
                j++;
            
            if(input_string[j] == '|')
            {
                printf("Invalid: command with conseqcutive pipes\n");
                return -1;
            }
        }
        i++;
    }

    return 0; // Valid

}

void execute_external_commands(char *input_string)
{
    //first validate pipes if passed properly or not , in the input
    if(validate_pipes(input_string) == -1)
    {
        return;
    }   

    // to get no of pipes in the passed input
    int no_of_pipes=0;

    //loop to avoid leading spaces
    while(*input_string != '\0' && *input_string == ' ')
        input_string++;

    //2d array and index for arguments
    char *arguments[50];
    int argumentindex=0;

    //1D-array to hold  index of next coming aruguments in the 2D-array
    int next_argument[50] = {0};
    int next_argument_index=1;

    // Returns first token
    char *token = strtok(input_string," ");
 
    // Keep printing tokens while one of the
    // delimiters present in str[].
    while(token != NULL) 
    {
        if(strcmp(token,"|") == 0)
        {
            arguments[argumentindex++] = NULL;
            next_argument[next_argument_index++] = argumentindex;
            
            no_of_pipes++;  //meanwhile the the pipe count
        }
        else
        {
            //update the token to 2d array
            arguments[argumentindex++] = token;
        }
        token = strtok(NULL, " ");
    }
    arguments[argumentindex] = NULL;    //insert NUll at the end of 2d-array

    //verification code
    int i=0;
    int fd[2];

    //take backup of stdout and stdin fd
    int stdout_backup = dup(1);
    int stdin_backup = dup(0);
    int j=0;

    //commands without pipe
    if(no_of_pipes == 0)
    {
        //create a child process
        pid = fork();
        //parent
        if(pid > 0)
        {
            waitpid(pid,&status,WUNTRACED);
        }
        //child
        else if(pid == 0)
        {
            //handl signals with default functioanlities
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);

            //execute the command
            execvp(arguments[0],arguments+0);
            
            //if execvep fails prompt
            perror("execvp");
        }
        //error handling for fork
        else
        {
            perror("fork");
        }
        return;
    }

    //commands with any number of pipes
    for(int i=0; i < (no_of_pipes+1); i++)
    {
        //create a pipe
        if(i != no_of_pipes)
        { 
           if(pipe(fd))
            {
                perror("pipe");
                return;
            }
        }
       
        //create child
        pid = fork();      
        
        //parent
        if(pid > 0)
        {
            if(i != no_of_pipes)
            {
                //close the write end of pipes
                close(fd[1]);
                dup2(fd[0],0);
                close(fd[0]);
            }
            else
            {
                dup2(stdin_backup,0);   //replace stdin back in fd 0
            }
        }
        //child
        else if(pid == 0)
        {
            //handle siganls with default attributes in the child
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);
        
            //redirect output to next process if pipe is present
            if(i != no_of_pipes)
            {
                close(fd[0]);   //close read end.
                dup2(fd[1],1);  //redirect output to pipe
                close(fd[1]);   //close read end.
            }
            else
            { 
                dup2(stdout_backup,1);
            }
            // int execvp(char *file, char *const argv[]);
            execvp(arguments[next_argument[j]],arguments+next_argument[j]);        
            perror("execvp");
            return;
        }
        //validate if fork failed and terminate
        else
        {
            perror("fork"); 
            return;
        }
        j++;
        waitpid(pid,&status,0);
    }
}