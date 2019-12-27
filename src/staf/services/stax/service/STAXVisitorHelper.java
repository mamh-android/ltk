/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.Iterator;

public abstract class STAXVisitorHelper implements STAXVisitor
{
    STAXVisitorHelper(Object o)
    { fData = o; }

    STAXVisitorHelper(Object o1, Object o2)
    { fData = o1; fData2 = o2; }

    public abstract void visit(Object o, Iterator iter);

    Object fData;
    Object fData2 = null;
}
