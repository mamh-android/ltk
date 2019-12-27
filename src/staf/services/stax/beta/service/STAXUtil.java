/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.util.StringTokenizer;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

/* Jython 2.1: Needs the following line
import org.python.core.__builtin__;
*/
// Jython 2.5: Needs the following 2 lines
import org.python.core.Py;
import org.python.core.CompileMode;

import org.python.core.PyCode;
import org.python.core.PyException;

import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;

/**
 *  STAXUtil - This class provides STAX utility functions
 */
public class STAXUtil 
{
    /**
     * Accepts Python code as niput and removes any white space from the
     * beginning of new lines and compiles the Python code to validate its
     * syntax and returns valid Python code.
     * 
     * @param value A string containing Python code to be parsed for Python
     * and compiled to validate its syntax.
     * 
     * @param action A STAXActionDefaultImpl object that references the
     * Python code.  It's used if an error occurs compiling the Python code
     * to provide additional information such as line number, xml file name,
     * the element in error, the attribute in error, etc.
     * 
     * @return String containing the valid Python code with white space
     * removed from the beginning of new lines.
     */
    public static String parseAndCompileForPython(String value,
                                                  STAXActionDefaultImpl action)
        throws STAXPythonCompileException
    {
        String parsedCode = parseForPython(value);

        try
        {
            compileForPython(parsedCode);
        }
        catch (STAXPythonCompileException e)
        {
            throw new STAXPythonCompileException(
                STAXUtil.formatErrorMessage(action) + "\n" + e.getMessage());
        }

        return parsedCode;
    }

    /**
     * Accepts Python code as niput and removes any white space from the
     * beginning of new lines and compiles the Python code to validate its
     * syntax and returns valid Python code.
     * 
     * @param value A string containing Python code to be parsed for Python
     * and compiled to validate its syntax.
     * 
     * @param errorInfo A string containing additional error information to
     * be prepended to the error message if a STAXPythonCompileException
     * occurs.  Allows you to identify the xml element (and attribute, if
     * applicable) containing the invalid Python code             .
     * 
     * @return String containing the valid Python code with white space
     * removed from the beginning of new lines.
     */
    public static String parseAndCompileForPython(String value,
                                                  String errorInfo)
        throws STAXPythonCompileException
    {
        String parsedCode = parseForPython(value);

        try
        {
            compileForPython(parsedCode);
        }
        catch (STAXPythonCompileException e)
        {
            throw new STAXPythonCompileException(errorInfo + "\n" +
                                                 e.getMessage());
        }

        return parsedCode;
    }


    /**
     *  Accepts Python code as input and removes any white space from the
     * beginning of new lines and returned the parsed Python code.
     * 
     * @param value A string containing python code to be parsed for Python
     * 
     * @return String containing Python code with white space removed from
     * the beginning of new lines
     */
    public static String parseForPython(String value)
    {
        StringBuffer parsedValue = new StringBuffer();
        int pos = 0;     // Position of first non-whitespace character
                         // in 1st line, taking into account tabs.

        // Skip any beginning lines that are just white space
        //  (including tabs, carriage returns, etc.)

        String line;
        boolean firstLine = true;
        BufferedReader br = new BufferedReader(new StringReader(value));
        StringTokenizer st;
        
        try
        {
            while ((line = br.readLine()) != null)
            {
                if (firstLine)
                {
                    // Find the number of leading whitespace characters.
                    // Go through the string character by character, and expand 
                    // each tab to the appropriate number of spaces required to 
                    // reach the next tab stop (set at intervals of 8 characters).

                    st = new StringTokenizer(line);
                    if (st.hasMoreTokens())
                    {
                        //Found first line that isn't just whitespace
                        firstLine = false;
                        pos = 0;
                        int len = line.length();
                        int i = 0;
                        
                        // Append 8 spaces for each leading tab character.
                        for (; i < len && line.charAt(i) == '\t'; i++)
                            pos+=8;

                        // For remaining tabs, add enough spaces to get to the next
                        // multiple of 8 (until reach a non-whitespace character).
                        for (; i < len; i++)
                        {  
                            char c = line.charAt(i); 
                            if (c == ' ')
                                pos++;
                            else if (c == '\t')
                            {
                                do
                                    pos++;
                                while (pos % 8 != 0);
                            }
                            else  // non-whitespace character found
                                break;
                        }
                        parsedValue.append(line.substring(i));
                    }
                }
                else
                {  
                    // Remove the number of leading white space characters
                    //  found in the 1st line (pos) from each additional line
                    //  if possible (but don't remove any non-whitespace chars).

                    int pos2 = 0;
                    int len = line.length();
                    int i = 0;

                    for (; i < len && pos2 <= pos; i++)
                    {
                        char c = line.charAt(i);
                        if (c == ' ')
                            pos2++;
                        else if (c == '\t')
                        {
                            do
                                pos2++;
                            while (pos2 % 8 != 0);
                        }
                        else  // non-whitespace character found
                            break;
                    } 
                    parsedValue.append("\n");

                    // Add leading blanks, if any.
                    while (pos2 > pos)
                    {
                        parsedValue.append(" ");
                        pos2--;
                    }
                    parsedValue.append(line.substring(i));
                }
            }
        }
        catch (Exception e)
        {
            System.out.println(e);
            return value;
        }
        finally
        {
            try
            {
                br.close();
            }
            catch (IOException e)
            {
                System.out.println(e);
                return value;
            }
        }

        return parsedValue.toString();
    }

