#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###############################################################################
# THIS MODULE IS REQUIRED BY ALL PERL APPS THAT WANT TO INTERACT WITH STAF    #
# AND SHOULD NOT BE MODIFIED.                                                 #
###############################################################################

package PLSTAF;

use 5.006;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration       use PLSTAF ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
        
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
        
);

our $VERSION = '3.0';

require XSLoader;
XSLoader::load('PLSTAF', $VERSION);

sub STAF::Submit2
{
    if ($#_ < 3)
    {
        die "Usage: STAF::Submit2(SYNCOPTION, MACHINE, SERVICE, REQUEST)\n";
    }

    my $syncoption = shift;

    return STAF::Submit(shift, shift, shift, $syncoption);
}

sub STAF::STAFHandle::new
{
    if ($#_ < 1)
    {
        die 'Usage: $handle = STAFHandle->new(HANDLENAMEORNUMBER' .
            '[, HANDLETYPE])';
    }
    
    my $type = shift;
    my $handleNameOrNumber = shift;
    my $self = {};
    $self->{doUnmarshallResult} = 1;
    
    if (@_)
    {
        # The STAF handle type argument was specified

        $self->{handleType} = shift;

        # Verify that the handleType is either 0 or 1

        if (!($self->{handleType} =~ /^[0-1]$/))
        {
            die "\nError: HANDLETYPE must be 0 (Standard Handle) or 1 " .
                "(Static Handle)\n" .
                'Usage: $handle = STAFHandle->new(HANDLENAMEORNUMBER[' .
                ", HANDLETYPE])\n";
        }
    }
    else
    {
        # The STAF handle type defaults to standard

        $self->{handleType} = $STAF::STAFHandle::kStandard;
    }

    if ($self->{handleType} == $STAF::STAFHandle::kStandard)
    {
        # Standard handle type - Create a new STAF handle

        $self->{rc}     = STAF::Register($handleNameOrNumber);
        $self->{handle} = $STAF::Handle;
    }
    else
    {
        # Static handle type - Assign the handle number

        if (!($handleNameOrNumber =~ /^\d+$/))
        {
            die "\nError: HANDLENAMEORNUMBER must contain the number of an " .
                "existing static handle\n" .
                'Usage: $handle = STAFHandle->new(HANDLENAMEORNUMBER[' .
                ", HANDLETYPE])\n";
        }
        
        $self->{rc}     = $STAF::kOk;
        $self->{handle} = $handleNameOrNumber;
    }

    return bless $self, $type;
}

sub STAF::STAFHandle::submit
{
    if ($#_ < 3)
    {
        die 'Usage: $handle->submit(MACHINE, SERVICE, REQUEST)';
    }

    my $self = shift;
    $STAF::Handle = $self->{handle};

    $self->{rc} = STAF::Submit(shift, shift, shift);

    return STAF::STAFResult->new($STAF::RC, $STAF::Result,
                                 $self->{doUnmarshallResult});
}

sub STAF::STAFHandle::submit2
{
    if ($#_ < 4)
    {
        die 'Usage: $handle->submit2(SYNCOPTION, MACHINE, SERVICE, REQUEST)';
    }

    my $self = shift;
    $STAF::Handle = $self->{handle};

    $self->{rc} = STAF::Submit2(shift, shift, shift, shift);

    return STAF::STAFResult->new($STAF::RC, $STAF::Result,
                                 $self->{doUnmarshallResult});
}

sub STAF::STAFHandle::unRegister
{
    if ($#_ < 0)
    {
        die 'Usage: $handle->unRegister()';
    }

    my $self = shift;
    $STAF::Handle = $self->{handle};

    $self->{rc} = STAF::UnRegister();

    return $STAF::RC;
}

sub STAF::STAFHandle::setDoUnmarshallResult
{
   if ($#_ < 1)
   {
       die 'Usage: $handle->setDoUnmarshallResult(doUnmarshallResult)';
   }

   my $self = shift;

   $self->{doUnmarshallResult} = shift;
}

sub STAF::STAFHandle::getDoUnmarshallResult
{
   my $self = shift;

   return $self->{doUnmarshallResult};
}

sub STAF::STAFResult::new
{
    if ($#_ < 2)
    {
        die "Usage: STAFResult->new(RC, RESULT[, doUnmarshallResult])\n";
    }

    my $type   = shift;
    my $self   = {};

    $self->{rc}     = shift;
    $self->{result} = shift;

    if (@_ && $_[0])
    {
        $self->{resultContext} = STAF::STAFUnmarshall($self->{result});
        $self->{resultObj} = $self->{resultContext}->getRootObject();
    }
    else
    {
        $self->{resultContext} = undef;
        $self->{resultObj} = undef;
    }

    return bless $self, $type;
}

sub STAF::WrapData
{
    if ($#_ < 0)
    {
        die "Usage: STAF::WrapData(MESSAGE)\n";
    }

    my $text = shift;
    return ":".length($text).":$text";
}

sub STAF::STAFUtil::WrapData
{
    if ($#_ < 0)
    {
        die "Usage: STAFUtil::WrapData(MESSAGE)\n";
    }

    my $text = shift;
    return ':'.length($text).":$text";
}

################################################################################

sub STAF::STAFMapClassDefinition::new
{
    my $type = shift;
    my $self = {};

    $self->{'keys'} = [];
    $self->{'name'}  = shift;

    return bless $self, $type;
}

sub STAF::STAFMapClassDefinition::_new
{
    my $type = shift;
    my $self = shift;

    return bless $self, $type;
}

sub STAF::STAFMapClassDefinition::name
{
    my $self = shift;

    return $self->{'name'};
}

sub STAF::STAFMapClassDefinition::createInstance
{
    my $self = shift;

    my $instance = { 'staf-map-class-name' => $self->{'name'} };
    return $instance;
}

sub STAF::STAFMapClassDefinition::addKey
{
    my $self = shift;
    my %key = ( 'key' => shift );

    $key{'display-name'} = shift if (@_);

    push @{$self->{'keys'}}, \%key;
}

sub STAF::STAFMapClassDefinition::setKeyProperty
{
    my $self = shift;
    my $keyName = shift;
    my $keyProperty = shift;
    my $keyValue = shift;

    foreach my $key ($self->keys())
    {
        $key->{$keyProperty} = $keyValue if ($key->{'key'} eq $keyName);
    }
}

sub STAF::STAFMapClassDefinition::keys
{
    my $self = shift;

    return @{$self->{'keys'}};
}

sub STAF::STAFMarshallingContext::new
{
    my $type = shift;
    my $self = { mapClassMap => {}, rootObject => undef };

    return bless $self, $type;
}

sub STAF::STAFMarshallingContext::setMapClassDefinition
{
    my $self = shift;
    my $mapClassDef = shift;

    $self->{'mapClassMap'}{$mapClassDef->name()} = $mapClassDef;
}

sub STAF::STAFMarshallingContext::getMapClassDefinition
{
    my $self = shift;
    my $mapClassDefName = shift;

    if (exists($self->{'mapClassMap'}{$mapClassDefName}))
    {
        return STAF::STAFMapClassDefinition->_new(
                   $self->{'mapClassMap'}{$mapClassDefName});
    }
    else
    {
        return STAF::STAFMapClassDefinition->new(undef);
    }
}

sub STAF::STAFMarshallingContext::hasMapClassDefinition
{
    my $self = shift;
    my $mapClassDefName = shift;

    return exists($self->{'mapClassMap'}{$mapClassDefName});
}

sub STAF::STAFMarshallingContext::getMapClassMap
{
    my $self = shift;

    return $self->{'mapClassMap'};
}

sub STAF::STAFMarshallingContext::mapClassDefinitionList()
{
    my $self = shift;

    return sort keys %{$self->{'mapClassMap'}};
}

sub STAF::STAFMarshallingContext::setRootObject
{
    my $self = shift;

    $self->{'rootObject'} = shift;
}

sub STAF::STAFMarshallingContext::getRootObject
{
    my $self = shift;

    return $self->{'rootObject'};
}

sub STAF::STAFMarshallingContext::getPrimaryObject()
{
    my $self = shift;

    return (keys %{$self->{'mapClassMap'}} > 0) ? $self : $self->{'rootObject'};
}

sub STAF::STAFMarshallingContext::marshall()
{
    my $self = shift;

    return STAF::STAFMarshall($self, $self);
}

sub STAF::STAFMarshallingContext::formatObject()
{
    my $self = shift;

    return STAF::STAFFormatObject($self->{'rootObject'}, $self);
}

sub STAF::STAFIsMarshalledData
{
    return shift =~ /\@SDT\//;
}

# Marshalling constants

use constant {
    MARSHALLED_DATA_MARKER => '@SDT/',
    NONE_MARKER            => '@SDT/$0:0:',
    SCALAR_MARKER          => '@SDT/$',
    SCALAR_STRING_MARKER   => '@SDT/$S',
    LIST_MARKER            => '@SDT/[',
    MAP_MARKER             => '@SDT/{',
    MC_INSTANCE_MARKER     => '@SDT/%',
    CONTEXT_MARKER         => '@SDT/*'
};

# Formatting constants

use constant {
    NONE_STRING            => '<None>',
    ENTRY_SEPARATOR        => '',
    INDENT_DELTA           => 2,
    LINE_SEP               => "\n"
};

# Marshalling function
#
# Syntax:
#   STAFMarshall(object[, context])
#
# Description:
#   This function is used to create a string-based marshalled representation
#   of the object specified by the argument 'object'.
# 
# Input arguments:
# - The required argument 'object' is the data structure to be marshalled. 
# - The optional argument 'context' specifies the STAFMarshallingContext
#   object that should be used when marshalling the string.
#   The default is None.
#
# Returns:
#   Returns a marshalled string.

sub STAF::STAFMarshall
{
    if (@_ < 1)
    {
        die "Syntax:  STAF::STAFMarshall(object[, context])\n";
    }
    
    my $object = shift;
    my $context;
    
    if (@_ && (ref($_[0]) eq 'STAF::STAFMarshallingContext'))
    {
        $context = shift;
    }
    else
    {
        $context = STAF::STAFMarshallingContext->new();
    }
    
    if (!defined $object)
    {
        return NONE_MARKER;
    }
    elsif (!ref($object))
    {
        # Simple scalar

        return SCALAR_STRING_MARKER . STAF::WrapData($object);
    }
    elsif (UNIVERSAL::isa($object, 'SCALAR'))
    {
        # Reference to a scalar.  Marshall the dereferenced scalar.

        my $m = STAF::STAFMarshall($$object, $context);

        return SCALAR_STRING_MARKER . STAF::WrapData($m);
    }
    elsif (UNIVERSAL::isa($object, 'ARRAY'))
    {
        # Array reference (aka List)

        # @SDT/[<number-of-items>:<array-length>:<SDT-Any-1>...<SDT-Any-n>

        my @list = map { STAF::STAFMarshall($_, $context) } @$object;
        my $listData = join('', @list);

        return LIST_MARKER . scalar(@list).STAF::WrapData($listData);
    }
    elsif (UNIVERSAL::isa($object, 'STAF::STAFMarshallingContext'))
    {
        # Marshalling Context
        
        if (scalar(keys %{$object->getMapClassMap()}) == 0)
        {
            return STAF::STAFMarshall($object->getRootObject(), $context);
        }
        else
        {
            my $contextMap = {};
            $contextMap->{'map-class-map'} = $object->getMapClassMap();

            # Note: We can't simply put the root object as a map key like
            # "root-object" and then marshall the whole map, as in the
            # unmarshalling routines, we need to be able to unmarshall the
            # root object in the context of the map-class-map.

            my $mcData = STAF::STAFMarshall($contextMap, $context) .
                         STAF::STAFMarshall($object->getRootObject(), $object);

            return CONTEXT_MARKER . STAF::WrapData($mcData);
        }
    }
    elsif (UNIVERSAL::isa($object, "HASH"))
    {
        # Hash reference (aka Map)

        # If a staf-map-class-name key exists in the map, make sure that
        # it's map class definition is provided in the marshalling context.
        # If it's not, then treat the map as a plain map object.

        my $mapData = '';
        
        if (exists($object->{'staf-map-class-name'}) &&
            $context->hasMapClassDefinition($object->{'staf-map-class-name'}))
        {
            # Map class object

            #@SDT/%:<map-class-instance-length>::<map-class-name-length>:<map-class-name>
            #    <SDT-Any-value-1>
            #    ...
            #    <SDT-Any-value-n>

            my $mapClassName = $object->{'staf-map-class-name'};
            my $mapClassDef = $context->getMapClassDefinition($mapClassName);

            $mapData = STAF::WrapData($mapClassName);
            
            foreach my $keyMap ($mapClassDef->keys())
            {
                my $thisObj;

                if (exists($object->{$keyMap->{'key'}}))
                {
                    $thisObj = $object->{$keyMap->{'key'}};
                }
                else
                {
                    $thisObj = NONE_MARKER;
                }

                $mapData .= STAF::STAFMarshall($thisObj, $context);
            }

            return MC_INSTANCE_MARKER . STAF::WrapData($mapData);
        }
        else
        {
            # Plain map object

            # @SDT/{:<map-length>::<key-1-length>:<key-1><SDT-Any>
            #                     ...
            #                     :<key-n-length>:<key-1><SDT-Any>

            while (my ($key, $value) = each %$object)
            {
                $mapData .= STAF::WrapData($key) .
                            STAF::STAFMarshall($value, $context);
            }

            return MAP_MARKER . STAF::WrapData($mapData);
        }
    }
    else
    {
        # Unrecognized data type.  Ues "stringified" data.

        return SCALAR_STRING_MARKER . STAF::WrapData('' . $object);
    }
}

# Unmarshalling function
#
# Syntax:
#   STAFUnmarshall(data [, context, [, flags]])
#
# Description:
#   This function is used to convert a string-based marshalled representation
#   specified by the argument 'data' back into a data structure.
# 
# Input arguments:
# - The required argument 'data' is a string to be unmarshalled. 
# - The optional argument 'context' specifies the STAFMarshallingContext
#   object that should be used when unmarshalling the string.
#   The default is None.
# - The optional argument 'flags' can be used to control how to unmarshall
#   the string. When a string is unmarshalled into a data structure, it is
#   possible that one of the string objects that is unmarshalled is itself
#   the string form of another marshalled data structure.
#   The default when no flags are specified is to recursively unmarshall
#   these nested objects. To disable this additional processing, specify
#   IGNORE_INDIRECT_OBJECTS=>1 for the flags argument. 
#
# Returns:
#   A STAFMarshallingContext object (from which you can get the data structure
#   via the STAFMarshallingContext classs's getRootObject() function). 

sub STAF::STAFUnmarshall
{
    if (@_ < 1)
    {
        die "Syntax:  STAF::STAFUnmarshall(data[, context[, flags]])\n";
    }

    my $data = shift;
    my $inputContext;
    
    if (@_ && (ref($_[0]) eq 'STAF::STAFMarshallingContext'))
    {
        $inputContext = shift;
    }
    else
    {
        $inputContext = STAF::STAFMarshallingContext->new();
    }

    my %parms = (@_);
    
    my $context = STAF::STAFMarshallingContext->new();

    # @SDT<type><typeInfo>:<length>:<data>

    if (!defined $data) {
        # Just set it to the root object

        $context->setRootObject($data);
    }
    elsif (substr($data, 0, length(NONE_MARKER)) eq NONE_MARKER)
    {
        # Nothing else need to do    
    }
    elsif (substr($data, 0, length(SCALAR_MARKER)) eq SCALAR_MARKER)
    {
        # @SDT/$S:<string-length>:<String>

        my $startIndex = index($data, ':') + 1;

        if ($startIndex <= 0)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $colonIndex = index($data, ':', $startIndex);

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $stringLength = substr($data, $startIndex, $colonIndex - $startIndex);

        if (!($stringLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        my $dataIndex = $colonIndex + 1;

        if ($stringLength != (length($data) - $dataIndex))
        {
            $context->setRootObject($data);
            return $context;
        }

        my $theString = substr($data, $dataIndex);

        if ((substr($theString, 0, length(MARSHALLED_DATA_MARKER)) eq
             MARSHALLED_DATA_MARKER) &&
            (!exists($parms{IGNORE_INDIRECT_OBJECTS}) ||
             !$parms{IGNORE_INDIRECT_OBJECTS}))
        {
            $context = STAF::STAFUnmarshall(
                $theString, $inputContext, %parms);
        }
        else
        {
            $context->setRootObject($theString);
        }
    }
    elsif (substr($data, 0, length(LIST_MARKER)) eq LIST_MARKER)
    {
        # @SDT/[<number-of-items>:<array-length>:<SDT-Any-1>...<SDT-Any-n>

        # Get number-of-items in the list

        my $startIndex = length(LIST_MARKER);
        my $colonIndex = index($data, ':');

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $numItems = substr($data, $startIndex, $colonIndex - $startIndex);

        if (!($numItems =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        # Get array-length

        my $colonIndex2 = index($data, ':', $colonIndex + 1);

        if ($colonIndex2 == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $arrayLength = substr($data, $colonIndex + 1, $colonIndex2 - $colonIndex - 1);

        if (!($arrayLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        my $dataIndex = $colonIndex2 + 1;
        
        if ($arrayLength != (length($data) - $dataIndex))
        {
            $context->setRootObject($data);
            return $context;
        }

        # Create an array (aka list) of the data

        my @theList = ();

        for (my $i = 0; $i < $numItems; $i++) {

            # Get the next item in the list and unmarshall it and add it to
            # the list

            $startIndex = index($data, ':', $dataIndex) + 1;

            if ($startIndex <= 0)
            {
                $context->setRootObject($data);
                return $context;
            }

            my $endIndex = index($data, ':', $startIndex);

            if ($endIndex == -1)
            {
                $context->setRootObject($data);
                return $context;
            }

            my $itemLength = substr($data, $startIndex,
                                    $endIndex - $startIndex);

            if (!($itemLength =~ /^[0-9]+$/))
            {
                $context->setRootObject($data);
                return $context;
            }

            $endIndex = $endIndex + 1 + $itemLength;

            if ($dataIndex >= length($data))
            {
                $context->setRootObject($data);
                return $context;
            }

            push @theList,
                 STAF::STAFUnmarshall(
                     substr($data, $dataIndex, $endIndex - $dataIndex),
                     $inputContext, %parms)->getPrimaryObject();

            $dataIndex = $endIndex;
        }

        $context->setRootObject(\@theList);
    }
    elsif (substr($data, 0, length(MAP_MARKER)) eq MAP_MARKER)
    {
        # @SDT/{:<map-length>::<key-1-length>:<key-1><SDT-Any>
        #                     ...
        #                     :<key-n-length>:<key-1><SDT-Any>

        # Get the map-length

        my $startIndex = index($data, ':') + 1;

        if ($startIndex <= 0)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $endIndex = index($data, ':', $startIndex);

        if ($endIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $mapLength = substr($data, $startIndex, $endIndex - $startIndex);

        if (!($mapLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        my $dataIndex = $endIndex + 1;
        
        if ($mapLength != (length($data) - $dataIndex))
        {
            $context->setRootObject($data);
            return $context;
        }
        
        my %theMap = ();

        while ($dataIndex < length($data))
        {
            # Get the key first

            my $colonIndex = index($data, ':', $dataIndex + 1);

            if ($colonIndex == -1)
            {
               $context->setRootObject($data);
               return $context;
            }

            my $keyLength = substr(
                $data, $dataIndex + 1, $colonIndex - $dataIndex - 1);

            if (!($keyLength =~ /^[0-9]+$/))
            {
                $context->setRootObject($data);
                return $context;
            }

            my $key = substr($data, $colonIndex + 1, $keyLength);

            $dataIndex = $colonIndex + 1 + $keyLength;

            if ($dataIndex >= length($data))
            {
                $context->setRootObject($data);
                return $context;
            }

            # Now, get the object and add the key and unmarshalled object
            # to the map

            my $colonIndex1 = index($data, ':', $dataIndex);

            if ($colonIndex1 == -1)
            {
                $context->setRootObject($data);
                return $context;
            }

            my $colonIndex2 = index($data, ':', $colonIndex1 + 1);
            
            if ($colonIndex2 == -1)
            {
                $context->setRootObject($data);
                return $context;
            }

            my $itemLength = substr($data, $colonIndex1 + 1,
                                    $colonIndex2 - $colonIndex1 - 1);

            if (!($itemLength =~ /^[0-9]+$/))
            {
                $context->setRootObject($data);
                return $context;
            }

            $theMap{$key} = STAF::STAFUnmarshall(
                substr($data, $dataIndex,
                       $colonIndex2 + $itemLength + 1 - $dataIndex),
                $inputContext, %parms)->getPrimaryObject();

            $dataIndex = $colonIndex2 + $itemLength + 1;
        }

        $context->setRootObject(\%theMap);
    }
    elsif (substr($data, 0, length(MC_INSTANCE_MARKER)) eq MC_INSTANCE_MARKER)
    {
        #@SDT/%:<map-class-instance-length>::<map-class-name-length>:<map-class-name>
        #    <SDT-Any-value-1>
        #    ...
        #    <SDT-Any-value-n>

        # Get the map-class-instance-length

        my $colonIndex = index($data, ':');

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $colonIndex2 = index($data, ':', $colonIndex + 1);

        if ($colonIndex2 == -1)
        {
            $context->setRootObject($data);
            return $context;
        }
        
        my $startIndex = $colonIndex + 1;

        my $mapClassInstanceLength = substr($data, $startIndex, $colonIndex2 - $startIndex);

        if (!($mapClassInstanceLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        my $dataIndex = $colonIndex2 + 1;

        if ($mapClassInstanceLength != (length($data) - $dataIndex))
        {
            $context->setRootObject($data);
            return $context;
        }

        # Get the map-class-name-length

        $colonIndex = index($data, ':', $dataIndex);

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        $colonIndex2 = index($data, ':', $colonIndex + 1);

        if ($colonIndex2 == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $mapClassNameLength = substr($data, $colonIndex + 1,
                                        $colonIndex2 - $colonIndex - 1);

        if (!($mapClassNameLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        # Get map-class-name

        my $mapClassName = substr($data, $colonIndex2 + 1,
                                  $mapClassNameLength);

        $dataIndex = $colonIndex2 + 1 + $mapClassNameLength;

        # Add the map class name to the map

        my %theMap = ();

        $theMap{'staf-map-class-name'} = $mapClassName;

        if (!$inputContext->hasMapClassDefinition($mapClassName))
        {
            print "Oops, map class '$mapClassName' doesn't exist\n";
        }

        # Unmarshall all of the actual keys and add to the map keys hash

        my $mapClassDef = $inputContext->getMapClassDefinition($mapClassName);
        my @keys = $mapClassDef->keys();
        my $keyIndex = 0;

        while ($dataIndex < length($data))
        {
            $colonIndex = index($data, ':', $dataIndex);

            if ($colonIndex == -1)
            {
                $context->setRootObject($data);
                return $context;
            }

            $colonIndex2 = index($data, ':', $colonIndex + 1);

            if ($colonIndex2 == -1)
            {
                $context->setRootObject($data);
                return $context;
            }

            my $itemLength = substr($data, $colonIndex + 1,
                                    $colonIndex2 - $colonIndex - 1);

            if (!($itemLength =~ /^[0-9]+$/))
            {
                $context->setRootObject($data);
                return $context;
            }

            if ($dataIndex >= length($data))
            {
                $context->setRootObject($data);
                return $context;
            }

            if (($colonIndex2 + $itemLength + 1 - $dataIndex) > length($data))
            {
                $context->setRootObject($data);
                return $context;
            }

            $theMap{$keys[$keyIndex++]{'key'}} = STAF::STAFUnmarshall(
                substr($data, $dataIndex,
                       $colonIndex2 + $itemLength + 1 - $dataIndex),
                $inputContext, %parms)->getPrimaryObject();

            $dataIndex = $colonIndex2 + $itemLength + 1;
        }

        $context->setRootObject(\%theMap);
    }
    elsif (substr($data, 0, length(CONTEXT_MARKER)) eq CONTEXT_MARKER)
    {
        # @SDT/*:<context-length>:@SDT/{:<mapClassLength>:<mapClassData>
        #                         <rootObject>

        # Get context-length

        my $colonIndex = index($data, ':');

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $contextLengthIndex = $colonIndex + 1;

        $colonIndex = index($data, ':', $contextLengthIndex);

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $contextLength = substr($data, $contextLengthIndex,
                                   $colonIndex - $contextLengthIndex);

        if (!($contextLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }
        
        my $contextIndex = $colonIndex + 1;

        # Get mapClassLength

        $colonIndex = index($data, ':', $contextIndex);

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $mapIndex = $contextIndex;
        my $mapDataIndex = index($data, ':', $colonIndex + 1);

        if ($mapDataIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $mapLength = substr($data, $colonIndex + 1,
                               $mapDataIndex - $colonIndex - 1);

        if (!($mapLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }
        
        $mapDataIndex = $mapDataIndex + 1;

        if ($mapLength > (length($data) - $mapDataIndex))
        {
            $context->setRootObject($data);
            return $context;
        }

        # Create a new marshalling context with the map classes and root object

        my $contextMap = STAF::STAFUnmarshall(
            substr($data, $mapIndex, $mapDataIndex + $mapLength - $mapIndex),
            $context, %parms);

        $context->{'mapClassMap'} =
            \%{$contextMap->getRootObject()->{'map-class-map'}};

        my $rootObjIndex = $mapDataIndex + $mapLength;
        $colonIndex = index($data, ':', $rootObjIndex);

        if ($colonIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $rootObjDataIndex = index($data, ':', $colonIndex + 1);

        if ($rootObjDataIndex == -1)
        {
            $context->setRootObject($data);
            return $context;
        }

        my $rootObjLength = substr($data, $colonIndex + 1,
                                   $rootObjDataIndex - 1 - $colonIndex);

        if (!($rootObjLength =~ /^[0-9]+$/))
        {
            $context->setRootObject($data);
            return $context;
        }

        $rootObjDataIndex = $rootObjDataIndex + 1;

        if ($rootObjLength > (length($data) - $rootObjDataIndex))
        {
            $context->setRootObject($data);
            return $context;
        }

        $context->setRootObject(
            STAF::STAFUnmarshall(
                substr($data, $rootObjIndex,
                       $rootObjDataIndex + $rootObjLength - $rootObjIndex),
                $context, %parms)->getPrimaryObject());
    }
    else
    {
        # We don't know what this data is, so just set it to the root object

        $context->setRootObject($data);
    }

    return $context;
}

# Formatting function
#
# Syntax:
#   STAFFormatObject(object[, context[, indentLevel]])
#
# Description:
#  This function is used to convert a data structure into a verbose formatted
#  hierarchical string that can be used when you want a "pretty print"
#  representation of an object. 
#
# Arguments:
# - The required argument 'obj' specifies the object to be formatted in a
#   verbose, more readable format. 
# - The optional argument 'context' specifies the STAFMarshallingContext
#   object that should be used when generating the "pretty print" output.
#   The default is None. 
# - The indentLevel option is not meant to be used by a user calling
#   the STAFFormatObject function, thus we don't document it exernally.
#   It's meant to be used internally by the STAFFormatObject method when
#   recursively calling itself.
#
# Returns:
#   A string containing the "pretty print" representation of an object

sub STAF::STAFFormatObject
{
    if (@_ < 1)
    {
        die "Syntax:  STAFFormatObject(object[, context])\n";
    }
    
    my $object = shift;
    my $context;
    
    if (@_ && (ref($_[0]) eq 'STAF::STAFMarshallingContext'))
    {
        $context = shift;
    }
    else
    {
        $context = STAF::STAFMarshallingContext->new();
    }
    
    my $indentLevel = 0;

    if (@_)
    {
        # XXX: What is a better way to make sure indentLevel is numeric?
        if ($_[0] >= 0 && $_[0] <= 1000)
        {
            $indentLevel = shift;
        }
    }

    my $output = '';
    my $indent;

    if (!defined $object)
    {
        return NONE_STRING;
    }
    elsif (!ref($object))
    {
        # Simple scalar

        return $object;
    }
    elsif (UNIVERSAL::isa($object, 'SCALAR'))
    {
        # Reference to a scalar.  Return the dereferenced scalar.

        return $$object;
    }
    elsif (UNIVERSAL::isa($object, 'ARRAY'))
    {
        # Array reference (aka List)

        $output .= '[';
        $indentLevel++;

        my $arraySize = scalar(@$object);

        if ($arraySize > 0)
        {
            $output .= LINE_SEP;
        }

        my $i = 0;

        foreach my $item (@$object)
        {
            $output .= ' ' x ($indentLevel * INDENT_DELTA);

            if (!defined $item)
            {
                $output .= NONE_STRING;
            }
            elsif (!ref($item))
            {
                # Simple scalar

                $output .= $item;
            }
            elsif (UNIVERSAL::isa($item, 'ARRAY') ||
                UNIVERSAL::isa($item, 'STAF::STAFMarshallingContext') ||
                UNIVERSAL::isa($item, 'HASH'))
            {
                $output .= STAF::STAFFormatObject($item, $context, $indentLevel);
            }
            else
            {
                # Unrecognized data type for item in array
                $output .= $item;
            }

            if ($i < ($arraySize - 1))
            {
                $output .= ENTRY_SEPARATOR;
            }

            $output .= LINE_SEP;
        }

        $indentLevel--;

        if (($arraySize > 0) && ($indentLevel > 0))
        {
            $output .= ' ' x ($indentLevel * INDENT_DELTA);
        }

        $output .= ']';
    }
    elsif (UNIVERSAL::isa($object, 'STAF::STAFMarshallingContext'))
    {
        # Marshalling Context
        
        return STAF::STAFFormatObject($object->getRootObject(), $object, $indentLevel);
    }
    elsif (UNIVERSAL::isa($object, 'HASH'))
    {
        # Hash reference (aka Map)

        $output .= '{';
        $indentLevel++;

        my $mapSize = scalar(keys %$object);

        if ($mapSize > 0)
        {
            $output .= LINE_SEP;
        }

        my $maxKeyLength = 0;
        my $keyString;

        # Check if the map object has a map class key and if the context
        # contains a map class definition for this map class.
        
        if (exists($object->{'staf-map-class-name'}) &&
            $context->hasMapClassDefinition($object->{'staf-map-class-name'}))
        {
            # Map class object

            my $mapClassDef = $context->getMapClassDefinition(
                $object->{'staf-map-class-name'});

            my @mapClassDefKeys = $mapClassDef->keys();

            # Determine maximum key length (e.g. length of the longest key)

            foreach my $keyMap (@mapClassDefKeys)
            {
                if (exists($keyMap->{'display-name'}))
                {
                    $keyString = $keyMap->{'display-name'};
                }
                else
                {
                    $keyString = $keyMap->{'key'};
                }

                if (length($keyString) > $maxKeyLength)
                {
                    $maxKeyLength = length($keyString);
                }
            }

            # Now print each object in the map

            my $i = 1;
            my $value;

            foreach my $keyMap (@mapClassDefKeys)
            {
                if (exists($keyMap->{'display-name'}))
                {
                    $keyString = $keyMap->{'display-name'};
                }
                else
                {
                    $keyString = $keyMap->{'key'};
                }

                $output .= ' ' x ($indentLevel * INDENT_DELTA) . $keyString;
                $output .= ' ' x ($maxKeyLength - length($keyString)) . ': ';

                if (exists($object->{$keyMap->{'key'}}))
                {
                    $value = $object->{$keyMap->{'key'}};

                    if (UNIVERSAL::isa($value, 'ARRAY') ||
                        UNIVERSAL::isa($value, 'STAF::STAFMarshallingContext') ||
                        UNIVERSAL::isa($value, 'HASH'))
                    {
                        $output .= STAF::STAFFormatObject($value, $context, $indentLevel);
                    }
                    elsif (!defined $value)
                    {
                        $output .= NONE_STRING;
                    }
                    elsif (UNIVERSAL::isa($value, 'SCALAR'))
                    {
                        $output .= STAF::STAFFormatObject($$value, $context, $indentLevel);
                    }
                    else
                    {
                        # Unrecognized data type for value in map class object
                        $output .= $value;
                    }
                }
                else
                {
                    $output .= NONE_STRING;
                }

                if ($i < $mapSize)
                {
                    $output .= ENTRY_SEPARATOR;
                }

                $output .= LINE_SEP;
                $i++;
            }
        }
        else
        {
            # Plain map object

            # Determine maximum key length (e.g. length of the longest key)

            foreach $keyString (keys %$object)
            {
                if (length($keyString) > $maxKeyLength)
                {
                    $maxKeyLength = length($keyString);
                }
            }

            # Now print each object in the map

            my $i = 1;
            
            while (my ($key, $value) = each %$object)
            {
                $output .= ' ' x ($indentLevel * INDENT_DELTA) . $key;
                $output .= ' ' x ($maxKeyLength - length($key)) . ': ';

                if (!defined $value)
                {
                    $output .= NONE_STRING;
                }
                elsif (!ref($value))
                {
                    # Simple scalar

                    $output .= $value;
                }
                elsif (UNIVERSAL::isa($value, 'ARRAY') ||
                       UNIVERSAL::isa($value, 'STAF::STAFMarshallingContext') ||
                       UNIVERSAL::isa($value, 'HASH'))
                {
                    $output .= STAF::STAFFormatObject($value, $context, $indentLevel);
                }
                elsif (UNIVERSAL::isa($value, 'SCALAR'))
                {
                    $output .= STAF::STAFFormatObject($$value, $context, $indentLevel);
                }
                else
                {
                    # Unrecognized data type for value in hash
                    $output .= $value;
                }

                if ($i < $mapSize)
                {
                    $output .= ENTRY_SEPARATOR;
                }

                $output .= LINE_SEP;
                $i++;
            }
        }

        $indentLevel--;

        if (($mapSize > 0) && ($indentLevel > 0))
        {
            $output .= ' ' x ($indentLevel * INDENT_DELTA);
        }

        $output .= '}';
    }
    else
    {
        # Unrecognized data type for object.  Return "stringified" object.

        return '' . $object;
    }
    
    return $output;
}


#############################################################################
# Note that the return codes are duplicated below in this file so any
# changes must be done in both places.
# In STAF 3.0.0 Beta 4, as part of the Perl service work, the return codes
# were inadvertently renamed with a STAFResult prefix instead of a STAF
# prefix.  However, this broke existing STAF Perl client code so we added
# these back in here.
#############################################################################

$STAF::kOk                          =  0;
$STAF::kInvalidAPI                  =  1;
$STAF::kUnknownService              =  2;
$STAF::kInvalidHandle               =  3;
$STAF::kHandleAlreadyExists         =  4;
$STAF::kHandleDoesNotExist          =  5;
$STAF::kUnknownError                =  6;
$STAF::kInvalidRequestString        =  7;
$STAF::kInvalidServiceResult        =  8;
$STAF::kREXXError                   =  9;
$STAF::kBaseOSError                 = 10;
$STAF::kProcessAlreadyComplete      = 11;
$STAF::kProcessNotComplete          = 12;
$STAF::kVariableDoesNotExist        = 13;
$STAF::kUnResolvableString          = 14;
$STAF::kInvalidResolveString        = 15;
$STAF::kNoPathToMachine             = 16;
$STAF::kFileOpenError               = 17;
$STAF::kFileReadError               = 18;
$STAF::kFileWriteError              = 19;
$STAF::kFileDeleteError             = 20;
$STAF::kSTAFNotRunning              = 21;
$STAF::kCommunicationError          = 22;
$STAF::kTrusteeDoesNotExist         = 23;
$STAF::kInvalidTrustLevel           = 24;
$STAF::kAccessDenied                = 25;
$STAF::kSTAFRegistrationError       = 26;
$STAF::kServiceConfigurationError   = 27;
$STAF::kQueueFull                   = 28;
$STAF::kNoQueueElement              = 29;
$STAF::kNotifieeDoesNotExist        = 30;
$STAF::kInvalidAPILevel             = 31;
$STAF::kServiceNotUnregisterable    = 32;
$STAF::kServiceNotAvailable         = 33;
$STAF::kSemaphoreDoesNotExist       = 34;
$STAF::kNotSemaphoreOwner           = 35;
$STAF::kSemaphoreHasPendingRequests = 36;
$STAF::kTimeout                     = 37;
$STAF::kJavaError                   = 38;
$STAF::kConverterError              = 39;
$STAF::kMoveError                   = 40;
$STAF::kInvalidObject               = 41;
$STAF::kInvalidParm                 = 42;
$STAF::kRequestNumberNotFound       = 43;
$STAF::kInvalidAsynchOption         = 44;
$STAF::kRequestNotComplete          = 45;
$STAF::kProcessAuthenticationDenied = 46;
$STAF::kInvalidValue                = 47;
$STAF::kDoesNotExist                = 48;
$STAF::kAlreadyExists               = 49;
$STAF::kDirectoryNotEmpty           = 50;
$STAF::kDirectoryCopyError          = 51;
$STAF::kDiagnosticsNotEnabled       = 52;
$STAF::kHandleAuthenticationDenied  = 53;
$STAF::kHandleAlreadyAuthenticated  = 54;
$STAF::kInvalidSTAFVersion          = 55;
$STAF::kRequestCancelled            = 56;
$STAF::kCreateThreadError           = 57;
$STAF::kMaximumSizeExceeded         = 58;
$STAF::kMaximumHandlesExceeded      = 59;
$STAF::kNotRequester                = 60;
$STAF::kUserDefined                 = 4000;

################################################################################
# Define the submit2 sync mode options
################################################################################

$STAF::STAFHandle::kReqSync          = 0;
$STAF::STAFHandle::kReqFireAndForget = 1;
$STAF::STAFHandle::kReqQueue         = 2;
$STAF::STAFHandle::kReqRetain        = 3;
$STAF::STAFHandle::kReqQueueRetain   = 4;

################################################################################
# Define the STAF Handle types
################################################################################

$STAF::STAFHandle::kStandard         = 0;
$STAF::STAFHandle::kStatic           = 1;


#############################################################################
# Note that the return codes are duplicated above in this file so any
# changes must be done in both places.
# In STAF 3.0.0 Beta 4, as part of the Perl service work, the return codes
# were inadvertently renamed with a STAFResult prefix instead of a STAF 
# prefix.  However, this broke existing STAF Perl client code so we added
# them back in above.
#############################################################################
package STAFResult;

use constant kOk                          => scalar  0;
use constant kInvalidAPI                  => scalar  1;
use constant kUnknownService              => scalar  2;
use constant kInvalidHandle               => scalar  3;
use constant kHandleAlreadyExists         => scalar  4;
use constant kHandleDoesNotExist          => scalar  5;
use constant kUnknownError                => scalar  6;
use constant kInvalidRequestString        => scalar  7;
use constant kInvalidServiceResult        => scalar  8;
use constant kREXXError                   => scalar  9;
use constant kBaseOSError                 => scalar 10;
use constant kProcessAlreadyComplete      => scalar 11;
use constant kProcessNotComplete          => scalar 12;
use constant kVariableDoesNotExist        => scalar 13;
use constant kUnResolvableString          => scalar 14;
use constant kInvalidResolveString        => scalar 15;
use constant kNoPathToMachine             => scalar 16;
use constant kFileOpenError               => scalar 17;
use constant kFileReadError               => scalar 18;
use constant kFileWriteError              => scalar 19;
use constant kFileDeleteError             => scalar 20;
use constant kSTAFNotRunning              => scalar 21;
use constant kCommunicationError          => scalar 22;
use constant kTrusteeDoesNotExist         => scalar 23;
use constant kInvalidTrustLevel           => scalar 24;
use constant kAccessDenied                => scalar 25;
use constant kSTAFRegistrationError       => scalar 26;
use constant kServiceConfigurationError   => scalar 27;
use constant kQueueFull                   => scalar 28;
use constant kNoQueueElement              => scalar 29;
use constant kNotifieeDoesNotExist        => scalar 30;
use constant kInvalidAPILevel             => scalar 31;
use constant kServiceNotUnregisterable    => scalar 32;
use constant kServiceNotAvailable         => scalar 33;
use constant kSemaphoreDoesNotExist       => scalar 34;
use constant kNotSemaphoreOwner           => scalar 35;
use constant kSemaphoreHasPendingRequests => scalar 36;
use constant kTimeout                     => scalar 37;
use constant kJavaError                   => scalar 38;
use constant kConverterError              => scalar 39;
use constant kMoveError                   => scalar 40;
use constant kInvalidObject               => scalar 41;
use constant kInvalidParm                 => scalar 42;
use constant kRequestNumberNotFound       => scalar 43;
use constant kInvalidAsynchOption         => scalar 44;
use constant kRequestNotComplete          => scalar 45;
use constant kProcessAuthenticationDenied => scalar 46;
use constant kInvalidValue                => scalar 47;
use constant kDoesNotExist                => scalar 48;
use constant kAlreadyExists               => scalar 49;
use constant kDirectoryNotEmpty           => scalar 50;
use constant kDirectoryCopyError          => scalar 51;
use constant kDiagnosticsNotEnabled       => scalar 52;
use constant kHandleAuthenticationDenied  => scalar 53;
use constant kHandleAlreadyAuthenticated  => scalar 54;
use constant kInvalidSTAFVersion          => scalar 55;
use constant kRequestCancelled            => scalar 56;
use constant kCreateThreadError           => scalar 57;
use constant kMaximumSizeExceeded         => scalar 58;
use constant kMaximumHandlesExceeded      => scalar 59;
use constant kNotRequester                => scalar 60;
use constant kUserDefined                 => scalar 4000;

###############################################################################

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PLSTAF - Perl extension for blah blah blah

=head1 SYNOPSIS

  use PLSTAF;
  blah blah blah

=head1 ABSTRACT

  This should be the abstract for PLSTAF.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for PLSTAF, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.



=head1 SEE ALSO

Mention other useful documentation such as the documentation of
related modules or operating system documentation (such as man pages
in UNIX), or any relevant external documentation such as RFCs or
standards.

If you have a mailing list set up for your module, mention it here.

If you have a web site set up for your module, mention it here.

=head1 AUTHOR

A. U. Thor, E<lt>a.u.thor@a.galaxy.far.far.awayE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2002 by A. U. Thor

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
