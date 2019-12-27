/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.lang.Math.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import javax.swing.table.*;
import java.lang.reflect.*;
import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.text.SimpleDateFormat;

/**
 * @deprecated  As of STAF 3.1.0.  Use the STAFVersion class provided in the
 *              JSTAF.jar file instead.
 */
public class STAXVersion
{
    private String fVersion;
    private int[]  fVersionArray = { 0, 0, 0, 0 };
    private String fText = new String("");

    public static int NUM_VERSION_LEVELS = 4;

    /*
       A STAX version must be of the format unless it's blank or "<N/A>",
       which equates to "no version" and is internally represented as
       0.0.0.0
        
           a[.b[.c[.d]]] [text]
    
       where:
       - a, b, c, and d (if specified) are numeric
       - text is separated by one or more spaces from the version numbers
       
       Otherwise, a STAXException is thrown. 
    */   

    public STAXVersion(String version) throws NumberFormatException
    {
        fVersion = version;

        String versionStr = new String("");

        // Verify that the version is valid

        if (version.equals("") || version.equals("<N/A>"))
        {
            // Do nothing
            return;
        }
        else
        {
            // Separate any text from the numeric version in version1

            int spaceIndex = fVersion.indexOf(" ");

            if (spaceIndex != -1)
            {
                versionStr = fVersion.substring(0, spaceIndex);
                fText = fVersion.substring(spaceIndex + 1).trim();
            }
            else
            {
                versionStr = fVersion;
            }
        }

        // Assign the versionArray values from the dot-separated numeric
        // values in versionStr.  If .b or .c or .d do not exist, then
        // .0 is substituted such that 2.5 == 2.5.0.0

        int dotIndex = -1;

        for (int i = 0; i < NUM_VERSION_LEVELS; i++)
        {
            dotIndex = versionStr.indexOf(".");

            if (dotIndex == -1)
            {
                if (!versionStr.equals(""))
                {
                    fVersionArray[i] = (new Integer(versionStr)).intValue();
                }

                break;
            }
            else
            {
                fVersionArray[i] = (new Integer(
                    versionStr.substring(0, dotIndex))).intValue();

                if (dotIndex < (versionStr.length() - 1))
                    versionStr = versionStr.substring(dotIndex + 1);
                else
                    versionStr = "";
            }
        }
    }

    public String getVersion()
    {
        return fVersion;
    }

    public int[] getVersionArray()
    {
        return fVersionArray;
    }

    public String getText()
    {
        return fText;
    }

    /*
       Compares this STAXVersion object to the given STAXVersion object.
       
       - If fVersion is equal to another version's fVersion, returns 0.
       - Each entry in fVersionArray is numerically compared with each
         corresponding entry in another version's fVersionArray.
       - If its fVersionArray is "equal" to another version's fVersionArray,
         then do:
         1) If fText is "" but another version's fText is not "", then return
            1 to indicate that this version is greater than another version.
         2) If fText is not "" but another version's fText is "", then return
            -1 to indicate that this version is less than another version.
         3) Return the result of a case-insensitive string compare of fText
            with another versions fText. 
       
       Note:  No version, e.g. blank or <N/A> has a fVersionVector set to
              {0,0,0,0} so that no version is less than any valid version.
   
       Parameters:
       
       version - the STAXVersion object to be compared to this STAXVersion
                 object.
       
       Returns:
       
       - The value 0 if the two STAXVersion objects are equal
       - A value less than 0 if this STAXVersion object is less than the
         given argument
       - A value greater than 0 if this STAXVersion object is greater than
         the given argument.
    */   

    public int compareTo(STAXVersion version)
    {
        // Check if the two versions are equal and, if so, return 0.

        if (fVersion.equals(version.getVersion()))
            return 0;
        
        // Compare numeric versions stored in the fVersionArray

        int[] versionArray = version.getVersionArray();

        for (int i = 0; i < NUM_VERSION_LEVELS; i++)
        {
            if (fVersionArray[i] < versionArray[i])
                return -1;
            else if (fVersionArray[i] > versionArray[i])
                return 1;
        }

        // Versions are equal so compare text

        if (fText.equals("") && !version.getText().equals(""))
            return 1;  
        else if (!fText.equals("") && version.getText().equals(""))
            return -1;
        else
            return fText.compareToIgnoreCase(version.getText());
    }

    public String toString()
    {
        return fVersion;
    }
}
