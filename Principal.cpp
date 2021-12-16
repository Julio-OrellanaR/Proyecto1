#include <iostream>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <time.h>
#include <fstream>
#include <string>
#include <ctype.h>
#include <cctype>
#include <bits/stdc++.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include <cmath>
#include <wchar.h>
#include <libgen.h>
using namespace std;

// --  GLOBALES--
//TODO: agregar el resto de comandos
vector<string> TodosComandos = {"mkdisk", "rmdisk", "fdisk", "exec", "pause", "mount", "unmount", "mkfs",
"login", "logout", "mkgrp", "rmgrp", "mkusr", "rmusr", "chmod", "mkfile", "mkdir", "rep", "pause"};
bool flag_login = false;
void CreatePartitionPimari(int, char, string, char, string);
void CreatePartitionExtend(int, char, string, char, string);
void createPartitionLogica(int, char, string, char, string);
bool existePARTITION(string, string);
int buscarLetra(string, string);
int buscarNumero(string, string);
void formatearEXT3(int, int, string);
void formatearEXT2(int, int, string);


//-- ESTRUCTURAS
//-- PARTITION --
struct PARTITION
{
    char part_status;
    char part_type;
    char part_fit;
    int part_start;
    int part_size;
    char part_name[16];
};


//-- MASTER BOOT RECORD -- MBR
struct MBR
{
    int MBR_tamanio;
    time_t MBR_fecha_creacion;
    int MBR_disk_signature;
    char disk_fit;
    PARTITION MBR_partition[4];
};

//-- EXTENDED BOOT RECORD -- EBR
struct EBR
{
    char EBR_part_status;
    char EBR_part_fit;
    int EBR_part_start;
    int EBR_part_size;
    int EBR_part_next;
    char EBR_part_name[16];
};

// -- MOUNT --
struct NDMOUNT
{
    string direccion;
    string nombre;
    string id;
    char letra;
    int num;
    int size = 0;
};
vector<NDMOUNT> Arreglomount;


//-- NODO DE MKFS --
struct NDMKFS
{
    string id;
    char tipo;
    string unit;
};
vector<NDMKFS> mkfs_init;

// -- Datos de usuario
struct Sesion{
    int id_user;
    int id_grp;
    int inicioSuper;
    int inicioJournal;
    int tipo_sistema;
    string direccion;
    char fit;
};

Sesion actualSesion;


//-- SUPER BLOQUE  --
struct SUPERBLOQUE
{
    int s_filesystem_type;
    int s_inodes_count;
    int s_blocks_count;
    int s_free_blocks_count;
    int s_free_inodes_count;
    time_t s_mtime;
    time_t s_umtime;
    int s_mnt_count;
    int s_magic;
    int s_inode_size;
    int s_block_size;
    int s_first_ino;
    int s_first_blo;
    int s_bm_inode_start;
    int s_bm_block_start;
    int s_inode_start;
    int s_block_start;
};


//-- TABLA DE INODOS --
struct INODOTABLA
{
    int i_uid;
    int i_gid;
    int i_size;
    time_t i_atime;
    time_t i_ctime;
    time_t i_mtime;
    int i_block[15];
    char i_type;
    int i_perm;
};


//-- BLOQUE DE ARCHIVOS --
struct BLOQUEARCHIVOS
{
    char b_content[64];
};


//-- CONTENT --
struct content
{
    char b_name[12];
    int b_inodo;
};


//-- BLOQUE DE CARPETAS --
struct BLOQUECARPETAS
{
    content b_content[4];
};

struct BLOQUEAPUNTADORES
{
    int b_pointers[16];
};


//-- JOURNALING --
struct JOURNAL
{
    char Journal_Tipo_Operacion[10];
    int Journal_tipo;
    char Journal_nombre[100];
    char Journal_contenido[100];
    time_t Journal_fecha;
    int Journal_propietario;
    int Journal_permisos;
};

struct USUARIO{
    int id_usr;
    int id_grp;
    char username[12];
    char password[12];
    char group[12];
};

//-- DISCO --
string path="/home/julio-or";
struct DISCO
{   
    int size;
    string fit;
    char unit = 'm'; // k= kilobytes, m= Megabytes
    string path;
}Discvar;

//-- CREACION DISCO --
void crearDisco(DISCO dc){
    cout<< "-----creacion disco------" << endl;
    string ruta = dc.path;
    char rutac[ruta.size()+1];
    strcpy(rutac, ruta.c_str());

    FILE *file = NULL;
    file = fopen(rutac, "r");
    if (file != NULL)
    {
        cout << "Ya existe el disco" << endl;
        return;
    }

    int tamanio;

    if (dc.unit == 'k') //kilobyte
    {
        tamanio = 1024 * dc.size;
    }else{  //megabyte 
        tamanio = 1024 * 1024 * dc.size;
        cout << "tamanio" << tamanio << endl;
    }

    file = fopen(rutac, "wb");
    fwrite("\0",1,1,file);

    fseek(file, tamanio, SEEK_SET);
    fwrite("\0", 1, 1, file);
    
    //se completo la retencion del disco, ahora modificamos el MBR

    MBR mbr;
    mbr.MBR_tamanio = tamanio;
    mbr.MBR_disk_signature = rand()%1000;
    mbr.MBR_fecha_creacion = time(0);

    for (int i = 0; i < 4; i++)
    {
        mbr.MBR_partition[i].part_status = '0';
        mbr.MBR_partition[i].part_size = 0;
        mbr.MBR_partition[i].part_fit = '0';
        mbr.MBR_partition[i].part_start = -1;
        strcpy(mbr.MBR_partition[i].part_name, "");
    }

    cout << "---- Disco Creado  ----" << "\n Disco \n" <<endl;
    cout << "fecha creacion: " << asctime(gmtime(&mbr.MBR_fecha_creacion)) << endl;
    cout << "Signature: " << mbr.MBR_disk_signature << endl;
    cout << "Tamaño: " << mbr.MBR_tamanio <<endl;
    cout << "Fit: " << mbr.disk_fit << endl;

    //escritura del mbr
    fseek(file, 0, SEEK_SET);
    fwrite(&mbr, sizeof(MBR), 1, file);
    fclose(file);
}

