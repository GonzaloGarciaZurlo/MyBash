# lab1-mybash

## Introducción
En este primer laboratorio (mybash) tenemos el objetivo de implementar y entender la estructura interna que posee un bash, asi como ademas entender como se ejecutan los comandos que vienen por defecto en un sistema operativo y sus respectivas llamadas a sistema que utilizan.     
Ademas de lo anterior dicho, buscamos implementar buenas practicas de programacion en un entorno de trabajo en equipo.   

## Uso
Para compilar el proyecto basta con ejecutar `make` en el directorio base, soporta x86 tanto 32 como 64 bits. 
Luego correr `./mybash`

Nuestro bash soporta tanto comandos simples con redirección (con los simbolos > <) , como estos concatenados con cualquier cantidad de pipes.    
Tambien en el prompt muestra el nombre de usuario y el directorio en el que se esta parado. 

---
## Modularización
Buscamos tener una buena organizacion en cada uno de los archivos que teniamos que modificar. Esto a partir de:

* Mantener un orden en cuanto a donde crear las funciones nuevas que necesitemos.

* Hacer que cada funcion la podamos utilizar para varias cosas independientemente del contexto en el cual se use.
   
* Tener un codigo facil de leer (para facilitar el trabajo en equipo) a partir de nombres representativos de las variables o crear nombres para una variable que resuman una linea de codigo para no volver a repetirla en el futuro, etc.    

Se utilizaron muchas funciones publicas provenientes de Glib principalmente aunque ademas creamos varias funciones nuevas para diferentes necesidades que se nos fueron presentando.

---
## Técnicas de Programación

En cuanto a la libreria mas destacable que utilizamos fue **Glib** que nos ayudo a manejar la estructura basica de los TADs implementados a partir del uso de las colas.
  

En cuanto al manejo de la memoria dinamica tratamos siempre de trabajar lo mas prolijo que sea posible, a partir de crear funciones auxiliares que liberen la memoria utilizada en ese entonces, para evitar el uso innecesario de llamadas a free(). Ademas de utilizar Valgrind cada vez que trabajamos alocando memoria para alguna funcion.  
Con respecto a los errores que pueden generar las entradas de los usuarios o el mismo sistema, tratamos de ser lo mas cautelosos posibles y tener en cuenta todos los factores que pueden llevar a un error. Para esto utilizamos algunas tecnicas como:

* La funcion assert para asegurarnos de que se cumplan tanto las precondiciones como las postcondiciones de una funcion.

* Mensajes de error a la hora de hacer llamadas al sistema (por si alguna de estas falla).

* Asegurarnos de probar nosotros mismos distintos tipos de entrada para ver como se comportan las funciones.    

## Herramientas de Programación
En cuanto a las herramientas que utilizamos destacan principalmente:

* **GCC:** Cumplio la funcion de compilar todos los archivos .c (incluido en el makefile). 

* **GDB:** Lo utilizamos para debuggear los programas a la hora de buscar algun fallo a corregir. (muy util utilizar la opción para seguir la ejecución de los procesos hijos `set follow-fork-mode child`)

* **Valgrind:** Nos ayudo en la depuracion de problemas de memory leaks.

* **VS CODE:** Fue el editor de texto utilizado por todos los integrantes, el cual nos ayudo a agilizar los desarrollos y ordenar el codigo.


---
## Desarrollo

Fue desarrollado por:
### Los Hackermans

* Federico Virgolini <fvirgolini@gmail.com>
* Gonzalo Garcia Zurlo <gonzalogarciazurlo@gmail.com>
* Santiago Monserrat Campanello <smonserratcampanello@gmail.com>

