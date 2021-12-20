use practica1;
SET GLOBAL local_infile=1;

load data local infile '/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/ArchivosCSV/locations.csv' 
into table temp_Locations
character set UTF8
fields terminated by ',' lines terminated by '\n' ignore 1 lines
(
@dataLT,@dataLT2
)
set location_type_code=if(@dataLT = '',NULL,@dataLT),
location_type_name=if(@dataLT2 = '',NULL,@dataLT2);
