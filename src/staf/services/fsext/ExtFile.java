package com.ibm.staf.service.fsext;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Author: Chris Alkov                                                       */
/* Date: 12/2001                                                             */
/* Revisions:                                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: ExtFile                                                            */
/* Description: This class extends java.io.File and provides the ability to  */
/*              capture or compare two files.                                */
/*                                                                           */
/*****************************************************************************/

import java.io.File;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Vector;

public class ExtFile extends File 
{
    private boolean saveFailures = false;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameters: file - A File to use to create an ExtFile                     */
/*                                                                           */
/*****************************************************************************/

public ExtFile(File file) 
{    
    super(file.getPath());  
    
}

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method inherited from File                       */
/* Parameters: Same as for java.io.File                                      */
/*                                                                           */
/*****************************************************************************/

public ExtFile(File parent, String child) 
{
    super(parent, child);
}

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method inherited from File                       */
/* Parameters: Same as for java.io.File                                      */
/*                                                                           */
/*****************************************************************************/

public ExtFile(String pathname) 
{
    super(pathname);
}

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method inherited from File                       */
/* Parameters: Same as for java.io.File                                      */
/*                                                                           */
/*****************************************************************************/

public ExtFile(String parent, String child) 
{
    super(parent, child);
}

/*****************************************************************************/
/*                                                                           */
/* Method: captureFile                                                       */
/* Description: Copies this file to another file                             */
/* Parameters: dest - File to use as the destination                         */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/

public void captureFile(File dest) throws IOException 
{    
    BufferedInputStream in = null;
    BufferedOutputStream out = null;

    try 
    {   
        in = new BufferedInputStream(new FileInputStream(this));
        out = new BufferedOutputStream(new FileOutputStream(dest));

        while(true) 
        {
            int data = in.read();
            
            if (data == -1) 
            {
                break;
            }
            out.write(data);
        }
        
    }
    finally 
    { // use finally block to ensure that streams are closed
        try 
        {
            out.close();
            in.close();
        }
        catch(NullPointerException npe) {} //don't care
    }   

}

/*****************************************************************************/
/*                                                                           */
/* Method: captureFile                                                       */
/* Description: Copies this file to another file (overloaded)                */
/* Parameters: dest - filename to use as the destination                     */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/

public void captureFile(String dest) throws IOException 
{    
    File destFile = new File(dest);
    captureFile(destFile);
    
}

/*****************************************************************************/
/*                                                                           */
/* Method: compareFile                                                       */
/* Description: Compares this file to another file, optionally saving this   */
/*              file if the comaparison fails                                */
/* Parameters: model - File to compare this file against                     */
/* Returns: true if files compared OK, false otherwise                       */
/*                                                                           */
/*****************************************************************************/
 
public boolean compareFile(File model) throws IOException 
{    
    boolean result;

    BufferedInputStream in1 = null;
    BufferedInputStream in2 = null;
    
    try 
    {
        in1 = new BufferedInputStream(new FileInputStream(this));       
        in2 = new BufferedInputStream(new FileInputStream(model));

        while (true) 
        {
            int data1 = in1.read();
            int data2 = in2.read();

            /* Throw away carriage return chars (0D).
                We do this because of the difference in new line sequences across platforms.
                Unix uses 0A and Win32 uses 0D0A (2-byte sequence) */

            if (data1 == 0x0D) 
            {
                data1 = in1.read();
            }
            
            if (data2 == 0x0D) 
            {
                data2 = in2.read();
            }

            if (data1 != data2) 
            {
                result = false;
                
                if (saveFailures) 
                {
                    File saveName = new File(model.getParent(), 
                        this.getName()+".fail");
                    this.captureFile(saveName);
                }
                break;
            }
    
            if (data1 == -1) 
            {
                result = true;
                break;
            }
        }
    }
    finally 
    {
        try 
        {
            in1.close();
            in2.close();
        }
        catch(NullPointerException npe) 
        {
            //don't care
        }
    }
    
    return result;
    
}

/*****************************************************************************/
/*                                                                           */
/* Method: compareFile                                                       */
/* Description: Optionally performs a lexicographic sort, then compares this */
/*              file to another file, optionally saving this file if the     */
/*              comaparison fails (overloaded)                               */
/* Parameters: model - File to compare this file against                     */
/*             sort - if true first perform sort, if false do not sort       */
/* Returns: true if files compared OK, false otherwise                       */
/*                                                                           */
/*****************************************************************************/

public boolean compareFile(File model, boolean sort) throws IOException 
{
    if (!sort) 
    { // do not sort
        return compareFile(model);
    }

    ExtFile sortedSource = this.sort();
    ExtFile sortedModel = new ExtFile(model).sort();

    boolean result = sortedSource.compareFile(sortedModel);

    if (!result && saveFailures) 
    {
        File saveName = new File(model.getParent(), 
            this.getName()+".fail");
        this.captureFile(saveName);
    }
    
    sortedSource.delete();
    sortedModel.delete();

    return result;
        
}

/*****************************************************************************/
/*                                                                           */
/* Method: compareFile                                                       */
/* Description: Compares this file to another file, optionally saving this   */
/*              file if the comaparison fails (overloaded)                   */
/* Parameters: model - filename of file to compare this file against         */
/* Returns: true if files compared OK, false otherwise                       */
/*                                                                           */
/*****************************************************************************/

public boolean compareFile(String model) throws IOException 
{
    File modelFile = new File(model);
    return compareFile(modelFile);
    
}

/*****************************************************************************/
/*                                                                           */
/* Method: compareFile                                                       */
/* Description: Optionally performs a lexicographic sort, then compares this */
/*              file to another file, optionally saving this file if the     */
/*              comaparison fails (overloaded)                               */
/* Parameters: model - filename of file to compare this file against         */
/*             sort - if true first perform sort, if false do not sort       */
/* Returns: true if files compared OK, false otherwise                       */
/*                                                                           */
/*****************************************************************************/

public boolean compareFile(String model, boolean sort) throws IOException 
{
    File modelFile = new File(model);
    return compareFile(modelFile, sort);
    
}

/*****************************************************************************/
/*                                                                           */
/* Method: isSaveFailures                                                    */
/* Description: Access method (get) for saveFailures field. saveFailures     */
/*              specifies if a file should be saved when a compare fails     */
/*                                                                           */
/*****************************************************************************/

public boolean isSaveFailures() 
{
    return saveFailures;
}

/*****************************************************************************/
/*                                                                           */
/* Method: isSaveFailures                                                    */
/* Description: Access method (set) for saveFailures field. saveFailures     */
/*              specifies if a file should be saved when a compare fails     */
/*                                                                           */
/*****************************************************************************/

public void setSaveFailures(boolean newSaveFailures) 
{
    saveFailures = newSaveFailures;
}

/*****************************************************************************/
/*                                                                           */
/* Method: sort                                                              */
/* Description: Performs a lexicographic sort on this file. The sort is      */
/*              performed on a line by line basis                            */
/* Parameters: none                                                          */
/* Returns: an ExtFile which represents the sorted version of this ExtFile   */
/*                                                                           */
/*****************************************************************************/

public ExtFile sort() throws IOException 
{
    /* Read each line of file and store as String Array */

    Vector unsorted = new Vector();
    BufferedReader in = null;
    
    try 
    {
        in = new BufferedReader(new FileReader(this));
        
        while (true) 
        {
            String line = in.readLine();
            
            if (line == null) 
            {
                break;
            }
            
            unsorted.add(line);
        }
    }
    finally 
    {
        try 
        {
            in.close();
        }
        catch(NullPointerException npe) {} //don't care
    }
    
    Object[] foo = new String[0];
    String[] sort = (String[]) unsorted.toArray(foo);

    /* Lexicographically sort array */

    for (int i=sort.length; --i >= 0; ) 
    {
        for (int j=0; j < i; j++) 
        {
            if (sort[j].compareTo(sort[j+1]) >= 0) 
            {
                String temp = sort[j];
                sort[j] = sort[j+1];
                sort[j+1] = temp;
            }
        }
    }

    /* Write sorted file back out */

    BufferedWriter out = null;
    ExtFile sortedFile = new ExtFile(this.getPath()+".sorted");

    try 
    {
        out = new BufferedWriter(new FileWriter(sortedFile));
        
        for (int i=0; i < sort.length; i++) 
        {
            out.write(sort[i]);
            out.newLine();
        }
        
        out.flush();
    }
    finally 
    {
        try 
        {
            out.close();
        }
        catch(NullPointerException npe) {} // don't care
    }
    
    return sortedFile;
    
}
}
