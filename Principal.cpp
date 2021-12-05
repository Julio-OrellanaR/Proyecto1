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
#include <bits/stdc++.h>
#include<algorithm>
using namespace std;

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
            cout << "extraje pos 0: " << tipoP.at(0) << " !pos 1: " << tipoP.at(2) << endl;
        }
        
        cout << i << ") "<< datos.at(i) << endl;
    }
    
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