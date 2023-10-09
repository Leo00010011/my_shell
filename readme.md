# Comandos disponibles:
- **help**: imprimir para ayuda general
- **history**: para imprimir el historial de comandos
- **again** \<número\>: ejecuta el comando indicado del historial
- **fd**: pone en el foreground el proceso más reciente del background
- **fd \<pid\>**: pone en el foreground el proceso indicado
- **jobs** : muestra los procesos en el background
- **cd**: para cambiar el current work directory
- **ls**: para listar los archivos de la carpeta actual
- **exit**: para cerrar el shell
- **cmd > file**: para escribir el output de cmd en file
- **cmd < file**: para recibir el file como input del cmd
- **cmd >> file**: para añadir al final del file en caso de que exista
- **cmd1 | cmd2**: para establecer la salida de cmd1 es la entrada de cmd2
- **cmd1 ; cmd2**: ejecuta el cmd2 después de haber terminado el cmd1
- **cmd1 && cmd2**: si el primer comando termina sin errores entonces se ejecuta el segundo
- **cmd1 || cmd2**: si el primer comando termina con errores entonces se ejecuta el segundo
- **cmd1 < file1 | cmd2 > file2 || cmd3 > file3 < file4 && cmd5**: Se pueden combinar las concatenaciones

# Detalles de la Implementación:
## Basic:
 Para los comentarios en el primer momento que se tiene la entrada parseada se chequea el primer argumento. Para interpretar la entrada separo el comando en conjuntos (que les puse átomos)los átomos son las partes de la instruccion que pueden depender de un solo **execv**,es decir las partes de la instruccion que estan delimitadas por: '&&' ,';' ,'||' ,'|' principio o final.(para ver detalles del parseo ver spaces y el porque los átomos ver chain) Luego al interpretar un átomo compruebo si el primer parametro es un **built-in** y ahi se van los casos de **cd** y **exit**. En el caso de **cd** solo se comprueba si esta el path y solo el path ademas de el en su átomo y se usa **chdir** para cambiar el **cwd** del proceso actual en la que se basa la direccion del shell. En el **exit** se hace algo semejante paracomprobar la sintaxis y en caso de ser correcta se guarda el **history** y se manda a terminar los procesos en **background** después se llama **exit(0)**.
 
 En el caso de las **redirecciones de entrada** la solucion fue similar, exeptuando la parte del comportamiento. Cuando un átomo no es **built-in** y se va a procesar un átomo se tiene la lógica siguiente: 
 - se recorre el átomo argumento por argumento    

     - si se encuentra un '>', '<' o '>>' intepreta el argumento que le sigue como el archivo    al que se refiere.    

     - todo lo que no tenga antes algúno de los anteriores sera **argv** para el **execv**
  
Para poder **redireccionar la entrada y la salida** al principio de la ejecucion del shell se duplican el **stdin** y el **stdout** en los **fd 3** y **4** respectivamente y al final de procesar cada átomo se copian en su lugar para restablecerlos en caso de que hayan sido modificados.por algúna **redireccion** o un **pipe**.

Cada vez que en el recorrido se encuentra un '>', '<' o '>>' se tiene el sgte comportamiento

**> :** se usa **open** con el path que indique el argumento que le sigue la opción **O_CREAT** | **OWRITE**  y el permiso **S_IRWXU**  para crear o sobrescribir el archivo indicado y obtener un **fd**.

**< :** en el este caso se comprueba si el archivo es accesible con **access** del **unistd** en caso de no serlo se lanza un error de sintaxis, en caso de serlo se llama **open** con **O_READ** y el path que indica el argumento que le sigue.

**>> :** en este caso se comprueba si el archivo es accesible de la misma forma que con el anterior en caso de no ser accesible se crea como en  '>' y en caso de serlo se abre con **open** y la opción **O_APPEND**.

En todos los casos anteriores se hace **dup2** con el **fd** que da el open en el **stdout** en el caso de '>>' y '>'; y en el **stdin** en el caso de '<' y después se cierra el **fd** que dio el open.

Los **pipes** se explicaran en **multi-pipe**

