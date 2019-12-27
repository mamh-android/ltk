/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service;

import java.text.*;
import java.util.*;

// STAFCommandParser - This class implements the standard STAF parser used by
//                     all STAF services.

public class STAFCommandParser
{
    public STAFCommandParser()
    {
        this(0, false);
    }

    public STAFCommandParser(int maxArgs)
    {
        this(maxArgs, false);
    }

    public STAFCommandParser(int maxArgs, boolean caseSensitive)
    {
        fMaxArgs = maxArgs;
        fCaseSensitive = caseSensitive;
        fOptions = new Vector();
        fOptionGroups = new Vector();
        fOptionNeeds = new Vector();
    }

    public static final int VALUEREQUIRED = 0;
    public static final int VALUENOTALLOWED = 1;
    public static final int VALUEALLOWED = 2;

    public void addOption(String name, int maxAllowed, int valueRequirement)
    {
        fOptions.addElement(new Option(name, maxAllowed, valueRequirement));
    }

    public void addOptionGroup(String optionNames, int min, int max)
    {
        fOptionGroups.addElement(new OptionGroup(optionNames, min,max));
    }

    public void addOptionNeed(String needers, String needees)
    {
        fOptionNeeds.addElement(new OptionNeed(needers, needees));
    }

    public STAFCommandParseResult parse(String data)
    {
        STAFCommandParseResult result =
                               new STAFCommandParseResult(fCaseSensitive);
        Vector wordList = new Vector();
        OptionValue optionValue = new OptionValue();
        char currentChar;
        boolean inQuotes = false;
        boolean inEscape = false;
        boolean isLiteral = false;
        boolean inLengthField = false;
        boolean inDataField = false;
        int dataLength = 0;

        for(int i = 0; i < data.length(); ++i)
        {
            currentChar = data.charAt(i);

            if ((currentChar == ':') && !inQuotes && !inEscape &&
               !inDataField && optionValue.data.length() == 0)
            {
                inLengthField = true;
            }
            else if (inLengthField)
            {
                if (currentChar == ':')
                {
                    inLengthField = false;
                    inDataField = true;

                    if (optionValue.data.length() == 0)
                    {
                        result.errorBuffer = "Invalid length delimited data " +
                                             "specifier";
                        result.rc = 1;
                        return result;
                    }

                    dataLength = new Integer(
                        optionValue.data.toString()).intValue();
                    optionValue = new OptionValue();
                }
                else if (Character.isDigit(currentChar))
                {
                    optionValue.data.append(currentChar);
                }
                else
                {
                    result.errorBuffer = "Invalid length delimited data " +
                                         "specifier";
                    result.rc = 1;
                    return result;
                }
            }
            else if (inDataField)
            {
                // Need to check if length > 0 to handle :0: case

                if (dataLength > 0)
                {
                    optionValue.data.append(currentChar);
                }

                if (--dataLength <= 0)
                {
                    wordList.addElement(optionValue);
                    optionValue = new OptionValue();
                    inDataField = false;
                }
            }
            else if (Character.isWhitespace(currentChar))
            {
                inEscape = false;

                if (inQuotes) optionValue.data.append(currentChar);
                else if (optionValue.data.length() != 0)
                {
                    if (isLiteral)
                        optionValue.optionOrValue = ISVALUE;
                    else if (isOption(optionValue.data))
                        optionValue.optionOrValue = ISOPTION;

                    wordList.addElement(optionValue);
                    optionValue = new OptionValue();
                    isLiteral = false;
                }
                else continue;
            }
            else if (currentChar == '\\')
            {
                if (inQuotes && !inEscape)
                {
                    inEscape = true;
                    continue;
                }
                else
                {
                    optionValue.data.append(currentChar);
                    inEscape = false;
                }
            }
            else if (currentChar == '"')
            {
                if (inEscape) optionValue.data.append(currentChar);
                else if (inQuotes && optionValue.data.length() != 0)
                {
                    if (isLiteral)
                        optionValue.optionOrValue = ISVALUE;
                    else if (isOption(optionValue.data))
                        optionValue.optionOrValue = ISOPTION;

                    wordList.addElement(optionValue);
                    optionValue = new OptionValue();
                    inQuotes = false;
                    isLiteral = false;
                }
                else
                {
                    inQuotes = true;
                    isLiteral = true;
                }

                inEscape = false;

            }  // end if quote character
            else
            {
                inEscape = false;
                optionValue.data.append(currentChar);
            }

        }  // for each character in parseString
        
        // Handle case where last option's value is :0:

        if (inDataField && dataLength == 0)
        {
            wordList.addElement(optionValue);
            optionValue = new OptionValue();
            inDataField = false;
        }
        
        if (inLengthField || inDataField)
        {
            result.errorBuffer = "Invalid length delimited data specifier";
            result.rc = 1;
            return result;
        }
        else if (optionValue.data.length() != 0)
        {
            if (isLiteral)
                optionValue.optionOrValue = ISVALUE;
            else if (isOption(optionValue.data))
                optionValue.optionOrValue = ISOPTION;

            wordList.addElement(optionValue);
        }

        // Now walk the word list looking for options, etc.

        STAFCommandParseResult.OptionInstance optionInstance =
            result.new OptionInstance();
        int valueRequirement = VALUENOTALLOWED;

        for(int i = 0; i < wordList.size(); ++i)
        {
            OptionValue currOptionValue = (OptionValue)wordList.elementAt(i);
            StringBuffer currentWord = currOptionValue.data;

            if (currOptionValue.optionOrValue == ISOPTION)
            {
                Option option = getOption(currentWord);

                if (valueRequirement == VALUEREQUIRED)
                {
                    result.errorBuffer = "Option, " + optionInstance.name +
                                         ", requires a value";
                    result.rc = 1;
                    return result;
                }
                else if (valueRequirement == VALUEALLOWED)
                {
                    result.addOptionInstance(optionInstance);
                }

                // Check once here for whether this new option instance will
                // exceed the limit for this option

                if ((result.optionTimes(option.name) == option.maxAllowed) &&
                   (option.maxAllowed != 0))
                {
                    result.errorBuffer = "You may have no more than " +
                                         option.maxAllowed +
                                         " instances of option " + option.name;
                    result.rc = 1;
                    return result;
                }

                optionInstance = result.new OptionInstance();
                optionInstance.name = currentWord.toString();
                valueRequirement = option.valueRequirement;

                if (valueRequirement == VALUENOTALLOWED)
                {
                    result.addOptionInstance(optionInstance);
                    optionInstance = result.new OptionInstance();
                }
            }
            else if (valueRequirement == VALUENOTALLOWED)
            {
                result.addArg(currentWord.toString());
            }
            else
            {
                optionInstance.value = currentWord.toString();
                result.addOptionInstance(optionInstance);

                optionInstance = result.new OptionInstance();
                valueRequirement = VALUENOTALLOWED;
            }

        }  // end for each word

        // If the last word was an option, we need to check for its value
        // requirements here

        if (valueRequirement == VALUEREQUIRED)
        {
            result.errorBuffer = "Option, " + optionInstance.name +
                                 ", requires a value";
            result.rc = 1;
            return result;
        }
        else if (valueRequirement == VALUEALLOWED)
        {
            result.addOptionInstance(optionInstance);
        }
        
        // Now check the restriction on number of arguments

        if (result.numArgs() > fMaxArgs)
        {
            result.errorBuffer = "You may have no more than " + fMaxArgs +
                                 " argument(s).  You specified " +
                                 result.numArgs() + " argument(s)." +
                                 "  The first excess argument is, " +
                                 result.arg(fMaxArgs + 1) + ".";
            result.rc = 1;
            return result;
        }
        
        // Now check all the group requirements

        for(int i = 0; i < fOptionGroups.size(); ++i)
        {
            OptionGroup group = (OptionGroup)fOptionGroups.elementAt(i);
            int groupCount = 0;
            int groupWordCount = group.names.size();

            for (int j = 0; j < groupWordCount; ++j)
            {
                if (result.optionTimes((String)group.names.elementAt(j)) != 0)
                    ++groupCount;
            }

            if ((groupCount < group.min) || (groupCount > group.max))
            {
                result.errorBuffer = "You must have at least " + group.min +
                                     ", but no more than " + group.max +
                                     " of the option(s), " + group.namesString;
                result.rc = 1;
                return result;
            }
        }


        // Now check the need requirements

        for(int i = 0; i < fOptionNeeds.size(); ++i)
        {
            OptionNeed need = (OptionNeed)fOptionNeeds.elementAt(i);

            boolean foundNeeder = false;
            boolean foundNeedee = false;

            for(int j = 0; (j < need.needers.size()) && !foundNeeder; ++j)
            {
                if (result.optionTimes((String)need.needers.elementAt(j)) != 0)
                    foundNeeder = true;
            }

            for(int k = 0; (k < need.needees.size()) && !foundNeedee; ++k)
            {
                if (result.optionTimes((String)need.needees.elementAt(k)) != 0)
                    foundNeedee = true;
            }

            if (foundNeeder && !foundNeedee)
            {
                result.errorBuffer = "When specifying one of the options " +
                                     need.needersString + ", you must also " +
                                     "specify one of the options " +
                                     need.needeesString;
                result.rc = 1;
                return result;
            }
        }

        return result;

    }  // end parse()


