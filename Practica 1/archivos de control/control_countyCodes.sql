use practica1;
SET GLOBAL local_infile=1;

load data local infile '/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/ArchivosCSV/country_codes.tsv' 
into table temp_countryCodes
character set UTF8
fields terminated by '\t' lines terminated by '\n' ignore 1 lines
(
@dataCC1,@dataCC2,@dataCC3,@dataCC4,@dataCC5,@dataCC6,@dataCC7,@dataCC8,@dataCC9,
@dataCC10,@dataCC11,@dataCC12,@dataCC13,@dataCC14,@dataCC15,@dataCC16,@dataCC17,@dataCC18
)
set name=if(@dataCC1 = '',NULL,@dataCC1),
iso2=if(@dataCC2 = '',NULL,@dataCC2),
name_name=if(@dataCC3 = '',NULL,@dataCC3),
name_aiddata_code=if(@dataCC4 = '',NULL,@dataCC4),
name_aiddata_name=if(@dataCC5 = '',NULL,@dataCC5),
name_cow_alpha=if(@dataCC6 = '',NULL,@dataCC6),
name_cow_numeric=if(@dataCC7 = '',NULL,@dataCC7),
name_fao_code=if(@dataCC8 = '',NULL,@dataCC8),
name_fips=if(@dataCC9 = '',NULL,@dataCC9),
name_geonames_id=if(@dataCC10 = '',NULL,@dataCC10),
name_imf_code=if(@dataCC11 = '',NULL,@dataCC11),
name_iso2=if(@dataCC12 = '',NULL,@dataCC12),
name_iso3=if(@dataCC13 = '',NULL,@dataCC13),
name_iso_numeric=if(@dataCC14 = '',NULL,@dataCC14),
name_oecd_code=if(@dataCC15 = '',NULL,@dataCC15),
name_oecd_name=if(@dataCC16 = '',NULL,@dataCC16),
name_un_code=if(@dataCC17 = '',NULL,@dataCC17),
name_wb_code=if(@dataCC18 = '',NULL,@dataCC18);