Después de procesar el átomo se ve si necesita **pipes**(ver **multi-pipe**) y se instala si es necesario y se llama **execv** con el **argv** que se ha formado, si **arg[0]** empieza con './' se considera que se quiere llamar un ejecutable que esta en **cwd**, en caso contrario se considera que se quiere uno de **bin**.

## Spaces:
Para leer la entrada se implementó un método llamado **get_line** cuyó source code esta en **input_reader.c**, el cual va leyendo el **stdin** en conjuntos de 100 bytes del **fd** y llevando lo  que encuentra en un **buffer**, hasta que encuentra el cambio de linea. Después se crea una estructura con la informacion del comando donde se lleva el siguiente argumento a procesar la si el átomo anterior devolvió 0 o 1 y una **linked list** de argumentos. La **linked list** de argumentos se construye con el metodo **split** que esta **input_reader.c**
 
Luego el procesamiento del comando ocurren en 3 metodos, **process_cmd** se encarga de la relación entre los átomos, **process_atom** que se encarga de poner las **redirecciones** y ver si el átomo es un **built-in**, y **built-in** que se encarga de ver si el argumento dado es **builtin** y en caso de serlo lo procesa.(ver **chain**) 

## Multi-pipe:
El comportamiento del **pipe** se decide después de procesar el átomo(poner las **redirecciones** ver **basic**). Cuando se procesa el átomo, el argumento a procesar(ver **spaces**) queda en un separador(ver **chain**) o en un **pipe**, si termina en un **pipe** lo que se hace es crear el **pipe** y poner la parte de  escritura en el **stdout** para que en **execv** que viene después el proceso escriba. Y se guarda en una variable que hay un **pipe de lectura** y el **fd** de ese **pipe** y se avanza de argumento a procesar en la estructura que lleva la info del procesamiento de la entrada(ver **spaces**), que al ver que hacer con  el siguiente átomo lo capte y ponga el **pipe de lectura** en el **stdin** y lo cierre, el **stdout** va a estar correctamente en ese momento de la ejecucion porque después de ejecutar cada átomo se restablecen el **stdin** y el **stdout** (ver **chain**). 

No estan implementados para ser usados con **built-in** 

## Background:
La mayor parte del código relacionado con esta funcionalidad esta en **jobs.c** y la implementacion consiste en una **pila** y varios metodos para gestionar los **jobs**. Para detectar que se quiere mandar un proceso al  **background** se analiza el último argumento justo después del parseo y en caso de serlo se 'crea' el **job** y se procesa el comando dentro de el, para resolver el caso de los **chains** dentro de **jobs**. 

Para 'crear' **job** se usa el metodo **add_job** que consiste en un **fork** y hacer **push** al **pid** en la **pila** de **jobs** pero con el **SIGCHLD** bloqueado para evitar la **condicion de carrera**, en que el proceso que se manda al **background** termina antes de que se añada a la **pila** cuando el **handler** procesa la **señal** el  proceso no se encuentra en la **pila** de **jobs** y después cuando se vuelve al flujo original del proceso padre se añade un **job** que ya termino a la **pila** y es un **job** que nunca va a ser retirado 

En el caso de los **fg**, pues el **fg** sin parametro llama al **fg** con parametro al con el **pid** que esta en el tope de la **pila** de **jobs**, y lo que hacen es bloquear el **SIGCHLD** y esperar cualquier proceso con **waitpid** y **arg -1** cuando llegue algún proceso se remueve de la **pila** comprueba si es el que se pidio para **foreground** y en caso afirmativo deja de esperar a que se termine algún proceso 

Para mantener la pila actualizada se usa un **handler** de **SIGCHLD** en que se hace **waitpid** **-1** con **WNOHANG** tantas veces como de un valor positivo, y asi se **cosechan** todos los posibles procesos que mandaron **SIGCHLD** y no han sido atedidos. Se usa **WNOHANG** para no entrar en conflicto con el **waitpid** de algún proceso que se llame en el **foreground**, pues si llega un **SIGCHLD**  este puede ser de un proceso en **background** o de el de **foreground** en el caso de que sea el de **foreground** en el momento en que se hace en **contex switching** se procesa primero el **handler** y este lo que haria seria **cosechar** el proceso actual y como dio positivo esperaría a que termine otro hijo que seria uno de **background**, bloqueando en shell hasta que todo proceso en **background** termine,por eso el **WNOHANG** 