    private boolean isOption(StringBuffer name)
    {
        return isOption(name.toString());
    }

    private boolean isOption(String name)
    {
        for(int i = 0; i < fOptions.size(); ++i)
        {
            Option option = (Option)fOptions.elementAt(i);

            if ((fCaseSensitive && option.name.equals(name)) ||
                (!fCaseSensitive && option.name.equalsIgnoreCase(name)))
            {
                return true;
            }
        }

        return false;
    }

    private Option getOption(StringBuffer name)
    {
        return getOption(name.toString());
    }

    private Option getOption(String name)
    {
        for(int i = 0; i < fOptions.size(); ++i)
        {
            Option option = (Option)fOptions.elementAt(i);

            if ((fCaseSensitive && option.name.equals(name)) ||
                (!fCaseSensitive && option.name.equalsIgnoreCase(name)))
            {
                return option;
            }
        }

        return new Option("<Unknown Option>", 1, VALUENOTALLOWED);
    }

    private int fMaxArgs;
    private boolean fCaseSensitive;
    private Vector fOptions;
    private Vector fOptionGroups;
    private Vector fOptionNeeds;


    // This class is used to hold data about each option

    private class Option
    {
        Option(String aName, int theMaxAllowed, int theValueRequirement)
        {
            name = aName;
            maxAllowed = theMaxAllowed;
            valueRequirement = theValueRequirement;
        }

