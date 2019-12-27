use XML::Simple;
use Data::Dumper;
#use Spreadsheet::WriteExcel ;
#use Spreadsheet::ParseExcel::Workbook;
#use Spreadsheet::ParseExcel;
use Tie::IxHash;
use Getopt::Long;
use Excel::Writer::XLSX;

my $title_col=9;
my $title_row=9;


Getopt::Long::GetOptions(
	'input_template_name|template:s'=>\$input_tmeplate_name,
	'input_info_name|info:s'=>\$input_info_name,
	'output_file_name|o:s'=>\$output_file_name,

);
#$format2->set_align('center');

if (! $input_tmeplate_name)
{
$input_tmeplate_name="template.csv";
}
if (! $input_info_name)
{
$input_info_name="testInfo.csv";
}
if (! $output_file_name)
{
$output_file_name="Test_result.xlsx";
}

   # my $workbook  = Spreadsheet::WriteExcel->new( 'chart_area.xls' );

my $workbook  = Excel::Writer::XLSX->new( $output_file_name );


my  %font_title1= (
	font =>'Arial',
	size => '15',
	color => 'black',
border => 2,
border_color => 'black',
);
my  %font_title2= (
	font =>'Arial',
	size => '13',
	color => 'black',
border => 1,
border_color => 'black',
);
my  %font_flag1= (
	font =>'Arial',
	size => '10',
	color => 'black',
border => 1,
border_color => 'black',
);
my  %font_normal= (
	font =>'Arial',
	size => '10',
	color => 'black',
      # diag_type   => '4',
      #  diag_border => '7',
border => 1,
border_color => 'black',
);


my %shading_title1 =(
	bg_color=> 'gray',
	pattern=>1,

);
my %shading_title2 =(
	bg_color=> 'silver',
	pattern=>1,
);
my %shading_pass =(
	bg_color=> 'green',
	pattern=>1,
);
my %shading_fail =(
	bg_color=> 'red',
	pattern=>1,
);
my %shading_block =(
	bg_color=> 'yellow',
	pattern=>1,
);
my %shading_oop =(
	bg_color=> 'gray',
	pattern=>1,
);
my %shading_no_test =(
	bg_color=> 'cyan',
	pattern=>1,
);
my %shading_normal =(
	bg_color=> 'white',
	pattern=>1,
);
my  $format_title1=$workbook->add_format(%font_title1,%shading_title1);
my  $format_title2=$workbook->add_format(%font_title2,%shading_title2);
my  $format_title3=$workbook->add_format(%font_title2,%shading_title2);
my  $format_title4=$workbook->add_format(%font_normal,%shading_title2);
my  $format_normal=$workbook->add_format(%font_normal,%shading_normal);
my  $format_normal_title=$workbook->add_format(%font_normal,%shading_normal);
my  $format_pass=$workbook->add_format(%font_flag1,%shading_pass);
my  $format_fail=$workbook->add_format(%font_flag1,%shading_fail);
my  $format_block=$workbook->add_format(%font_flag1,%shading_block);
my  $format_oop=$workbook->add_format(%font_flag1,%shading_oop);
my  $format_no_test=$workbook->add_format(%font_flag1,%shading_no_test);



$format_title1->set_bold();
$format_title2->set_bold();
$format_title3->set_bold();
$format_title4->set_bold();
$format_pass->set_bold();
$format_fail->set_bold();
$format_block->set_bold();
$format_oop->set_bold();
$format_no_test->set_bold();
$format_normal_title->set_bold();


my $worksheet = $workbook->add_worksheet('test_result');
$worksheet->set_row(0, 20);
$worksheet->set_row($title_col, 20);
$worksheet->set_row(1, 18);
$worksheet->set_column(0 ,0, 15);
$worksheet->set_column(1 ,1, 12);
$worksheet->set_column(2 ,2, 15);
$worksheet->set_column(3 ,3, 20);
$worksheet->set_column(4 ,4, 10);
$worksheet->set_column(9 ,9, 40);
$worksheet->merge_range(0,0,0,$title_row,"Linux BSP Validation Tracking Sheet",$format_title1);
$worksheet->merge_range(1,0,1,3,"Test Status",$format_title3);
#$worksheet->merge_range(3,0,3,1,"Test Result",$format_title3);
$worksheet->merge_range($title_col,0,$title_col,$title_row,"Test Checklist",$format_title3);
$worksheet->merge_range(1,4,$title_col-1,$title_row,"",$format_normal_title);

$worksheet->write(2,0,"Test Project",$format_title2);
$worksheet->write(3,0,"Test Result",$format_title2);
$worksheet->write(4,0,"PASS",$format_pass);
$worksheet->write(5,0,"FAIL",$format_fail);
$worksheet->write(6,0,"STOP",$format_block);
$worksheet->write(7,0,"TIME-OUT",$format_no_test);
$worksheet->write(8,0,"TOTAL",$format_normal_title);




