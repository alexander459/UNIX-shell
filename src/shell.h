#define TRUE 1
#define FALSE 0

/*takes a line and counts the number of the words it contains.
Words are seperated with space*/
int count_words(char *line);

/*removes the newline character from the line*/
void rm_newline(char *str);

/*takes a line and returns the arguments in a string array and the number of arguments*/
char **get_arguments(char *line, int *words);

/*used to implement the cd command. Returns 1 in success and 0 in failure*/
int cd(char *path);

/*takes an instruction with pipes and counts the commands it contains*/
int pipe_count_cmd(char *instruction);



/*takes a line with one or more substring seperated with <delimiter> and returns a pointer to the next 
  string of the line or NULL if there is no next string
  e_i is used to return by reference the index where the current sub string ends*/
char *delim_get_next(char *line, int *e_i, char delimiter);

/*takes a line with two strings seperated with >> and returns a pointer to the next 
  string of the line or NULL if there is no next string
  e_i is used to return by reference the index where the current sub string ends*/
char *apd_get_next(char *line, int *e_i);

/*takes a line after redirection and returns the file name or NULL if there is no file name*/
char *get_fileName(char *instruction);



/*executes a spiple command (without pipes or redirection)*/
int exec_simple(char **argv);

/*executes a command containing pipes*/
int exec_pipe(char *instruction);

/*executes a command containing pipes*/
int exec_rdr(char *instruction, char r);

/*executes a command containing append*/
int exec_apd(char *instruction);



/*returns 1 if the instruction uses pipes and 0 if not*/
int has_pipe(char *instruction);

/*returns '<' or '>' if the command contains redirection and 0 if not*/
char has_redirection(char *instruction);

/*returns 1 if the command uses >> and 0 if not*/
int has_append(char *instruction);