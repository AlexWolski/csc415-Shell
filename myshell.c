/****************************************************************
 * Name        : Alex Wolski                                    *
 * Class       : CSC 415                                        *
 * Date        : 10/20/18                                       *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFERSIZE 256
#define MAXPARAMS 5
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

int main(int* argc, char** argv)
{
  int numArgs;
  int pid;
  char** arguments = malloc(BUFFERSIZE);
  char* input = malloc(BUFFERSIZE);
  const char* delims = " \n";

  while (1)
  {
    //Reset the variables modified in the last loop. "arguments" is populated with null bytes.
    numArgs = 0;
    memset(arguments, 0, BUFFERSIZE);

    //Prompt the user for an input and read it
    printf("%s", PROMPT);
    fgets(input, BUFFERSIZE, stdin);

    //If the user pressed CTRL+D, exit the program
    if(feof(stdin))
    {
      printf("\n");
      exit(0);
    }

    //If the user enters "exit", exit the program
    if(strcmp(input, "exit\n") == 0)
      exit(0);

    //Set the first index of the arguments array to the command
    arguments[0] = strtok(input, delims);

    //Loop and populate the array with the arguments
    while(arguments[numArgs] != NULL)
    {
      numArgs += 1;
      arguments[numArgs] = strtok(NULL, delims);
    }

    //Execute the command
    if(fork() == 0)
      execvp(arguments[0], arguments);
    else
      wait(NULL);
  }
  
  return 0;
}
