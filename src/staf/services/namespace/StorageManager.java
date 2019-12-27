/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;

import com.ibm.staf.*;
import com.ibm.staf.service.*;

import java.io.File;
import java.util.Iterator;
import java.util.Map;

import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.apache.xerces.jaxp.DocumentBuilderFactoryImpl;
    
/**
 * Manages the storage of the persistent data for Namespaces.
 * This is an implementation of the XMLManipulator interface.
 * This implementation provides methods to read from a Namespaces XML
 * file and to write to a Namespaces XML file.
 * <p>
 * Note that this storage manager could be changed to store the
 * Namespaces persistent data in a format other than XML without
 * requiring any changes to other classes in the Namespace service.
 * 
 * @see XMLManipulator
 */ 

public class StorageManager extends XMLManipulator
{
    private NamespaceManager fNamespaceManager;
    private String fFileName;  // Fully-qualified namespaces file name
    private File fFile;
    
    /** Relative filename of the DTD file provided in the service jar */
    public final static String sDTD = "resources/Namespaces.dtd";

    /** Name of the XML file where this service stores the namespaces data */
    protected static String sXmlFileName = "Namespaces.xml";

    /**
     * Creates a new StorageManager instance.
     * If the Namespaces xml file exists, loads the NamespaceManager's
     * map of namespaces with Namespace instances.
     * 
     * @param dataDir The name of the directory where the Namespaces xml file
     * should be stored
     * @param nsMgr The NamespaceManager object that contains the map of
     * namespaces
     * @throws NSException if errors occur while reading or writing the
     * Namespaces xml file
     */
    public StorageManager(String dataDir, NamespaceManager nsMgr) throws
        NSException
    {
        this(dataDir, sXmlFileName, nsMgr);
    }

    public StorageManager(String dataDir, String fileName,
                          NamespaceManager nsMgr) throws NSException
    {
        fNamespaceManager = nsMgr;

        sXmlFileName = fileName;

        super.setDtdFile(sDTD);

        fFile = new File(dataDir, sXmlFileName);
        fFileName = fFile.toString();
        super.setXmlFile(fFileName);

        if (fFile.exists())
        {
            loadNamespaces();
        }
    }

    /**
     * Returns the fully-qualified name of the file that contains the
     * namespaces data.
     * @return the fully-qualified name of the Namespaces file 
     */ 
    public String getFileName()
    {
        return fFileName;
    }

    /**
     * Returns a File object that contains the namespaces data.
     * @return the File object that contains the namespaces data. 
     */ 
    public File getFile()
    {
        return fFile;
    }
    
    /**
     * Returns the short name of the file that contains the namespaces
     * data (e.g. Namespaces.xml).
     * @return the short name of the Namespaces file
     */ 
    public String getShortFileName()
    {
        return sXmlFileName;
    }

