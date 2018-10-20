/****************************************************************
 * Name        : Alex Wolski                                    *
 * Class       : CSC 415                                        *
 * Date        : 10/20/18                                       *
 * Description : Writting a simple bash shell program           *
 *               that will execute simple commands. The main    *
 *               goal of the assignment is working with         *
 *               fork, pipes and exec system calls.             *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>

#define BUFFERSIZE 256
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)
#define MAXARGS

struct command
{
  char*** commandTable;
  char* inputFile;
  char* outputFile;
  int append;
  int background;
};

int getInput(char* prompt, char* input, int maxSize);
int parseCommands(char* string, struct command* allCommands, int commandSize);
void execute(struct command* allCommands, int numCommands);
void printWorkingDirectory();
void changeDirectory(char* destination);

int main(int* argc, char** argv)
{
  //Store what the user inputs
  char* input = malloc(BUFFERSIZE);
  struct command allCommands = {malloc(4*BUFFERSIZE)};
  int numCommands;
  
  while (1)
  {
    //Reset the "arguments" array by populating it with null bytes.
    memset(allCommands.commandTable, 0, BUFFERSIZE);
    allCommands.inputFile = NULL;
    allCommands.outputFile = NULL;
    allCommands.append = 0;
    allCommands.background = 0;

    //Get an input from the user. If the user wants to exit the program, then exit.
    if(getInput(PROMPT, input, BUFFERSIZE) == 0)
      exit(0);

    numCommands = parseCommands(input, &allCommands, BUFFERSIZE);

    //Execute the commands
    execute(&allCommands, numCommands);
  }
  
  return 0;
}

//Prompt the user for an input and store it in the given buffer
int getInput(char* prompt, char* input, int maxSize)
{
    //Prompt the user for an input and read it
    printf("%s", prompt);
    fgets(input, maxSize, stdin);

    //If the user pressed CTRL+D, return false
    if(feof(stdin))
    {
      printf("\n");
      return(0);
    }
    
    //If the user enters "exit", return false
    if(strcmp(input, "exit\n") == 0)
      return(0);

    return 1;
}

//Take a string of commands and arguments and parse it into a 2D array
int parseCommands(char* string, struct command* allCommands, int commandSize)
{
    //Integers to iterate through the 2D array
    int command = 0;
    int argument = 1;
    //Delimiters to split the input string
    const char* delims = " \n";
    //Store the current token being processed
    char* token;
    
    //Create the first row and set its first index to the first token
    allCommands->commandTable[0] = malloc(commandSize);
    allCommands->commandTable[0][0] = strtok(string, delims);
    
    //While there are tokens left, keep looping and populate the array with the tokens
    while((token = strtok(NULL, delims)) != NULL)
    {
      //If the current token is a new pipe symbol, create a new array and start counting from its beginning
      if(strcmp(token, "|") == 0)
      {
	command += 1;
	argument = 1;
	allCommands->commandTable[command] = malloc(commandSize);
	allCommands->commandTable[command][0] = strtok(NULL, delims);
      }
      else if(strcmp(token, "<") == 0)
      {
	char* fileName = strtok(NULL, delims);
	allCommands->inputFile = malloc(sizeof(fileName));
        strcpy(allCommands->inputFile, fileName);
      }
      else if(strcmp(token, ">") == 0)
      {
	char* fileName = strtok(NULL, delims);
	allCommands->outputFile = malloc(sizeof(fileName));
        strcpy(allCommands->outputFile, fileName);
      }
      else if(strcmp(token, ">>") == 0)
      {
	char* fileName = strtok(NULL, delims);
	allCommands->outputFile = malloc(sizeof(fileName));
        strcpy(allCommands->outputFile, fileName);
	allCommands->append = 1;
      }
      //Otherwise, add the token to the array
      else
      {
        allCommands->commandTable[command][argument] = token;
	argument += 1;
      }
    }

    if(argument > 0 && strcmp(allCommands->commandTable[command][argument-1], "&") == 0)
    {
      allCommands->background = 1;
      allCommands->commandTable[command][argument-1] = 0;
    }
    
    //Return the number of commands in the array
    return command + 1;
}

//Execute the command
void execute(struct command* allCommands, int numCommands)
{ 
  int defaultIn = dup(fileno(stdin));
  int defaultOut = dup(fileno(stdout));
  int input;
  int output;
  int childProcess;
  
  if(allCommands->inputFile != NULL)
  {
    input = open(allCommands->inputFile, O_RDONLY);
  }
  else
  {
    input = dup(defaultIn);
  }
  
  for(int i = 0; i < numCommands; i++)
  {
    dup2(input, 0);
    close(input);
    
    if(i != numCommands - 1)
    {
      int pipeCmd[2];
      pipe(pipeCmd);
      //Inherits
      input = pipeCmd[0];
      output = pipeCmd[1];
    }
    else
    {
      if(allCommands->outputFile == NULL)
      {
	output = dup(defaultOut);
      }
      else if(allCommands->append == 1)
      {
	output = open(allCommands->outputFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
      }
      else
      {
	output = open(allCommands->outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
      }
    }
    
    dup2(output, 1);
    close(output);

    if(strcmp(allCommands->commandTable[i][0], "pwd") == 0)
    {
      printWorkingDirectory();
    }
    else if(strcmp(allCommands->commandTable[i][0], "cd") == 0)
    {
      changeDirectory(allCommands->commandTable[i][1]);
    }
    else
    {
      childProcess = fork();
      
      if(childProcess == 0)
      {
        execvp(allCommands->commandTable[i][0], allCommands->commandTable[i]);
        perror("");
        exit(1);
      }
    }
  }
  
  dup2(defaultIn, 0);
  close(defaultIn);
  dup2(defaultOut, 1);
  close(defaultOut);

  if(allCommands->background == 0)
  {
    waitpid(childProcess, NULL, 0);
  }
}

/*Built-In shell commands*/

//Print the directory the shell is currently in
void printWorkingDirectory()
{
  //Create a string to store the file path. The maximum filepath length is 4096 byte.
  char* input = malloc(4096);
  getcwd(input, 4096);
  printf("%s\n", input);
}

//Change the directory the shell is currently in
void changeDirectory(char* destination)
{
  chdir(destination);
}