void CreatePartitionPimari(int size, char unit, string pathPart, char fit, string name){
    char auxUnit = 0;
    char auxFit = 0;
    string auxPath = pathPart;
    int size_bytes = 1024;
    char buff = '1';

    //definimos el fit que utilizara segun el parametro
    if (fit != 0)
    {
        auxFit = fit;
    }else{
        auxFit = 'W';

    }

    //definimos el unit
    if (unit != 0)
    {
        auxUnit = unit;
        if (auxUnit == 'B')
        {   
            size_bytes = size;
        }else if (auxUnit = 'K')
        {
            size_bytes = size * 1024;
        }else if (auxUnit == 'M')
        {
            size_bytes = size * 1024 * 1024;
        }else{
            cout << "ERROR" << endl;
        }
    }else{
        size_bytes = size * 1024;
    }

    FILE *fp;
    MBR mbr;
    
    if ((fp = fopen(auxPath.c_str(), "rb+")))
    {
        bool flagPartition = false;
        int indicadorPartition = 0;
        fseek(fp, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, fp);

        //revisamos particiones
        for (int i = 0; i < 4; i++)
        {
            if (mbr.MBR_partition[i].part_start == -1 )
            {
                flagPartition = true;
                indicadorPartition = i;
                break;
            }
            
            if ((mbr.MBR_partition[i].part_status == '1') && (mbr.MBR_partition[i].part_size >= size_bytes))
            {
                flagPartition = true;
                indicadorPartition = i;
                break;
            }
        }

        if (flagPartition)
        {
            int espaciousado = 0;
            for (int i = 0; i < 4; i++)
            {
                if (mbr.MBR_partition[i].part_status != '1')
                {
                    espaciousado += mbr.MBR_partition[i].part_size;
                }
            }
            
            cout << "Espacio disponible: " << (mbr.MBR_tamanio - espaciousado) << " Bytes" << endl;
            cout << "Espacio necesario:  " << size_bytes << " Bytes" << endl;

            //verificar espacios
            if ((mbr.MBR_tamanio - espaciousado) >= size_bytes)
            {
                if (!existePARTITION(pathPart, name))
                {
                    mbr.disk_fit = auxFit;


                    if (mbr.disk_fit  == 'F')
                    {
                        mbr.MBR_partition[indicadorPartition].part_type = 'P';
                        mbr.MBR_partition[indicadorPartition].part_fit = auxFit;

                        if (indicadorPartition == 0)
                        {
                            mbr.MBR_partition[indicadorPartition].part_start = sizeof(mbr);
                        }else{
                            int resultado = mbr.MBR_partition[indicadorPartition - 1].part_start + mbr.MBR_partition[indicadorPartition - 1].part_size;
                            mbr.MBR_partition[indicadorPartition].part_start = resultado;
                        }

                        mbr.MBR_partition[indicadorPartition].part_size = size_bytes;
                        mbr.MBR_partition[indicadorPartition].part_status = '0';
                        strcpy(mbr.MBR_partition[indicadorPartition].part_name, name.c_str());

                        //guarda el nuevo mbr
                        //apuntamos al inicio
                        fseek(fp, 0, SEEK_SET);
                        fwrite(&mbr, sizeof(MBR), 1, fp);
                        //se escribe la particion extendida
                        fseek(fp, mbr.MBR_partition[indicadorPartition].part_start, SEEK_SET);

                        EBR ebr;
                        ebr.EBR_part_fit = auxFit;
                        ebr.EBR_part_status = '0';
                        ebr.EBR_part_size = 0;
                        ebr.EBR_part_next = -1;
                        ebr.EBR_part_start = mbr.MBR_partition[indicadorPartition].part_start;
                        strcpy(ebr.EBR_part_name, "");
                        //escribimos archivo
                        fwrite(&ebr, sizeof(EBR), 1, fp);

                        for (int i = 0; i < (size_bytes - (int)sizeof(EBR)); i++)
                        {
                            fwrite(&buff, 1, 1, fp);
                        }

                        cout << "...\n" << "Particion primaria creada con exito" << endl;
 
                    }else if (mbr.disk_fit  == 'B')
                    {
                        int BestIndex = indicadorPartition;
                        for (int i = 0; i < 4; i++)
                        {
                            if (mbr.MBR_partition[i].part_start == -1 || (mbr.MBR_partition[i].part_status == '1' && mbr.MBR_partition[i].part_size >= size_bytes))
                            {
                                if (i != indicadorPartition)
                                {
                                    if (mbr.MBR_partition[BestIndex].part_size > mbr.MBR_partition[i].part_size)
                                    {
                                        BestIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        mbr.MBR_partition[BestIndex].part_type = 'P';
                        mbr.MBR_partition[BestIndex].part_fit = auxFit;


                        //start
                        if (BestIndex == 0)
                        {
                            mbr.MBR_partition[BestIndex].part_start = size_bytes;
                        }else{
                            int resultado = mbr.MBR_partition[BestIndex - 1].part_start + mbr.MBR_partition[BestIndex - 1].part_size;
                            mbr.MBR_partition[BestIndex].part_start = resultado;
                        }
                        
                        mbr.MBR_partition[BestIndex].part_size = size_bytes;
                        mbr.MBR_partition[BestIndex].part_status = '0';
                        strcpy(mbr.MBR_partition[BestIndex].part_name, name.c_str());
                        //colocamos el aputnador 
                        fseek(fp, 0, SEEK_SET);
                        fwrite(&mbr, sizeof(MBR), 1, fp);
                        //Guardamos nuevo apuntador de la nueva particion
                        fseek(fp, mbr.MBR_partition[BestIndex].part_start, SEEK_SET);
                        //escribimos en la nueva particion
                        for (int i = 0; i < size_bytes; i++)
                        {
                            fwrite(&buff, 1, 1, fp);
                        }
                        cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    
                    }else if (mbr.disk_fit  == 'W')
                    {
                        int worstIndex = indicadorPartition;

                        for (int i = 0; i < 4; i++)
                        {
                            if (mbr.MBR_partition[i].part_start == -1 || (mbr.MBR_partition[i].part_status == '1' && mbr.MBR_partition[i].part_size >= size_bytes))
                            {
                                if (i != indicadorPartition)
                                {
                                    if (mbr.MBR_partition[worstIndex].part_size < mbr.MBR_partition[i].part_size)
                                    {
                                        worstIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        mbr.MBR_partition[worstIndex].part_type = 'P';
                        mbr.MBR_partition[worstIndex].part_fit = auxFit;

                        //start
                        if (worstIndex == 0)
                        {
                            mbr.MBR_partition[worstIndex].part_start = sizeof(mbr);
                        }else{
                            int resultado = mbr.MBR_partition[worstIndex - 1].part_start + mbr.MBR_partition[worstIndex - 1].part_size;
                            mbr.MBR_partition[worstIndex].part_size = resultado;
                        }

                        mbr.MBR_partition[worstIndex].part_size = size_bytes;
                        mbr.MBR_partition[worstIndex].part_status = '0';
                        strcpy(mbr.MBR_partition[worstIndex].part_name, name.c_str());

                        //guardamos 
                        fseek(fp, 0, SEEK_SET);
                        fwrite(&mbr, sizeof(MBR), 1, fp);

                        //mandamos el nuevo aputnador para guardar
                        fseek(fp, mbr.MBR_partition[worstIndex].part_start, SEEK_SET);

                        //escribimos en el MBR
                        for (int i = 0; i < size_bytes; i++)
                        {
                            fwrite(&buff, 1, 1, fp);
                        }

                        cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                        
                    }
                }else
                {
                    cout << "ERROR ya existe una particion con ese nombre" << endl;
                }
            }else
            {
                cout << "ERROR la particion a crear excede el espacio libre" << endl;
            }
        }else{
            cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
            cout << "Elimine alguna para poder crear una" << endl;
        }
        fclose(fp);
    }else{
        cout << "ERROR no existe el disco" << endl;
    }
}

void CreatePartitionExtend(int size, char unit, string pathPart, char fit, string name){
    char auxUnit = 0;
    char auxFit = 0;
    string auxPath = pathPart;
    int size_bytes = 1024;
    char buff = '1';

    //definimos el fit que utilizara segun el parametro
    if (fit != 0)
    {
        auxFit = fit;
    }else{
        auxFit = 'W';
    }
    
    //definimos el unit
    if (unit != 0)
    {
        auxUnit = unit;
        if (auxUnit == 'B')
        {   
            size_bytes = size;
        }else if (auxUnit = 'K')
        {
            size_bytes = size * 1024;
        }else if (auxUnit == 'M')
        {
            size_bytes = size * 1024 * 1024;
        }else{
            cout << "ERROR" << endl;
        }
    }else{
        size_bytes = size * 1024;
    }

    FILE *fp;
    MBR mbr;

    if ((fp = fopen(auxPath.c_str(), "rb+")))
    {
        bool flagPartition = false;
        bool flagExtendida = false; 
        int indicadorPartition = 0;
        fseek(fp, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, fp);

        for (int i = 0; i < 4; i++)
        {
            if (mbr.MBR_partition[i].part_type == 'E')
            {
                flagExtendida = true;
                break;
            }
        }

        if (!flagExtendida)
        {
            //primero verificamos si existe particion libre
            for (int i = 0; i < 4; i++)
            {
                if (mbr.MBR_partition[i].part_start == -1 || (mbr.MBR_partition[i].part_status == '1' && mbr.MBR_partition[i].part_size>=size_bytes))
                {
                    flagPartition = true;
                    indicadorPartition = i;
                    break;
                }
                
            }
            
            if (flagPartition)
            {
                //verificamos el espacio libre del disco
                int espaciousado = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (mbr.MBR_partition[i].part_status != '1')
                    {
                        espaciousado += mbr.MBR_partition[i].part_size;
                    }
                }

                cout << "Espacio disponible: " << (mbr.MBR_tamanio - espaciousado) << " Bytes" << endl;
                cout << "Espacio necesario:  " << size_bytes << " Bytes" << endl;
                
                //verficamos espacios en la particion
                if ((mbr.MBR_tamanio - espaciousado) >= size_bytes)
                {
                    if (!existePARTITION(pathPart, name))
                    {
                        mbr.disk_fit = auxFit;
                        if (mbr.disk_fit = 'F')
                        {
                            mbr.MBR_partition[indicadorPartition].part_type = 'E';
                            mbr.MBR_partition[indicadorPartition].part_fit = auxFit;

                            //start
                            if (indicadorPartition == 0)
                            {
                                mbr.MBR_partition[indicadorPartition].part_start = sizeof(mbr);
                            }else{
                                int resultado = mbr.MBR_partition[indicadorPartition - 1].part_start + mbr.MBR_partition[indicadorPartition - 1].part_size;
                                mbr.MBR_partition[indicadorPartition].part_start = resultado;
                            }

                            mbr.MBR_partition[indicadorPartition].part_size = size_bytes;
                            mbr.MBR_partition[indicadorPartition].part_status = '0';
                            strcpy(mbr.MBR_partition[indicadorPartition].part_name, name.c_str());

                            //guarda el nuevo mbr
                            //apuntamos al inicio
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&mbr, sizeof(MBR), 1, fp);
                            //se escribe la particion extendida
                            fseek(fp, mbr.MBR_partition[indicadorPartition].part_start, SEEK_SET);

                            EBR ebr;
                            ebr.EBR_part_fit = auxFit;
                            ebr.EBR_part_status = '0';
                            ebr.EBR_part_size = 0;
                            ebr.EBR_part_next = -1;
                            ebr.EBR_part_start = mbr.MBR_partition[indicadorPartition].part_start;
                            strcpy(ebr.EBR_part_name, "");
                            //escribimos archivo
                            fwrite(&ebr, sizeof(EBR), 1, fp);

                            for (int i = 0; i < (size_bytes - (int)sizeof(EBR)); i++)
                            {
                                fwrite(&buff, 1, 1, fp);
                            }
                            cout << "...\n" << "Particion extendida creada con exito"<< endl;

                        }else if (mbr.disk_fit = 'B')
                        {
                            int BestIndex = indicadorPartition;
                            for (int i = 0; i < 4; i++)
                            {
                                if (mbr.MBR_partition[i].part_start == -1 || (mbr.MBR_partition[i].part_status == '1' && mbr.MBR_partition[i].part_size >= size_bytes))
                                {
                                    if (i != indicadorPartition)
                                    {
                                        if (mbr.MBR_partition[BestIndex].part_size > mbr.MBR_partition[i].part_size)
                                        {
                                            BestIndex = i;
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            mbr.MBR_partition[BestIndex].part_type = 'E';
                            mbr.MBR_partition[BestIndex].part_fit = auxFit;

                            //start
                            if (BestIndex == 0)
                            {
                                mbr.MBR_partition[BestIndex].part_start = size_bytes;
                            }else{
                                int resultado = mbr.MBR_partition[BestIndex - 1].part_start + mbr.MBR_partition[BestIndex - 1].part_size;
                                mbr.MBR_partition[BestIndex].part_start = resultado;
                            }
                            
                            mbr.MBR_partition[BestIndex].part_size = size_bytes;
                            mbr.MBR_partition[BestIndex].part_status = '0';
                            strcpy(mbr.MBR_partition[BestIndex].part_name, name.c_str());
                            //colocamos el aputnador 
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&mbr, sizeof(MBR), 1, fp);
                            //Guardamos nuevo apuntador de la nueva particion
                            fseek(fp, mbr.MBR_partition[BestIndex].part_start, SEEK_SET);
                            //escribimos en la nueva particion
                            for (int i = 0; i < size_bytes; i++)
                            {
                                fwrite(&buff, 1, 1, fp);
                            }
                            cout << "...\n" << "Particion Extendida creada con exito" <<  endl;

                        }else if (mbr.disk_fit == 'W')
                        {
                            int worstIndex = indicadorPartition;

                            for (int i = 0; i < 4; i++)
                            {
                                if (mbr.MBR_partition[i].part_start == -1 || (mbr.MBR_partition[i].part_status == '1' && mbr.MBR_partition[i].part_size >= size_bytes))
                                {
                                    if (i != indicadorPartition)
                                    {
                                        if (mbr.MBR_partition[worstIndex].part_size < mbr.MBR_partition[i].part_size)
                                        {
                                            worstIndex = i;
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            mbr.MBR_partition[worstIndex].part_type = 'E';
                            mbr.MBR_partition[worstIndex].part_fit = auxFit;

                            //start
                            if (worstIndex == 0)
                            {
                                mbr.MBR_partition[worstIndex].part_start = sizeof(mbr);
                            }else{
                                int resultado = mbr.MBR_partition[worstIndex - 1].part_start + mbr.MBR_partition[worstIndex - 1].part_size;
                                mbr.MBR_partition[worstIndex].part_size = resultado;
                            }

                            mbr.MBR_partition[worstIndex].part_size = size_bytes;
                            mbr.MBR_partition[worstIndex].part_status = '0';
                            strcpy(mbr.MBR_partition[worstIndex].part_name, name.c_str());

                            //guardamos 
                            fseek(fp, 0, SEEK_SET);
                            fwrite(&mbr, sizeof(MBR), 1, fp);

                            //mandamos el nuevo aputnador para guardar
                            fseek(fp, mbr.MBR_partition[worstIndex].part_start, SEEK_SET);

                            EBR ebr;
                            ebr.EBR_part_fit = auxFit;
                            ebr.EBR_part_status = '0';
                            ebr.EBR_part_start = mbr.MBR_partition[worstIndex].part_start;
                            ebr.EBR_part_size = 0;
                            ebr.EBR_part_next = -1;
                            strcpy(ebr.EBR_part_name, "");

                            fwrite(&ebr, sizeof(EBR), 1, fp);
                            for(int i = 0; i < (size_bytes - (int)sizeof(EBR)); i++){
                                fwrite(&buff,1,1,fp);
                            }
                            cout << "...\n" << "Particion extendida creada con exito"<< endl;
                        }
                    }else{
                        cout << "ERROR ya existe una particion con ese nombre" << endl;
                    }
                }else{
                    cout << "ERROR la particion a crear excede el tamano libre" << endl;
                }
            }else{
                cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
                cout << "Elimine alguna para poder crear una" << endl;
            }
        }else{
            cout << "ERROR ya existe una particion extendida en este disco" << endl;
        }
    fclose(fp);
    }else{
        cout << "ERROR no existe el disco" << endl;
    }
}

void createPartitionLogica(int size, char unit, string pathPart, char fit, string name){
    char auxUnit = 0;
    char auxFit = 0;
    string auxPath = pathPart;
    int size_bytes = 1024;
    char buff = '1';

    //definimos el fit que utilizara segun el parametro
    if (fit != 0)
    {
        auxFit = fit;
    }else{
        auxFit = 'W';
    }
    
    //definimos el unit
    if (unit != 0)
    {
        auxUnit = unit;
        if (auxUnit == 'B')
        {   
            size_bytes = size;
        }else if (auxUnit = 'K')
        {
            size_bytes = size * 1024;
        }else if (auxUnit == 'M')
        {
            size_bytes = size * 1024 * 1024;
        }else{
            cout << "ERROR" << endl;
        }
    }else{
        size_bytes = size * 1024;
    }

    cout << "tamaño particion "<< size_bytes << endl;
    FILE *fp;
    MBR mbr;

    if ((fp = fopen(auxPath.c_str(), "rb+")))
    {
        int indicaExtendida = -1;
        fseek(fp, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, fp);

        for (int i = 0; i < 4; i++)
        {
            if (mbr.MBR_partition[i].part_type == 'E')
            {
                indicaExtendida = i;
                break;
            }
        }

        if(!existePARTITION(pathPart, name))
        {
            if (indicaExtendida != -1)
            {
                EBR ebr;
                int contador = mbr.MBR_partition[indicaExtendida].part_start;
                fseek(fp, contador, SEEK_SET);
                fread(&ebr, sizeof(EBR), 1, fp);

                //comprobamos si es la primera
                if (ebr.EBR_part_size == 0)
                {
                    if (mbr.MBR_partition[indicaExtendida].part_size < size_bytes)
                    {
                        cout << "ERROR la logica supera el espacio disponible de la extendida" << endl;
                    }else
                    {
                        ebr.EBR_part_status = '0';
                        ebr.EBR_part_fit = auxFit;
                        ebr.EBR_part_start = ftell(fp) - sizeof(EBR); //Para regresar al inicio de la extendida
                        //ftell da el tamaño usado y el size of es de la esctrura creada
                        ebr.EBR_part_size = size_bytes;
                        ebr.EBR_part_next = -1;
                        strcpy(ebr.EBR_part_name, name.c_str());
                        fseek(fp, mbr.MBR_partition[indicaExtendida].part_start, SEEK_SET);
                        fwrite(&ebr, sizeof(EBR), 1, fp);
                        
                        cout << "...\nParticion logica creada con exito "<< endl;
                    }
                }else{
                    while((ebr.EBR_part_next != -1) && (ftell(fp) < (mbr.MBR_partition[indicaExtendida].part_size + mbr.MBR_partition[indicaExtendida].part_start))){
                        fseek(fp, ebr.EBR_part_next, SEEK_SET);
                        fread(&ebr, sizeof(EBR), 1, fp);
                    }

                    //byte en el que estoy mas el tamaño que necesito me da el tamaño que quiero par la nueva particion
                    int tamanioNeed = ebr.EBR_part_start + ebr.EBR_part_size + size_bytes;

                    //comprobamos si el tamaño necesario es menor de la particion que se creara
                    if (tamanioNeed <= (mbr.MBR_partition[indicaExtendida].part_size + mbr.MBR_partition[indicaExtendida].part_start))
                    {
                        //le damos el byte al proximo ebr
                        ebr.EBR_part_next = ebr.EBR_part_start + ebr.EBR_part_size;

                        //Escribimos el next del proximo ebr
                        fseek(fp, ftell(fp) - sizeof (EBR),SEEK_SET);
                        fwrite(&ebr, sizeof(EBR),1 ,fp);
                        //Escribimos el nuevo EBR
                        //TODO: cambiar a ebr.part next para simplificar
                        fseek(fp, ebr.EBR_part_start + ebr.EBR_part_size, SEEK_SET);

                        ebr.EBR_part_status = 0;
                        ebr.EBR_part_fit = auxFit;
                        ebr.EBR_part_start = ftell(fp);
                        ebr.EBR_part_size = size_bytes;
                        ebr.EBR_part_next = -1;
                        strcpy(ebr.EBR_part_name, name.c_str());
                        fwrite(&ebr,sizeof(EBR),1,fp);
                        //tenemos guardada la nueva particion logica
                        cout << "Espacio necesario: "<< tamanioNeed << endl;
                        cout << "Espacio Existente: " << mbr.MBR_partition[indicaExtendida].part_size + mbr.MBR_partition[indicaExtendida].part_start << endl;
                        cout << "  ---- Particion logica creada con exito ----\n "<< endl;
                    }else{
                        cout << "ERROR la particion logica a crear excede el espacio disponible de la particion extendida" << endl;
                    } 
                }
            }else
            {
                cout << "ERROR Primero tenemos que crear una particion Extendida " << endl;
            }
        }else
        {
            cout << "ERROR ya existe una particion con ese nombre" << endl;
        }
    fclose(fp);
    }else
    {
        cout << "ERROR no existe el disco" << endl;
    }
}


// -- EXISTENCIA de PARTICION --
bool existePARTITION(string ruta, string name){
    int extendida = -1;
    FILE *fp;

    if ((fp = fopen(ruta.c_str(), "rb+")))
    {
        MBR mbr;
        fseek(fp, 0, SEEK_SET);
        fread(&mbr, sizeof(MBR), 1, fp);

        for (int i = 0; i < 4; i++)
        {
            if (strcmp(mbr.MBR_partition[i].part_name, name.c_str()) == 0)
            {
                fclose(fp);
                return true;

            }else if (mbr.MBR_partition[i].part_type == 'E')
            {
                extendida = i;
            }
        }

        if (extendida != -1)
        {
            fseek(fp, mbr.MBR_partition[extendida].part_start, SEEK_SET);
            EBR ebr;

            while (((fread(&ebr, sizeof(EBR), 1, fp)) != 0) && (ftell(fp) < (mbr.MBR_partition[extendida].part_size + mbr.MBR_partition[extendida].part_start)))
            {
                if (strcmp(ebr.EBR_part_name, name.c_str()) == 0)
                {
                    fclose(fp);
                    return true;
                }
                if (ebr.EBR_part_next == -1)
                {
                    fclose(fp);
                    return false;
                }
            }
        }
    }
    fclose(fp);
    return false;
}

int buscarParticionP_E(string ruta, string name){
    string auxPath = ruta;
    string auxName = name;
    FILE *fp;

    if((fp = fopen(auxPath.c_str(), "rb+"))){

        MBR mbr;
        fseek(fp,0,SEEK_SET);
        fread(&mbr,sizeof(MBR),1,fp);
        for(int i = 0; i < 4; i++){
            if(mbr.MBR_partition[i].part_status != '1'){
                if(strcmp(mbr.MBR_partition[i].part_name,auxName.c_str()) == 0){
                    return i;
                }
            }
        }

    }
    return -1;
}


int buscarParticion_Logica(string ruta, string name){
    string auxPath = ruta;
    string auxName = name;
    FILE *fp;

    if((fp = fopen(auxPath.c_str(),"rb+"))){

        int extendida = -1;
        MBR mbr;

        fseek(fp,0,SEEK_SET);
        fread(&mbr,sizeof(MBR),1,fp);

        for(int i = 0; i < 4; i++){
            if(mbr.MBR_partition[i].part_type == 'E'){
                extendida = i;
                break;
            }
        }

        if(extendida != -1){
            EBR ebr;
            fseek(fp, mbr.MBR_partition[extendida].part_start,SEEK_SET);

            while(fread(&ebr,sizeof(EBR),1,fp)!=0 && (ftell(fp) < mbr.MBR_partition[extendida].part_start + mbr.MBR_partition[extendida].part_size)){
                if(strcmp(ebr.EBR_part_name, auxName.c_str()) == 0){
                    return (ftell(fp) - sizeof(EBR));
                }
            }
        }
        fclose(fp);
    }
    return -1;
}

void mostrarListaMount(){
    for (int i = 0; i < Arreglomount.size(); i++)
    {
        cout << "" << Arreglomount[i].nombre << endl;
        cout << "" << Arreglomount[i].id << endl;
    }
    
}

void Comandomount(string name, string pathrut){

    int indicadorPart = buscarParticionP_E(pathrut, name);
    NDMOUNT varMOUNT;


    if(indicadorPart != -1){

        FILE *fp;

        if((fp = fopen(pathrut.c_str(),"rb+"))){

            MBR mbr;
            fseek(fp, 0, SEEK_SET);
            fread(&mbr, sizeof(MBR),1,fp);

            mbr.MBR_partition[indicadorPart].part_status = '2';

            fseek(fp,0,SEEK_SET);
            fwrite(&mbr,sizeof(MBR),1,fp);
            fclose(fp);
            int letra = buscarLetra(pathrut,name);

            if(letra == -1){
                cout << "\033[31mERROR, La particion ya fue montada.\033[0m" << endl;
            }else{

                int num = buscarNumero(pathrut, name);
                char auxLetra = static_cast<char>(letra);
                string id = "vd";
                id += auxLetra + to_string(num);

                //llenamos el nodo
                varMOUNT.direccion = pathrut;
                varMOUNT.letra =  auxLetra;
                varMOUNT.nombre = name;
                varMOUNT.num = num;
                varMOUNT.size = varMOUNT.size ++;
                varMOUNT.id = id; 
                Arreglomount.push_back(varMOUNT);

                cout << "\033[94mParticion montada con exito.\033[0m" << endl;
                mostrarListaMount();
            }
        }else{
            cout << "\033[31mERROR, no existe disco.\033[0m" << endl;
        }
    }else{// logica
        int indicadorPart = buscarParticion_Logica(pathrut,name);

        if(indicadorPart != -1){

            FILE *fp;

            if((fp = fopen(pathrut.c_str(), "rb+"))){
                EBR ebr;

                fseek(fp, indicadorPart, SEEK_SET);
                fread(&ebr, sizeof(EBR),1,fp);
                ebr.EBR_part_status = '2';

                fseek(fp, indicadorPart, SEEK_SET);
                fwrite(&ebr, sizeof(EBR), 1, fp);
                fclose(fp);

                int letra = buscarLetra(pathrut,name);
                if(letra == -1){
                    
                    cout << "\033[31mERROR, La particion ya fue montada.\033[0m" << endl;

                }else{

                    int num = buscarNumero(pathrut,name);
                    char auxLetra = static_cast<char>(letra);
                    string id = "vd";
                    id += auxLetra + to_string(num);
                    
                    varMOUNT.direccion = pathrut;
                    varMOUNT.letra = auxLetra;
                    varMOUNT.nombre = name;
                    varMOUNT.num = num;
                    varMOUNT.size = varMOUNT.size ++;
                    varMOUNT.id = id; 
                    Arreglomount.push_back(varMOUNT);

                    cout << "\033[94mParticion montada con exito.\033[0m" << endl;
                    mostrarListaMount();
                }
            }else{
                cout << "\033[31mERROR, no existe disco.\033[0m" << endl;
            }
        }else{
            cout << "\033[31mERROR, No existe pareticion con el nombre a montar.\033[0m" << endl;
        }
    }
}


int buscarLetra(string rupath, string nombre){
    int retorno = 'a';

    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if((rupath == Arreglomount[i].direccion) && (nombre == Arreglomount[i].nombre)){
            return -1;
        }else{
            if(rupath == Arreglomount[i].direccion){
                return Arreglomount[i].letra;
            }else if(retorno <= Arreglomount[i].letra){
                retorno++;
            }
        }
    }
    
    return retorno;
}

int buscarNumero(string ruthpath, string nombre){
    int retorno = 1;

    NDMOUNT mount;

    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if((ruthpath == Arreglomount[i].direccion) && (retorno == Arreglomount[i].num)){
            retorno++;
        }
    }
    
    return retorno;
}

// -- UNMOUNT --
void comando_unmount(string id){
    bool existe = false;

    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if( id == Arreglomount[i].id){
            Arreglomount.erase((Arreglomount.begin()+ i));
            existe = true;
            break;
        }
    }

    if (existe)
    {
        mostrarListaMount();
        cout << "\033[94mSe desmonto correctamente.\033[0m" << endl;
    }else{
        cout << "\033[31mERROR, No esta Montado " << id <<".\033[0m" << endl;
    }
}

void fdisk_desmontar(string id, int tamanioMount){

    for (int i = 0; i < tamanioMount; i++)
    {
        if( id == Arreglomount[i].id){
            Arreglomount.erase((Arreglomount.begin()+ i));
            break;
        }
    }
}

bool busca_IDmount(string id){
    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if (Arreglomount[i].id == id)
        {
            return true;
        }
    }
    return false;
}

int index_IDmount(string id){
    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if (Arreglomount[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

void formatearEXT3(int inicio, int tamanio, string direccion){

    //por el numero de estructuras se despeja n y la formula fue dada por el auxiliar
    double n = 1 + ((tamanio - static_cast<int>(sizeof(SUPERBLOQUE)) - 1)/(4 + static_cast<int>(sizeof(INODOTABLA)) +3*static_cast<int>(sizeof(BLOQUEARCHIVOS))));
    int num_estructuras = static_cast<int>(floor(n));//Bitmap indos
    int num_bloques = 3*num_estructuras;//Bitmap bloques
    int super_size = static_cast<int>(sizeof(SUPERBLOQUE));
    int journal_size = static_cast<int>(sizeof(JOURNAL))*num_estructuras;


    SUPERBLOQUE super;
    super.s_filesystem_type = 3;    //EXT3
    super.s_inodes_count = num_estructuras; //Total de inodos
    super.s_blocks_count = num_bloques; //total de bloques
    super.s_free_blocks_count = num_bloques - 2; //libres
    super.s_free_inodes_count = num_estructuras - 2; 
    super.s_mtime = time(nullptr);
    super.s_umtime = 0;
    super.s_mnt_count = 0;
    super.s_magic = 0xEF53;
    super.s_inode_size = sizeof(INODOTABLA);
    super.s_block_size = sizeof(BLOQUEARCHIVOS);
    super.s_first_ino = 2;
    super.s_first_blo = 2;
    super.s_bm_inode_start = inicio + super_size + journal_size;
    super.s_bm_block_start = inicio + super_size + journal_size + num_estructuras;
    super.s_inode_start = inicio + super_size + journal_size + num_estructuras + num_bloques;
    super.s_block_start = inicio + super_size + journal_size + num_estructuras + num_bloques + static_cast<int>(sizeof(INODOTABLA))*num_estructuras;

    INODOTABLA inodo;
    BLOQUECARPETAS bloque;

    //este buffer nos ayudara a escribir en el archivo
    char buffer = '0';
    char buffer2 = '1';
    char buffer3 = '2';

    FILE *fp = fopen(direccion.c_str(), "rb+");

    //en cada fseek ubicamos el puntero para poder guardar el dato que nos importa
    //guardamos el Super Bloque
    fseek(fp,inicio,SEEK_SET);
    fwrite(&super, sizeof(SUPERBLOQUE), 1, fp);


    //guardamos el BITMAP DE INODOS, unitario cada inodo
    for(int i = 0; i < num_estructuras; i++){
        fseek(fp,super.s_bm_inode_start + i,SEEK_SET);
        fwrite(&buffer,sizeof(char),1,fp);
    }
    
    //guardamos el bit para / y users.txt en BM
    fseek(fp,super.s_bm_inode_start,SEEK_SET);
    fwrite(&buffer2,sizeof(char),1,fp);
    fwrite(&buffer2,sizeof(char),1,fp);

    //guardamos el Bitmap de bloques en su cantidad de bloques
    for(int i = 0; i < num_bloques; i++){
        fseek(fp,super.s_bm_block_start + i,SEEK_SET);
        fwrite(&buffer,sizeof(char),1,fp);
    }

    //guradamos el bit en la raiz y el users.txt 
    fseek(fp,super.s_bm_block_start,SEEK_SET);
    fwrite(&buffer2,sizeof(char),1,fp);
    fwrite(&buffer3,sizeof(char),1,fp);

    //el inodo para la carpeta root
    inodo.i_uid = 1;
    inodo.i_gid = 1;
    inodo.i_size = 0;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);
    inodo.i_block[0] = 0;

    //recorremos los regristros  hasta llegar al 15 en el triple indirecto
    for(int i = 1; i < 15;i++){
        inodo.i_block[i] = -1;
    }
    inodo.i_type = '0';
    inodo.i_perm = 664;

    //guardamos en inodotabla
    fseek(fp,super.s_inode_start,SEEK_SET);
    fwrite(&inodo,sizeof(INODOTABLA),1,fp);

    //bloque para su carpeta root

    strcpy(bloque.b_content[0].b_name,".");//Actual
    bloque.b_content[0].b_inodo=0;

    strcpy(bloque.b_content[1].b_name,"..");//Padre
    bloque.b_content[1].b_inodo=0;

    strcpy(bloque.b_content[2].b_name,"users.txt");
    bloque.b_content[2].b_inodo=1;

    strcpy(bloque.b_content[3].b_name,".");
    bloque.b_content[3].b_inodo=-1;
    fseek(fp,super.s_block_start,SEEK_SET);
    fwrite(&bloque,sizeof(BLOQUECARPETAS),1,fp);

    //inodo para los usuarios.txt

    inodo.i_uid = 1;
    inodo.i_gid = 1;
    inodo.i_size = 27;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);
    inodo.i_block[0] = 1;

    //aca es donde registramos cada vez con un -1 haciendo el formateo
    for(int i = 1; i < 15;i++){
        inodo.i_block[i] = -1;
    }
    inodo.i_type = '1';
    inodo.i_perm = 755;
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);
    fwrite(&inodo,sizeof(INODOTABLA),1,fp);

    //bloque para los usuarios.txt

    BLOQUEARCHIVOS archivo;
    //el memset es lo que va sobrescribir 
    memset(archivo.b_content, 0, sizeof(archivo.b_content));
    strcpy(archivo.b_content,"1,G,root\n1,U,root,root,123\n");
    fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS)),SEEK_SET);
    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

    cout << "\033[94m EXT3.\033[0m" << endl;
    cout << ".----------------------------." << endl;
    cout << "\033[94mDisco formateado con exito \n.\033[0m" << endl;

    fclose(fp);
    
}

void formatearEXT2(int inicio, int tamanio, string direccion){

    //calculamos el tamaño del EXT2
    //la formula original es = la que dio el aux mario xd
    double n = 1 + ((tamanio - static_cast<int>(sizeof(SUPERBLOQUE)) - 1 ) / (4 + static_cast<int>(sizeof(INODOTABLA)) +3*static_cast<int>(sizeof(BLOQUEARCHIVOS))));
    //calculamos el num de estructuras
    int num_estructuras = static_cast<int>(floor(n));//Numero de inodos
    int num_bloques = 3 * num_estructuras; //el numero de bloques es el triple del de inodos

    SUPERBLOQUE super;
    //realizamos el lleando de todo el super bloque
    super.s_filesystem_type = 2; //EXT2
    super.s_inodes_count = num_estructuras; //Total de inodos
    super.s_blocks_count = num_bloques; //total de bloques
    super.s_free_blocks_count = num_bloques -2; //libres
    super.s_free_inodes_count = num_estructuras -2;
    super.s_mtime = time(nullptr);
    super.s_umtime = 0;
    super.s_mnt_count = 0;
    super.s_magic = 0xEF53; //valor que identifica al sistema de archivos
    super.s_inode_size = sizeof(INODOTABLA);
    super.s_block_size = sizeof(BLOQUEARCHIVOS);
    super.s_first_ino = 2;
    super.s_first_blo = 2;
    super.s_bm_inode_start = inicio + static_cast<int>(sizeof(SUPERBLOQUE)); //guarta del bitmap de inodos
    super.s_bm_block_start = inicio + static_cast<int>(sizeof(SUPERBLOQUE)) + num_estructuras; //bitmap de bloques
    super.s_inode_start = inicio + static_cast<int>(sizeof (SUPERBLOQUE)) + num_estructuras + num_bloques; //tabla inodos
    super.s_block_start = inicio + static_cast<int>(sizeof(SUPERBLOQUE)) + num_estructuras + num_bloques + (static_cast<int>(sizeof(INODOTABLA))*num_estructuras);

    INODOTABLA inodo;
    BLOQUECARPETAS bloque;

    //este buffer nos ayudara a escribir en el archivo
    char buffer = '0';
    char buffer2 = '1';
    char buffer3 = '2';

    FILE *fp = fopen(direccion.c_str(), "rb+");


    //en cada fseek ubicamos el puntero para poder guardar el dato que nos importa
    //guardamos el Super Bloque
    fseek(fp, inicio, SEEK_SET);
    fwrite(&super, sizeof(SUPERBLOQUE), 1, fp);

    
    //guardamos el BITMAP DE INODOS, unitario cada inodo
    for(int i = 0; i < num_estructuras; i++){
        fseek(fp, super.s_bm_inode_start + i, SEEK_SET);
        fwrite(&buffer, sizeof(char), 1, fp);
    }

    //guardamos el bit para / y users.txt en BM
    fseek(fp,super.s_bm_inode_start,SEEK_SET);
    fwrite(&buffer2, sizeof(char), 1, fp);
    fwrite(&buffer2, sizeof(char), 1, fp);

    //guardamos el Bitmap de bloques
    for(int i = 0; i < num_bloques; i++){
        fseek(fp, super.s_bm_block_start + i, SEEK_SET);
        fwrite(&buffer, sizeof(char), 1, fp);
    }

    //guradamos el bit en la raiz y el users.txt 
    fseek(fp, super.s_bm_block_start, SEEK_SET);
    fwrite(&buffer2, sizeof(char), 1, fp);
    fwrite(&buffer3, sizeof(char), 1, fp);


    
    //el inodo para la carpeta root
    inodo.i_uid = 1;
    inodo.i_gid = 1;
    inodo.i_size = 0;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);
    inodo.i_block[0] = 0;

    //recorremos los regristros  hasta llegar al 15 en el triple indirecto
    for(int i = 1; i < 15;i++)
        inodo.i_block[i] = -1;
    inodo.i_type = '0';
    inodo.i_perm = 664;
    
    //guardamos en inodotabla
    fseek(fp, super.s_inode_start, SEEK_SET);
    fwrite(&inodo, sizeof(INODOTABLA), 1, fp);

    
    //bloqueo para su carpeta root
    strcpy(bloque.b_content[0].b_name,".");//Actual, se mantiene el original
    bloque.b_content[0].b_inodo = 0;

    strcpy(bloque.b_content[1].b_name,"..");//Padre
    bloque.b_content[1].b_inodo = 0;

    strcpy(bloque.b_content[2].b_name,"users.txt");
    bloque.b_content[2].b_inodo = 1;

    strcpy(bloque.b_content[3].b_name,".");
    bloque.b_content[3].b_inodo=-1;

    //guradamos en el bloue de arranque
    fseek(fp, super.s_block_start, SEEK_SET);
    fwrite(&bloque,sizeof(BLOQUECARPETAS),1,fp);

    //inodo para usuarios.txt
    inodo.i_uid = 1;
    inodo.i_gid = 1;
    inodo.i_size = 27;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);
    inodo.i_block[0] = 1;

    //aca es donde registramos cada vez con un -1 haciendo el formateo
    for(int i = 1; i < 15;i++){
        inodo.i_block[i] = -1;
    }

    inodo.i_type = '1';
    inodo.i_perm = 755;     //le damos el permiso de solo lectura
    fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fwrite(&inodo,sizeof(INODOTABLA),1,fp);

    //bloque para los usuarios.txt
    BLOQUEARCHIVOS archivo;
    //que es memset?
    memset(archivo.b_content, 0, sizeof(archivo.b_content));

    //apuntamos y guardamos el nuevo conenido para el bloque de carpetas
    strcpy(archivo.b_content,"1,G,root\n1,U,root,root,123\n");
    fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS)),SEEK_SET);
    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

    cout << "\033[94m EXT2.\033[0m" << endl;
    cout << ".----------------------------." << endl;
    cout << "\033[94mDisco formateado con exito \n.\033[0m" << endl;

    fclose(fp);
}

bool buscar_IdMkfs(string id){

    for (int i = 0; i < mkfs_init.size(); i++)
    {
        if (id == mkfs_init[i].id)
        {
            return true;
        }
    }
    return false;
}

char obtenerLogic(string direccion, string nombre){
    string auxPath = direccion;
    string auxName = nombre;
    FILE * fp;
    
    if((fp = fopen(auxPath.c_str(), "rb+"))){
        int extendida = -1;
        MBR mbr;

        fseek(fp,0,SEEK_SET);
        fread(&mbr,sizeof(MBR),1,fp);

        for(int i = 0; i < 4; i++){
            if(mbr.MBR_partition[i].part_type == 'E'){
                extendida = i;
                break;
            }
        }

        if(extendida != -1){

            EBR ebr;
            fseek(fp, mbr.MBR_partition[extendida].part_start,SEEK_SET);
            //retornamos el tipo de particion que se realizo  de la particion
            while(fread(&ebr,sizeof(EBR),1,fp)!=0 && (ftell(fp) < mbr.MBR_partition[extendida].part_start + mbr.MBR_partition[extendida].part_size)){
                if(strcmp(ebr.EBR_part_name, auxName.c_str()) == 0){
                    return ebr.EBR_part_fit;
                }
            }
        }
        fclose(fp);
    }
    return -1;
}

int buscarGrupo(string name){
    FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    SUPERBLOQUE super;
    INODOTABLA inodo;

    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Leemos el inodo del archivo users.txt

    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BLOQUEARCHIVOS archivo;
            fseek(fp,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
            }
            strcat(cadena,archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    char *token = strtok_r(cadena,"\n",&end_str);
    while(token != nullptr){
        char id[2];
        char tipo[2];
        char group[12];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);
        strcpy(id,token2);
        if(strcmp(id,"0") != 0){//Verificar que no sea un U/G eliminado
            token2 = strtok_r(nullptr,",",&end_token);
            strcpy(tipo,token2);
            if(strcmp(tipo,"G") == 0){
                strcpy(group,end_token);
                if(strcmp(group,name.c_str()) == 0)
                    return atoi(id);
            }
        }
        token = strtok_r(nullptr,"\n",&end_str);
    }

    return -1;
}

USUARIO obtener_usuario(string direccion,int inicioSuper, int usuario){
    FILE *fp = fopen(direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    SUPERBLOQUE super;
    INODOTABLA inodo;
    USUARIO usr;

    fseek(fp,inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Nos posicionamos en el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BLOQUEARCHIVOS archivo;
            fseek(fp,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
            }
            strcat(cadena,archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    char *token = strtok_r(cadena,"\n",&end_str);
    while(token != nullptr){
        char id[2];
        char tipo[2];
        char user[12];
        char grupo[12];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);
        strcpy(id,token2);
        if(strcmp(id,"0") != 0){//Verificar que no sea un U/G eliminado
            token2 = strtok_r(nullptr,",",&end_token);
            strcpy(tipo,token2);
            if(strcmp(tipo,"U") == 0){
                token2 = strtok_r(nullptr,",",&end_token);
                strcpy(grupo,token2);
                token2 = strtok_r(nullptr,",",&end_token);
                strcpy(user,token2);
                int idAux = atoi(id);
                if(idAux == usuario){
                    string groupName(grupo);
                    usr.id_usr = atoi(id);
                    usr.id_grp = buscarGrupo(groupName);
                    strcpy(usr.username,user);
                    strcpy(usr.group,grupo);
                    return usr;
                }

            }
        }
        token = strtok_r(nullptr,"\n",&end_str);
    }
    return usr;
}

// -- Verificaro los datos para login
int verificarDatos_login(string user, string password, string direccion){
    FILE *fp = fopen(direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    SUPERBLOQUE super;
    INODOTABLA inodo;

    //nos ubicamos en la parte logeada
    fseek(fp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fp);
    //Leemos el inodo del archivo users.txt

    //apuntamos al inodo donde se creo
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo, sizeof(INODOTABLA), 1, fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){

            BLOQUEARCHIVOS archivo;
            //unicamos donde esatara el bipmat de bloques
            fseek(fp, super.s_block_start, SEEK_SET);

            //Leemos el bitmap
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo, sizeof(BLOQUEARCHIVOS), 1, fp);
            }
            //copiamso el contenido en la cadena inicial
            strcat(cadena, archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    //generamos tokens
    char *token = strtok_r(cadena,"\n",&end_str);

    while(token != nullptr){
        char id[2];
        char tipo[2];
        string group;
        char user_[12];
        char password_[12];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);

        strcpy(id,token2);

        if(strcmp(id,"0") != 0){//Verificar que no sea un U/G eliminado
            //analizamos la secuencia de tokens
            token2 = strtok_r(nullptr,",",&end_token);

            strcpy(tipo,token2);

            if(strcmp(tipo,"U") == 0){
                token2 = strtok_r(nullptr,",",&end_token);
                group = token2;

                token2 = strtok_r(nullptr,",",&end_token);
                strcpy(user_,token2);

                token2 = strtok_r(nullptr,",",&end_token);
                strcpy(password_,token2);

                if(strcmp(user_,user.c_str()) == 0){
                    if(strcmp(password_,password.c_str()) == 0){
                        actualSesion.direccion = direccion;
                        actualSesion.id_user = atoi(id);
                        actualSesion.id_grp = buscarGrupo(group);
                        return 1;
                    }else
                        return 2;
                }
            }
        }
        token = strtok_r(nullptr,"\n",&end_str);
    }

    return 0;
}

// -- LOGIN --
int log_entra(string direccion, string nombre, string user, string password){
    int index = buscarParticionP_E(direccion, nombre);

    if(index != -1){

        MBR mbr;
        SUPERBLOQUE super;
        INODOTABLA inodo;

        FILE *fp = fopen(direccion.c_str(),"rb+");

        //leemos y apuntamos en la particion que xiste
        fread(&mbr, sizeof(MBR), 1, fp);
        fseek(fp, mbr.MBR_partition[index].part_start, SEEK_SET);

        //leemos y apuntamos a la inicio de la tabla de nodos del superbloque
        fread(&super, sizeof(SUPERBLOQUE), 1, fp);
        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);

        //leemos y apuntamos a la inicio de la tabla de inodos
        fread(&inodo, sizeof(INODOTABLA), 1, fp);
        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);

        inodo.i_atime = time(nullptr);

        //guardamos en la tabla INODOS
        fwrite(&inodo,sizeof(INODOTABLA),1,fp);
        fclose(fp);

        //iniciamos sesion en la particion de index
        actualSesion.inicioSuper = mbr.MBR_partition[index].part_start;
        //fit usada en la particion
        actualSesion.fit = mbr.MBR_partition[index].part_fit;
        //JOURNAL en la session
        actualSesion.inicioJournal = mbr.MBR_partition[index].part_start + static_cast<int>(sizeof(SUPERBLOQUE));
        //identificacion del numero utilizado
        actualSesion.tipo_sistema = super.s_filesystem_type;
        //verificamos el login correcto
        return verificarDatos_login(user,password, direccion);
    }else{//en logicas
        index = buscarParticion_Logica(direccion, nombre);
        if(index != -1){

            SUPERBLOQUE super;
            INODOTABLA inodo;

            FILE * fp = fopen(direccion.c_str(),"rb+");
            fseek(fp, index + static_cast<int>(sizeof(EBR)), SEEK_SET);
            fread(&super,sizeof(SUPERBLOQUE),1,fp);
            fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);

            fread(&inodo, sizeof(INODOTABLA), 1, fp);
            fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);

            inodo.i_atime = time(nullptr);

            fwrite(&inodo, sizeof(INODOTABLA), 1, fp);
            fclose(fp);

            //ubicamos el super del login
            actualSesion.inicioSuper = index + static_cast<int>(sizeof(EBR));
            actualSesion.fit = obtenerLogic(direccion, nombre);
            return verificarDatos_login(user,password,direccion);
        }
    }
    return 0;
}