### ***TADs scommand y pipeline***
Comenzamos desarrollando el TAD scommand. Este sirve para representar un comando simple utilizando una 3-upla de la forma: `([char*], char* , char*)`, al primer elemento lo utilizamos para almacenar los argumentos del comando, el segundo y tercero cumplen la funcion de recibir y redirigir los datos de entrada y de salida. Para la manipulacion de la lista (el primer elemento de la 3-upla) vimos conveniente tratar a esta como una cola, a parir de la implementacion Gqueue de la biblioteca Glib.  
Luego, el TAD pipline nos sirve para representar varios comandos y decidir si estos tienen que esperar o no a ser ejecutados, este tiene la forma: `([scommand], bool)`.Al igual que en scommand manipulamos la lista usando Gqueue.  
A partir de estas dos TADs implementamos las funciones propuestas, las cuales nos serviran para el desarrollo de los siguientes apartados. Ademas agregamos tres funciones extra, una para copiar un comando (copy_scommand), otra para listar un comando con sus respectivos argumentos (scommand_to_array) y otra para obtener el n comando que se encuentra en un pipeline (pipeline_get_nth_command),los cuales utilizaremos para el desarrollo del execute.  
Tuvimos algunos problemas a la hora de implementar las funciones scommand_to_string y pipeline_to_string debido a memory leaks causados por el uso de la funcion stmerge (proveniente de strextra). Esto lo solucionamos a partir de dos funciones auxiliares (strmerge_and_free y strmerge_and_free_first) las cuales llaman a la funcion strmerge original para luego liberar la memoria de los 2 parametros introducidos (en el caso de strmerge_and_free_first libera unicamente la memoria del primer parametro).

### ***Execute y builtin***
Comenzando por el execute, utilizamos la funcion scommand_to_array, creada anteriormente en el archivo command.c, para la implementacion de la funcion "execute_command". Esta cumple la funcion de detectar si existen archivos de entrada o de salida para luego redireccionar la memoria hacia ellos. Ademas ejecuta el execvp del comando introducido.  
Luego, implementamos la funcion "external_exec", en esta tiene el objetivo de ejecutar cualquier comando externo. Para ello definimos dos booleanos (before y after) los cuales nos serviran para recorrer los comandos del pipeline y para saber si existen comandos siguientes o anteriores a medida que se lo recorre. Si existe un comando siguiente se crea un nuevo pipe y se llama a la funcion fork. Analizando el proceso hijo, si hay un comando siguiente se cierra el comando anterior y se redirecciona con dup2 al nuevo comando hacia el archivo de salida para finalemnte cerrarlo, ademas si hay un comando anterior se cierra el comando actual y se redirecciona el comando anterior con el archivo de entrada y tambien lo cierra. Se sigue con ejecutar el comando con "execute_command" y se elimina el comando frontal del pipe para seguir reccoriendolo, luego volviendo con el proceso padre si existe comando anterior se cierran las dos direcciones del pipe viejo y espera a que termine el hijo. Por ultimo si hay un comando siguiente renombre a los comandos viejos como nuevos y se cambia el valor de before a true, para seguir recorriendo el pipe.       
Finalmente implementamos la funcion "execute_pipeline", la cual reune todas las funciones auxiliares creadas en el archivo y sirve para ejecutar un numero indefinido de pipelines. Esta empieza por corroborar si uno de los comandos es interno y si lo es, ejecuta la funcion "builtin_exec" implementada en el archivo builtin.c (cabe aclarar que el programa no funciona mezclando  comandos internos con externos, ejecutando solamente el primer comando interno). Despues, si todos los comandos son externos va recorriendo todos los pipelines ejecutando cada comando individualmente (a traves de una copia de cada comando), utilizando "external_exec".  
Para el archivo builtin.c, empezamos implementando las funciones que chequean si un comando del pipeline es interno (ya sea "cd" o "exit"), en el caso de reconocer el "exit", recorremos el pipeline con la funcion "pipeline_get_nth_command" implementada anteriormente, comparando si alguno de los comandos es efectivamente un "exit", despues en el caso de detectar el "cd" simplemente compara si el primer comando es cd, ya que si fuera introducido mas adelante se considera como un error de entrada. Luego, para la funcion "builtin_exec" simplemente ejecuta el comando cd a traves de chdir. Decidimos que no es necesario considerar el caso en el que tenemos que ejecutar el comando exit, ya que esto simplemente se chequea en el archivo main (mybash) a traves de la linea "quit = quit || builtin_is_exit(pipe);" que si la funcion builtin_is_exit devuelve true entonces se sale del comando while donde se estaba ejecutando para luego salir del bash.

---
## Conclusiones
Consideramos que este laboratorio nos ayudo a aprender y familiarizarnos con como funcionan los procesos, las llamadas a sistemas, los filedescriptors.  
Tambien nos ayudo a comprender un poco mas el funcionamiento de "C" y el poder que puede llegar a tener usando las ya mencionadas llamadas a sistema. 
Ademas nos ayudo a comprender como funciona por dentro un bash y sus funciones predeterminadas. 
Consideramos tambien muy importante la experiencia que nos genero poder trabajar en equipo para un mismo proyecto, a traves de herramientas colaborativas como BitBucket.