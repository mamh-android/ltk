/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;
import java.util.StringTokenizer;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import com.ibm.staf.*;
import com.ibm.staf.service.*;

public class STAXFunctionActionFactory implements STAXActionFactory,
                                                  STAXListRequestHandler,
                                                  STAXQueryRequestHandler
{
    static final String sFunctionListInfoMapClassName = new String(
        "STAF/Service/STAX/FunctionListInfo");
    static final String sQueryFunctionMapClassName = new String(
        "STAF/Service/STAX/QueryFunction");
    static final String sQueryFunctionImportMapClassName = new String(
        "STAF/Service/STAX/QueryFunctionImport");

    private static String fDTDInfo =
"\n" +
"<!--================= The Function Element ========================= -->\n" +
"<!--\n" +
"     The function element defines a named task which can be called.\n" +
"     The name, requires, and scope attribute values are literals.\n" +
"     If desired, the function can be described using a function-prolog\n" +
"     element (or the deprecated function-description element) and/or a\n" +
"     function-epilog element.  Also, if desired, the function element\n" +
"     can define the arguments that can be passed to the function.\n" +
"     The function element can also define any number of function-import\n" +
"     elements if it requires functions from other xml files.  The\n" +
"     function-import element must specify either the file or directory\n" +
"     attribute.\n" +
"-->\n" +
"<!ELEMENT function    ((function-prolog | function-description)?,\n" +
"                       (function-epilog)?,\n" +
"                       (function-import)*,\n" +
"                       (function-no-args | function-single-arg |\n" +
"                        function-list-args | function-map-args)?,\n" +
"                       (%task;))>\n" +
"<!ATTLIST function\n" +
"          name         ID       #REQUIRED\n" +
"          requires     IDREFS   #IMPLIED\n" +
"          scope        (local | global) \"global\"\n" +
">\n" +
"\n" +
"<!ELEMENT function-prolog       (#PCDATA)>\n" +
"\n" +
"<!ELEMENT function-epilog       (#PCDATA)>\n" +
"\n" +
"<!ELEMENT function-description  (#PCDATA)>\n" +
"\n" +
"<!ELEMENT function-import       (#PCDATA)>\n" +
"<!ATTLIST function-import\n" +
"          file         CDATA    #IMPLIED\n" +
"          directory    CDATA    #IMPLIED\n" +
"          machine      CDATA    #IMPLIED\n" +
">\n" +
"<!ELEMENT function-no-args      EMPTY>\n" +
"\n" +
"<!ELEMENT function-single-arg   (function-required-arg |\n" +
"                                 function-optional-arg |\n" +
"                                 function-arg-def)>\n" +
"\n" +
"<!ELEMENT function-list-args    ((((function-required-arg+,\n" +
"                                    function-optional-arg*) |\n" +
"                                  (function-required-arg*,\n" +
"                                    function-optional-arg+)),\n" +
"                                 (function-other-args)?) |\n" +
"                                 function-arg-def+)>\n" +
"\n" +
"<!ELEMENT function-map-args     (((function-required-arg |\n" +
"                                   function-optional-arg)+,\n" +
"                                  (function-other-args+)?) |\n" +
"                                  function-arg-def+)>\n" +
"\n" +
"<!ELEMENT function-required-arg (#PCDATA)>\n" +
"<!ATTLIST function-required-arg\n" +
"          name         CDATA    #REQUIRED\n" +
">\n" +
"\n" +
"<!ELEMENT function-optional-arg (#PCDATA)>\n" +
"<!ATTLIST function-optional-arg\n" +
"          name         CDATA    #REQUIRED\n" +
"          default      CDATA    \"None\"\n" +
">\n" +
"\n" +
"<!ELEMENT function-other-args   (#PCDATA)>\n" +
"<!ATTLIST function-other-args\n" +
"          name         CDATA    #REQUIRED\n" +
">\n" +
"\n" +
"<!ELEMENT function-arg-def      (function-arg-description?,\n" +
"                                 function-arg-private?,\n" +
"                                 function-arg-property*)>\n" +
"<!ATTLIST function-arg-def\n" +
"          name         CDATA    #REQUIRED\n" +
"          type         (required | optional | other) \"required\"\n" +
"          default      CDATA    \"None\"\n" +
">\n" +
"\n" +
"<!ELEMENT function-arg-description  (#PCDATA)>\n" +
"\n" +
"<!ELEMENT function-arg-private   EMPTY>\n" +
"\n" +
"<!ELEMENT function-arg-property  (function-arg-property-description?,\n" +
"                                 function-arg-property-data*)>\n" +
"<!ATTLIST function-arg-property\n" +
"          name         CDATA    #REQUIRED\n" +
"          value        CDATA    #IMPLIED\n" +
">\n" +
"\n" +
"<!ELEMENT function-arg-property-description  (#PCDATA)>\n" +
"\n" +
"<!ELEMENT function-arg-property-data (function-arg-property-data)*>\n" +
"<!ATTLIST function-arg-property-data\n" +
"          type         CDATA    #REQUIRED\n" +
"          value        CDATA    #IMPLIED\n" +
">\n";

    private STAFMapClassDefinition fFunctionListInfoMapClass;
    private STAFMapClassDefinition fQueryFunctionMapClass;
    private STAFMapClassDefinition fQueryFunctionImportMapClass;

    public STAXFunctionActionFactory(STAX staxService)
    {
        staxService.registerListHandler("FUNCTIONS", this);
        staxService.registerQueryHandler("FUNCTION", "Function Name", this);

        // Construct map-class for LIST JOB <Job ID> FUNCTIONS information

        fFunctionListInfoMapClass = new STAFMapClassDefinition(
            sFunctionListInfoMapClassName);

        fFunctionListInfoMapClass.addKey("function", "Function");
        fFunctionListInfoMapClass.addKey("file", "File");
        fFunctionListInfoMapClass.addKey("machine", "Machine");

        // Construct map-class for query function information for a job

        fQueryFunctionMapClass = new STAFMapClassDefinition(
            sQueryFunctionMapClassName);

        fQueryFunctionMapClass.addKey("function", "Function");
        fQueryFunctionMapClass.addKey("file", "File");
        fQueryFunctionMapClass.addKey("machine", "Machine");
        fQueryFunctionMapClass.addKey("scope", "Scope");
        fQueryFunctionMapClass.addKey("requires", "Requires");
        fQueryFunctionMapClass.addKey("imports", "Imports");

        // Construct map-class for query function import info for a job

        fQueryFunctionImportMapClass = new STAFMapClassDefinition(
            sQueryFunctionImportMapClassName);

        fQueryFunctionImportMapClass.addKey("file", "File");
        fQueryFunctionImportMapClass.addKey("directory", "Directory");
        fQueryFunctionImportMapClass.addKey("machine", "Machine");
        fQueryFunctionImportMapClass.addKey("functions", "Functions");
    }

    public STAFResult handleListRequest(String type, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("functions"))
        {
            // LIST JOB <Job ID> FUNCTIONS

            // Create the marshalling context and set its map class definitions
            // and create an empty list to contain the function entries

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fFunctionListInfoMapClass);
            List<Map<String, Object>> functionList =
                new ArrayList<Map<String, Object>>();

            // Iterate through the sorted function map

            for (STAXFunctionAction function :
                 job.getSTAXDocument().getSortedFunctionMap().values())
            {
                Map<String, Object> functionMap = new TreeMap<String, Object>();
                functionMap.put("staf-map-class-name",
                                fFunctionListInfoMapClass.name());
                functionMap.put(
                    "function", (String)function.getName());
                functionMap.put("file", (String)function.getXmlFile());
                functionMap.put("machine", (String)function.getXmlMachine());

                functionList.add(functionMap);
            }

            mc.setRootObject(functionList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryRequest(String type, String key, STAXJob job,
                                         STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("function"))
        {
            // QUERY JOB <Job ID> FUNCTION <Function Name>

            // Create the marshalling context and set its map class definitions

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fQueryFunctionMapClass);
            mc.setMapClassDefinition(fQueryFunctionImportMapClass);

            STAXFunctionAction function = (STAXFunctionAction)job.getFunction(
                key);
            
            if (function == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "Function '" + key + "' does not exist."); 
            }

            Map<String, Object> functionMap = new TreeMap<String, Object>();
            functionMap.put("staf-map-class-name",
                            fQueryFunctionMapClass.name());
            functionMap.put("function", key);
            functionMap.put("file", function.getXmlFile());
            functionMap.put("machine", function.getXmlMachine());
            functionMap.put("scope", function.getScope());

            // Convert string of required functions into list of functions
                
            List<String> reqFunctionList = new ArrayList<String>();
            StringTokenizer requiresString = new StringTokenizer(
                function.getRequires(), " ");

            while (requiresString.hasMoreTokens())
            {
                reqFunctionList.add(requiresString.nextToken());
            }

            functionMap.put("requires", reqFunctionList);

            // Process function-imports

            List<STAXFunctionImport> functionImportList =
                function.getImportList();
            List<Map<String, Object>> importList =
                new ArrayList<Map<String, Object>>();
            
            if (functionImportList != null)
            {
                for (STAXFunctionImport importData : functionImportList)
                {
                    Map<String, Object> importMap = new TreeMap<String, Object>();
                    importMap.put("staf-map-class-name",
                                  fQueryFunctionImportMapClass.name());

                    // Note that the file/directory name and machine values
                    // are those, that were specified in the function-import
                    // element, not their resolved, normalized values

                    String file = importData.getFile();

                    if (file != null)
                        importMap.put("file", importData.getFile());
                    else
                        importMap.put("directory", importData.getDirectory());

                    importMap.put("machine", importData.getMachine());

                    List<String> functionList = new ArrayList<String>();

                    if (importData.getFunctions().length() != 0)
                    {
                        // Convert string of functions into list of functions

                        requiresString = new StringTokenizer(
                            importData.getFunctions());

                        while (requiresString.hasMoreTokens())
                        {
                            functionList.add(requiresString.nextToken());
                        }
                    }
                    
                    importMap.put("functions", functionList);

                    importList.add(importMap);
                }
            }

            functionMap.put("imports", importList);

            mc.setRootObject(functionMap);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryJobRequest(STAXJob job,
                                            STAXRequestSettings settings)
    {
        return new STAFResult(STAFResult.Ok, ""); 
    }

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return null;  // The function element is not a task element.
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXFunctionAction function = new STAXFunctionAction();

        function.setLineNumber(root);
        function.setXmlFile(job.getXmlFile());
        function.setXmlMachine(job.getXmlMachine());

        String functionName = new String();
        String scope = new String();
        String requires = new String();
        STAXAction functionAction = null;
        
        // Used to make sure all argument names are unique for a function
        HashSet<String> argNameSet = new HashSet<String>();

        // Used to make sure that for function-arg-def arguments, "required"
        // and "optional" args are specified before "other" args
        boolean functionArgDefOtherSpecified = false;
        
        NamedNodeMap rootAttrs = root.getAttributes();

        for (int i = 0; i < rootAttrs.getLength(); ++i)
        {
            Node thisAttr = rootAttrs.item(i);

            if (thisAttr.getNodeName().equals("name"))
            {
                function.setName(thisAttr.getNodeValue());
            }
            else if (thisAttr.getNodeName().equals("scope"))
            {
                function.setScope(thisAttr.getNodeValue());
            }
            else if (thisAttr.getNodeName().equals("requires"))
            {
                function.setRequires(thisAttr.getNodeValue());
            }
        }

        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                function.setLineNumber(thisChild);

                if (thisChild.getNodeName().equals("function-description") ||
                    thisChild.getNodeName().equals("function-prolog"))
                {
                    function.setProlog(handleChild(thisChild, function));
                }
                else if (thisChild.getNodeName().equals("function-epilog"))
                {
                    function.setEpilog(handleChild(thisChild, function));
                }
                else if (thisChild.getNodeName().equals("function-import"))
                {
                    // Get required file attribute and optional machine and
                    // functions attributes
                    
                    String file = null;
                    String directory = null;
                    String machine = null;
                    String functions = null;

                    NamedNodeMap childAttrs = thisChild.getAttributes();

                    for (int j = 0; j < childAttrs.getLength(); ++j)
                    {
                        Node thisAttr = childAttrs.item(j);

                        if (thisAttr.getNodeName().equals("file"))
                        {
                            file = thisAttr.getNodeValue();
                        }
                        else if (thisAttr.getNodeName().equals("directory"))
                        {
                            directory = thisAttr.getNodeValue();
                        }
                        else if (thisAttr.getNodeName().equals("machine"))
                        {
                            machine = thisAttr.getNodeValue();
                        }
                    }

                    if ((file != null) && (directory != null))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "Only one of the following attributes " +
                            "are allowed:  file | directory"));

                        throw new STAXInvalidXMLAttributeException(
                            STAXUtil.formatErrorMessage(function),
                            function);
                    }
                    else if ((file == null) && (directory == null))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "One of the following attributes must be " +
                            "specified: file | directory"));

                        throw new STAXInvalidXMLAttributeException(
                            STAXUtil.formatErrorMessage(function),
                            function);
                    }

                    functions = handleChild(thisChild, function);

                    if ((directory != null) && (functions.length() != 0))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "Cannot specify functions if the \"directory\" " +
                            "attribute is specified."));

                        throw new STAXInvalidXMLAttributeException(
                            STAXUtil.formatErrorMessage(function),
                            function);
                    }
                    
                    function.addToImportList(
                        file, directory, machine, functions);
                }
                else if (thisChild.getNodeName().equals("function-no-args"))
                {
                    function.setArgDefinition(
                        STAXFunctionAction.FUNCTION_ALLOWS_NO_ARGS);
                }
                else if (thisChild.getNodeName().equals("function-single-arg"))
                {
                    function.setArgDefinition(
                        STAXFunctionAction.FUNCTION_DEFINES_ONE_ARG);

                    // Iterate child element to get a required/option arg
                    handleArg(thisChild, function, argNameSet,
                              functionArgDefOtherSpecified);
                }
                else if (thisChild.getNodeName().equals("function-list-args"))
                {
                    function.setArgDefinition(
                        STAXFunctionAction.FUNCTION_DEFINES_LIST_ARGS);

                    // Iterate child elements to get required/optional args
                    handleArg(thisChild, function, argNameSet,
                              functionArgDefOtherSpecified);
                }
                else if (thisChild.getNodeName().equals("function-map-args"))
                {
                    function.setArgDefinition(
                        STAXFunctionAction.FUNCTION_DEFINES_MAP_ARGS);

                    // Iterate child elements to get required/optional args
                    handleArg(thisChild, function, argNameSet,
                              functionArgDefOtherSpecified);
                }
                else
                {
                    if (functionAction != null)
                    {
                        function.setElementInfo(
                            new STAXElementInfo(
                                thisChild.getNodeName(),
                                STAXElementInfo.NO_ATTRIBUTE_NAME,
                                STAXElementInfo.LAST_ELEMENT_INDEX,
                                "Invalid element type \"" +
                                thisChild.getNodeName() + "\""));

                        throw new STAXInvalidXMLElementCountException(
                            STAXUtil.formatErrorMessage(function),
                            function);
                    }

                    STAXActionFactory factory = 
                        staxService.getActionFactory(thisChild.getNodeName());

                    if (factory == null)
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "No action factory for element type \"" +
                            thisChild.getNodeName() + "\""));

                        throw new STAXInvalidXMLElementException(
                            STAXUtil.formatErrorMessage(function),
                            function);
                    }

                    functionAction = factory.parseAction(
                        staxService, job, thisChild);

                    function.setAction(functionAction);
                }
            }
            else
            {
                function.setElementInfo(new STAXElementInfo(
                    root.getNodeName(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(function), function);
            }
        }

        return function;
    }
    
    private String handleChild(Node root, STAXFunctionAction action)
                               throws STAXException
    {
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            // XXX: Should I be able to have a COMMENT_NODE here?

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.CDATA_SECTION_NODE)
            {
                return thisChild.getNodeValue();
            }
            else if (thisChild.getNodeType() == Node.TEXT_NODE)
            {
                String value = thisChild.getNodeValue().trim();
                
                if (!value.equals(""))
                {
                    return value;
                }
            }
            else
            {
                action.setElementInfo(
                    new STAXElementInfo(
                        root.getNodeName(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        "Contains invalid node type: " +
                        Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);

            }
        }

        return new String();
    }

    void handleArg(Node root, STAXFunctionAction function,
                   HashSet<String> argNameSet,
                   boolean functionArgDefOtherSpecified) throws STAXException
    {
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                function.setLineNumber(thisChild);

                if (thisChild.getNodeName().equals("function-required-arg"))
                {
                    int    type = STAXFunctionAction.ARG_REQUIRED;
                    String name = new String();
                    String defaultValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();
                        }
                    }

                    if (name.equals(""))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The \"name\" attribute's value cannot be null."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }
                    else if (!argNameSet.add(name))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The value \"" + name + "\" has already been " +
                            "used.  The name for each argument defined for " +
                            "a function must be unique."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }
                   
                    function.addArg(new STAXFunctionArgument(
                        name, STAXFunctionAction.ARG_REQUIRED,
                        handleChild(thisChild, function)));
                }
                else if (thisChild.getNodeName().equals(
                    "function-optional-arg"))
                {
                    String name = new String();
                    String defaultValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();
                        }
                        else if (thisAttr.getNodeName().equals("default"))
                        {
                            function.setElementInfo(new STAXElementInfo(
                                thisChild.getNodeName(),
                                thisAttr.getNodeName(),
                                STAXElementInfo.LAST_ELEMENT_INDEX));

                            defaultValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), function);
                        }
                    }

                    if (name.equals(""))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The \"name\" attribute's value cannot be null."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }
                    else if (!argNameSet.add(name))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The value \"" + name + "\" has already been " +
                            "used.  The name for each argument defined for " +
                            "a function must be unique."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }
                        
                    function.addArg(
                        new STAXFunctionArgument(
                            name, STAXFunctionAction.ARG_OPTIONAL,
                            defaultValue,
                            handleChild(thisChild, function)));
                }
                else if (thisChild.getNodeName().equals("function-other-args"))
                {
                    String name = new String();
                    String defaultValue = new String();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();
                        }
                    }

                    if (name.equals(""))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The \"name\" attribute's value cannot be null."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }
                    else if (!argNameSet.add(name))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The value \"" + name + "\" has already been " +
                            "used.  The name for each argument defined for " +
                            "a function must be unique."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }

                    function.addArg(
                        new STAXFunctionArgument(
                            name, STAXFunctionAction.ARG_OTHER, defaultValue,
                            handleChild(thisChild, function)));
                }
                else if (thisChild.getNodeName().equals("function-arg-def"))
                {
                    function.setDefinedWithFunctionArg(true);

                    String name = new String();
                    String type = new String();
                    String defaultValue = new String();
                    String description = new String();
                    int functionType = 0;

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            name = thisAttr.getNodeValue();
                        }
                        else if (thisAttr.getNodeName().equals("type"))
                        {
                            type = thisAttr.getNodeValue();
                            
                            if (type.equals("required"))
                            {
                                if (functionArgDefOtherSpecified)
                                {
                                    function.setElementInfo(new STAXElementInfo(
                                        thisChild.getNodeName(), "type",
                                        STAXElementInfo.LAST_ELEMENT_INDEX,
                                        "Required arguments must be defined" +
                                        " before arguments with type " +
                                        "\"other\"."));

                                    throw new STAXInvalidFunctionArgumentException(
                                        STAXUtil.formatErrorMessage(function), function);
                                }

                                functionType = STAXFunctionAction.ARG_REQUIRED;
                            }
                            else if (type.equals("optional"))
                            {
                                if (functionArgDefOtherSpecified)
                                {
                                    function.setElementInfo(new STAXElementInfo(
                                        thisChild.getNodeName(), "type",
                                        STAXElementInfo.LAST_ELEMENT_INDEX,
                                        "Optional arguments must be defined" +
                                        " before arguments with type " +
                                        "\"other\"."));

                                    throw new STAXInvalidFunctionArgumentException(
                                        STAXUtil.formatErrorMessage(function), function);
                                }

                                functionType = STAXFunctionAction.ARG_OPTIONAL;
                            }
                            else if (type.equals("other"))
                            {
                                functionType = STAXFunctionAction.ARG_OTHER;
                                functionArgDefOtherSpecified = true;
                            }
                        }
                        else if (thisAttr.getNodeName().equals("default"))
                        {
                            function.setElementInfo(new STAXElementInfo(
                                thisChild.getNodeName(),
                                thisAttr.getNodeName(),
                                STAXElementInfo.LAST_ELEMENT_INDEX));

                            defaultValue = STAXUtil.parseAndCompileForPython(
                                thisAttr.getNodeValue(), function);
                        }
                    }

                    if (name.equals(""))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The \"name\" attribute's value cannot be null."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }
                    else if (!argNameSet.add(name))
                    {
                        function.setElementInfo(new STAXElementInfo(
                            thisChild.getNodeName(), "name",
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            "The value \"" + name + "\" has already been " +
                            "used.  The name for each argument defined for " +
                            "a function must be unique."));

                        throw new STAXInvalidFunctionArgumentException(
                            STAXUtil.formatErrorMessage(function), function);
                    }

                    // Iterate child elements to get the arg-def sub-elements
                    STAXFunctionArgument arg = handleArgDef(
                        thisChild, function, argNameSet);

                    arg.setName(name);
                    arg.setType(functionType);
                    arg.setDefaultValue(defaultValue);
                                                 
                    function.addArg(arg);
                }
            }
            else
            {
                function.setElementInfo(new STAXElementInfo(
                    root.getNodeName(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(function), function);
            }
        }
    }

    STAXFunctionArgument handleArgDef(Node root, STAXFunctionAction function,
                                      HashSet<String> argNameSet)
                                      throws STAXException
    {
        STAXFunctionArgument arg = new STAXFunctionArgument();

        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                if (thisChild.getNodeName().equals("function-arg-description"))
                {
                    arg.setDescription(handleChild(thisChild, function));
                }

                if (thisChild.getNodeName().equals("function-arg-private"))
                {
                    arg.setPrivate(true);
                }
                if (thisChild.getNodeName().equals("function-arg-property"))
                {
                    STAXFunctionArgumentProperty argProperty =
                        new STAXFunctionArgumentProperty();

                    NamedNodeMap attrs = thisChild.getAttributes();

                    for (int j = 0; j < attrs.getLength(); ++j)
                    {
                        Node thisAttr = attrs.item(j);

                        if (thisAttr.getNodeName().equals("name"))
                        {
                            argProperty.setName(thisAttr.getNodeValue());
                        }
                        else if (thisAttr.getNodeName().equals("value"))
                        {
                            argProperty.setValue(thisAttr.getNodeValue());
                        }
                    }
                    
                    NodeList propertyChildren = thisChild.getChildNodes();

                    for (int k = 0; k < propertyChildren.getLength(); ++k)
                    {
                        Node thisPropertyChild = propertyChildren.item(k);

                        if (thisPropertyChild.getNodeType() ==
                            Node.COMMENT_NODE)
                        {
                            /* Do nothing */
                        }
                        else if (thisPropertyChild.getNodeType() ==
                                 Node.ELEMENT_NODE)
                        {
                            if (thisPropertyChild.getNodeName().equals(
                                "function-arg-property-description"))
                            {
                                argProperty.setDescription(
                                    handleChild(thisPropertyChild, function));
                            }
                            else if (thisPropertyChild.getNodeName().equals(
                                "function-arg-property-data"))
                            {
                                STAXFunctionArgumentPropertyData argPropData =
                                   handleArgPropertyData(thisPropertyChild);

                                argProperty.addData(argPropData);
                            }
                        }
                    }
                    
                    arg.addProperty(argProperty);
                }
            }
        }

        return arg;
    }
    
    STAXFunctionArgumentPropertyData handleArgPropertyData(Node root)
                                                           throws STAXException
    {
        STAXFunctionArgumentPropertyData propData = new
            STAXFunctionArgumentPropertyData();

        NamedNodeMap attrs = root.getAttributes();

        for (int j = 0; j < attrs.getLength(); ++j)
        {
            Node thisAttr = attrs.item(j);

            if (thisAttr.getNodeName().equals("type"))
            {
                propData.setType(thisAttr.getNodeValue());
            }
            else if (thisAttr.getNodeName().equals("value"))
            {
                propData.setValue(thisAttr.getNodeValue());
            }
        }
            
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                if (thisChild.getNodeName().equals(
                        "function-arg-property-data"))
                {
                    propData.addData(handleArgPropertyData(thisChild));
                }
            }
        }

        return propData;
    }
}
