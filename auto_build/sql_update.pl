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
my %test_case;

foreach my $case_number(0..20000)
{  	
	#print "dd","\n";
	$ID=$case_id->[$case_number]{'case_info'}{'case_id'};
	$casepath=$case_id->[$case_number]{'case_info'}{'casepath'};
	$test_case{$ID}{"casepath"}=$casepath;
	if ( ! $case_id->[$case_number]{'case_info'}{'case_id'} )
{
	print $case_number," case updated","\n";
	last ;
}
}

print $test_case{'TC_LCD_01'}{'casepath'};