//motodo para guardar en el jornal
void guardarJournal(char* operacion,int tipo,int permisos,char *nombre,char *content){
    SUPERBLOQUE super;
    JOURNAL journal;

    //llenamos el JOURNAL
    strcpy(journal.Journal_Tipo_Operacion,operacion);
    journal.Journal_tipo = tipo;
    strcpy(journal.Journal_nombre,nombre);
    strcpy(journal.Journal_contenido,content);
    journal.Journal_fecha = time(nullptr);
    journal.Journal_propietario = actualSesion.id_user;
    journal.Journal_permisos = permisos;

    FILE *fp = fopen(actualSesion.direccion.c_str(), "rb+");
    //Buscar el ultimo journal
    JOURNAL journalAux;
    bool ultimo = false;


    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);

    //calculamos cuales seran los limites del journal
    int inicio_journal = actualSesion.inicioSuper + static_cast<int>(sizeof(SUPERBLOQUE));
    int final_journal = super.s_bm_inode_start;
    
    //ubicamos el punero del incio del journal
    fseek(fp, inicio_journal, SEEK_SET);

    while((ftell(fp) < final_journal) && !ultimo)
    {   
        //leemos el jorunal para asignarle el ultimo
        fread(&journalAux, sizeof(JOURNAL), 1, fp);
        if(journalAux.Journal_tipo != 0 && journalAux.Journal_tipo != 1)
            ultimo = true;
    }

    fseek(fp,ftell(fp)- static_cast<int>(sizeof(JOURNAL)),SEEK_SET);
    fwrite(&journal,sizeof(JOURNAL),1,fp);
    fclose(fp);
}


int buscarBit(FILE *fp, char tipo, char fit){
    SUPERBLOQUE super;

    int inicio_bm = 0;
    char tempBit = '0';
    int bit_libre = -1;
    int tam_bm = 0;

    fseek(fp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fp);

    //identificamos tipos
    if(tipo == 'I')
    {
        tam_bm = super.s_inodes_count;
        inicio_bm = super.s_bm_inode_start;

    }else if(tipo == 'B')
    {
        tam_bm = super.s_blocks_count;
        inicio_bm = super.s_bm_block_start;
    }

    //---------------Tipo de ajuste a utilizar----------------
    if(fit == 'F')
    {//Primer ajuste
        for(int i = 0; i < tam_bm; i++)
        {
            //aputnamos el bloque inicio
            fseek(fp, inicio_bm + i,SEEK_SET);
            tempBit = static_cast<char>(fgetc(fp));

            if(tempBit == '0')
            {
                bit_libre = i;
                return bit_libre;
            }
        }

        if(bit_libre == -1)
            return -1;

    }else if(fit == 'B')
    {//Mejor ajuste
        int libres = 0;
        int auxLibres = -1;

        for(int i = 0; i < tam_bm; i++)
        {//Primer recorrido
            //nos ubicamos en cada bloque segun el tamaño del bm
            fseek(fp,inicio_bm + i,SEEK_SET);
            tempBit = static_cast<char>(fgetc(fp));

            if(tempBit == '0')
            {
                libres++;
                if(i+1 == tam_bm)
                {
                    if(auxLibres == -1 || auxLibres == 0)
                        auxLibres = libres;
                    else{
                        if(auxLibres > libres)
                            auxLibres = libres;
                    }
                    libres = 0;
                }
            //si el bit es 1
            }else if(tempBit == '1')
            {
                //espacio libre
                if(auxLibres == -1 || auxLibres == 0)
                    auxLibres = libres;
                else{
                    if(auxLibres > libres)
                        auxLibres = libres;
                }
                libres = 0;
            }
        }

        for(int i = 0; i < tam_bm; i++)
        {
            //apuntamos el inicio de bloque para luago guardarlo
            fseek(fp,inicio_bm + i, SEEK_SET);
            tempBit = static_cast<char>(fgetc(fp));
            if(tempBit == '0')
            {
                libres++;
                if(i+1 == tam_bm)
                    return ((i+1)-libres); //returnamos el bit donde empieza
            }else if(tempBit == '1')
            {
                if(auxLibres == libres)
                    return ((i+1) - libres);
                libres = 0;
            }
        }

        return -1;

    }else if(fit == 'W')
    {//Peor ajuste
        int libres = 0;
        int auxLibres = -1;

        for (int i = 0; i < tam_bm; i++) 
        {//Primer recorrido
            fseek(fp,inicio_bm + i, SEEK_SET);
            tempBit = static_cast<char>(fgetc(fp));

            if(tempBit == '0')
            {
                libres++;
                if(i+1 == tam_bm)
                {
                    if(auxLibres == -1 || auxLibres == 0)
                        auxLibres = libres;
                    else {
                        if(auxLibres < libres)
                            auxLibres = libres;
                    }
                    libres = 0;
                }

            }else if(tempBit == '1')
            {
                if(auxLibres == -1 || auxLibres == 0)
                    auxLibres = libres;
                else{
                    if(auxLibres < libres)
                        auxLibres = libres;
                }
                libres = 0;
            }
        }

        for (int i = 0; i < tam_bm; i++)
        {
            fseek(fp,inicio_bm + i, SEEK_SET);
            tempBit = static_cast<char>(fgetc(fp));

            if(tempBit == '0')
            {
                libres++;
                if(i+1 == tam_bm)
                    return ((i+1) - libres);
            }else if(tempBit == '1')
            {
                if(auxLibres == libres)
                    return ((i+1) - libres);
                libres = 0;
            }
        }
        return -1;
    }
    return 0;
}

int buscarId_grupo(){
    FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    int aux_id = -1;

    SUPERBLOQUE super;
    INODOTABLA inodo;
    
    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Leemos el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for(int i = 0; i < 15; i++)
    {
        if(inodo.i_block[i] != -1)
        {
            BLOQUEARCHIVOS archivo;
            //nuos ubicamos en el inicio de la tabla de bloques
            fseek(fp,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++)
            {
                fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
            }
            strcat(cadena,archivo.b_content);
        }
    }

    fclose(fp);

    //tokens para guardar en el user.txt
    char *end_str;
    char *token = strtok_r(cadena,"\n",&end_str);

    while(token != nullptr)
    {   //llenamos datos
        char id[2];
        char tipo[2];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);

        strcpy(id,token2);
        if(strcmp(id,"0") != 0)
        {//Verificar que no sea un U/G eliminado
            token2 = strtok_r(nullptr,",",&end_token);
            strcpy(tipo,token2);

            if(strcmp(tipo,"G") == 0)
            {//se encuentra la ps del U/G
                aux_id = atoi(id);
            }

        }
        token = strtok_r(nullptr,"\n",&end_str);
    }
    return ++aux_id;
}

void agregarUsuario_TXT(string datos){
    FILE *fp = fopen(actualSesion.direccion.c_str(), "rb+");

    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUEARCHIVOS archivo;
    int blockIndex = 0;

    //napuntamos el usuario ingresado.
    fseek(fp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fp);
    //Leemos el inodo del archivo users.txt
    fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo, sizeof(INODOTABLA), 1, fp);

    for(int i = 0; i < 12; i++)
    {
        if(inodo.i_block[i] != -1)
            blockIndex = inodo.i_block[i];//Ultimo bloque utilizado del archivo
    }

    fseek(fp, super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*blockIndex, SEEK_SET);
    fread(&archivo, sizeof(BLOQUEARCHIVOS), 1, fp);
    
    int usando = static_cast<int>(strlen(archivo.b_content));
    int espacioLibre = 63 - usando;

    if(datos.length() <= espacioLibre)
    {
        strcat(archivo.b_content,datos.c_str());
        fseek(fp, super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*blockIndex, SEEK_SET);
        fwrite(&archivo, sizeof(BLOQUEARCHIVOS), 1, fp);

        //leemos el archivo users.txt
        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
        fread(&inodo, sizeof(INODOTABLA), 1, fp);

        //tamaño del inodo
        inodo.i_size = inodo.i_size + datos.length();
        inodo.i_mtime = time(nullptr);

        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
        fwrite(&inodo, sizeof(INODOTABLA), 1, fp);
    }else{
        string val = "";
        string val2 = "";
        int i = 0;

        //cargamos las variables auxiliares
        for(i = 0; i < espacioLibre; i++)
            val += datos.at(i);

        for(; i < datos.length(); i++)
            val2  += datos.at(i);

        //Guardamos lo que cabe en el primer bloque
        strcat(archivo.b_content, val.c_str());
        fseek(fp, super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*blockIndex, SEEK_SET);
        fwrite(&archivo, sizeof(BLOQUEARCHIVOS), 1, fp);

        BLOQUEARCHIVOS auxArchivo;

        strcpy(auxArchivo.b_content, val2.c_str());
        int bit = buscarBit(fp, 'B', actualSesion.fit);
        /*Guardamos el bloque en el bitmap y en la tabla de bloques*/
        fseek(fp, super.s_bm_block_start + bit, SEEK_SET);
        //escribimos el caracter en el archivo
        fputc('2', fp);
        fseek(fp,super.s_block_start + (static_cast<int>(sizeof(BLOQUEARCHIVOS))*bit),SEEK_SET);
        fwrite(&auxArchivo,sizeof(BLOQUEARCHIVOS),1,fp);

        //Guardamos el modificado del inodo
        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
        fread(&inodo, sizeof(INODOTABLA), 1, fp);

        //llenamos el bloque de inodos
        inodo.i_size = inodo.i_size + datos.length();
        inodo.i_mtime = time(nullptr);
        inodo.i_block[blockIndex] = bit;

        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
        fwrite(&inodo, sizeof(INODOTABLA),1,fp);

        //Guardamos la nueva cantidad de bloques libres y el primer bloque libre
        super.s_first_blo = super.s_first_blo + 1;
        super.s_free_blocks_count = super.s_free_blocks_count - 1;
        fseek(fp, actualSesion.inicioSuper, SEEK_SET);
        fwrite(&super, sizeof(SUPERBLOQUE), 1, fp);
    }
    fclose(fp);
}

// - eliminar grupo -
void eliminarGrupo(string name){
    FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUEARCHIVOS archivo;

    int col = 1;
    char actual;
    int posicion = 0;
    int numBloque = 0;
    int id = -1;
    char tipo = '\0';
    string grupo = "";
    string palabra = "";
    bool flag = false;


    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Nos posicionamos en el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for (int i = 0; i < 12; i++) 
    {
        if(inodo.i_block[i] != -1)
        {
            //ubicamos ell i nodos del del archvo users.txt
            fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*inodo.i_block[i],SEEK_SET);
            fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

            for(int j = 0; j < 63; j++)
            {
                //ubicamos carpeta
                actual = archivo.b_content[j];
                if(actual=='\n')
                {
                    if(tipo == 'G')
                    {
                        //ubicamos el grupo 
                        grupo = palabra;
                        if(strcmp(grupo.c_str(),name.c_str()) == 0)
                        {
                            fseek(fp,super.s_block_start+static_cast<int>(sizeof(BLOQUEARCHIVOS))*numBloque,SEEK_SET);
                            fread(&archivo,sizeof(BLOQUECARPETAS),1,fp);

                            archivo.b_content[posicion] = '0';
                            fseek(fp,super.s_block_start+static_cast<int>(sizeof(BLOQUEARCHIVOS))*numBloque,SEEK_SET);
                            fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

                            cout << "\033[94mGrupo eliminado con exito\033[0m" << endl;
                            flag = true;
                            break;
                        }
                    }
                    col = 1;
                    palabra = "";

                }else if(actual != ',')
                {
                    palabra += actual;
                    col++;
                }else if(actual == ',')
                {
                    if(col == 2)
                    {
                        id = atoi(palabra.c_str());
                        posicion = j-1;
                        numBloque = inodo.i_block[i];
                    }
                    else if(col == 4)
                        tipo = palabra[0];
                    col++;
                    palabra = "";
                }
            }
            if(flag)
                break;
        }
    }
    fclose(fp);
}

bool buscarUsuario(string name){
    FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    SUPERBLOQUE super;
    INODOTABLA inodo;

    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Nos posicionamos en el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BLOQUEARCHIVOS archivo;
            fseek(fp,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
            }
            strcat(cadena,archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    char *token = strtok_r(cadena,"\n",&end_str);
    while(token != nullptr){
        char id[2];
        char tipo[2];
        char user[12];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);
        strcpy(id,token2);
        if(strcmp(id,"0") != 0){//Verificar que no sea un U/G eliminado
            token2 = strtok_r(nullptr,",",&end_token);
            strcpy(tipo,token2);
            if(strcmp(tipo,"U") == 0){
                token2 = strtok_r(nullptr,",",&end_token);
                token2 = strtok_r(nullptr,",",&end_token);
                strcpy(user,token2);

                if(strcmp(user,name.c_str()) == 0)
                    return true;
            }
        }
        token = strtok_r(nullptr,"\n",&end_str);
    }

    return false;
}

int buscarId_usr(){
    FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

    char cadena[400] = "\0";
    int res = 0;
    SUPERBLOQUE super;
    INODOTABLA inodo;

    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Nos posicionamos en el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)), SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BLOQUEARCHIVOS archivo;
            fseek(fp,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
            }
            strcat(cadena,archivo.b_content);
        }
    }

    fclose(fp);

    char *end_str;
    char *token = strtok_r(cadena,"\n",&end_str);
    while(token != nullptr){
        char id[2];
        char tipo[2];
        char *end_token;
        char *token2 = strtok_r(token,",",&end_token);
        strcpy(id,token2);
        if(strcmp(id,"0") != 0){//Verificar que no sea un U/G eliminado
            token2 = strtok_r(nullptr,",",&end_token);
            strcpy(tipo,token2);
            if(strcmp(tipo,"U") == 0)
                res++;
        }
        token = strtok_r(nullptr,"\n",&end_str);
    }
    return ++res;
}

void eliminarUsuario(string name){
    FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUEARCHIVOS archivo;

    int col = 1;
    char actual;
    string palabra = "";
    int posicion = 0;
    int numBloque = 0;
    int id = -1;
    char tipo = '\0';
    string grupo = "";
    string usuario = "";
    bool flag = false;

    fseek(fp,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Nos posicionamos en el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    for (int i = 0; i < 12; i++) 
    {
        if(inodo.i_block[i] != -1)
        {
            fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*inodo.i_block[i],SEEK_SET);
            fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

            for(int j = 0; j < 63; j++)
            {
                actual = archivo.b_content[j];
                if(actual=='\n'){
                    if(tipo == 'U'){
                        if(strcmp(usuario.c_str(),name.c_str()) == 0){
                            fseek(fp,super.s_block_start+static_cast<int>(sizeof(BLOQUEARCHIVOS))*numBloque,SEEK_SET);
                            fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
                            archivo.b_content[posicion] = '0';
                            fseek(fp,super.s_block_start+static_cast<int>(sizeof(BLOQUEARCHIVOS))*numBloque,SEEK_SET);
                            fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
                            cout << "USUARIO eliminado con exito" << endl;
                            flag = true;
                            break;
                        }
                        usuario = "";
                        grupo = "";
                    }
                    col = 1;
                    palabra = "";
                }else if(actual != ','){
                    palabra += actual;
                    col++;
                }else if(actual == ','){
                    if(col == 2){
                        id = atoi(palabra.c_str());
                        posicion = j-1;
                        numBloque = inodo.i_block[i];
                    }
                    else if(col == 4)
                        tipo = palabra[0];
                    else if(grupo == "")
                        grupo = palabra;
                    else if(usuario == "")
                        usuario = palabra;
                    col++;
                    palabra = "";
                }
            }
            if(flag)
                break;
        }
    }

    fclose(fp);
}

int obtenerByte_inBloque(FILE *fpTemp,int auxpos, char tipo){
    SUPERBLOQUE super;

    //el inicio de sesion del super bloque
    fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fpTemp);

    if(tipo == '1')
    {
        //byte ubicado el tbla de inodos del super bloque
        return (super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*auxpos);
    }else if(tipo == '2')
        return (super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*auxpos);
    return 0;
}

int buscar_CarpetaAr(FILE *fpTemp, char* ruta){
    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUECARPETAS carpeta;
    BLOQUEAPUNTADORES apuntador;

    vector<string> arreglo;
    //tokens separados por /, para el nombre de carpetas
    char *token = strtok(ruta,"/");
    int contador = 0;
    int numInodo = 0;

    //obtenenmos las carpetas que se encuentran
    while(token != nullptr)
    {
        arreglo.push_back(token);
        contador++;
        token = strtok(nullptr,"/");
    }

    //ubicamos el puntero en l inicio del bloque del usuario
    fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fpTemp);
    numInodo = super.s_inode_start; //Byte donde inicia el inodo

    //recorremos los bloques
    for (int i2 = 0; i2 < contador; i2++) 
    {
        //tood el inicio del inodo apuntamos
        fseek(fpTemp, numInodo, SEEK_SET);
        fread(&inodo,sizeof(INODOTABLA),1,fpTemp);

        int avanza = 0;
        for(int i = 0; i < 15; i++)
        {
            if(inodo.i_block[i] != -1)
            {//Apuntadores directos
                int byteBloque = obtenerByte_inBloque(fpTemp, inodo.i_block[i], '2');
                //apuntamos a los nodos indirectos
                fseek(fpTemp, byteBloque, SEEK_SET);
                //si son nodos indirectos entrarar a < 12
                if(i < 12)
                {
                    //leemos las carpetas
                    fread(&carpeta, sizeof(BLOQUECARPETAS), 1, fpTemp);
                    for (int j = 0; j < 4; j++)
                    {
                        if((i2 == contador - 1) && (strcasecmp(carpeta.b_content[j].b_name,arreglo.at(i2).c_str()) == 0))
                        {//Tendria que ser la carpeta
                            //retornamos las carpetas contenidas
                            return carpeta.b_content[j].b_inodo;
                        }else if((i2 != contador - 1) && (strcasecmp(carpeta.b_content[j].b_name,arreglo.at(i2).c_str()) == 0))
                        {//tendria que ser archivo
                            numInodo = obtenerByte_inBloque(fpTemp, carpeta.b_content[j].b_inodo, '1');
                            avanza = 1;
                            break;
                        }
                    }
                }else if(i == 12)
                {//mantiene en Apuntador indirecto
                //leemos el bloque de apuntadores
                    fread(&apuntador, sizeof(BLOQUEAPUNTADORES), 1, fpTemp);

                    for(int j = 0; j < 16; j++)
                    {//entramos a buscar en el tamaño del bloque apuntadores entre archivos y carpetas
                        if(apuntador.b_pointers[j] != -1)
                        {//si el apuntador no esta vacio
                            //obtenenmos el byte del inico del bloque
                            byteBloque = obtenerByte_inBloque(fpTemp, apuntador.b_pointers[j], '2');
                            //apuntamos y leemos dese el bloque que empezara el byte del contenido
                            fseek(fpTemp, byteBloque, SEEK_SET);
                            fread(&carpeta,sizeof(BLOQUECARPETAS),1,fpTemp);

                            for (int k = 0; k < 4; k++) 
                            {//vemos los bloques
                                if((i2 == contador - 1) && (strcasecmp(carpeta.b_content[k].b_name, arreglo.at(i2).c_str()) == 0))
                                {//Tendria que ser la carpeta y comparacion de carpetas
                                    return carpeta.b_content[k].b_inodo;
                                }else if((i2 != contador - 1) && (strcasecmp(carpeta.b_content[k].b_name, arreglo.at(i2).c_str()) == 0))
                                {//comparacion de arhcivos
                                    numInodo = obtenerByte_inBloque(fpTemp, carpeta.b_content[k].b_inodo, '1');
                                    avanza = 1;
                                    break;
                                }
                            }
                            //no contiene
                            if(avanza == 1)
                                break;
                        }
                    }
                    //no pasa nada si continua en eso
                }else if(i == 13){

                }else if(i == 14){

                }
                if(avanza == 1)
                    break;
            }
        }
    }
    return -1;
}

void permisos_todos(FILE* fpTemp, int num, int permisos){
    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUECARPETAS carpeta;
    char byte ='0';

    //apuntamos y leemos el inicio a fin del super bloque
    fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fpTemp);

    //apuntamos al inicio del super bloque si es 0 y si no apunta a todo el super bloque
    fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*num, SEEK_SET);
    fread(&inodo, sizeof(INODOTABLA), 1, fpTemp);

    //le damos el valor del inodo permisos
    inodo.i_perm = permisos;
    //guardamos el bitmap ubicada en el bloque
    fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*num, SEEK_SET);
    fwrite(&inodo, sizeof(INODOTABLA), 1, fpTemp);

    for(int i = 0; i < 15; i++)
    {
        if(inodo.i_block[i] != -1)
        {
            //apuntamos al inicio del bloque en cada bloque i nodos
            fseek(fpTemp, super.s_bm_block_start + inodo.i_block[i], SEEK_SET);

            //damos el bye del archivo
            byte = static_cast<char>(fgetc(fpTemp));
            if(byte == '1')
            {
                //apuntamos para recorrer desde el inicio del bloque hasta el i nodo de boques
                fseek(fpTemp, super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*inodo.i_block[i], SEEK_SET);
                fread(&carpeta, sizeof(BLOQUECARPETAS), 1, fpTemp);
                for(int j = 0; j < 4; j++)
                {
                    //si no esta disponible, para cambiarle a los que estan ocupados
                    if(carpeta.b_content[j].b_inodo != -1)
                    {
                        //cambia todos lo permisos de los que si esten disponibles
                        if(strcmp(carpeta.b_content[j].b_name, ".")!=0 &&  strcmp(carpeta.b_content[j].b_name, "..")!=0)
                            permisos_todos(fpTemp, carpeta.b_content[j].b_inodo, permisos);
                    }
                }
            }
        }
    }
}

bool permisos_Escritura(int permisos, bool flagUser, bool flagGroup){
    
    //string los permisos
    string aux = to_string(permisos);
    char propietario = aux[0];
    char grupo = aux[1];
    char otros = aux[2];

    //permisos UGO linux
    if((propietario == '2' || propietario == '3' || propietario == '6' || propietario || '7') && flagUser)
        return true;
    else if((grupo == '2' || grupo == '3' || grupo == '6' || grupo == '7') && flagGroup)
        return true;
    else if(otros == '2' || otros == '3' || otros == '6' || otros == '7')
        return true;
    return false;
}

INODOTABLA crear_Inodo(int size,char type,int perm){
    INODOTABLA inodo ;

    //llenamos dastos actuales
    inodo.i_uid = actualSesion.id_user;
    inodo.i_gid = actualSesion.id_grp;
    inodo.i_size = size;
    inodo.i_atime = time(nullptr);
    inodo.i_ctime = time(nullptr);
    inodo.i_mtime = time(nullptr);

    for(int i = 0; i < 15; i++)
        inodo.i_block[i] = -1;
    inodo.i_type = type;
    inodo.i_perm = perm;

    return inodo;
}

BLOQUECARPETAS crear_BloqCarpeta(){    
    BLOQUECARPETAS carpeta;

    //recorremos los 4 contenidos de el bloque 
    for(int i = 0; i < 4; i++)
    {
        strcpy(carpeta.b_content[i].b_name,"");
        carpeta.b_content[i].b_inodo = -1;
    }
    return carpeta;
}

int buscar_ContentDisponible(FILE* fpTemp, int numInodo, INODOTABLA &inodo, BLOQUECARPETAS &carpeta, BLOQUEAPUNTADORES &apuntadores,int &content,int &bloque,int &pointer){
    int disponible = 0;
    SUPERBLOQUE super;
    //apuntamos al inicio del inicio del super bloque 
    fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE), 1, fpTemp);
    fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fpTemp);
    //Buscamos un espacio disponible en el bloque carpeta

    for(int i = 0; i < 15; i++)
    {
        if(inodo.i_block[i] != -1)
        {
            if(i == 12)
            {//Apuntador indirecto simple
                //apuntamos en el i nodo del indirecto simple
                fseek(fpTemp, super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[i], SEEK_SET);
                fread(&apuntadores,sizeof(BLOQUEAPUNTADORES), 1, fpTemp);
                for(int j = 0; j < 16; j++)
                {
                    if(apuntadores.b_pointers[j] != -1)
                    {   //apuntamos al inicio del bloque, para apuntadores
                        fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*apuntadores.b_pointers[j],SEEK_SET);
                        fread(&carpeta,sizeof(BLOQUECARPETAS),1,fpTemp);

                        //recorremos el hasta el content 4
                        for(int k = 0; k < 4; k++)
                        {
                            //content libre si entra al if y completamos
                            if(carpeta.b_content[k].b_inodo == -1)
                            {
                                disponible = 1;
                                bloque = i;
                                pointer = j;
                                content = k;
                                break;
                            }
                        }
                    }
                    if(disponible == 1)
                        break;
                }
            }else if(i == 13){//Apuntador indirecto doble

            }else if(i == 14){//Apuntador indirecto triple

            }else{//Apuntadores directos
                //apuntamos con indirectos al bloque de carpetad de cada bloque contenido
                fseek(fpTemp, super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*inodo.i_block[i], SEEK_SET);
                fread(&carpeta,sizeof(BLOQUECARPETAS),1,fpTemp);
                for(int j = 0; j < 4; j++)
                {
                    //si esta disponible el inodo del bloque coneetnido
                    if(carpeta.b_content[j].b_inodo == -1)
                    {
                        disponible = 1;
                        bloque = i;
                        content = j;
                        break;
                    }
                }
            }
        }
        if(disponible == 1)
            break;
    }
    return disponible;
}