    /**
     * Populates the NamespaceManager's map of namespaces with the namespaces
     * stored in persistent data.
     * 
     * Reads and validates the Namespaces XML file and creates a Namespace
     * instance for each namespace and adds the loads the NamespaceManager's
     * map of namespaces with these namespaces
     *
     * @throws NSException if errors occur while reading the Namespaces xml
     * file
     */ 
    public void loadNamespaces() throws NSException
    {
        // Delete any namespaces curently in the Namespaces map
        fNamespaceManager.deleteAll();

        Document doc = loadDocument();
        Node root = doc.getDocumentElement();
        root.normalize();
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node elem = children.item(i);

            if ((elem.getNodeType() == Node.ELEMENT_NODE) &&
                (elem.getNodeName().equals("namespace")))
            {
                Namespace namespace = fromXML(elem, Namespace.sNONE);
            }
        }
    }
    
    /**
     * Saves the namespaces in the NamespaceManager's map to persistent data.
     * 
     * Writes the contents of the Namespaces XML file by converting the
     * namespaces in the NamespacesManager's map to XML elements.
     *
     * * @throws NSException if errors occur while writing the Namespaces xml
     * file
     */ 
    public void saveNamespaces() throws NSException
    {
        // Create a XML Document

        Document doc = null;

        try
        {
            DocumentBuilderFactory factory = DocumentBuilderFactoryImpl.
                newInstance();
            factory.setValidating(true);
            DocumentBuilder docBuilder = factory.newDocumentBuilder();
            doc = docBuilder.newDocument();
        }
        catch (javax.xml.parsers.ParserConfigurationException e)
        {
            throw new NSException(
                "Error creating XML document when saving namespaces: " +
                e.getMessage());
        }
        
        Iterator iter = fNamespaceManager.getNamespaceMapCopy().
            values().iterator();

        Element root = doc.createElement("namespaces");
        doc.appendChild(root);

        while (iter.hasNext())
        {
            Namespace namespace = (Namespace)iter.next();

            if (namespace.getParent().equalsIgnoreCase(Namespace.sNONE))
            {
                Element elem = toXML(namespace, doc);
                root.appendChild(elem);
            }
        }

        this.saveDocument(doc);
    }

    /**
     * Creates a Namespace instance from each &lt;namespace> element in
     * the Namespaces XML file.  This includes assigning variables from
     * any &lt;var> elements that the &lt;namespace> element may contain.
     * This also includes creating any children namespaces recursively
     * that may be inscluded in the &lt;namespace> element.
     * 
     * @param nsElem an instance of &lt;namespace> element in the XML file
     * @param parent the name of the parent namespace
     * @return an instance of a Namespace element
     * @throws NSException if errors occur reading the Namespaces xml file
     */ 
    private Namespace fromXML(Node nsElem, String parent) throws
        NSException
    {
        // Get the name and description attributes that are required for
        // a namespace element

        String name = "";
        String description = "";

        NamedNodeMap attrs = nsElem.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node attr = attrs.item(i);

            if (attr.getNodeName().equals("name"))
            {
                name = attr.getNodeValue();
            }
            else if (attr.getNodeName().equals("description"))
            {
                description = attr.getNodeValue();
            }
        }
        
        // Make sure that a namespace with this name does not already exist

        if (fNamespaceManager.contains(name))
        {
            throw new NSException(
                "Please correct following error in XML file '" + fFileName +
                "':\nNamespace with name=\"" + name + "\" already exists.  " +
                "The name of a namespace must be unique (case-insensitive).");
        }

        // Create the namespace and add to the Namespace Map

        Namespace namespace = new Namespace(name, description, parent);
        fNamespaceManager.create(name, namespace);

        // Add any variables for the namespace

        NodeList children = nsElem.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node child = children.item(i);

            if ((child.getNodeType() == Node.ELEMENT_NODE) &&
                (child.getNodeName() == "var"))
            {
                // Get the key and value attributes that are required
                // for a var element

                String key = "";
                String value = "";

                NamedNodeMap varAttrs = child.getAttributes();

                for (int j = 0; j < varAttrs.getLength(); ++j)
                {
                    Node varAttr = varAttrs.item(j);

                    if (varAttr.getNodeName().equals("key"))
                    {
                        key = varAttr.getNodeValue();
                    }
                    else if (varAttr.getNodeName().equals("value"))
                    {
                        value = varAttr.getNodeValue();
                    }

                }

                // Add any variables for the namespace

                namespace.setVariable(key, value);
            }
        }

        // Add any child namespaces for the namespace

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node child = children.item(i);

            if ((child.getNodeType() == Node.ELEMENT_NODE) &&
                (child.getNodeName() == "namespace"))
            {
                // Get the name attribute for a namespace element

                NamedNodeMap nsAttrs = child.getAttributes();

                for (int j = 0; j < nsAttrs.getLength(); ++j)
                {
                    Node nsAttr = nsAttrs.item(j);

                    if (nsAttr.getNodeName().equals("name"))
                    {
                        String childName = nsAttr.getNodeValue();

                        // Create child namespaces recursively

                        Namespace childNS = fromXML(child, name);
                        namespace.addChild(childName, childNS);
                    }
                }
            }
        }
        
        return namespace;
    }

    /**
     * Creates a &lt;namespace> element from a Namespace instance.
     * This includes creating the &lt;var> sub-elements.
     * This also includes creating any children &lt;namespace> elements
     * recursively that may be included in the &lt;namespace> element.
     * 
     * @param ns an instance of a Namespace
     * @param doc the XML document being created
     * @return an instance of an XML Element representing the namespace
     */ 
    private Element toXML(Namespace ns, Document doc)
    {
        // Create a namespace element

        // Add Namespace Element and its attributes
        Element nsElem = doc.createElement("namespace");
        nsElem.setAttribute("name", ns.getName());
        nsElem.setAttribute("description", ns.getDescription());
        
        // Create var elements for the namespace

        Iterator varIter = ns.getVariables().values().iterator();

        while (varIter.hasNext())
        {
            Variable variable = (Variable)varIter.next();
            String varKey = variable.getKey();
            String varValue = variable.getValue();
            
            Element varElem = doc.createElement("var");
            varElem.setAttribute("key", varKey);
            varElem.setAttribute("value", varValue);

            nsElem.appendChild(varElem);
        }
        
        // Add children namespace elements recursively

        Iterator childrenIter = ns.getChildren().values().iterator();

        while (childrenIter.hasNext())
        {
            Namespace childNS = (Namespace)childrenIter.next();
            Element child = toXML(childNS, doc);
            
            nsElem.appendChild(child);
        }
        
        return nsElem;
    }
}