$worksheet->write(3,3,"",$format_normal_title);
$worksheet->write(4,3,"",$format_normal_title);
$worksheet->write(5,3,"",$format_normal_title);
$worksheet->write(6,3,"",$format_normal_title);
$worksheet->write(7,3,"",$format_normal_title);
$worksheet->write(8,3,"",$format_normal_title);


########################################33
my @array2;
my @array;
my $template_file="$input_tmeplate_name";
open (my $in,"$template_file");
@array=<$in>;
@col_read_title=split (/\,/,$array[0]);
print $array[0];



my @template_data;
my %template_read;
for my $array_num(1..$#array)
{
	@col_read=split (/\",\"/,$array[$array_num]);

for my $a (0..$#col_read)
{
$template_data[$array_num-1][$a]=$col_read[$a];
$template_data[$array_num-1][$a]=~ s/"//;
}

	#$template_data[$array_num-1][0]=$col_read[0];
	#$template_data[$array_num-1][1]=$col_read[1];
	#$template_data[$array_num-1][2]=$col_read[2];	
	#@col_read=split (/"/,$array[$array_num]);
	#$template_data[$array_num-1][3]=$col_read[4];
	#@col_read1=split (/\,/,$col_read[2]);
	#for my $col_read_num(1..$#col_read1)
	#{
	#$template_data[$array_num-1][3+$col_read_num]=$col_read1[$col_read_num];
	#}
}
print "bbbbbbbbbbbbb11b","\n";
print Dumper(@template_data);
print $#template_data,"\n";


for my $array_num(0..$#template_data)
{
my $diff_id_time=1;
$diff_id=$template_data[$array_num][0];
for my $array_num1($array_num+1..$#template_data-1)
{
$diff_id1=$template_data[$array_num1][0];

if ( "$diff_id" eq "$diff_id1" )
{
$template_data[$array_num1][0]=$template_data[$array_num1][0]."@".$diff_id_time;
$diff_id_time=$diff_id_time+1;
}
}
}


for my $array_num(0..$#template_data)
{
	for $array_num1(0..$#col_read_title)
{
	$col_read_title1=$col_read_title[$array_num1+1];
	$col_read_title1=~ s/\"//g;
	$template_read{$template_data[$array_num][0]}{$col_read_title1}=$template_data[$array_num][$array_num1+1];
$template_read{$template_data[$array_num][0]}{$col_read_title1}=~ s/\"//g;
print "AAAAAAAAAAA","\n";
#print $template_data[0][0],"\n";
print $col_read_title1,"\n";
#print $template_data[0][1],"\n";
print "AAAAAAAAAAA","\n";
}
}



print "bbbbbbbbbbbbbb","\n";
print Dumper(%template_read);
#####################################################



########################################33
my $inf_flag=0;
my @array_inf;
my $inf_file="$input_info_name";
open (my $in,"$inf_file");
@array_inf=<$in>;
@col_read_inf_title=split (/\,/,$array_inf[0]);

my @inf_data;
my %inf_read;
for my $array_num(1..$#array_inf)
{
	@col_read=split (/\,/,$array_inf[$array_num]);
	for my $col_read_num(0..$#col_read)
	{
	$inf_data[$array_num-1][$col_read_num]=$col_read[$col_read_num];
	}
}


#print "YYYYYYYYYt","\n";
#print Dumper(@inf_data),"\n";
#print Dumper(@array_inf),"\n";

#print Dumper($template_data[0]);
print $#template_data,"\n";

for my $array_num(0..$#inf_data)
{
	for $array_num1(0..$#col_read_inf_title)
{
	#$col_read_inf_title1=$col_read_inf_title[$array_num1];
	$col_read_inf_title[$array_num1]=~ s/\"//g;
	$col_read_inf_title[$array_num1]=~ s/\n//g;
	$inf_data[$array_num][$array_num1]=~ s/\"//g;
	$inf_data[$array_num][$array_num1]=~ s/\n//g;
	$inf_read{$inf_flag}{$col_read_inf_title[$array_num1]}=$inf_data[$array_num][$array_num1];
	#print "YYYYYYYYY","\n";
	#print $col_read_inf_title[$array_num1]," ",$inf_data[$array_num][$array_num1];
}
	$inf_flag=$inf_flag+1;
}
#print "YYYYYYYYY","\n";
print Dumper($inf_read{'1'}{'Test Date'});
#####################################################



$worksheet->write(2,2,"Test Info",$format_title3);
$worksheet->write(2,3,"Cumulative Result",$format_title3);
$worksheet->write(3,2,"Test Board",$format_normal_title);
$worksheet->write(4,2,"Test Version",$format_normal_title);
$worksheet->write(5,2,"Test Type",$format_normal_title);
$worksheet->write(6,2,"Test Engineer",$format_normal_title);
$worksheet->write(7,2,"Test Date",$format_normal_title);
$worksheet->write(8,2,"LTK Version",$format_normal_title);

my $Board=$inf_read{0}{'Board'};
my $Version=$inf_read{0}{'Version'};
my $Type=$inf_read{0}{'Type'};
my $Engineer=$inf_read{0}{'Engineer'};
my $Date=$inf_read{0}{'Date'};
my $LTKVersion=$inf_read{0}{'LTKVersion'};
#my $LTKVersion=$inf_read{0}{'LTKVersion'};


for my $array_num(1..$#inf_data)
{
$Board=$Board.";"."$inf_read{$array_num}{'Board'}";
$Version=$Version.";"."$inf_read{$array_num}{'Version'}";
$Type=$Type.";"."$inf_read{$array_num}{'Type'}";
$Engineer=$Engineer.";"."$inf_read{$array_num}{'Engineer'}";
$Date=$Date.";"."$inf_read{$array_num}{'Date'}";
$LTKVersion=$LTKVersion.";"."$inf_read{$array_num}{'LTKVersion'}";
#$LTKVersion=$LTKVersion.";"."$inf_read{$array_num}{'LTKVersion'}";
}

$worksheet->write(3,3,"$Board",$format_normal);
$worksheet->write(4,3,"$Version",$format_normal);
$worksheet->write(5,3,"$Type",$format_normal);
$worksheet->write(6,3,"$Engineer",$format_normal);
$worksheet->write(7,3,"$Date",$format_normal);
$worksheet->write(8,3,"$LTKVersion",$format_normal);
$worksheet->write(2,1,"$inf_read{'0'}{'Project'}",$format_normal);




for my $write_title(0..$#col_read_title)
{
$col_read_title[$write_title]=~ s/\"//g;
$worksheet->write($title_col+1,$write_title,$col_read_title[$write_title],$format_title4);
}


my $result_category="EMEI beta2 sp1";
my $result_date="2012_03_13";
my $result_pass=0;
my $result_fail=0;
my $result_block=0;
my $result_no_test=0;




for  my $array_num(0..$#template_data)
{
$recover_id=$template_data[$array_num][0];
$recover_id=~ s/@[0-100]//g;
	$worksheet->write($title_col+2+$array_num,0,$recover_id,$format_normal);
	for my $array_num1(1..$#col_read_title)
{
	$col_read_title1=$col_read_title[$array_num1];
	$col_read_title1=~ s/\"//g;
	if ($col_read_title[$array_num1] eq "Status")
{
$test_status=$template_read{$template_data[$array_num][0]}{$col_read_title1};
if ($test_status eq "Pass" )
{
$result_pass=$result_pass+1;
$worksheet->write($title_col+2+$array_num,$array_num1,$template_read{$template_data[$array_num][0]}{$col_read_title1},$format_pass);
}
if ($test_status eq "Fail" )
{
$result_fail=$result_fail+1;
$worksheet->write($title_col+2+$array_num,$array_num1,$template_read{$template_data[$array_num][0]}{$col_read_title1},$format_fail);
}
if ($test_status eq "Stopped" )
{
$result_block=$result_block+1;
$worksheet->write($title_col+2+$array_num,$array_num1,$template_read{$template_data[$array_num][0]}{$col_read_title1},$format_block);
}
if ($test_status eq "Timeout" )
{
$result_no_test=$result_no_test+1;
$worksheet->write($title_col+2+$array_num,$array_num1,$template_read{$template_data[$array_num][0]}{$col_read_title1},$format_no_test);
}



}

else
{

	$worksheet->write($title_col+2+$array_num,$array_num1,$template_read{$template_data[$array_num][0]}{$col_read_title1},$format_normal);
}

}

}

my $result_total=$result_pass+$result_fail+$result_block+$result_no_test;
#$worksheet->write(2,1,$result_category,$format_normal);
#$worksheet->write(3,1,$result_date,$format_normal);
$worksheet->write(4,1,$result_pass,$format_normal);
$worksheet->write(5,1,$result_fail,$format_normal);
$worksheet->write(6,1,$result_block,$format_normal);
$worksheet->write(7,1,$result_no_test,$format_normal);
$worksheet->write(8,1,$result_total,$format_normal);


    my $chart = $workbook->add_chart( type => 'column',embedded => 1 );

    # Configure the first series. (Sample 1)
    $chart->add_series(
       name       => '=test_result!$B$3',
        categories => '=test_result!$A$5:$A$8',
        values     => '=test_result!$B$5:$B$8',
	#values     => ['test_result',4,1,7,1],
    );

 #$chart->set_legend( position => 'bottom' );

    # Add a chart title and some axis labels.
  #  $chart->set_title ( num_font => { name => 'Test_result', size => 10 } );
#$chart->set_title ( name => 'Test_result', num_font => { name => 'Arial', size => 5 }  );
    $chart->set_title(
        name      => 'Sales Results Chart',
        name_font => {
            name  => 'Calibri',
           # color => 'yellow',
		size => 10 ,
        },
    ); 

   #$chart->set_x_axis( name => 'Test result' );
    #$chart->set_y_axis( name => 'Test number' );
#$chart->set_style( 20 );
    # Insert the chart into the worksheet (with an offset).

    $worksheet->insert_chart( 1,4, $chart, 3, 3 ,0.15*8,0.53);


#$worksheet->write(22,1,"hello",$format_normal);





