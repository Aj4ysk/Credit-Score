#include"main.h"
//list of builtin commands
char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
    "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
    "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help","jobs","fg","bg", NULL};
    //for storing all  external commands
char *external_commands[154];
int external_command_index=0;

char prompt[100] = "MINI_SHELL:";
char path[100];
int status;
char two_d_array[25][60];
int two_d_array_index;

char input_string[100];
char backup_input_string[100];

pid_t pid;
char command[20];
int main()
{
    //change the default signal behaviour to own signal handler function
    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);
    
    //clear the screen
    system("clear");
    
    //declare and initiallize default prompt.
    // char prompt[100] = "MINI_SHELL:";
    
    //extract_external_command into a 2d array using function call
    extract_external_commands(external_commands);
    

    //call scan input function
    scan_input(prompt, input_string);
}

void extract_external_commands(char **external_commands)
{    
    //open the file with external commands
    int fd = open("external_commands.txt",O_RDONLY);
    if(fd == -1)
    {
        perror("open");
        return;
    }
    
    char ch;
    char temp_command[100];
    int temp_command_index=0;
    
    //read from the file
    while(read(fd,&ch,1) )
    {
        if(ch == '\n' )
        {
            temp_command[temp_command_index-1] = '\0';  //insert '\0' at the end 
            external_commands[external_command_index] = calloc((temp_command_index+1),sizeof(char));    //dynamicaly create the 1d array 
            strcpy(external_commands[external_command_index],temp_command); //update the 2d array external_commands
            
            external_command_index++;   //meanwhile get the count of commands that are extracted using this index variable       
            temp_command_index=0;
        }
        else
        {
            //if it is characater update into that array 
            temp_command[temp_command_index++] = ch;
        }
    }
    //after reaching end of the file last string/command
    temp_command[temp_command_index] = '\0';
    
    //handle the last command after reaching \n or EOF
    external_commands[external_command_index] = calloc((temp_command_index+1),sizeof(char));
    strcpy(external_commands[external_command_index],temp_command);
    external_command_index++;
}


void scan_input(char *prompt, char *input_string)
{
    while(1)
    {
        //print prompt
        printf(ANSI_COLOR_GREEN"%s",prompt);
        printf(ANSI_COLOR_BLUE"%s",getcwd(path,100) );
        printf(ANSI_COLOR_RESET" ");
        
        //clear input string
        memset(input_string,'\0',strlen(input_string));
        //read input from user
        scanf("%[^\n]",input_string);
        strcpy(backup_input_string,input_string); 
        
        getchar();  //to handling lefte over characters in stdin buffer 
        if (input_string[0] == '\0')
        {
            continue;
        }

        if(input_string[0] == 'P' && input_string[1] == 'S' && input_string[2] == '1' && input_string[3] == '=')
        {
            int command_type;
            
            int flag=0;
            char *temp = input_string;
            while(*temp)
            {
                if(*temp == ' ')    //check for spaces if it is PS1
                {
                    flag=1;
                    break;
                }
                temp++;
            }
            if(flag)
            {
                printf("%s: command not found\n",input_string);
            }
            else
            {
                strcpy(prompt,input_string+4);
                strcat(prompt,":");
            }
        }
        else  //if PS1 not present validate the commands 
        {
            //get the first comand
            strcpy(command, get_command(input_string));
            
            //check the command type
            int command_type = check_command_type(command);
            
            if(command_type == BUILTIN)
            {
                execute_internal_commands(command,input_string);
            }
            else if(command_type == EXTERNAL)
            {
                //for handle "clear" command
                if(strcmp(command,"clear") == 0)
                {
                    system("clear");
                    continue;
                }
                execute_external_commands(input_string);
            }
            else
            {
                printf("%s: command not found\n",command);
            }        
        } 
    }
}