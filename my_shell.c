
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "structures.h"
#include "input_reader.h"
#include "jobs.h"
#include "history.h"
#include "my_shell.h"
#include <fcntl.h>
#include <errno.h>
static int pid_fg = -1;
static bool has_sigint = false;


cmd_state* create_cmd_state(char* buffer)
{
    cmd_state* cmd = malloc(sizeof(cmd_state));
    cmd->arg_list = split_line(buffer);
    cmd->current_arg = cmd->arg_list->first;
    cmd->last_result = -1;
    return cmd;
}
/*si el comando donde esta current_args es built-in lo ejecuta y retorna 0
  si el comando donde esta current_args es built-in y tiene errores de sintaxis retorna -1
  si el comando donde esta current_args no es built-in retorna 1 */
int built_in(cmd_state* cmd, int count_arg)
{
    if(strcmp(cmd->current_arg->value,"cd") == 0)
    {
        cmd->last_result = 0;
        if(count_arg > 2)
        {
            return -1;
        }
        cmd->current_arg = cmd->current_arg->next;
        if(cmd->current_arg == NULL)
        {
            return -1;
        }
        if(chdir(cmd->current_arg->value) < 0)
        {
            return -1;
        }
        cmd->current_arg = cmd->current_arg->next;
        return 0;
    }
    if(strcmp(cmd->current_arg->value,"jobs") == 0)
    {
        cmd->last_result = 0;
        if(count_arg > 1)
        {
            return -1;
        }
        pid_t* jobs = get_jobs();
        for(int i = 0; jobs[i] != NULL; i++)
        {
            printf("%d\n",jobs[i]);
        }
        cmd->current_arg = cmd->current_arg->next;
        return 0;
    }
    if(strcmp(cmd->current_arg->value,"fg") == 0)
    {
        cmd->last_result = 0;
        if(count_arg == 1)
        {
            pid_fg = last_job();
            fg_pid(pid_fg);
            cmd->current_arg = cmd->current_arg->next;
            return 0;
        }
        if(count_arg == 2)
        {
            cmd->current_arg = cmd->current_arg->next;
            if(cmd->current_arg == NULL)
            {
                return -1;
            }
            pid_fg = strtol(cmd->current_arg->value,NULL,10);
            fg_pid(pid_fg);
            cmd->current_arg = cmd->current_arg->next;
            return 0;
        }
        return -1;
    }
    if(strcmp(cmd->current_arg->value,"history") == 0)
    {
        cmd->last_result = 0;
        if(count_arg != 1)
        {
            return -1;
        }
        char** histories = get_all_histories();
        for(int i = 0; histories[i] != NULL; i++)
        {
            printf("%d %s\n",i + 1,histories[i]);
        }
        cmd->current_arg = cmd->current_arg->next;
        return 0;
    }
    if(strcmp(cmd->current_arg->value,"again") == 0)
    {
        cmd->last_result = 0;
        if(count_arg != 2)
        {
            return -1;
        }
        cmd->current_arg = cmd->current_arg->next;
        if(cmd->current_arg == NULL)
        {
            return -1;
        }
        int index = strtol(cmd->current_arg->value,NULL,10);
        if(index < 1 || index > 10)
        {
            return -1;
        }
        if(index >= get_history_count())
        {
            return 0;
        }
        char* buffer = get_history(index - 1 ,NULL);
        cmd_state* again_cmd = create_cmd_state(buffer);
        cmd->current_arg = cmd->current_arg->next;
        int result = process_cmd(again_cmd);
        if(result == 0)
        {
            return 0;
        }
        return -1;
    }
    if(strcmp(cmd->current_arg->value,"true") == 0)
    {
        if(count_arg != 1)
        {
            return -1;
        }
        cmd->last_result = 0;
        cmd->current_arg = cmd->current_arg->next;
        return 0;
    }
    if(strcmp(cmd->current_arg->value,"false") == 0)
    {
        if(count_arg != 1)
        {
            return -1;
        }
        cmd->last_result = 1;
        cmd->current_arg = cmd->current_arg->next;
        return 0;
    }
    if(strcmp(cmd->current_arg->value,"exit") == 0)
    {
        if(count_arg != 1)
        {
            return -1;
        }
        kill_all_jobs();
        save_histories();
        exit(0);
    }
    if(strcmp(cmd->current_arg->value,"help") == 0)
    {
        if(count_arg == 1)
        {
            cmd->current_arg = cmd->current_arg->next;
            printf("Integrante: Leonardo Ulloa C212\n");
            printf("Funcionalidades: \n");
            printf("basic: (3 puntos)\n");
            printf("spaces: (0,5 puntos)");
            printf("multi-pipe: (1 punto)\n");
            printf("background: (0.5 puntos)\n");
            printf("history: (0.5 puntos)\n");
            printf("chain: (0.5 puntos)\n");
            printf("help: (1 punto)\n");
            printf("\n");
            printf("comandos built-in:\n");
            printf("cd: cambia directorios\n");
            printf("exit: termina el shell de forma correcta\n");
            printf("help: muestra la ayuda general\n");
            printf("again history fd fd <pid> jobs");
            printf("again <numero>: ejecuta el comando de la historia indicado\n");
            printf("history: imprime la historia\n");
            printf("fd: pone en foreground el proceso mas reciente del background\n");
            printf("fd <pid>: pone en foreground el proceso del background indicado\n");
            printf("jobs: imprime los procesos que estan en background\n");
            printf("\n");
            printf("Total: 7\n");
            return 0;
        }
        if(count_arg == 2)
        {
            if(cmd->current_arg->next == NULL)
            {
                return -1;
            }
            cmd->current_arg = cmd->current_arg->next;
            if(strcmp(cmd->current_arg->value,"basic") == 0)
            {
                printf("basic:\n");
                printf(" Para los comentarios en el primer momento que se tiene la entrada parseada se chequea\n");
                printf("el primer argumento.\n");
                printf(" Para interpretar la entrada separo el comando en conjuntos (que les puse atomos)\n");
                printf("los atomos son las partes de la instruccion que pueden depender de un solo execv,\n");
                printf("es decir las partes de la instruccion que estan delimitadas por: '&&' ,';' ,'||' ,'|'\n");
                printf("principio o final.(para ver detalles del parseo ver spaces y el porque los atomos ver chain)\n");
                printf(" Luego al interpretar un atomo compruebo si el primer parametro es un built-in y\n");
                printf("ahi se van los casos de cd y exit. En el caso de cd solo se comprueba si esta el path\n");
                printf("y solo el path ademas de el en su atomo y se usa chdir para cambiar el cwd del proceso \n");
                printf("actual en la que se basa la direccion del shell. En el exit se hace algo semejante para\n");
                printf("comprobar la sintaxis y en caso de ser correcta se guarda el history y se manda a \n");
                printf("terminar los procesos en background despues se llama exit(0).\n");
                printf("\n");
                printf("En el caso de las redirecciones de entrada la solucion fue similar, exeptuando la parte\n");
                printf("del comportamiento, cuando un atomo no es built-in y se va a procesar un atomo se tiene \n");
                printf("la logica siguiente: \n");
                printf("- se recorre el atomo argumento por argumento\n");
                printf("    - si se encuentra un '>', '<' o '>>' intepreta el argumento que le sigue como el archivo\n");
                printf("    al que se refiere.\n");
                printf("    - todo lo que no tenga antes alguno de los anteriores sera argv para el execv\n");
                printf("Para poder redireccionar la entrada y la salida al principio de la ejecucion del shell\n");
                printf("se duplican el stdin y el stdout en los fd 3 y 4 respectivamente y al final de procesar\n");
                printf("cada atomo se copian en su lugar para restablecerlos en caso de que hayan sido modificados.\n");
                printf("por alguna redireccion o un pipe.\n");
                printf("Cada vez que en el recorrido se encuentra un '>', '<' o '>>' se tiene el sgte comportamiento\n");
                printf("> : se usa open con el path que indique el argumento que le sigue la opcion O_CREAT | OWRITE  \n");
                printf("y el permiso S_IRWXU  para crear o sobrescribir el archivo indicado y obtener un fd.\n");
                printf("\n");
                printf("< : en el este caso se comprueba si el archivo es accesible con access del unistd en caso de no\n");
                printf("serlo se lanza un error de sintaxis, en caso de serlo se llama open con O_READ y el path que\n");
                printf("indica el argumento que le sigue.\n");
                printf("\n");
                printf(">> : en este caso se comprueba si el archivo es accesible de la misma forma que con el anterior\n");
                printf("en caso de no ser accesible se crea como en  '>' y en caso de serlo se abre con open y la opcion\n");
                printf("O_APPEND.\n");
                printf("En todos los casos anteriores se hace dup2 con el fd que da el open en el stdout en el caso de '>>'\n");
                printf(" y '>'; y en el stdin en el caso de '<' y despues se cierra el fd que dio el open.\n");
                printf("\n");
                printf("Los pipes se explicaran en multi-pipe\n");
                printf("\n");
                printf("Despues de procesar el atomo se ve si necesita pipes(ver multi-pipe) y se instala si es necesario\n");
                printf("y se llama execv con el argv que se ha formado, si arg[0] empieza con './' se considera que se.\n");
                printf("quiere llamar un ejecutable que esta en cwd, en caso contrario se considera que se quiere uno de\n");
                printf("bin.\n");
                printf("\n");


                return 0;
            }
            if(strcmp(cmd->current_arg->value,"spaces") == 0)
            {
                printf(" Para leer la entrada se implemento un metodo llamado get_line cuyo source code esta en\n");
                printf("input_reader.c, el cual va leyendo el stdin en conjuntos de 100 bytes del fd y llevando lo \n");
                printf("que encuentra en un buffer, hasta que encuentra el cambio de linea. Despues se crea una\n");
                printf("estructura con la informacion del comando donde se lleva el siguiente argumento a procesar\n");
                printf("la si el atomo anterior devolvio 0 o 1 y una linked list de argumentos. El linked list de\n");
                printf("argumentos se construye con el metodo split que esta input_reader.c\n");
                printf("\n");
                printf(" Luego el procesamiento del comando ocurren en 3 metodos, process_cmd se encarga de la relacion\n");
                printf("entre los atomos, process_atom que se encarga de poner las redirecciones y ver si el atomo\n");
                printf("es un built-in, y built-in que se encarga de ver si el argumento dado es builtin y en caso\n");
                printf("de serlo lo procesa.(ver chain)\n");
                printf("\n");

                return 0;
            }
            if(strcmp(cmd->current_arg->value,"multi-pipe") == 0)
            {
                printf("El comportamiento del pipe se decide despues de procesar el atomo(poner las redirecciones ver\n");
                printf("basic). Cuando se procesa el atomo, el argumento a procesar(ver spaces) queda en un separador(ver\n");
                printf("chain) o en un pipe, si termina en un pipe lo que se hace es crear el pipe y poner la parte de \n");
                printf("escritura en el stdout para que en execv que viene despues el proceso escriba. Y se guarda en una\n");
                printf("variable que hay un pipe de lectura y el fd de ese pipe y se avanza de argumento a procesar en la\n");
                printf("estructura que lleva la info del procesamiento de la entrada(ver spaces), que al ver que hacer con \n");
                printf("el siguiente atomo lo capte y ponga el pipe de lectura en el stdin y y lo cierre, el stdout va a\n");
                printf("estar correctamente en ese momento de la ejecucion porque despues de ejecutar cada atomo se restablecen\n");
                printf("el stdin y el stdout (ver chain).\n");
                printf("\n");
                printf("No estan implementados para ser usados con built-in\n");
                printf("\n");

                return 0;
            }
            if(strcmp(cmd->current_arg->value,"background") == 0)
            {
                printf("La mayor parte del codigo relacionado con esta funcionalidad esta en jobs.c y la implementacion consiste\n");
                printf("en una pila y varios metodos para gestionar los jobs. Para detectar que se quiere mandar un proceso al \n");
                printf("background se analiza el ultimo argumento justo despues del parseo y en caso de serlo se 'crea' el job y\n");
                printf("se procesa el comando dentro de el, para resolver el caso de los chains dentro de jobs.\n");
                printf("\n");
                printf(" Para 'crear' job se usa el metodo add_job que consiste en un fork y hacer push al pid en la pila de jobs\n");
                printf("pero con el SIGCHLD bloqueado para evitar la condicion de carrera que explican en el libro, en que el proceso\n");
                printf("que se manda al background termina antes de que se annada a la pila cuando el handler procesa la sennal el \n");
                printf("proceso no se encuentra en la pila de jobs y despues cuando se vuelve al flujo original del proceso padre\n");
                printf("se annade un job que ya termino a la pila y es un job que nunca va a ser retirado\n");
                printf("\n");
                printf(" En el caso de los fg, pues el fg sin parametro llama al fg con parametro al con el pid que esta en el tope de\n");
                printf("la pila de jobs, y lo que hacen es bloquear el SIGCHLD y esperar cualquier proceso con waitpid y arg -1\n");
                printf("cuando llegue algun proceso se remueve de la pila comprueba si es el que se pidio para foreground y en caso\n");
                printf("afirmativo deja de esperar a que se termine algun proceso\n");
                printf("\n");
                printf(" Para mantener la pila actualizada se usa un handler de SIGCHLD en que se hace waitpid -1 con WNOHANG tantas\n");
                printf("veces como de un valor positivo, y asi se cosechan todos los posibles procesos que mandaron SIGCHLD y no\n");
                printf("han sido atedidos. Se usa WNOHANG para no entrar en conflicto con el waitpid de algun proceso que se llame en\n");
                printf("el foreground, pues si llega un SIGCHLD  este puede ser de un proceso en background o de el de foreground\n");
                printf("en el caso de que sea el de foreground en el momento en que se hace en contex switching se procesa primero\n");
                printf("el handler y este lo que haria seria cosechar el proceso actual y como dio positivo esperaria a que termine otro\n");
                printf("hijo que seria uno de background, bloqueando en shell hasta que todo proceso en background termine,por eso el\n");
                printf("WNOHANG\n");
                printf("\n");
                return 0;
            }
            if(strcmp(cmd->current_arg->value,"history") == 0)
            {
                printf(" Los codigos fuentes de history esta en history.c y a su vez usa unos codigos para leer y escribir variables\n");
                printf("en archivos que estan en save.c. Mi implementacion de history consiste en una cola circular de tamanno fijo\n");
                printf("y cada vez que se lee una linea del shell para ser procesada esta es puesta en la cola. En el caso de los comandos\n");
                printf("que empiezan con again, este y su argumento son removidos en el usando una funcion que esta en input_reader.c\n");
                printf("\n");
                printf(" Para salvar las variables se escriben los datos en el siguiente formato <size><value><size><value>..., donde\n");
                printf("size se un unsigned int para indicar la cantidad de bytes del valor a leer, entonces la lectura consiste en\n");
                printf("leer un unsigned int del archivo y leer a continuacion esa cantidad de bytes del archivos y guardarlos en una \n");
                printf("memoria reservada con malloc y ese puntero se le entrega al usuario\n");
                printf(" \n");
                printf(" En el caso de que el comando que llaman comienza con history se hace es guardar primero el history y despues\n");
                printf("ejecutar el comando para que history quede de ultimo, y en el caso de que no primero se ejecuta el comando y \n");
                printf("despues se guarda en el history, esto se hizo para evitar confunciones con el again, pues si se llama again a un\n");
                printf("numero y hay 10 argumentos su posicion va a ser una menos que la mostrada si se llama history justo anteriormente \n");
                printf("\n");
                printf(" El again lo que hace es pedirle el comando al history, parsearlo y ejecutarlo reutilizando funciones ya implementadas\n");
                printf("por el shell para ejecutar sus comandos.\n");
                printf("\n");
                return 0;
            }
            if(strcmp(cmd->current_arg->value,"chain") == 0)
            {
                printf(" Para implementar chains y multi-pipe se interpreto el codigo en cadenas de atomos relacionadas entre si, donde \n");
                printf("atomo es toda aquella informacion que solo tiene que ver con un execv o un built-in, en otras palabras todo aquello\n");
                printf("que esta entre '||', '&&', ';' o '|', de esta forma los comandos se pueden ejecutar en un bucle donde se procesa un atomo\n");
                printf("se ejecuta y se define su relacion con el siguiente (en el caso de los pipes la relacion se define antes de ejecutar).\n");
                printf("Esto se implemento utilizando la estrucura comentada en spaces en la que se lleva en una variable el argumento a procesar,\n");
                printf("y se trabaja con la invariante de que cuando se llama process_atom esta va quedar apuntando a un argumento de separacion\n");
                printf("o a NULL en el caso de que termine. Para saber el comportameinto se utiliza el valor de la variable last_result en\n");
                printf("la que se guarda si el ultimo atomo ejecutado retorno 0 o 1 y se asume que los bult-in siempre retornan 1\n");
                printf("\n");
                printf("Para ejecutar los chain todo se necesita poner pipes o archivos en el stdin y el stdout, para poder recuperarlos \n");
                printf("despues se le hace un dup2 antes de ejecutar el shell en las posiciones 3 y 4 respectivamente, para despues de cada\n");
                printf("ejecucion se restablezcan ya que en el unico caso que se necesitaria alguna modificacion de la tabla de fd del atomo\n");
                printf("anterior seria en los pipes y esto se hace en el ciclo siguinte de la manera descrita a continuacion\n");
                printf("- todo lo que esta a continuacion se va a ejecutar en un ciclo hasta que se acaben los argumentos\n");
                printf("    - se restablecen el stdin y el stdout con dup2 a las salvas comentadas anteriormente\n");
                printf("    - se pone el pipe de lectura en el stdin en caso de que sea necesario\n");
                printf("    - se analiza la relacion entre atomos\n");
                printf("       * en el caso del primer ciclo si se encuentra algun argumento separador da error\n");
                printf("    - se procesa el atomo poniendo las redirecciones necesarias y teniendo en cuenta que va a dejar el siguiente argumento\n");
                printf("    a procesar en un argumento separador o al final\n");
                printf("    - en caso de ser built-in se pasa al ciclo siguiente\n");
                printf("    - se analiza si el argumento separador que viene es un pipe y en caso afirmativo se instala el pipe\n");
                printf("    - si no es built-in se llama fork y execv con los argumentos ya recogidos en el process_atom\n");
                printf("\n");
                printf("\n");
                return 0;
            }
            return -1;
        }

        
        return -1;
    }
    return 1;
}

