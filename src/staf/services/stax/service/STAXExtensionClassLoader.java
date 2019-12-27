/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import com.ibm.staf.*;
import java.util.jar.*;
import java.io.IOException;
import java.io.InputStream;

public class STAXExtensionClassLoader extends ClassLoader
{
    static public final boolean DEBUG = false;
    static public final String STAX_CLASS_PREFIX = "STAX-INF/classes/";

    STAXExtensionClassLoader(JarFile jarFile, ClassLoader loader)
    {
        super(loader);
        fJarFile = jarFile;
    }

    public Class findClass(String name) throws ClassNotFoundException
    {
        if (DEBUG) System.out.println("Loading: " + name);

        Class theClass = null;

        try
        {
            JarEntry classEntry = fJarFile.getJarEntry(STAX_CLASS_PREFIX +
                                                       name.replace('.', '/') +
                                                       ".class");
            if (classEntry != null)
            {
                InputStream classStream = fJarFile.getInputStream(classEntry);
                byte [] classData = new byte[(int)classEntry.getSize()];
                int bytesToRead = classData.length;

                for (int bytesRead = 0; bytesToRead > 0;
                     bytesToRead -= bytesRead)
                {
                    bytesRead = classStream.read(classData,
                                                 classData.length - bytesToRead,
                                                 bytesToRead);
                }

                theClass = defineClass(name, classData, 0, classData.length);
            }
        }
        catch (IOException e)
        {
            if (DEBUG)
            {
                System.out.println("Caught IOException reading from " +
                                   "jar file " + fJarFile.getName() + ":");
                e.printStackTrace();
            }
        }

        if (theClass == null) throw new ClassNotFoundException(name);

        return theClass;
    }

    JarFile fJarFile;
}