int nuevaCarpeta(FILE *stream, char fit, bool flagP, char *path, int index){
    SUPERBLOQUE super;
    INODOTABLA inodo,inodoNuevo;
    BLOQUECARPETAS carpeta, carpetaNueva, carpetaAux;
    BLOQUEAPUNTADORES apuntadores;
    vector<string> lista ;

    char copiaPath[500];
    char directorio[500];
    char nombreCarpeta[80];

    strcpy(copiaPath,path);
    strcpy(directorio, dirname(copiaPath));
    strcpy(copiaPath,path);
    strcpy(nombreCarpeta,basename(copiaPath));
    char *token = strtok(path,"/");
    int cont = 0;
    int numInodo = index;
    int response = 0;

    while(token != nullptr){
        cont++;
        lista.push_back(token);
        token = strtok(nullptr,"/");
    }

    fseek(stream,actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,stream);

    if(cont == 1){//Solo es una carpeta '/home' | '/archivos'
        int content = 0;
        int bloque = 0;
        int pointer = 0;
        int libre = buscar_ContentDisponible(stream,numInodo,inodo,carpeta,apuntadores,content,bloque,pointer);
        if(libre == 1){
            if(bloque == 12){//Apuntador indirecto simple
                bool permissions = permisos_Escritura(inodo.i_perm,(inodo.i_uid == actualSesion.id_user),(inodo.i_gid == actualSesion.id_grp));
                if(permissions || (actualSesion.id_user == 1 && actualSesion.id_grp == 1) ){
                    char buffer = '1';
                    int bitInodo = buscarBit(stream,'I',fit);
                    //Agregamos la carpeta al espacio libre en el bloque
                    carpeta.b_content[content].b_inodo = bitInodo;
                    strcpy(carpeta.b_content[content].b_name,nombreCarpeta);
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*apuntadores.b_pointers[pointer],SEEK_SET);
                    fwrite(&carpeta,sizeof(BLOQUECARPETAS),1,stream);
                    //Creamos el nuevo inodo carpeta
                    inodoNuevo = crear_Inodo(0,'0',664);
                    int bitBloque = buscarBit(stream,'B',fit);
                    inodoNuevo.i_block[0] = bitBloque;
                    fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                    fwrite(&inodoNuevo,sizeof(INODOTABLA),1,stream);
                    //Guardamos el bit del inodo en el bitmap
                    fseek(stream,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Creamos el nuevo bloque carpeta
                    carpetaNueva = crear_BloqCarpeta();
                    carpetaNueva.b_content[0].b_inodo = bitInodo;
                    carpetaNueva.b_content[1].b_inodo = numInodo;
                    strcpy(carpetaNueva.b_content[0].b_name,".");
                    strcpy(carpetaNueva.b_content[1].b_name,"..");
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                    fwrite(&carpetaNueva,sizeof(BLOQUECARPETAS),1,stream);
                    //Guardamos el bit del bloque en el bitmap
                    fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Sobreescribimos el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_free_blocks_count = super.s_free_blocks_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 1;
                    fseek(stream,actualSesion.inicioSuper,SEEK_SET);
                    fwrite(&super,sizeof(SUPERBLOQUE),1,stream);
                    return 1;
                }else
                    return 2;
            }else if(bloque == 13){//Apuntador indirecto doble

            }else if(bloque == 14){//Apuntador indirecto triple

            }else{//Apuntadores directos
                bool permisos = permisos_Escritura(inodo.i_perm,(inodo.i_uid == actualSesion.id_user),(inodo.i_gid == actualSesion.id_grp));
                if(permisos || (actualSesion.id_user == 1 && actualSesion.id_grp == 1) ){
                    char buffer = '1';
                    int bitInodo = buscarBit(stream,'I',fit);
                    //Agregamos la carpeta al espacio libre en el bloque
                    carpeta.b_content[content].b_inodo = bitInodo;
                    strcpy(carpeta.b_content[content].b_name,nombreCarpeta);
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*inodo.i_block[bloque],SEEK_SET);
                    fwrite(&carpeta,sizeof(BLOQUECARPETAS),1,stream);
                    //Creamos el nuevo inodo
                    inodoNuevo = crear_Inodo(0,'0',664);
                    int bitBloque = buscarBit(stream,'B',fit);
                    inodoNuevo.i_block[0] = bitBloque;
                    fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                    fwrite(&inodoNuevo,sizeof(INODOTABLA),1,stream);
                    //Creamos el nuevo bloque carpeta
                    carpetaNueva = crear_BloqCarpeta();
                    carpetaNueva.b_content[0].b_inodo = bitInodo;
                    carpetaNueva.b_content[1].b_inodo = numInodo;
                    strcpy(carpetaNueva.b_content[0].b_name,".");
                    strcpy(carpetaNueva.b_content[1].b_name,"..");
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                    fwrite(&carpetaNueva,sizeof(BLOQUECARPETAS),1,stream);
                    //Guardamos los bits en los bitmaps
                    fseek(stream,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Sobreescribimos el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_free_blocks_count = super.s_free_blocks_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 1;
                    fseek(stream,actualSesion.inicioSuper,SEEK_SET);
                    fwrite(&super,sizeof(SUPERBLOQUE),1,stream);
                    return 1;
                }else
                    return 2;
            }
        }else if(libre == 0){//Todos bloques estan llenos
            bool flag = false;//Primera vez
            pointer = -1;
            fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
            fread(&inodo,sizeof(INODOTABLA),1,stream);
            for (int i = 0; i < 15; i++) {
                if(i == 12){//Apuntador indirecto simple
                    if(inodo.i_block[i] == -1){//Primera vez
                        bloque = 12;
                        flag = true;
                        break;
                    }else{
                        fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[12],SEEK_SET);
                        fread(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,stream);
                        for(int j = 0; j < 16; j++){
                            if(apuntadores.b_pointers[j] == -1){
                                bloque = 12;
                                pointer = j;
                                break;
                            }
                        }
                    }
                    if(flag || pointer!= -1)
                        break;
                }else if(i == 13){

                }else if(i == 14){

                }else{
                    if(inodo.i_block[i] == -1){
                        bloque = i;
                        break;
                    }
                }
            }

            if(bloque == 12 && flag){//Apuntador indirecto simple
                bool permissions = permisos_Escritura(inodo.i_perm,(inodo.i_uid == actualSesion.id_user),(inodo.i_gid == actualSesion.id_grp));
                if(permissions || (actualSesion.id_user == 1 && actualSesion.id_grp == 1) ){
                    char buffer = '1';
                    char buffer3 = '3';
                    //Guardamos el bloque en el inodo
                    int bitBloque = buscarBit(stream,'B',fit);//Apuntador
                    inodo.i_block[bloque] = bitBloque;
                    fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                    fwrite(&inodo,sizeof(INODOTABLA),1,stream);
                    //Escribimos el bit del bloque apuntador en el bitmap
                    fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer3,sizeof(char),1,stream);
                    //Creamos el bloque de apuntadores como es simple solo se debe crear uno
                    int bitBloqueCarpeta = buscarBit(stream,'B',fit);//Carpeta
                    apuntadores.b_pointers[0] = bitBloqueCarpeta;
                    for(int i = 1; i < 16; i++)
                        apuntadores.b_pointers[i] = -1;
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*bitBloque,SEEK_SET);
                    fwrite(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,stream);
                    //Creamos la carpeta del apuntador
                    int bitInodo = buscarBit(stream,'I',fit);
                    carpetaAux = crear_BloqCarpeta();
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreCarpeta);
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloqueCarpeta,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BLOQUECARPETAS),1,stream);
                    //Escribimos el bit del bloque carpeta en el bitmap
                    fseek(stream,super.s_bm_block_start + bitBloqueCarpeta,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Creamos el nuevo inodo carpeta
                    inodoNuevo = crear_Inodo(0,'0',664);
                    bitBloque = buscarBit(stream,'B',fit);//Carpeta
                    inodoNuevo.i_block[0] = bitBloque;
                    fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                    fwrite(&inodoNuevo,sizeof(INODOTABLA),1,stream);
                    //Escribimos el bit del inodo en el bitmap
                    fseek(stream,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Creamos el nuevo bloque carpeta
                    carpetaNueva = crear_BloqCarpeta();
                    carpetaNueva.b_content[0].b_inodo = bitInodo;
                    carpetaNueva.b_content[1].b_inodo = numInodo;
                    strcpy(carpetaNueva.b_content[0].b_name,".");
                    strcpy(carpetaNueva.b_content[1].b_name,"..");
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                    fwrite(&carpetaNueva,sizeof(BLOQUECARPETAS),1,stream);
                    //Guardamos el bit en el bitmap de bloques
                    fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Sobreescribimos el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_free_blocks_count = super.s_free_blocks_count - 3;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 3;
                    fseek(stream,actualSesion.inicioSuper,SEEK_SET);
                    fwrite(&super,sizeof(SUPERBLOQUE),1,stream);
                    return 1;
                }else
                    return 2;
            }else if(bloque == 12 && !flag){
                char buffer = '1';
                //Escribir el numero de bloque en el bloque de apuntadores
                int bitBloque = buscarBit(stream,'B',fit);
                apuntadores.b_pointers[pointer] = bitBloque;
                fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[12],SEEK_SET);
                fwrite(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,stream);
                //Creamos el bloque auxiliar
                int bitInodo = buscarBit(stream,'I',fit);
                carpetaAux = crear_BloqCarpeta();
                carpetaAux.b_content[0].b_inodo = bitInodo;
                strcpy(carpetaAux.b_content[0].b_name,nombreCarpeta);
                fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                fwrite(&carpetaAux,sizeof(BLOQUECARPETAS),1,stream);
                //Escribimos el bit del bloque carpeta en el bitmap
                fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                fwrite(&buffer,sizeof(char),1,stream);
                //Creamos el nuevo inodo carpeta
                inodoNuevo = crear_Inodo(0,'0',664);
                inodoNuevo.i_uid = actualSesion.id_user;
                inodoNuevo.i_gid = actualSesion.id_grp;
                bitBloque = buscarBit(stream,'B',fit);//Carpeta
                inodoNuevo.i_block[0] = bitBloque;
                fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                fwrite(&inodoNuevo,sizeof(INODOTABLA),1,stream);
                //Escribimos el bit del inodo en el bitmap
                fseek(stream,super.s_bm_inode_start + bitInodo,SEEK_SET);
                fwrite(&buffer,sizeof(char),1,stream);
                //Creamos el nuevo bloque carpeta
                carpetaNueva = crear_BloqCarpeta();
                carpetaNueva.b_content[0].b_inodo = bitInodo;
                carpetaNueva.b_content[1].b_inodo = numInodo;
                strcpy(carpetaNueva.b_content[0].b_name,".");
                strcpy(carpetaNueva.b_content[1].b_name,"..");
                fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                fwrite(&carpetaNueva,sizeof(BLOQUECARPETAS),1,stream);
                //Guardamos el bit en el bitmap de bloques
                fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                fwrite(&buffer,sizeof(char),1,stream);
                //Sobreescribimos el super bloque
                super.s_free_inodes_count = super.s_free_inodes_count - 1;
                super.s_free_blocks_count = super.s_free_blocks_count - 2;
                super.s_first_ino = super.s_first_ino + 1;
                super.s_first_blo = super.s_first_blo + 2;
                fseek(stream,actualSesion.inicioSuper,SEEK_SET);
                fwrite(&super,sizeof(SUPERBLOQUE),1,stream);
                return 1;
            }
            else if(bloque == 13){//Apuntador indirecto doble

            }else if(bloque == 14){//Apuntador indirecto triple

            }else{//Apuntadores directos
                bool permissions = permisos_Escritura(inodo.i_perm,(inodo.i_uid == actualSesion.id_user),(inodo.i_gid == actualSesion.id_grp));
                if(permissions || (actualSesion.id_user == 1 && actualSesion.id_grp == 1) ){
                    char buffer = '1';
                    int bitBloque = buscarBit(stream,'B',fit);
                    inodo.i_block[bloque] = bitBloque;
                    //Sobreescribimos el inodo
                    fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                    fwrite(&inodo,sizeof(INODOTABLA),1,stream);
                    //Bloque carpeta auxiliar
                    int bitInodo = buscarBit(stream,'I',fit);
                    carpetaAux = crear_BloqCarpeta();
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreCarpeta);
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BLOQUECARPETAS),1,stream);
                    //Escribimos el bit en el bitmap de blqoues
                    fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Creamos el nuevo inodo
                    inodoNuevo = crear_Inodo(0,'0',664);
                    bitBloque = buscarBit(stream,'B',fit);
                    inodoNuevo.i_block[0] = bitBloque;
                    fseek(stream,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                    fwrite(&inodoNuevo,sizeof(INODOTABLA),1,stream);
                    //Escribimos el bit en el bitmap de inodos
                    fseek(stream,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Creamos el nuevo bloque carpeta
                    carpetaNueva = crear_BloqCarpeta();
                    carpetaNueva.b_content[0].b_inodo = bitInodo;
                    carpetaNueva.b_content[1].b_inodo = numInodo;
                    strcpy(carpetaNueva.b_content[0].b_name,".");
                    strcpy(carpetaNueva.b_content[1].b_name,"..");
                    fseek(stream,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                    fwrite(&carpetaNueva,sizeof(BLOQUECARPETAS),1,stream);
                    //Guardamos el bit en el bitmap de bloques
                    fseek(stream,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,stream);
                    //Sobreescribimos el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_free_blocks_count = super.s_free_blocks_count - 2;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 2;
                    fseek(stream,actualSesion.inicioSuper,SEEK_SET);
                    fwrite(&super,sizeof(SUPERBLOQUE),1,stream);
                    return 1;
                }else
                    return 2;
            }

        }
    }else{//Es un directorio '/home/usac/archivos'
        //Verificar que exista el directorio
        int existe = buscar_CarpetaAr(stream,directorio);
        if(existe == -1){
            if(flagP){
                int index = 0;
                string aux = "";
                //Crear posibles carpetas inexistentes
                for(int i = 0; i < cont; i++){
                    aux += "/"+lista.at(i);
                    char dir[500];
                    char auxDir[500];
                    strcpy(dir,aux.c_str());
                    strcpy(auxDir,aux.c_str());
                    int carpeta = buscar_CarpetaAr(stream,dir);
                    if(carpeta == -1){
                        response = nuevaCarpeta(stream,fit,false,auxDir,index);
                        if(response == 2)
                            break;
                        strcpy(auxDir,aux.c_str());
                        index = buscar_CarpetaAr(stream,auxDir);
                    }else
                        index = carpeta;
                }
            }else
                return 3;
        }else{//Solo crear la carpeta en el directorio
            char dir[100] = "/";
            strcat(dir,nombreCarpeta);
            return nuevaCarpeta(stream,fit,false,dir,existe);
        }
    }

    return response;
}

string SplitFilename (const string& str) {
  unsigned found = str.find_last_of("/\\");
  return str.substr(found+1);
}

int Archivo_nuevo(FILE *fpTemp, char fit, bool flagP, char *path, int size, string contenido, int index,char *auxPath){
    SUPERBLOQUE super;
    INODOTABLA inodo,inodoNuevo;
    BLOQUECARPETAS carpeta, carpetaNueva;
    BLOQUEAPUNTADORES apuntadores;

    vector<string> lista;

    char copiaPath[500];
    char directorio[500];
    char nombreCarpeta[80];

    //simulamos las variables de los contenidos
    string content = "";
    string contentSize = "0123456789";

    //asignacion de variables
    strcpy(copiaPath, path);
    strcpy(directorio, dirname(copiaPath));
    strcpy(copiaPath, path);
    strcpy(nombreCarpeta, basename(copiaPath));
    strcpy(copiaPath,path);

    char *token = strtok(path,"/");
    int cont = 0;
    int numInodo = index;
    int finalSize = size;

    //recorremos hasta que termine los tokens
    while(token != nullptr)
    {
        cont++;
        lista.push_back(token);
        token = strtok(nullptr,"/");
    }

    if(contenido.length() != 0)
    {
        FILE *archivoCont;

        if((archivoCont = fopen(contenido.c_str(),"r")))
        {
            //ubicamos el puntero en el inicio del archivo contenido
            fseek(archivoCont, 0, SEEK_END);
            //tamaño de la finalizacion
            finalSize = static_cast<int>(ftell(archivoCont));
            fseek(archivoCont, 0, SEEK_SET);

            for (int i = 0; i < finalSize; i++)
                content += static_cast<char>(fgetc(archivoCont));
        }else
            return 3;
    }

    //traemos el puntero al inicio del bloque de la sesion
    fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fpTemp);

    if(cont == 1)
    {
        int bloque = 0;
        int pointer = 0;
        int b_content = 0;
        int libre = buscar_ContentDisponible(fpTemp, numInodo, inodo, carpeta, apuntadores, b_content,bloque, pointer);

        //si el bloque esta libre
        if(libre == 1)
        {
            bool permisos = permisos_Escritura(inodo.i_perm, (inodo.i_uid == actualSesion.id_user), (inodo.i_gid == actualSesion.id_grp));
            if(permisos || (actualSesion.id_user == 1 && actualSesion.id_grp == 1)){
                char buffer = '1';
                char buffer2 = '2';
                char buffer3 = '3';

                //Agregamos el archivo al bloque correspondiente
                int bitInodo = buscarBit(fpTemp,'I',fit);
                carpeta.b_content[b_content].b_inodo = bitInodo;
                strcpy(carpeta.b_content[b_content].b_name,nombreCarpeta);
                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*inodo.i_block[bloque],SEEK_SET);
                fwrite(&carpeta,sizeof(BLOQUECARPETAS),1,fpTemp);

                //Creamos el nuevo inodo archivo
                inodoNuevo = crear_Inodo(0,'1',664);
                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                fwrite(&inodoNuevo,sizeof(INODOTABLA),1,fpTemp);

                //Registramos el inodo en el bitmap
                fseek(fpTemp,super.s_bm_inode_start + bitInodo,SEEK_SET);
                fwrite(&buffer,sizeof(char),1,fpTemp);

                //Si viene el parametro -size/-cont
                if(finalSize != 0)
                {
                    double n = static_cast<double>(finalSize)/static_cast<double>(63);
                    int numBloques = static_cast<int>(ceil(n));
                    int caracteres = finalSize;
                    size_t charNum = 0;
                    size_t contChar = 0;
                    numInodo = buscar_CarpetaAr(fpTemp,auxPath);

                    for (int i = 0; i < numBloques; i++) 
                    {
                        BLOQUEARCHIVOS archivo;
                        //llenamos el bloque de memorio con el archivo del bloque contenido
                        memset(archivo.b_content, 0, sizeof(archivo.b_content));

                        if(i == 12)
                        {
                            int bitBloqueA = buscarBit(fpTemp,'B',fit);//Apuntador
                            //Guardamos el bloque en el inodo archivo

                            fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                            fread(&inodo,sizeof(INODOTABLA),1,fpTemp);
                            inodo.i_block[i] = bitBloqueA;
                            fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                            fwrite(&inodo,sizeof(INODOTABLA),1,fpTemp);
                            //Registramos el bloque apuntador en el bitmap

                            fseek(fpTemp,super.s_bm_block_start + bitBloqueA,SEEK_SET);
                            fwrite(&buffer3,sizeof(char),1,fpTemp);
                            //Creamos el bloque de apuntadores como es simple solo se debe crear uno

                            int bitBloque = buscarBit(fpTemp,'B',fit);//Primer bloque archivo
                            apuntadores.b_pointers[0] = bitBloque;
                            for(int i = 1; i < 16; i++)
                                apuntadores.b_pointers[i] = -1;
                            fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*bitBloqueA,SEEK_SET);
                            fwrite(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,fpTemp);
                            //Empezamos con la creacion de bloques archivos

                            //Registramos el primer bloque en el bitmap
                            fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                            fwrite(&buffer2,sizeof(char),1,fpTemp);

                            if(caracteres > 63)
                            {
                                for(int j = 0; j < 63; j++)
                                {
                                    if(content.length() != 0)
                                    {//-cont
                                        archivo.b_content[j] = content[contChar];
                                        contChar++;
                                    }else{//-size
                                        archivo.b_content[j] = contentSize[charNum];
                                        charNum++;
                                        if(charNum == 10)
                                            charNum = 0;
                                    }
                                }

                                //Guardamos el bloque
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                caracteres -= 63;

                            }else{
                                for (int j = 0; j < caracteres; j++) 
                                {
                                    if(content.length() != 0)
                                    {//llnado del content 
                                        archivo.b_content[j] = content[contChar];
                                        contChar++;
                                    }else{
                                        archivo.b_content[j] = contentSize[charNum];
                                        charNum++;
                                        if(charNum == 10)
                                            charNum = 0;
                                    }
                                }
                                //Guardamos el bloque
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                            }
                        }else if(i > 12 && i < 28)
                        {
                            int libre = 0;
                            //apuntamos al arhhivo del incio de su tabla de inodos
                            fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                            fread(&inodo,sizeof(INODOTABLA),1,fpTemp);
                            fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[12],SEEK_SET);
                            fread(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,fpTemp);

                            for (int a = 0; a < 16; a++) 
                            {
                                if(apuntadores.b_pointers[a] == -1)
                                {//apuntador bloque libre
                                    libre = a;
                                    break;
                                }
                            }
                            int bitBloque = buscarBit(fpTemp,'B',fit);
                            apuntadores.b_pointers[libre] = bitBloque;

                            fseek(fpTemp, super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[12], SEEK_SET);
                            fwrite(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,fpTemp);
                            //Registramos el bloque en el bitmap
                            fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                            fwrite(&buffer2,sizeof(char),1,fpTemp);

                            if(caracteres > 63)
                            {
                                for(int j = 0; j < 63; j++)
                                {
                                    if(content.length() != 0)
                                    {//-cont
                                        archivo.b_content[j] = content[contChar];
                                        contChar++;
                                    }else{//-size
                                        archivo.b_content[j] = contentSize[charNum];
                                        charNum++;
                                        if(charNum == 10)
                                            charNum = 0;
                                    }
                                }
                                //Guardamos el bloque
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                caracteres -= 63;

                            }else{
                                for (int j = 0; j < caracteres; j++) 
                                {
                                    if(content.length() != 0)
                                    {
                                        archivo.b_content[j] = content[contChar];
                                        contChar++;
                                    }else{
                                        archivo.b_content[j] = contentSize[charNum];
                                        charNum++;
                                        if(charNum == 10)
                                            charNum = 0;
                                    }
                                }
                                //Guardamos el bloque llenado
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                            }
                        }else if(i == 29){//Apuntador doble

                        }else{
                            int bitBloque = buscarBit(fpTemp, 'B', fit);
                            //Registramos el bloque en el bitmap
                            fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                            fwrite(&buffer2,sizeof(char),1,fpTemp);

                            //si no ses modifico en los pasos anteriores
                            if(caracteres > 63)
                            {
                                for(int j = 0; j < 63; j++)
                                {
                                    if(content.length() != 0)
                                    {//-cont
                                        archivo.b_content[j] = content[contChar];
                                        contChar++;
                                    }else{//-size
                                        archivo.b_content[j] = contentSize[charNum];
                                        charNum++;
                                        if(charNum == 10)
                                            charNum = 0;
                                    }
                                }
                                //Guardamos el bloque en el respectivo inodo archivo
                                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                fread(&inodo,sizeof(INODOTABLA),1,fpTemp);

                                //GUardamos el bloque
                                inodo.i_block[i] = bitBloque;
                                inodo.i_size = finalSize;
                                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                fwrite(&inodo,sizeof(INODOTABLA), 1, fpTemp);
                                
                                //Guardamos el bloque
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                caracteres -= 63;

                            }else
                            {
                                for (int j = 0; j < caracteres; j++)
                                {
                                    if(content.length() != 0)
                                    {//llenamos los caracteres
                                        archivo.b_content[j] = content[contChar];
                                        contChar++;
                                    }else{
                                        archivo.b_content[j] = contentSize[charNum];
                                        charNum++;
                                        if(charNum == 10)
                                            charNum = 0;
                                    }
                                }
                                //Guardamos el bloque en el respectivo inodo archivo
                                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                fread(&inodo,sizeof(INODOTABLA),1,fpTemp);
                                inodo.i_block[i] = bitBloque;
                                inodo.i_size = finalSize;

                                //apuntamos al inicio del super
                                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                fwrite(&inodo,sizeof(INODOTABLA),1,fpTemp);
                                //Guardamos el bloque
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                            }
                        }
                    }
                    //Modificamos el super bloque
                    super.s_free_blocks_count = super.s_free_blocks_count - numBloques;
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + numBloques; 

                    //apuntamos y guardamos en el super bloque el inicio de la sesion del bloque
                    fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
                    fwrite(&super,sizeof(SUPERBLOQUE),1,fpTemp);
                    return 1;
                }

                super.s_free_inodes_count = super.s_free_inodes_count - 1;
                super.s_first_ino = super.s_first_ino + 1;

                //aputamos el apuntador nen el inicio del bloque y cargamos los datos en todo el sper bloque
                fseek(fpTemp, actualSesion.inicioSuper, SEEK_SET);
                fwrite(&super, sizeof(SUPERBLOQUE), 1, fpTemp);
                return 1;
            }else
                return 2;
        }else{//Todos los bloques estan llenos
            //apuntar en el incio del nodo del inodo
            fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo, SEEK_SET);
            fread(&inodo, sizeof(INODOTABLA), 1, fpTemp);

            for (int i = 0; i < 15; i++) 
            {
                if(inodo.i_block[i] == -1)
                {
                    bloque = i;
                    break;
                }
            }
            if(bloque == 12){//Apuntador simple indirecto
                bool permissions = permisos_Escritura(inodo.i_perm,(inodo.i_uid == actualSesion.id_user),(inodo.i_gid == actualSesion.id_grp));
                if(permissions || (actualSesion.id_user == 1 && actualSesion.id_grp == 1)){
                }else
                    return 2;
            }else if(bloque == 13){//Apuntador doble indirecto

            }else if(bloque == 14){//Apuntador triple indirecto

            }else{//Apuntadores directos
                bool permisos = permisos_Escritura(inodo.i_perm,(inodo.i_uid == actualSesion.id_user),(inodo.i_gid == actualSesion.id_grp));
                
                if(permisos || (actualSesion.id_user == 1 && actualSesion.id_grp == 1))
                {
                    char buffer = '1';
                    char buffer2 = '2';
                    char buffer3 = '3';
                    int bitBloque = buscarBit(fpTemp,'B',fit);
                    //Guardamos el bloque en el inodo
                    inodo.i_block[bloque] = bitBloque;
                    fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                    fwrite(&inodo,sizeof(INODOTABLA),1,fpTemp);
                    //Creamos el nuevo bloque carpeta

                    //llenamos los datos de la carpeta
                    int bitInodo = buscarBit(fpTemp,'I',fit);
                    carpetaNueva.b_content[0].b_inodo = bitInodo;
                    carpetaNueva.b_content[1].b_inodo = -1;
                    carpetaNueva.b_content[2].b_inodo = -1;
                    carpetaNueva.b_content[3].b_inodo = -1;
                    strcpy(carpetaNueva.b_content[0].b_name,nombreCarpeta);
                    strcpy(carpetaNueva.b_content[1].b_name,"");
                    strcpy(carpetaNueva.b_content[2].b_name,"");
                    strcpy(carpetaNueva.b_content[3].b_name,"");

                    //apuntamos al inicio del bloque de carpeta
                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*bitBloque,SEEK_SET);
                    fwrite(&carpetaNueva,sizeof(BLOQUECARPETAS),1,fpTemp);

                    //Registramos el bloque en el bitmap
                    fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,fpTemp);

                    //Creamos el nuevo inodo
                    inodoNuevo = crear_Inodo(0,'1',664);
                    //apuntamos y guardamos al bit del inodo 
                    fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                    fwrite(&inodoNuevo,sizeof(INODOTABLA),1,fpTemp);
                    //guardamso en tabla inodo

                    fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*bitInodo,SEEK_SET);
                    fwrite(&inodoNuevo,sizeof(INODOTABLA),1,fpTemp);
                    //Registramos el inodo en el bitmap
                    fseek(fpTemp,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    fwrite(&buffer,sizeof(char),1,fpTemp);
                    //Si viene el parametro -size/-cont

                    if(finalSize != 0)
                    {
                        double n = static_cast<double>(finalSize)/static_cast<double>(63);
                        int numBloques = static_cast<int>(ceil(n));
                        int caracteres = finalSize;
                        size_t charNum = 0;
                        size_t contChar = 0;

                        numInodo = buscar_CarpetaAr(fpTemp,auxPath);
                        for (int i = 0; i < numBloques; i++) 
                        {
                            BLOQUEARCHIVOS archivo;
                            //hacemos la copia de datos
                            memset(archivo.b_content, 0, sizeof(archivo.b_content));
                            if(i == 12)
                            {
                                int bitBloqueA = buscarBit(fpTemp,'B',fit);//Apuntador
                                //Guardamos el bloque en el inodo archivo
                                
                                //apuntamos y leemos el nodo de inicio en el que estara el i nodo(bitmap)
                                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                fread(&inodo,sizeof(INODOTABLA),1,fpTemp);
                                inodo.i_block[i] = bitBloqueA;

                                fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                fwrite(&inodo,sizeof(INODOTABLA),1,fpTemp);
                                //Registramos el bloque apuntador en el bitmap

                                fseek(fpTemp,super.s_bm_block_start + bitBloqueA,SEEK_SET);
                                fwrite(&buffer3,sizeof(char),1,fpTemp);
                                //Creamos el bloque de apuntadores como es simple solo se debe crear uno

                                int bitBloque = buscarBit(fpTemp,'B',fit);//Primer bloque archivo
                                apuntadores.b_pointers[0] = bitBloque;

                                for(int i = 1; i < 16; i++)
                                    apuntadores.b_pointers[i] = -1;
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*bitBloqueA,SEEK_SET);
                                fwrite(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,fpTemp);
                                //Empezamos con la creacion de bloques archivos

                                //Registramos el primer bloque en el bitmap
                                fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                                fwrite(&buffer2,sizeof(char),1,fpTemp);

                                if(caracteres > 63)
                                {
                                    for(int j = 0; j < 63; j++)
                                    {
                                        if(content.length() != 0)
                                        {//-cont
                                            archivo.b_content[j] = content[contChar];
                                            contChar++;
                                        }else{//-size
                                            archivo.b_content[j] = contentSize[charNum];
                                            charNum++;
                                            if(charNum == 10)
                                                charNum = 0;
                                        }
                                    }
                                    //Guardamos el bloque
                                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                    caracteres -= 63;

                                }else{
                                    for (int j = 0; j < caracteres; j++) 
                                    {
                                        if(content.length() != 0)
                                        {
                                            archivo.b_content[j] = content[contChar];
                                            contChar++;
                                        }else{
                                            archivo.b_content[j] = contentSize[charNum];
                                            charNum++;
                                            if(charNum == 10)
                                                charNum = 0;
                                        }
                                    }
                                    //Guardamos el bloque
                                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                }
                            }else if(i > 12 && i < 28)
                            {
                                int libre = 0;
                                //ubicamos el bitmap y leemos
                                fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo, SEEK_SET);
                                fread(&inodo,sizeof(INODOTABLA),1,fpTemp);
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[12],SEEK_SET);
                                fread(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,fpTemp);

                                for (int a = 0; a < 16; a++) 
                                {
                                    if(apuntadores.b_pointers[a] == -1)
                                    {
                                        libre = a;
                                        break;
                                    }
                                }

                                int bitBloque = buscarBit(fpTemp,'B',fit);
                                //apuntamos al nuevo bitmap
                                apuntadores.b_pointers[libre] = bitBloque;
                                fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[12],SEEK_SET);
                                fwrite(&apuntadores,sizeof(BLOQUEAPUNTADORES),1,fpTemp);
                                //Registramos el bloque en el bitmap
                                fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                                fwrite(&buffer2,sizeof(char),1,fpTemp);

                                if(caracteres > 63)
                                {
                                    for(int j = 0; j < 63; j++)
                                    {
                                        if(content.length() != 0)
                                        {//-cont
                                            archivo.b_content[j] = content[contChar];
                                            contChar++;
                                        }else{//-size
                                            archivo.b_content[j] = contentSize[charNum];
                                            charNum++;
                                            if(charNum == 10)
                                                charNum = 0;
                                        }
                                    }
                                    //Guardamos el bloque
                                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                    caracteres -= 63;
                                }else{
                                    for (int j = 0; j < caracteres; j++) {
                                        if(content.length() != 0){
                                            archivo.b_content[j] = content[contChar];
                                            contChar++;
                                        }else{
                                            archivo.b_content[j] = contentSize[charNum];
                                            charNum++;
                                            if(charNum == 10)
                                                charNum = 0;
                                        }
                                    }
                                    //Guardamos el bloque
                                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                }

                            }else if(i == 29){//Apuntador doble

                            }else{//Apuntadores simples
                                int bitBloque = buscarBit(fpTemp,'B',fit);
                                //Registramos el bloque en el bitmap
                                fseek(fpTemp,super.s_bm_block_start + bitBloque,SEEK_SET);
                                fwrite(&buffer2,sizeof(char),1,fpTemp);

                                if(caracteres > 63)
                                {
                                    for(int j = 0; j < 63; j++)
                                    {
                                        if(content.length() != 0)
                                        {//-cont
                                            archivo.b_content[j] = content[contChar];
                                            contChar++;
                                        }else{//-size
                                            archivo.b_content[j] = contentSize[charNum];
                                            charNum++;
                                            if(charNum == 10)
                                                charNum = 0;
                                        }
                                    }
                                    //Guardamos el bloque en el respectivo inodo archivo
                                    fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                    fread(&inodo, sizeof(INODOTABLA), 1, fpTemp);
                                    inodo.i_block[i] = bitBloque;

                                    //inodo del arhicvo de su bitmap
                                    fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                    fwrite(&inodo,sizeof(INODOTABLA),1,fpTemp);

                                    //Guardamos el bloque
                                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                    caracteres -= 63;
                                }else{
                                    for (int j = 0; j < caracteres; j++) 
                                    {
                                        if(content.length() != 0)
                                        {
                                            archivo.b_content[j] = content[contChar];
                                            contChar++;
                                        }else{
                                            archivo.b_content[j] = contentSize[charNum];
                                            charNum++;
                                            if(charNum == 10)
                                                charNum = 0;
                                        }
                                    }
                                    //Guardamos el bloque en el respectivo inodo archivo
                                    fseek(fpTemp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                    fread(&inodo, sizeof(INODOTABLA), 1, fpTemp);
                                    inodo.i_block[i] = bitBloque;
                                    fseek(fpTemp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*numInodo,SEEK_SET);
                                    fwrite(&inodo, sizeof(INODOTABLA), 1, fpTemp);
                                    //Guardamos el bloque
                                    fseek(fpTemp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*bitBloque,SEEK_SET);
                                    fwrite(&archivo,sizeof(BLOQUEARCHIVOS),1,fpTemp);
                                }
                            }
                        }
                        //Modificamos el super bloque
                        //rescribimos los valores del super bloque
                        super.s_free_blocks_count = super.s_free_blocks_count - numBloques;
                        super.s_free_inodes_count = super.s_free_inodes_count - 1;
                        super.s_first_ino = super.s_first_ino + 1;
                        super.s_first_blo = super.s_first_blo + numBloques;

                        //guardamos enn el actual los cambios realizados
                        fseek(fpTemp,actualSesion.inicioSuper,SEEK_SET);
                        fwrite(&super,sizeof(SUPERBLOQUE),1,fpTemp);
                        return 1;
                    }
                    
                    //rescribimos lo que esta en eun bloque predifinido
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;

                    fseek(fpTemp,actualSesion.inicioSuper,SEEK_SET);
                    fwrite(&super,sizeof(SUPERBLOQUE), 1, fpTemp);
                    return 1;
                }else
                    return 2;
            }
        }
    }else{//Directorio
        int existe = buscar_CarpetaAr(fpTemp,directorio);
        if(existe == -1)
        {
            if(flagP)
            {//so tiene el permiso
                int index = 0;
                string aux = "";
                //Crear posibles carpetas inexistentes
                for (int i = 0; i < cont; i++) 
                {
                    if(i == cont -1)
                    {//obtenes cada token
                        char dir[100] = "/";
                        strcat(dir, nombreCarpeta);
                        //creamos el nuevo archivo
                        return Archivo_nuevo(fpTemp, fit, false, dir, size, contenido, index, auxPath);
                    }else{//si ya existe el archivo
                        aux += "/"+lista.at(i);
                        char dir[500];
                        char auxDir[500]; 

                        strcpy(dir, aux.c_str());
                        strcpy(auxDir, aux.c_str());
                        int carpeta = buscar_CarpetaAr(fpTemp,dir);
                        if(carpeta == -1)
                        {//si la siguiente carpeta no existe la crea
                            nuevaCarpeta(fpTemp,fit,false,auxDir,index);
                            strcpy(auxDir, aux.c_str());
                            index = buscar_CarpetaAr(fpTemp,auxDir);
                        }else
                            index = carpeta;
                    }
                }
            }else
                return 4;
        }else{//Crear el archivo en el directorio
            char dir[100] = "/";
            strcat(dir, nombreCarpeta);
            return Archivo_nuevo(fpTemp,fit,false,dir,size,contenido,existe,auxPath);
        }
    }

    return 0;
}

