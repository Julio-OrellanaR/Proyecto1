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
using namespace std;

//-- ESTRUCTURAS
//-- PARTITION --
struct PARTITION
{
    char part_status;
    char part_type = 'P';
    string part_fit = "FF";
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
    string disk_fit="FF";
    PARTITION MBR_partition[4];
};

//-- DISCO --
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
        mbr.MBR_partition[i].part_fit = "FF";
        mbr.MBR_partition[i].part_start = tamanio;
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
                Discvar.fit = datoComan;
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
                    cout<< "error" << endl;
                }
                
                
                cout << "unit: " << tipoP.at(2) << endl;

            }else if (coman == "-path")
            {
                Discvar.path = datoComan;
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
}

//-- FDISK --
void FDISK(vector<string> datos){
    cout << "estamos en fdisk" << endl;
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

    }else if (comando == "fdisk")
    {
        cout << "nos vamos a fdisk" << endl;

    }
}



int main(){

    //TODO: agregar el resto de comandos
    vector<string> TodosComandos = {"mkdisk", "rmdisk", "fdisk"};

    while (true)
    {
        cout <<  "ruta a pedir: " << ends;
        string ruta;
        getline(cin, ruta);

        cout << "la ruta es: " << ruta << endl;

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