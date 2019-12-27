/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
package com.ibm.staf.service.stax.staxdoc;

import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.text.MessageFormat;

/******************************************************************************
* Handle Resources.
******************************************************************************/
class ResourceHandler
{
  private static ResourceBundle messages;

  /////////////////////////////////////////////////////////////////////////////
  static void initResource()
  {
    try
    {
      messages = ResourceBundle.getBundle("resources.staxdoc");
    }
    catch (MissingResourceException e)
    {
      throw new Error("Fatal: Resource for staxdoc is missing");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  static String getText(String key)
  {
    return getText(key, (String) null);
  }

  static String getText(String key, String a1)
  {
    return getText(key, a1, (String) null);
  }

  static String getText(String key, String a1, String a2)
  {
    if (messages == null)
    {
      initResource();
    }
    try
    {
      String message = messages.getString(key);
      String[] args = new String[2];
      args[0] = a1;
      args[1] = a2;
      return MessageFormat.format(message, (Object[])args);
    }
    catch (MissingResourceException e)
    {
      throw new Error("Resource is broken. There is no " + key + " key in resource.");
    }
  }



}
