#include "save.h"
#include <string.h>
#include "fcntl.h"
#include <stdio.h>
#include "history.h"
#include "unistd.h"

static fs_queue* history = NULL;

static int fd_history = -1;

int init_history()
{
    history = create_fs_queue(10);
    char* path = malloc(sizeof(char)*100);
    getcwd(path,100);
    strcat(path,"/history");
    if((fd_history = open(path,O_CREAT | O_RDWR,S_IRWXU)) < 0)
    {
        return -1;
    }
    free(path);
    int result = 1;
    char* buffer = load_var(fd_history,&result);
    while (result > 0)
    {
        enqueue_fs(history,buffer);
        buffer = load_var(fd_history,&result);
    }
    return 0;
}

int add_history(char* cmd)
{
    if(history == NULL)
    {
        return -1;
    }
    char* copy = malloc(strlen(cmd)*sizeof(char));
    strcat(copy,cmd);
    enqueue_fs(history,copy);
    return 0;
}

char* get_history(int index,int* result)
{
    if(history == NULL)
    {
        *result = -1;
        return NULL;
    }
    if(index >= history->count || index < 1)
    {
        *result = -1;
        return NULL;
    }
    char* copy = malloc(sizeof(char)*strlen((char*)get_fs_queue(history,index)));
    strcat(copy,(char*)get_fs_queue(history,index));
    return copy;
}

char** get_all_histories()
{
    if(history == NULL)
    {
        return NULL;
    }
    char** result = malloc(sizeof(char*)*history->count + 1);
    char* current;
    for (int i = 0; i < history->count; i++)
    {
        current = malloc(sizeof(char)*strlen(get_fs_queue(history,i)));
        strcat(current,get_fs_queue(history,i));
        result[i] = current;
    }
    result[history->count] = NULL;
    
    return result;
}

int save_histories()
{
    char* path = malloc(sizeof(char)*100);
    getcwd(path,100);
    strcat(path,"/history");

    if(access(path,F_OK) == 0)
    {
        remove(path);
    }

    if((fd_history = open(path,O_CREAT | O_RDWR,S_IRWXU)) < 0)
    {
        free(path);
        return -1;
    }
    free(path);

    char* buffer;
    for (int i = 0; i < history->count; i++)
    {
        buffer = get_fs_queue(history,i);
        save_var(fd_history,buffer,sizeof(char)*strlen(buffer));
    }
    return 0;
}

int get_history_count()
{
    return history->count;
}