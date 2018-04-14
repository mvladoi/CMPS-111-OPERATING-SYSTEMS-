
#include <stdio.h>
#include <sys/types.h> /* include necessary header files */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>






unsigned  total_char_count = 0;
unsigned  total_word_count = 0;
unsigned  total_line_count = 0;




//This function I used in my other assignmnets too and is inspired 
//from stack overflow. After testin unix wc, it seems that the function
//consider this kind of alphabet.

int my_is_alpha(int c)
{
    char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz"
                      "1234567890-=`~!@#$%^&*()_+"
                      "[]\{}|;':\",./<>?";
    char *letter = alphabet;

    while(*letter != '\0' && *letter != c)
        ++letter;

    if (*letter)
        return 1;

    return 0;
}




// 

void print_data(int open_file, char* file_name){

 //Initialize the variables 
   unsigned  char_count = 0;
   unsigned  word_count = 0;
   unsigned  line_count = 0;


 //Allocate the buffer
  const int buf_size = 2048;
  char* buffer = malloc(buf_size);

  if (!buffer) {
    perror("Error: malloc failed");
    exit(EXIT_FAILURE);
  }
  memset(buffer, 0, buf_size);


  //Read a segment of data
  ssize_t nr_bytes = 0;
  unsigned i = 0;
  int is_alpha = 0;
  int is_space = 0;


  //Count the number of lines, words and bytes
  while ((nr_bytes = read (open_file, buffer, sizeof (buffer))) > 0) {

    for (i = 0; i < nr_bytes; i++) {

     char_count++;


     if (buffer[i] == '\n') {
        line_count++;
      
      }


      if ((my_is_alpha(buffer[i])) && (!is_alpha)){
        word_count++;
        is_alpha = 1;
        is_space = 0;
      
      }



      if ((!my_is_alpha(buffer[i])) && (!is_space)){
        is_space = 1;
        is_alpha = 0;
        
      }


 }    
}


  //Compute the word count
  total_char_count += char_count;
  total_word_count += word_count;
  total_line_count += line_count;


  printf ("%u %6u %6u %s \n",  char_count, word_count, line_count, file_name);


  //Close the file 
  close(open_file);

  //Dealocate the buffer
  free (buffer);

}









int main (int argc, char **argv)
{
  //Initializa the variables
  int open_file = 0;
  int bool_stdin = 0;
         

  //Check if the number of argumets is ok
  if (argc == 1) {
    print_data(0, "");
    bool_stdin = 1;
  }

  
  unsigned j = 0;

 //Loop over all input file txt 
  for (j = 1; j < argc; j++){

  //Open the file
  open_file = open(argv[j], O_RDONLY);

  if (open_file < 0) {
    perror("Error: Can not open the file !");
    exit(EXIT_FAILURE);
  }

  

  print_data (open_file, argv[j]);
  
}




if (!bool_stdin && argc > 2)
printf ("%u %6u %6u total \n",  total_char_count, total_word_count, total_line_count);


return EXIT_SUCCESS;
}




