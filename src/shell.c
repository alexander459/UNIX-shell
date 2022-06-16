#include"shell.h"
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include <stdio.h>
#include<fcntl.h>

/*used to implement the cd command. Returns 1 in success and 0 in failure*/
int cd(char *path){
  char home[100]="/home/";
  char *user_name;

  if(path==NULL){
    user_name=getlogin();
    path=strcat(home, user_name);
  }

  if(chdir(path)==-1)
    return 0;
  return 1;
}

/*removes the newline character from the line*/
void rm_newline(char *str){
  int i;
  for(i=0; str[i]!='\n'; i++);
  str[i]='\0';
  return;
}

/*takes a line and returns the arguments in a string array and the number of arguments
  The array will be NULL terminated*/
char **get_arguments(char *line, int *args){
  int words=count_words(line);
  int i, j, k;
  int bytes=1;
  int writting=0;

  /*create an array to store the words*/
  char **array=(char**)malloc(words*sizeof(char*));

  *args=words;
  if(words==0)
    return NULL;

  
  for(i=0; i<words; i++)
    array[i]=(char*)malloc(1);

  j=0;
  k=0;
  /*copy the words in the new array*/
  for(i=0; line[i]!='\0' && line[i]!=';'; i++){

    if(line[i]==' ' && !writting){
      continue;
    }else if(line[i]==' ' && writting){
      array[k]=realloc(array[k], bytes);
      array[k][j]=0;  /*NULL terminate the previous word*/
      k++; /*move to the next word*/
      j=0;  /*start from the begining*/
      writting=0;
      bytes=1;
    }

    if(line[i]!=' '){
      writting=1;
      /*allocate space to the column for storing the word*/
      array[k]=realloc(array[k], bytes);
      array[k][j]=line[i];
      j++;
      bytes++;
    }
  }

  if(writting){
    array[k]=realloc(array[k], bytes);
    array[k][j]=0;
  }

  /*allocate memmory for the arguments array +1 for the NULL pointer*/
  array=(char**)realloc(array, (words+1)*sizeof(char*));
  /*store all the arguments to the arguments array*/
  array[words]=NULL;  /*args array must be NULL terminated*/

  return array;
}

/*takes a line and counts the number of the words it contains.
Words are seperated with spaces*/
int count_words(char *line){
  int words=0;
  int i;
  int in_word=0;  /*bool. True if a word has been seen to avoid
  counting extra words with lines started by spaces*/

  if(line[0]=='\0')
    return 0;

  for(i=0; line[i]!=0 && line[i]!=';'; i++){
    if(line[i]!=' ' && !in_word){
      in_word=1; /*you are iterating now a word*/
      words++;
    }else if(line[i]==' ')
      in_word=0; /*now you are not iterating a word*/
  }
  return words;
}

/*takes a line with one or more substring seperated with <delimiter> and returns a pointer to the next 
  string of the line or NULL if there is no next string
  e_i is used to return by reference the index where the current sub string ends*/
char *delim_get_next(char *line, int *e_i, char delimiter){
  int i;
  int start_found, end_found;
  int end_index, start_index;
  char *part;

  start_found=0;
  end_found=0;
  i=-1;
  do{
    i++;
    if(!start_found){
      if(line[i]!=delimiter && line[i]!=' ' && line[i]!='\0'){
        start_index=i;
        start_found=1;
      }
    }else{
      if(line[i]=='\0' || line[i]==delimiter){  /*instruction terminates here*/
        if(!end_found){
          end_index=i-1;
          *e_i=i-1;
        }
        part=(char*)malloc((end_index-start_index)+2);
        part=strncpy(part, &line[start_index], (end_index-start_index)+1);
        part[(end_index-start_index)+1]='\0';
        return part;
      }else if(line[i]==' '){  /*maybe instruction terminates here*/
        end_index=i-1;
        *e_i=i-1;
        end_found=1;
      }else{                   /*instruction does not terminate here*/
        end_found=0;
      }
    }
  }while(line[i]!='\0');
  return NULL;
}


/*executes a simple command (without pipes or redirection)*/
int exec_simple(char **argv){
  int pid;
  pid=fork();
  if(pid<0)
    printf("fork failed\n");
  else if(pid>0)
    wait(NULL);
  else{
    /*execute the command*/
    if(execvp(argv[0], argv)==-1){
      printf("Can not execute command %s\n", argv[0]);
      exit(0);
    }
  }
  return 1;
}


/*takes an instruction with pipes and counts the commands it contains*/
int pipe_count_cmd(char *instruction){
  int counter=0;
  int on_cmd=0;
  int i;

  for(i=0; instruction[i]!=0; i++){
    if(!on_cmd && instruction[i]!=' ' && instruction[i]!='|'){
      on_cmd=1;
      counter++;
    }else if(instruction[i]=='|')
      on_cmd=0;
  }
  return counter;
}

/*returns 1 if the instruction uses pipes and 0 if not*/
int has_pipe(char *instruction){
  int i;
  for(i=0; instruction[i]!='\0'; i++){
    if(instruction[i]=='|')
      return 1;
  }
  return 0;
}