int Archivo_crear(string ruta, bool p, int size, string cont){
    //abrimos el archivo en la direccion indicada
    FILE *fp = fopen(actualSesion.direccion.c_str(), "rb+");

    SUPERBLOQUE super;
    //nos ubicamos en el puntero de el inicio del bloque de la session
    fseek(fp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);

    //llenado de datos
    char auxPath[500];
    char auxPath2[500];
    strcpy(auxPath, ruta.c_str());
    strcpy(auxPath2, ruta.c_str());
    int existe = buscar_CarpetaAr(fp,auxPath);
    strcpy(auxPath, ruta.c_str());

    int response = -1;

    //existencia del arhcivo
    if(existe != -1)
        response = 0;
    else
        response = Archivo_nuevo(fp, actualSesion.fit, p, auxPath, size, cont, 0, auxPath2);

    fclose(fp);
    return response;
}

int Carpeta_crear(string ruta, bool p){
    //abrimas el usuario que accedera el usuario.txt
    FILE *fp = fopen(actualSesion.direccion.c_str(), "rb+");

    SUPERBLOQUE super;
    //apuntamos al inicio de la secion de su bloque
    fseek(fp, actualSesion.inicioSuper, SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);

    string aux = ruta;
    char auxPath[500];
    strcpy(auxPath,aux.c_str());
    int existe = buscar_CarpetaAr(fp,auxPath);
    strcpy(auxPath,aux.c_str());
    int response = -1;

    //si existe carpeta
    if(existe != -1)
        response = 0;
    else
        response = nuevaCarpeta(fp,actualSesion.fit,p,auxPath,0);//creamos carpeta

    fclose(fp);

    return response;
}

string SplitFolder(const string& str)
{
  size_t found;
  cout << "Splitting: " << str << endl;
  found=str.find_last_of("/\\");
  cout << " folder: " << str.substr(0,found) << endl;
  return str.substr(0,found);
}

string ObtenerExtension(string direccion){
    string aux = direccion;
    string delimiter = ".";
    size_t pos = 0;
    while((pos = aux.find(delimiter))!=string::npos){
        aux.erase(0,pos+delimiter.length());
    }
    return string(aux);
}

void REPORTE_DISK(string auxPath, string auxId){
    FILE *fp;
    FILE *graphDot;

    string extension = auxPath.substr(auxPath.find_last_of(".")+1);
    string destino = "";
    string ruta = "";

    //bandera del ID
    bool flagId = false;

    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if (auxId == Arreglomount[i].id)
        {
            ruta = Arreglomount[i].direccion;
            flagId = true;
            break;
        }
    }
    

    destino = auxPath;
    if (flagId)
    {
        //abrimos la ruta del ID
        if((fp = fopen(ruta.c_str(), "r")))
        {
            //emepzamos a llenar la estructura del diagraph, con el bloque principal
            graphDot = fopen("grafica.dot", "w");
            fprintf(graphDot,"digraph G{\n\n");
            fprintf(graphDot, "  tbl [\n    shape=box\n    label=<\n");
            fprintf(graphDot, "     <table border=\'0\' cellborder=\'2\' width=\'600\' height=\"200\" color=\'LIGHTSTEELBLUE\'>\n");
            fprintf(graphDot, "     <tr>\n");
            fprintf(graphDot, "     <td height=\'200\' width=\'100\'> MBR </td>\n");

            MBR mbr;
            //apuntamos y leemos el mbr
            fseek(fp, 0, SEEK_SET);
            fread(&mbr, sizeof(MBR), 1, fp);

            int tamanio = mbr.MBR_tamanio;
            double espacioUsado = 0;
            
            //recorremos las 4 particiones
            for(int i = 0; i < 4; i++)
            {
                //tamaño termoral de cada particion
                int temp_size = mbr.MBR_partition[i].part_size;

                if(mbr.MBR_partition[i].part_start != -1)
                {//Particion vacia
                    double porcet = (temp_size*100)/tamanio;
                    double porcet_2 = (porcet*500)/100;
                    espacioUsado += porcet;

                    if(mbr.MBR_partition[i].part_status != '1')
                    {
                        if(mbr.MBR_partition[i].part_type == 'P')
                        {
                            //llenamos con los datos de las pimarias por medio de abstraccion de variables
                            fprintf(graphDot, "     <td height=\'200\' width=\'%.1f\'>PRIMARIA <br/> Ocupado: %.1f%c</td>\n",porcet_2,porcet,'%');
                            //Verificar que no haya espacio fragmentado
                            if(i!=3)
                            {
                                //calculamos los espacios
                                int data1 = mbr.MBR_partition[i].part_start + mbr.MBR_partition[i].part_size;
                                int punto2 = mbr.MBR_partition[i+1].part_start;

                                if(mbr.MBR_partition[i+1].part_start != -1)
                                {
                                    if((punto2-data1)!=0)
                                    {//Hay fragmentacion
                                        int fragmentacion = punto2-data1;
                                        double porcet = (fragmentacion*100)/tamanio;
                                        double porcet_2 = (porcet*500)/100;
                                        fprintf(graphDot,"     <td height=\'200\' width=\'%.1f\'>LIBRE<br/> Ocupado: %.1f%c</td>\n",porcet_2,porcet,'%');
                                    }
                                }

                            }else{
                                //calculos de tamaños
                                int data1 = mbr.MBR_partition[i].part_start + mbr.MBR_partition[i].part_size;
                                int mbr_size = tamanio + (int)sizeof(MBR);

                                if((mbr_size-data1)!=0){//Libre
                                    double libre = (mbr_size - data1) + sizeof(MBR);
                                    double porcet = (libre*100)/tamanio;
                                    double porcet_2 = (porcet*500)/100;
                                    fprintf(graphDot, "     <td height=\'200\' width=\'%.1f\'>LIBRE<br/> Ocupado: %.1f%c</td>\n",porcet_2, porcet, '%');
                                }
                            }
                        }else{//Extendida
                            EBR ebr;

                            fprintf(graphDot,"     <td  height=\'200\' width=\'%.1f\'>\n     <table border=\'0\'  height=\'200\' WIDTH=\'%.1f\' cellborder=\'1\'>\n",porcet,porcet);
                            fprintf(graphDot,"     <tr>  <td height=\'60\' colspan=\'15\'>EXTENDIDA</td>  </tr>\n     <tr>\n");
                            fseek(fp, mbr.MBR_partition[i].part_start,SEEK_SET); //apuntamos al inico de cara particion
                            fread(&ebr,sizeof(EBR),1,fp);//leemos el EBR resultante

                            if(ebr.EBR_part_fit != 0)
                            {//Si hay mas de alguna logica
                                fseek(fp, mbr.MBR_partition[i].part_start,SEEK_SET);
                                while(fread(&ebr,sizeof (EBR),1,fp)!=0 && (ftell(fp) < (mbr.MBR_partition[i].part_start + mbr.MBR_partition[i].part_size)))
                                {//entramos a la lectura del EBR, este recorrera mientras encaje en particiones y tamaño
                                    temp_size = ebr.EBR_part_size;
                                    porcet = (temp_size*100)/tamanio;

                                    if(porcet!=0)
                                    {//porcentaje ocupado
                                        if(ebr.EBR_part_status != '1')
                                        {//si esta ocupada la logica
                                            fprintf(graphDot, "     <td height=\'140\'>EBR</td>\n");
                                            fprintf(graphDot, "     <td height=\'140\'>LOGICA<br/>Ocupado: %.1f%c</td>\n",porcet,'%');
                                        }else{//Espacio no asignado
                                            fprintf(graphDot, "      <td height=\'150\'>LIBRE 1 <br/> Ocupado: %.1f%c</td>\n",porcet,'%');
                                        }
                                        if(ebr.EBR_part_next==-1)
                                        {//calculamos las que estan libres en tamaños
                                            temp_size = (mbr.MBR_partition[i].part_start + mbr.MBR_partition[i].part_size) - (ebr.EBR_part_start + ebr.EBR_part_size);
                                            porcet = (temp_size*100)/tamanio;
                                            if(porcet!=0){
                                                fprintf(graphDot, "     <td height=\'150\'>LIBRE 2<br/> Ocupado: %.1f%c </td>\n",porcet,'%');
                                            }
                                            break;
                                        }else
                                            //apuntamos a las vacias
                                            fseek(fp, ebr.EBR_part_next, SEEK_SET);
                                    }
                                }
                            }else{//porcentaje de la logica oucapda
                                fprintf(graphDot,"     <td height=\'140\'> Ocupado %.1f%c</td>",porcet,'%');
                            }
                            fprintf(graphDot,"     </tr>\n     </table>\n     </td>\n");
                            //Verificar que no haya espacio fragmentado
                            if(i!=3)
                            {//particion 3
                                //tamaños en esta particion
                                int data1 = mbr.MBR_partition[i].part_start + mbr.MBR_partition[i].part_size;
                                int punto2 = mbr.MBR_partition[i+1].part_start;

                                if(mbr.MBR_partition[i+1].part_start != -1)
                                {//si no estan ocupadas
                                    if((punto2-data1)!=0)
                                    {//Hay fragmentacion
                                        int fragmentacion = punto2-data1;
                                        double porcet = (fragmentacion*100)/tamanio;
                                        double porcet_2 = (porcet*500)/100;
                                        fprintf(graphDot,"     <td height=\'200\' width=\'%.1f\'>LIBRE<br/> Ocupado: %.1f%c</td>\n",porcet_2,porcet,'%');
                                    }
                                }
                            }else{//particion 4
                                int data1 = mbr.MBR_partition[i].part_start + mbr.MBR_partition[i].part_size;
                                int mbr_size = tamanio + (int)sizeof(MBR);

                                if((mbr_size-data1)!=0)
                                {//Libre, fragmentacion
                                    double libre = (mbr_size - data1) + sizeof(MBR);
                                    double porcet = (libre*100)/tamanio;
                                    double porcet_2 = (porcet*500)/100;
                                    fprintf(graphDot, "     <td height=\'200\' width=\'%.1f\'>LIBRE<br/> Ocupado: %.1f%c</td>\n",porcet_2, porcet, '%');
                                }
                            }
                        }
                    }else{//Espacio no asignado
                        fprintf(graphDot,"     <td height=\'200\' width=\'%.1f\'>LIBRE <br/> Ocupado: %.1f%c</td>\n",porcet_2,porcet,'%');
                    }
                }
            }
            //cierre de la grafica
            fprintf(graphDot,"     </tr> \n     </table>        \n>];\n\n}");
            fclose(graphDot);
            fclose(fp);
            string comando = "dot -T"+ ObtenerExtension(auxPath) +" grafica.dot -o " + destino;
            system(comando.c_str());
            cout << "\033[31m\n ---- Reporte DISK generado con exito ----\n\033[0m" << endl;
        }else{
            cout << "\033[94m\n ... Error, no existe el disco ...\n\033[0m" << endl;
        }
    }else
    {
        cout << "\033[94m\n ... Error, no se monto la particion ...\n\033[0m" << endl;
    }
}

void REPORTE_MBR(string auxPath, string auxId){
    string destino = auxPath;
    FILE *fp;
    FILE *graphDot;
    bool flagId = false;

    //tipo de extension
    string extension = auxPath.substr(auxPath.find_last_of(".")+1);
    string ruta = "";

    //buscamos el ID del Mout
    for (int i = 0; i < Arreglomount.size(); i++)
    {
        if (auxId == Arreglomount[i].id)
        {
            ruta = Arreglomount[i].direccion;
            flagId = true;
            break;
        }
    }

    if(flagId)
    {
        if((fp = fopen(ruta.c_str(),"r")))
        {
            //llenado de la parte principal
            //graphDot se utiliza par el diagraph
            graphDot = fopen("grafica.dot", "w");
            //fprintf fuciona para ir agrgando lineas al diagrama
            fprintf(graphDot,"digraph G{ \n");
            fprintf(graphDot,"subgraph cluster{\n label=\"MBR\"");
            fprintf(graphDot,"\ntbl[shape=box,label=<\n");
            fprintf(graphDot,"<table border=\'0\' cellborder=\'1\' cellspacing=\'0\' width=\'300\'  height=\'200\' >\n");
            fprintf(graphDot, "<tr>  <td width=\'150\'> <b>Nombre</b> </td> <td width=\'150\'> <b>Valor</b> </td>  </tr>\n");
            
            MBR mbr;
            fseek(fp, 0, SEEK_SET);
            fread(&mbr, sizeof(MBR), 1, fp);

            int tamanio = mbr.MBR_tamanio;

            //el porcentaje d es la llamada del tamaño
            fprintf(graphDot,"<tr>  <td><b>mbr_tamaño</b></td><td>%d</td>  </tr>\n",tamanio);

            //creamos una estructura temporal para el tiempo realizado
            struct tm *tm;
            char fecha[100];
            tm = localtime(&mbr.MBR_fecha_creacion);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graphDot,"<tr>  <td><b>mbr_fecha_creacion</b></td> <td>%s</td>  </tr>\n",fecha);
            fprintf(graphDot,"<tr>  <td><b>mbr_disk_signature</b></td> <td>%d</td>  </tr>\n",mbr.MBR_disk_signature);
            fprintf(graphDot,"<tr>  <td><b>Disk_fit</b></td> <td>%c</td>  </tr>\n",mbr.disk_fit);

            int index_Extendida = -1; //si no hay extendida
            for (int i = 0; i < 4; i++)
            {
                //si existe particion
                if(mbr.MBR_partition[i].part_start!=-1 && mbr.MBR_partition[i].part_status!='1')
                {
                    //si existe particion tipo Extendida
                    if(mbr.MBR_partition[i].part_type == 'E')
                    {
                        index_Extendida = i;
                    }
                    char status[3];

                    //si es accedido
                    if(mbr.MBR_partition[i].part_status == '0')
                        strcpy(status,"0");
                    else if(mbr.MBR_partition[i].part_status == '2')
                        strcpy(status,"2");
                    //llenamos atributos del MBR
                    fprintf(graphDot,"<tr>  <td><b>part_status_%d</b></td> <td>%s</td>  </tr>\n",(i+1),status);
                    fprintf(graphDot,"<tr>  <td><b>part_type_%d</b></td> <td>%c</td>  </tr>\n",(i+1),mbr.MBR_partition[i].part_type);
                    fprintf(graphDot,"<tr>  <td><b>part_fit_%d</b></td> <td>%c</td>  </tr>\n",(i+1),mbr.MBR_partition[i].part_fit);
                    fprintf(graphDot,"<tr>  <td><b>part_start_%d</b></td> <td>%d</td>  </tr>\n",(i+1),mbr.MBR_partition[i].part_start);
                    fprintf(graphDot,"<tr>  <td><b>part_size_%d</b></td> <td>%d</td>  </tr>\n",(i+1),mbr.MBR_partition[i].part_size);
                    fprintf(graphDot,"<tr>  <td><b>part_name_%d</b></td> <td>%s</td>  </tr>\n",(i+1),mbr.MBR_partition[i].part_name);
                }
            }

            //finalizacion del grafo si hay extendida
            fprintf(graphDot,"</table>\n");
            fprintf(graphDot, ">];\n}\n");

            //si hay extendida
            if(index_Extendida != -1)
            {
                int posicion_eber = 1;
                EBR ebr;

                fseek(fp,mbr.MBR_partition[index_Extendida].part_start,SEEK_SET);

                //se ejecutara hasta que el archivo se termine de leer
                while(fread(&ebr,sizeof(EBR),1,fp)!=0 && (ftell(fp) < mbr.MBR_partition[index_Extendida].part_start + mbr.MBR_partition[index_Extendida].part_size)) 
                {
                    //llenamos la extendida
                    if(ebr.EBR_part_status != '1')
                    {
                        fprintf(graphDot,"subgraph cluster_%d{\n label=\"EBR_%d\"\n", posicion_eber, posicion_eber);
                        fprintf(graphDot,"\ntbl_%d[shape=box, label=<\n ",posicion_eber);
                        fprintf(graphDot, "<table border=\'0\' cellborder=\'1\' cellspacing=\'0\'  width=\'300\' height=\'160\' >\n ");
                        fprintf(graphDot, "<tr>  <td width=\'150\'><b>Nombre</b></td> <td width=\'150\'><b>Valor</b></td>  </tr>\n");
                        
                        char status[3];
                        //verificacamos si esta disponible
                        if(ebr.EBR_part_status == '0')
                            strcpy(status,"0");
                        else if(ebr.EBR_part_status == '2')
                            strcpy(status,"2");
                        fprintf(graphDot, "<tr>  <td><b>part_status_1</b></td> <td>%s</td>  </tr>\n",status);
                        fprintf(graphDot, "<tr>  <td><b>part_fit_1</b></td> <td>%c</td>  </tr>\n",ebr.EBR_part_fit);
                        fprintf(graphDot, "<tr>  <td><b>part_start_1</b></td> <td>%d</td>  </tr>\n",ebr.EBR_part_start);
                        fprintf(graphDot, "<tr>  <td><b>part_size_1</b></td> <td>%d</td>  </tr>\n",ebr.EBR_part_size);
                        fprintf(graphDot, "<tr>  <td><b>part_next_1</b></td> <td>%d</td>  </tr>\n",ebr.EBR_part_next);
                        fprintf(graphDot, "<tr>  <td><b>part_name_1</b></td> <td>%s</td>  </tr>\n",ebr.EBR_part_name);
                        fprintf(graphDot, "</table>\n");
                        fprintf(graphDot, ">];\n}\n");
                        posicion_eber++;
                    }

                    if(ebr.EBR_part_next == -1)
                        break;
                    else
                        fseek(fp, ebr.EBR_part_next, SEEK_SET);
                }
            }

            fprintf(graphDot,"}\n");
            fclose(graphDot);
            fclose(fp);
            string comando = "dot -T"+ extension+" grafica.dot -o "+ destino;
            system(comando.c_str());
            cout << "Reporte generado con exito " << endl;
            cout << "\033[31m\n--- Reporte MBR generado con exito ---.\n\033[0m" << endl;
        }else
        {
            cout << "\033[94m\n ... Error, no existe el disco ...\n\033[0m" << endl;
        }
    }else
    {
        cout << "\033[94m\n ... Error, no se monto la particion ...\n\033[0m" << endl;
    }
}

