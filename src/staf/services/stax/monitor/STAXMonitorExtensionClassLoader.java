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
import java.net.URL;
import javax.swing.ImageIcon;

public class STAXMonitorExtensionClassLoader extends ClassLoader
{
    static public final boolean DEBUG = false;
    static public final String STAX_MONITOR_CLASS_PREFIX = 
        "STAX-INF/classes/";
        
    static public final String STAX_MONITOR_IMAGE_PREFIX = 
        "STAX-INF/images/";

    STAXMonitorExtensionClassLoader(JarFile jarFile, ClassLoader loader)
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
            JarEntry classEntry = fJarFile.getJarEntry(
                STAX_MONITOR_CLASS_PREFIX + name.replace('.', '/') + ".class");
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
    
    public ImageIcon getImage(String name)
    {
        name = STAX_MONITOR_IMAGE_PREFIX + name;

        JarEntry entry = fJarFile.getJarEntry(name);
                   
        int c, i = 0;
        byte buffer[] = new byte[0];
        
        try
        {
            InputStream in = fJarFile.getInputStream(entry);
                       
            buffer = new byte[(int)entry.getSize()];
        
            while((c = in.read()) != -1)
            {
                buffer[i] = (byte)c;
                i++;
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
                   
        ImageIcon image = new ImageIcon(buffer);

        return image;
    }

    JarFile fJarFile;
}