/*-1 si hay error de sintaxis, 0 si es built-in y 1 si no lo es
**si no es built-in pone los argumentos del execv en arg**/
int process_atom(cmd_state* cmd,linked_list* args)
{
    int count_arg = 0;
    linked_node* last_arg = cmd->current_arg;
    while (last_arg != NULL && strcmp(last_arg->value, "||") != 0 &&  strcmp(last_arg->value, "&&") != 0 &&
        strcmp(last_arg->value, ";") != 0 &&  strcmp(last_arg->value, "then") != 0 && 
        strcmp(last_arg->value, "else") != 0 && strcmp(last_arg->value, "end") != 0 &&
        strcmp(last_arg->value, "if") != 0 && strcmp(last_arg->value, "|") != 0)
    {
        count_arg++;
        last_arg = last_arg->next;
    }
    int result = built_in(cmd,count_arg);
    if(result < 0)
    {
        return -1;
    }
    if(result == 1)
    {
        int fd;
        while (cmd->current_arg != last_arg)
        {
            if(strcmp(cmd->current_arg->value,">") == 0)
            {
                cmd->current_arg = cmd->current_arg->next;
                if(cmd->current_arg == last_arg)
                {
                    return -1;
                    break;
                }
                fd = open(cmd->current_arg->value,O_CREAT | O_WRONLY,S_IRWXU);
                dup2(fd,1);
                close(fd);
                cmd->current_arg = cmd->current_arg->next;
                continue;
            }
            if(strcmp(cmd->current_arg->value,"<") == 0)
            {
                cmd->current_arg = cmd->current_arg->next;
                if(cmd->current_arg == last_arg)
                {
                    return -1;
                    break;
                }
                if(access(cmd->current_arg->value,F_OK) != 0)
                {
                    return -1;
                    break;
                }
                fd = open(cmd->current_arg->value,O_RDONLY,0);
                dup2(fd,0);
                close(fd);
                cmd->current_arg = cmd->current_arg->next;
                continue;
            }
            if(strcmp(cmd->current_arg->value,">>") == 0)
            {
                cmd->current_arg = cmd->current_arg->next;
                if(cmd->current_arg == last_arg)
                {
                    return -1;
                    break;
                }
                if(access(cmd->current_arg->value,F_OK) != 0)
                {
                    fd = open(cmd->current_arg->value,O_CREAT | O_WRONLY,S_IRWXU);
                    dup2(fd,1);
                    close(fd);

                }
                else
                {
                    fd = open(cmd->current_arg->value,O_APPEND,0);
                    dup2(fd,1);
                    close(fd);

                }
                cmd->current_arg = cmd->current_arg->next;
                continue;
            }
            add_last(args, cmd->current_arg->value);
            cmd->current_arg = cmd->current_arg->next;
        }
        return 1;  
    }
    return 0; 
}

