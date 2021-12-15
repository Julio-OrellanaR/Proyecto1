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
"login", "logout", "mkgrp", "rmgrp", "mkusr", "rmusr", "chmod", "mkfile", "mkdir"};
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
                            cout << "Usuario eliminado con exito" << endl;
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
    
    cout << "el falg " << valPath << endl;
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
        {//Usuario root
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
                    {//Usuario root
                        if(buscarGrupo(group) != -1)
                        {
                            if(!buscarUsuario(user))
                            {
                                int id = buscarId_usr();
                                string datos = to_string(id) + ",U,"+group+","+user+","+pass+"\n";
                                agregarUsuario_TXT(datos);
                                cout << "Usuario creado con exito " << endl;
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
        if(actualSesion.id_user == 1 && actualSesion.id_grp == 1){//Usuario root
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
        if(actualSesion.id_user == 1 && actualSesion.id_grp == 1)//Usuario root
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

        cout << "\033[94m\n--- Sesion finalizada ---.\033[0m" << endl;
    }else
        cout << "\033[31mERROR, no hay sesion acutalmente.\033[0m" << endl;
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
                string ident = "\"";
                if(strstr(datoComan.c_str(), ident.c_str())){
                    string uno = SplitCExec(datoComan);
                    uno = path + uno;
                    BorrarDisco(uno);

                }else{
                    datoComan = path + datoComan;
                    BorrarDisco(datoComan);
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
    cout << "estamos en pause" << endl;
    
    
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
    }
    
    
    
    
     
}

int main(){

    while (true)
    {
        cout <<  "Ingrese comando: " << ends;
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