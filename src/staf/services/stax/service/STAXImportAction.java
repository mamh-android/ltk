/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;

import java.util.*;

import org.python.core.*;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

import java.io.File;
import java.io.StringReader;
import org.xml.sax.SAXNotRecognizedException;
import org.xml.sax.SAXNotSupportedException;

public class STAXImportAction extends STAXActionDefaultImpl
{
    public static String kIgnore = "ignore";
    public static String kError = "error";

    public STAXImportAction()
    { /* Do Nothing */ }

    public STAXImportAction(String machine, String file, String mode)
    { 
        fUnevalMachine = machine;
        fMachine = machine;
        fUnevalFile = file;
        fFile = file;
        fUnevalMode = mode;
        fMode = mode;
    }

    public STAXImportAction(String machine, String file, String mode,
                            String replace)
    { 
        fUnevalMachine = machine;
        fMachine = machine;
        fUnevalFile = file;
        fFile = file;
        fUnevalMode = mode;
        fMode = mode;
        fUnevalReplace = replace;
    }

    public String getMachine() { return fMachine; } 

    public void setMachine(String machine) 
    {
        fUnevalMachine = machine; 
        fMachine = machine; 
    }
    
    public String getFile() { return fFile; } 

    public void setFile(String file) 
    {
        fUnevalFile = file; 
        fFile = file; 
    }

    public String getDirectory() { return fDirectory; } 

    public void setDirectory(String directory) 
    {
        fUnevalDirectory = directory; 
        fDirectory = directory; 
    }
    
    public String getMode() { return fMode; } 

    public void setMode(String mode)
    {
        fUnevalMode = mode; 
        fMode = mode; 
    }
    
    public boolean getReplace() { return fReplace; }
    
    public void setReplace(String replace)
    {
        fUnevalReplace = replace;
    }

    public String getImportInclude() { return fImportInclude; }

    public void setImportInclude(String importInclude)
    {
        fUnevalImportInclude = importInclude; 
        fImportInclude = importInclude; 
    }

    public String getImportExclude() { return fImportExclude; }

    public void setImportExclude(String importExclude)
    {
        fUnevalImportExclude = importExclude; 
        fImportExclude = importExclude; 
    }
    
    public String getInfo()
    {
        if (fDirectory != null)
        {
            return fMachine + ":" + fDirectory + ":" + fReplace + ":" +
                fMode + ":" + fImportExclude;
        }
        else
        {
            return fMachine + ":" + fFile + ":" + fReplace + ":" +
                fMode + ":" + fImportExclude;
        }
    }
    
    public String getDetails()
    {
        return "Machine:" + fMachine + 
               ";File:" + fFile + 
               ";Directory:" + fDirectory +
               ";Replace:" + fReplace +
               ";Mode:" + fMode +
               ";Include:" + fImportInclude +
               ";Exclude:" + fImportExclude;
    }
    
    public String getXMLInfo()
    {
        String xmlInfo = "<import";

        if (fUnevalDirectory == null)
            xmlInfo += " file=\"" + fFile + "\"";
        else
            xmlInfo += " directory=\"" + fDirectory + "\"";

        if (fUnevalMachine != null)
            xmlInfo += " machine=\"" + fMachine + "\"";
        
        if (!fUnevalReplace.equals("0"))
        {
            xmlInfo += " replace=\"" + fUnevalReplace + "\"";
        }

        xmlInfo += " mode=\"" + fMode + "\"";

        xmlInfo += ">";
            
        if (!(fImportInclude.equals("")))
        {
            xmlInfo += "\n  <import-include>" + fImportInclude + 
                "</import-include>";
        }
        
        if (!(fImportExclude.equals("")))
        {
            xmlInfo += "\n  <import-exclude>" + fImportExclude + 
                "</import-exclude>";
        }

        xmlInfo += "\n</import>";
               
        return xmlInfo;
    }
    