/*executes a pair of commands containing pipes*/
int exec_pipe(char *instruction){
  char **argv;
  char *command;
  int argc;
  int fd[2], pid;
  int cmd_count;  /*contains the number of the commands seperated with |*/
  int end_index;
  int i;

  int read_from=0;

  cmd_count=pipe_count_cmd(instruction);
  for(i=0; i<cmd_count; i++){
    /*get the command and create the argv*/
    command=delim_get_next(instruction, &end_index, '|');
    instruction=&instruction[end_index+1];
    argv=get_arguments(command, &argc);


    /*create the pipe*/
    pipe(fd);

    /*fork and run the command in argv*/
    pid=fork();
    if(pid<0){
      printf("fork failed\n");
      return 0;
    }
    if(pid==0){
      /*duplicate stdin with read from in order to read from the corect fd*/
      dup2(read_from, 0);

      /*if the instruction is not the last, then dont write ouutput in stdout but in pipe*/
      if(delim_get_next(instruction, &end_index, '|')!=NULL){
        dup2(fd[1], 1);
      }
      close(fd[0]);
      if(execvp(argv[0], argv)==-1){
        printf("Can not execute command %s\n", argv[0]);
        exit(0);
      }
    }else{
      wait(NULL);
      close(fd[1]);
      read_from=fd[0];  /*save the fd to read from the next command*/
    }
  }
  return 1;
}


/*returns '<' or '>' if the command contains redirection and 0 if not*/
char has_redirection(char *instruction){
  int i;
  for(i=0; instruction[i]!='\0'; i++){
    if((instruction[i]=='<' || instruction[i]=='>') && (instruction[i+1]!='>' && instruction[i-1]!='>'))
      return instruction[i];
  }
  return 0;
}


/*takes a line after redirection and returns the file name or NULL if there is no file name*/
char *get_fileName(char *instruction){
  int i;
  for(i=0; instruction[i]!=0; i++){
    if(instruction[i]!=' ' && instruction[i]!='<' && instruction[i]!='>')
      return &instruction[i];
  }
  return NULL;
}


/*executes a command containing pipes*/
int exec_rdr(char *instruction, char r){
  char *fileName;
  char *command;
  char **argv;
  int pid;
  int fd;
  int end_index, argc;
  /*the instruction is of type <cmd> '<'' or '>' <file>
  so there are 2 objects. the command with the arguments and a file name*/
  /*get the command and create the argv*/
  command=delim_get_next(instruction, &end_index, r);
  instruction=&instruction[end_index+1];
  argv=get_arguments(command, &argc);
  fileName=get_fileName(instruction);
  if(fileName==NULL){
    printf("there is no file name\n");
    return 0;
  }

  
  pid=fork();
  if(pid<0){
    printf("fork failed\n");
    return 0;
  }

  if(pid==0){
    if(r=='>'){   /*write output of command in file*/
      /*open a file to write in it*/
      fd=open(fileName, O_CREAT|O_WRONLY, 0777);
      if(fd==-1){
        printf("error openning %s\n", fileName);
        exit(0);
      }
      dup2(fd, 1);
      close(fd);
    }else{       /*use input from a file*/
      /*open a file to take input*/
      fd=open(fileName, O_RDONLY);
      if(fd==-1){
        printf("error openning %s\n", fileName);
        exit(0);
      }
      dup2(fd, 0);
      close(fd);
    }
    if(execvp(argv[0], argv)==-1){
      printf("Can not execute command %s\n", argv[0]);
      exit(0);
    }
  }else{
    wait(NULL);
  }
  return 1;
}


/*returns 1 if the command uses >> and 0 if not*/
int has_append(char *instruction){
  int i;
  for(i=0; instruction[i]!=0; i++){
    if(instruction[i]=='>' && instruction[i+1]=='>')
      return 1;
  }
  return 0;
}

/*takes a command with >> and returns the command aand the file name*/
char *apd_get_next(char *line, int *e_i){
  int i;
  int start_found, end_found;
  int end_index, start_index;
  char *part;

  start_found=0;
  end_found=0;
  i=-1;
  do{
    i++;
    if(!start_found){
      if(line[i]!='>' && line[i]!=' ' && line[i]!='\0'){
        start_index=i;
        start_found=1;
      }
    }else{
      if(line[i]=='\0' || (line[i]=='>' && line[i+1]=='>')){  /*instruction terminates here*/
        if(!end_found){
          end_index=i-1;
          *e_i=i-1;
        }
        part=(char*)malloc((end_index-start_index)+2);
        part=strncpy(part, &line[start_index], (end_index-start_index)+1);
        part[(end_index-start_index)+1]='\0';
        return part;
      }else if(line[i]==' '){  /*maybe instruction terminates here*/
        end_index=i-1;
        *e_i=i-1;
        end_found=1;
      }else{                   /*instruction does not terminate here*/
        end_found=0;
      }
    }
  }while(line[i]!='\0');
  return NULL;
}


/*executes a command containing append*/
int exec_apd(char *instruction){
  char *fileName;
  char *command;
  char **argv;
  int pid;
  int fd;
  int end_index, argc;
  /*there are 2 objects. the command with the arguments and a file name*/
  /*get the command and create the argv*/
  command=apd_get_next(instruction, &end_index);
  instruction=&instruction[end_index+1];
  argv=get_arguments(command, &argc);
  fileName=get_fileName(instruction);
  if(fileName==NULL){
    printf("there is no file name\n");
    return 0;
  }

  pid=fork();
  if(pid<0){
    printf("fork failed\n");
    return 0;
  }

  if(pid==0){
    fd=open(fileName, O_WRONLY|O_APPEND|O_CREAT, 0777);
    if(fd==-1){
      printf("error openning file %s\n", fileName);
      exit(0);
    }
    dup2(fd, 1);
    close(fd);
    if(execvp(argv[0], argv)==-1){
      printf("Can not execute command %s\n", argv[0]);
      exit(0);
    }
  }else{
    wait(NULL);
  }
  return 1;
}