/*retorna 1 si dio error en el execv
  retorna 0 si el comando se ejecuto correctamente
  retorna -1 si hubo algun error de sintaxis*/
int process_cmd(cmd_state* cmd)
{
    if(strcmp(cmd->arg_list->first->value,"#") == 0)
    {
        return 0;
    }

    if(strcmp(cmd->arg_list->last->value,"&") == 0)
    {
        remove_last(cmd->arg_list);
        int pid = -1;
        if((pid = add_job()) != 0)
        {
            return 0;
        }
        else
        {
            if(process_cmd(cmd) < 0)
            {
                printf("sintax _error");
            }
            exit(0);
        }
    }
    int result;
    linked_list* args_list;
    bool have_pipe = false;
    int fd_read_pipe = -1;
    while(cmd->current_arg != NULL)
    {
        args_list = create_linked_list();
        dup2(3,0);
        dup2(4,1);
        if(have_pipe)
        {
            dup2(fd_read_pipe,0);
            close(fd_read_pipe);
            have_pipe = false;
        }
        if(strcmp(cmd->current_arg->value,"&&") == 0)
        {
            if(cmd->last_result == -1)
            {
                return -1;
            }
            if(cmd->current_arg->next == NULL)
            {
                return -1;
            }
            cmd->current_arg = cmd->current_arg->next;
            if(cmd->last_result == 0)
            {
                result = process_atom(cmd,args_list);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if(strcmp(cmd->current_arg->value,"||") == 0)
            {
                if(cmd->last_result == -1)
                {
                    return -1;
                }
                if(cmd->current_arg->next == NULL)
                {
                    return -1;
                }   
                cmd->current_arg = cmd->current_arg->next;
                if(cmd->last_result == 1)
                {
                    result = process_atom(cmd,args_list);
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                if(strcmp(cmd->current_arg->value,";") == 0)
                {
                    if(cmd->current_arg->next == NULL)
                    {
                        return 0;
                    }
                    cmd->current_arg = cmd->current_arg->next;
                    result = process_atom(cmd,args_list);
                }
                else
                {
                    if(strcmp(cmd->current_arg->value,"|") == 0)
                    {
                        return -1;
                    }
                    else
                    {
                        result = process_atom(cmd,args_list);
                    }
                }
            }
        }
        
        if(result == -1)
        {
            return -1;
        }
        
        if(cmd->current_arg != NULL && strcmp(cmd->current_arg->value,"|") == 0)
        {
            if(cmd->current_arg->next == NULL)
            {
                return -1;
            }
            if(result == 0)
            {
                return -1;
            }
            cmd->current_arg = cmd->current_arg->next;
            int* pipe_fd = malloc(sizeof(int)*2);
            pipe(pipe_fd);
            dup2(pipe_fd[1],1);
            close(pipe_fd[1]);
            have_pipe = true;
            fd_read_pipe = pipe_fd[0];
            free(pipe_fd);
        }

        if(result == 1)
        {
            if(args_list->count == 0)
            {
                return -1;
            }
            if(strlen(args_list->first->value) >= 3 &&  ((char*)args_list->first->value)[0] == '.' &&
            ((char*)args_list->first->value)[1] == '/' )
            {
                if(access(args_list->first->value,F_OK) != 0)
                {
                    return -1;
                }
            }
            else
            {
                char* path = malloc(sizeof(char)*(strlen(args_list->first->value) + strlen("/bin/")));
                strcat(path,"/bin/");
                strcat(path,args_list->first->value);
                if(access(path,F_OK) != 0)
                {
                    return -1;
                }
                args_list->first->value = path;
            }
            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask,SIGCHLD);
            sigprocmask(SIG_BLOCK,&mask,NULL);
                        
            if((pid_fg = fork()) == 0)
            {
                sigprocmask(SIG_UNBLOCK,&mask,NULL);
                char** args = malloc(sizeof(char*)*(args_list->count + 1));
                linked_node* current = args_list->first;
                for(int i = 0; i < args_list->count;i++)
                {
                    args[i] = current->value;
                    current = current->next;
                }
                args[args_list->count] = NULL;
                if(execv(args[0],args) < 0)
                {
                    exit(1);
                }
            }
            else
            {
                int stat;
                while(waitpid(pid_fg,&stat,0) < 0)
                {
                    if(errno == ECHILD)
                    {
                        break;
                    }
                }
                pid_fg = -1;
                has_sigint = false;
                sigprocmask(SIG_UNBLOCK,&mask,NULL);
                
                if(WIFEXITED(stat))
                {
                    cmd->last_result = WEXITSTATUS(stat);
                }
                else
                {
                    cmd->last_result = 1;
                }
            }
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    init_history();
    init_jobs();
    dup2(0,3);
    dup2(1,4);
    while (true)
    {
        char* path = malloc(sizeof(char)*100);
        getcwd(path,sizeof(char)*100);
        printf("my_shell:%s$ ",path);
        fflush(stdout);
        free(path);
        char* buffer = get_line(0);
        cmd_state* cmd = create_cmd_state(buffer);
        if(cmd->arg_list->count == 0)
        {
            continue;
        }
        if(strcmp(cmd->arg_list->first->value,"history") == 0 && buffer[0] != ' ')
        {
            add_history(buffer);
        }
        
        int result = process_cmd(cmd);
        dup2(3,0);
        dup2(4,1);       

        if(result == -1)
        {
            printf("sintax error\n");
        }
        if(buffer[0] != ' ')
        {
            if(strcmp(cmd->arg_list->first->value,"again" ) == 0 )
            {
                if(cmd->arg_list->first->next == NULL)
                {
                    printf("sintax error\n");
                    continue;
                }
                cmd->current_arg = cmd->arg_list->first->next;
                char* history = remove_first_arg(buffer);
                history = remove_first_arg(history);
                int result = 0;
                char* again_cmd = get_history(strtol(cmd->current_arg->value,NULL,10) - 1,&result);
                if(result < 0)
                {
                    continue;
                }
                if(history == NULL)
                {
                    add_history(again_cmd);
                    free(history);
                    free(again_cmd);
                }
                else
                {
                    realloc(again_cmd,strlen(history) + strlen (again_cmd));
                    strcat(again_cmd,history);
                    add_history(again_cmd);
                    free(history);
                    free(again_cmd);
                }
            }
            else
            {
                if(strcmp(cmd->arg_list->first->value,"history") != 0)
                {
                    add_history(buffer);
                }
            }   
        }
        free(buffer);
    }
    return 0;
    
}