    public void execute(STAXThread thread)
    {
        /* 
        If the "file" attribute is specified for the import element (so
        that it is processing 1 file), STAXResult will be set to a list
        containing:
            
        - None (if the file was imported successfully), or a list containing
          a STAXImportError object and a string with details about the error.
             
        - A list of the successfully imported functions that were requested
          to be imported.
                 
        - A list of the successfully imported functions that were required
          by other functions.
                 
        - A list of the functions that were requested to be imported but
          already existed (so they were not imported).
                 
        - A list of the functions that were required by other functions but
          already existed (so they were not imported).
              
        - A list of the functions there were not requested to be imported
          and were not required by other functions.
                 
        - A list of functions requested to be imported that were not found.
                 
        If the "directory" attribute is specified for the import element
        (so that it is processing any number of files), STAXResult will be
        set to a list containing:
           
        - None (if all xml files in the directory were imported successfully),
          or a list containing a STAXImportDirectoryError object and a string
          with details about the error.
        
        - A list with information about the xml files that were successfully
          imported and/or error messages about xml files that were not
          successfully imported.  Each entry in this list will be set to
          list containing:
          
          - The file name (for an xml file in the directory that was
            successfully imported), or a list containing a STAXImportError
            object and a text string with details about the error.
                
          - A list of the successfully imported functions that were requested
            to be imported.  
                
          - A list of the successfully imported functions that were required
            by other functions.
                 
          - A list of the functions that were requested to be imported but
            already existed (so they were not imported).
                 
          - A list of the functions that were required by other functions but
            already existed (so they were not imported).
                 
          - A list of the functions there were not requested to be imported
            and were not required by other functions.
                 
          - A list of functions requested to be imported that were not found.
        */            
    
        fThread = thread;
        
        String evalElem = getElement();
        String evalAttr = "machine";
        
        // Indicates whether to continue processing if an error occurs
        boolean continueFlag = false;

        try
        {
            boolean machineSpecified = false;

            if (fMachine != null)
            {
                machineSpecified = true;
                fMachine = fThread.pyStringEval(fUnevalMachine);
            }
            else
            {
                // Machine attribute is not specified
                fMachine = fThread.pyStringEval("STAXCurrentXMLMachine");
            }

            if (fUnevalFile != null)
            {
                evalAttr = "file";
                fFile = fThread.pyStringEval(fUnevalFile);
            }
            
            boolean directorySpecified = false;

            if (fUnevalDirectory != null)
            {
                directorySpecified = true;
                evalAttr = "directory";
                fDirectory = fThread.pyStringEval(fUnevalDirectory);
            }

            evalAttr = "replace";
            fReplace = thread.pyBoolEval(fUnevalReplace);
            
            evalAttr = "mode";
            fMode = fThread.pyStringEval(fUnevalMode);

            if (fMode.equalsIgnoreCase("error"))
            {
                fMode = "error";
            }
            else if (fMode.equalsIgnoreCase("ignore"))
            {
                fMode = "ignore";
            }
            else
            {
                // Invalid mode
                fThread.popAction();

                String errorData = "[STAXImportModeError, r'" + 
                    "Invalid import mode: " + fMode + "']";

                PyObject signalData = fThread.pyObjectEval(errorData);
                fThread.pySetVar("STAXSignalData", signalData);

                setElementInfo(new STAXElementInfo(
                    evalElem, evalAttr,
                    "Invalid import mode: " + fMode +
                    "\nImport mode must be 'error' or 'ignore'."));

                fThread.setSignalMsgVar(
                    "STAXImportErrorMsg", STAXUtil.formatErrorMessage(this));

                fThread.raiseSignal("STAXImportError");

                return;
            }
            
            boolean emptyImportList = false;
            boolean emptyExcludeList = false;
            Vector<String> importList = new Vector<String>();
            Vector<String> excludeList = new Vector<String>();
           
            evalElem = "import-include";
            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;

            if (fUnevalImportInclude.equals(""))
            {
                emptyImportList = true;
            }
            else
            {
                List list = fThread.pyListEval(fUnevalImportInclude);
                Iterator iter = list.iterator();

                while (iter.hasNext())
                    importList.add((String)iter.next());
            }

            evalElem = "import-exclude";

            if (fUnevalImportExclude.equals(""))
            {
                emptyExcludeList = true;
            }
            else
            {
                List list = fThread.pyListEval(fUnevalImportExclude);
                Iterator iter = list.iterator();

                while (iter.hasNext())
                    excludeList.add((String)iter.next());
            }

            evalElem = getElement();
            
            // Check if fMachine contains any STAF variables.

            if (fMachine.indexOf("{") != -1)
            {
                // Resolve variables on the local STAX service machine

                STAFResult result = fThread.getJob().submitSync(
                    "local", "VAR", "RESOLVE STRING " +
                    STAFUtil.wrapData(fMachine));

                if (result.rc != STAFResult.Ok)
                {
                    String errorType = "STAXFileCopyError";

                    String errorMsg = "Error resolving STAF variables in " +
                        "the \"machine\" attribute, RC: " + result.rc +
                        ", Result: " + result.result;

                    evalAttr = "machine";

                    handleImportError(errorType, errorMsg, evalElem, evalAttr);

                    throw new STAXImportException("");
                }

                fMachine = result.result;
            }

            // Check if fFile or fDirectory contains any STAF variables.

            evalAttr = "file";
            String unresValue = fFile;

            if (directorySpecified)
            {
                evalAttr = "directory";
                unresValue = fDirectory;
            }

            if (unresValue.indexOf("{") != -1)
            {
                // Resolve variables on the local STAX service machine

                STAFResult result = fThread.getJob().submitSync(
                    "local", "VAR", "RESOLVE STRING " +
                    STAFUtil.wrapData(unresValue));

                if (result.rc != STAFResult.Ok)
                {
                    String errorType = "STAXFileCopyError";

                    String errorMsg = "Error resolving STAF variables in " +
                        "the \"" + evalAttr + "\" attribute, RC: " +
                        result.rc + ", Result: " + result.result;

                    handleImportError(errorType, errorMsg, evalElem, evalAttr);

                    throw new STAXImportException("");
                }

                if (!directorySpecified)
                    fFile = result.result;
                else
                    fDirectory = result.result;
            }

            // Get the file separator for the machine where the import file/
            // directory resides as this is needed to normalize the path name
            // and may also be needed to determine if its a relative file
            // name, and to get the parent directory.

            STAFResult result = STAXFileCache.getFileSep(
                fMachine, fThread.getJob().getSTAFHandle());

            if (result.rc != STAFResult.Ok)
            {
                String errorType = "STAXNoResponseFromMachine";
                String errMsg = "Error submitting a STAF request " +
                    "to machine " + fMachine + " to get ";

                if (!directorySpecified)
                    errMsg = errMsg + "file " + fFile;
                else
                    errMsg = errMsg = "directory " + fDirectory;

                errMsg = errMsg + ", RC: " + result.rc +
                    ", Result: " + result.result;

                if (machineSpecified)
                    evalAttr = "machine";
                else
                    evalAttr = null;

                handleImportError(errorType, errMsg, evalElem, evalAttr);

                throw new STAXImportException("");
            }

            String fileSep = result.result;

            // Set a flag to indicate if the file name is case-sensitive
            // (e.g. true if the file resides on a Unix machine, false if
            // Windows)

            boolean caseSensitiveFileName = true;

            if (fileSep.equals("\\"))
            {
                // File resides on a Windows machine so file name is
                // not case-sensitive

                caseSensitiveFileName = false;
            }

            // If no machine attribute is specified, then the file specified
            // could be a relative file name so need to assign its absolute
            // file name

            if (!machineSpecified)
            {
                // Check if a relative path was specified

                String entry = fFile;
                evalAttr = "file";

                if (directorySpecified)
                {
                    evalAttr = "directory";
                    entry = fDirectory;
                }

                if (STAXUtil.isRelativePath(entry, fileSep))
                {
                    // Assign the absolute name assuming it is relative
                    // to the STAXCurrentXMLFile's parent path

                    String currentFile = fThread.pyStringEval(
                        "STAXCurrentXMLFile");
                    
                    if (currentFile.equals(STAX.INLINE_DATA))
                    {
                        // Cannot specify a relative file path if
                        // STAXCurrentXmlFile is STAX.INLINE_DATA

                        String errMsg = "Invalid import " + evalAttr + ": " +
                            entry + "\nCannot specify a relative path since" +
                            " STAXCurrentXMLFile=" + STAX.INLINE_DATA;

                        handleImportError(
                            "STAXFileCopyError", errMsg, evalElem, evalAttr);
                     
                        throw new STAXImportException("");
                    }
                    
                    entry = STAXUtil.getParentPath(currentFile, fileSep) +
                        entry;

                    if (!directorySpecified)
                        fFile = entry;
                    else
                        fDirectory = entry;
                }
            }
         
            // Normalize the import file/directory name so that we have a
            // better chance at matching file names that are already cached

            if (!directorySpecified)
                fFile = STAXUtil.normalizeFilePath(fFile, fileSep);
            else
                fDirectory = STAXUtil.normalizeFilePath(fDirectory, fileSep);

            // Create a STAX XML Parser

            STAXParser parser = new STAXParser(fThread.getJob().getSTAX());
            
            // Create a list of files to process

            List<String> theFileList = new ArrayList<String>();

            if (!directorySpecified)
            {
                // Thread will be just one file to process

                theFileList.add(fFile);
            }
            else
            {
                // Submit a FS LIST DIRECTORY request for all *.xml files
                // in the directory

                result = fThread.getJob().submitSync(
                    fMachine, "FS", "LIST DIRECTORY " +
                    STAFUtil.wrapData(fDirectory) +
                    " TYPE F EXT xml CASEINSENSITIVE");

                if (result.rc != 0)           
                {
                    String errorType = "STAXFileCopyError";

                    if (result.rc == STAFResult.NoPathToMachine)
                    {
                        errorType = "STAXNoResponseFromMachine";
                        evalAttr = "machine";
                    }

                    String errorMsg = "Error submitting a FS LIST DIRECTORY" +
                        " request to list all *.xml files in directory \"" +
                        fDirectory + "\" on machine \"" + fMachine +
                        "\", RC: " + result.rc + ", Result: " + result.result;

                    handleImportError(errorType, errorMsg, evalElem, evalAttr);

                    throw new STAXImportException("");
                }

                Iterator dirListIter = ((List)result.resultObj).iterator();

                while (dirListIter.hasNext())
                {
                    theFileList.add(fDirectory + fileSep +
                                    (String)dirListIter.next());
                }
            }
            
            Iterator<String> fileListIter = theFileList.iterator();

            // Indicates whether to continue processing if an error occurs
            continueFlag = true;
            
            while (fileListIter.hasNext() && continueFlag)
            {
                try
                {
                    fFile = fileListIter.next();

                    Date dLastModified = null;
                    STAXJob job = null;

                    // If file caching is enabled, find the modification date of
                    // the file being imported

                    if (fThread.getJob().getSTAX().getFileCaching())
                    {
                        if (STAXFileCache.get().isLocalMachine(fMachine))
                        {
                            File file = new File(fFile);
                    
                            // Make sure the file exists

                            if (file.exists())
                            {
                                long lastModified = file.lastModified();

                                if (lastModified > 0)
                                {
                                    // Chop off the milliseconds because some
                                    // systems don't report mod time to
                                    // milliseconds

                                    lastModified =
                                        ((long)(lastModified/1000))*1000;

                                    dLastModified = new Date(lastModified);
                                }
                            }
                        }
                
                        if (dLastModified == null)
                        {
                            // Find the remote file mod time using STAF

                            STAFResult entryResult =
                                fThread.getJob().submitSync(
                                    fMachine, "FS", "GET ENTRY " +
                                    STAFUtil.wrapData(fFile) + " MODTIME");

                            if (entryResult.rc == 0)
                            {
                                String modDate = entryResult.result;
                                dLastModified = STAXFileCache.convertSTAXDate(
                                    modDate);
                            }
                        }
                
                        // Check for an up-to-date file in the cache

                        if ((dLastModified != null) &&
                            STAXFileCache.get().checkCache(
                                fMachine, fFile, dLastModified,
                                caseSensitiveFileName))
                        {
                            // Get the doc from cache

                            STAXDocument doc = STAXFileCache.get().getDocument(
                                fMachine, fFile, caseSensitiveFileName);
                    
                            if (doc != null)
                            {
                                job = new STAXJob(
                                    fThread.getJob().getSTAX(), doc);
                            }
                        }
                    }
            
                    // If the file was not in cache, then retrieve it

                    if (job == null)
                    {
                        result = fThread.getJob().submitSync(
                            fMachine, "FS", "GET FILE " + fFile);
                               
                        if (result.rc != 0)           
                        {
                            String errorType = "STAXFileCopyError";

                            if (result.rc == STAFResult.NoPathToMachine)
                                errorType = "STAXNoResponseFromMachine";

                            String errorMsg =
                                "FS GET request to import file \"" + fFile +
                                "\" from machine \"" + fMachine +
                                "\" failed, RC: " + result.rc +
                                ", Result: " + result.result;

                            continueFlag = handleImportError(
                                errorType, errorMsg, evalElem, evalAttr);
                     
                            throw new STAXImportException("");
                        }
                
                        // Parse the XML document
                        job = parser.parse(result.result, fFile, fMachine);
                
                        // Add the XML document to the cache
                        if (fThread.getJob().getSTAX().getFileCaching() &&
                            (dLastModified != null))
                        {
                            STAXFileCache.get().addDocument(
                                fMachine, fFile, job.getSTAXDocument(),
                                dLastModified, caseSensitiveFileName);
                        }
                    }
                
                    // Make sure that all the required/requested functions are
                    // in the job's function map

                    fFunctionListDoesNotExist = new ArrayList<String>();
                    fFunctionListNotRequestedNotRequired =
                        new ArrayList<String>();
                    fFunctionListImportedRequested = new ArrayList<String>();
                    fFunctionListImportedRequired = new ArrayList<String>();
                    fFunctionListExistingRequested = new ArrayList<String>();
                    fFunctionListExistingRequired = new ArrayList<String>();

                    fFunctionMap = job.getSTAXDocument().getFunctionMap();

                    fThread.pyExec("import re");

                    // Iterate through the imported xml file's functions

                    for (String nextFunction : fFunctionMap.keySet())
                    {
                        boolean includeMatch = false;
                        boolean excludeMatch = false;

                        if (emptyImportList)
                        {
                            checkForMatch(
                                fFunctionMap, nextFunction, excludeList);
                        }
                        else
                        {
                            boolean included = false;
                    
                            for (int im = 0; im < importList.size(); im++)
                            {
                                includeMatch = grepMatch(
                                    nextFunction, importList.elementAt(im));
                        
                                if (includeMatch)
                                {
                                    included = checkForMatch(fFunctionMap, 
                                        nextFunction, excludeList);
                                }
                            }
                    
                            if (!included && !(fFunctionListImportedRequired.
                                contains(nextFunction)))
                            {
                                if (!(fFunctionListNotRequestedNotRequired.
                                    contains(nextFunction)))
                                {
                                    fFunctionListNotRequestedNotRequired.add(
                                        nextFunction); 
                                }
                            }
                        }
                    }
            
                    // process required functions
                    for (int rf = 0; rf < fRequiredFunctions.size(); rf++)
                    {
                        addRequiredFunctions(fRequiredFunctions.elementAt(rf));
                    }
            
                    // handle any non-"*" functions that were not found
                    for (int im = 0; im < importList.size(); im++)
                    {
                        if (importList.elementAt(im).indexOf("*") == -1)
                        {
                            if (!(fFunctionMap.containsKey(
                                    importList.elementAt(im))) &&
                                !(excludeList.contains(
                                    importList.elementAt(im))))
                            {
                                fFunctionListDoesNotExist.add(
                                    importList.elementAt(im));
                            }
                        }
                    }

                    // Create a PyList with information about the imported
                    // file to be used in the STAXResult variable

                    PyList resultPyList = new PyList();

                    if (directorySpecified)
                        resultPyList.append(new PyString(fFile));
                    else
                        resultPyList.append(Py.None);

                    resultPyList.append(convertJavaListToPyList(
                        fFunctionListImportedRequested));
                    resultPyList.append(convertJavaListToPyList(
                        fFunctionListImportedRequired));
                    resultPyList.append(convertJavaListToPyList(
                        fFunctionListExistingRequested));
                    resultPyList.append(convertJavaListToPyList(
                        fFunctionListExistingRequired));
                    resultPyList.append(convertJavaListToPyList(
                        fFunctionListNotRequestedNotRequired));
                    resultPyList.append(convertJavaListToPyList(
                        fFunctionListDoesNotExist));
                    
                    if (!directorySpecified)
                        fPyResultList = resultPyList;
                    else
                        fPyResultList.append(resultPyList);

                    // The names of the functions that were added to the job's
                    // function map are in the fFunctionListImportedRequested
                    // and fFunctionListImportedRequired lists.  Merge them
                    // into a single list so we can iterate through the merged
                    // list.

                    ArrayList<String> functionList =
                        fFunctionListImportedRequested;
                    functionList.addAll(fFunctionListImportedRequired);

                    // Recursively process any function-import elements for
                    // all the imported functions that were added to the job's
                    // function map

                    try
                    {
                        for (String functionName : functionList)
                        {
                            STAXFunctionAction functionAction =
                                (STAXFunctionAction)fThread.getJob().
                                getSTAXDocument().getFunction(functionName);

                            if (functionAction != null)
                                fThread.getJob().addImportedFunctions(
                                    functionAction);
                        }
                    }
                    catch (Exception e)
                    {
                        fThread.setSignalMsgVar(
                            "STAXFunctionImportErrorMsg", e.toString());
                        fThread.popAction();
                        fThread.raiseSignal("STAXFunctionImportError");

                        return;
                    }
                }
                catch (STAXPythonEvaluationException ex)
                {
                    fThread.popAction();

                    setElementInfo(new STAXElementInfo(evalElem, evalAttr));

                    fThread.setSignalMsgVar(
                        "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this),
                        ex);

                    fThread.raiseSignal("STAXPythonEvaluationError");

                    return;
                }
                catch (STAXXMLParseException ex)
                {
                    continueFlag = handleParserException(ex);
                }
                catch (STAXImportException ex)
                {
                    // Do nothing as the exception has been handled
                }
                catch (STAXException ex)
                {
                    continueFlag = handleParserException(ex);   
                }

                if (!continueFlag) return;

            } // End while (fileListIter.hasNext() && continueFlag)
        }
        catch (STAXPythonEvaluationException ex)
        {
            fThread.popAction();

            setElementInfo(new STAXElementInfo(evalElem, evalAttr));

            fThread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), ex);