void REPORTE_INODO(string auxPath, string destino, string extension, int bm_inode_start,int inode_start,int bm_block_start){

    FILE *fp = fopen(auxPath.c_str(), "r");

    INODOTABLA inodo;
    int aux = bm_inode_start;
    int num = 0;
    char buff;

    //Archivo para esacribir el dot
    FILE *graph = fopen("grafica.dot","w");
    fprintf(graph,"digraph G{\n\n");

    //mientras el bitmap bitmap de nodos es menor al bitmap de bloque entonces ejecuta
    while(aux < bm_block_start)
    {   
        //apuntamos al inicio del bitmap del inoco del primero mientras aumenta 
        fseek(fp, bm_inode_start + num, SEEK_SET);
        //obtenemos el byte del archivo
        buff = static_cast<char>(fgetc(fp));
        aux++;

        //si se puede escribir en el espacio entonces ejecutamos
        if(buff == '1')
        {
            //apuntamos al inicio del bitmap de inodos
            fseek(fp,inode_start + static_cast<int>(sizeof(INODOTABLA))*num,SEEK_SET);
            fread(&inodo,sizeof(INODOTABLA),1,fp);
            fprintf(graph, "    nodo_%d [ shape=none fontname=\"Century Gothic\" label=<\n",num);
            fprintf(graph, "   <table border=\'0\' cellborder=\'1\' cellspacing=\'0\' bgcolor=\"Orange\">");
            fprintf(graph, "    <tr> <td colspan=\'2\'> <b>Inodo %d</b> </td></tr>\n",num);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_uid </td> <td bgcolor=\"white\"> %d </td>  </tr>\n",inodo.i_uid);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_gid </td> <td bgcolor=\"white\"> %d </td>  </tr>\n",inodo.i_gid);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_size </td> <td bgcolor=\"white\"> %d </td> </tr>\n",inodo.i_size);
            
            //creacion para fecha temporal
            struct tm *tm;
            char fecha[100];
            tm=localtime(&inodo.i_atime);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_atime </td> <td bgcolor=\"white\"> %s </td>  </tr>\n",fecha);
            
            tm=localtime(&inodo.i_ctime);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_ctime </td> <td bgcolor=\"white\"> %s </td>  </tr>\n",fecha);
            
            tm=localtime(&inodo.i_mtime);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_mtime </td> <td bgcolor=\"white\"> %s </td></tr>\n",fecha);
           
           //recorremos los 15 inodos del bloque
            for(int b = 0; b < 15; b++)
                fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_block_%d </td> <td bgcolor=\"white\"> %d </td> </tr>\n",b,inodo.i_block[b]);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_type </td> <td bgcolor=\"white\"> %c </td> </tr>\n",inodo.i_type);
            fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> i_perm </td> <td bgcolor=\"white\"> %d </td> </tr>\n",inodo.i_perm);
            fprintf(graph, "   </table>>]\n");
        }
        num++;
    }
    fprintf(graph,"\n}");
    fclose(graph);

    fclose(fp);

    //ejecutamos en la consola
    string comando = "dot -T"+extension+" grafica.dot -o "+destino;
    system(comando.c_str());
    cout << "\033[31m\n--- Reporte INODO generado con exito ---.\n\033[0m" << endl;
    
}

void REPORTE_JOURNALING(string auxPath, string destino, string extension,int inicio_super){
    //abrimos el disco
    FILE *fp = fopen(auxPath.c_str(),"r");

    SUPERBLOQUE super;
    JOURNAL journal;

    //apuntamos al inicio del super bloque
    fseek(fp, inicio_super, SEEK_SET);
    fread(&super, sizeof(SUPERBLOQUE), 1, fp);

    //abrimos para escribir el .dot
    FILE *graph = fopen("grafica.dot","w");
    fprintf(graph,"digraph G{\n");
    fprintf(graph, "    nodo [shape=none, fontname=\"Century Gothic\" label=<\n");
    fprintf(graph, "   <table border=\'0\' cellborder='1\' cellspacing=\'0\'>\n");
    fprintf(graph, "    <tr> <td COLSPAN=\'7\' bgcolor=\"Orange\"> <b>JOURNALING</b> </td></tr>\n");
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"><b>Operacion</b></td> <td bgcolor=\"lightblue\"><b>Tipo</b></td><td bgcolor=\"lightblue\"><b>Nombre</b></td><td bgcolor=\"lightblue\"><b>Contenido</b></td>\n");
    fprintf(graph, "    <td bgcolor=\"lightblue\"><b>Propietario</b></td><td bgcolor=\"lightblue\"><b>Permisos</b></td><td bgcolor=\"lightblue\"><b>Fecha</b></td></tr>\n");
    
    //aapuntamos al incio del bitmap del super bloque
    fseek(fp,inicio_super + static_cast<int>(sizeof(SUPERBLOQUE)),SEEK_SET);

    //mientras el puntero no avance la posicion del bitmap de nodos 
    while(ftell(fp) < super.s_bm_inode_start)
    {
        //leemos el journal para llenar el reporte
        fread(&journal, sizeof(JOURNAL), 1, fp);
        if(journal.Journal_tipo == 0 || journal.Journal_tipo == 1)
        {
            //que este disponible o ocupado
            struct tm *tm;
            char fecha[100];
            tm = localtime(&journal.Journal_fecha);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph,"<tr><td>%s</td><td>%d</td><td>%s</td><td>%s</td><td>%d</td><td>%d</td><td>%s</td></tr>\n",journal.Journal_Tipo_Operacion,journal.Journal_tipo,journal.Journal_nombre,journal.Journal_contenido,journal.Journal_propietario,journal.Journal_permisos,fecha);
        }
    }
    fprintf(graph, "   </table>>]\n");
    fprintf(graph,"}");
    fclose(graph);

    fclose(fp);
    
    //ejecutamos comando en el terminal
    string comando = "dot -T"+extension+" grafica.dot -o "+destino;
    system(comando.c_str());
    cout << "\033[31m\n---- Reporte JOURNALING generado con exito ----.\n\033[0m" << endl;
}

void REPORTE_BLOQUES(string auxPath, string destino, string extension, int bm_block_start, int block_start, int inode_start){
    //abrimos el disco
    FILE *fp = fopen(auxPath.c_str(),"r");

    BLOQUECARPETAS carpeta;
    BLOQUEARCHIVOS archivo;
    BLOQUEAPUNTADORES apuntador;

    //inicio del bitmap de bloques
    int aux = bm_block_start;
    int i = 0;
    char buff;

    //abrimos el .dot para llenar la grafica
    FILE *graph = fopen("grafica.dot","w");
    fprintf(graph,"digraph G{\n\n");

    //mientras el bitmap de bloques no superela cantidad de inicio de bitmap de inodos
    while(aux < inode_start)
    {
        //apuntamos al bloque bitmap inicio y cada vez se va ir avanzndo un bloque mas
        fseek(fp,bm_block_start + i,SEEK_SET);

        //obtenemos el estado del archivo
        buff = static_cast<char>(fgetc(fp));
        aux++;

        //si el buff es uno uno completamos la tabla con los datos que serian las particiones
        if(buff == '1')
        {   //apuntamos al bitmap de inicio en el espacio de la carepta
            fseek(fp,block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*i,SEEK_SET);
            fread(&carpeta,sizeof(BLOQUECARPETAS),1,fp);
            fprintf(graph, "    nodo_%d [ shape=none, fontname=\"Century Gothic\" label=< \n",i);
            fprintf(graph, "   <table border=\'0\' cellborder='1' cellspacing='0' bgcolor=\"seagreen\">");
            fprintf(graph, "    <tr> <td colspan=\'2\'> <b>Bloque Carpeta %d</b> </td></tr>\n",i);
            fprintf(graph, "    <tr> <td bgcolor=\"mediumseagreen\"> b_name </td> <td bgcolor=\"mediumseagreen\"> b_inode </td></tr>\n");
            
            //recorremos las 4 particiones
            for(int c = 0;c < 4;c++)
                fprintf(graph, "    <tr> <td bgcolor=\"white\"> %s </td> <td bgcolor=\"white\"> %d </td></tr>\n",carpeta.b_content[c].b_name,carpeta.b_content[c].b_inodo);
            fprintf(graph, "   </table>>]\n\n");

        }else if(buff == '2')
        {   
            //apuntamos al incio del bitmp, hasta el archivo creado
            fseek(fp,block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*i,SEEK_SET);
            fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
            fprintf(graph, "    nodo_%d [ shape=none, label=< \n",i);
            fprintf(graph, "   <table border=\'0\' cellborder='1' cellspacing='0' bgcolor=\"sandybrown\">");
            fprintf(graph, "    <tr> <td colspan=\'2\'> <b>Bloque Archivo %d </b></td></tr>\n",i);
            fprintf(graph, "    <tr> <td colspan=\'2\' bgcolor=\"white\"> %s </td></tr>\n",archivo.b_content);
            fprintf(graph, "   </table>>]\n\n");
        }else if(buff == '3')
        {
            //apuntamos a los 16 bloques y leeemos
            fseek(fp, block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*i,SEEK_SET);
            fread(&apuntador,sizeof(BLOQUEAPUNTADORES),1,fp);


            fseek(fp, block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*i, SEEK_SET);
            fread(&apuntador,sizeof(BLOQUEAPUNTADORES), 1, fp);

            //llrnsmod los datos segun lo apuntado
            fprintf(graph, "    bloque_%d [shape=plaintext fontname=\"Century Gothic\" label=< \n",i);
            fprintf(graph, "   <table border=\'0\' bgcolor=\"khaki\">\n");
            fprintf(graph, "    <tr> <td> <b>Pointer block %d</b></td></tr>\n",i);
            
            //recorremos los 16 bloques
            for(int j = 0; j < 16; j++)
                fprintf(graph, "    <tr> <td bgcolor=\"white\">%d</td> </tr>\n",apuntador.b_pointers[j]);
            fprintf(graph, "   </table>>]\n\n");
        }
        i++;
    }

    fprintf(graph,"\n}");
    fclose(graph);

    fclose(fp);

    //ejecutamos la grafica en la terminal
    string comando = "dot -T"+extension+ " grafica.dot -o "+destino;
    system(comando.c_str());
    cout << "\033[94m\n---- Reporte de BLOQUES generado con exito ----.\n\033[0m" << endl;
}

void REPORTE_BM_INODE(string direccion, string destino, int start_bm, int num_inodo){
    //abrimos el disco a reportar
    FILE *fp = fopen(direccion.c_str(),"rb+");

    char byte;
    //abrimos el archivo de destino, tiene que ser un txt
    FILE *report = fopen(destino.c_str(),"w+");
    fseek(report,0,SEEK_SET);
    int cont = 0;

    for (int i = 0; i < num_inodo; i++) 
    {
        //apuntamos al inicio del bitmap de inicio
        fseek(fp, start_bm + i,SEEK_SET);
        //obtenemos el byte
        byte = static_cast<char>(fgetc(fp));

        //si no esta ocupado obtenemos el 0 en el reporte
        if(byte == '0')
            fprintf(report,"0 ");
        else
        //si esta ocupado ese espacio se llena con 1
            fprintf(report,"1 ");
        //si el contador encuentra 20 en la cantidad de inodos
        if(cont == 19){
            //reiniciamos a 0 el contador
            cont = 0;
            fprintf(report, "\n");
        }else
            cont++;
    }
    fclose(report);

    fclose(fp);
    cout << "\033[31m\n---- Reporte BM generado con exito ----\n.\033[0m" << endl;
}

void REPORTE_TREE(string auxPath, string ubicacion, string extension, int start_super){
    //abrimos el disco que se realizara el reporte
    FILE *fp = fopen(auxPath.c_str(),"r");

    //inicialisamos estructuras
    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUECARPETAS carpeta;
    BLOQUEARCHIVOS archivo;
    BLOQUEAPUNTADORES apuntador;

    //apuntamos al inicio del super bloque
    fseek(fp, start_super, SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);

    //incio del bitmap de inodos
    int aux_S_istart = super.s_bm_inode_start;
    int i = 0;

    char buff;

    //abrimos el archivo dot que vamos a escribir
    FILE *graph = fopen("grafica.dot", "w");
    fprintf(graph, "digraph G{\n\n");
    fprintf(graph, "    rankdir=\"LR\" \n");

    //Creamos los inodos - mientras el aux del super no supere a los bitmas del bloque
    while(aux_S_istart < super.s_bm_block_start)
    {
        //apuntamos al bitmap de inodos del super bloque
        fseek(fp,super.s_bm_inode_start + i,SEEK_SET);

        buff = static_cast<char>(fgetc(fp));
        aux_S_istart++;
        int port = 0;

        //si lee el valor 1 segut el archivo se extraeron datos
        if(buff == '1')
        {
            //apuntamos al super bloque de inodos segun cada bitmap de la tabla inodos
            fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*i,SEEK_SET);
            fread(&inodo,sizeof(INODOTABLA),1,fp);

            fprintf(graph, "    inodo_%d [ shape=plaintext fontname=\"Century Gothic\" label=<\n",i);
            fprintf(graph, "   <table bgcolor=\"royalblue\" border=\'0\' >");
            fprintf(graph, "    <tr> <td colspan=\'2\'><b>Inode %d</b></td></tr>\n",i);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_uid </td> <td bgcolor=\"white\"> %d </td>  </tr>\n",inodo.i_uid);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_gid </td> <td bgcolor=\"white\"> %d </td>  </tr>\n",inodo.i_gid);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_size </td><td bgcolor=\"white\"> %d </td> </tr>\n",inodo.i_size);
            
            //estructura de tiempo temporal
            struct tm *tm;
            char fecha[100];
            tm=localtime(&inodo.i_atime);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_atime </td> <td bgcolor=\"white\"> %s </td> </tr>\n",fecha);
            tm=localtime(&inodo.i_ctime);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_ctime </td> <td bgcolor=\"white\"> %s </td> </tr>\n",fecha);
            tm=localtime(&inodo.i_mtime);
            strftime(fecha,100,"%d/%m/%y %H:%S",tm);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_mtime </td> <td bgcolor=\"white\"> %s </td> </tr>\n",fecha);
            
            //recorremos los 15 bloques
            for(int b = 0; b < 15; b++){
                fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_block_%d </td> <td bgcolor=\"white\" port=\"f%d\"> %d </td></tr>\n",port,b,inodo.i_block[b]);
                port++;
            }
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_type </td> <td bgcolor=\"white\"> %c </td>  </tr>\n",inodo.i_type);
            fprintf(graph, "    <tr> <td bgcolor=\"lightsteelblue\"> i_perm </td> <td bgcolor=\"white\"> %d </td>  </tr>\n",inodo.i_perm);
            fprintf(graph, "   </table>>]\n\n");

            //Creamos los bloques relacionados al inodo
            for (int j = 0; j < 15; j++) 
            {
                port = 0;
                if(inodo.i_block[j] != -1)
                {
                    //apuntamos al inicio del bitmap del superbloque de cada inodo
                    fseek(fp,super.s_bm_block_start + inodo.i_block[j],SEEK_SET);

                    //valor extraido de la lecutra
                    buff = static_cast<char>(fgetc(fp));
                    if(buff == '1')
                    {//Bloque carpeta
                        fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*inodo.i_block[j],SEEK_SET);
                        fread(&carpeta,sizeof(BLOQUECARPETAS),1,fp);
                        fprintf(graph, "    bloque_%d [shape=plaintext fontname=\"Century Gothic\" label=< \n",inodo.i_block[j]);
                        fprintf(graph, "   <table bgcolor=\"seagreen\" border=\'0\'>\n");
                        fprintf(graph, "    <tr> <td colspan=\'2\'><b>Folder block %d</b></td></tr>\n",inodo.i_block[j]);
                        fprintf(graph, "    <tr> <td bgcolor=\"mediumseagreen\"> b_name </td> <td bgcolor=\"mediumseagreen\"> b_inode </td></tr>\n");
                        
                        //recorremos las 4 content de los bloques
                        for(int c = 0;c < 4; c++)
                        {
                            fprintf(graph, "    <tr> <td bgcolor=\"white\" > %s </td> <td bgcolor=\"white\"  port=\"f%d\"> %d </td></tr>\n",carpeta.b_content[c].b_name,port,carpeta.b_content[c].b_inodo);
                            port++;
                        }
                        fprintf(graph, "   </table>>]\n\n");

                        //Relacion de bloques a inodos
                        for(int c = 0; c < 4; c++)
                        {
                            //buscamos el tipo de bloque de carpetas
                            if(carpeta.b_content[c].b_inodo !=-1)
                            {
                                if(strcmp(carpeta.b_content[c].b_name,".")!=0 && strcmp(carpeta.b_content[c].b_name,"..")!=0)
                                    fprintf(graph, "    bloque_%d:f%d -> inodo_%d;\n",inodo.i_block[j],c,carpeta.b_content[c].b_inodo);
                            }
                        }
                    }else if(buff == '2')
                    {//Bloque archivo
                        //apuntamos al inodo de cada bloque de archivos
                        fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*inodo.i_block[j],SEEK_SET);
                        fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

                        //llenamos la info con los inodos del bloque
                        fprintf(graph, "    bloque_%d [shape=plaintext fontname=\"Century Gothic\" label=< \n",inodo.i_block[j]);
                        fprintf(graph, "   <table border=\'0\' bgcolor=\"sandybrown\">\n");
                        fprintf(graph, "    <tr> <td> <b>File block %d</b></td></tr>\n",inodo.i_block[j]);
                        fprintf(graph, "    <tr> <td bgcolor=\"white\"> %s </td></tr>\n",archivo.b_content);
                        fprintf(graph, "   </table>>]\n\n");

                    }else if(buff == '3')
                    {//Bloque apuntador
                        //apuntador del super bloque en cada apuntador
                        fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[j],SEEK_SET);
                        fread(&apuntador,sizeof(BLOQUEAPUNTADORES),1,fp);

                        fprintf(graph, "    bloque_%d [shape=plaintext fontname=\"Century Gothic\" label=< \n",inodo.i_block[j]);
                        fprintf(graph, "   <table border=\'0\' bgcolor=\"khaki\">\n");
                        fprintf(graph, "    <tr> <td> <b>Pointer block %d</b></td></tr>\n",inodo.i_block[j]);
                        
                        //recorremos los apuntadores
                        for(int a = 0; a < 16; a++){
                            fprintf(graph, "    <tr> <td bgcolor=\"white\" port=\"f%d\">%d</td> </tr>\n",port,apuntador.b_pointers[a]);
                            port++;
                        }
                        fprintf(graph, "   </table>>]\n\n");

                        //Bloques carpeta/archivos del bloque de apuntadores
                        for (int x = 0; x < 16; x++) 
                        {
                            port = 0;
                            //apuntadores
                            if(apuntador.b_pointers[x] != -1)
                            {
                                //apuntamos al bimtap de los apuntadores
                                fseek(fp,super.s_bm_block_start + apuntador.b_pointers[x],SEEK_SET);
                                buff = static_cast<char>(fgetc(fp));

                                if(buff == '1')
                                {
                                    //apuntamos al inicio de bloque de los apuntadores  
                                    fseek(fp, super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*apuntador.b_pointers[x],SEEK_SET);
                                    fread(&carpeta,sizeof(BLOQUECARPETAS),1,fp);

                                    fprintf(graph, "    bloque_%d [shape=plaintext fontname=\"Century Gothic\" label=< \n",apuntador.b_pointers[x]);
                                    fprintf(graph, "   <table border=\'0\' bgcolor=\"seagreen\" >\n");
                                    fprintf(graph, "    <tr> <td colspan=\'2\'> <b>Folder block %d</b> </td></tr>\n",apuntador.b_pointers[x]);
                                    fprintf(graph, "    <tr> <td bgcolor=\"mediumseagreen\"> b_name </td> <td bgcolor=\"mediumseagreen\"> b_inode </td></tr>\n");
                                    
                                    //contenidos de las carpetas
                                    for(int c = 0;c < 4; c++)
                                    {
                                        fprintf(graph, "    <tr> <td bgcolor=\"white\"> %s </td> <td bgcolor=\"white\" port=\"f%d\"> %d </td></tr>\n",carpeta.b_content[c].b_name,port,carpeta.b_content[c].b_inodo);
                                        port++;
                                    }
                                    fprintf(graph, "   </table>>]\n\n");

                                    //Relacion de bloques a inodos
                                    for(int c = 0; c < 4; c++)
                                    {
                                        //entramos al contenido de cada carpeta
                                        if(carpeta.b_content[c].b_inodo !=-1)
                                        {
                                            if(strcmp(carpeta.b_content[c].b_name,".")!=0 && strcmp(carpeta.b_content[c].b_name,"..")!=0)
                                                fprintf(graph, "    bloque_%d:f%d -> inodo_%d;\n",apuntador.b_pointers[x],c,carpeta.b_content[c].b_inodo);
                                        }
                                    }

                                //si entra al bloque de archivos
                                }else if(buff == '2')
                                {
                                    //apuntamos al bloque de archivos
                                    fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUEARCHIVOS))*apuntador.b_pointers[x],SEEK_SET);
                                    fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);

                                    fprintf(graph, "    bloque_%d [shape=plaintext fontname=\"Century Gothic\" label=< \n",apuntador.b_pointers[x]);
                                    fprintf(graph, "   <table border=\'0\' bgcolor=\"sandybrown\">\n");
                                    fprintf(graph, "    <tr> <td> <b>File block %d</b></td></tr>\n",apuntador.b_pointers[x]);
                                    fprintf(graph, "    <tr> <td bgcolor=\"white\"> %s </td></tr>\n",archivo.b_content);
                                    fprintf(graph, "   </table>>]\n\n");
                                }else if(buff == '3'){
                                    
                                }
                            }
                        }

                        //Relacion de bloques apuntador a bloques archivos/carpetas
                        for(int b = 0; b < 16; b++)
                        {
                            if(apuntador.b_pointers[b] != -1)
                                fprintf(graph, "    bloque_%d:f%d -> bloque_%d;\n",inodo.i_block[j],b,apuntador.b_pointers[b]);
                        }
                    }
                    //Relacion de inodos a bloques
                    fprintf(graph, "    inodo_%d:f%d -> bloque_%d; \n",i,j,inodo.i_block[j]);
                }
            }
        }
        i++;
    }

    fprintf(graph,"\n\n}");
    fclose(graph);

    fclose(fp);

    string comando = "dot -T"+extension+" grafica.dot -o "+ubicacion;
    system(comando.c_str());
    cout << "Reporte Tree generado con exito " << endl;
}

void comando_MKDIR(string auxPath, bool auxP){
    
    //almacenamiento variables
    string valPath = auxPath;
    bool valP = auxP;

    string name = SplitFilename(valPath);
    if(name.length() <= 11)
    {
        if(flag_login)
        {
            int result = Carpeta_crear(valPath,valP);

            if(result == 0)
                cout << "\033[31mERROR: La carpeta ya existe.\n\033[0m" << endl;
            else if(result == 1)
            {//si no existe
                if(actualSesion.tipo_sistema == 3)
                {
                    char aux[500];
                    char operacion[8];
                    char content[5];

                    //completamos los valores de las variables
                    strcpy(aux,valPath.c_str());
                    strcpy(operacion, "mkdir");
                    strcpy(content, "null");

                    //mandamos los valores al journal
                    guardarJournal(operacion,1,664,aux,content);
                }
                cout << "\033[94m \n----  Carpeta creada con exito  ------\n\033[0m" << endl;
            }else if(result == 2)
                cout << "\033[31mERROR, No tienen permisos de escritura.\n\033[0m" << endl;
            else if(result == 3)
            {
                cout << "\033[31mERROR, No existe el directorio y no esta el parametro -p.\n\033[0m" << endl;
            }
        }else
            cout << "\033[31mERROR, necesita iniciar sesion para poder ejecutar este comando.\n\033[0m" << endl;
    }else
        cout << "\033[31mERROR, el nombre de la carpeta es mas grande de lo esperado.\n\033[0m" << endl;
}

void REPORTE_SUPERBLOQUE(string direccion, string destino, string extension, int start_super){
    
    //abrimos el disco
    FILE* fp = fopen(direccion.c_str(),"r");

    SUPERBLOQUE super;

    //apuntamos al inicio del super bloque
    fseek(fp,start_super,SEEK_SET);
    fread(&super,sizeof (super),1,fp);

    //abrimos el archivo de lectura
    FILE *graph = fopen("grafica.dot","w");
    fprintf(graph,"digraph G{\n");
    fprintf(graph, "    nodo [shape=none, fontname=\"Century Gothic\" label=<");
    fprintf(graph, "   <table border=\'1\' cellborder='1\' cellspacing=\'0\' bgcolor=\"Orange\">");
    fprintf(graph, "    <tr> <td COLSPAN=\'2\'> <b>SUPERBLOQUE</b> </td></tr>\n");
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_inodes_count </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_inodes_count);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_blocks_count </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_blocks_count);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_free_block_count </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_free_blocks_count);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_free_inodes_count </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_free_inodes_count);
    
    //esctructura de tiempo
    struct tm *tm;
    char fecha[100];
    tm=localtime(&super.s_mtime);
    strftime(fecha,100,"%d/%m/%y %H:%S",tm);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_mtime </td> <td bgcolor=\"white\"> %s </td></tr>\n",fecha);
    tm=localtime(&super.s_umtime);
    strftime(fecha,100,"%d/%m/%y %H:%S",tm);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_umtime </td> <td bgcolor=\"white\"> %s </td> </tr>\n",fecha);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_mnt_count </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_mnt_count);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_magic </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_magic);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_inode_size </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_inode_size);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_block_size </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_block_size);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_first_ino </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_first_ino);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_first_blo </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_first_blo);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_bm_inode_start </td> <td bgcolor=\"white\"> %d </td></tr>\n",super.s_bm_inode_start);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_bm_block_start </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_bm_block_start);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_inode_start </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_inode_start);
    fprintf(graph, "    <tr> <td bgcolor=\"lightblue\"> s_block_start </td> <td bgcolor=\"white\"> %d </td> </tr>\n",super.s_block_start);
    fprintf(graph, "   </table>>]\n");
    fprintf(graph,"\n}");
    fclose(graph);

    fclose(fp);

    string comando = "dot -T"+extension +" grafica.dot -o "+destino;
    system(comando.c_str());
    cout << "\033[94m ----- Reporte SUPERBLOQUE generado con exito de exito -----\n\033[0m" << endl;
}

