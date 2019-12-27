BEGIN { $| = 1; print "1..5\n"; }
END {print "not ok 1\n" unless $loaded;}
use PLSTAF;
$loaded = 1;
print "ok 1\n";

my $handle = PLSTAF::newHandle();
my $syncoption = PLSTAF::SyncReq();
my $result;
$_ = "ok";
PLSTAF::ProcRegister("Example", $handle) or $_ = "not ok";
print "$_ 2\n";
$_ = "ok";
$result = PLSTAF::ProcSubmit($handle, $syncoption, "LOCAL", "ping", "ping") or $_ = "not ok";
$_ = "not ok" unless ($result eq "PONG");
print "$_ 3\n";
$_ = "ok";
PLSTAF::ProcUnRegister($handle) or $_ = "not ok";
print "$_ 4\n";
$_ = "ok";
PLSTAF::delHandle($handle) or $_ = "not ok";
print "$_ 5\n";
