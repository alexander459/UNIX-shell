#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stddef.h>
#include<stdlib.h>
#include"shell.h"

int main(){
  char *user_name; /*holds the username*/
  char *path=NULL;  /*holds the path for the working directory*/
  char *line;       /*used to take the shell inputs*/
  char **argv; /*contains the command and the arguments seperated in arrays of strings*/

  int argc; /*contains the number of arguments of a command (counting the command)*/
  int pid;
  int end_index;
  size_t line_size=10;
  char *command_pointer=NULL; /*points to the next instruction in case of multiple commands*/
  char *instruction, r;

  /*used to store the arguments for the instructions using pipes*/
  char *argv_p1;
  char *argv_p2;
  /*get username and working directory*/
  user_name=getlogin();
  path=getcwd(path, 1000);
  line=(char*)malloc(line_size);
  while(TRUE){
    /*print credentinals*/
    printf("%s@cs345sh%s$ ", user_name, path);

    /*read the instruction line*/
    getline(&line, &line_size, stdin);
    rm_newline(line);
    command_pointer=line;


    /*now execute all the commands in this line. Take every single instruction and execute it*/
    while((instruction=delim_get_next(command_pointer, &end_index, ';'))!=NULL){
      /*move the poiter to the next command*/
      command_pointer=&command_pointer[end_index+1];

      r=has_redirection(instruction);
      /*check if the instruction is simple*/
      if(!has_pipe(instruction) && !r && !has_append(instruction)){
        /*if there are no pipes prepere the arguments in argv and run the command*/
        argv=get_arguments(instruction, &argc);
        
        /*check the command and run it*/
        if(strcmp(argv[0], "cd")==0){
          if(cd(argv[1]))
            path=getcwd(path, 1000);
          else
            printf("No such a directory\n");
        }else if(strcmp(argv[0], "exit")==0)
          return 0;
        else{
          exec_simple(argv);
        }
      }

      /*if the instruction has pipes*/
      if(has_pipe(instruction)){
        if(!exec_pipe(instruction)){
          printf("Couldn't execute command %s piped with %s\n", argv_p1, argv_p2);
          return -1;
        }
      }

      /*if the instruction has redirection*/
      if(r){
        exec_rdr(instruction, r);
      }

      /*if the instruction has append*/
      if(has_append(instruction)){
        exec_apd(instruction);
      }
    }
  }
  return 0;
}