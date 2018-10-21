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
//Needed to use the waitpid function
#include <sys/types.h>
#include <sys/wait.h>
//Needed for the history feature
#include <readline/readline.h>
#include <readline/history.h>
//Needed for getting the home directory
#include <pwd.h>

//Maximum size of the input the user can enter
#define BUFFERSIZE 256
//The string and size of the string used to prompt the user
#define PROMPT "myShell "
#define PROMPTSIZE sizeof(PROMPT)

//Simple keywords for booleans
#define true 1
#define false 0



//Contain all of the commands and thier information from an input
struct command
{
  //A 2D array to store all of the commands entered
  char*** commandTable;
  //The file the first command should read from
  char* inputFile;
  //The file the last command should write to
  char* outputFile;
  //Boolean to determine if the last command should append (1) to or truncate (0) the output file
  int append;
  //Boolean to determine if all of the commands should run in the background (1) or not (0)
  int background;
};

//Functions declerations
int getInput(char* prompt, char* input, int maxSize);
int parseCommands(char* string, struct command* allCommands, int commandSize);
void execute(struct command* allCommands, int numCommands);
void printWorkingDirectory();
void changeDirectory(char* destination);

int main(int argc, char** argv)
{
  //Store what the user inputs
  char* input = malloc(BUFFERSIZE);
  //Create a structure to hold all of the commands.
  //Each command in the command table could use as much memory as the input.
  //So allocate the command table with enough memory to hold the maximum number of commands with all with the same length as  input
  struct command allCommands = { malloc(BUFFERSIZE*BUFFERSIZE/2) };
  //Used to hold the number of commands in the struct
  int numCommands;
  
  while (1)
  {
    //Reset the variables in the struct. Use memset to set all of the elements of the 2D array to 0
    memset(allCommands.commandTable, 0, BUFFERSIZE*BUFFERSIZE/2);
    allCommands.inputFile = NULL;
    allCommands.outputFile = NULL;
    allCommands.append = false;
    allCommands.background = false;

    //Get an input from the user. If the user wants to exit the program, then exit.
    if(!getInput(PROMPT, input, BUFFERSIZE))
      return 0;
    
    //Parse the input into the struct and store the number of commands parsed
    //The worst case is that the entire input is in one command, so allocate each command the same amount of memory as the input
    numCommands = parseCommands(input, &allCommands, BUFFERSIZE);

    //Execute the commands if there are any
    if(numCommands > 0)
    {
      execute(&allCommands, numCommands);
    }
  }
  
  return 0;
}

//Return the current directory. Helper function for getInput
char* getCurrDirectory()
{
  //Full working directory
  char* fullDirectory = malloc(4096);
  getcwd(fullDirectory, 4096);

  //Get the length of the working directory
  int fullLength = 0;

  while(fullDirectory[fullLength] != '\0')
  {
    fullLength++;
  }
  
  //Home directory
  char* homeDirectory = getenv("HOME");
  //Get the length of the home directory
  int homeLength = sizeof(homeDirectory);
  
  //If the home directory is a substring of the current directory, simplify the current directory
  if(!strncmp(fullDirectory, homeDirectory, homeLength) && fullLength >= homeLength)
  {
    //Working directory excluding the home direcory. It is the length of the full directory - home directory + ~
    char* simplifiedDirectory = malloc(fullLength - homeLength + 10);

    //Copy the second half of the full directory not including the home directory
    strncpy(simplifiedDirectory, fullDirectory + homeLength + 1, fullLength - homeLength);
    //Replace the first element with '~' to represent the home directory
    simplifiedDirectory[0] = '~';
    
    return simplifiedDirectory;
  }
  
  //If the home directory is not a substring, return the whole directory
  return fullDirectory;
}

//Prompt the user for an input and store it in the given buffer. If the return value is 0, the user wants to exit
int getInput(char* prompt, char* input, int maxSize)
{
    //Prompt the user for an input and read it
    printf("%s", prompt);

    //Get the current directory and print it
    printf("%s", getCurrDirectory());
    
    char* readBuffer = readline(" >> ");

    //If the user enters "CTR+D", return false
    if(readBuffer == NULL)
    {
      printf("\n");
      return false;
    }

    //If the user didn't only enter newline, add the input to the comand history and copy it to input
    if(sizeof(readBuffer) > 0)
    {
      add_history(readBuffer);
      strcpy(input, readBuffer);
    }
    //If the user only input a newline, exit
    else
      return false;
    
    //If the user enters "exit", return false
    if(!strcmp(input, "exit"))
      return false;

    return true;
}

void makeRoom(char** array, int targetIndex)
{
  int i = 0;
  
  //Keep iterating until i is at the end of the aray
  while(array[i] != NULL)
  {
    i++;
  }

  //Loop back trough the array and move each elemetn up one index
  for(; i > targetIndex; i--)
  {
    array[i] = array[i-1];
  }
}

