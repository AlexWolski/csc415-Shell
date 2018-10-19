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

#define BUFFERSIZE 256
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

int main(int* argc, char** argv)
{
  //Store what the user inputs
  char* input = malloc(BUFFERSIZE);
  //2D array of string to store each command and all of their arguments
  char*** commandTable = malloc(BUFFERSIZE);
  int numCommands;
  
  while (1)
  {
    //Reset the "arguments" array by populating it with null bytes.
    memset(commandTable, 0, BUFFERSIZE);

    //Get an input from the user. If the user wants to exit the program, then exit.
    if(getInput(PROMPT, input, BUFFERSIZE) == 0)
      exit(0);

    //Parse the user's input into a 2D array where the rows are commands and the columns are arguments
    numCommands = parseCommands(input, commandTable, BUFFERSIZE);

    //Execute the commands
    execute(commandTable, numCommands);
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
int parseCommands(char* string, char*** commandTable, int commandSize)
{
    //Integers to iterate through the 2D array
    int command = 0;
    int argument = 1;
    //Delimiters to split the input string
    const char* delims = " \n";
    //Store the current token being processed
    char* token;
    
    //Create the first row and set its first index to the first token
    commandTable[0] = malloc(commandSize);
    commandTable[0][0] = strtok(string, delims);
    
    //While there are tokens left, keep looping and populate the array with the tokens
    while((token = strtok(NULL, delims)) != NULL)
    {
      //If the current token is a new pipe symbol, create a new array and start counting from its beginning
      if(strcmp(token, "|") == 0)
      {
	command += 1;
	argument = 0;
	commandTable[command] = malloc(commandSize);
	commandTable[command][0] = strtok(NULL, delims);
      }
      //Otherwise, add the token to the array
      else
      {
        commandTable[command][argument] = token;
      }

      argument += 1;
    }

    //Return the number of commands in the array
    return command + 1;
}



//Execute the command
int execute(char*** commands, int numCommands)
{
    if(strcmp(commands[0][0], "pwd") == 0)
    {
      printWorkingDirectory(commands[0][1]);
    }
    else if(strcmp(commands[0][0], "cd") == 0)
    {
      changeDirectory(commands[0][1]);
    }
    else
    {
      //Execute the command
      if(fork() == 0)
        execvp(commands[0][0], commands[0]);
      else
        wait(NULL);
    }
}

///Built-In shell commands

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
