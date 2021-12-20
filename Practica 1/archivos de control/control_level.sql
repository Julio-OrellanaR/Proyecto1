use practica1;
SET GLOBAL local_infile=1;

load data local infile '/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/ArchivosCSV/level_1a.csv' 
into table temp_Level
character set UTF8
fields terminated by ',' lines terminated by '\n' ignore 1 lines
(
@dataLvl1,@dataLvl2,@dataLvl3,@dataLvl4,@dataLvl5,@dataLvl6,@dataLvl7
)
set project_id = if(@dataLvl1 = '',NULL,@dataLvl1),
project_location_id = if(@dataLvl2 = '',NULL,@dataLvl2),
geoname_id = if(@dataLvl3 = '',NULL,@dataLvl3),
transactions_start_year = if(@dataLvl4 = '',NULL,@dataLvl4),
transactions_end_year_ = if(@dataLvl5 = '',NULL,@dataLvl5),
event_split_commitments = if(@dataLvl6 = '',NULL,@dataLvl6),
event_split_disbursements = if(@dataLvl7 = '',NULL,@dataLvl7);
