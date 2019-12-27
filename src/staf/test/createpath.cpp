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
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <directory>" << endl;
        return 1;
    }

    try
    {
        STAFFSPath path(argv[1]);
        STAFFSEntryPtr dir = path.createDirectory(kSTAFFSCreatePath);
    }
    catch (STAFException &se)
    { se.write("main"); }

    return 0;
}
