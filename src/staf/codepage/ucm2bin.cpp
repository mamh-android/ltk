/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/* ************************************************************************* */
/*                                                                           */
/* ucm2bin                                                                   */
/* Benjamin S. Vera-Tudela                                                   */ 
/* IBM Corp. - 4/10/2000                                                     */
/*                                                                           */
/* This utility program takes a ucm file (from the IBM Classes for Unicode   */
/* package) and converts it into a binary format that STAFConverter uses to  */
/* do codepage translation from/to UTF-8/local code page.                    */
/*                                                                           */
/* ************************************************************************* */

#include "STAFOSTypes.h"
#include <cstring>
#include "STAF_iostream.h"
#include "STAFConverter.h"

/* ************************************************************************* */

int main(int argc, char **argv, char **envp)
{
    if (argc < 2)
    {
        cout << "Usage: ucm2bin <ucm files ...>" << endl;
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        char *ucmFile = argv[i];
        char  cnvName[256] = { 0 };

        strcpy(cnvName, ucmFile);

        char *dot = strrchr(cnvName, '.');

        if (dot == 0)
        {
            cout << "Invalid file name: " << ucmFile << endl;
            return 1;
        }

        // replace the '.' with a null
        *dot = 0; 

        char *ext = dot + 1;

        if (strcmp(ext, "ucm") != 0)
        {
            cout << "Invalid file name: " << ucmFile << endl;
            return 1;
        }

        cout << "Converting " << cnvName << " ... " << flush;

        STAFConverter conv(cnvName, STAFConverter::kUCM);

        cout << "Done" << endl;
    }

    return 0;
}

/* ************************************************************************* */