    /** 
     * Accepts parsed Python code as input and compiles the Python code to
     * validate its syntax.  Throws a STAXPythonCompileException if an error
     * occurs compiling the Python code.
     * 
     * @param value A string containing parsed python code to be compiled by
     * Python to validate its syntax.
     */
    public static void compileForPython(String parsedCode)
        throws STAXPythonCompileException
    {
        try
        {
            /* Jython 2.1:
            PyCode code = __builtin__.compile(parsedCode, "<string>", "exec");
            */
            // Jython 2.5:
            Py.compile_flags(
                parsedCode, "<string>",
                CompileMode.exec, Py.getCompilerFlags());
        }
        catch (Exception e)
        {
            throw new STAXPythonCompileException(
                "Python code compile failed for:\n" + parsedCode +
                "\n\n" + e.toString());
        }
    }

    /** 
     * Generates a formatted error message the contains important information
     * such as the line number of the action that failed, the element name
     * of the action that failed.
     * @param action The action that is in error.
     */ 
    public static String formatErrorMessage(STAXActionDefaultImpl action)
    {
        STAXElementInfo info = action.getElementInfo();

        if (info.getElementName() == null)
        {
            info.setElementName(action.getElement());
        }

        StringBuffer errMsg = new StringBuffer("File: ");
         
        errMsg.append(action.getXmlFile()).append(", Machine: ").append(
            action.getXmlMachine());

        errMsg.append("\nLine ").append(action.getLineNumber(
            info.getElementName(), info.getElementIndex())).append(": ");

        if ((info.getElementName() != null) &&
            (info.getAttributeName() == null))
        {
            errMsg.append("Error in element type \"").append(
                info.getElementName()).append("\".");
        }
        else if ((info.getElementName() != null) &&
                 (info.getAttributeName() != null))
        {
            errMsg.append("Error in attribute \"").append(
                info.getAttributeName()).append(
                    "\" associated with element type \"").append(
                        info.getElementName()).append("\".");
        }

        if (info.getErrorMessage() != null)
        {
            errMsg.append("\n\n").append(info.getErrorMessage());
        }

        return errMsg.toString();
    }

    /**
     * Generates a formatted string for an action.  It contains the element
     * name of the action, any info provided by the action's getInfo() method,
     * line number of the element, the XML file containing the element, and
     * the machine where the XML file esided.
     * 
     * @param action The action representing the element being executed
     * 
     * @return String containing a formatted string containing information
     * about an action
     */ 
    public static String formatActionInfo(STAXActionDefaultImpl action)
    {
        StringBuffer actionInfo = new StringBuffer(action.getElement());

        if ((action.getInfo() != null) && (action.getInfo().length() > 0))
        {
            actionInfo.append(": ").append(action.getInfo());
        }

        actionInfo.append(" (Line: ").append(action.getLineNumber()).append(
            ", File: ").append(action.getXmlFile()).append(
                ", Machine: ").append(action.getXmlMachine()).append(")");

        return actionInfo.toString();
    }

    /** Get line number by getting value of attribute _ln for the input node
     * @param node a map of the attributes for an element in the DOM tree
     * @return a striong containing the line number
     */
    public static String getLineNumberFromAttrs(NamedNodeMap attrs)
    {
        String lineNumber = "Unknown";

        Node thisAttr = attrs.getNamedItem("_ln");

        if (thisAttr != null)
            lineNumber = thisAttr.getNodeValue();

        return lineNumber;
    }


