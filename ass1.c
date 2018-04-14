#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern char **get_line();
int instr = 100; // nr of instruction to execut
int size = 1024; //  size of the files buffer



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  size_t nr_c_tokens;
  char** args;

} Instruction_component;





////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initialize_instruction_component(Instruction_component* my_instruction) {
  my_instruction-> nr_c_tokens = 0;
  my_instruction-> args = NULL;
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////
void insert_component_arg( Instruction_component* my_instruction_component , char* my_component_arg) {

  if (!my_instruction_component->args) {
    my_instruction_component->args = (char **)malloc(instr * sizeof(char*));
    if (!my_instruction_component->args) {
      perror("Error: malloc failed for my_instruction_component->args");
      exit(EXIT_FAILURE);
    }
  }


  size_t p = my_instruction_component->nr_c_tokens;

  my_instruction_component->args[p] = (char*)malloc(size * sizeof(char));
  if (!my_instruction_component->args[p]) {
    perror("Error: malloc failed for my_instruction_component->args[nr_c_tokens]");
    exit(EXIT_FAILURE);
  }


  if ((( strcmp(my_component_arg, "|")) == 0) || (( strcmp(my_component_arg, ";")) == 0)) {
    my_instruction_component->args[p] = NULL;
  }
  else {
    memcpy(my_instruction_component->args[p], my_component_arg, strlen(my_component_arg) + 1);
  }

  my_instruction_component-> nr_c_tokens++;

}







/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print_instruction_component(Instruction_component* my_instruction_component) {

  printf("The size of  instr_comp is %lu: \n", my_instruction_component->nr_c_tokens);
  for (int i = 0; my_instruction_component->args[i] != NULL; i++) {
    printf("Argument instr_comp %d: %s\n", i, my_instruction_component->args[i]);

  }

}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void free_instruction_component(Instruction_component* my_instruction_component) {

  if (my_instruction_component) {

    for (int i = 0; my_instruction_component->args[i] != NULL; i++) {
      free(my_instruction_component->args[i]);
      my_instruction_component->args[i] = NULL;

    }
    free(my_instruction_component);
    my_instruction_component = NULL;
  }


}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  size_t nr_simple_instr;
  Instruction_component** argv;
  char* out_file;
  char* in_file;
  char* err_file;
  int pipe;

} Instruction ;






