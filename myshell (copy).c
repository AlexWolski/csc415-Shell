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
    //Reset the "arguments" array by populating it with null bytes.
    memset(arguments, 0, BUFFERSIZE);

    if(getInput(PROMPT, input, BUFFERSIZE) == 0)
      exit(0);

    numArgs = parseString(input, arguments);

    execute(arguments);
  }
  
  return 0;
}

int getInput(char* prompt, char* input, int maxSize)
{
    //Prompt the user for an input and read it
    printf("%s", prompt);
    fgets(input, maxSize, stdin);

    //If the user pressed CTRL+D, exit the program
    if(feof(stdin))
    {
      printf("\n");
      return(0);
    }
    
    //If the user enters "exit", exit the program
    if(strcmp(input, "exit\n") == 0)
      return(0);

    return 1;
}

int parseString(char* string, char** output)
{
  int numTokens = 0;
  
    //Set the first index of the first token
    output[0] = strtok(string, delims);

    //Loop and populate the array with the tokens
    while(output[numArgs] != NULL)
    {
      numTokens += 1;
      output[numArgs] = strtok(NULL, delims);
    }

    return numTokens;
}

int execute(char** arguments)
{
    //Execute the command
    if(fork() == 0)
      execvp(arguments[0], arguments);
    else
      wait(NULL);
}
