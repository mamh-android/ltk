/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.Comparator;

public class STAXObjectComparator implements Comparator
{
    public int compare(Object o1, Object o2)
    {
        if (o1.hashCode() < o2.hashCode()) return -1;
        else if (o1.hashCode() > o2.hashCode()) return 1;

        return 0;
    }
}