    /**
     * Converts a STAF request number that is > Integer.MAX_VALUE to a
     * negative number.
     * 
     * Doing this for now to handle STAF request numbers > integer.MAX_VALUE
     * so as not to impact STAX extensions that implement the
     * STAXSTAFRequestCompleteListener interface's requestComplete() method
     * which passes requestNumber as an int.
     * 
     * In a future major STAX release, we may change the requestComplete()
     * method to represent requestNumber as a long instead of an int instead
     * of converting it to a negative number.
     * 
     * @param requestNumberString a string containing the STAF Request Number
     * 
     * @return an Integer object that contains the converted request number
     */
    public static Integer convertRequestNumber(String requestNumberString)
        throws NumberFormatException
    {
        try
        {
            return new Integer(requestNumberString);                                        
        }
        catch (NumberFormatException e)
        {
            try
            {
                long longRequestNumber = new Long(requestNumberString).
                    longValue();

                if (longRequestNumber > Integer.MAX_VALUE)
                {
                    int requestNumber = -1 *
                        ((int)(longRequestNumber - Integer.MAX_VALUE));

                    return new Integer(requestNumber);
                }
                else
                {
                    throw e;
                }
            }
            catch (NumberFormatException e2)
            {
                throw e2;
            }
        }
    }

    /**
     * This method gets the short name for a class by removing the STAX
     * package name at the beginning of the class name (if present) and by
     * removing the specified suffix at the end of the class name (if present)
     * 
     * @param longClassName A string containing a STAX class name
     * @param classSuffix A string containing a suffix to be removed from the
     * long class name
     * 
     * @return String containing the "short" class name
     */
    public static String getShortClassName(String longClassName,
                                           String classSuffix)
    {
        String className = longClassName;

        if (className.startsWith(STAX.PACKAGE_NAME))
            className = className.substring(STAX.PACKAGE_NAME.length());

        if (className.endsWith(classSuffix))
        {
            className = className.substring(
                0, className.length() - classSuffix.length());
        }

        return className;
    }
    
    /**
     * Normalizes a file path.
     * 
     * @param path the file path to be normalized
     * @param fileSep the file separator for the machine where the file resides
     * @return a normalized file path if the path is valid; otherwise, the
     *  original path is returned
     */
    public static String normalizeFilePath(String path, String fileSep)
    {
        String normalized = path;

        if ((path == null) || (path.length() == 0))
        {
            return path;
        }

        if (fileSep.equals("\\"))
        {
            // Windows path
        
            // Check for windows drive letter

            char driveLetter = path.charAt(0);
            boolean isWindowsDrivePath = path.length() > 1 &&
                path.charAt(1) == ':' &&
                (driveLetter >= 'a' && driveLetter <= 'z') ||
                (driveLetter >= 'A' && driveLetter <= 'Z');
                
            if (isWindowsDrivePath)
            {
                // Path starts with Windows drive letter + :

                String uriPath = path.replace('\\', '/');

                try
                {
                    URI uri = new URI("file:/" + uriPath);
                    uri = uri.normalize();

                    // Need to remove the leading forward slash
                    normalized = uri.getPath().substring(1);

                    // Since Windows, use \ instead of / for the file separator
                    normalized = normalized.replace('/', '\\');
                }
                catch (URISyntaxException e)
                {
                    return path;
                }
            }
            else if (path.charAt(0) == '\\' || path.charAt(0) == '/')
            {
                // Check if Windows UNC path, e.g. \\test.ibm.com\folder1

                boolean isUNC = path.startsWith("\\\\");
                String uncServer = null;
                String uriPath = path.replace('\\', '/');
                
                if (isUNC)
                {
                    // Extract the server name
                    int index = uriPath.indexOf('/', 2);
                
                    if (index == -1)
                        return path;
                
                    uncServer = uriPath.substring(2, index);
                
                    // The file path to be normalized is the remainder of
                    // the path
                    uriPath = uriPath.substring(index);
                }
            
                try
                {
                    URI uri = new URI("file:" + uriPath);
                    uri = uri.normalize();
                    normalized = uri.getPath();
                }
                catch (URISyntaxException e)
                {
                    return path;
                }
            
                if (isUNC)
                {
                    // Add the \\ and server name back to the normalized UNC
                    // file path
                    normalized = "\\\\" + uncServer +
                        normalized.replace('/', '\\');
                }
            }
            else
            {
                // Relative path (doesn't start with / or \ or a Windows
                // drive letter)

                String uriPath = path.replace('\\', '/');

                try
                {
                    URI uri = new URI("file:/" + uriPath);
                    uri = uri.normalize();
                
                    // Need to remove the leading forward slash
                    normalized = uri.getPath().substring(1);
                }
                catch (URISyntaxException e)
                {
                    return path;
                }
            }
        }
        else
        {
            // Unix path
            
            String uriPath = path;

            // Replace any backslashes in the path with the percent encoded
            // string for a backslash, %5C, as can't construct a URI for a
            // path containing a backslash

            int backslashIndex = path.indexOf('\\');

            if (backslashIndex > -1)
            {
                uriPath = uriPath.replaceAll("\\\\", "%5C");
            }
            
            // Determine if it's an absolute or normal path

            if (uriPath.charAt(0) == '/')
            {
                // Absolute path

                try
                {
                    URI uri = new URI("file:" + uriPath);
                    uri = uri.normalize();
                    normalized = uri.getPath();
                }
                catch (URISyntaxException e)
                {
                    return path;
                }
            }
            else
            {
                // Relative path (doesn't start with /)

                try
                {
                    URI uri = new URI("file:/" + uriPath);
                    uri = uri.normalize();
                
                    // Need to remove the leading forward slash
                    normalized = uri.getPath().substring(1);
                }
                catch (URISyntaxException e)
                {
                    return path;
                }
            }
        }
        
        if (normalized == null)
        {
            return path;
        }
        
        return normalized;
    }

