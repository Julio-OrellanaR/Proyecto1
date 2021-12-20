use practica1;

create table countryCodes(
  id int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
  name    varchar(50) NOT NULL UNIQUE,
  iso2    varchar(3)  NULL,
  name_name varchar(50) NOT NULL,
  name_aiddata_code int(11) NULL,
  name_aiddata_name varchar(50) NULL,
  name_cow_alpha varchar(15) NULL,
  name_cow_numeric int(5) NULL,
  name_fao_code int(5) NULL,
  name_fips varchar(6) NULL,
  name_geonames_id int(11) NULL,
  name_imf_code int(11) NULL,
  name_iso2 varchar(7) NULL,
  name_iso3 varchar(8) NULL,
  name_iso_numeric int(11) NULL,
  name_oecd_code int(11) NULL,
  name_oecd_name varchar(50) NULL,
  name_un_code int(11) NULL,
  name_wb_code varchar(6) NULL
);

CREATE TABLE Currency (
  idCurrency int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
  name_Currency varchar(15)  NOT NULL UNIQUE
);

CREATE TABLE geoName(
id int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
idGeoname int(11) NOT NULL,
place_name varchar(100) NOT NULL,
latitude decimal(16.5) NOT NULL,
longitude decimal(16.5) NOT NULL,
location_type_code int(11) NOT NULL,
gazetteer_adm_code varchar(250) NOT NULL,
gazetteer_adm_name varchar(250) NOT NULL,
location_class int(2) NOT NULL,
geographic_exactness int(2) NOT NULL
);

CREATE TABLE Level (
id int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
project_id int(11) NOT NULL,
project_location_id varchar(10) NOT NULL,
geoname_id int(11) NOT NULL,
transactions_start_year int(11) NOT NULL,
transactions_end_year_ int(11) NOT NULL,
event_split_commitments decimal(16.5) NULL,
event_split_disbursements decimal(16.5) NULL
);

CREATE TABLE Locations (
  idLocation int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
  location_type_code varchar(6) NOT NULL UNIQUE,
  location_type_name varchar(250) NOT NULL
);

CREATE TABLE project (
  idProyecto int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY, 
  project_id varchar(15) NOT NULL UNIQUE,
  is_geocoded int(2) NOT NULL,
  project_title varchar(250) NOT NULL,
  start_actual_isodate date NULL,
  end_actual_isodate date NULL,
  donors varchar(15) NOT NULL,
  donors_iso3 varchar(5) NOT NULL,
  recipients int(11) NOT NULL,
  recipients_iso3 varchar(4) NOT NULL,
  ad_sector_codes varchar(100) NOT NULL,
  ad_sector_names varchar(300) NOT NULL,
  status int(11) NOT NULL,
  transactions_start_year int(11) NOT NULL,
  transactions_end_year int(11) NOT NULL,
  total_commitments decimal(16,5) NOT NULL,
  total_disbursements decimal(16,5) NOT NULL
);


CREATE TABLE transaction(
id int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
id_transaction varchar(25) NOT NULL UNIQUE,
idProject int(11) NOT NULL,
transaction_isodate date NOT NULL,
transaction_year int(11) NOT NULL,
transaction_value_code varchar(5) NOT NULL,
transaction_currency int(11) NOT NULL,
transaction_value decimal(16.5) NULL
);

CREATE TABLE status(
idStatus int(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
name varchar(25) NOT NULL UNIQUE
);

ALTER TABLE transaction
ADD FOREIGN KEY (transaction_currency) REFERENCES Currency(idCurrency);

ALTER TABLE geoName
ADD FOREIGN KEY (location_type_code) REFERENCES Locations(idLocation);

ALTER TABLE transaction
ADD FOREIGN KEY (idProject) REFERENCES project(idProyecto);

ALTER TABLE project
ADD FOREIGN KEY (status) REFERENCES status(idStatus);

ALTER TABLE project
ADD FOREIGN KEY (recipients) REFERENCES countryCodes(id);

ALTER TABLE Level
ADD FOREIGN KEY (project_id) REFERENCES project(idProyecto);

ALTER TABLE Level
ADD FOREIGN KEY (geoname_id) REFERENCES geoName(id);

