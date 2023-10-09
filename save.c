#include <unistd.h>
#include <stdlib.h>
#include "save.h"
void save_var(int fd, void* buffer,unsigned int size)
{
    write(fd, &size, sizeof(unsigned int));
    write(fd, buffer,size);
}

void* load_var(int fd,int* result)
{
    int temp;
    unsigned int size;
    read(fd,&size,sizeof(unsigned int));
    void* var = malloc(size);
    temp = read(fd,var,size);
    if(result != NULL)
    {
        *result = temp;
    }
    return var;
}