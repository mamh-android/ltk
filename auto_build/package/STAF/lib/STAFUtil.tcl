#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###################################################
# STAFUtil.tcl - STAF Utility TCL Library         #
###################################################

package provide STAF 3.0
package require TCLSTAF

namespace eval STAF {

    variable kOk                                 0
    variable kInvalidAPI                         1
    variable kUnknownService                     2
    variable kInvalidHandle                      3
    variable kHandleAlreadyExists                4
    variable kHandleDoesNotExist                 5
    variable kUnknownError                       6
    variable kInvalidRequestString               7
    variable kInvalidServiceResult               8
    variable kREXXError                          9
    variable kBaseOSError                       10
    variable kProcessAlreadyComplete            11
    variable kProcessNotComplete                12
    variable kVariableDoesNotExist              13
    variable kUnResolvableString                14
    variable kInvalidResolveString              15
    variable kNoPathToMachine                   16
    variable kFileOpenError                     17
    variable kFileReadError                     18
    variable kFileWriteError                    19
    variable kFileDeleteError                   20
    variable kSTAFNotRunning                    21
    variable kCommunicationError                22
    variable kTrusteeDoesNotExist               23
    variable kInvalidTrustLevel                 24
    variable kAccessDenied                      25
    variable kSTAFRegistrationError             26
    variable kServiceConfigurationError         27
    variable kQueueFull                         28
    variable kNoQueueElement                    29
    variable kNotifieeDoesNotExist              30
    variable kInvalidAPILevel                   31
    variable kServiceNotUnregisterable          32
    variable kServiceNotAvailable               33
    variable kSemaphoreDoesNotExist             34
    variable kNotSemaphoreOwner                 35
    variable kSemaphoreHasPendingRequests       36
    variable kTimeout                           37
    variable kJavaError                         38
    variable kConverterError                    39
    variable kMoveError                         40
    variable kInvalidObject                     41
    variable kInvalidParm                       42
    variable kRequestNumberNotFound             43
    variable kInvalidAsynchOption               44
    variable kRequestNotComplete                45
    variable kProcessAuthenticationDenied       46
    variable kInvalidValue                      47
    variable kDoesNotExist                      48
    variable kAlreadyExists                     49
    variable kDirectoryNotEmpty                 50
    variable kDirectoryCopyError                51
    variable kDiagnosticsNotEnabled             52
    variable kHandleAuthenticationDenied        53
    variable kHandleAlreadyAuthenticated        54
    variable kInvalidSTAFVersion                55
    variable kRequestCancelled                  56
    variable kThreadCreateError                 57
    variable kMaximumSizeExceeded               58
    variable kMaximumHandlesExceeded            59
    variable kNotRequester                      60

}

################################################################
# STAF::WrapData - Wraps a TCL String using colon delimited    # 
#                  STAF format                                 # 
# Accepts: A string                                            #
# Returns: Colon-delimited version of input string             #
################################################################

proc STAF::WrapData {data} { 
  return :[string length $data]:$data
}


################################################################################
# STAF::internalCheckCommandInput - Used internally by STAF to ensure a caller #
#                                   has provided proper parameters to a proc   #
# optionDefs:                                                                  #
#   {                                                                          #
#     { optionName ?default value? } ...                                       #
#   }                                                                          #
#                                                                              #
# Note: If no default value is provided then the option is not allowed to      #
#       have a value                                                           #
#                                                                              #
# argDefs:                                                                     #
#   {                                                                          #
#     { argName ?default value? } ...                                          #
#     ?otherArgs?                                                              #
#   }                                                                          #
#                                                                              #
# Note: If no default value is provided then the argument must be present      #
# Note: If an argName of "otherArgs" is specified, then all remaining args     #
#       will be grouped with that argument                                     #
#                                                                              #
# Returns:                                                                     #
#   {                                                                          #
#     { returnCode ?error string if non-zero RC? }                             #
#     ?{ optionName optionValue ... }?  -- if RC == 0                          #
#     ?{ argName argValue ... }?        -- if RC == 0                          #
#   }                                                                          #
#                                                                              #
# Note: Only options actually specified by the user are returned               #
################################################################################

