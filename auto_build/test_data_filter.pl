use XML::Simple;
use Data::Dumper;
use Spreadsheet::ParseExcel;
use Spreadsheet::ParseExcel::Workbook;
use Spreadsheet::WriteExcel ;
use Tie::IxHash;
use Getopt::Long; 
 use Spreadsheet::ParseExcel;
Getopt::Long::GetOptions(
 'input_excel_folder|i:s'=>\$input_excel_folder,
);


my @test_sheet;
my $row_max=0;
my $col_max=0;
my $parser   = Spreadsheet::ParseExcel->new();
my $workbook = $parser->parse("$input_excel_folder");
if ( !defined $workbook ) {
      die $parser->error(), ".\n";
}

for my $worksheet ( $workbook->worksheets("Test Case") ) {
#my $worksheet=$workbook->worksheets("Test Case");
               my ( $sheet_MinRow,$sheet_MaxRow ) = $worksheet->row_range();
               my ($sheet_MinCol, $sheet_MaxCol ) = $worksheet->col_range();
#print "sheet row:$sheet_MinRow--$sheet_MaxRow\n";
#print "sheet col:$sheet_MinCol--$sheet_MaxCol\n";
$row_max=$sheet_MaxRow;
$col_max=$sheet_MaxCol;
               for my $row ($sheet_MinRow .. $sheet_MaxRow ) {
                   for my $col ( $sheet_MinCol .. $sheet_MaxCol ) {

                       my $cell = $worksheet->get_cell( $row, $col );
                       next unless $cell;
                       $test_sheet[$row][$col]=$cell->value();
                   }
               }
	last;
}

my $finish_flag=9;
my $auto_flag=11;
my $test_data_path=21;
my $case_id=2;
my @test_data_sheet;
#print "$test_sheet[1][9] $test_sheet[1][11] $test_sheet[1][21] ";

for my $row(0..$row_max)
{
if ( "$test_sheet[$row][$finish_flag]" eq "finished" and "$test_sheet[$row][$auto_flag]" eq "Y")
{
if ( ! $test_sheet[$row][$test_data_path] )
{ $test_sheet[$row][$test_data_path]=NO_DATA; }
print "$test_sheet[$row][$case_id]|$test_sheet[$row][$test_data_path]\n";
}
}
 