void REPORTE_FILE(string direccion, string destino, string extension, string nombreRuta, int start_super,int n){
    FILE *fp = fopen(direccion.c_str(),"r");

    SUPERBLOQUE super;
    INODOTABLA inodo;
    BLOQUEARCHIVOS archivo;

    //apuntador al inicio del super bloque
    fseek(fp,start_super,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);

    //apuntador en el incio de la tabla de inodos
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*n,SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    //abrimos el archivo dot para escribirlo
    FILE *graph = fopen("grafica.dot","w");
    fprintf(graph,"digraph G{\n");
    fprintf(graph, "    nodo [shape=none, fontname=\"Century Gothic\" label=<");
    fprintf(graph, "   <table border=\'1\' cellborder='1\' cellspacing=\'0\' bgcolor=\"Orange\">");
    fprintf(graph, "    <tr><td align=\"center\"> <b>%s</b> </td></tr>\n",nombreRuta.c_str());
    fprintf(graph, "    <tr><td bgcolor=\"lightblue\">");
    
    //recorremos los 15 bloques
    for (int i = 0; i < 15; i++) 
    {
        if(inodo.i_block[i] != -1)
        {
            if(i == 12)
            {//Apuntador indirecto simple
                BLOQUEAPUNTADORES apuntador;
                fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUEAPUNTADORES))*inodo.i_block[i],SEEK_SET);
                fread(&apuntador,sizeof(BLOQUEAPUNTADORES),1,fp);

                //recorremos el bloque de carpetas
                for(int j = 0; j < 16; j++)
                {
                    //recorremos los contenedores que estan disponibles
                    if(apuntador.b_pointers[j] != -1){
                        //apuntador inodos de las carpetas apuntadas
                        fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*apuntador.b_pointers[j],SEEK_SET);
                        fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
                        fprintf(graph,"%s <br/>",archivo.b_content);
                    }
                }
            }else if(i == 13){

            }else if(i == 14){

            }else{//Apuntadores directos
                fseek(fp,super.s_block_start + static_cast<int>(sizeof(BLOQUECARPETAS))*inodo.i_block[i],SEEK_SET);
                fread(&archivo,sizeof(BLOQUEARCHIVOS),1,fp);
                fprintf(graph,"%s <br/>",archivo.b_content);
            }
        }
    }
    fprintf(graph, "    </td></tr>\n");
    fprintf(graph, "   </table>>]\n");
    fprintf(graph,"\n}");
    fclose(graph);

    fclose(fp);

    string comando = "dot -T"+extension+" grafica.dot -o "+destino;
    system(comando.c_str());
    cout << "Reporte file generado con exito " << endl;
}

void REPORTE_LS(string direccion, string destino, string extension, int start_super, int n, USUARIO user, string name){
    FILE *fp = fopen(direccion.c_str(),"rb+");

    SUPERBLOQUE super;
    INODOTABLA inodo;

    //apuntamos al inicio del superbloque
    fseek(fp,start_super,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);

    //apuntador del bitmap de inodos
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*n,SEEK_SET);
    fread(&inodo,sizeof(INODOTABLA),1,fp);

    FILE *graph = fopen("grafica.dot","w");
    fprintf(graph,"digraph G{\n\n");
    fprintf(graph, "    nodo [ shape=none, fontname=\"Century Gothic\" \n");
    fprintf(graph, "    label=< <table border=\'0\' cellborder='1\' cellspacing=\'0\' bgcolor=\"Orange\">\n");
    fprintf(graph, "     <tr> <td><b>Permisos</b></td><td><b>Owner</b></td><td><b>Grupo</b></td><td><b>Size</b></td><td><b>Fecha</b></td><td><b>Hora</b></td><td><b>Tipo</b></td><td><b>Name</b></td> </tr>\n");

    //permisos del i nodo de permisos
    string auxPermisos = to_string(inodo.i_perm);
    char propietario = auxPermisos[0];
    char grupo = auxPermisos[1];
    char otros = auxPermisos[2];
    char permisos[50];

    //Tipo de permisos para el propietario
    if(propietario == '0')
        strcpy(permisos,"---");
    else if(propietario == '1')
        strcpy(permisos,"--x");
    else if(propietario == '2')
        strcpy(permisos,"-w-");
    else if(propietario == '3')
        strcpy(permisos,"-wx");
    else if(propietario == '4')
        strcpy(permisos,"r--");
    else if(propietario == '5')
        strcpy(permisos,"r-x");
    else if(propietario == '6')
        strcpy(permisos,"rw-");
    else if(propietario == '7')
        strcpy(permisos,"rwx");

    //Tipo de permisos para grupo
    if(grupo == '0')
        strcat(permisos," ---");
    else if(grupo == '1')
        strcat(permisos," --x");
    else if(grupo == '2')
        strcat(permisos," -w-");
    else if(grupo == '3')
        strcat(permisos," -wx");
    else if(grupo == '4')
        strcat(permisos," r--");
    else if(grupo == '5')
        strcat(permisos," r-x");
    else if(grupo == '6')
        strcat(permisos," rw-");
    else if(grupo == '7')
        strcat(permisos," rwx");

    //Tipo de permisos para otros
    if(otros == '0')
        strcat(permisos," ---");
    else if(otros == '1')
        strcat(permisos," --x");
    else if(otros == '2')
        strcat(permisos," -w-");
    else if(otros == '3')
        strcat(permisos," -wx");
    else if(otros == '4')
        strcat(permisos," r--");
    else if(otros == '5')
        strcat(permisos," r-x");
    else if(otros == '6')
        strcat(permisos," rw-");
    else if(otros == '7')
        strcat(permisos," rwx");

    //llenamos
    fprintf(graph,"<tr> <td bgcolor=\"white\">%s</td> ",permisos);
    fprintf(graph, "<td bgcolor=\"white\">%s</td>",user.username);
    fprintf(graph, "<td bgcolor=\"white\">%s</td>",user.group);
    fprintf(graph, "<td bgcolor=\"white\">%d</td>",inodo.i_size);

    struct tm *tm;
    char fecha[100];
    tm=localtime(&inodo.i_atime);
    strftime(fecha,100,"%d/%m/%y",tm);
    fprintf(graph, "<td bgcolor=\"white\">%s</td>",fecha);
    strftime(fecha,100,"%H:%S",tm);
    fprintf(graph,"<td bgcolor=\"white\">%s</td>",fecha);
    if(inodo.i_type == '0')
        fprintf(graph,"<td bgcolor=\"white\">%s</td>","Carpeta");
    else if(inodo.i_type == '1')
        fprintf(graph,"<td bgcolor=\"white\">%s</td>","Archivo");
    fprintf(graph, "<td bgcolor=\"white\">%s</td> </tr>\n",name.c_str());

    fprintf(graph, "    </table>>]\n");
    fprintf(graph,"\n}");
    fclose(graph);

    fclose(fp);

    string comando = "dot -T"+extension+" grafica.dot -o "+destino;
    system(comando.c_str());
    cout << "Reporte ls generado con exito " << endl;
}

// -- Ejecucion comando MKFILE --
void comando_MKFILE(string ruta, int size, bool P, string cont){

    bool flagP = P;

    string valPath;
    
    //comillas traera el path
    if (ruta[0] == '\"')
    {
        char replacement[] = "";
        valPath = ruta.replace(0, 1, replacement);
        valPath = ruta.replace(ruta.size()-1, 1, replacement);
    }else{
        valPath = ruta;
    }

    string valCont = cont;
    int valSize = size;

    //nombre de la ruta
    string name = SplitFilename(valPath);

    if(name.length() <= 11)
    {
        if(flag_login)
        {
            int result = Archivo_crear(valPath, flagP, valSize, valCont);
            if(result == 1)
            {
                if(actualSesion.tipo_sistema == 3)
                {
                    char aux[500];
                    char operacion[8];
                    char content[500];

                    //asignamos los nombres
                    strcpy(aux, valPath.c_str());
                    strcpy(operacion, "mkfile");
                    strcpy(content, valCont.c_str());

                    //si no esta disponible se reescribe
                    if(valCont.length() != 0)
                        guardarJournal(operacion, 0, 664, aux, content);
                    else{
                        //si no existe solo se creay se llena
                        strcpy(content, to_string(valSize).c_str());
                        guardarJournal(operacion, 0, 664, aux, content);
                    }
                }
                cout << "\033[94m ----- Archivo creado con exito -----\n\033[0m" << endl;
            }else if(result == 2)
                cout << "\033[31mERROR, el usuario actual no tiene permisos de escritura.\n\033[0m" << endl;
            else if(result == 3)
                cout << "\033[31mERROR, el archivo retenido no existe.\n\033[0m" << endl;
            else if(result == 4)
                cout << "\033[31mERROR, no existe la ruta y no se tiene el parametro -p.\n\033[0m" << endl;
        }else
            cout << "\033[31mERROR, necesita iniciar sesion para poder ejecutar este comando.\n\033[0m" << endl;
    }else
        cout << "\033[31mERROR, el nombre del archivo es mas grande de lo esperado.\n\033[0m" << endl;
}

// -- Ejeccuion comando CHmod --
void comando_CHMOD(string auxPath, string UGO, bool auxR){
    
    //bandera del R
    bool flagR = auxR;
    //Variables para obtener los valores de los parametros(nodos)
    string valPath = auxPath;
    string valUgo = UGO;

    
    if(flag_login)
    {   
        int propietario = stoi(to_string(valUgo.at(0))) - '0';//Obtener el valor numero de un char
        int grupo = stoi(to_string(valUgo.at(1))) - '0';
        int otros = stoi(to_string(valUgo.at(2))) - '0';

        if((propietario >= 0 && propietario <= 7) && (grupo >= 0 && grupo <= 7) && (otros >= 0 && otros <= 7))
        {
            char auxPath[500];
            strcpy(auxPath, valPath.c_str());

            FILE *fp = fopen(actualSesion.direccion.c_str(),"rb+");

            SUPERBLOQUE super;
            INODOTABLA inodo;

            int existe = buscar_CarpetaAr(fp, auxPath);

            //apuntamos al inicio de sesion del archivo en el inico del super bloque
            fseek(fp, actualSesion.inicioSuper, SEEK_SET);
            fread(&super, sizeof(SUPERBLOQUE), 1, fp);
            //nos ubicammos y leemos el user.txt
            fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*existe, SEEK_SET);
            fread(&inodo, sizeof(INODOTABLA), 1, fp);

            if(existe != -1)
            {
                if((actualSesion.id_user ==1 && actualSesion.id_grp == 1) || actualSesion.id_user == inodo.i_uid)
                {
                    if(flagR)
                        //si viene comando -R cambia permiso de formas recursiva
                        permisos_todos(fp, existe, stoi(valUgo));
                    else{
                        //damos el inodo al permiso
                        inodo.i_perm = stoi(valUgo);
                        //nos colocamos en el inicio de la carpeta 
                        fseek(fp, super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*existe, SEEK_SET);
                        fwrite(&inodo,sizeof(INODOTABLA),1,fp);
                    }
                    cout << "\033[94mPermisos cambiados exitosamente\n.\033[0m" << endl;
                }else
                    cout << "\033[31mERROR: Para cambiar los permisos debe ser el usuario root o ser dueno de la carpeta/archivo.\n \033[0m" << endl;
            }else
                cout << "\033[31mERROR: La ruta no existe\n.\033[0m" << endl;
            fclose(fp);
        }else
            cout << "\033[31mERROR: alguno de los digitos se sale del rango predeterminado\n.\033[0m"<< endl;
    }else
        cout << "\033[31mERROR: Se necesita iniciar sesion para poder ejecutar este comando\n.\033[0m" << endl;
            
}

// -- Ejecucion del comando login
void comando_login(string usr, string password, string id){

    bool flagId = false;
    bool flagMkfs = false;

    flagMkfs = buscar_IdMkfs(id);
    flagId = busca_IDmount(id);

    if (flagMkfs)
    {
        if(!flag_login)
        {
            if(flagId)
            {
                int indiceID = index_IDmount(id);
                int res = log_entra(Arreglomount[indiceID].direccion, Arreglomount[indiceID].nombre, usr, password);
                if(res == 1)
                {
                    flag_login = true;
                    cout << "\033[94mSesion iniciada con exito rotundo \033[0m" << endl;
                }else if(res == 2)
                    cout << "\033[31mERROR, contraseña incorrecta\033[0m" << endl;
                else if(res == 0)
                    cout << "\033[usuario no econtrado\033[0m" << endl;
            }else
                cout << "\033[31mERROR, no se encuentra ninguna particion montada con ese id "<< id <<"\033[0m" << endl;
        }else{
            cout << "\033[31mERROR, sesion activa, cierre sesion para poder volver a iniciar sesion\033[0m" << endl;
        }
    }else{
       cout << "\033[31mERROR, Particion Montada no a sido formateada para el archivo users.txt\033[0m" << endl; 
    }
}

// --  Ejecucion del comando RMUSR --
void comando_RMUSR(string usr){

    if(flag_login)
    {
        if(actualSesion.id_user == 1 && actualSesion.id_grp == 1)
        {//USUARIO root
            if(buscarUsuario(usr)){
                eliminarUsuario(usr);
            }else
                cout << "ERROR el usuario no existe" << endl;
        }else
           cout << "ERROR solo el usuario root puede ejecutar este comando" << endl;
    }else
        cout << "ERROR necesita iniciar sesion para poder ejecutar este comando" << endl;
}

// --  Ejecucion del comando MKUSR --
void comando_MKUSR(string auxUsr, string auxPass, string auxGrupo){
    
    bool flagUser = false;
    bool flagPassword = false;
    bool flagGroup = false;
    bool flag = false;
    
    
    string user = auxUsr;
    string pass = auxPass;
    string group = auxGrupo;

    if(user.length() <= 10)
    {
        if(pass.length() <= 10)
        {
            if(group.length() <= 10)
            {
                if(flag_login)
                {
                    if(actualSesion.id_user == 1 && actualSesion.id_grp == 1)
                    {//USUARIO root
                        if(buscarGrupo(group) != -1)
                        {
                            if(!buscarUsuario(user))
                            {
                                int id = buscarId_usr();
                                string datos = to_string(id) + ",U,"+group+","+user+","+pass+"\n";
                                agregarUsuario_TXT(datos);
                                cout << "USUARIO creado con exito " << endl;
                                //Guardamos el registro en el journal si es un sistema EXT3
                                if(actualSesion.tipo_sistema ==3)
                                {
                                    char aux[64];
                                    char operacion[10];
                                    char content[2];
                                    strcpy(aux,datos.c_str());
                                    strcpy(operacion,"mkusr");
                                    memset(content,0,2);
                                    guardarJournal(operacion,0,0,aux,content);
                                }
                            }else
                                cout << "ERROR el usuario ya existe" <<endl;
                        }else
                            cout << "ERROR no se encuentra el grupo al que pertenecera el usuario " << endl;
                    }else
                        cout << "ERROR solo el usuario root puede ejecutar este comando" << endl;
                }else
                    cout << "ERROR necesita iniciar sesion para poder ejecutar este comando" << endl;
            }else
                cout << "ERROR grupo del usuario excede de los 10 caracteres permitidos" << endl;
        }else
            cout << "ERROR contrasena de usuario excede de los 10 caracteres permitidos" << endl;
    }else
        cout << "ERROR nombre de usuario excede de los 10 caracteres permitidos" << endl;
                
}

// -- ejecucion RMGROUP --
void comando_RMGRP(string name){
    string nombreGrupo = name;
    if(flag_login){
        if(actualSesion.id_user == 1 && actualSesion.id_grp == 1){//USUARIO root
            int grupo = buscarGrupo(nombreGrupo);
            if(grupo != -1){
                eliminarGrupo(nombreGrupo);
            }else
                cout << "\033[31mERROR el grupo no existe" << endl;
        }else
           cout << "\033[31mERROR solo el usuario root puede ejecutar este comando\033[0m" << endl;
    }else
        cout << "\033[31mERROR necesita iniciar sesion para poder ejecutar este comando\033[0m" << endl;
}

// -- ejecucion MKGRP --
void comando_MKGRP(string name){
    string nombreGrupo = name;

    if(flag_login)
    {
        if(actualSesion.id_user == 1 && actualSesion.id_grp == 1)//USUARIO root
        {
            if(nombreGrupo.length() <= 10)
            {
                int grupo = buscarGrupo(nombreGrupo);
                if(grupo == -1)
                {
                    int idGrupo = buscarId_grupo();
                    string nuevoGrupo = to_string(idGrupo)+",G,"+nombreGrupo+"\n";
                    agregarUsuario_TXT(nuevoGrupo);
                    cout << "Grupo creado con exito "<< endl;
                    cout << "\033[94m---- Grupo creado con exito ---.\033[0m" << endl;
                    //Guardamos el registro en el journal si es un EXT3
                    if(actualSesion.tipo_sistema ==3)
                    {
                        char aux[64];
                        char operacion[10];
                        char content[2];

                        strcpy(aux,nuevoGrupo.c_str());
                        strcpy(operacion,"mkgrp");
                        memset(content,0,2);
                        guardarJournal(operacion,0  ,0,aux,content);
                    }
                }else
                    cout << "\033[31mERROR ya existe un grupo con ese nombre\033[0m" << endl;
            }else
                cout << "\033[31mERROR el nombre del grupo no puede exceder los 10 caracters\033[0m" << endl;
        }else
            cout << "\033[31mERROR solo el usuario root puede ejecutar este comando\033[0m" << endl;
    }else
        cout << "\033[31mERROR necesita iniciar sesion para poder ejecutar este comando\033[0m" << endl;
}

//-- ejecucion LOGOUT -- 
void logOUT(){
    //si esta en sesion activa
    if(flag_login){

        flag_login = false;

        actualSesion.id_user = -1;
        actualSesion.direccion = "";
        actualSesion.inicioSuper = -1;

        cout << "\033[31m\n--- Sesion finalizada ---.\033[0m" << endl;
    }else
        cout << "\033[94mERROR, no hay sesion acutalmente.\033[0m" << endl;
}

// -- Ejecucion comando MKFS --
void comando_Mkfs(string id, string type, string fs){
    //Este comando trabaja con el mount
    //datos bandera del comando
    bool flagId = false;
    bool flagType = false;
    bool flagFs = false;
    bool flag = false;

    NDMKFS varMkfs;


    flagId = busca_IDmount(id);
    if(flagId)
    {
        int indiceID = index_IDmount(id);
        int index = buscarParticionP_E(Arreglomount[indiceID].direccion, Arreglomount[indiceID].nombre);
        if(index != -1)
        {
            MBR mbr;
            FILE *fp;

            fp = fopen(Arreglomount[indiceID].direccion.c_str(),"rb+");

            fread(&mbr, sizeof(MBR), 1, fp);

            int inicio = mbr.MBR_partition[index].part_start;
            int tamanio = mbr.MBR_partition[index].part_size;

            if(fs == "3fs")
            {
                formatearEXT3(inicio,tamanio, Arreglomount[indiceID].direccion);
                
                varMkfs.id = id;
                varMkfs.tipo = 3;
                varMkfs.unit = type;
                mkfs_init.push_back(varMkfs);

                cout << "\033[94mSe Formateo correctamente con EXT3" << ", tipo:"<< type <<".\033[0m" << endl;
            }else{
                formatearEXT2(inicio, tamanio, Arreglomount[indiceID].direccion);

                varMkfs.id = id;
                varMkfs.tipo = 2;
                varMkfs.unit = type;
                mkfs_init.push_back(varMkfs);

                cout << "\033[94mSe Formateo correctamente con EXT2" << ", tipo:"<< type <<".\033[0m" << endl;
            }
            fclose(fp);
        }else{
            cout << "\033[31mERROR, no se puede formaterar particiones logicas " << id <<".\033[0m" << endl;
            index = buscarParticion_Logica(Arreglomount[indiceID].direccion, Arreglomount[indiceID].nombre);
        }
    }else{
        
        cout << "\033[31mERROR, no se encuentra ninguna particion montada con ese id " << id <<".\033[0m" << endl;
    }
}

//--Existencia vector--
bool existeEnVector(vector<string> v, string busqueda) {
    return find(v.begin(), v.end(), busqueda) != v.end();
}

//--Minusculas--
string minusculas(string palabra){
    string normal = palabra;
    for (int i = 0; i < normal.length(); i++)
    {
        normal[i] = tolower(normal[i]);
    }
    return normal;
}

//--SPLIT--
vector<string> split(string line){
    string temp;
    vector<string> tmp{};
    stringstream input_stringstream(line);

    int contador = 0;
    while (getline(input_stringstream, temp, ' '))
    {
        tmp.push_back(temp);
        contador ++;
    }
    return tmp;
}

// -- SPLIT Ruta -- 
vector<string> splitRuta(string line){
    string temp;
    vector<string> tmp{};
    stringstream input_stringstream(line);

    int contador = 0;
    while (getline(input_stringstream, temp, '/'))
    {
        tmp.push_back(temp);
        contador ++;
    }
    return tmp;
}

// -- SPLIT --
void crearVerificar_rutas(string ruta){
    string tempPath = path;
    vector<string> nuevo;
    nuevo = splitRuta(ruta);
    
    if ( nuevo.size() == 2){
        tempPath;
        if (mkdir(tempPath.c_str(), 0777) == -1)cerr << "Error :  " << strerror(errno) << endl;
            else cout << "Directory created"<<endl;
    }else{
        for(int i=1; i < nuevo.size()-1; i++)
        {
            tempPath += "/" + nuevo[i];
            if (mkdir(tempPath.c_str(), 0777) == -1)cerr << "Error :  " << strerror(errno) << endl;
                else cout << "Directory created"<<endl;
        }
    }
}

vector<string> splitExec(string line){
    string temp;
    vector<string> tmp{};
    stringstream input_stringstream(line);

    int contador = 0;
    while (getline(input_stringstream, temp, '#'))
    {
        tmp.push_back(temp);
        contador ++;
    }
    return tmp;
}

//--SPLIT  PARAMETROS--
vector<string> splitParam(string linea){
    string temp;
    vector<string> tmp{};
    
    stringstream input_stringstream(linea);

    int contador_ = 0;
    int contadorD = 0;
    for (int i = 0; i < linea.size(); i++)
    {
        if (linea[i] == '~')
        {
            contador_++;
        }else if (linea[i] == ':')
        {
            contadorD++;
        }
    }

    if (contadorD == 1)
    {
        if (contador_ == 2)
        {
            while (getline(input_stringstream, temp, '~'))
            {
                tmp.push_back(temp);
            }
            return tmp;
        }else{
            return {"SIN SIMBOLO"};
        }
        
    }else{
        return {"SIN PUNTOS"};
    }
    
}

string SplitCExec (const string& str) {
  unsigned found = str.find_first_of("\"");
  string var = str.substr(found+1);
  unsigned found1 = var.find_last_of("\"");
  
  return var.substr(0, found1);
}

string obtener_Directorio(string direccion){
    string aux = direccion;
    string delimiter = "/";
    size_t pos = 0;
    string res = "";
    while((pos = aux.find(delimiter))!=string::npos){
        res += aux.substr(0,pos)+"/";
        aux.erase(0,pos + delimiter.length());
    }
    return string(res);
}

string obtener_nombreArchivo(string direccion){
    string aux = direccion;
    string delimiter = "/";
    size_t pos = 0;
    string res = "";
    while((pos = aux.find(delimiter))!=string::npos){
        aux.erase(0,pos + delimiter.length());
    }
    delimiter = ".";
    pos = aux.find(delimiter);
    res = aux.substr(0,pos);
    return string(res);
}

//-- Borrar disco (RMdisk) --
int BorrarDisco(string path){
    if(remove(path.c_str()) != 0 )
    {
        perror("Error al borrar archivo!.");
        return 1;
    }
    else{
        puts("El archivo se borro con exito!");
        return 0;
    }
    return 0;
}

//-- MKDISK --
void MKDISK(vector<string> datos){

    for (int i = 1; i < datos.size(); i++)
    {   
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP[0]);
            string datoComan = tipoP[2];
            if (coman == "-size")
            {
                Discvar.size = stoi(datoComan);
                cout << "size: " << tipoP.at(2) << endl;

            }else if (coman == "-fit")
            {
                string tipFit = minusculas(datoComan);
                if (tipFit == "wf")
                {
                    Discvar.fit = "WF";
                }else if (tipFit == "ff")
                {
                    Discvar.fit = "WF";
                }else if (tipFit == "bf")
                {
                    Discvar.fit = "BF";
                }else{
                    cout << "FIT Invalido" << endl;
                    break;
                }
                
                
                cout << "fit: " << tipoP.at(2) << endl;

            }else if (coman == "-unit")
            {
                string pal = minusculas(datoComan);
                if (pal == "k")
                {
                    Discvar.unit = 'k';
                }else if (pal == "m")
                {
                    Discvar.unit = 'm';
                }else{
                    cout<< "error de parametro" << endl;
                }
                
                cout << "unit: " << tipoP.at(2) << endl;

            }else if (coman == "-path")
            {
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    cout << "se encontro " << endl;
                    string uno = SplitCExec(datoComan);
                    Discvar.path = path + uno; 
                    crearVerificar_rutas(uno);

                }else{
                    Discvar.path = path + datoComan; 
                    crearVerificar_rutas(datoComan);
                }

                cout << "path: " << tipoP.at(2) << endl;
            }else{
                cout << "comando Invalido" << endl;
                break;
            }
        }
    }
    crearDisco(Discvar);
}

//-- RMDISK --
void RMDISK(vector<string> datos){

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-path")
            {
                string uno;
                int num = -1;
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    uno = SplitCExec(datoComan);
                    uno = path + uno;
                    num = BorrarDisco(uno);

                    int tamanioMount = Arreglomount.size();
                    vector<NDMOUNT> temp = Arreglomount;

                    if (num == 0)
                    {
                        for (int i = 0; i < tamanioMount; i++)
                        {
                            if (uno == temp[i].direccion)
                            {
                                fdisk_desmontar(temp[i].id, tamanioMount);
                            }
                        }
                    }

                }else{
                    uno = path + datoComan;
                    num = BorrarDisco(uno);
                    int tamanioMount = Arreglomount.size();

                    vector<NDMOUNT> temp = Arreglomount;

                    if (num == 0)
                    {
                        for (int i = 0; i < tamanioMount; i++)
                        {
                            if (uno == temp[i].direccion)
                            {
                                fdisk_desmontar(temp[i].id, tamanioMount);
                            }
                        }
                    }
                }
            }
            
        }
    }
}

