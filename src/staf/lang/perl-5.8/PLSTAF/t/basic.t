BEGIN { $| = 1; print "1..5\n"; }
END {print "not ok 1\n" unless $loaded;}
use PLSTAF;
$loaded = 1;
print "ok 1\n";

PLSTAF::basictest();
print " 2\n";
$_ = PLSTAF::returntest() or $_ = "not ok";
print "$_ 3\n";
PLSTAF::argtest(4);
@_ = PLSTAF::listandargtest(5) or @_ = ("not ok", 5);
print join(" ",@_), "\n";
