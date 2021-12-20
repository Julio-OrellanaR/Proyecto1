#!/bin/bash
mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/script_eliminar.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/script_database.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/script_temporales.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/archivos de control/control_countyCodes.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/archivos de control/control_geoName.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/archivos de control/control_level.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/archivos de control/control_location.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/archivos de control/control_project.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/archivos de control/control_transaction.sql"

mysql -h localhost --local_infile=1 -u root -pJulio123- < "/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/script_llenado.sql"
