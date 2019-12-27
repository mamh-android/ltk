use XML::Simple;
use Data::Dumper;
use Spreadsheet::ParseExcel;
use Spreadsheet::ParseExcel::Workbook;
use Spreadsheet::WriteExcel ;
use Tie::IxHash;
use Getopt::Long;
Getopt::Long::GetOptions(
 'input:s'=>\$input,
  );
#sub read_number_from_Case_information{
 my ($platform)=@_;
# my $file =  "$input/Case_information.xml";
my $file=$input;
if(-e $file)
{
 print $file;
 print "\n";
 my $xs1 = XML::Simple->new(); 
 my $doc = $xs1->XMLin($file);
 my @case_id;
 $doc=$doc->{'anon'}[0];
 $case_id=$doc;
}
else
{ 
return;
}
my @casepath_name;
my @casepath_name_number;
my $casepath_number=0;
my @test_plan;
my $row=0;


$server_IP="10.38.34.175";
$client_name="wsun";
$password="123456";
$data_base="ltk";
$table_name="TestCase";
`mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; create table $table_name (ID varchar(40), Type varchar(40), Domain varchar(40), Description varchar(255), LEVEL varchar(40));"`;

print "AAAAAA",$resultt,"\n";

foreach my $case_number(0..20000)
{  	
	#print "dd","\n";
	$ID='\''.$case_id->[$case_number]{'case_info'}{'case_id'}.'\'';
	$Type='\''.$case_id->[$case_number]{'case_info'}{'category'}.'\'';
	$Domain='\''.$case_id->[$case_number]{'case_info'}{'objective1'}.'\'';
	$Description='\''.$case_id->[$case_number]{'case_info'}{'test_point'}.'\'';
	$Description=~ s/\n//g;
	$LEVEL='\''.$case_id->[$case_number]{'case_info'}{'testlevel'}.'\'';
	print $ID,$Type,$Domain,$Description,$LEVEL,"\n";
	#print "mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; delete from $table_name where ID=$ID"","\n";
	`mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; delete from $table_name where ID=$ID"`;
	`mysql -h$server_IP -u$client_name -p$password -e "use $data_base ; insert into $table_name values($ID,$Type,$Domain,$Description,$LEVEL)"`;
	if ( ! $case_id->[$case_number]{'case_info'}{'case_id'} )
{
	print $case_number," case updated","\n";
	exit ;
}
}

print "update end , done!\n";

