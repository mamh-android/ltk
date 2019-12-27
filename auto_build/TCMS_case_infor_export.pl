use LWP::UserAgent;
use HTTP::Cookies;
use HTTP::Headers;
use Data::Dumper;
use LWP::Simple;
use Getopt::Long; 
use XML::Simple;
# This perl script is used to download the Case information from TCMS ,
# The downloaded files are TCMS_Case_information.xml and bsp_test_case.xls
# The command line option 'folder_for_excel' is the destinationary folder for downloading 
Getopt::Long::GetOptions(
 'folder_for_excel:s'=>\$folder_for_excel,
  );
my$ua=new LWP::UserAgent(keep_alive =>1);
$ua->timeout(100);
$ua->agent('Mozilla/5.0');
$ua->cookie_jar(HTTP::Cookies->new(file=>'getTCMS.cookies',autosave=>1));
#Login TCMS
my$res=$ua->post("http://atf.marvell.com/opLogin.do?method=check",
[
userName=>"wsun",
password=>"marvell999",
projId=>"3",
],
);



if($res->is_success){
print Dumper($res);
print $res->decoded_content,"\n";
}
##########################################################################################################
my $is_success=0;
my $number=1;
until(($is_success eq 1)||($number  eq 10))
{
my$res=$ua->post('http://atf.marvell.com/case.do?method=list',
[
objectiveId=>'0',
recursive=>'Y',
control=>'0',
searchType=>'Y',
limit=>'0',
start=>'0',
],
);

if($res->is_success){
open(OUTFILE,"> $folder_for_excel/TCMS_Case_information.xml");
print OUTFILE  $res->decoded_content;
close OUTFILE;	
}
$is_success=$res->is_success; 
$number=$number+1;
}
##########################################################################################################
my $is_success=0;
my $number=1;
until(($is_success eq 1)||($number  eq 10))
{
my $res=$ua->post("http://atf.marvell.com/caseExport.do?method=generateExcel",
[
objLevel=>"1",
paramWithRecycle=>"false",
ext-comp-1241=>'3',
ext-comp-1242=>'ALL',
undefined=>,
undefined=>,
undefined=>,
undefined=>,
],
);
$is_success=$res->is_success;
if($res->is_success){
print Dumper($res);
print $res->decoded_content,"\n";
$res->decoded_content=~/{success:true,msg:'([\w\d_.-]+)'/;
print $1;
getstore("http://atf.marvell.com/caseExcel/$1","$folder_for_excel/bsp_test_case.xls");
}
$number=$number+1;
}



exit(0);


