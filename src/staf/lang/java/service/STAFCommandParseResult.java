/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service;

import java.util.*;

// STAFCommandParseResult - This class contains the results of parsing a
//                          command string with the STAFCommandParser.
//
// This class provides the following instance data
//
//   rc - This indicates whether the command was parsed successfully.  Zero
//        indicates a successful parse.  Non-zero indicates an error.
//
//   errorBuffer - If rc is non-zero, this will contain a textual description
//                 of the error.
//
// This class provides the following methods
//
//   optionTimes - This returns the number of times a particular option was
//                 specified.
//
//   optionValue - This returns the value of a specific instance of an option.
//                 If the given instance does not exist, an empty string is
//                 returned.
//
//   numInstances - This returns the total number of options specified.
//
//   instanceName - Returns the name of the option for the given instance.
//
//   instanceValue - Returns the value of option for the given instance.
//
//   numArgs - Returns the total number of arguments specified.
//
//   arg - Returns the specified argument.

public class STAFCommandParseResult
{
    STAFCommandParseResult()
    {
        this(false);
    }

    STAFCommandParseResult(boolean caseSensitive)
    {
        fCaseSensitive = caseSensitive;
        fOptionInstances = new Vector();
        fArgs = new Vector();

        rc = 0;
        errorBuffer = new String();
    }

    public int rc;
    public String errorBuffer;

    public int optionTimes(String name)
    {
        int count = 0;

        for(int i = 0; i < fOptionInstances.size(); ++i)
        {
            OptionInstance instance =
                           (OptionInstance)fOptionInstances.elementAt(i);

            if ((fCaseSensitive && instance.name.equals(name)) ||
                (!fCaseSensitive && instance.name.equalsIgnoreCase(name)))
            {
                ++count;
            }
        }

        return count;
    }

    public String optionValue(String name)
    {
        return optionValue(name, 1);
    }

    public String optionValue(String name, int instanceNumber)
    {
        int count = 0;

        for(int i = 0; i < fOptionInstances.size(); ++i)
        {
            OptionInstance instance =
                           (OptionInstance)fOptionInstances.elementAt(i);

            if ((fCaseSensitive && instance.name.equals(name)) ||
                (!fCaseSensitive && instance.name.equalsIgnoreCase(name)))
            {
                if (++count == instanceNumber)
                    return instance.value;
            }
        }

        return new String();
    }

    public int numInstances() { return fOptionInstances.size(); }

    public String instanceName(int instanceNumber)
    {
        OptionInstance instance =
            (OptionInstance)fOptionInstances.elementAt(instanceNumber - 1);

        return instance.name;
    }

    public String instanceValue(int instanceNumber)
    {
        OptionInstance instance =
            (OptionInstance)fOptionInstances.elementAt(instanceNumber - 1);

        return instance.value;
    }

    public int numArgs() { return fArgs.size(); }

    public String arg(int argNumber)
    {
        return (String)fArgs.elementAt(argNumber - 1);
    }


    // These methods are used by the STAFCommandParser class to add the
    // various options and arguments to the parse result.

    void addOptionInstance(OptionInstance instance)
    {
        fOptionInstances.addElement(instance);
    }

    void addArg(String value)
    {
        fArgs.addElement(value);
    }


    // Instance data

    private boolean fCaseSensitive;
    private Vector fOptionInstances;
    private Vector fArgs;


    // This class is used to represent a given instance of an option.

    class OptionInstance
    {
        OptionInstance()
        {
            this(new String(), new String());
        }

        OptionInstance(String theName, String theValue)
        {
            name = theName;
            value = theValue;
        }

        String name;
        String value;
    }
}
