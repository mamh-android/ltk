/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.staxdoc;

import java.util.*;

/******************************************************************************
* Stores command line options and global variables.
******************************************************************************/
public class Options
{
  public String destDir = null;

  public String srcPathString = ".";

  public boolean verbose = false;
  public String doctitle = "";
  public String windowtitle = "STAX Documentation Generator (STAXDoc)";

  public String overviewFilename = null;

  public List packagePathsList = new ArrayList();
  public List packageNamesList = new ArrayList();

  // Create static variables for the valid functionSummary option

  public static final String FIRSTSENTENCE = "FirstSentence";
  public static final String ALL = "All";
  private static List fFunctionSummaryValues = new ArrayList();
  
  // The functionSummary option defaults to the FIRSTSENTENCE of the prolog
  public String functionSummary = FIRSTSENTENCE;

  public Options()
  {
      fFunctionSummaryValues.add(FIRSTSENTENCE);
      fFunctionSummaryValues.add(ALL);
  }

  static public List getFunctionSummaryValues()
  {
      return fFunctionSummaryValues;
  }

  // Checks if the value specified for the function summary option is
  // valid.  If valid, returns true.  If invalid, returns false.

  static public boolean validFunctionSummaryValue(String optionValue)
  {
      Iterator iter = fFunctionSummaryValues.iterator();

      while (iter.hasNext())
      {
          String value = (String)iter.next();
          if (optionValue.equalsIgnoreCase(value))
          {
              return true;
          }
      }
      
      return false;
  }

}