proc STAF::internalCheckCommandInput {commandInput optionDefs argDefs} {
    # Initialize error buffers

    set errorBuffer {wrong # args: should be "}
    set validOptions {valid options are}

    # Set up option definitions

    foreach optionDef $optionDefs {
        if {[llength $optionDef] == 2} {
            set options([lindex $optionDef 0]) "VALUEALLOWED"
            set outputOptions([lindex $optionDef 0]) [lindex $optionDef 1]
            set errorBuffer "$errorBuffer ?-[lindex $optionDef 0] ?value??"
            set validOptions "$validOptions -[lindex $optionDef 0]"
        } else {
            set options([lindex $optionDef 0]) "NOVALUEALLOWED"
            set errorBuffer "$errorBuffer ?-[lindex $optionDef 0]?"
            set validOptions "$validOptions -[lindex $optionDef 0]"
        }
    }

    # Set up argument definitions

    set numArgsDefined 0

    foreach argDef $argDefs {
        if {[llength $argDef] == 2} {
            set args($numArgsDefined) [list [lindex $argDef 0] \
                                            "OPTIONAL [lindex $argDef 1]"]
            set errorBuffer "$errorBuffer ?[lindex $argDef 0]?"
        } else {
            set args($numArgsDefined) [list [lindex $argDef 0] REQUIRED]
            set errorBuffer "$errorBuffer [lindex $argDef 0]"
        }

        incr numArgsDefined
    }

    # Get all options out of the input

    while {[string index [lindex $commandInput 0] 0] == "-" && [info exists options([string range [lindex $commandInput 0] 1 end])] == 1} {
        set currOption [string range [lindex $commandInput 0] 1 end]
        set currOptionValue {}
        set commandInput [lrange $commandInput 1 end]

        if {[info exists options($currOption)] == 0} {
            return [list [list 1 "bad option \"-$currOption\": $validOptions"]]
        } else {
            if {$options($currOption) == "NOVALUEALLOWED"} {
                set outputOptions($currOption) {}
            } elseif {[string index [lindex $commandInput 0] 0] != "-"} {
                set outputOptions($currOption) [lindex $commandInput 0]
                set commandInput [lrange $commandInput 1 end]
            }
        }
    }

    # Now process arguments

    set currArgNum 0

    while {$currArgNum != $numArgsDefined} {
        if {[lindex [lindex $args($currArgNum) 1] 0] == "REQUIRED"} {
            if {[llength $commandInput] == 0} {
                return [list [list 1 $errorBuffer] {} {}]
            }

            set outputArgs([lindex $args($currArgNum) 0]) \
                [lindex $commandInput 0]
            set commandInput [lrange $commandInput 1 end]
        } else {
            if {[llength $commandInput] != 0} {
                set outputArgs([lindex $args($currArgNum) 0]) \
                    [lindex $commandInput 0]
                set commandInput [lrange $commandInput 1 end]
            } else {
                set outputArgs([lindex $args($currArgNum) 0]) \
                    [lrange [lindex $args($currArgNum) 1] 1 end]
            }
        }

        incr currArgNum
    }

    return [list [list 0 "Ok"] [array get outputOptions] [array get outputArgs]]
}


namespace eval STAF {

    variable NoneType    "STAF_DT_NONE"
    variable ScalarType  "STAF_DT_SCALAR"
    variable ListType    "STAF_DT_LIST"
    variable MapType     "STAF_DT_MAP"
    variable ContextType "STAF_DT_CONTEXT"

    variable CompleteTypeList [list $NoneType $ScalarType $ListType $MapType $ContextType]
}


proc STAF::datatype {args} {
    if {[llength $args] == 0} {
        error {wrong # args: should be "STAF::datatype subcommand ?arg ...?"}
    }

    set subcommand [lindex $args 0]
    set args [lrange $args 1 end]

    switch $subcommand {
        createNone {
            set checkResult [STAF::internalCheckCommandInput $args {} {}]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            return [list $STAF::NoneType "None"]
        }

        createScalar {
            # args: [value]
            set myArgs [list {value {}}]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return "$STAF::ScalarType $input(value)"
        }

        createList {
            # args: [value]
            set myArgs [list {value {}}]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return "$STAF::ListType $input(value)"
        }

        createMap {
            # args: [value]
            set myArgs [list {value {}}]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return "$STAF::MapType $input(value)"
        }

        createContext {
            # args: [value]
            set myArgs [list {value {}}]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return "$STAF::ContextType $input(value)"
        }

        getType {
            # args: object
            set myArgs [list object]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            set type [lindex $input(object) 0]

            if {[lsearch -exact $STAF::CompleteTypeList $type] == -1} {
                return $STAF::ScalarType
            }

            return $type
        }

        getValue {
            # args: object
            set myArgs [list object]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            set objType [lindex $input(object) 0]

            if {[lsearch -exact $STAF::CompleteTypeList $objType] == -1} {
                return $input(object)
            }

            return [lrange $input(object) 1 end]
        }

        setValue {
            # args: objectVar
            set myArgs [list objectVar newValue]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            upvar $input(objectVar) object

            if {[lindex $object 0] == $STAF::NoneType} {
                error "Can't set the value of a None object"
            }

            return [set object "[lindex $object 0] $input(newValue)"]
        }

        default {
            error "Unknown subcommand '$subcommand' specified for STAF::datatype"
        }
    }
}

proc STAF::mapclassdef {args} {
    if {[llength $args] == 0} {
        error {wrong # args: should be "STAF::mapclassdef subcommand ?arg ...?"}
    }

    set subcommand [lindex $args 0]
    set args [lrange $args 1 end]

    switch $subcommand {
        create {
            # args: mapclassname ?definitionObject?
            
            # The definitionObject argument is for internal use only

            set myArgs [list name {mapClassDefObj {}}]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            # ???: We might want to look at handling the mapClassDefObj a
            #      a bit more gracefully.  Like, maybe doing it as an option
            #      instead of an arg.

            if {$input(mapClassDefObj) != ""} {
                return $input(mapClassDefObj)
            }

            set mcd(name) $input(name)
            set mcd(keys) [STAF::datatype createList]

            return [STAF::datatype createMap [array get mcd]]
        }

        createInstance {
            # args: mapclassdef
            set myArgs [list mapclassdef]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            set mapClassName [STAF::mapclassdef getName $input(mapclassdef)]
            set instance($STAF::MAP_CLASS_NAME_KEY) $mapClassName

            return [STAF::datatype createMap [array get instance]]
        }

        addKey {
            # args: mapclassdefVar keyName ?displayName?
            set myArgs [list mapclassdefVar keyName {displayName {}}]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            upvar $input(mapclassdefVar) mcd

            set key(key) $input(keyName)
            set key($STAF::DISPLAY_NAME_KEY) $input(displayName)

            array set mcdArray [STAF::datatype getValue $mcd]
            lappend mcdArray(keys) [STAF::datatype createMap [array get key]]
            STAF::datatype setValue mcd [array get mcdArray]

            return $mcd
        }

        setKeyProperty {
            # args: mapclassdefVar, keyName, property, value):
            set myArgs [list mapclassdefVar keyName property value]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            upvar $input(mapclassdefVar) mcd

            array set mcdArray [STAF::datatype getValue $mcd]
            set outputKeys [STAF::datatype createList]

            foreach key [STAF::datatype getValue $mcdArray(keys)] {
                array set thisKey [STAF::datatype getValue $key]

                if {![string compare $thisKey(key) $input(keyName)]} {
                    set thisKey($input(property)) $input(value)
                }

                lappend outputKeys [STAF::datatype createMap [array get thisKey]]
            }

            set mcdArray(keys) $outputKeys
            STAF::datatype setValue mcd [array get mcdArray]

            return $mcd
        }

        getKeys {
            # args: mapclassdef
            set myArgs [list mapclassdef]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcdArray [STAF::datatype getValue $input(mapclassdef)]

            return $mcdArray(keys)
        }

        getName {
            # args: mapclassdef
            set myArgs [list mapclassdef]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcdArray [STAF::datatype getValue $input(mapclassdef)]

            return $mcdArray(name)
        }

        getMapClassDefinitionObject {
            # args: mapclassdef
            
            # This subcommand is for internal use only
            
            set myArgs [list mapclassdef]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return $input(mapclassdef)
        }

        default {
            error "Unknown subcommand '$subcommand' specified for STAF::mapclassdef"
        }
    }
}


proc STAF::mcontext {args} {
    if {[llength $args] == 0} {
        error {wrong # args: should be "STAF::mcontext subcommand ?arg ...?"}
    }

    set subcommand [lindex $args 0]
    set args [lrange $args 1 end]

    switch $subcommand {
        testOption {
            set myOptions [list opt1 {opt2 def2} opt3]
            set myArgs [list req1 req2 {oarg3 val3} {oarg4 val4}]
            set checkResult \
                [STAF::internalCheckCommandInput $args $myOptions $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }
        }

        create {
            # args: ?rootObject?
            set myArgs [list [list rootObj [STAF::datatype createNone]] [list mapClassMap [STAF::datatype createMap]]]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }
            # array set options [lindex $checkResult 1]
            array set input [lindex $checkResult 2]

            return [STAF::datatype createContext [array get input]]
        }

        setMapClassDefinition {
            set myArgs [list mcVar mapClassDef]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            upvar $input(mcVar) mc
            array set mcArray [STAF::datatype getValue $mc]
            array set mcmArray [STAF::datatype getValue $mcArray(mapClassMap)]

            set mcdName [STAF::mapclassdef getName $input(mapClassDef)]
            set mcmArray($mcdName) $input(mapClassDef)
            STAF::datatype setValue mcArray(mapClassMap) [array get mcmArray]
            STAF::datatype setValue mc [array get mcArray]

            return $mc
        }

        getMapClassDefinition {
            # args: mc mcdName
            set myArgs [list mc mcdName]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcArray [STAF::datatype getValue $input(mc)]
            array set mcmArray [STAF::datatype getValue $mcArray(mapClassMap)]
            set mcdName $input(mcdName)

            if {[info exists mcmArray($mcdName)]} {
                return $mcmArray($mcdName)
            } else {
                return []
            }
        }

        hasMapClassDefinition {
            # args: mc mcdName
            set myArgs [list mc mcdName]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcArray [STAF::datatype getValue $input(mc)]
            array set mcmArray [STAF::datatype getValue $mcArray(mapClassMap)]

            return [info exists mcmArray($input(mcdName))]
        }

        getMapClassMap {
            # args: mc
            set myArgs [list mc]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcArray [STAF::datatype getValue $input(mc)]

            if {[info exists mcArray(mapClassMap)]} {
                return $mcArray(mapClassMap)
            } else {
                return []
            }
        }

        getMapClassDefinitionNames {
            # args: mc
            set myArgs [list mc]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcArray [STAF::datatype getValue $input(mc)]
            array set mcmArray [STAF::datatype getValue $mcArray(mapClassMap)]

            return [array names mcmArray]
        }

        setRootObject {
            # args: mcVar rootObj
            set myArgs [list mcVar rootObj]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            upvar $input(mcVar) mc
            array set mcArray [STAF::datatype getValue $mc]
            set mcArray(rootObj) $input(rootObj)
            STAF::datatype setValue mc [array get mcArray]

            return $mc
        }

        getRootObject {
            # args: mc
            set myArgs [list mc]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcArray [STAF::datatype getValue $input(mc)]

            return $mcArray(rootObj)
        }

        getPrimaryObject {
            # args: mc
            set myArgs [list mc]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]
            array set mcArray [STAF::datatype getValue $input(mc)]
            array set mcmArray [STAF::datatype getValue $mcArray(mapClassMap)]

            if {[array size mcmArray]} {
                return $input(mc)
            } else {
                return $mcArray(rootObj)
            }
        }

        marshall {
            # args: mc
            set myArgs [list mc]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return [STAF::marshall -context $input(mc) $input(mc)]
        }

        formatObject {
            # args: mc
            set myArgs [list mc]
            set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

            if {[lindex [lindex $checkResult 0] 0] != 0} {
                error [lindex [lindex $checkResult 0] 1]
            }

            array set input [lindex $checkResult 2]

            return [STAF::formatObject -context $input(mc) $input(mc)]
        }

        default {
            error "Unknown subcommand '$subcommand' specified for STAF::mcontext"
        }
    }
}


proc STAF::stringStartsWith {fullString startingString} {
    return [string equal -length [string length $startingString] $fullString $startingString]
}


namespace eval STAF {

    # These variables are for internal use only
    
    variable MARSHALLED_DATA_MARKER  {@SDT/}
    variable NONE_MARKER             {@SDT/$0:0:}
    variable SCALAR_MARKER           {@SDT/$}
    variable SCALAR_STRING_MARKER    {@SDT/$S}
    variable LIST_MARKER             {@SDT/[}
    variable MAP_MARKER              "@SDT/\{"
    variable MC_INSTANCE_MARKER      {@SDT/%}
    variable CONTEXT_MARKER          {@SDT/*}

    variable NONE_STRING             {<None>}
    variable DISPLAY_NAME_KEY        {display-name}
    variable MAP_CLASS_MAP_KEY       {map-class-map}
    variable MAP_CLASS_NAME_KEY      {staf-map-class-name}
    variable ENTRY_SEPARATOR         {}
    # 80 spaces
    variable SPACES                  {                                                                                }
    variable INDENT_DELTA            {2}

}


proc STAF::isMarshalledData {args} {
    # args: data
            
    set myArgs [list data]
    set checkResult [STAF::internalCheckCommandInput $args {} $myArgs]

    if {[lindex [lindex $checkResult 0] 0] != 0} {
        error [lindex [lindex $checkResult 0] 1]
    }

    array set input [lindex $checkResult 2]
    set data $input(data)

    return [STAF::stringStartsWith $data $STAF::MARSHALLED_DATA_MARKER]
}


proc STAF::marshall {args} {
    # options: -context <context>
    # args: obj
    set myOptions [list {context {}}]
    set myArgs [list obj]
    set checkResult [STAF::internalCheckCommandInput $args $myOptions $myArgs]

    if {[lindex [lindex $checkResult 0] 0] != 0} {
        error [lindex [lindex $checkResult 0] 1]
    }

    array set options [lindex $checkResult 1]
    array set input [lindex $checkResult 2]

    set context $options(context)
    set obj $input(obj)

    if {[STAF::datatype getType $obj] == $STAF::NoneType} {

        return $STAF::NONE_MARKER

    } elseif {[STAF::datatype getType $obj] == $STAF::ListType} {

        set theList [STAF::datatype getValue $obj]
        set theListLength [llength $theList]
        set listData ""

        foreach item $theList {
            append listData [STAF::marshall -context $context $item]
        }

        set listDataLength [string length $listData]

        return "$STAF::LIST_MARKER$theListLength:$listDataLength:$listData"

    } elseif {[STAF::datatype getType $obj] == $STAF::MapType} {

        set mapValue [STAF::datatype getValue $obj]
        array set map $mapValue

        if {$context != "" && \
            [STAF::datatype getType $context] == $STAF::ContextType && \
            [array names map $STAF::MAP_CLASS_NAME_KEY] != "" && \
            [STAF::mcontext hasMapClassDefinition $context \
                            $map($STAF::MAP_CLASS_NAME_KEY)]} {

            set mapClassName $map($STAF::MAP_CLASS_NAME_KEY)
            set mapClass [STAF::mcontext getMapClassDefinition \
                                         $context $mapClassName]
            set mapData ":[string length $mapClassName]:$mapClassName"
            set keyListObj [STAF::mapclassdef getKeys $mapClass]

            foreach keyObj [STAF::datatype getValue $keyListObj] {
                array set key [STAF::datatype getValue $keyObj]
                append mapData [STAF::marshall -context $context \
                                $map($key(key))]
            }

            set mapDataLength [string length $mapData]

            return "$STAF::MC_INSTANCE_MARKER:$mapDataLength:$mapData"

        } else {
            set mapData ""
            array set map [STAF::datatype getValue $obj]

            foreach key [array names map] {
                set keyLength [string length $key]
                append mapData ":" $keyLength ":" $key \
                    [STAF::marshall -context $context $map($key)]
            }

            return "$STAF::MAP_MARKER:[string length $mapData]:$mapData"
        }

    } elseif {[STAF::datatype getType $obj] == $STAF::ContextType} {

        set mapClassMapObj [STAF::mcontext getMapClassMap $obj]
        array set mapClassMap [STAF::datatype getValue $mapClassMapObj]

        if {[array names mapClassMap] == ""} {

            return [STAF::marshall -context $context \
                    [STAF::mcontext getRootObject $obj]]

        } else {

            set contextMap($STAF::MAP_CLASS_MAP_KEY) $mapClassMapObj
            set contextMapObj [STAF::datatype createMap [array get contextMap]]

            set mcData [STAF::marshall -context $context $contextMapObj]
            append mcData [STAF::marshall -context $obj \
                [STAF::mcontext getRootObject $obj]]

            return "$STAF::CONTEXT_MARKER:[string length $mcData]:$mcData"
        }

    } else {

        # Treat this as a scalar

        set scalarValue [STAF::datatype getValue $obj]
        set scalarLength [string length $scalarValue]

        return "$STAF::SCALAR_STRING_MARKER:$scalarLength:$scalarValue"
    }
}


proc STAF::unmarshall {args} {
    # options: -context <context> -ignoreIndirectObjects
    # args: marshalledData
    set myOptions [list [list context [STAF::mcontext create]] ignoreIndirectObjects]
    set myArgs [list md]
    set checkResult [STAF::internalCheckCommandInput $args $myOptions $myArgs]

    if {[lindex [lindex $checkResult 0] 0] != 0} {
        error [lindex [lindex $checkResult 0] 1]
    }

    array set options [lindex $checkResult 1]
    array set input [lindex $checkResult 2]

    set context $options(context)
    set md $input(md)

    set flags ""

    if {[array names options "ignoreIndirectObjects"] != ""} {
        set flags "-ignoreIndirectObjects"
    }

    # Begin unmarshalling

    if {[STAF::stringStartsWith $md $STAF::NONE_MARKER]} {

        return [STAF::mcontext create]

    } elseif {[STAF::stringStartsWith $md $STAF::SCALAR_MARKER]} {

        # @SDT/$S:<string-length>:<String>

        set colonIndex [string first ":" $md]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set dataIndex [expr $colonIndex + 1]

        set colonIndex [string first ":" $md $dataIndex]
            
        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }
        
        set stringLength [string range $md $dataIndex [expr $colonIndex - 1]]

        if {[string is integer -strict $stringLength] == -1} {
            return [STAF::mcontext create $md]
        }

        set dataIndex [expr $colonIndex + 1]

        set dataLength [expr [string length $md] - $dataIndex]

        if {[string equal $stringLength $dataLength] == 0} {
            return [STAF::mcontext create $md]
        }

        set theString [string range $md $dataIndex end]

        if {[STAF::stringStartsWith $md $STAF::MARSHALLED_DATA_MARKER] &&
            [array names options "ignoreIndirectObjects"] == ""} {
            return [STAF::unmarshall -context $context $theString]
        }

        return [STAF::mcontext create $theString]

    } elseif {[STAF::stringStartsWith $md $STAF::LIST_MARKER]} {

        # @SDT/[<number-of-items>:<array-length>:<SDT-Any-1>...<SDT-Any-n>

        # Get number-of-items in the list

        set colonIndex [string first ":" $md]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set numItems [string range $md [string length $STAF::LIST_MARKER] \
            [expr $colonIndex - 1]]

        if {[string is integer -strict $numItems] == -1} {
            return [STAF::mcontext create $md]
        }

        # Get array-length

        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]
        
        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }

        set arrayLength [string range $md [expr $colonIndex + 1] \
                                          [expr $colonIndex2 - 1]]

        if {[string is integer -strict $arrayLength] == -1} {
            return [STAF::mcontext create $md]
        }

        set dataIndex [expr $colonIndex2 + 1]

        set dataLength [expr [string length $md] - $dataIndex]

        if {[string equal $arrayLength $dataLength] == 0} {
            return [STAF::mcontext create $md]
        }

        # Create an array (aka list) of the data

        set theList [list]

        for {set i 0} {$i < $numItems} {incr i} {

            # Get the next item in the list and unmarshall it and add it to
            # the list
            
            set colonIndex1 [string first ":" $md $dataIndex]

            if {$colonIndex1 == -1} {
                return [STAF::mcontext create $md]
            }

            set colonIndex2 [string first ":" $md [expr $colonIndex1 + 1]]

            if {$colonIndex2 == -1} {
                return [STAF::mcontext create $md]
            }

            set itemLength [string range $md [expr $colonIndex1 + 1] [expr $colonIndex2 - 1]]

            if {[string is integer -strict $itemLength] == -1} {
                return [STAF::mcontext create $md]
            }
            
            set dataToUnmarshall [string range $md $dataIndex [expr $colonIndex2 + $itemLength]]
            set outMC [eval {STAF::unmarshall} -context {$context} $flags {$dataToUnmarshall}]

            lappend theList [STAF::mcontext getPrimaryObject $outMC]

            set dataIndex [expr $colonIndex2 + $itemLength + 1]
        }

        return [STAF::mcontext create [STAF::datatype createList $theList]]

    } elseif {[STAF::stringStartsWith $md $STAF::MAP_MARKER]} {

        # :<map-length>::<key-1-length>:<key-1><SDT-Any> 
        #                ...
        #               :<key-n-length>:<key-1><SDT-Any>

        # Get the map-length
        
        set colonIndex [string first ":" $md]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }
            
        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]
        
        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }

        set mapLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

        if {[string is integer -strict $mapLength] == -1} {
            return [STAF::mcontext create $md]
        }

        set dataIndex [expr $colonIndex2 + 1]
        set dataLength [string length $md]

        if {$mapLength != [expr $dataLength - $dataIndex]} {
            return [STAF::mcontext create $md]
        }

        # Create a map of the data

        while {$dataIndex < $dataLength} {

            # Get the key first

            set keyColonIndex1 [string first ":" $md $dataIndex]

            if {$keyColonIndex1 == -1} {
                return [STAF::mcontext create $md]
            }

            set keyColonIndex2 [string first ":" $md [expr $keyColonIndex1 + 1]]

            if {$keyColonIndex2 == -1} {
                return [STAF::mcontext create $md]
            }

            set keyLength [string range $md [expr $keyColonIndex1 + 1] [expr $keyColonIndex2 - 1]]

            if {[string is integer -strict $keyLength] == -1} {
                return [STAF::mcontext create $md]
            }

            set key [string range $md [expr $keyColonIndex2 + 1] [expr $keyColonIndex2 + $keyLength]]

            set dataIndex [expr $keyColonIndex2 + 1 + $keyLength]
            
            if {$dataIndex >= [string length $md]} {
                return [STAF::mcontext create $md]
            }

            # Now, get the object and add the key and unmarshalled object to
            # the map

            set colonIndex1 [string first ":" $md $dataIndex]

            if {$colonIndex1 == -1} {
                return [STAF::mcontext create $md]
            }

            set colonIndex2 [string first ":" $md [expr $colonIndex1 + 1]]

            if {$colonIndex2 == -1} {
                return [STAF::mcontext create $md]
            }

            set itemLength [string range $md [expr $colonIndex1 + 1] [expr $colonIndex2 - 1]]

            if {[string is integer -strict $itemLength] == -1} {
                return [STAF::mcontext create $md]
            }

            set dataToUnmarshall [string range $md $dataIndex [expr $colonIndex2 + $itemLength]]
            set outMC [eval {STAF::unmarshall} -context {$context} $flags {$dataToUnmarshall}]

            set map($key) [STAF::mcontext getPrimaryObject $outMC]

            set dataIndex [expr $colonIndex2 + $itemLength + 1]
        }

        return [STAF::mcontext create [STAF::datatype createMap [array get map]]]

    } elseif {[STAF::stringStartsWith $md $STAF::MC_INSTANCE_MARKER]} {

        #@SDT/%:<map-class-instance-length>::<map-class-name-length>:<map-class-name>
        #    <SDT-Any-value-1>
        #    ...
        #    <SDT-Any-value-n>

        # Get the map-class-instance-length
        
        set colonIndex [string first ":" $md]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]

        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }
        
        set mapClassInstanceLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

        if {[string is integer -strict $mapClassInstanceLength] == -1} {
            return [STAF::mcontext create $md]
        }

        set dataIndex [expr $colonIndex2 + 1]
        set dataLength [string length $md]
        
        if {$mapClassInstanceLength != [expr $dataLength - $dataIndex]} {
            return [STAF::mcontext create $md]
        }

        # Get the map-class-name-length

        set colonIndex [string first ":" $md $dataIndex]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]

        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }

        set mapClassNameLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

        if {[string is integer -strict $mapClassNameLength] == -1} {
            return [STAF::mcontext create $md]
        }

        # Get map-class-name

        set mapClassName [string range $md [expr $colonIndex2 + 1] [expr $colonIndex2 + $mapClassNameLength]]
        set dataIndex [expr $colonIndex2 + 1 + $mapClassNameLength]

        # Add the map class name to the map

        set map($STAF::MAP_CLASS_NAME_KEY) $mapClassName

        # Unmarshall all of the actual keys and add to the map keys hash

        set mapClass [STAF::mcontext getMapClassDefinition $context $mapClassName]
        set keys [STAF::datatype getValue [STAF::mapclassdef getKeys $mapClass]]
        set keyIndex 0

        while {$dataIndex < $dataLength} {
            set colonIndex [string first ":" $md $dataIndex]

            if {$colonIndex == -1} {
                return [STAF::mcontext create $md]
            }

            set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]

            if {$colonIndex2 == -1} {
                return [STAF::mcontext create $md]
            }

            set itemLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

            if {[string is integer -strict $itemLength] == -1} {
                return [STAF::mcontext create $md]
            }

            set dataToUnmarshall [string range $md $dataIndex [expr $colonIndex2 + $itemLength]]
            set outMC [eval {STAF::unmarshall} -context {$context} $flags {$dataToUnmarshall}]
            array set theKey [STAF::datatype getValue [lindex $keys $keyIndex]]
            
            set map($theKey(key)) [STAF::mcontext getPrimaryObject $outMC]

            set dataIndex [expr $colonIndex2 + $itemLength + 1]
            incr keyIndex
        }

        return [STAF::mcontext create [STAF::datatype createMap [array get map]]]

    } elseif {[STAF::stringStartsWith $md $STAF::CONTEXT_MARKER]} {

        # @SDT/*:<context-length>:@SDT/X:<mapClassLength>:<mapClassData>
        #                         <rootObject>

        # Get context-length
        
        set colonIndex [string first ":" $md]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]

        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }

        set contextLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

        if {[string is integer -strict $contextLength] == -1} {
            return [STAF::mcontext create $md]
        }

        set contextIndex [expr $colonIndex2 + 1]

        # Get mapClassLength

        set colonIndex [string first ":" $md $contextIndex]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]

        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }

        set mapIndex $contextIndex

        set mapLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

        if {[string is integer -strict $mapLength] == -1} {
            return [STAF::mcontext create $md]
        }

        set mapDataIndex [expr $colonIndex2 + 1]
        set dataLength [string length $md]

        if {$mapLength > [expr $dataLength - $mapDataIndex]} {
            return [STAF::mcontext create $md]
        }

        # Create a new marshalling context with the map classes and root object

        set dataToUnmarshall [string range $md $mapIndex [expr $mapDataIndex + $mapLength - 1]]
        set outMC [eval {STAF::unmarshall} -context {$context} $flags {$dataToUnmarshall}]
        set contextMapObj [STAF::mcontext getPrimaryObject $outMC]
        array set contextMap [STAF::datatype getValue $contextMapObj]
        set mapClassMapObj $contextMap($STAF::MAP_CLASS_MAP_KEY)
        set newContext [STAF::mcontext create [STAF::datatype createNone] $mapClassMapObj]

        set rootObjIndex [expr $mapDataIndex + $mapLength]
        set colonIndex [string first ":" $md [expr $rootObjIndex]]

        if {$colonIndex == -1} {
            return [STAF::mcontext create $md]
        }

        set rootObjIndex [expr $mapDataIndex + $mapLength]

        set colonIndex2 [string first ":" $md [expr $colonIndex + 1]]

        if {$colonIndex2 == -1} {
            return [STAF::mcontext create $md]
        }

        set rootObjLength [string range $md [expr $colonIndex + 1] [expr $colonIndex2 - 1]]

        if {[string is integer -strict $rootObjLength] == -1} {
            return [STAF::mcontext create $md]
        }
        
        set rootObjDataIndex [expr $colonIndex2 + 1]

        if {$rootObjLength != [expr $dataLength - $rootObjDataIndex]} {
            return [STAF::mcontext create $md]
        }

        set dataToUnmarshall [string range $md $rootObjIndex [expr $rootObjDataIndex + $rootObjLength - 1]]
        set outMC [eval {STAF::unmarshall} -context {$newContext} $flags {$dataToUnmarshall}]

        STAF::mcontext setRootObject newContext [STAF::mcontext getPrimaryObject $outMC]

        return $newContext

    } else {
        return [STAF::mcontext create $md]
    }
}


proc STAF::formatObject {args} {
    # options: ?-context <context>? ?-indentLevel <indentLevel>?
    # args: object
    
    # Note that the optional -indentLevel option is not meant to be used
    # by a user calling the formatObject procedure, thus we don't document
    # it externally.  It's meant to be used internally by the formatObject
    # method when recursively calling itself.
    
    set myOptions [list [list context ""] [list indentLevel 0]]
    set myArgs [list obj]
    set checkResult [STAF::internalCheckCommandInput $args $myOptions $myArgs]

    if {[lindex [lindex $checkResult 0] 0] != 0} {
        error [lindex [lindex $checkResult 0] 1]
    }

    array set options [lindex $checkResult 1]
    array set input [lindex $checkResult 2]

    set obj $input(obj)
    set context $options(context)
    set indentLevel $options(indentLevel)

    # Begin formatting
    
    set output ""

    set lineSep "\n"
    
    if {[STAF::datatype getType $obj] == $STAF::ListType} {

        set theList [STAF::datatype getValue $obj]
        set theListLength [llength $theList]

        append output "\["
        incr indentLevel 1

        if {$theListLength > 0} {
            append output $lineSep
        }

        # Format each object in the list
        
        set index 1

        foreach item $theList {

            set indent [expr ($indentLevel * $STAF::INDENT_DELTA) - 1]
            append output [string range $STAF::SPACES 0 $indent]
            
            if {[STAF::datatype getType $item] == $STAF::ListType || \
                [STAF::datatype getType $item] == $STAF::MapType || \
                [STAF::datatype getType $item] == $STAF::ContextType} {

                append output [eval {STAF::formatObject} \
                    -context {$context} -indentLevel $indentLevel {$item}]

            } elseif {[STAF::datatype getType $item] == $STAF::NoneType} {
                append output $STAF::NONE_STRING

            } else {
                append output [STAF::datatype getValue $item]
            }

            if {$index < $theListLength} {
                append output $STAF::ENTRY_SEPARATOR
            }

            append output $lineSep

            incr index 1
        }
        
        incr indentLevel -1

        if {$theListLength > 0} {
            set indent [expr ($indentLevel * $STAF::INDENT_DELTA) - 1]
            append output [string range $STAF::SPACES 0 $indent]
       }

        append output "]"

    } elseif {[STAF::datatype getType $obj] == $STAF::MapType} {

        array set map [STAF::datatype getValue $obj]

        append output "\{"
        incr indentLevel 1
        set mapNameList [array names map]
        set mapSize [llength $mapNameList]

        if {$mapSize > 0} {
            append output $lineSep
        }

        if {$context != "" && \
            [STAF::datatype getType $context] == $STAF::ContextType && \
            [array names map $STAF::MAP_CLASS_NAME_KEY] != "" && \
            [STAF::mcontext hasMapClassDefinition $context \
                            $map($STAF::MAP_CLASS_NAME_KEY)]} {

            set mapClassName $map($STAF::MAP_CLASS_NAME_KEY)
            set mapClass [STAF::mcontext getMapClassDefinition $context \
                          $mapClassName]
            set keyListObj [STAF::mapclassdef getKeys $mapClass]
            set keyList [STAF::datatype getValue $keyListObj]

            # Determine maximum key length

            set maxKeyLength 0

            foreach keyObj $keyList {

                array set key [STAF::datatype getValue $keyObj]
                set keyName $key(key)

                if {[array names key $STAF::DISPLAY_NAME_KEY] != ""} {
                    set keyName $key($STAF::DISPLAY_NAME_KEY)
                }

                set keyLength [string length $keyName]

                if {$keyLength > $maxKeyLength} {
                    set maxKeyLength $keyLength
                }
            }
            
            # Now print each object in the map
            
            set index 1
            set keyListLength [llength $keyList]

            foreach keyObj $keyList {

                array set key [STAF::datatype getValue $keyObj]
                set keyName $key(key)

                if {[array names key $STAF::DISPLAY_NAME_KEY] != ""} {
                    set keyName $key($STAF::DISPLAY_NAME_KEY)
                }

                set indent [expr ($indentLevel * $STAF::INDENT_DELTA) - 1]
                append output [string range $STAF::SPACES 0 $indent]
                
                append output $keyName
                set indent [expr $maxKeyLength - [string length $keyName] - 1]
                append output [string range $STAF::SPACES 0 $indent]
                append output ": "

                set theObj $map($key(key))

                if {[STAF::datatype getType $theObj] == $STAF::ListType || \
                    [STAF::datatype getType $theObj] == $STAF::MapType || \
                    [STAF::datatype getType $theObj] == $STAF::ContextType} {

                    append output [eval {STAF::formatObject} \
                        -context {$context} -indentLevel $indentLevel \
                        {$theObj}]
                
                } elseif {[STAF::datatype getType $theObj] == $STAF::NoneType} {
                    append output $STAF::NONE_STRING

                } else {
                    append output [STAF::datatype getValue $theObj]
                }

                if {$index < $keyListLength} {
                    append output $STAF::ENTRY_SEPARATOR
                }

                append output $lineSep

                incr index 1
            }
        } else {

            # Determine maximum key length

            set maxKeyLength 0

            foreach key [array names map] {

                set keyLength [string length $key]
        
                if {$keyLength > $maxKeyLength} {
                    set maxKeyLength $keyLength
                }
            }

            # Now print each object in the map
        
            set index 1

            foreach key [array names map] {
            
                set keyLength [string length $key]

                set indent [expr ($indentLevel * $STAF::INDENT_DELTA) - 1]
                append output [string range $STAF::SPACES 0 $indent]
                append output $key
                set indent [expr $maxKeyLength - $keyLength - 1]
                append output [string range $STAF::SPACES 0 $indent]
                append output ": "

                set theObj $map($key)

                if {[STAF::datatype getType $theObj] == $STAF::ListType || \
                    [STAF::datatype getType $theObj] == $STAF::MapType || \
                    [STAF::datatype getType $theObj] == $STAF::ContextType} {

                    append output [eval {STAF::formatObject} \
                        -context {$context} -indentLevel $indentLevel \
                        {$theObj}]

                } elseif {[STAF::datatype getType $theObj] == $STAF::NoneType} {
                    append output $STAF::NONE_STRING

                } else {
                    append output [STAF::datatype getValue $theObj]
                }

                if {$index < $mapSize} {
                    append output $STAF::ENTRY_SEPARATOR
                }

                append output $lineSep
            
                incr index 1
            }
        }

        incr indentLevel -1

        if {$mapSize > 0} {
            set indent [expr ($indentLevel * $STAF::INDENT_DELTA) - 1]
            append output [string range $STAF::SPACES 0 $indent]
        }

        append output "\}"

    } elseif {[STAF::datatype getType $obj] == $STAF::ContextType} {

        return [eval {STAF::formatObject} \
            -context {$obj} -indentLevel $indentLevel \
            {[STAF::mcontext getRootObject $obj]}]

    } elseif {[STAF::datatype getType $obj] == $STAF::NoneType} {

         return $STAF::NONE_STRING

    } else {

        # Treat this as a scalar

        return [STAF::datatype getValue $obj]
    }

    return $output
}