//Take a string of commands and arguments and parse it into a command struct
int parseCommands(char* string, struct command* allCommands, int commandSize)
{
    //Integers to iterate through the 2D array
    int command = 0;
    int argument = 1;
    //Delimiters to split the input string
    const char* delims = " ";
    //Store the current token being processed
    char* token;
    
    //Create the first row and set its first index to the first token.
    allCommands->commandTable[0] = malloc(commandSize);
    allCommands->commandTable[0][0] = strtok(string, delims);
    
    //While there are tokens left, keep looping and populate the array with the tokens
    while((token = strtok(NULL, delims)) != NULL)
    {
      //If the current token is a new pipe symbol, create a new array and start counting from the beginning of that array
      if(!strcmp(token, "|"))
      {
	command++;
	argument = 1;
	allCommands->commandTable[command] = malloc(commandSize);
	allCommands->commandTable[command][0] = strtok(NULL, delims);
      }
      //If an input file is specified, store it in the struct
      else if(!strcmp(token, "<"))
      {
	allCommands->inputFile = strtok(NULL, delims);
      }
      //If an output file is specified, store it in the struct
      else if(!strcmp(token, ">"))
      {
	allCommands->outputFile = strtok(NULL, delims);
      }
      //If an output file is specified in append mode, store it in the struct and toggle the append boolean
      else if(!strcmp(token, ">>"))
      {
	allCommands->outputFile = strtok(NULL, delims);
	allCommands->append = true;
      }
      //Otherwise, add the token to the array
      else
      {
        allCommands->commandTable[command][argument] = token;
	argument++;
      }
    }

    if(allCommands->commandTable[0][0] == NULL)
    {
      return 0;
    }
    
    //If the last argument is '&', toggle the background boolean in the struct
    if(argument > 0 && !strcmp(allCommands->commandTable[command][argument-1], "&"))
    {
      allCommands->background = true;
      //Remove '&' from the command
      allCommands->commandTable[command][argument-1] = 0;
    }

    //If the last command is ls or grep, add the option to enable color for commands
    if(!strcmp(allCommands->commandTable[command][0], "ls") || !strcmp(allCommands->commandTable[command][0], "grep"))
    {
      makeRoom(allCommands->commandTable[command], 1);

      //Set the second element to the color option
      allCommands->commandTable[command][1] = "--color=auto";
    }
    
    //Return the number of commands in the array
    return command + 1;
}

//Execute the command
void execute(struct command* allCommands, int numCommands)
{
  //Store the defualt input and outfile locations
  int defaultIn = dup(fileno(stdin));
  int defaultOut = dup(fileno(stdout));
  //Store the input and output locations for each process
  int input;
  int output;
  //Hold the process id of the child process
  int childProcess;

  //If there is no input file stored in the struct, use the default input
  if(allCommands->inputFile != NULL)
  {
    input = open(allCommands->inputFile, O_RDONLY);
  }
  //Otherwise, store its file id in the 'input' input
  else
  {
    input = dup(defaultIn);
  }

  //Loop through all of the commands in the struct
  for(int i = 0; i < numCommands; i++)
  {
    //Redirect the input to the location determined above
    dup2(input, 0);
    close(input);

    //If the current command isn't the last command, set up a pipe to the next command
    if(i != numCommands - 1)
    {
      //Create a pipe
      int pipeCmd[2];
      pipe(pipeCmd);
      //The child will inherit 'input' which points to the read end of the pipe
      input = pipeCmd[0];
      //Set the output of the current process to the write end of the pipe so that the next process can read it
      output = pipeCmd[1];
    }
    //If the current command is the last command, determine where the output should go
    else
    {
      //If there is no output file stored in the strut, set the output to the default output location
      if(allCommands->outputFile == NULL)
      {
	output = dup(defaultOut);
      }
      //If there is an output file specified and the append boolean is true, open the file with the append flag
      else if(allCommands->append)
      {
	output = open(allCommands->outputFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
      }
      //If there is an output file specifid and the append boolean is flase, open the file with the truncate flag
      else
      {
	output = open(allCommands->outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
      }
    }

    //After the output location is determined (default location, a file, or a pipe), redirect the program's output to there
    dup2(output, 1);
    close(output);

    //If the current command is 'pwd', run the built in pwd function
    if(!strcmp(allCommands->commandTable[i][0], "pwd"))
    {
      printWorkingDirectory();
    }
    //If the current command is 'cd', run the built in cd function
    else if(!strcmp(allCommands->commandTable[i][0], "cd"))
    {
      changeDirectory(allCommands->commandTable[i][1]);
    }
    //If the command is not built into the shell, create a child process, find the program, and execute it
    else
    {
      //Create a child process
      childProcess = fork();

      //If the current process is the child, execute the command
      if(!childProcess)
      {
        execvp(allCommands->commandTable[i][0], allCommands->commandTable[i]);
        perror("");
        exit(1);
      }
    }
  }

  //After all of the commands are run, reset the defaut input and output files
  dup2(defaultIn, 0);
  close(defaultIn);
  dup2(defaultOut, 1);
  close(defaultOut);


  //If the background boolean is not set in the struct, wait for the last process to finish before returning
  if(!allCommands->background)
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