//-- FDISK --
void FDISK(vector<string> datos){
    
    string rutaTotal = "";
    char fit = 0;
    int size_part = 0;
    char unit = 0;
    char typeFd = 'P';
    string name = "";


    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);
            string datoComanMinus = minusculas(tipoP.at(2));
            if (coman == "-path")
            {
                rutaTotal = path + datoComan;
            }else if (coman == "-size")
            {
                if (stoi(datoComan) >= 0)
                {
                    size_part = stoi(datoComan);
                }else{
                    cout << "Tamaño invalido" << endl;
                    break;
                }

            }else if (coman == "-unit")
            {   
                if (datoComanMinus == "b")
                {
                    //size
                    unit = 'B';
                }else if (datoComanMinus == "k")
                {
                    //1024 * size
                    unit = 'K';
                }else if (datoComanMinus == "m")
                {
                    //1024 * 1024 * size
                    unit = 'M';
                }else{
                    cout << "parametro invalido en -size \n" << endl;
                    break;
                }
                
            }else if (coman == "-type")
            {
                if (datoComanMinus == "p")
                {
                    typeFd = 'P';
                }else if (datoComanMinus == "e")
                {
                    typeFd = 'E';
                }else if (datoComanMinus == "l")
                {
                    typeFd = 'L';
                }else{
                    cout << "parametro invalido en el -type \n" << endl;
                    break;
                }
            }else if (coman == "-fit")
            {
                string tipFit = minusculas(datoComan);
                if (tipFit == "wf")
                {
                    fit = 'W';
                }else if (tipFit == "ff")
                {
                    fit = 'F';
                }else if (tipFit == "bf")
                {
                    fit = 'B';
                }else{
                    cout << "FIT Invalido" << endl;
                    break;
                }

            }else if (coman == "-delete")
            {
                datoComan = minusculas(datoComan);
                if (datoComan == "fast")
                {
                    continue;
                }else if (datoComan == "full")
                {
                    continue;
                }else{
                    cout << "parametro invalido -delete" << endl;
                    break;
                }
                
            }else if (coman == "-name")
            {
                name = datoComan;
            }else if (coman == "-add")
            {
                continue;
            }else{
                cout << "comando invalido en el FDisk" << endl;
            }
        }
    }
    if (typeFd == 'P')
    {
        CreatePartitionPimari(size_part, unit, rutaTotal, fit, name);
    }else if (typeFd == 'E')
    {
        CreatePartitionExtend(size_part, unit, rutaTotal, fit, name);
    }else
    {
        createPartitionLogica(size_part, unit, rutaTotal, fit, name);
    }
    
}

// -- PAUSE --
void PAUSE(vector<string> datos){
    int pause;
    cout << "\n++++++   PAUSAMOS ++++++ (presione cualquier tecla para continuar)\n" << endl;
    pause = cin.get();
}

// -- MOUNT --
void MOUNT(vector<string> datos){
    string auxName = "";
    string auxPath = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-path")
            {
                auxPath = path + datoComan;
            }else if (coman == "-name")
            {
                auxName = datoComan;
            }else{
                cout << "ERROR, no es comando valido" << endl;
                break;
            }
        }
    }

    if (auxPath != "" && auxName != "")
    {
        Comandomount(auxName, auxPath);
    }else
    {
        cout << "ERROR, no se llenaron los datos" << endl;
    }
}

// -- UNMOUNT --
void UNMOUNT(vector<string> datos){
    string auxId = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-id")
            {
                auxId = datoComan;
            }else{
                cout << "ERROR, no es comando valido" << endl;
                break;
            }
        }
    }

    if (auxId != "")
    {
        comando_unmount(auxId);
    }else
    {
        cout << "ERROR, no se llenaron los datos" << endl;
    }
}

// -- MKFS --
void MKFS(vector<string> datos){
    string auxId = "";
    string auxType = "full";
    string auxFs = "2fs";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);
            string datocomanMinus = minusculas(datoComan);

            if (coman == "-id")
            {
                auxId = datoComan;
            }else if (coman == "-type")
            {   
                if (datocomanMinus == "fast"  || datocomanMinus == "full")
                {
                    auxType = datocomanMinus;
                }else{
                    cout << "\033[94mERROR, parametro type invalido de MKFS.\033[0m" << endl;
                }
            }else if (coman == "-fs")
            {
                auxFs = datocomanMinus;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxId != "")
    {
        comando_Mkfs(auxId, auxType, auxFs);
    }else
    {
        cout << "\033[94mERROR, no se recibio el id en los parametros.\033[0m" << endl;
    }
}

// -- LOGIN --
void LOGIN(vector<string> datos){
    string auxUsr = "";
    string auxPass = "";
    string auxId = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);
            string datocomanMinus = minusculas(datoComan);

            if (coman == "-id")
            {
                auxId = datoComan;
            }else if (coman == "-usr")
            {   
                auxUsr = datocomanMinus;

            }else if (coman == "-pwd")
            {
                auxPass = datocomanMinus;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxId != "" && auxPass != "" && auxUsr != "")
    {
        comando_login(auxUsr, auxPass, auxId);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- LOGOUT --
void LOGOUT(vector<string> datos){
    logOUT();
}

// -- MKGRP --
void MKGRP(vector<string> datos){
    string auxName = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-name")
            {
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    auxName = uno;
                }else{
                    auxName = datoComan;
                }
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxName != "")
    {
        comando_MKGRP(auxName);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- RMGRP --
void RMGRP(vector<string> datos){
    string auxName = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-name")
            {
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    auxName = uno;
                }else{
                    auxName = datoComan;
                }
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxName != "")
    {
        comando_RMGRP(auxName);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- MKUSR --
void MKUSR(vector<string> datos){
    string auxUsr = "";
    string auxPass = "";
    string auxGrupo = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "\033[94mFalta el simbolo ~, omitimos linea\033[0m" << endl;
                break;
            }else{
                cout << "\033[94mFalta el simbolo :, omitimos linea\033[0m" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);
            string datocomanMinus = minusculas(datoComan);

            if (coman == "-grp")
            {
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    auxGrupo = uno;
                }else{
                    auxGrupo = datoComan;
                }
                
            }else if (coman == "-usr")
            {   
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    auxUsr = uno;
                }else{
                    auxUsr = datoComan;
                }

            }else if (coman == "-pwd")
            {
                auxPass = datoComan;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxGrupo != "" && auxPass != "" && auxUsr != "")
    {
        comando_MKUSR(auxUsr, auxPass, auxGrupo);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- RMUSR --
void RMUSR(vector<string> datos){
    string auxUsr = "";

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            if (tipoP.at(0) == "SIN SIMBOLO")
            {
                cout << "Falta el simbolo ~, omitimos linea" << endl;
                break;
            }else{
                cout << "Falta el simbolo :, omitimos linea" << endl;
                break;
            }
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-usr")
            {
                auxUsr = datoComan;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxUsr != "")
    {
        comando_RMUSR(auxUsr);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- CHMOD --
void CHMOD(vector<string> datos){
    string auxPath = "";
    string auxUgo = "";
    bool auxR = false;

    if(datos.size() == 4)
        auxR = true;

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            continue;
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);
            string datocomanMinus = minusculas(datoComan);

            if (coman == "-path")
            {
                auxPath = path + datoComan;
            }else if (coman == "-ugo")
            {   
                auxUgo = datoComan;
            }else if (coman == "sin puntos")
            {
                auxR = true;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxPath != "" && auxUgo != "")
    {
        cout << auxR << auxUgo << auxPath << endl;
        comando_CHMOD(auxPath, auxUgo, auxR);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- MKFILE --
void MKFILE(vector<string> datos){
    string auxPath = "";
    bool auxP = false;
    int auxSize = 0;
    string auxCont = "";


    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            auxP = true;
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-path")
            {
                auxPath = datoComan;
            }else if (coman == "-size")
            {
                auxSize = stoi(datoComan);  
            }else if (coman == "-cont")
            {
                auxCont = datoComan;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxPath != "")
    {
        comando_MKFILE(auxPath, auxSize, auxP, auxCont);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- MKDIR --
void MKDIR(vector<string> datos){
    string auxPath = "";
    bool auxP = false;

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            auxP = true;
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-path")
            {
                auxPath = datoComan;
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxPath != "")
    {
        comando_MKDIR(auxPath, auxP);
    }else
    {
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

// -- REP -- 
void REPORTES(vector<string> datos){
    string auxPath = "";
    string auxName = "";
    string auxId = "";
    string auxRuta_file = "";
    bool auxP = false;

    for (int i = 1; i < datos.size(); i++)
    {
        vector<string> tipoP;
        tipoP = splitParam(datos.at(i));
        if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
        {
            auxP = true;
        }else{
            string coman = minusculas(tipoP.at(0));
            string datoComan = tipoP.at(2);

            if (coman == "-path")
            {
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    auxPath = path + uno; 
                    crearVerificar_rutas(uno);
                }else{
                    auxPath = path + datoComan;
                    crearVerificar_rutas(datoComan);
                }
            }else if (coman == "-name")
            {
                auxName = minusculas(datoComan);
            }else if (coman == "-id")
            {
                auxId = datoComan;
            }else if (coman == "-ruta")
            {
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    auxRuta_file = uno; 
                    crearVerificar_rutas(uno);
                }else{
                    auxRuta_file = datoComan;
                    crearVerificar_rutas(datoComan);
                }
            }else{
                cout << "\033[94mERROR, no es comando valido\033[0m" << endl;
                break;
            }
        }
    }

    if (auxPath != "" && auxId != "" && auxName != "")
    {
        if (auxName == "mbr")
        {
            REPORTE_MBR(auxPath, auxId);
        }else if (auxName == "disk")
        {
            REPORTE_DISK(auxPath, auxId);
        }else if (auxName == "inode")
        {   
            string extension = ObtenerExtension(auxPath);
            string direccion = "";
            string nombre = "";
            //bandera del ID
            bool flagId = false;

            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    direccion = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                int index = buscarParticionP_E(direccion, nombre);
                if(index != -1)
                {//Primaria|Extendida
                    MBR mbr;
                    SUPERBLOQUE super;
                    FILE *fp = fopen(direccion.c_str(),"rb+");

                    fread(&mbr,sizeof(MBR),1,fp);
                    fseek(fp,mbr.MBR_partition[index].part_start,SEEK_SET);
                    fread(&super,sizeof(SUPERBLOQUE),1,fp);
                    fclose(fp);

                    REPORTE_INODO(direccion, auxPath, extension, super.s_bm_inode_start,super.s_inode_start,super.s_bm_block_start);
                }else{//Logica
                    int index = buscarParticion_Logica(direccion, nombre);
                    if(index != -1)
                    {
                        EBR ebr;
                        SUPERBLOQUE super;
                        FILE *fp = fopen(direccion.c_str(),"rb+");

                        //apuntamos a la particion
                        fseek(fp,index,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,fp);
                        fread(&super,sizeof(SUPERBLOQUE),1,fp);
                        fclose(fp);

                        REPORTE_INODO(direccion, auxPath, extension, super.s_bm_inode_start,super.s_inode_start,super.s_bm_block_start);
                    }
                }
            }else{
                cout << "\033[94mError, no se monto la particion\033[0m" << endl;
            }
            
        }else if (auxName == "journaling")
        {
            string extension = ObtenerExtension(auxPath);
            string direccion = "";
            string nombre = "";
            //bandera del ID
            bool flagId = false;

            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    direccion = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }
            if (flagId)
            {
                int index = buscarParticionP_E(direccion, nombre);
                if(index != -1){//Primaria|Extendida
                    MBR mbr;
                    SUPERBLOQUE super;
                    FILE *fp = fopen(direccion.c_str(),"rb+");

                    //apuntamos a la particion
                    fread(&mbr,sizeof(MBR),1,fp);
                    fseek(fp,mbr.MBR_partition[index].part_start,SEEK_SET);
                    fread(&super,sizeof(SUPERBLOQUE),1,fp);
                    fclose(fp);
                    REPORTE_JOURNALING(direccion, auxPath, extension, mbr.MBR_partition[index].part_start);
                }
            }else{
                cout << "\033[94mError, no se monto la particion, no se encuentra id\033[0m" << endl;
            }
        }else if (auxName == "block")
        {
            string extension = ObtenerExtension(auxPath);
            string direccion = "";
            string nombre = "";
            //bandera del ID
            bool flagId = false;

            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    direccion = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                int indicadorParticion = buscarParticionP_E(direccion, nombre);
                if(indicadorParticion != -1)
                {//Primaria|Extendida
                    MBR mbr;
                    SUPERBLOQUE super;
                    FILE *fp = fopen(direccion.c_str(),"rb+");

                    //apuntamos a la particion
                    fread(&mbr, sizeof(MBR), 1, fp);
                    fseek(fp, mbr.MBR_partition[indicadorParticion].part_start, SEEK_SET);
                    fread(&super,sizeof(SUPERBLOQUE),1,fp);
                    fclose(fp);
                    REPORTE_BLOQUES(direccion, auxPath, extension,super.s_bm_block_start,super.s_block_start,super.s_inode_start);
                }else{//Logica
                    int indicadorParticion = buscarParticion_Logica(direccion, nombre);
                    if(indicadorParticion != -1)
                    {
                        EBR ebr;
                        SUPERBLOQUE super;
                        FILE *fp = fopen(direccion.c_str(),"rb+");

                        //apuntamos a la particon
                        fseek(fp,indicadorParticion,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,fp);
                        fread(&super,sizeof(SUPERBLOQUE),1,fp);
                        fclose(fp);
                        REPORTE_BLOQUES(direccion, auxPath, extension,super.s_bm_block_start,super.s_block_start,super.s_inode_start);
                    }
                }
            }else{
                cout << "\033[94mError, no se monto la particion\033[0m" << endl;
            }
            
        }else if (auxName == "bm_inode") //solo recibe .txt
        {
            string extension = ObtenerExtension(auxPath);
            string direccion = "";
            string nombre = "";
            //bandera del ID
            bool flagId = false;

            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    direccion = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                int indicadorParticion = buscarParticionP_E(direccion, nombre);
                if(indicadorParticion != -1)
                {//Primaria|Extendida
                    MBR mbr;
                    SUPERBLOQUE super;
                    FILE *fp = fopen(direccion.c_str(),"rb+");

                    //apuntamos a la direccion
                    fread(&mbr, sizeof(MBR), 1, fp);
                    fseek(fp, mbr.MBR_partition[indicadorParticion].part_start,SEEK_SET);
                    fread(&super,sizeof(SUPERBLOQUE),1,fp);
                    fclose(fp);
                    REPORTE_BM_INODE(direccion, auxPath, super.s_bm_inode_start,super.s_inodes_count);
                }else{//Logica

                    int indicadorParticion = buscarParticion_Logica(direccion, nombre);
                    if(indicadorParticion != -1)
                    {
                        EBR ebr;
                        SUPERBLOQUE super;
                        FILE *fp = fopen(direccion.c_str(),"rb+");

                        //apuntamos a la particion
                        fseek(fp,indicadorParticion,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,fp);
                        fread(&super,sizeof(SUPERBLOQUE),1,fp);
                        fclose(fp);
                        REPORTE_BM_INODE(direccion, auxPath,super.s_bm_inode_start,super.s_inodes_count);
                    }
                }
            }else{
                cout << "\033[91mError, no se monto la particion\033[0m" << endl;
            }
            
        }else if (auxName == "bm_block")
        {
            string extension = ObtenerExtension(auxPath);
            string direccion = "";
            string nombre = "";

            //bandera del ID
            bool flagId = false;

            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    direccion = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                //buscamos particiones
                int indicadorParticion = buscarParticionP_E(direccion, nombre);
                if(indicadorParticion != -1)
                {//Primaria|Extendida
                    MBR mbr;
                    SUPERBLOQUE super;

                    //abrimo la direccion
                    FILE *fp = fopen(direccion.c_str(),"rb+");

                    //apuntador de la particion
                    fread(&mbr, sizeof(MBR), 1, fp);
                    fseek(fp, mbr.MBR_partition[indicadorParticion].part_start, SEEK_SET);
                    fread(&super,sizeof(SUPERBLOQUE),1,fp);
                    fclose(fp);
                    REPORTE_BM_INODE(direccion, auxPath, super.s_bm_block_start,super.s_blocks_count);

                }else{//Logica
                    int indicadorParticion = buscarParticion_Logica(direccion, nombre);

                    if(indicadorParticion != -1)
                    {
                        EBR ebr;
                        SUPERBLOQUE super;
                        FILE *fp = fopen(direccion.c_str(),"rb+");

                        //apuntamos a la particion
                        fseek(fp,indicadorParticion,SEEK_SET);
                        fread(&ebr, sizeof(EBR), 1, fp);
                        fread(&super,sizeof(SUPERBLOQUE),1,fp);
                        fclose(fp);

                        REPORTE_BM_INODE(direccion, auxPath, super.s_bm_block_start,super.s_blocks_count);
                    }
                }
            }else
            {
                cout << "\033[94mError, no se monto la particion\033[0m" << endl;
            }
        }else if (auxName == "tree")
        {
            //obtiene extension
            string extension = ObtenerExtension(auxPath);
            string auxAuxPath = "";
            string nombre = "";

            //bandera del ID
            bool flagId = false;

            //obtenemos datos para ejecutar los siguientes comandos
            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    auxAuxPath = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                //encontramos las particiones
                int indicadorParticion = buscarParticionP_E(auxAuxPath, nombre);
                string destino = obtener_Directorio(auxPath) + obtener_nombreArchivo(auxPath) + ".pdf";
                
                //si no existe la particion
                if(indicadorParticion != -1)
                {
                    //abrimos las rutas
                    MBR mbr;
                    FILE *fp = fopen(auxAuxPath.c_str(),"rb+");

                    //colocamos el puntero en el inicio de las particiones
                    fread(&mbr, sizeof(MBR), 1, fp);
                    fseek(fp, mbr.MBR_partition[indicadorParticion].part_start, SEEK_SET);
                    fclose(fp);

                    REPORTE_TREE(auxAuxPath, auxPath, extension, mbr.MBR_partition[indicadorParticion].part_start);
                }else{
                    int indicadorParticion = buscarParticion_Logica(auxAuxPath, nombre);
                    if(indicadorParticion != -1)
                    {//si esta libre realiza el reporte
                        EBR ebr;
                        FILE *fp = fopen(auxAuxPath.c_str(),"rb+");
                        fseek(fp,indicadorParticion,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,fp);
                        int start = static_cast<int>(ftell(fp));
                        fclose(fp);
                        REPORTE_TREE(auxAuxPath, auxPath, extension,start);
                    }
                }
            }
            
        }else if (auxName == "sb")
        {
            //obtiene extension
            string extension = ObtenerExtension(auxPath);
            string auxAuxPath = "";
            string nombre = "";

            //bandera del ID
            bool flagId = false;

            //obtenemos datos para ejecutar los siguientes comandos
            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    auxAuxPath = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                int indicadorParticion = buscarParticionP_E(auxAuxPath, nombre);
                if(indicadorParticion != -1){//Primaria|Extendida
                    MBR mbr;
                    FILE *fp = fopen(auxAuxPath.c_str(),"rb+");

                    //apuntamos a la particion
                    fread(&mbr,sizeof(MBR),1,fp);
                    fseek(fp,mbr.MBR_partition[indicadorParticion].part_start,SEEK_SET);
                    fclose(fp);
                    REPORTE_SUPERBLOQUE(auxAuxPath, auxPath, extension, mbr.MBR_partition[indicadorParticion].part_start);
                }else{
                    int indicadorParticion = buscarParticion_Logica(auxAuxPath, nombre);
                    if(indicadorParticion != -1){
                        EBR ebr;
                        FILE *fp = fopen(auxAuxPath.c_str(),"rb+");

                        //apuntamos a la particion
                        fseek(fp,indicadorParticion,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,fp);
                        int inicio = static_cast<int>(ftell(fp));
                        fclose(fp);
                        REPORTE_SUPERBLOQUE(auxAuxPath, auxPath, extension, inicio);
                    }
                }
            }
            
        }else if (auxName == "file")
        {
            //obtiene extension
            string extension = ObtenerExtension(auxPath);
            string auxAuxPath = "";
            string nombre = "";

            //bandera del ID
            bool flagId = false;

            //obtenemos datos para ejecutar los siguientes comandos
            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    auxAuxPath = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }


            if (flagId)
            {
                int indicadorParticion = buscarParticionP_E(auxAuxPath, nombre);
                if(indicadorParticion != -1)
                {
                    MBR mbr;
                    char auxRuta[500];
                    //asignamos el valor de aux ruta al archivo de lectura
                    strcpy(auxRuta, auxRuta_file.c_str());

                    //abrimos disco
                    FILE *fp = fopen(auxAuxPath.c_str(),"rb+");
                    fread(&mbr, sizeof(MBR),1,fp);
                    int existeCarpeta = buscar_CarpetaAr(fp, auxRuta);

                    //existencia carpeta
                    if(existeCarpeta != -1)
                    {
                        char nombreRuta[50];
                        char auxRuta[400];

                        //apuntador de la carpeta
                        strcpy(auxRuta, auxRuta_file.c_str());
                        strcpy(nombreRuta, basename(auxRuta));
                        REPORTE_FILE(auxAuxPath, auxPath, extension, nombreRuta, mbr.MBR_partition[indicadorParticion].part_start,existeCarpeta);
                    }
                    fclose(fp);
                }
            }else
            {
                cout << "\033[91mERROR, No se encontro ID del moun.\033[0m" << endl;
            }
        }else if (auxName == "ls")
        {
            //obtiene extension
            string extension = ObtenerExtension(auxPath);
            string auxAuxPath = "";
            string nombre = "";

            //bandera del ID
            bool flagId = false;

            //obtenemos datos para ejecutar los siguientes comandos
            for (int i = 0; i < Arreglomount.size(); i++)
            {
                if (auxId == Arreglomount[i].id)
                {
                    auxAuxPath = Arreglomount[i].direccion;
                    nombre = Arreglomount[i].nombre;
                    flagId = true;
                    break;
                }
            }

            if (flagId)
            {
                int index = buscarParticionP_E(auxAuxPath, nombre);
                if(index != -1)
                {
                    MBR mbr;
                    SUPERBLOQUE super;
                    INODOTABLA inodo;
                    char auxRuta[500];

                    strcpy(auxRuta, auxRuta_file.c_str());

                    //abrimos la ruta de direccion
                    FILE *fp = fopen(auxAuxPath.c_str(),"rb+");
                    fread(&mbr,sizeof(MBR),1,fp);
                    fseek(fp,mbr.MBR_partition[index].part_start,SEEK_SET);
                    fread(&super,sizeof(SUPERBLOQUE),1,fp);

                    int existe = buscar_CarpetaAr(fp, auxRuta);
                    if(existe != -1)
                    {
                        char nombre[50];
                        char auxRuta[400];

                        //asigmamos la ruta que viene en el comando ruta
                        strcpy(auxRuta, auxRuta_file.c_str());
                        strcpy(nombre,basename(auxRuta));

                        //apuntador del bimap de inodos principal
                        fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA))*existe,SEEK_SET);
                        fread(&inodo,sizeof(INODOTABLA),1,fp);

                        //obtenemos el usuario del creado
                        USUARIO user = obtener_usuario(auxAuxPath, mbr.MBR_partition[index].part_start, inodo.i_uid);
                        REPORTE_LS(auxAuxPath, auxPath, extension,mbr.MBR_partition[index].part_start,existe,user, string(nombre));
                    }
                    else
                        cout << "ERROR: No se encuentra la ruta " << endl;
                    fclose(fp);
                }else{

                }
            }else
            {
                cout << "\033[91mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
            }
        }
    }else{
        cout << "\033[94mERROR, no se recibio algun parametro obligatorio.\033[0m" << endl;
    }
}

//-- comandos --
void mandaraComando(string comando, vector<string> datos){
    if (comando == "mkdisk")
    {
        MKDISK(datos);
                
    }else if (comando == "rmdisk")
    {
        RMDISK(datos);

    }else if (comando == "fdisk")
    {
        FDISK(datos);

    }else if (comando == "exec")
    {

        for (int i = 1; i < datos.size(); i++)
        {
            vector<string> tipoP;
            tipoP = splitParam(datos.at(i));

            if (tipoP.at(0) == "SIN SIMBOLO" || tipoP.at(0) == "SIN PUNTOS")
            {
                if (tipoP.at(0) == "SIN SIMBOLO")
                {
                    cout << "Falta el simbolo ~, omitimos linea" << endl;
                    break;
                }else{
                    cout << "Falta el simbolo :, omitimos linea" << endl;
                    break;
                }
            }else{
                string coman = minusculas(tipoP[0]);
                string datoComan = tipoP[2];

                if(coman == "-path"){
                    
                    if (datoComan.substr(datoComan.find_last_of(".")+1) == "sh")
                    {
                        ifstream archivo(datoComan.c_str());
                        string linea;


                        while (getline(archivo, linea))
                        {
                            if (linea.empty())
                            {
                                continue;
                            }else if(linea.at(0) == '#')
                            {
                                continue;
                            }else{
                                
                                string comentario = "#";

                                vector<string> var;

                                if (strstr(linea.c_str(), comentario.c_str()))
                                {
                                    vector<string> temp{};
                                    temp = splitExec(linea);
                                    var = split(temp[0]);
                                }else{
                                    var = split(linea);
                                }

                                vector<string> comandosE{};

                                string comando = minusculas(var[0]);

                                if (existeEnVector(TodosComandos, comando) )
                                {
                                    mandaraComando(comando, var);

                                }else if (comando == "exit")
                                {
                                    exit(0);
                                }else{
                                    cout << "No existe el comando: " << comando << endl;
                                }
                            }
                        }
                    }else{
                        cout << "Extension invalida del archivo" << endl;
                        break;
                    }

                }else{
                    cout << "Comando invalido del EXEC" << endl;
                    break;
                }
            }
        }
    }else if (comando == "pause")
    {
        PAUSE(datos);
    }else if (comando == "mount")
    {
        MOUNT(datos);
    }else if (comando == "unmount")
    {
        UNMOUNT(datos);
    }else if (comando == "mkfs")
    {
        MKFS(datos);  
    }else if (comando == "login")
    {
        LOGIN(datos);
    }else if (comando == "logout")
    {
        LOGOUT(datos);
    }else if (comando == "mkgrp")
    {
        MKGRP(datos);
    }else if (comando == "rmgrp")
    {
        RMGRP(datos);
    }else if (comando == "mkusr")
    {
        MKUSR(datos);
    }else if (comando == "rmusr")
    {
        RMUSR(datos);
    }else if (comando == "chmod")
    {
        CHMOD(datos);
    }else if (comando == "mkfile")
    {
        MKFILE(datos);
    }else if (comando == "mkdir")
    {
        MKDIR(datos);
    }else if (comando == "rep")
    {
        REPORTES(datos);
    }else if (comando == "pause")
    {
        cout << "pause" << endl;
    }
    
}

int main(){

    cout <<  "$$---------------------------------------------------------------$$" << endl;
    cout <<  "##                  Julio José Orellana Ruíz                     ##" << endl;
    cout <<  "##                         201908120                             ##" << endl;
    cout <<  "##                                                               ##" << endl;
    cout <<  "##                    Proyecto 1 --- MIA                         ##" << endl;
    cout <<  "##             curso vacas -- 2do. Semestre -- 2021              ##" << endl;
    cout <<  "$$---------------------------------------------------------------$$\n" << endl;

    while (true)
    {
        cout <<  " >> Ingrese comando: " << ends;
        string ruta;
        getline(cin, ruta);

        vector<string> var;
        var = split(ruta);
        vector<string> comandosE{};

        string comando = minusculas(var[0]);

        if (existeEnVector(TodosComandos, comando) )
        {
            mandaraComando(comando, var);

        }else if (comando == "exit")
        {
            exit(0);
        }else{
            cout << "No existe el comando: " << comando << endl;
        }
    }
    
    return 0;
}