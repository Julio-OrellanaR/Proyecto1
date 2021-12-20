use practica1;
SET GLOBAL local_infile=1;

load data local infile '/home/julio-or/Documentos/Lab archivos/Proyecto1/Practica 1/ArchivosCSV/transactions.csv' 
into table temp_transaction
character set UTF8
fields terminated by ',' lines terminated by '\n' ignore 1 lines
(
@dataT1,@dataT2,@dataT3,@dataT4,@dataT5,@dataT6,@dataT7
)
set id_transaction = if(@dataT1 = '',NULL,@dataT1),
idProject =if(@dataT2 = '',NULL,@dataT2),
transaction_isodate= str_to_date(@dataT3, '%d/%m/%Y'),
transaction_year=if(@dataT4 = '',NULL,@dataT4),
transaction_value_code=if(@dataT5 = '',NULL,@dataT5),
transaction_currency=if(@dataT6 = '',NULL,@dataT6),
transaction_value=if(@dataT7 = '',NULL,@dataT7);
