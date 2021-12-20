SET FOREIGN_KEY_CHECKS = 0;

insert into project(
project_id,
is_geocoded,
project_title,
start_actual_isodate,
end_actual_isodate,
donors,
donors_iso3,
recipients,
recipients_iso3 ,
ad_sector_codes ,
ad_sector_names ,
status,
transactions_start_year,
transactions_end_year ,
total_commitments ,
total_disbursements
)
select distinct
temp_project.project_id,
temp_project.is_geocoded,
temp_project.project_title,
temp_project.start_actual_isodate,
temp_project.end_actual_isodate,
temp_project.donors ,
temp_project.donors_iso3,
temp_project.recipients ,
temp_project.recipients_iso3 ,
temp_project.ad_sector_codes ,
temp_project.ad_sector_names ,
temp_project.status ,
temp_project.transactions_start_year,
temp_project.transactions_end_year ,
temp_project.total_commitments ,
temp_project.total_disbursements 
from temp_project;
SET FOREIGN_KEY_CHECKS = 1;


SET FOREIGN_KEY_CHECKS = 0;
insert into transaction(
id_transaction,
idProject,
transaction_isodate,
transaction_year ,
transaction_value_code ,
transaction_currency ,
transaction_value 
)
select distinct
temp_transaction.id_transaction,
temp_transaction.idProject,
temp_transaction.transaction_isodate,
temp_transaction.transaction_year ,
temp_transaction.transaction_value_code ,
temp_transaction.transaction_currency ,
temp_transaction.transaction_value 
from temp_transaction;
SET FOREIGN_KEY_CHECKS = 1;

SET FOREIGN_KEY_CHECKS = 0;

insert into countryCodes(
name,
iso2,
name_name,
name_aiddata_code,
name_aiddata_name,
name_cow_alpha,
name_cow_numeric,
name_fao_code,
name_fips,
name_geonames_id,
name_imf_code,
name_iso2,
name_iso3,
name_iso_numeric,
name_oecd_code,
name_oecd_name,
name_un_code,
name_wb_code
)
select distinct 
temp_countryCodes.name,
temp_countryCodes.iso2,
temp_countryCodes.name_name,
temp_countryCodes.name_aiddata_code,
temp_countryCodes.name_aiddata_name,
temp_countryCodes.name_cow_alpha,
temp_countryCodes.name_cow_numeric,
temp_countryCodes.name_fao_code,
temp_countryCodes.name_fips,
temp_countryCodes.name_geonames_id,
temp_countryCodes.name_imf_code,
temp_countryCodes.name_iso2,
temp_countryCodes.name_iso3,
temp_countryCodes.name_iso_numeric,
temp_countryCodes.name_oecd_code,
temp_countryCodes.name_oecd_name,
temp_countryCodes.name_un_code,
temp_countryCodes.name_wb_code
from  temp_countryCodes;
SET FOREIGN_KEY_CHECKS = 1;

SET FOREIGN_KEY_CHECKS = 0;
insert into geoName(
idGeoname,
place_name,
latitude,
longitude,
location_type_code,
gazetteer_adm_code,
gazetteer_adm_name,
location_class,
geographic_exactness
)
select 
temp_geoName.idGeoname,
temp_geoName.place_name,
temp_geoName.latitude,
temp_geoName.longitude,
temp_geoName.location_type_code,
temp_geoName.gazetteer_adm_code,
temp_geoName.gazetteer_adm_name,
temp_geoName.location_class,
temp_geoName.geographic_exactness
from  temp_geoName;
SET FOREIGN_KEY_CHECKS = 1;

SET FOREIGN_KEY_CHECKS = 0;
insert into Level(
project_id,
project_location_id,
geoname_id,
transactions_start_year,
transactions_end_year_,
event_split_commitments,
event_split_disbursements
)
select distinct 
temp_Level.project_id,
temp_Level.project_location_id,
temp_Level.geoname_id,
temp_Level.transactions_start_year,
temp_Level.transactions_end_year_,
temp_Level.event_split_commitments,
temp_Level.event_split_disbursements
from  temp_Level;
SET FOREIGN_KEY_CHECKS = 1;

SET FOREIGN_KEY_CHECKS = 0;
insert into Locations(
location_type_code,
location_type_name
)
select distinct
temp_Locations.location_type_code,
temp_Locations.location_type_name
from temp_Locations;
SET FOREIGN_KEY_CHECKS = 1;
