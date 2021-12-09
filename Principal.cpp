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
using namespace std;

// --  GLOBALES--
//TODO: agregar el resto de comandos
vector<string> TodosComandos = {"mkdisk", "rmdisk", "fdisk", "exec", "pause"};
void CreatePartitionPimari(int, char, string, char, string, string);
bool existePARTITION(string, string);


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
    string EBR_part_fit;
    int EBR_part_start;
    int EBR_part_size;
    int EBR_part_next;
    char EBR_part_name[16];
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

void CreatePartitionPimari(int size, char unit, string pathPart, string fit, string name){
    char auxUnit = 0;
    char auxFit = 0;
    string auxPath = pathPart;
    int size_bytes = 1024;
    char buff = '1';

    //definimos el fit que utilizara segun el parametro
    if (fit != "")
    {
        if (fit == "WF")
        {
            auxFit = 'W';
        }else if (fit == "FF")
        {
            auxFit = 'F';
        }else if (fit == "BF")
        {
            auxFit = 'B';
        }
        
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
                        mbr.MBR_partition[indicadorPartition].part_type = 'E';
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

                        cout << "...\n" << "Particion extendida creada con exito" << endl;
 
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

        cout << extendida << endl;

        if (extendida != -1)
        {
            fseek(fp, mbr.MBR_partition[extendida].part_start, SEEK_SET);
            EBR ebr;

            while (((fread(&ebr, sizeof(EBR), 1, fp)) != 0) && (ftell(fp) < (mbr.MBR_partition[extendida].part_size + mbr.MBR_partition[extendida].part_start)))
            {
                cout << "estoy recorriendo " << endl;
                if (strcmp(ebr.EBR_part_name, name.c_str()) == 0)
                {
                    cout << "primera opcion" << endl;
                    fclose(fp);
                    return true;
                }
                if (ebr.EBR_part_next == -1)
                {
                    cout << "segunda opcion"<< ebr.EBR_part_next << endl;
                    fclose(fp);
                    return false;
                }
            }
        }
    }
    fclose(fp);
    return false;
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
    cout << "estamos en mkdisk" << endl;

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
    cout << "estamos en rmdisk" << endl;

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
    cout << "estamos en fdisk" << endl;
    string rutaTotal = "";
    string fit = "";
    int size_part = 0;
    char unit = 'B';
    char type = 0;
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
                    type = 'P';
                }else if (datoComan == "E")
                {
                    type = 'E';
                }else if (datoComan == "L")
                {
                    type = 'L';
                }else{
                    cout << "parametro invalido" << endl;
                    break;
                }
            }else if (coman == "-fit")
            {
                string tipFit = minusculas(datoComan);
                if (tipFit == "wf")
                {
                    fit = "WF";
                }else if (tipFit == "ff")
                {
                    fit = "FF";
                }else if (tipFit == "bf")
                {
                    fit = "BF";
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
    CreatePartitionPimari(size_part, unit, rutaTotal, fit, name);
    
    cout << "Salimos del Fdisk" << endl;
    
}

// -- PAUSE --
void PAUSE(vector<string> datos){
    cout << "estamos en pause" << endl;
    
    
}


//--comandos
void mandaraComando(string comando, vector<string> datos){
    if (comando == "mkdisk")
    {
        cout << "nos vamos a mkdisk" << endl;
        MKDISK(datos);
                
    }else if (comando == "rmdisk")
    {
        cout << "nos vamos a rmdisk" << endl;
        RMDISK(datos);

    }else if (comando == "fdisk")
    {
        cout << "nos vamos a fdisk" << endl;
        FDISK(datos);

    }else if (comando == "exec")
    {
        cout << "nos vamos al exec" << endl;
        cout << "estamos ene exec" << endl;

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

                                cout << "la espliteada de exec: " << var[0] << endl;
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
    }
    
}

int main(){

    while (true)
    {
        cout <<  "comando a pedir: " << ends;
        string ruta;
        getline(cin, ruta);

        cout << "el comando es: " << ruta << endl;

        vector<string> var;
        var = split(ruta);
        vector<string> comandosE{};

        cout << "la espliteada: " << var[0] << endl;
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