            fThread.raiseSignal("STAXPythonEvaluationError");

            return;
        }
        catch (STAXImportException ex)
        {
            if (fMode.equalsIgnoreCase("error")) return;
        }
        catch (STAXException ex)
        {
            handleParserException(ex);   
            
            if (!continueFlag) return;
        }
        catch (SAXNotRecognizedException ex)
        {
            handleParserException(ex);

            if (!continueFlag) return;
        }
        catch (SAXNotSupportedException ex)
        {
            handleParserException(ex);

            if (!continueFlag) return;
        }

        // Set Python variable STAXResult with the result from importing
        // the file or directory

        if (fUnevalDirectory == null)
        {
            fThread.pySetVar("STAXResult", fPyResultList);
        }
        else
        {
            // A directory was specified to be imported

            PyList theResult = new PyList();

            if (!fDirectoryImportError)
            {
                theResult.append(Py.None);
            }
            else
            {
                // Create a PyList containing the STAXImportDirectoryError
                // class and an error message identifying the directory that
                // could not be successfully imported

                String errorMsg = "Directory: " + fDirectory +
                    ", Machine: " + fMachine;

                PyList errorData = createErrorList(
                    "STAXImportDirectoryError", errorMsg);

                theResult.append(errorData);
            }

            theResult.append(fPyResultList);

            fThread.pySetVar("STAXResult", theResult);
        }

        fThread.popAction();
    }

    private boolean handleImportError(String errorType, String errorMsg,
                                      String evalElem, String evalAttr)
        throws STAXPythonEvaluationException
    {
        // Create a PyList containing the errorType class and the error
        // message. 

        PyList errorData = createErrorList(errorType, errorMsg);

        // Check the mode and handle accordingly

        if (fMode.equals(kError))
        {
            // Raise a STAXImportError signal
            
            fThread.pySetVar("STAXSignalData", errorData);

            setElementInfo(new STAXElementInfo(evalElem, evalAttr, errorMsg));

            fThread.setSignalMsgVar("STAXImportErrorMsg",
                                    STAXUtil.formatErrorMessage(this));
            fThread.popAction();  // Setting after setSignalMsgVar so it
                                  // includes the import element in the
                                  // call stack logged in the message
            fThread.raiseSignal("STAXImportError");

            return false; // Do not continue
        }
        else
        {
            // Set fPyResultList as a PyList with errorData as its first
            // element and with empty lists for its next 6 elements:
            // [ errorData,[],[],[],[],[],[] ]

            PyList resultList = new PyList();
            PyList emptyList = new PyList();

            resultList.append(errorData);

            for (int i = 1; i < 7; ++i)
                resultList.append(emptyList);

            if (fDirectory == null)
            {
                fPyResultList = resultList;
            }
            else
            {
                fPyResultList.append(resultList);
                fDirectoryImportError = true;
            }

            return true; // Continue
        }
    }

    public boolean handleParserException(Exception ex)
    {    
        // Create a PyList containing the STAXXMLParseError class and the
        // error message. 

        String errorMsg = "Cause: " + ex.getClass().getName() + "\n";

        // Make sure that the error message contains the File: and Machine:
        // in which the error occurred

        if (ex.getMessage().indexOf("File: ") == -1 ||
            ex.getMessage().indexOf("Machine: ") == -1)
        {
            errorMsg = errorMsg + "\nFile: " + fFile +
                ", Machine: " + fMachine;
        }

        errorMsg = errorMsg + ex.getMessage();

        PyList errorData = createErrorList("STAXXMLParseError", errorMsg);
        
        // Check the mode and handle accordingly

        if (fMode.equals(kError))
        {
            fThread.pySetVar("STAXSignalData", errorData);
            fThread.setSignalMsgVar("STAXImportErrorMsg", errorMsg);
            fThread.popAction();  // Setting after setSignalMsgVar so it
                                  // includes the import element in the
                                  // call stack logged in the message
            fThread.raiseSignal("STAXImportError");
            return false; // Do not continue
        }
        else
        {
            // Set fPyResultList as a PyList with errorData as its first
            // element and with empty lists for its next 6 elements:
            // [ errorData,[],[],[],[],[],[] ]

            PyList resultList = new PyList();
            PyList emptyList = new PyList();

            resultList.append(errorData);

            for (int i = 1; i < 7; ++i)
                resultList.append(emptyList);

            if (fDirectory == null)
            {
                fPyResultList = resultList;
            }
            else
            {
                fPyResultList.append(resultList);
                fDirectoryImportError = true;
            }

            return true;
        }
    }    

    /**
     * Create a PyList containing the errorType class and the error message
     * @param errorType - the name of a STAXUnique class for the error
     * @param errorMsg - the error message
     * @return PyList containing the errorType class object and a PyString
     * containing the error message
     */
    private PyList createErrorList(String errorType, String errorMsg)
    {
        PyList errorData = new PyList();

        try
        {
            errorData.append(fThread.pyObjectEval(errorType));
        }
        catch (STAXPythonEvaluationException e)
        {
            // Should never happen because the errorType should have been
            // defined by STAXThread
            System.out.println(
                "STAXImportAction::createErrorList: fThread.pyObjectEval " +
                "failed for " + errorType);

            // Assign a string containing the error type instead of the
            // error type's class object
            errorData.append(new PyString(errorType));
        }

        errorData.append(new PyString(errorMsg));

        return errorData;
    }

    /**
     * Convert a Java list of String objects into a PyList of PyString objects
     * @param a Java list of String objects
     * @return a PyList of PyString objects* 
     */ 
    private PyList convertJavaListToPyList(List<String> javaList)
    {
        PyList pyList = new PyList();

        for (String entry : javaList)
        {
            pyList.append(new PyString(entry));
        }

        return pyList;
    }

    public void addRequiredFunctions(String currentFunction)
    {
        if (!(fThread.getJob().functionExists(currentFunction)))
        {
            STAXFunctionAction function = 
                (STAXFunctionAction)fFunctionMap.get(currentFunction);        

            fThread.getJob().addFunction(function);
           
            fFunctionListImportedRequired.add(currentFunction);
            fFunctionListNotRequestedNotRequired.remove(currentFunction);

            StringTokenizer requiredFunctions = 
                new StringTokenizer(function.getRequires(), " ");
            
            while (requiredFunctions.hasMoreElements())
            {
                addRequiredFunctions((String)requiredFunctions.nextElement());
            }
        }
        else
        {
            if (!(fFunctionListExistingRequired.contains(currentFunction)) &&
                !(fFunctionListImportedRequired.contains(currentFunction)))
            {
                fFunctionListExistingRequired.add(currentFunction);
            }
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXImportAction clone = new STAXImportAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalMachine = fUnevalMachine;
        clone.fMachine = fMachine;
        clone.fUnevalFile = fUnevalFile;
        clone.fFile = fFile;
        clone.fUnevalDirectory = fUnevalDirectory;
        clone.fDirectory = fDirectory;
        clone.fUnevalMode = fUnevalMode;
        clone.fMode = fMode;
        clone.fUnevalReplace = fUnevalReplace;
        clone.fReplace = fReplace;
        clone.fUnevalImportInclude = fUnevalImportInclude;
        clone.fUnevalImportExclude = fUnevalImportExclude;
        clone.fImportInclude = fImportInclude;
        clone.fImportExclude = fImportExclude;

        return clone;
    }
    
    private boolean grepMatch(String targetString, String matchString)
        throws STAXPythonEvaluationException
    {
        return fThread.pyBoolEval("re.match('" + matchString + 
           "', '" + targetString + "')");
    }
    
    private boolean checkForMatch(HashMap functionMap, String function,
                                  Vector<String> excludeList) 
                                  throws STAXPythonEvaluationException
    {
        boolean excludeMatch = false;
        boolean included = false;
        
        for (int ex = 0; ex < excludeList.size(); ex++)
        {
            if (grepMatch(function, excludeList.elementAt(ex)))
            {
                excludeMatch = true;
            }
        }
                    
        if (!excludeMatch)
        {
            handleFunctionImport(functionMap, function);
            
            included = true;
        }
        else
        {
            if (!(fFunctionListNotRequestedNotRequired.contains(function)))
            {
                fFunctionListNotRequestedNotRequired.add(function);
            }
        }
        
        return included;
    }
    
    private void handleFunctionImport(HashMap functionMap, 
                                      String functionToImport)
    {
        STAXFunctionAction function = 
            (STAXFunctionAction)functionMap.get(functionToImport);
                    
        StringTokenizer requiredFunctions = 
            new StringTokenizer(function.getRequires(), " ");
            
        while (requiredFunctions.hasMoreTokens())
        {
            fRequiredFunctions.add(requiredFunctions.nextToken());
        }
                    
        if (!(fThread.getJob().functionExists(functionToImport)) || fReplace)
        {
            // Add/replace the function
            
            fThread.getJob().addFunction(function);
            
            fFunctionListImportedRequested.add(functionToImport);
        }
        else
        {
            // Function already exists or has already been imported
            // (and fReplace is not true)
            
            fFunctionListExistingRequested.add(functionToImport);
        }
    }

    STAXThread fThread = null;    
    private String fUnevalMachine = new String();
    private String fMachine = new String();    
    private String fUnevalFile = new String();
    private String fFile = new String();
    private String fUnevalDirectory = new String();
    private String fDirectory = new String();
    private String fUnevalMode = new String();
    private String fMode = new String();
    private String fUnevalReplace = new String();
    private boolean fReplace = false;
    private String fUnevalImportInclude = new String();
    private String fUnevalImportExclude = new String();
    private String fImportInclude = new String();
    private String fImportExclude = new String();
    
    private ArrayList<String> fFunctionListDoesNotExist;
    private ArrayList<String> fFunctionListNotRequestedNotRequired;
    private ArrayList<String> fFunctionListImportedRequested;
    private ArrayList<String> fFunctionListImportedRequired;
    private ArrayList<String> fFunctionListExistingRequested;
    private ArrayList<String> fFunctionListExistingRequired;

    private PyList fPyResultList = new PyList();
    private boolean fDirectoryImportError = false;

    private Vector<String> fRequiredFunctions = new Vector<String>();
    private HashMap<String, STAXFunctionAction> fFunctionMap;
}