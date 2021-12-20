# Proyecto 1 -- Manejo de Implementación de Archivos
Primer Proyecto de Manejo e implementación de Archivos.  Vacaciones segundo semestre 2021

## Manual de usuario:
En este manual de usuario se explicara cuales son las funcionalidas con las que cuenta el ejecutable y la forma de utilizarlo correctamente:

### Ejecucion:

1. Se descarga el proyecto del repositorio
2. Se abre la terminal en la ubicacion de la carpeta donde se aloja el archivo Principal.cpp y el ejecutable **Principal**
3. Ejecutar el comando: **./Principal** para abrir el programa e interactuar con el:

### Forma correcta del uso del ejecutable:

Al tener ejecutado el programa en la terminal, les mostrara:

![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/Consola%20principal.png)

Los comandos disponibles para esta version son:

- mkdisk 
- rmdisk 
- fdisk 
- exec 
- pause 
- mount 
- unmount 
- mkfs
- login 
- logout 
- mkgrp 
- rmgrp 
- mkusr 
- rmusr 
- chmod 
- mkfile 
- mkdir 
- rep 
- pause

Estos comandos son los que reconoce la consola

importante: **ESTRUCURA QUE TIENEN QUE LLEVAR LOS COMANDOS ESTAN EN EL ENUNCIADO DEL PROYECTO**

## comentarios:
Tiene que ir luego del simbolo **#** y eso no tiene que afectar la entrada del analizador

Ejemplo: 
 - #ejecuta el script
 - #llevo mi numeral antes entonces todo esto es un comentario



## Forma de utilizar los comandos
Utilizacion del comando **Mkdirsk**:
- La funcionalidad de este comando es crear un disco en la ubicacion que se le indica en el path.
La salida de este comando muestra el tamño, tipo de particion y fecha que se crea

![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/Consola%20principal.png)



### comando **Mkdirsk**: 

Este parámetro elimina un archivo que representa a un disco duro mostrando un
mensaje de confirmación para eliminar. Tendrá los siguientes parámetros:



### comando **FDISK**: 
Este comando administra las particiones en el archivo que representa al disco duro.
Deberá mostrar un error si no se pudo realizar la operación solicitada sobre la
partición, especificando por qué razón no pudo crearse (Por espacio, por
restricciones de particiones, etc.).

Ejemplo: fdisk -size~:~100 -unit~:~K -fit~:~BF -path~:~/Disco1.disk -name~:~Particion3


### comando **MOUNT**:
Este comando montará una partición del disco en el sistema. Cada partición se
identificará por un id que tendrá la siguiente estructura: VD + LETRA + NUMERO.
Por ejemplo: vda1, vda2, vdb1, vdc1… La letra será la misma para particiones en el
mismo disco y el número diferente para particiones en el mismo disco.

Ejemplo: 
#Monta las particiones de Disco1.dsk
- mount -path~:~/home/Disco1.disk –name~:~Part1 #id=vda1
- mount -path~:~/home/Disco2.disk –name~:~Part1 #id=vdb1
- mount -path~:~/home/Disco3.disk –name~:~Part2 #id=vdc1
- mount -path~:~/home/Disco1.disk –name~:~Part2 #id=vda2


### comando **UNMOUNT**:
Desmonta una partición del sistema. Se utilizará el id que se le asignó a la partición
al momento de cargarla.

Ejemplo:
- unmount -id~:~vda1 #Desmonta la partición con id vda1 (En Disco1.dsk)


### comando **Mkfs**:
Este comando realiza un formateo completo de la partición, se formateará como
ext2 por defecto si en caso el comando fs no está definido.

Ejemplo:
- mkfs -id~:~vda1 -fs~:~2fs -type~:~fast #Este es formateado con ext2
- mkfs -id~:~vda1 -fs~:~3fs -type~:~fast #Este es formateado con ext3


### comando **Login**:
Este comando se utiliza para iniciar sesión en el sistema. No se puede iniciar otra
sesión sin haber hecho un logout antes, si no, debe mostrar un mensaje de error
indicando que debe cerrar sesión antes.

Ejemplo:

#Se loguea en el sistema como usuario root
- login -usr~:~root -pwd~:~123 -id~:~vda1
- login -usr~:~"mi usuario" -pwd~:~"mi pwd" -id~:~vdb2


### comando **logout**:
Este comando se utiliza para cerrar sesión. Debe haber una sesión activa
anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error.

Ejemplo:

#Termina la sesión del usuario Logout
- Logout


### comando **mkgrp**:
Este comando creará un grupo para los usuarios de la partición y se guardará en el
archivo users.txt de la partición, este comando solo lo puede utilizar el usuario root.
Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error, si el grupo
a ingresar ya existe deberá mostrar un mensaje de error.

Ejemplo:
#Crea el grupo usuarios en la partición de la sesión actual
- mkgrp -name~:~"usuarios"


### comando **rmgrp**:
Este comando eliminará un grupo para los usuarios de la partición. Solo lo puede
utilizar el usuario root, si lo utiliza alguien más debe mostrar un error.

Ejemplo:
#Elimina el grupo de usuarios en la partición de la sesión actual
- rmgrp -name~:~usuarios



### comando **mkusr**:
Este comando crea un usuario en la partición. Solo lo puede ejecutar el usuario root,
si lo utiliza otro usuario deberá mostrar un error.

Ejemplo:
#Crea el grupo usuarios en la partición de la sesión actual
- Mkusr -usr~:~user1 -grp~:~usuarios -pwd~:~usuario


### comando **rmusr**:
Este comando elimina un usuario en la partición. Solo lo puede ejecutar el usuario
root, si lo utiliza otro usuario deberá mostrar un error.

Ejemplo:

#Elimina el usuario user1 de la partición de la sesión actual
- rmusr -usr~:~user1


### comando **chmod**:
Cambiará los permisos de uno o varios archivos o carpetas. Lo podrá utilizar el
usuario root en todos los archivos o carpetas y también lo podrán utilizar otros
usuarios, pero solo sobre sus propios archivos.

Ejemplo:

#Cambia los permisos de la carpeta home recursivamente #Todos los archivos o
carpetas que tengan /home cambiarán #Por ejemplo si existiera
- chmod -id~:~vda1 -path~:~/home -ugo~:~777

### comando **mkfile**:
Este comando permitirá crear un archivo, el propietario será el usuario que
actualmente ha iniciado sesión. Tendrá los permisos 664. El usuario deberá tener el
permiso de escritura en la carpeta padre, si no debe mostrar un error.

Ejemplo:

#Crea el archivo a.txt
-mkFile -SIZE~:~15 -PatH~:~/home/user/docs/a.txt –p




### comando **mkdir**:
Este comando es similar a mkfile, pero no crea archivos, sino carpetas. El propietario
será el usuario que actualmente ha iniciado sesión. Tendrá los permisos 664. . El
usuario deberá tener el permiso de escritura en la carpeta padre, si no debe mostrar
un error.

Ejemplo:

#Crea la carpeta usac
#Si no existen las carpetas home user o docs se crea
- Mkdir -P -id~:~vda1 -path~:~/home/user/docs/usac



### comando **EXEC**:
El programa podrá ejecutar scripts con el comando exec. Debe mostrar el
contenido de la línea que está leyendo y su resultado. También debe mostrar los
comentarios del script.

Ejemplo:

#ejecuta el script
- exec -path=/home/Desktop/calificacion.sh



### comando **rep**:
Recibirá el nombre del reporte que se desea y lo generará con graphviz en una
carpeta existente.

Ejemplo:
Nombre del reporte a generar. Tendrá los siguientes
valores:
- MBR
- DISK
- inode
- Journaling
- block
- bm_inode
- bm_block
- tree
- sb
- file
- ls


-rep -id~:~vda2 -Path~:~"/home/user/reports/reporte 2.pdf" -name~:~ls -ruta~:~"/home/mis documentos"


## representacion de los comandos en consola:

![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/mkdisk.png)



![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/comando%20consola%201.png)



![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/comando%203.png)



![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/comando%204.png)



![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/comando%205.png)


![consola](https://github.com/Julio-OrellanaR/Proyecto1/blob/main/Imagenes/comando%206.png)
