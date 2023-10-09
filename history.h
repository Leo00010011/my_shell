#ifndef SHELL_TOOLS_GUARD
#define SHELL_TOOLS_GUARD

#include "structures.h"
/*carga el history del archivo o lo crea en caso de que no exista*/
int init_history();
/*annade una copia del comando que se pasa como argumento*/
int add_history(char* cmd);
/*entrega una copia del valor con indice index en el history, suponiendo que los indices empiezan en 0
 **retorna NULL si history no se ha incializado o no existe elemento en esa posicion de la lista **/
char* get_history(int index,int* result);
/*entrega un array terminado en NULL con una copia de cada uno de los elementos que estan en history
**retorna NUll si history no se ha inicializado o no existe elemento en esa posicion en la lista**/
char** get_all_histories();
/*guarda la informacion de history en un archivo localizado en el cwd llamado history
**retorna -1 si ocurrio algun error creando el archivo**/
int save_histories();

int get_history_count();

#endif