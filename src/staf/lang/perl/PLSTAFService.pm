#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2004                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

package PLSTAFService;

use 5.008;
use strict;
use warnings;

use threads;
use threads::shared;
use Thread::Queue;

require Exporter;
require PLSTAF;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration       use PLSTAFService ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(

) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(

);

our $VERSION = '1.0';
XSLoader::load('PLSTAF', $VERSION);

# Preloaded methods go here.
package STAFServiceInterface;

use 5.008;
use strict;
use warnings;

use threads;
use threads::shared;
                                            
our $funcHash;                              # holds request handling subs
our $requests = Thread::Queue->new();       # holds requests for threads to grab
our $runAThread : shared = 0;               # signal variable
our $numAvailable: shared = 0;              # number of availabe threads
our $ConstructRequest = 1;                  # .
our $InitRequest = 2;                       # .
our $AcceptRequest = 3;                     # constants
our $TermRequest = 4;                       # .
our $DestructRequest = 5;                   # .

# This method gets called by perl services when they are ready to receive 
# requests.  It basically takes a request and passes it to the appropriate
# sub, creating a new thread to handle it if none are available.
sub start()
{
   my $type = shift;
   $funcHash = shift;
   my $serviceName = $funcHash->{serviceName};

   # set up an inbound STAF connection with the name $serviceName
   STAFPerlServiceHelper::initConnection($serviceName);
   
   my %hash : shared;
 
   while(1)
   {

      %hash = %{STAFPerlServiceHelper::getRequest()};

      my $result;
      my $rc = 0;
           
      if ($hash{"reqType"} == $ConstructRequest)
      {
         #$rc = &{ $funcHash->{construct} }();
         
         #STAFPerlServiceHelper::sendResults($hash{reqNum}, $rc, "OK");
         STAFPerlServiceHelper::sendResults($hash{reqNum}, 0, "");
      }
      elsif ($hash{"reqType"} == $InitRequest)
      {
         $rc = &{ $funcHash->{init} }(\%hash);
         
         STAFPerlServiceHelper::sendResults($hash{"reqNum"}, $rc, "");
      }
      elsif ($hash{"reqType"}==$AcceptRequest)
      {
         # XXX: I did this because Perl keeps on dying when I try to enqueue
         # \%hash.  Assigning \%hash to a scalar works for some reason
         my $sharedHash;
         $sharedHash = \%hash;
         
         $requests->enqueue($sharedHash);
         
         # If there are no available threads, create one.
         {
            lock($numAvailable);
            if($numAvailable == 0)
            {
               
               $numAvailable++;
               threads->new(\&requestLoop);
              
               
               #Debug:  If you want to keep track of 
               #        when new threads are created 
               #
               #print "Here are all current threads:\n";
               #for my $t (threads->list()) 
               #{
               #    printf "$t has tid = %d\n", $t->tid();
               #}

            }
         }
         
         # Reduce number of available threads and raise signal
         {
            lock($runAThread);
            lock($numAvailable);
            $numAvailable--;
            $runAThread = 1;
            cond_signal($runAThread);
         }  
      }
      elsif ($hash{"reqType"}==$TermRequest)
      {
         $rc = &{ $funcHash->{term} }(\%hash);

         STAFPerlServiceHelper::sendResults($hash{"reqNum"}, $rc, "");
         exit; # DMB hack to terminate the perl interpreter
               # DMB causes "A thread exited while 2 threads were running."
      }
      elsif ($hash{"reqType"}==$DestructRequest)
      {
         STAFPerlServiceHelper::sendResults($hash{"reqNum"}, 0, "");
      }
    }  
}

# XXX: TMG: Currently, Perl Interpreter Threads don't work particularly well
# with PLSTAF.dll and possibly all XS modules.  It would be much simpler to 
# have perl create a worker thread to handle every request, and have it detach
# from the worker thread. The worker would then be responsible for sending data 
# back the data to staf.  For now we're stuck doing things this way.

# This sub is the loop that the threads run while waiting for work
sub requestLoop {
   while(1)
   {
      my %hash;
      # wait for a signal to wake up
      {
         lock($runAThread);
         while($runAThread != 1)
         {
            cond_wait($runAThread);
         }
         $runAThread = 0;
      }

      # get the request
      %hash = %{$requests->dequeue()};
            
      # handle the request
      my $result = &{ $funcHash->{acceptRequest} }(\%hash);

      # send the results
      STAFPerlServiceHelper::sendResults($hash{"reqNum"}, $result->{rc}, $result->{result});
         
      #now that the thread is done doing it's work, let the parent thread know
      #that it is available again
      {
         lock($numAvailable);
         $numAvailable++;
      }
   }
}

package PLSTAFCommandParser;

sub new {
   my ($self, $obj) = @_;
   bless($obj, 'STAFCommandParser');

   return $obj;
}

package STAFCommandParser;

use constant VALUENOTALLOWED => scalar 0;
use constant VALUEALLOWED    => scalar 1;
use constant VALUEREQUIRED   => scalar 2;

sub parse {
    if ($#_ < 1)
    {
        die 'Usage: $result = STAFCommandParser->parse($request)';
    }

    my ($self, $request) = @_;

    my $obj = {};

    my $result = $self->parseRequest($request);

    $obj->{'rc'}          = $result->rc();
    $obj->{'errorBuffer'} = $result->errorBuffer();
    $obj->{'result'}      = $result;

    bless($obj, 'STAFCommandParseResult');

    return $obj;
}

package STAFCommandParseResult;

sub optionTimes {
    return shift->{'result'}->optionTimes(@_);
}

sub optionValue {
    return shift->{'result'}->optionValue(@_);
}

sub numInstances {
    return shift->{'result'}->numInstances(@_);
}

sub instanceName {
    return shift->{'result'}->instanceName(@_);
}

sub instanceValue {
    return shift->{'result'}->instanceValue(@_);
}

sub numArgs {
    return shift->{'result'}->numArgs(@_);
}

sub arg {
    return shift->{'result'}->arg(@_);
}

1;
__END__

