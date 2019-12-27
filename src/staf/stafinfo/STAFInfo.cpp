/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAF_iostream.h"
#include "STAFException.h"
#include "STAFDynamicLibrary.h"

extern "C"
{
typedef char * (*infoFunc)(void);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <STAF Library>..." << endl << endl
             << "For example, " << endl << endl
             << argv[0] << " libSTAF.so libJSTAF.so" << endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        try
        {
            cout << "Processing library " << argv[i] << endl;

            STAFDynamicLibrary lib(argv[i]);
            infoFunc info = (infoFunc)lib.getAddress("STAFGetInformation");

            cout << info() << endl;
        }
        catch (STAFException &se)
        { se.write("main()"); }
        catch (...)
        { cout << "Caught unknown exception in main()" << endl; }
    }

    return 0;
}
