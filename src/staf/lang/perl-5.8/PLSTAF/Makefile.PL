use ExtUtils::MakeMaker;

eval { require 5.8.0 }
  or die <<EOF;
#################################################################################
# Production releases of Perl prior to version 5.8.0 lack a reentrant
# regex engine.  It is extremely dangerous to use these versions of Perl
# with STAF.  I suggest you update your version of Perl before
# proceeding.
#
################################################################################
EOF

sub MY::test {
  package MY;
  my $inherited = shift->SUPER::test(@_);
  $inherited =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/;
  $inherited;
}

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
ExtUtils::MakeMaker::WriteMakefile(
    'NAME'		=> 'PLSTAF',
    'VERSION_FROM'	=> 'PLSTAF.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    'LIBS'		=> ['-L/usr/local/staf/lib -L/opt/staf/lib -LC:\\STAF\\lib -lSTAF -lpthread -lstream -lstd -lCsup -lm -ldld -ldl'], # e.g., '-lm'
    'INC'		=> '-I/usr/local/staf/include -I/opt/staf/include -IC:\\STAF\\include',
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
);
