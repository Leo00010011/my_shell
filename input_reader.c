#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "input_reader.h"
#include "structures.h"


void init_reader(my_file* file, int fd)
{
    file->fd = fd;
    file->buffer = malloc(sizeof(char)*100);
    file->count = read(fd,file->buffer,sizeof(char)*100) / sizeof(char);
    file->pos = 0;
}

int refresh_buffer(my_file* file,int size)
{
    free(file->buffer);
    file->buffer = malloc(sizeof(char)*size);
    int result = read(file->fd,file->buffer,sizeof(char)*size);
    file->count = result/sizeof(char);
    file->pos = 0;
    return result;
}

char* get_arg_from_line(my_file* file,int* result)
{
    if(result == NULL)//si pasan un result NULL, para no tener que seguir chequeando si es NULL
    {
        result = malloc(sizeof(int));
    }

    if(file->buffer == NULL)//si no se ha inicializado la estructura dar error
    {
        *result = -1;
        return NULL;
    }

    int buffer_pos;//posicion donde empieza el texto en el buffer
    int arg_pos = 0;//posicion donde se quedo copiando el resultado
    char* arg = NULL;//el resultado que se ha ido armando
    *result = 0;

    do
    {
        while (file->pos < file->count && file->buffer[file->pos] == ' ')
        {
            *result = 1;
            file->pos++;
        }
        if(file->pos < file->count)
        {
            buffer_pos = file->pos;
            break;
        }
    }while(refresh_buffer(file,100) > 0);
    
    //si termino con el buffer vacio fue que ya se leyo todo el archivo y no encontro texto
    if(file->count == 0 ||file->buffer[file->pos] == '\n')
    {
        return NULL;
    }
    do
    {
        while (file->pos < file->count && file->buffer[file->pos] != ' ' && file->buffer[file->pos] != '\n')
        {
            file->pos++;
        }
        if(file->pos < file->count)
        {
            break;
        }
        else//salvando el pedazo de argumento que se obtuvo antes de refrescar el buffer
        {
            arg = realloc(arg,arg_pos + file->pos - buffer_pos + 1);
            for(int i = 0; buffer_pos + i < file->pos; i++)
            {
                arg[arg_pos + i] =file->buffer[buffer_pos + i];
            }
            arg_pos += file->pos - buffer_pos;
            buffer_pos = 0;
        }
    }while(refresh_buffer(file,100) > 0);
    
    if(file->count == 0)//si se termino con un eof retorna el argumento que se pudo salvar
    {
        if(strlen(arg) != 0)
        {
            if(*result != 1)
            {
                *result = 2;
            }
        }
        else
        {
            *result = 0;
        }
        return arg;
    }
    //copiando el pedazo de argumento que esta dentro del buffer actual
    arg = realloc(arg,(arg_pos + file->pos - buffer_pos + 1)*sizeof(char));

    for(int i = 0; buffer_pos + i < file->pos; i++)
    {
        arg[arg_pos + i] =file->buffer[buffer_pos + i];
    }
    arg[arg_pos + file->pos - buffer_pos] = '\0';
    
    if(strlen(arg) != 0)
    {
        if(*result != 1)
        {
            *result = 2;
        }
    }
    else
    {
        *result = 0;
    }
    return arg;
}

char* get_line(int fd)
{
    char* buffer = NULL;
    char* result = NULL;
    int last_size = 0;
    bool end_founded = false;
    int i;
    while(!end_founded)
    {
        buffer = malloc(sizeof(char)*100);
        read(fd,buffer,sizeof(char)*100);
        for(i = 0; i < 100; i++)
        {
            if(buffer[i] == '\n')
            {
                end_founded = true;
                break;
            }
        }
        result = realloc(result,last_size + i + 1);
        for(int j = 0;j < i; j++)
        {
            result[last_size + j] = buffer[j];
        }
        last_size += i;
        free(buffer);
    }
    result[last_size] = '\0';
    return result;
}

linked_list* split_line(char* buffer)
{
    int pos = 0;
    linked_list* result = create_linked_list();
    while (buffer[pos] != '\0')
    {
        while (buffer[pos] != '\0' && buffer[pos] == ' ')
        {
            pos++;
        }
        int first_pos = pos;
        while(buffer[pos] != '\0' && buffer[pos] != ' ')
        {
            pos++;
        }
        if(pos == first_pos)
        {
            return result;
        }
        char* item = malloc(pos - first_pos + 1);
        for(int i = 0; first_pos + i < pos; i++)
        {
            item[i] = buffer[i + first_pos];
        }
        item[pos - first_pos] = '\0';
        add_last(result,item);
    }
    return result;
}

char* remove_first_arg(char* buffer)
{
    int pos = 0;
    int len = strlen(buffer);
    while (pos < len && buffer[pos] == ' ')
    {
        pos++;
    }
    while(pos < len && buffer[pos] != ' ')
    {
        pos++;
    }
    if(pos == len)
    {
        return NULL;
    }
    char* result = malloc((len - pos + 1)*sizeof(char));
    int i = 0;
    for(i = 0; pos + i < len; i++ )
    {
        result[i] = buffer[pos + i];
    }
    result[i] = '\0';
    return result;
}