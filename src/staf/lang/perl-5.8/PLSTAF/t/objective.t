BEGIN { $| = 1; print "1..4\n"; }
END {print "not ok 1\n" unless $loaded;}
use PLSTAF;
$loaded = 1;
print "ok 1\n";

my $result;
$_ = "ok";
my $staf = PLSTAF->new("Objective Example") or $_ = "not ok";
print "$_ 2\n";
$_ = "ok";
#$staf->Register("Objective Example") or $_ = "not ok";
#print "$_ 3\n";
#$_ = "ok";
$staf->setSyncMode("sync") or $_ = "not ok";
print "$_ 3\n";
$_ = "ok";
$result = $staf->Submit("LOCAL", "ping", "ping") or $_ = "not ok";
$_ = "not ok" unless ($result eq "PONG");
print "$_ 4\n";
