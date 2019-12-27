/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

/**
 * This class is used to compare STAF versions. This class is useful if you
 * want to verify that a STAF version or a version of a STAF service is at a
 * particular level of STAF. 
 *
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFVersion">
 *      Section "3.4.14 Class STAFVersion" in the STAF Java User's
 *      Guide</a>
 */ 
public class STAFVersion
{
    private String fVersion;
    private int[]  fVersionArray = { 0, 0, 0, 0 };
    private String fText = new String("");

    /**
     * The number of numeric version levels to compare.  The default is 4 which
     * compares all 4 of the version levels in a version such as 3.4.14.1.
     * The value specifies should be from 1 to 4.
     */ 
    public static int NUM_VERSION_LEVELS = 4;

    /**
     * This constructs an object representing a STAF Version that contains up
     * to four numeric levels and an optional text string.
     * 
     * @param  version A String representing a STAF version that has the
     * following format (unless it is blank which equates to "no version" and
     * is internally represented as 0.0.0.0):
     * <p>
     * <code>a[.b[.c[.d]]] [text]</code>
     * <p>
     * where:
     * <ul>
     * <li> a, b, c, and d (if specified) are numeric
     * <li> text is separated by one or more spaces from the version numbers
     * </ul>  
     * <p>
     * A NumberFormatException is thrown if a non-numeric value is specified
     * in a, b, c, or d.. 
     */
    public STAFVersion(String version) throws NumberFormatException
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

    /**
     * Gets the string format of the STAFVersion object containing the version
     * specified when constructing a new STAFVersion object. 
     * 
     * @return A String containing the version specified when constructing a
     *         new STAFVersion object. 
     */
    public String getVersion()
    {
        return fVersion;
    }

    /**
     * Gets the array (of size 4) of numeric version levels for this
     * STAFVersion object.
     * 
     * @return An int[] of size 4 (the maximum number of version levels) where
     *         each integer in the array is a version level for this
     *         STAFVersion object.
     */ 
    public int[] getVersionArray()
    {
        return fVersionArray;
    }

    /**
     * Gets any optional text specified in the version. 
     * 
     * @return The text specified for this STAFVersion object.
     */ 
    public String getText()
    {
        return fText;
    }

    /**
     * This method compares STAF versions.  STAFVersion instances are compared
     * as follows:
     * <ul>
     * <li>The numeric versions (<code>a[.b[.c[.d]]]</code>) are numerically
     *     compared (up to the number of levels specified by
     *     <code>NUM_VERSION_LEVELS</code>.
     * <li>If the numeric versions are "equal", then the <code>[text]</code>
     *     values are compared using a case-insensitive string compare.
     *     Except, note that no text is considered greater than any text
     *     (e.g. "3.1.0" > "3.1.0 Beta 1") .
     * </ul>
     * <p>
     * Here are some examples of STAFVersion comparisons:
     * <ul>
     * <li><code>"3" = "3.0" = "3.0.0" = 3.0.0.0"</code>
     * <li><code>"3.0.0" < "3.1.0"</code>
     * <li><code>"3.0.2" < "3.0.3"</code>
     * <li><code>"3.0.0" < "3.1"</code>
     * <li><code>"3.0.9" < "3.0.10"</code>
     * <li><code>"3.1.0 Alpha 1" < "3.1.0 Beta 1"</code>
     * <li><code>"3.1.0 Beta 1" < "3.1.0"</code>
     * </ul>
     *
     * @param version  A STAFVersion object representing the STAF version to
     *                 be compared.
     * @return         If the two STAFVersion objects are equal, returns 0.
     *                 Or if this STAFVersion object is less than
     *                 <code>version</code>, returns -1.
     *                 Or if this STAFVersion object is greater than
     *                 <code>version</code>, returns 1.
     */
    public int compareTo(STAFVersion version)
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

    /**
     * Returns the string format of the STAFVersion object containing the version
     * specified when constructing a new STAFVersion object. 
     * 
     * @return A String containing the version specified when constructing a
     *         new STAFVersion object. 
     */
    public String toString()
    {
        return fVersion;
    }
}