/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the structure
void initialize_instruction(Instruction* my_instruction) {

  my_instruction->nr_simple_instr = 0;
  my_instruction->argv = NULL;
  my_instruction->out_file = NULL;
  my_instruction->in_file = NULL;
  my_instruction->err_file = NULL;
  my_instruction-> pipe = 0;

}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int is_file(char* input){

  struct stat my_stat;
  int ret = 0;
  ret = stat(input, &my_stat );
  if ( ret!= 0)
    return 0;
  if( S_ISREG(my_stat.st_mode) ) 
    return 1;
  return 0;

}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void insert_instruction_components(Instruction* my_instruction, char** input) {

  int a = 0;
  size_t p = 0;

  for (int i = 0; input[i] != NULL; i++) {

    if (!a) {

      my_instruction->argv[p] = (Instruction_component *)malloc(sizeof(Instruction_component));

      if (! my_instruction->argv[p]) {
        perror("Error: malloc failed for my_instruction->arv[p]");
        exit(EXIT_FAILURE);

      }
      initialize_instruction_component(my_instruction->argv[p]);

      a = 1;

    }

    if ((( strcmp(input[i], "<")) == 0) ||  (( strcmp(input[i], ">")) == 0)) {
      if ((my_instruction->in_file) || (my_instruction->out_file))
        i++;
      continue;
    }


    if (( strcmp(input[i], "|")) == 0) {
      my_instruction-> pipe = 1;
    }

    insert_component_arg(my_instruction->argv[p], input[i]);

    if ((( strcmp(input[i], "|")) == 0) || (( strcmp(input[i], ";")) == 0)) {
      my_instruction->nr_simple_instr++;
      p++;
      a = 0;

    }





  }
  my_instruction->nr_simple_instr++;
  insert_component_arg(my_instruction->argv[p], "|");

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_instruction_files(Instruction* my_instruction, char** input) {


  for (int i = 0; input[i] != NULL; i++) {


    if (( strcmp(input[i], ">")) == 0) {

      if (input[i + 1]) {

        my_instruction->out_file  = (char *)malloc(size * sizeof(char*));
        memcpy(my_instruction->out_file, input[i + 1], strlen(input[i + 1]) + 1);

      }

      else {
        perror("Error: enter a file name for input");
        exit(EXIT_FAILURE);
      
      }
      
      i++;

    }


    if (( strcmp(input[i], "<")) == 0) {

      if (input[i + 1]) {


        my_instruction->in_file = (char *)malloc(size * sizeof(char*));
        if (!my_instruction->in_file) {
          perror("Error: malloc failed for my_instruction->in_file");
          exit(EXIT_FAILURE);
        }

        memcpy(my_instruction->in_file, input[i + 1], strlen(input[i + 1]) + 1);
      
      }
      else {
        perror("Error: enter a file name for output");
        exit(EXIT_FAILURE);
      }

      i++;


    }

  }

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set the instruction to the right parameters, allocating memory on the heap
void  set_instruction(Instruction* my_instruction, char** input) {


  my_instruction->argv = (Instruction_component **)malloc(instr * sizeof(Instruction_component*));
  if (!my_instruction->argv) {
    perror("Error: malloc failed for my_instruction->argv");
    exit(EXIT_FAILURE);
  }



  set_instruction_files(my_instruction, input);
  insert_instruction_components(my_instruction, input);
 
}








/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Print the instruction values, used for debugging

void print_instruction(Instruction* my_instruction) {

  printf("nr_simple_instr %lu: \n", my_instruction->nr_simple_instr);
  printf("pipe: %d: \n", my_instruction->pipe);

  if (my_instruction->in_file) {
    printf("in_file: %s: \n", my_instruction->in_file);
  }

  if (my_instruction->out_file) {
    printf("out_file: %s: \n", my_instruction->out_file);
  }


  for (int i = 0; my_instruction->argv[i] != NULL; i++) {
    printf("Argument %d: \n", i);
    print_instruction_component(my_instruction->argv[i]);
  }

}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void free_instruction (Instruction* my_instruction) {

  if (my_instruction->in_file) {
    free(my_instruction->in_file);
    my_instruction->in_file = NULL;
  }


  if (my_instruction->out_file) {
    free(my_instruction->out_file);
    my_instruction->out_file = NULL;
  }


  if (my_instruction) {

    for (int i = 0; my_instruction->argv[i] != NULL; i++) {
      free_instruction_component(my_instruction->argv[i]);
      my_instruction->argv[i] = NULL;

    }
    
  }



}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Fuction to print the current directory, user name and promt message
void show_prompt() {

printf("%s: @myshell# ", getenv("USER"));
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void save_in_out(int* store_in, int* store_out) {

  *store_in = dup(0);
  *store_out = dup(1);

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_instr_input(Instruction* my_instruction, int* process_in, int* store_in) {

  if (my_instruction->in_file) {
    *process_in = open(my_instruction->in_file, O_RDONLY);
  }

  else {
    *process_in = dup(*store_in);
  }

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void redirect_instr_input(int* process_in) {
  dup2(*process_in,0);
  close(*process_in);

}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_instr_output(Instruction* my_instruction, int* process_out, int* store_out) {

  if (my_instruction->out_file) {
    *process_out = open (my_instruction->out_file, O_WRONLY | O_APPEND);
  }
  else {
    *process_out = dup(*store_out);
  }

}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void make_pipe(int* instr_pipe, int* process_out, int* process_in) {
  pipe(instr_pipe);
  *process_out = instr_pipe[1];
  *process_in = instr_pipe[0];
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void redirect_instr_output(int* process_out) {
  dup2(*process_out,1);
  close(*process_out);

}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void load_in_out(int* store_in, int* store_out) {
  dup2(*store_in, 0);
  dup2(*store_out, 1);
  close(*store_in);
  close(*store_out);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void execute_cd (Instruction* my_instruction){

  int i = my_instruction->nr_simple_instr - 1;
  if ( my_instruction->argv[i]->nr_c_tokens > 3 ) {
    perror("Error: extra arguments for cd");
    return;
  }

  char * path =NULL; 
  if (my_instruction->argv[i]->nr_c_tokens == 3 ) 
    path = my_instruction->argv[i]->args[1];
  else 
    path = getenv("HOME");

  


  if (chdir(path) == 0 ) 
    return;

  
  perror( "Error: No directory with this name" );
 
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void create_out_file(char* file);




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void execute_instruction(Instruction* my_instruction) {


  if (( strcmp(my_instruction->argv[0]->args[0], "exit")) == 0) {
    free_instruction(my_instruction);
    exit(EXIT_SUCCESS);
  }

  if (( strcmp(my_instruction->argv[0]->args[0], "cd")) == 0) {
    execute_cd(my_instruction);
    return;
  }

  if ( (my_instruction->in_file) && (!is_file(my_instruction->in_file))){
    perror("Error: ");
    return;
  }

  if (( my_instruction-> in_file) && (!is_file(my_instruction->out_file))){
    create_out_file(my_instruction->out_file);
  }


  pid_t fork_instr, wpid;
  int status;
  int store_in, store_out, process_in, process_out;
  int instr_pipe[2];

  save_in_out(&store_in, &store_out);
  set_instr_input(my_instruction, &process_in, &store_in);


  for (size_t i = 0; i < my_instruction->nr_simple_instr; i++) {

    redirect_instr_input(&process_in);

    if ( i == my_instruction-> nr_simple_instr - 1) {
      set_instr_output(my_instruction, &process_out, &store_out);
    }

    else {

      make_pipe(instr_pipe, &process_out, &process_in);
    }


    redirect_instr_output(&process_out);


    fork_instr = fork();



    if (fork_instr == 0) {


      execvp(my_instruction->argv[i]->args[0], my_instruction->argv[i]->args);
      perror("execvp error");
      exit(EXIT_FAILURE);
    }

    else if (fork_instr < 0) {
      perror ("fork error");
      exit(EXIT_FAILURE);
    }

  }


  load_in_out(&store_in, &store_out);



  wpid = waitpid(fork_instr, &status, WUNTRACED);


}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void create_out_file(char* file){

Instruction my_instruction;
char* input [3];
input [0] = "touch";
input [1] = file;
input [2] = NULL;
initialize_instruction(&my_instruction);
set_instruction(&my_instruction,input);
execute_instruction(&my_instruction);
free_instruction(&my_instruction);


}








//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
  
  Instruction my_instruction;
  char** input; 

  while (1) {
    show_prompt();

    input = get_line(); 

    if ((input) && input[0]){ 

      initialize_instruction(&my_instruction);

      set_instruction(&my_instruction, input);
        
      //print_instruction(&my_instruction); //for debugging

      execute_instruction(&my_instruction);

      free_instruction(&my_instruction);
   }
  }

}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////




