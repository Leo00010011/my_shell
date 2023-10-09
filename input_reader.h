#ifndef INPUT_READER_GUARD
#define INPUT_READER_GUARD
#include "structures.h"
typedef struct
{
    int fd;
    char* buffer;
    int count;
    int pos;
}my_file;

/*inicializa el input reader segun para el archivo que indica el fd*/
void init_reader(my_file* file, int fd);
/*busca el siguiente argumento separado por espacios en el archivo 
en result pone:
-1 si el buffer es NULL
 0 si no encontro otro argumento
 1 si comenzo con espacio el argumeto
 2 si comenzo sin espacio el argumento*/
char* get_arg_from_line(my_file* file,int* result);

char* get_line(int fd);

linked_list* split_line(char* buffer);

char* remove_first_arg(char* buffer);

#endif