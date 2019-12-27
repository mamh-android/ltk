package PLSTAF;

require 5.8.0;
use strict;
use warnings;
use English '-no_match_vars';

use Carp;
require DynaLoader;

our @ISA = qw(DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

our %EXPORT_TAGS = ( );

our @EXPORT_OK = ( );

our @EXPORT = ( );

our $VERSION = '1.3';

bootstrap PLSTAF $VERSION;

# Preloaded methods go here.

sub new() {
  my $proto = shift;
  my $name = shift;
  my $class = ref($proto) || $proto;
  my $self  = {};
  my $handle = PLSTAF::newHandle();
  my $syncmode = PLSTAF::SyncReq();
  $self->{SYNCNAME} = "Sync";
  $self->{SYNC}     = \$syncmode;
  $self->{HANDLE}   = \$handle;
  $name = "$PROGRAM_NAME ($PID)" unless defined $name and $name;
  $self->{NAME}     = $name;
  PLSTAF::ProcRegister($self->{NAME}, ${$self->{HANDLE}});
  bless ($self, $class);
  return $self;
}

#sub Register($) {
  #my $self = shift;
  #my $name = shift;
  #$self->{NAME} = $name;
  #if (!PLSTAF::ProcRegister($self->{NAME}, ${$self->{HANDLE}}))
  #{
    #$self->{NAME} = undef;
    #return 0;
  #}
  #return 1;
#}

sub setSyncMode() {
  my $self = shift;
  my $mode = shift;
  my $modeobj;
  local $_;
  foreach ($mode) {
    if ($_ =~ m/sync/i ) { $modeobj = PLSTAF::SyncReq(); $self->{SYNCNAME} = "Sync"; last; }
    if ($_ =~ m/fireandforget/i ) { $modeobj = PLSTAF::FireAndForgetReq(); $self->{SYNCNAME} = "Fire & Forget"; last; }
    if ($_ =~ m/queue/i ) { $modeobj = PLSTAF::QueueReq(); $self->{SYNCNAME} = "Queue"; last; }
    if ($_ =~ m/retain/i ) { $modeobj = PLSTAF::RetainReq(); $self->{SYNCNAME} = "Retain"; last; }
    if ($_ =~ m/queueretain/i ) { $modeobj = PLSTAF::QueueRetainReq(); $self->{SYNCNAME} = "Queue Retain"; last; }
    return 0;
  }
  $self->{SYNC} = \$modeobj;
  return 1;
}

sub Submit() {
  my $self= shift;
  my $host= shift;
  my $service= shift;
  my $command= shift;
  return PLSTAF::ProcSubmit(${$self->{HANDLE}}, ${$self->{SYNC}}, $host, $service, $command);
}

sub DESTROY() {
  my $self = shift;
  if (defined $self->{NAME}) { PLSTAF::ProcUnRegister(${$self->{HANDLE}});}
  PLSTAF::delHandle(${$self->{HANDLE}});
}
  

1;
#$staf->Register("Simple STAF Test");
__END__
=head1 NAME

PLSTAF - Perl module to access PLSTAF functionality

=head1 SYNOPSIS

  use PLSTAF;
  my $staf = PLSTAF->new("Simple STAF Test");
  $staf->setSyncMode("sync");
  my $result = $staf->Submit("LOCAL", "ping", "ping");
  print $result, "\n";

=head1 DESCRIPTION

The PLSTAF perl module allows you to access the functionality of STAF from
within Perl, including STAF services written in Perl.

=head1 EXPORT

None.

=head1 USAGE

=head2 Build a new PLSTAF Object

Step one is to build a PLSTAf object, the easiest way to do this is with:

  my $staf = PLSTAF->new("Name of your program")
    or die "Could not create a STAF object";

You may omit the name argument, in which case it defaults to:

  use English;
  $name = "$PROGRAM_NAME ($PID)";

If you do set your own, you may find that embedding the PID is useful on
occasion.

=head2 Set the Synchronisation mode

All requests to STAF have a synchronisation mode -- you set this like
so:

  $staf->setSyncMode("sync") or die "Could not set sync mode";

Valid values are "sync", "fireandforget", "queue", "retain", and
"queueretain".  See the main STAF docs for more on these sync modes.

The default mode is "sync", so you may not need to set it.

=head2 Send requests and receive replies

Requests are submitted like this:

  $reply = $staf->Submit($host, $service, $request)
    or warn "Problem submitting request";

Where $host is the host to send the request to, $service is the service
that will handle the request, and $request is the request to the
service.

Any reply received is returned.

=head1 AUTHOR

Philip Graham Willoughby L<pwilloug@uk.ibm.com>

=head1 SEE ALSO

L<perl(1)> L<http://staf.sourceforge.net/>

=cut
