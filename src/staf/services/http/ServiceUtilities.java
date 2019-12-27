package com.ibm.staf.service.utility;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
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
/* Class: ServiceUtilities                                                   */
/* Description: Provides some common functions required by the FSEXT service */
/*              All functions are provided as static methods.                */
/*                                                                           */
/*****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.io.File;
import java.util.HashMap;
import java.util.Vector;

public class ServiceUtilities
{
/*****************************************************************************/
/*                                                                           */
/* Method: createMapOfFiles                                                  */
/* Description: creates a HashMap of all files contained in a directory      */
/*              this will navigate all subdirectories                        */
/* Parameters: dir - the directory to get files from                         */
/* Returns: a HashMap using the fully qualified filenames (String) as the    */
/*          keys and a Boolean object with a value of false as the values    */
/*                                                                           */
/*****************************************************************************/

public static HashMap createMapOfFiles(File dir)
{
    File[] files = dir.listFiles();
    HashMap fileMap = new HashMap();

    for(int i=0; i < files.length; i++) {
        if(files[i].isFile()) {// add filename to map with a value of Boolean(false)
            fileMap.put(files[i].getPath(), new Boolean(false));
        }
        else{//reenter this method to resolve the subdir
            HashMap subDirFiles = createMapOfFiles(files[i]);
            fileMap.putAll(subDirFiles);
        }
    }

    return fileMap;
}

/*****************************************************************************/
/*                                                                           */
/* Method: lineContainsStrings                                               */
/* Description: determines whether a given String contains a list of Strings */
/* Parameters: line - the String to search                                   */
/*             strings - a Vector of Strings to search for                   */
/*             ignoreCase - if true perform a case insensitive search,       */
/*                          if false perform case sensitive search           */
/* Returns: true if all Strings were found in the search String, false       */
/*          otherwise                                                        */
/*                                                                           */
/*****************************************************************************/

public static boolean lineContainsStrings(String line, Vector strings,
                                          boolean ignoreCase)
{
    int numStrings = strings.size();
    boolean searchResult = true;
    for(int i=0; i < numStrings; i++) {
        String tempS = (String) strings.get(i);
        if(ignoreCase) {
            tempS = tempS.toLowerCase();
        }
        if(line.indexOf(tempS) == -1) {// at least one of the strings was not found
            searchResult = false;
            break;
        }
    }

    return searchResult;
}
}
