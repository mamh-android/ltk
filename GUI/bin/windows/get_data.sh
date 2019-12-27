#!/bin/bash

server_IP=$1
client_name=$2
password=$3
key_word=$4

server_IP=10.38.34.175
client_name=wsun
password=123456
key_word=$1

data_base=ltk
key_col=Project
table1=TestInfo
table2=template
table3=TestCase
same_col_12="ProjectIndex"
same_col_23="ID"
#mysql -h$server_IP -u$client_name -p$password -e "use users ; insert into user2 values('123456777',$loop_time);"
#mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; select * from user2 where pwd REGEXP $key_word;"

"Test Project","Test Board","Test Version","Test Type","Test Engineer","LTK version","Test Date"
ID,"Test Type","Test Domain","Test case Description","Test Level",Status,Defect,CostTime,Comment
#output_col_ID=$table2.ID,$table3.Type,$table3.Domain,$table3.Description,$table3.LEVEL,$table2.Status,$table2.Defect,$table2.CostTime,$table2.Comment
output_col_Info=$table1.Project,$table1.Board,$table1.Version,$table1.Type,$table1.Engineer,$table1.LTKVersion,$table1.Date
output_col_ID=$table2.ID,$table3.Type,$table3.Domain,$table3.Description,$table3.LEVEL,$table2.Status,$table2.Defect,$table2.CostTime,$table2.Comment,$table1.Board,$table1.Version,$table1.Engineer,$table1.LTKVersion,$table1.Date

echo "use $data_base ; select "$table1".*,"$table2".*,"$table3".* from $table2 INNER JOIN $table3 on $table2.$same_col_23=$table3.$same_col_23 INNER JOIN $table on $table1.$same_col_12=$table2.$same_col_12 where $key_col REGEXP \"$key_word\";"


mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; select "$table1".*,"$table2".*,"$table3".* from $table2 INNER JOIN $table3 on $table2.$same_col_23=$table3.$same_col_23 INNER JOIN $table1 on $table1.$same_col_12=$table2.$same_col_12 where $key_col REGEXP \"$key_word\";"

mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; select $output_col_ID from $table2 INNER JOIN $table3 on $table2.$same_col_23=$table3.$same_col_23 INNER JOIN $table1 on $table1.$same_col_12=$table2.$same_col_12 where $key_col REGEXP \"$key_word\";" | tee template_tmp.csv 

mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; select $output_col_Info from $table2 INNER JOIN $table3 on $table2.$same_col_23=$table3.$same_col_23 INNER JOIN $table1 on $table1.$same_col_12=$table2.$same_col_12 where $key_col REGEXP \"$key_word\" ;" #| tee testInfo_tmp.csv 



mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; select $output_col_Info from $table2 INNER JOIN $table3 on $table2.$same_col_23=$table3.$same_col_23 INNER JOIN $table1 on $table1.$same_col_12=$table2.$same_col_12 where $key_col REGEXP \"$key_word\" group by Project;" | tee testInfo_tmp.csv 

cat testInfo_tmp.csv | sed -e s/"	"/\",\"/g | sed s/$/'"'/ |  sed s/^/'"'/ > testInfo.csv 
#cat template_tmp.csv | sed -e s/"	"/\",\"/g | sed s/$/'"'/ |  sed s/^/'"'/ > template.csv 

temp_time=0
case_result=9
while [ $temp_time -lt $case_result ]
do
#cat template_tmp.csv | sed -e s/"	"/\"\,\"/  > template_tmp.csv
sed -ie s/'\t'/\"\,\"/ template_tmp.csv
let temp_time=$temp_time+1
done
sed -ie s/'\t'/';'/g template_tmp.csv
cat template_tmp.csv | sed s/$/'"'/ |  sed s/^/'"'/ > template.csv 
cat template_tmp.csv