        String name;
        int maxAllowed;
        int valueRequirement;
    }


    // This class is used to hold information about the way options may
    // be grouped

    private class OptionGroup
    {
        OptionGroup(String theNames, int theMin, int theMax)
        {
            min = theMin;
            max = theMax;
            namesString = theNames;
            names = new Vector();

            StringTokenizer tokenizer = new StringTokenizer(theNames);

            while (tokenizer.hasMoreTokens())
            {
                names.addElement(tokenizer.nextToken());
            }
        }

        String namesString;
        Vector names;
        int min;
        int max;
    }


    // This class is used to hold information about which options are
    // required in the procesence of other options

    private class OptionNeed
    {
        OptionNeed(String theNeeders, String theNeedees)
        {
            needersString = theNeeders;
            needers = new Vector();

            StringTokenizer tokenizer = new StringTokenizer(theNeeders);

            while (tokenizer.hasMoreTokens())
            {
                needers.addElement(tokenizer.nextToken());
            }

            needeesString = theNeedees;
            needees = new Vector();

            tokenizer = new StringTokenizer(theNeedees);

            while (tokenizer.hasMoreTokens())
            {
                needees.addElement(tokenizer.nextToken());
            }
        }

        String needersString;
        String needeesString;
        Vector needers;
        Vector needees;
    }


    private static final int ISOPTION = 0;
    private static final int ISVALUE = 1;

    private class OptionValue
    {
        OptionValue()
        {
            data = new StringBuffer();
            optionOrValue = STAFCommandParser.ISVALUE;
        }

        StringBuffer data;
        int optionOrValue;
    }
}
