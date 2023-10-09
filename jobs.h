#ifndef JOBS_GUARD
#define JOBS_GUARD
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>
#include "structures.h"

/*handler para actualizar la lista de jobs*/
void update_jobs();
/*inicializa la pila de jobs e instala el handler*/
void init_jobs();
/*borra un determinado job de la pila de jobs*/
void delete_job(pid_t pid);
/*hace fork y annade el pid en la pila de jobs*/
int add_job();
/*entrega un array terminado en NULL con los pid que de los procesos que estan en background*/
pid_t* get_jobs();
/*envia un SIGKILL a todos los procesos que estan en background*/
void kill_all_jobs();
/*retorna el pid del ultimo job*/
int last_job();
/*hace waitpid al proceso indicado manteniendo en un estado correcto la estructura de los jobs*/
void fg_pid(pid_t pid);
#endif