    public static boolean isRelativePath(String path, String fileSeparator)
    {
        // Check if a relative or absolute path was specified.
        // - On Unix, an absolute path starts with a slash or a tilde (~)
        //   and a relative path does not.
        // - On Windows, an absolute path starts with a slash or a backslash,
        //   or with a drive specification, 'x:/' or 'x:\', where x is the
        //   drive letter.  Also, even if 'x:' is not followed by a / or \,
        //   assume absolute since we won't be able to prepend a parent
        //   directory
        // - Also, assume it's an absolute file name if it's null or an empty
        //   string, regardless of operating system.
        // If it isn't an absolute path, it's a relative path.

        if ((path == null) || (path.length() == 0))
        {
            // Assume absolute file name
            return false;
        }

        if (fileSeparator.equals("\\"))
        {
            // Windows path

            if ((path.charAt(0) == '\\') || (path.charAt(0) == '/'))
            {
                // Absolute file name because starts with a slash or backslash
                return false;
            }
            else if ((path.length() > 1) &&
                     (path.charAt(1) == ':') &&
                     ((path.charAt(0) >= 'a' && path.charAt(0) <= 'z') ||
                      (path.charAt(0) >= 'A' && path.charAt(0) <= 'Z')))
            {
                // Absolute file name because starts with a drive specification
                return false;
            }
        }
        else
        {
            // Unix path

            if ((path.charAt(0) == '/') || (path.charAt(0) == '~'))
            {
                // Absolute file name because starts with a slash or tilde
                return false;
            }
        }

        // File name is a relative path since its not an absolute path

        return true;
    }

    /**
     * Returns the parent path for the specified file name.
     * @param A string containing the file name
     * @param A string containing the file separator for the operating system
     * of the machine where the file resides
     */ 
    public static String getParentPath(String fileName, String fileSeparator)
    {
        int endParentPathIndex = -1;

        if (fileSeparator.equals("\\"))
        {
            // Windows filename

            // Check if the file name ends in a \ or / (and isn't just \ or /)
            // and if so, remove the trailing file separator

            if ((fileName.length() > 1) &&
                ((fileName.endsWith("\\")) || (fileName.endsWith("/"))))
            {
                fileName = fileName.substring(0, fileName.length() - 1);
            }

            // Find the last slash or backslash in the file name and assign
            // everything before it, including the last slash or backslash

            int lastSlashIndex = fileName.lastIndexOf('/');
            int lastBackslashIndex = fileName.lastIndexOf('\\');

            if (lastSlashIndex >= lastBackslashIndex)
                endParentPathIndex = lastSlashIndex;
            else
                endParentPathIndex = lastBackslashIndex;
        }
        else
        {
            // Unix filename

            // Check if the file name ends with the file separator (and isn't
            // just "/") and if so, remove the trailing file separator

            if ((fileName.length() > 1) && (fileName.endsWith(fileSeparator)))
            {
                fileName = fileName.substring(0, fileName.length() - 1);
            }

            // Find the last slash in the file name and assign everything
            // before it, including the last slash

            endParentPathIndex = fileName.lastIndexOf('/');
        }
        
        if (endParentPathIndex == -1)
            return "";
        
        return fileName.substring(0, endParentPathIndex + 1);
    }
}
