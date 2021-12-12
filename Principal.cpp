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
using namespace std;

// --  GLOBALES--
//TODO: agregar el resto de comandos
vector<string> TodosComandos = {"mkdisk", "rmdisk", "fdisk", "exec", "pause", "mount", "unmount", "mkfs",
"login"};
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
        tamanio == 1024 * 1024 * dc.size;
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
                    int tamanioNeed = ebr.EBR_part_start + ebr.EBR_part_size;

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
                        cout << "Particion logica creada con exito "<< endl;
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

    fseek(fp, actualSesion.inicioSuper,SEEK_SET);
    fread(&super,sizeof(SUPERBLOQUE),1,fp);
    //Leemos el inodo del archivo users.txt
    fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);
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

        fread(&mbr,sizeof(MBR),1,fp);
        fseek(fp,mbr.MBR_partition[index].part_start,SEEK_SET);

        fread(&super,sizeof(SUPERBLOQUE),1,fp);
        fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);

        fread(&inodo,sizeof(INODOTABLA),1,fp);
        fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);

        inodo.i_atime = time(nullptr);

        fwrite(&inodo,sizeof(INODOTABLA),1,fp);
        fclose(fp);

        actualSesion.inicioSuper = mbr.MBR_partition[index].part_start;
        actualSesion.fit = mbr.MBR_partition[index].part_fit;
        actualSesion.inicioJournal = mbr.MBR_partition[index].part_start + static_cast<int>(sizeof(SUPERBLOQUE));
        actualSesion.tipo_sistema = super.s_filesystem_type;

        return verificarDatos_login(user,password, direccion);
    }else{
        index = buscarParticion_Logica(direccion, nombre);
        if(index != -1){

            SUPERBLOQUE super;
            INODOTABLA inodo;

            FILE *fp = fopen(direccion.c_str(),"rb+");
            fseek(fp,index + static_cast<int>(sizeof(EBR)),SEEK_SET);
            fread(&super,sizeof(SUPERBLOQUE),1,fp);
            fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);

            fread(&inodo,sizeof(INODOTABLA),1,fp);
            fseek(fp,super.s_inode_start + static_cast<int>(sizeof(INODOTABLA)),SEEK_SET);

            inodo.i_atime = time(nullptr);

            fwrite(&inodo,sizeof(INODOTABLA),1,fp);
            fclose(fp);

            actualSesion.inicioSuper = index + static_cast<int>(sizeof(EBR));
            actualSesion.fit = obtenerLogic(direccion, nombre);
            return verificarDatos_login(user,password,direccion);
        }
    }
    return 0;
}


// -- Ejecucion del comando login
void comando_login(string usr, string password, string id){

    /*Banderas para verificar cuando venga un parametro y si se repite*/
    bool flagUsr = false;
    bool flagPass = false;
    bool flagId = false;
    bool flag = false;
    bool flagMkfs = false;

    /*Variables para obtener los valores de cada nodo*/

    flagMkfs = buscar_IdMkfs(id);
    flagId = busca_IDmount(id);
    if (flagMkfs)
    {
        if(!flag_login){
        if(flagId){
            int indiceID = index_IDmount(id);
            int res = log_entra(Arreglomount[indiceID].direccion, Arreglomount[indiceID].nombre, usr, password);
            if(res == 1){
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
                Discvar.path = path + datoComan;
                
                //encontramos el tamaño del nombre del disco con todo y extension
                int pivote;
                int contador = 0;
                for (int i = 0; i < datoComan.length(); i++)
                {
                    if ((datoComan[((datoComan.length()-1) - i)]) == '/')
                    {
                        pivote = contador;
                        break;
                    }
                    contador ++;
                }
                int empieza = Discvar.path.length() - pivote - 1;
                //aca recortamos la ruta para que nos cree esas carpetas
                string cortada = Discvar.path.substr(0, empieza);

                if (mkdir(cortada.c_str(), 0777) == -1)cerr << "Error :  " << strerror(errno) << endl;
                else cout << "Directory created"<<endl; 
                

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
            string datoComan = path + tipoP.at(2);
            if (coman == "-path")
            {
                BorrarDisco(datoComan);
            }
            
        }
    }
    
}

//-- FDISK --
void FDISK(vector<string> datos){
    
    string rutaTotal = "";
    char fit = 0;
    int size_part = 0;
    char unit = 'B';
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
                if (datoComan == "B")
                {
                    //size
                    unit = 'B';
                }else if (datoComan == "K")
                {
                    //1024 * size
                    unit = 'K';
                }else if (datoComan == "M")
                {
                    //1024 * 1024 * size
                    unit = 'M';
                }else{
                    cout << "parametro invalido" << endl;
                    break;
                }
                
            }else if (coman == "-type")
            {
                if (datoComan == "P")
                {
                    typeFd = 'P';
                }else if (datoComan == "E")
                {
                    typeFd = 'E';
                }else if (datoComan == "L")
                {
                    typeFd = 'L';
                }else{
                    cout << "parametro invalido" << endl;
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
                    cout << "parametro invalido" << endl;
                    break;
                }
                
            }else if (coman == "-name")
            {
                name = datoComan;
            }else if (coman == "-add")
            {
                name = datoComan;
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

//--comandos
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
                                vector<string> var;
                                var = split(linea);

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