## History
Los códigos fuentes de history esta en **history.c** y a su vez usa unos códigos para leer y escribir variables en archivos que estan en **save.c**. Mi implementacion de **history** consiste en una **cola circular** de tamaño fijo y cada vez que se lee una linea del **shell** para ser procesada esta es puesta en la cola. En el caso de los comandos que empiezan con **again**, este y su argumento son removidos en el usando una funcion que esta en **input_reader.c** 

Para salvar las variables se escriben los datos en el siguiente formato \<size\>\<value\>\<size\>\<value\>..., donde size es un **unsigned int** para indicar la cantidad de **bytes** del valor a leer, entonces la lectura consiste en leer un **unsigned int** del archivo y leer a continuación esa cantidad de **bytes** del archivos y guardarlos en una  **memoria reservada** con **malloc** y ese **puntero** se le entrega al usuario 

En el caso de que el comando que llaman comienza con **history** se hace es guardar primero el **history** y después ejecutar el comando para que **history** quede de último, y en el caso de que no primero se ejecuta el comando y  después se guarda en el **history**, esto se hizo para evitar confunciones con el **again**, pues si se llama **again** a un número y hay 10 argumentos su posicion va a ser una menos que la mostrada si se llama **history** justo anteriormente  

El **again** lo que hace es pedirle el comando al **history**, parsearlo y ejecutarlo reutilizando funciones ya implementadas por el **shell** para ejecutar sus comandos. 

## Chain
Para implementar **chains** y **multi-pipe** se interpretó el código en cadenas de átomos relacionadas entre si, donde  átomo es toda aquella informacion que solo tiene que ver con un **execv** o un **built-in**, en otras palabras todo aquello que esta entre '||', '&&', ';' o '|', de esta forma los comandos se pueden ejecutar en un bucle donde se procesa un átomo se ejecuta y se define su relación con el siguiente (en el caso de los **pipes** la relación se define antes de ejecutar). Esto se implementó utilizando la estrucura comentada en **spaces** en la que se lleva en una variable el argumento a procesar, y se trabaja con la **invariante** de que cuando se llama **process_atom** esta va quedar apuntando a un argumento de separacion o a **NULL** en el caso de que termine. Para saber el comportameinto se utiliza el valor de la variable **last_result** en la que se guarda si el último átomo ejecutado retorno 0 o 1 y se asume que los **bult-in** siempre retornan 1 

Para ejecutar los **chain** todo se necesita poner **pipes** o archivos en el **stdin** y el **stdout**, para poder recuperarlos  después se le hace un **dup2** antes de ejecutar el shell en las posiciones 3 y 4 respectivamente, para después de cada ejecucion se restablezcan ya que en el único caso que se necesitaría algúna modificación de la **tabla de fd** del átomo anterior seria en los **pipes** y esto se hace en el ciclo siguinte de la manera descrita a continuación 
- todo lo que esta a continuación se va a ejecutar en un ciclo hasta que se acaben los argumentos
   *   se restablecen el **stdin** y el **stdout** con **dup2** a las salvas comentadas anteriormente
   *  se pone el **pipe** de lectura en el **stdin** en caso de que sea necesario
   *  se analiza la relación entre átomos        
     \* en el caso del primer ciclo si se encuentra algún argumento separador da error
  *  se procesa el átomo poniendo las **redirecciones** necesarias y teniendo en cuenta que va a dejar el siguiente argumento a procesar en un argumento separador o al final
  * en caso de ser **built-in** se pasa al ciclo siguiente
  * se analiza si el argumento separador que viene es un **pipe** y en caso afirmativo se instala el **pipe**
  * si no es **built-in** se llama **fork** y **execv** con los argumentos ya recogidos en el **process_atom** 





