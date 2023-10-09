my_shell: my_shell.o history.o structures.o save.o jobs.o input_reader.o
	gcc -o my_shell my_shell.o history.o structures.o save.o jobs.o input_reader.o

my_shell.o: my_shell.c
	gcc -c my_shell.c

history.o: structures.o save.o history.c
	gcc -c history.c 

structures.o: structures.c
	gcc -c structures.c

save.o: save.c
	gcc -c save.c

jobs.o: structures.o jobs.c
	gcc -c jobs.c

input_reader.o: input_reader.c
	gcc -c input_reader.c