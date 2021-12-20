use practica1;
SET GLOBAL local_infile=1;

load data local infile '/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/ArchivosCSV/projects.csv' 
into table temp_project
character set latin1
fields terminated by ',' lines terminated by '\n' ignore 1 lines
(
@dataP1,@dataP2,@dataP3,@dataP4,@dataP5,@dataP6,@dataP7,@dataP8,@dataP9,@dataP10,@dataP11,@dataP12,@dataP13,
@dataP14,@dataP15,@dataP16
)
set project_id=if(@dataP1 = '',NULL,@dataP1),
is_geocoded=if(@dataP2 = '',NULL,@dataP2),
project_title=if(@dataP3 = '',NULL,@dataP3),
start_actual_isodate=if(@dataP4 = '',NULL,str_to_date(@dataP4, '%d/%m/%Y')),
end_actual_isodate=if(@dataP5 = '',NULL,str_to_date(@dataP5, '%d/%m/%Y')),
donors=if(@dataP6 = '',NULL,@dataP6),
donors_iso3=if(@dataP7 = '',NULL,@dataP7),
recipients=if(@dataP8 = '',NULL,@dataP8),
recipients_iso3=if(@dataP9 = '',NULL,@dataP9),
ad_sector_codes=if(@dataP10 = '',NULL,@dataP10),
ad_sector_names=if(@dataP11 = '',NULL,@dataP11),
status=if(@dataP12 = '',NULL,@dataP12),
transactions_start_year=if(@dataP13 = '',NULL,@dataP13),
transactions_end_year=if(@dataP14 = '',NULL,@dataP14),
total_commitments=if(@dataP15 = '',NULL,@dataP15),
total_disbursements=if(@dataP16 = '',NULL,@dataP16);
