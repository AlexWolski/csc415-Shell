# CSC 415 - Project 3 - My Shell

Student Name: Alex Wolski<br />
Student ID: 918276364

## Build/Configure Instructions
1. Download the "readline" library. This can be done by running a command:<br /><br />
sudo apt-get install libreadline-dev<br />
for Debian based platforms, or<br />
yum install readline-devel<br />
for platforms with yum<br /><br />
If your linux platform uses neither of the above, follow the cutom intallation instructions:<br />
https://github.com/JuliaLang/readline/blob/master/INSTALL<br />
2. Open the terminal and change the directory to inside "csc415-p3-AlexWolski"
3. Run the command "make" or "make myshell"

## Run Instructions
Run the command "./myshell"
<br />
Enter a linux command you want to run	(eg. ls -al)
<br />
You can also pipe commands together	(eg. ls -al | more)

## List Of Extra Credits Comepleted
1. Pipe works for any number of commands
2. The current working directory is shown on the prompt
3. History of commands is stored and can be navigated using the up and down arrow keys
4. The left and right arrowkeys move the cursor
5. Tab will autocomplete a filename
6. File names with spaces can be surrounded by quotation marks
7. ls, and grep have color output
8. The prompt is colored

## Collaboration
I worked on or shared ideas about parts of this project with:
<br />
Andrew Sarmiento
<br />
Saywer Thompson
<br />
Amari Bolmer
<br />
Mubarak Akinbola
<br />
Vincent Santos
