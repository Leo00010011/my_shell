#ifndef SAVE_GUARD
#define SAVE_GUARD
/*salva el contenido del buffer en el archivo indicado para ser leido por load_var*/
void save_var(int fd, void* buffer,unsigned int size);
/*carga el contenido de una variable guardada con save_var
  ** el pos del fd debe estar en la primera posicion del entero que indica el tamanno de la variable*
  ** para evitar complicaciones se recomienda solo leer el archivo con load_var y modificarlo con save_var*
  ** si result != NULL en el se pone el valor que retorna el read al intentar leer el archivo* */
  
void* load_var(int fd,int* result);

#endif
