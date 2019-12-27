/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import org.w3c.dom.Node;

/**
 * An interface for creating a STAX action based on information obtained by
 * parsing an XML node from a DOM tree for elements whose DTD information
 * it provides.
 * This interface defines the DTD information for one or more XML elements,
 * the name of an element to add to the STAX DTD's task entity (if any), and
 * how to parse the XML element.
 * <p>
 * There is just one instance of each factory class.
 * All STAX action factory classes must implement this interface.
 *
 * @see STAXAction
 */
public interface STAXActionFactory
{
    /**
     * Returns the DTD information for an XML element.
     *
     * @return a string containing the DTD information for an XML element
     */
    public String getDTDInfo();

    /**
     * Returns the name of an element that should be added to the task
     * entity in the STAX DTD.  This allows the element to embedded within
     * other STAX elements that contain one or more task elements.  
     * Return "" if no element needs to be added to the task entity in
     * the STAX DTD.
     *
     * @return a string containing an element that should be added to
     * the task entity in the STAX DTD
     */
    public String getDTDTaskName();

    /**
     * Parses the XML element, creating a STAXAction object that contains
     * the information obtained from parsing a node from the DOM tree that
     * represents the XML element and returns the STAXAction object created.
     *
     * @param  staxService an object representing the STAX Service
     * @param  job         an object representing the STAX job
     * @param  root        the XML node from a DOM tree representing an action
     *                     element
     * @return a new STAXAction object representing the XML element
     * @throws STAXException
     *           if an error occurs while parsing the XML element
     * @see STAXAction
     */
    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root)
                      throws STAXException;
}
