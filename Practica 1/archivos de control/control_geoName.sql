use practica1;
SET GLOBAL local_infile=1;

load data local infile '/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/ArchivosCSV/geonames.csv' 
into table temp_geoName
character set UTF8
fields terminated by ',' lines terminated by '\n' ignore 1 lines
(
@dataGN1,@dataGN2,@dataGN3,@dataGN4,@dataGN5,@dataGN6,@dataGN7,@dataGN8,@dataGN9
)
set idGeoname=if(@dataGN1 = '',NULL,@dataGN1),
place_name=if(@dataGN2 = '',NULL,@dataGN2),
latitude=if(@dataGN3 = '',NULL,@dataGN3),
longitude=if(@dataGN4 = '',NULL,@dataGN4),
location_type_code=if(@dataGN5 = '',NULL,@dataGN5),
gazetteer_adm_code=if(@dataGN6 = '',NULL,@dataGN6),
gazetteer_adm_name=if(@dataGN7 = '',NULL,@dataGN7),
location_class=if(@dataGN8 = '',NULL,@dataGN8),
geographic_exactness=if(@dataGN9 = '',NULL,@dataGN9);
