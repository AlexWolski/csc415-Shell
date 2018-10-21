myshell: myshell.c
	gcc myshell.c -L/usr/local/lib -I/usr/local/include -lreadline -Wall -o myshell

clean:
	rm myshell
	
