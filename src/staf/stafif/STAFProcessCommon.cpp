/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFInternalProcess.h"


// A shell command, can contain the following substitution characters (if
// contained in string validSubstitutionChars assigned for Windows or Unix).
//   %c means substitute the command
//   %C means substitute the command, quotified
//   %p means substitute the password
//   %P means substitute the password, quotified
//   %t means substitute the title
//   %T means substitute the title, quotified
//   %u means substitute the user
//   %U means substitute the user, quotified
//   %w means substitute the workload
//   %W means substitute the workload, quotified
//   %x means substitute the command, adding stdin, stdout, stderr redirection
//   %X means substitute the command, adding stdin, stdout, stderr redirection,
//      quotified
//   %% means substitute a % (that is, escape the %)

// -------------------------------------------------------------------------
// Verify that a shell command only contains valid substitution characters.
// Also, verify that it contains %c or %C.
//
// Input:   the shell command
// Output:  return code: kSTAFOk if valid, else invalid
// -------------------------------------------------------------------------
STAFRC_t STAFProcessValidateShellSubstitutionChars(const STAFString &shellCmd)
{
    static STAFString sPercent(kUTF8_PERCENT);
    static STAFString sCmdQChar("C");
    static STAFString sXtermQChar("X");
    static STAFString sSubChars(gSTAFProcessShellSubstitutionChars);
    unsigned int lastPos = 0;
    bool valid = false;  // Indicates if shellCmd contains %c or %C or %X or %x
    
    for (unsigned int currPos = shellCmd.find(sPercent);
         currPos != STAFString::kNPos;
         currPos = shellCmd.find(sPercent, lastPos))
    {
        STAFString substituteChar = shellCmd.subString(currPos + 1, 1,
                                                       STAFString::kChar);

        lastPos = currPos + 1 + substituteChar.length();

        if (sSubChars.find(substituteChar) == STAFString::kNPos)
            return kSTAFInvalidValue;

        if ((substituteChar.toUpperCase() == sCmdQChar) ||
            (substituteChar.toUpperCase() == sXtermQChar))
            valid = true;
    }
    
    if (!valid)
        return kSTAFInvalidValue;

    return kSTAFOk;
}

// -------------------------------------------------------------------------
// Replace substitution characters in a shell command with the appropriate
// values.
//
// Input:   the shell command
// Input:   process substitution data structure containing command,
//          title, and workload, stdin, stdout, and stderr
// Output:  the shell command with the substitutions done
// Output:  return code: kSTAFOk if valid, else invalid
// -------------------------------------------------------------------------
STAFRC_t STAFProcessReplaceShellSubstitutionChars(const STAFString &shellCmd,
             const STAFProcessShellSubstitutionData &data, STAFString &output)
{
    STAFRC_t rc = STAFProcessValidateShellSubstitutionChars(shellCmd);

    if (rc != kSTAFOk) return rc;
    
    // Replace substitution characters

    static STAFString sPercent(kUTF8_PERCENT);
    static STAFString sCmdChar("c");
    static STAFString sCmdQChar("C");
    static STAFString sPasswordChar("p");
    static STAFString sPasswordQChar("P");
    static STAFString sTitleChar("t");
    static STAFString sTitleQChar("T");
    static STAFString sUserChar("u");
    static STAFString sUserQChar("U");
    static STAFString sWorkloadChar("w");
    static STAFString sWorkloadQChar("W");
    static STAFString sXtermChar("x");
    static STAFString sXtermQChar("X");

    unsigned int lastPos = 0;

    for (unsigned int currPos = shellCmd.find(sPercent);
         currPos != STAFString::kNPos;
         currPos = shellCmd.find(sPercent, lastPos))
    {
        output += shellCmd.subString(lastPos, currPos - lastPos);

        STAFString substituteChar = shellCmd.subString(currPos + 1, 1,
                                                       STAFString::kChar);
        lastPos = currPos + 1 + substituteChar.length();

        if (substituteChar == sCmdChar)
        {
            output += data.command;
        }
        else if (substituteChar == sCmdQChar)
        {
            // Quotify command 
            STAFString cmd = "'" + (data.command).replace("'", "'\\''") +
                             "'";
            output += cmd;  
        }
        else if ((substituteChar == sTitleChar) ||
                 (substituteChar == sTitleQChar))
        {
            STAFString title = data.title;

            if (title.length() == 0) title = "<Unknown>";

            if (substituteChar == sTitleQChar)
            {
                // Quotify title
                title = "'" + title.replace("'", "'\\''") + "'";
            }
            output += title;
        }
        else if ((substituteChar == sWorkloadChar) ||
                 (substituteChar == sWorkloadQChar))
        {
            STAFString workload = data.workload;

            if (workload.length() == 0) workload = "<Unknown>";
            
            if (substituteChar == sWorkloadQChar)
            {
                // Quotify workload
                workload = "'" + workload.replace("'", "'\\''") + "'";
            }
            output += workload;
        }
        else if ((substituteChar == sXtermChar) ||
                 (substituteChar == sXtermQChar))
        {
            STAFString xtermCmd = data.command;
            STAFString stdinfile = data.stdinfile;
            STAFString stdoutfile = data.stdoutfile;
            STAFString stderrfile = data.stderrfile;

            if (stdinfile.length() != 0) xtermCmd += " " + stdinfile;
            if (stdoutfile.length() != 0) xtermCmd += " " + stdoutfile;
            if (stderrfile.length() != 0) xtermCmd += " " + stderrfile;

            if (substituteChar == sXtermQChar)
            {
                // Quotify command 
                xtermCmd = "'" + xtermCmd.replace("'", "'\\''") + "'";
            }
            output += xtermCmd;  
        }
        else if ((substituteChar == sUserChar) ||
                 (substituteChar == sUserQChar))
        {
            STAFString username = data.username;

            if (username.length() == 0) username = "";
            
            if (substituteChar == sUserQChar)
            {
                // Quotify username
                username = "'" + username.replace("'", "'\\''") + "'";
            }
            output += username;

        }
        else if ((substituteChar == sPasswordChar) ||
                 (substituteChar == sPasswordQChar))
        {
            STAFString password = data.password;

            if (password.length() == 0) password = "";
            
            if (substituteChar == sPasswordQChar)
            {
                // Quotify username
                password = "'" + password.replace("'", "'\\''") + "'";
            }
            output += password;
        }
        else if (substituteChar == sPercent)
        {
            output += sPercent;
        }
    }

    output += shellCmd.subString(lastPos);

    return kSTAFOk;
}


