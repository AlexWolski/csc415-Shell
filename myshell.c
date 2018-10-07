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
  char** arguments = malloc(BUFFERSIZE);
  char* input = malloc(BUFFERSIZE);
  cosnt char* delims = " \n";

  while (1)
  {
    numArgs = 0;
    memset(arguments, 0, BUFFERSIZE);
    
    printf("%s", PROMPT);
    fgets(input, BUFFERSIZE, stdin);

    if(feof(stdin))
    {
      printf("\n");
      exit(0);
    }
    
    if(strcmp(input, "exit\n") == 0)
      exit(0);

    arguments[0] = strtok(input, delims);
    
    while(arguments[numArgs] != NULL)
    {
      numArgs += 1;
      arguments[numArgs] = strtok(NULL, delims);
    }
  }
  
  return 0;
}
