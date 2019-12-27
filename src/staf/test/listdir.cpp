/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAFFileSystem.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <directory>..." << endl << endl
             << "Environment variables:" << endl
             << "STAF_FS_NAME_PATTERN sets the name pattern (default = '*')"
             << endl
             << "STAF_FS_EXTPATTERN sets the extension pattern (default = '*')"
             <<endl;
        return 1;
    }

    STAFString namePattern("*");
    STAFString extPattern("*");

    if (getenv("STAF_FS_NAME_PATTERN") != 0)
        namePattern = getenv("STAF_FS_NAME_PATTERN");

    if (getenv("STAF_FS_EXT_PATTERN") != 0)
        extPattern = getenv("STAF_FS_EXT_PATTERN");

    for (int i = 1; i < argc; ++i)
    {
        try
        {
            STAFFSEntryPtr dir = STAFFSPath(argv[i]).getEntry();

            if (dir->type() != kSTAFFSDirectory)
            {
                cout << argv[i] << " is not a directory" << endl;
                continue;
            }

            STAFFSEnumPtr dirEnum = dir->enumerate(namePattern, extPattern,
                                                   kSTAFFSNormal);

            cout << endl << argv[i] << ":" << endl;

            for (; dirEnum->isValid(); dirEnum->next())
            {
                cout << "    " << dirEnum->entry()->path().setRoot(
                                  STAFString()).clearDirList().asString()
                     << endl;
            }
        }
        catch (STAFException &se)
        { se.write("main"); }
    }

    return 0;
}
