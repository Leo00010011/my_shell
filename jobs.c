#include "jobs.h"

static stack* jobs = NULL;

void update_jobs()
{
    int pid = -1;
    while((pid = waitpid(-1,NULL,WNOHANG)) > 0)
    {
        delete_job(pid);
    }
}

void init_jobs()
{
    signal(SIGCHLD,update_jobs);
    jobs = create_stack();
}

void delete_job(pid_t pid)
{
    if(jobs == NULL || jobs->count == 0)
    {
        return;
    }

    linked_node* current = jobs->top;
    
    while(current != NULL)
    {
        if(*((int*)current->value) == pid)
        {
            jobs->count--;
            if(current->preview != NULL)
            {
                ((linked_node*)(current->preview))->next = current->next;
            }

            if(current->next != NULL)
            {
                ((linked_node*)(current->next))->preview = current->preview;
            }
            else
            {
                jobs->top = current->preview;
            }


            free(current->value);
            free(current);
            return;
        }
        current = current->preview;
    }
}

int add_job()
{
    if(jobs == NULL)
    {
        return -1;
    }
    sigset_t sigchld_set;
    sigemptyset(&sigchld_set);
    sigaddset(&sigchld_set,SIGCHLD);
    sigprocmask(SIG_BLOCK,&sigchld_set,NULL);
    pid_t* pid = malloc(sizeof(int));

    if((*pid = fork()) == 0)
    {
        sigprocmask(SIG_UNBLOCK,&sigchld_set,NULL);
        return 0;
    }
    push(jobs, pid);
    sigprocmask(SIG_UNBLOCK,&sigchld_set,NULL);

    return *pid;
}

pid_t* get_jobs()
{
    pid_t* result = malloc(sizeof(int)*(jobs->count + 1));
    linked_node* current = jobs->top;
    int i = 0;
    while(current != NULL)
    {
        result[i] = *((pid_t*)current->value);
        current = current->preview;
        i++;
    }
    
    result[i] = NULL;
    return result;
}

void kill_all_jobs()
{
    int current_job;
    while(jobs->top != NULL)
    {
        current_job = *(int*)pop(jobs);
        kill(current_job,SIGKILL);
    }
}

void fg_pid(pid_t pid)
{
    sigset_t sigchld_set;
    sigemptyset(&sigchld_set);
    sigaddset(&sigchld_set,SIGCHLD);
    sigprocmask(SIG_BLOCK,&sigchld_set,NULL);
    int w_pid = -1;
    while (1)
    {
        if((w_pid = waitpid(-1,NULL,0)) > 0 )
        {
            delete_job(w_pid);
            if(w_pid == pid)
            {
                break;
            } 
        }
        if(errno == ECHILD)
        {
            break;
        }
    }
    sigprocmask(SIG_UNBLOCK,&sigchld_set,NULL);
}

int last_job()
{
    return *((int*)jobs->top->value);
}


